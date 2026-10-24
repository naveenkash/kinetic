// Per match, inside the rewards transaction (backend-architecture 12.3 step 5): the match's
// Tonnage counts toward the Claim's Manifest, and private.pay_manifest pays every crate the
// player has reached and not been paid (Free always, Sponsored with `sponsorship:<claim_id>`).
// The same function pays passed Sponsored crates when the sponsorship is granted.
import type postgres from "npm:postgres@3.4.5";

type Tx = postgres.TransactionSql;

export interface ManifestPaid {
  tonnage_t: number;
  free: number[];
  sponsored: number[];
}

export async function payManifest(tx: Tx, claimId: string, account: string, tonnage: number): Promise<ManifestPaid> {
  await tx`
    insert into game.manifest_progress (account_id, claim_id, tonnage_t) values (${account}, ${claimId}, ${tonnage})
    on conflict (account_id, claim_id) do update set tonnage_t = manifest_progress.tonnage_t + excluded.tonnage_t`;
  const [row] = await tx<{ paid: ManifestPaid }[]>`select private.pay_manifest(${account}, ${claimId}) as paid`;
  return { ...row.paid, tonnage_t: Number(row.paid.tonnage_t) };
}
