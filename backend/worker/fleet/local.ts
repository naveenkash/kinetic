// Local fleet: starts one game-server process per match on this machine. With no Unreal
// server build yet, the default command is the fake server in tools/fake-server.ts.
//
// The child gets a clean environment: only what the match needs plus PATH, HOME and the
// Deno cache settings. It never sees the worker's signing key, service key or database URL.
import type { AllocateRequest, Allocation, Fleet } from "./types.ts";

const PASSTHROUGH = ["PATH", "HOME", "TMPDIR", "DENO_DIR", "DENO_INSTALL_ROOT"];

function freePort(): number {
  const listener = Deno.listen({ hostname: "127.0.0.1", port: 0 });
  const port = (listener.addr as Deno.NetAddr).port;
  listener.close();
  return port;
}

export class LocalFleet implements Fleet {
  readonly name = "local";
  private readonly running = new Map<string, Deno.ChildProcess>();

  constructor(private readonly command: string[], private readonly logDir?: string) {
    if (command.length === 0) throw new Error("LocalFleet needs a server command");
  }

  async allocate(req: AllocateRequest): Promise<Allocation> {
    const port = freePort();
    const [cmd, ...args] = this.command;

    // Open the log before spawning, so a failure here leaves no untracked process.
    let log: Deno.FsFile | null = null;
    if (this.logDir) {
      await Deno.mkdir(this.logDir, { recursive: true });
      log = await Deno.open(`${this.logDir}/${req.matchId}.log`, { create: true, append: true });
    }

    const inherited = PASSTHROUGH.flatMap((key) => {
      const value = Deno.env.get(key);
      return value === undefined ? [] : [[key, value] as const];
    });
    const env = { ...Object.fromEntries(inherited), ...req.env, PORT: String(port) };

    let child: Deno.ChildProcess;
    try {
      child = new Deno.Command(cmd, {
        args,
        env,
        clearEnv: true,
        stdin: "null",
        stdout: log ? "piped" : "inherit",
        stderr: log ? "piped" : "inherit",
      }).spawn();
    } catch (e) {
      log?.close();
      throw e;
    }

    if (log) {
      const writer = log.writable.getWriter();
      const pump = async (stream: ReadableStream<Uint8Array>) => {
        for await (const chunk of stream) await writer.write(chunk);
      };
      Promise.allSettled([pump(child.stdout), pump(child.stderr)])
        .then(() => writer.close())
        .catch(() => {});
    }

    const fleetRef = `local-${port}-${child.pid}`;
    this.running.set(fleetRef, child);
    child.status.finally(() => this.running.delete(fleetRef)).catch(() => {});
    return { fleetRef, host: "127.0.0.1", port };
  }

  deallocate(fleetRef: string): Promise<void> {
    const child = this.running.get(fleetRef);
    if (child) {
      try {
        child.kill("SIGTERM");
      } catch {
        // Already exited.
      }
    }
    return Promise.resolve();
  }

  async shutdown(): Promise<void> {
    const children = [...this.running.values()];
    for (const child of children) {
      try {
        child.kill("SIGTERM");
      } catch {
        // Already exited.
      }
    }
    await Promise.allSettled(children.map((c) => c.status));
  }
}
