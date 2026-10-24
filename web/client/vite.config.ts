import { defineConfig } from "vite";

// The WebAssembly module (make wasm) is written to public/, served as /kinetic.js and
// /kinetic.wasm and copied as-is into the build.
export default defineConfig({
  base: "./",
  server: { port: 5173 },
});
