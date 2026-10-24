// three.js view of a snapshot. The simulation is z-up in metres; three.js is y-up, so a
// simulation point (x, y, z) is drawn at (x, z, -y).
import * as THREE from "three";
import type { Arena, LineState, PlayerState, Snapshot, Vec } from "./kinetic";

const TeamColor = [0xe69f00, 0x56b4e9];  // Amber and Sky (05 11a)
const Bone = 0xe8e6e0;
const Sky = 0x1a1410;

const SolidColor: Record<string, number> = {
  floor: 0x5a5550,
  bridge: 0x6a645c,
  trench: 0x0d0b09,
  container: 0x8a5a3c,
  post: 0x7c8186,
  wall: 0x6f6b66,
};

export function toThree(v: Vec): THREE.Vector3 {
  return new THREE.Vector3(v[0], v[2], -v[1]);
}

interface PlayerView {
  group: THREE.Group;
  body: THREE.Mesh;
  cutter: THREE.Mesh;
  ring: THREE.Mesh;
}

export class View {
  readonly renderer: THREE.WebGLRenderer;
  readonly scene = new THREE.Scene();
  readonly camera = new THREE.PerspectiveCamera(70, 1, 0.05, 400);
  private players: PlayerView[] = [];
  private crates: THREE.Mesh[] = [];
  private lines = new Map<number, { mesh: THREE.Mesh; dogs: THREE.Mesh[] }>();
  private readonly unitCylinder = new THREE.CylinderGeometry(1, 1, 1, 6, 1, true);
  private readonly dogGeometry = new THREE.SphereGeometry(0.12, 8, 6);

  constructor(canvas: HTMLCanvasElement, arena: Arena) {
    this.renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    this.scene.background = new THREE.Color(Sky);
    this.scene.fog = new THREE.Fog(Sky, 40, 160);

    // The Yard at night: sodium cloud light from above, a floodlight key, low fill.
    this.scene.add(new THREE.HemisphereLight(0xd88a4a, 0x2a2018, 2.2));
    const key = new THREE.DirectionalLight(0xf4e7c8, 2.6);
    key.position.set(-30, 60, 20);
    this.scene.add(key);
    const rim = new THREE.DirectionalLight(0x9fb8d8, 0.8);
    rim.position.set(40, 30, -30);
    this.scene.add(rim);
    this.scene.add(new THREE.AmbientLight(0xffffff, 0.35));

    for (const s of arena.solids) {
      const size = [s.max[0] - s.min[0], s.max[1] - s.min[1], s.max[2] - s.min[2]];
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(size[0], size[2], size[1]),
        new THREE.MeshLambertMaterial({ color: SolidColor[s.tag] ?? 0x555555 }),
      );
      mesh.position.copy(toThree([(s.min[0] + s.max[0]) / 2, (s.min[1] + s.max[1]) / 2, (s.min[2] + s.max[2]) / 2]));
      this.scene.add(mesh);
      if (s.tag === "floor" || s.tag === "bridge") {
        // Hazard tape along the drop: yellow-and-black at every floor edge.
        const edges = new THREE.LineSegments(
          new THREE.EdgesGeometry(mesh.geometry),
          new THREE.LineBasicMaterial({ color: 0xe8c34a }),
        );
        edges.position.copy(mesh.position);
        this.scene.add(edges);
      }
    }
    for (const c of arena.crates) {
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(c.half[0] * 2, c.half[2] * 2, c.half[1] * 2),
        new THREE.MeshLambertMaterial({ color: c.mass > 100 ? 0x7a5a2a : 0x9c7a42 }),
      );
      this.scene.add(mesh);
      this.crates.push(mesh);
    }
    this.resize();
    window.addEventListener("resize", () => this.resize());
  }

  private resize(): void {
    const w = window.innerWidth, h = window.innerHeight;
    this.renderer.setSize(w, h, false);
    this.camera.aspect = w / h;
    this.camera.updateProjectionMatrix();
  }

  private playerView(index: number, p: PlayerState, you: number): PlayerView {
    while (this.players.length <= index) {
      const team = p.team;
      const group = new THREE.Group();
      const body = new THREE.Mesh(
        new THREE.CapsuleGeometry(0.4, 1.0, 4, 10),
        new THREE.MeshLambertMaterial({ color: 0x2f3236 }),
      );
      group.add(body);
      // Catch Plates in the team tint: chest, back and shoulders.
      const plateMat = new THREE.MeshLambertMaterial({ color: TeamColor[team], emissive: TeamColor[team], emissiveIntensity: 0.35 });
      const plates: [number, number, number, number, number, number][] = [
        [0.36, 0.35, 0, 0.06, 0.4, 0.5], [-0.36, 0.35, 0, 0.06, 0.4, 0.5],
        [0, 0.6, 0.38, 0.3, 0.12, 0.12], [0, 0.6, -0.38, 0.3, 0.12, 0.12],
      ];
      for (const [x, y, z, sx, sy, sz] of plates) {
        const plate = new THREE.Mesh(new THREE.BoxGeometry(sx, sy, sz), plateMat);
        plate.position.set(x, y, z);
        group.add(plate);
      }
      const visor = new THREE.Mesh(new THREE.BoxGeometry(0.12, 0.14, 0.5), new THREE.MeshBasicMaterial({ color: 0xbfe6fb }));
      visor.position.set(0.34, 0.75, 0);
      group.add(visor);
      const cutter = new THREE.Mesh(new THREE.BoxGeometry(0.5, 0.05, 0.05), new THREE.MeshBasicMaterial({ color: 0xff7a2a }));
      cutter.position.set(0.5, 0.1, -0.35);
      group.add(cutter);
      const ring = new THREE.Mesh(new THREE.RingGeometry(0.55, 0.7, 24), new THREE.MeshBasicMaterial({ color: TeamColor[team], side: THREE.DoubleSide }));
      ring.rotation.x = -Math.PI / 2;
      ring.position.y = -0.88;
      ring.visible = false;
      group.add(ring);
      this.scene.add(group);
      this.players.push({ group, body, cutter, ring });
    }
    const v = this.players[index];
    v.ring.visible = index === you;
    return v;
  }

  private lineView(l: LineState): { mesh: THREE.Mesh; dogs: THREE.Mesh[] } {
    let v = this.lines.get(l.id);
    if (!v) {
      const mesh = new THREE.Mesh(this.unitCylinder, new THREE.MeshBasicMaterial({ color: TeamColor[l.team] }));
      const dogs = [0, 1].map(() => new THREE.Mesh(this.dogGeometry, new THREE.MeshBasicMaterial({ color: 0xd9d4c7 })));
      this.scene.add(mesh, ...dogs);
      v = { mesh, dogs };
      this.lines.set(l.id, v);
    }
    return v;
  }

  draw(s: Snapshot, cameraYaw: number, cameraPitch: number): void {
    s.players.forEach((p, i) => {
      const v = this.playerView(i, p, s.you);
      v.group.visible = p.standing;
      v.group.position.copy(toThree(p.pos));
      v.group.rotation.y = p.yaw;
      v.cutter.visible = p.drawn;
      const body = v.body.material as THREE.MeshLambertMaterial;
      body.color.setHex(p.stunned ? 0x6a2a24 : p.braced ? 0x24303a : 0x2f3236);
    });
    s.crates.forEach((c, i) => this.crates[i]?.position.copy(toThree(c)));

    const seen = new Set<number>();
    for (const l of s.lines) {
      seen.add(l.id);
      const v = this.lineView(l);
      const a = toThree(l.a), b = toThree(l.b);
      const mid = a.clone().add(b).multiplyScalar(0.5);
      const len = Math.max(0.001, a.distanceTo(b));
      // Width and brightness carry the force (05 11: emissive, brightness = force).
      const force = Math.min(1, l.force / 1200);
      const radius = l.live ? 0.025 + 0.03 * force : 0.012;
      v.mesh.position.copy(mid);
      v.mesh.scale.set(radius, len, radius);
      v.mesh.quaternion.setFromUnitVectors(new THREE.Vector3(0, 1, 0), b.clone().sub(a).normalize());
      const mat = v.mesh.material as THREE.MeshBasicMaterial;
      const base = new THREE.Color(l.mode === "set" ? Bone : TeamColor[l.team]);
      mat.color.copy(base.multiplyScalar(l.mode === "set" ? 0.45 : 0.55 + 0.45 * force));
      v.dogs[0].position.copy(a);
      v.dogs[1].position.copy(b);
    }
    for (const [id, v] of this.lines) {
      if (!seen.has(id)) {
        this.scene.remove(v.mesh, ...v.dogs);
        (v.mesh.material as THREE.Material).dispose();
        v.dogs.forEach((d) => (d.material as THREE.Material).dispose());
        this.lines.delete(id);
      }
    }

    // Third person, over the right shoulder.
    const me = s.you >= 0 ? s.players[s.you] : undefined;
    const eye = me ? toThree(me.pos).add(new THREE.Vector3(0, 0.7, 0)) : new THREE.Vector3(0, 30, 0);
    const forward = toThree([Math.cos(cameraPitch) * Math.cos(cameraYaw), Math.cos(cameraPitch) * Math.sin(cameraYaw), Math.sin(cameraPitch)]);
    const right = toThree([Math.sin(cameraYaw), -Math.cos(cameraYaw), 0]);
    if (me && me.standing) {
      this.camera.position.copy(eye).addScaledVector(forward, -4.2).addScaledVector(right, 0.7).add(new THREE.Vector3(0, 0.35, 0));
      this.camera.lookAt(eye.clone().addScaledVector(forward, 10));
    } else {
      // Out of the round, or spectating: a high view of the Yard.
      this.camera.position.set(0, 42, 38);
      this.camera.lookAt(0, 0, 0);
    }
    this.renderer.render(this.scene, this.camera);
  }
}
