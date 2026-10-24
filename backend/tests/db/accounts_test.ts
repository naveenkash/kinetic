// Export, deletion and retention (backend-architecture 19) against the local database, with an
// in-memory object store standing in for Supabase Storage.
import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import { accountsPass, type ObjectStore, runRetention } from "../../worker/accounts_pass.ts";
import type { WorkerContext } from "../../worker/loop.ts";

const DB_URL = Deno.env.get("DATABASE_URL") ?? "postgresql://postgres:postgres@127.0.0.1:55422/postgres";
const tag = `${Date.now() % 1_000_000}`;

class MemoryStore implements ObjectStore {
  objects = new Map<string, string>();
  // Runs once, on the next list call (a test's way in while a job is running); then the call
  // fails if failNext is set.
  onList: (() => Promise<void>) | null = null;
  failNext = false;
  upload(bucket: string, path: string, body: string) {
    this.objects.set(`${bucket}/${path}`, body);
    return Promise.resolve();
  }
  remove(bucket: string, paths: string[]) {
    for (const p of paths) this.objects.delete(`${bucket}/${p}`);
    return Promise.resolve();
  }
  async list(bucket: string, prefix: string) {
    const hook = this.onList;
    this.onList = null;
    if (hook) await hook();
    if (this.failNext) {
      this.failNext = false;
      throw new Error("storage blip");
    }
    const start = `${bucket}/${prefix}/`;
    return [...this.objects.keys()].filter((k) => k.startsWith(start)).map((k) => k.slice(start.length));
  }
}

async function makeAccount(sql: postgres.Sql, name: string): Promise<string> {
  const id = crypto.randomUUID();
  await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
            values (${id}, ${name + "@users.kinetic.invalid"}, 'authenticated', 'authenticated',
                    ${sql.json({ kinetic_provider: "dev", kinetic_provider_user_id: name })})`;
  await sql`select api.srv_ensure_account(${id}, 'dev', ${name}, ${name})`;
  return id;
}

Deno.test("export, deletion and retention", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const store = new MemoryStore();
  const ctx = { sql, functionsUrl: "", log: () => {}, storage: store } as unknown as WorkerContext;
  const accounts: string[] = [];
  const matches: string[] = [];
  // The full pass would also run retention over the shared database: mark it as just run.
  const [saved] = await sql`select value from game.config where key = 'retention_checked_at'`;
  await sql`insert into game.config (key, value) values ('retention_checked_at', ${sql.json(new Date().toISOString())})
            on conflict (key) do update set value = excluded.value`;
  try {
    const a = await makeAccount(sql, `ac_a_${tag}`);
    const b = await makeAccount(sql, `ac_b_${tag}`);
    accounts.push(a, b);
    const match = crypto.randomUUID();
    matches.push(match);
    await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
              values (${match}, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed')`;
    await sql`insert into game.match_players (match_id, slot, team, account_id, harness) values
              (${match}, 1, 1, ${a}, 'Rigger'), (${match}, 2, 2, ${b}, 'Breaker')`;
    await sql`update game.ledgers set tonnage_total = 1234 where account_id = ${a}`;

    await t.step("an export writes the player's archive to their own folder", async () => {
      const [job] =
        await sql`insert into game.account_jobs (account_id, kind) values (${a}, 'export') returning job_id`;
      await accountsPass(ctx);
      const [done] = await sql`select status, result_path from game.account_jobs where job_id = ${job.job_id}`;
      assertEquals([done.status, done.result_path], ["done", `exports/${a}/${job.job_id}.json`]);
      const archive = JSON.parse(store.objects.get(`exports/${a}/${job.job_id}.json`)!);
      assertEquals(archive.ledger.tonnage_total, 1234);
      assertEquals(archive.matches.length, 1);
      assertEquals(archive.identities[0].provider, "dev");
    });

    await t.step("a deletion waits out its grace, then removes the player and anonymizes the match", async () => {
      await sql`insert into game.match_reports (match_id, payload, status)
                values (${match}, ${sql.json({ players: [{ account_id: a, team: 1 }, { account_id: b, team: 2 }] })},
                        'processed')`;
      await sql`insert into auth.audit_log_entries (id, payload, created_at)
                values (gen_random_uuid(), ${sql.json({ action: "login", actor_id: a })}, now())`;
      await sql`insert into game.sanctions (account_id, type, reason) values (${a}, 'matchmaking_ban', 'griefing')`;
      const [job] = await sql`insert into game.account_jobs (account_id, kind, run_after)
                              values (${a}, 'deletion', now() + interval '14 days') returning job_id`;
      await accountsPass(ctx);
      assertEquals((await sql`select 1 from game.accounts where account_id = ${a}`).length, 1, "still in its grace");
      await sql`update game.account_jobs set run_after = now() - interval '1 second' where job_id = ${job.job_id}`;
      await accountsPass(ctx);
      assertEquals((await sql`select 1 from game.accounts where account_id = ${a}`).length, 0);
      assertEquals((await sql`select 1 from auth.users where id = ${a}`).length, 0);
      const roster = await sql`select account_id from game.match_players where match_id = ${match} order by slot`;
      assertEquals(roster.map((r) => r.account_id), [null, b], "the other player's history stays whole");
      assertEquals([...store.objects.keys()].filter((k) => k.startsWith(`exports/${a}`)), []);
      const [done] = await sql`select status, account_id from game.account_jobs where job_id = ${job.job_id}`;
      assertEquals([done.status, done.account_id], ["done", null]);
      const [payload] = await sql`select payload from game.match_reports where match_id = ${match}`;
      assertEquals(payload.payload.players.map((p: { account_id: string | null }) => p.account_id), [null, b]);
      const audit = await sql`select 1 from auth.audit_log_entries where payload ->> 'actor_id' = ${a}`;
      assertEquals(audit.length, 0, "the Auth trail is gone");
    });

    await t.step("a ban survives deletion: the same identity signing up again gets it back", async () => {
      const again = crypto.randomUUID();
      accounts.push(again);
      await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
                values (${again}, ${`ac_a_${tag}@users.kinetic.invalid`}, 'authenticated', 'authenticated',
                        ${sql.json({ kinetic_provider: "dev", kinetic_provider_user_id: `ac_a_${tag}` })})`;
      await sql`select api.srv_ensure_account(${again}, 'dev', ${`ac_a_${tag}`}, ${`ac_a2_${tag}`})`;
      const bans = await sql`select type, reason from game.sanctions where account_id = ${again}`;
      assertEquals(bans.map((s) => s.type), ["matchmaking_ban"]);
      assert(bans[0].reason.startsWith("retained after deletion"));
    });

    await t.step("a job left running is taken again; a deletion cancelled meanwhile does not happen", async () => {
      const [job] = await sql`insert into game.account_jobs (account_id, kind, status, started_at, attempts)
                              values (${b}, 'export', 'running', now() - interval '31 minutes', 1) returning job_id`;
      await accountsPass(ctx);
      const [redone] = await sql`select status, attempts from game.account_jobs where job_id = ${job.job_id}`;
      assertEquals([redone.status, redone.attempts], ["done", 2]);
      // The player cancels while the worker is mid-deletion (sent from another connection, it
      // waits on the job's row lock) and the storage call then fails: the cancel stands, the
      // account and its export are kept, and nothing is retried.
      store.objects.set(`exports/${b}/keep.json`, "{}");
      const [del] =
        await sql`insert into game.account_jobs (account_id, kind) values (${b}, 'deletion') returning job_id`;
      const other = postgres(DB_URL, { max: 2, onnotice: () => {} });
      let cancel: Promise<unknown> = Promise.resolve();
      store.failNext = true;
      store.onList = async () => {
        // .execute() sends it now; postgres.js queries are otherwise lazy until awaited.
        cancel = other`update game.account_jobs set status = 'cancelled', completed_at = now()
                       where job_id = ${del.job_id} and status in ('queued', 'running')`.execute();
        await new Promise((r) => setTimeout(r, 200));
      };
      await accountsPass(ctx);
      await cancel;
      const [after] = await sql`select status from game.account_jobs where job_id = ${del.job_id}`;
      assertEquals(after.status, "cancelled");
      assertEquals((await sql`select 1 from game.accounts where account_id = ${b}`).length, 1);
      assert(store.objects.has(`exports/${b}/keep.json`), "the cancelled player keeps their export");
      // A cancel that arrives while a deletion succeeds waits for it, then finds nothing to cancel.
      const c = await makeAccount(sql, `ac_c_${tag}`);
      accounts.push(c);
      const [job2] =
        await sql`insert into game.account_jobs (account_id, kind) values (${c}, 'deletion') returning job_id`;
      let late: Promise<unknown[]> = Promise.resolve([]);
      store.onList = async () => {
        late = other`update game.account_jobs set status = 'cancelled' where job_id = ${job2.job_id}
                     and status in ('queued', 'running') returning job_id`.execute();
        await new Promise((r) => setTimeout(r, 200));
      };
      await accountsPass(ctx);
      assertEquals((await late).length, 0, "too late to cancel");
      assertEquals((await sql`select 1 from game.accounts where account_id = ${c}`).length, 0);
      await other.end();
      await sql`delete from game.account_jobs where job_id = ${del.job_id}`;
    });

    await t.step("old replays go, unless a report still needs them", async () => {
      const kept = crypto.randomUUID();
      matches.push(kept);
      await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
                values (${kept}, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed')`;
      await sql`insert into game.match_players (match_id, slot, team, account_id, harness)
                values (${kept}, 1, 1, ${b}, 'Rigger')`;
      await sql`insert into game.match_reports (match_id, payload, status, received_at, replay_path, raw_events_path)
                values (${kept}, '{}', 'processed', now() - interval '40 days', ${"replays/" + kept + ".replay"},
                        ${"match-events/" + kept + ".jsonl.gz"})`;
      for (const m of [match, kept]) store.objects.set(`replays/${m}.replay`, "r");
      await sql`insert into game.reports (reporter_id, reported_id, match_id, reason) values (${b}, ${b}, ${kept}, 'other')`;
      await sql`update game.match_reports set received_at = now() - interval '40 days', replay_path = ${
        "replays/" + match + ".replay"
      }, raw_events_path = ${"match-events/" + match + ".jsonl.gz"} where match_id = ${match}`;
      // Only this test's matches: retention must not touch other rows on a shared database.
      await runRetention(ctx, { matchIds: [match, kept] });
      assert(!store.objects.has(`replays/${match}.replay`));
      assert(store.objects.has(`replays/${kept}.replay`), "kept for the open report");
      const [r] = await sql`select replay_path, raw_events_path from game.match_reports where match_id = ${match}`;
      assertEquals([r.replay_path, r.raw_events_path !== null], [null, true], "events are kept for 90 days");
    });
  } finally {
    await sql`delete from game.reports where match_id in ${sql(matches)}`;
    await sql`delete from game.match_reports where match_id in ${sql(matches)}`;
    await sql`delete from game.matches where match_id in ${sql(matches)}`;
    await sql`delete from game.account_jobs where account_id in ${
      sql(accounts)
    } or account_id is null and kind = 'deletion'
              and completed_at > now() - interval '1 minute'`;
    await sql`delete from auth.users where id in ${sql(accounts)}`;
    await sql`delete from game.sanctions where account_id in ${sql(accounts)}`;
    await sql`delete from game.retained_sanctions where reason = 'griefing'`;
    if (saved) {
      await sql`update game.config set value = ${sql.json(saved.value)} where key = 'retention_checked_at'`;
    } else {
      await sql`delete from game.config where key = 'retention_checked_at'`;
    }
    await sql.end();
  }
});
