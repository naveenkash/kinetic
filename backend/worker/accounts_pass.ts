// The worker's account pass (backend-architecture 19): export archives, deletions after their
// 14-day grace, and storage retention. Storage is reached through ObjectStore, so tests can
// stand one in; the worker uses Supabase Storage with the service key.
//
// Jobs: a job left `running` for RECLAIM_AFTER_S (the worker died mid-job) is taken again. A
// failed job is retried with backoff (from a minute, doubling) and failed for good after
// MAX_JOB_ATTEMPTS; the runbook covers failed jobs.
import type postgres from "npm:postgres@3.4.5";
import type { WorkerContext } from "./loop.ts";

export interface ObjectStore {
  upload(bucket: string, path: string, body: string, contentType: string): Promise<void>;
  remove(bucket: string, paths: string[]): Promise<void>;
  list(bucket: string, prefix: string): Promise<string[]>;
}

export interface RetentionScope {
  // Tests limit retention to their own matches; the worker leaves this unset.
  matchIds?: string[];
}

const JOBS_PER_PASS = 10;
const RECLAIM_AFTER_S = 30 * 60;
export const MAX_JOB_ATTEMPTS = 6;
const RETRY_BASE_S = 60;
export const RETENTION_EVERY_S = 24 * 3600;
// Section 19 (PROPOSED): replays 30 days, raw match events 90, kept while an open player report
// or a quarantined match report needs them; export archives 7 days after they were written.
const REPLAY_DAYS = 30;
const EVENTS_DAYS = 90;
const EXPORT_DAYS = 7;
const RETENTION_BATCH = 100;
// Batches per retention run; a backlog larger than this finishes on the next tick.
const RETENTION_BATCHES = 50;

const message = (e: unknown) => (e instanceof Error ? e.message : String(e));

// Claims due jobs of one kind, so a second worker instance cannot run the same job; a job
// running too long is taken again.
async function claimJobs(sql: postgres.Sql, kind: string) {
  return await sql<{ job_id: string; account_id: string | null }[]>`
    update game.account_jobs set status = 'running', started_at = now(), attempts = attempts + 1
     where job_id in (
       select job_id from game.account_jobs
        where kind = ${kind}
          and ((status = 'queued' and run_after <= now())
               or (status = 'running' and started_at < now() - make_interval(secs => ${RECLAIM_AFTER_S})))
        order by run_after limit ${JOBS_PER_PASS} for update skip locked)
    returning job_id, account_id`;
}

// Both touch only a job still running: one the player cancelled meanwhile stays cancelled.
async function jobDone(sql: postgres.Sql, jobId: string, path: string | null = null) {
  await sql`update game.account_jobs set status = 'done', result_path = ${path}, error = null, completed_at = now()
             where job_id = ${jobId} and status = 'running'`;
}

async function jobFailed(sql: postgres.Sql, jobId: string, error: string) {
  await sql`
    update game.account_jobs
       set status = case when attempts >= ${MAX_JOB_ATTEMPTS} then 'failed' else 'queued' end,
           error = ${error},
           run_after = now() + make_interval(secs => ${RETRY_BASE_S} * power(2, least(attempts, 10))),
           completed_at = case when attempts >= ${MAX_JOB_ATTEMPTS} then now() end
     where job_id = ${jobId} and status = 'running'`;
}

async function runExports(ctx: WorkerContext): Promise<void> {
  for (const job of await claimJobs(ctx.sql, "export")) {
    try {
      if (!job.account_id) throw new Error("account_gone");
      const [row] = await ctx.sql<{ archive: unknown }[]>`select private.account_archive(${job.account_id}) as archive`;
      const path = `${job.account_id}/${job.job_id}.json`;
      await ctx.storage!.upload("exports", path, JSON.stringify(row.archive, null, 2), "application/json");
      await jobDone(ctx.sql, job.job_id, `exports/${path}`);
    } catch (e) {
      await jobFailed(ctx.sql, job.job_id, message(e));
    }
  }
}

// Deletes the account in one transaction that holds the job's row, so a cancel waits for it
// or wins before it: the job must still be running when its exports are removed and
// private.delete_account runs (sanctions retained, payloads anonymized, the Auth trail and
// user deleted).
async function runDeletions(ctx: WorkerContext): Promise<void> {
  for (const job of await claimJobs(ctx.sql, "deletion")) {
    try {
      if (!job.account_id) throw new Error("account_gone");
      const account = job.account_id;
      await ctx.sql.begin(async (tx) => {
        const [still] = await tx`select 1 as x from game.account_jobs
                                  where job_id = ${job.job_id} and status = 'running' for update`;
        if (!still) return; // Cancelled: the player kept their account and exports
        const exports = await ctx.storage!.list("exports", account);
        if (exports.length > 0) await ctx.storage!.remove("exports", exports.map((n) => `${account}/${n}`));
        await tx`update game.account_jobs set status = 'done', error = null, completed_at = now()
                  where job_id = ${job.job_id}`;
        await tx`select private.delete_account(${account})`;
      });
    } catch (e) {
      await jobFailed(ctx.sql, job.job_id, message(e));
    }
  }
}

async function expireReplays(ctx: WorkerContext, scope: RetentionScope): Promise<boolean> {
  const only = scope.matchIds ?? null;
  for (
    const [column, bucket, days] of [["replay_path", "replays", REPLAY_DAYS], [
      "raw_events_path",
      "match-events",
      EVENTS_DAYS,
    ]] as const
  ) {
    for (let batch = 0;; batch++) {
      if (batch >= RETENTION_BATCHES) return false;
      const rows = await ctx.sql<{ match_id: string; path: string }[]>`
        select r.match_id, ${ctx.sql(column)} as path from game.match_reports r
         where ${ctx.sql(column)} is not null and r.received_at < now() - make_interval(days => ${days})
           and r.status <> 'quarantined'
           and not exists (select 1 from game.reports p where p.match_id = r.match_id and p.status = 'open')
           and (${only}::uuid[] is null or r.match_id = any(${only}::uuid[]))
         order by r.received_at limit ${RETENTION_BATCH}`;
      if (rows.length === 0) break;
      await ctx.storage!.remove(bucket, rows.map((r) => r.path.slice(bucket.length + 1)));
      await ctx.sql`update game.match_reports set ${ctx.sql(column)} = null where match_id in ${
        ctx.sql(rows.map((r) => r.match_id))
      }`;
    }
  }
  return true;
}

async function expireExports(ctx: WorkerContext): Promise<boolean> {
  for (let batch = 0; batch < RETENTION_BATCHES; batch++) {
    const old = await ctx.sql<{ job_id: string; result_path: string }[]>`
      select job_id, result_path from game.account_jobs
       where kind = 'export' and status = 'done' and result_path is not null
         and completed_at < now() - make_interval(days => ${EXPORT_DAYS})
       limit ${RETENTION_BATCH}`;
    if (old.length === 0) return true;
    await ctx.storage!.remove("exports", old.map((j) => j.result_path.slice("exports/".length)));
    await ctx.sql`update game.account_jobs set status = 'expired', result_path = null
                   where job_id in ${ctx.sql(old.map((j) => j.job_id))}`;
  }
  return false;
}

export async function runRetention(ctx: WorkerContext, scope: RetentionScope = {}): Promise<void> {
  const replaysDone = await expireReplays(ctx, scope);
  const finished = scope.matchIds ? replaysDone : (await expireExports(ctx)) && replaysDone;
  // A backlog the batch budget did not finish runs again next tick.
  if (finished && !scope.matchIds) {
    await ctx.sql`
      insert into game.config (key, value) values ('retention_checked_at', ${ctx.sql.json(new Date().toISOString())})
      on conflict (key) do update set value = excluded.value`;
  }
}

async function retentionDue(ctx: WorkerContext): Promise<boolean> {
  const [due] = await ctx.sql<{ due: boolean }[]>`
    select coalesce((select (value #>> '{}')::timestamptz from game.config where key = 'retention_checked_at')
                    < now() - make_interval(secs => ${RETENTION_EVERY_S}), true) as due`;
  return due.due;
}

export async function accountsPass(ctx: WorkerContext): Promise<void> {
  if (!ctx.storage) return;
  await runExports(ctx);
  await runDeletions(ctx);
  if (await retentionDue(ctx)) await runRetention(ctx);
}
