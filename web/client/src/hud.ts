// The HUD over the view: clock and score, phase, mode chips, the Cutter's warm-up, the
// line-on-body edge indicator (ui-hud 4.6) and the Marshal's feed.
import type { Snapshot } from "./kinetic";

const $ = (id: string) => document.getElementById(id) as HTMLElement;

const PhaseText: Record<string, string> = {
  reset: "Reset", drop: "Drop", live: "", sudden_death: "Sudden death", round_end: "Round over", match_over: "Match over",
};

export class Hud {
  private lastFeed = "";

  constructor() {
    for (const team of [0, 1]) {
      const pips = $(`pips-${team}`);
      for (let i = 0; i < 3; i++) {
        const pip = document.createElement("div");
        pip.className = "pip";
        pips.appendChild(pip);
      }
    }
  }

  draw(s: Snapshot, cameraYaw: number): void {
    const clock = Math.max(0, Math.ceil(s.clock));
    $("clock").textContent = `${Math.floor(clock / 60)}:${String(clock % 60).padStart(2, "0")}`;
    for (const team of [0, 1]) {
      $(`pips-${team}`).querySelectorAll(".pip").forEach((pip, i) => pip.classList.toggle("won", i < s.wins[team]));
    }
    const me = s.you >= 0 ? s.players[s.you] : undefined;
    $("phase").textContent = me && !me.standing && s.phase === "live" ? "Scrapped. Watching." : PhaseText[s.phase] ?? "";

    if (me) {
      document.querySelectorAll<HTMLElement>("#modes span").forEach((el) => el.classList.toggle("on", el.dataset.mode === me.mode));
      $("cutter-fill").style.width = `${Math.round(me.warm * 100)}%`;
      $("crosshair").classList.toggle("warm", me.drawn && me.warm >= 1);
      $("state").textContent = me.stunned ? "Staggered" : me.braced ? "Braced" : me.airborne ? "Off your feet" : me.class;
    }

    // Edge indicator: an arrow at the screen's edge toward each line on your body.
    const edges = $("edges");
    edges.replaceChildren();
    if (me && me.standing) {
      const w = window.innerWidth, h = window.innerHeight;
      for (const line of s.onYou) {
        const dx = line.toward[0] - me.pos[0], dy = line.toward[1] - me.pos[1];
        // Angle of the line relative to where the camera looks: 0 ahead, positive to the left.
        let rel = Math.atan2(dy, dx) - cameraYaw;
        rel = Math.atan2(Math.sin(rel), Math.cos(rel));
        const ahead = Math.abs(rel) < 0.6;
        const r = Math.min(w, h) * 0.42;
        const x = w / 2 - Math.sin(rel) * r, y = h / 2 - Math.cos(rel) * r;
        const arrow = document.createElement("div");
        arrow.className = "edge";
        arrow.style.left = `${x - 14}px`;
        arrow.style.top = `${y - 13}px`;
        arrow.style.transform = `rotate(${-rel}rad)`;
        arrow.style.opacity = ahead ? "0.55" : "1";
        const label = document.createElement("div");
        label.className = "edge-label";
        label.style.left = `${x}px`;
        label.style.top = `${y + 18}px`;
        label.textContent = `Line on you: ${line.plate.replace("_", " ")}`;
        edges.append(arrow, label);
      }
    }

    const feed = s.feed.join("\n");
    if (feed !== this.lastFeed) {
      this.lastFeed = feed;
      $("feed").replaceChildren(...s.feed.map((t) => Object.assign(document.createElement("div"), { textContent: t })));
    }
  }
}
