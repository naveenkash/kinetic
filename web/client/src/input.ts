// Keyboard and mouse to the simulation's buttons. Held inputs are read each tick; presses
// are latched until the next tick consumes them.
import { Buttons } from "./kinetic";

export class Input {
  yaw = 0;
  pitch = 0;
  private keys = new Set<string>();
  private pressed = 0;
  private mode = 0;
  private rightHeld = false;
  onHelp: () => void = () => {};
  readonly sensitivity = 0.0022;

  constructor(private canvas: HTMLCanvasElement) {
    window.addEventListener("keydown", (e) => {
      if (e.repeat) return;
      this.keys.add(e.code);
      if (e.code === "Space") this.pressed |= Buttons.Jump;
      if (e.code === "KeyE") this.pressed |= Buttons.SelfDog;
      if (e.code === "KeyR") this.pressed |= Buttons.Release;
      if (e.code === "Digit1") this.mode = 1;
      if (e.code === "Digit2") this.mode = 2;
      if (e.code === "Digit3") this.mode = 3;
      if (e.code === "KeyH") this.onHelp();
    });
    window.addEventListener("keyup", (e) => this.keys.delete(e.code));
    window.addEventListener("blur", () => { this.keys.clear(); this.rightHeld = false; });
    canvas.addEventListener("contextmenu", (e) => e.preventDefault());
    canvas.addEventListener("mousedown", (e) => {
      if (document.pointerLockElement !== canvas) {
        canvas.requestPointerLock();
        return;
      }
      if (e.button === 0) this.pressed |= this.rightHeld ? Buttons.Swing : Buttons.Fire;
      if (e.button === 2) this.rightHeld = true;
    });
    window.addEventListener("mouseup", (e) => { if (e.button === 2) this.rightHeld = false; });
    document.addEventListener("mousemove", (e) => {
      if (document.pointerLockElement !== canvas) return;
      this.yaw -= e.movementX * this.sensitivity;
      this.pitch = Math.max(-1.4, Math.min(1.4, this.pitch - e.movementY * this.sensitivity));
    });
  }

  // One tick's input; presses are cleared once read.
  take(): { forward: number; right: number; buttons: number; mode: number } {
    const k = this.keys;
    const forward = (k.has("KeyW") ? 1 : 0) - (k.has("KeyS") ? 1 : 0);
    const right = (k.has("KeyD") ? 1 : 0) - (k.has("KeyA") ? 1 : 0);
    let buttons = this.pressed;
    if (k.has("ShiftLeft") || k.has("ShiftRight")) buttons |= Buttons.Sprint;
    if (k.has("KeyC")) buttons |= Buttons.Brace;
    if (this.rightHeld) buttons |= Buttons.Draw;
    const mode = this.mode;
    this.pressed = 0;
    this.mode = 0;
    return { forward, right, buttons, mode };
  }

  get locked(): boolean {
    return document.pointerLockElement === this.canvas;
  }
}
