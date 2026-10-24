// The KINETIC browser prototype: pick a harness, then play the Yard against bots. The
// simulation runs at a fixed 60 Hz; the view draws the latest snapshot every frame.
import { Classes, Game, loadKinetic } from "./kinetic";
import { Hud } from "./hud";
import { Input } from "./input";
import { View } from "./render";

const Blurbs: Record<string, string> = {
  Climber: "90 kg, 50 m line, fastest Dogs. Light: easy to Haul.",
  Scrapper: "110 kg, 20 m line, instant recall, quick Cutter.",
  Rigger: "130 kg, 40 m line, two set lines, can brace.",
  Doubler: "130 kg, 30 m line. (Overspin and the second line are not in the prototype.)",
  Breaker: "220 kg, 20 m line, 1,000 N Haul, braces hard.",
};

const Step = 1 / 60;

async function main(): Promise<void> {
  const status = document.getElementById("status") as HTMLElement;
  let module;
  try {
    module = await loadKinetic();
  } catch (e) {
    status.textContent = "Could not load kinetic.wasm. Run `make wasm` in web/ first.";
    throw e;
  }
  // ?class=Rigger starts at once; ?demo watches a bot in your slot.
  const params = new URLSearchParams(location.search);
  const demo = params.has("demo");
  const chosen = Classes.findIndex((c) => c.toLowerCase() === (params.get("class") ?? "").toLowerCase());
  if (demo || chosen >= 0) {
    start(module, chosen >= 0 ? chosen : 2, !demo, Number(params.get("seed") ?? Math.floor(Math.random() * 1e9)));
    return;
  }
  status.textContent = "Pick a harness. Click the view to capture the mouse; Esc to release it.";
  const picker = document.getElementById("classes") as HTMLElement;
  Classes.forEach((name, index) => {
    const button = document.createElement("button");
    button.innerHTML = `<b>${name}</b><small>${Blurbs[name]}</small>`;
    button.addEventListener("click", () => start(module, index, true, Math.floor(Math.random() * 1e9)));
    picker.appendChild(button);
  });
}

function start(module: Awaited<ReturnType<typeof loadKinetic>>, playerClass: number, play: boolean, seed: number): void {
  (document.getElementById("start") as HTMLElement).classList.add("hidden");
  (document.getElementById("hud") as HTMLElement).classList.remove("hidden");
  const canvas = document.getElementById("view") as HTMLCanvasElement;
  const game = new Game(module, seed, playerClass, play);
  const view = new View(canvas, game.arena);
  const hud = new Hud();
  const input = new Input(canvas);
  const help = document.getElementById("help") as HTMLElement;
  input.onHelp = () => help.classList.toggle("hidden");

  let snapshot = game.snapshot();
  const me = snapshot.players[Math.max(0, snapshot.you)];
  input.yaw = me ? me.yaw : 0;
  let last = performance.now();
  let accumulator = 0;
  let resets = snapshot.resets;

  const frame = (now: number) => {
    accumulator = Math.min(accumulator + (now - last) / 1000, 0.25);
    last = now;
    while (accumulator >= Step) {
      const t = input.take();
      game.step(t.forward, t.right, input.yaw, input.pitch, t.buttons, t.mode);
      accumulator -= Step;
    }
    snapshot = game.snapshot();
    if (!play) {
      // The demo: follow slot 0's bot with its own camera.
      const bot = snapshot.players[0];
      input.yaw = bot.yaw;
      input.pitch = bot.pitch;
      snapshot.you = 0;
    }
    const mine = snapshot.players[snapshot.you];
    // A reset (a new round) turns you to face the other side: adopt the spawn facing, which
    // the simulation holds for a moment for this.
    if (mine && snapshot.resets !== resets) {
      resets = snapshot.resets;
      input.yaw = mine.yaw;
      input.pitch = 0;
    }
    view.draw(snapshot, input.yaw, input.pitch);
    hud.draw(snapshot, input.yaw);
    requestAnimationFrame(frame);
  };
  requestAnimationFrame(frame);
}

main();
