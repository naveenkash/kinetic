// The simulation, compiled from web/sim with Emscripten (make wasm), and the shapes of the
// JSON it returns.

export type Vec = [number, number, number];

export interface ArenaSolid { min: Vec; max: Vec; tag: string }
export interface Arena { name: string; killZ: number; solids: ArenaSolid[]; crates: { half: Vec; mass: number }[] }

export interface PlayerState {
  slot: number; team: number; class: string; pos: Vec; yaw: number; pitch: number;
  standing: boolean; grounded: boolean; braced: boolean; airborne: boolean; stunned: boolean;
  drawn: boolean; warm: number; mode: "haul" | "jack" | "hang";
}

export interface LineState {
  id: number; owner: number; team: number; a: Vec; b: Vec; live: boolean;
  mode: "slack" | "haul" | "jack" | "hang" | "set"; force: number;
}

export interface Snapshot {
  t: number; phase: string; clock: number; round: number; wins: [number, number]; you: number; resets: number;
  players: PlayerState[]; crates: Vec[]; lines: LineState[];
  onYou: { toward: Vec; plate: string }[]; feed: string[];
}

export const Buttons = {
  Jump: 1 << 0, Sprint: 1 << 1, Brace: 1 << 2, Fire: 1 << 3,
  SelfDog: 1 << 4, Release: 1 << 5, Draw: 1 << 6, Swing: 1 << 7,
} as const;

interface NativeGame {
  step(forward: number, right: number, yaw: number, pitch: number, buttons: number, mode: number): void;
  snapshot(): string;
  arena(): string;
  delete(): void;
}

interface KineticModule { Game: new (seed: number, playerClass: number, play: boolean) => NativeGame }

export const Classes = ["Climber", "Scrapper", "Rigger", "Doubler", "Breaker"] as const;

export async function loadKinetic(): Promise<KineticModule> {
  const url = new URL("kinetic.js", document.baseURI).href;
  const mod = await import(/* @vite-ignore */ url);
  return (await mod.default()) as KineticModule;
}

export class Game {
  private native: NativeGame;
  readonly arena: Arena;

  constructor(module: KineticModule, seed: number, playerClass: number, play = true) {
    this.native = new module.Game(seed, playerClass, play);
    this.arena = JSON.parse(this.native.arena()) as Arena;
  }

  step(forward: number, right: number, yaw: number, pitch: number, buttons: number, mode: number): void {
    this.native.step(forward, right, yaw, pitch, buttons, mode);
  }

  snapshot(): Snapshot {
    return JSON.parse(this.native.snapshot()) as Snapshot;
  }
}
