// Supabase Storage for the worker (exports, retention), with the service key.
import { serviceClient } from "../supabase/functions/_shared/supabase.ts";
import type { ObjectStore } from "./accounts_pass.ts";

const LIST_LIMIT = 1000;

export function supabaseStorage(): ObjectStore {
  const storage = serviceClient().storage;
  return {
    async upload(bucket, path, body, contentType) {
      const { error } = await storage.from(bucket).upload(path, new Blob([body], { type: contentType }), {
        contentType,
        upsert: true,
      });
      if (error) throw new Error(`upload ${bucket}/${path}: ${error.message}`);
    },
    async remove(bucket, paths) {
      const { error } = await storage.from(bucket).remove(paths);
      if (error) throw new Error(`remove from ${bucket}: ${error.message}`);
    },
    async list(bucket, prefix) {
      const { data, error } = await storage.from(bucket).list(prefix, { limit: LIST_LIMIT });
      if (error) throw new Error(`list ${bucket}/${prefix}: ${error.message}`);
      return (data ?? []).map((o) => o.name);
    },
  };
}
