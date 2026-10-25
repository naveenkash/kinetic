"""The hard parts of a harness character (art/characters/harness.py builds and rigs them; the
suit, head, hands and boots are modelled in harness_body.py).

Every part is made in the rest pose, in the skeleton's frame (metres; X forward, Y the
character's left, Z up), and either follows one bone rigidly (plates, the Spool, buckles, the
helmet) or is skinned: it takes its weights from the surface under it (straps, belt, leg loops,
wraps, laces), so it moves with the cloth or the boot it lies on.

- The harness: Catch Plates at their class sizes with lips or chamfers, rolled edges, bolts,
  stamped numbers and the team-tint rim band; webbing straps that follow the body (the chest
  cross, the shoulder straps, the waist belt with its buckle, the leg loops), their paths
  smoothed and their edges rolled, each carrying its own length and width for the weave and
  stitching (harness_look.webbing); the spine frame with its gel reservoirs and sight glasses;
  the telemetry pack and its seal; the lease plate.
- The Spool on the right forearm: housing, drum slot, drum face with the 12-segment mode ring,
  two Dog cradles with Dogs in them, the ceramic line exit and its tape bands, the brake, the
  cradle straps and buckle, the inspection seal; and each class's variant (the Climber's cuff,
  the Scrapper's capacitor can, the Doubler's twin drums and snap guard, the Breaker's gauntlet).
- The Cutter in its sheath; the boots' hardware (eyelets, criss-cross laces and bow, the
  Climber's hook-and-loop straps, the Breaker's shin shell, heel plate and shaft straps); the
  class headgear (shell, crown ridge, brim or visor, rivets, or the Climber's wrap); and each
  class's extras (the Scrapper's pouches, chain and wrench; the Doubler's neon tape; the
  Climber's wraps).
"""
from __future__ import annotations

import math

import bmesh
import bpy
from mathutils import Matrix, Vector

import kinetic_art as ka

TEAM = "TeamTint"


class Parts:
    """Collects parts; each records which bone it follows, or (skinned) takes its weights from the
    body under it once the body is weighted."""

    def __init__(self, mats: dict):
        self.mats = mats
        self.rigid: list = []    # Every part, in order
        self.skinned: list = []  # The soft ones (straps, wraps, laces): weights from the surface under them
        self.body = None         # The suit, once made and weighted in the rest pose: straps are sized to it
        self._co = None
        self._torso = None
        self.bone_frame = None   # (head, tail) of a bone by name, when the gear goes on another skeleton
        self.bone_map = {}       # This skeleton's bone names for the ones the gear is written for

    def limb_point(self, bone: str, z: float, default):
        """Where a limb's bone passes height z (x, y): a strap round the thigh is centred on it."""
        if self.bone_frame is None:
            return default
        head, tail = self.bone_frame(bone)
        t = (head.z - z) / max(head.z - tail.z, 1e-6)
        q = head + (tail - head) * t
        return float(q.x), float(q.y)

    def ring(self, origin, axis, pad: float = 0.004, half: float = 0.012, reach: float = 0.12, n: int = 24,
             torso: bool = False, side: float = 0.0):
        """A closed loop round the body's cross-section through `origin` across `axis` (a wrap
        round a forearm at any angle, a belt, a leg loop), `pad` off the surface: n points. With
        `torso` only the torso counts (not the arms beside it); with `side` only that side of the
        body (+1 left, -1 right: a thigh, not its neighbour)."""
        import numpy as np
        o, a = np.asarray(origin, float), np.asarray(axis, float)
        a = a / np.linalg.norm(a)
        u = np.cross(a, (0.0, 0.0, 1.0) if abs(a[2]) < 0.9 else (1.0, 0.0, 0.0))
        u /= np.linalg.norm(u)
        v = np.cross(a, u)
        rel = self._co - o
        keep = (np.abs(rel @ a) < half) & (np.linalg.norm(rel - np.outer(rel @ a, a), axis=1) < reach)
        if torso:
            keep &= self._torso
        if side:
            keep &= self._co[:, 1] * side > 0.004
        pts = rel[keep]
        ang = np.arctan2(pts @ v, pts @ u)
        rad = np.hypot(pts @ u, pts @ v)
        bins = np.full(n, np.nan)
        idx = ((ang + np.pi) / (2 * np.pi) * n).astype(int) % n
        for i in range(n):
            if np.any(idx == i):
                bins[i] = rad[idx == i].max()
        if np.all(np.isnan(bins)):
            bins[:] = 0.05
        good = ~np.isnan(bins)
        k = np.arange(n)
        bins = np.interp(k, k[good], bins[good], period=n)
        bins = np.maximum(bins, np.roll(bins, 1) * 0.5 + np.roll(bins, -1) * 0.5)  # No dips between samples
        theta = (k + 0.5) / n * 2 * np.pi - np.pi
        return [Vector(o + (u * np.cos(t) + v * np.sin(t)) * (r + pad)) for t, r in zip(theta, bins)]

    def torso_x(self, z: float) -> float:
        """The middle of the torso front to back at height z."""
        import numpy as np
        co = self._co
        keep = (np.abs(co[:, 2] - z) < 0.02) & self._torso & (np.abs(co[:, 1]) < 0.12)
        pts = co[keep]
        return float((pts[:, 0].max() + pts[:, 0].min()) / 2) if len(pts) else 0.0

    def set_body(self, body) -> None:
        import numpy as np
        self.body = body
        co = np.empty(len(body.data.vertices) * 3)
        body.data.vertices.foreach_get("co", co)
        self._co = co.reshape(-1, 3)
        arm = {g.index for g in body.vertex_groups if g.name.split("_")[0] in ("upperarm", "lowerarm", "hand")}
        torso = np.ones(len(self._co), bool)
        for v in body.data.vertices:
            if sum(g.weight for g in v.groups if g.group in arm) > 0.3:
                torso[v.index] = False
        self._torso = torso

    def fit(self, cx: float, cy: float, z: float, pad: float = 0.006, reach: float = 0.2):
        """The body's half-widths (front-back, side-side) in a 4 cm slice at z around (cx, cy),
        plus pad: a strap that goes round it sits on it. A slice of the torso leaves the arms out."""
        import numpy as np
        co = self._co
        keep = (np.abs(co[:, 2] - z) < 0.02) & (np.hypot(co[:, 0] - cx, co[:, 1] - cy) < reach)
        if abs(cy) < 1e-6:
            keep &= self._torso
        else:
            keep &= co[:, 1] * cy > 0  # A limb's slice stays on its own side
        pts = co[keep]
        if not len(pts):
            return 0.08, 0.08
        return float(np.abs(pts[:, 0] - cx).max() + pad), float(np.abs(pts[:, 1] - cy).max() + pad)

    def add(self, obj, bone: str | None, slot: str | None = None, skinned: bool = False):
        import harness_look as hl
        bone = self.bone_map.get(bone, bone)
        for m in list(obj.modifiers):  # Solidified and bevelled shapes, as they will be joined
            bpy.context.view_layer.objects.active = obj
            bpy.ops.object.modifier_apply(modifier=m.name)
        hl.face_out(obj)
        if slot is not None:
            obj.data.materials.clear()
            obj.data.materials.append(self.mats[slot])
        if skinned or bone is None:
            self.skinned.append(obj)
        else:
            group = obj.vertex_groups.new(name=bone)
            group.add(list(range(len(obj.data.vertices))), 1.0, "REPLACE")
        self.rigid.append(obj)
        return obj


# Shapes ----------------------------------------------------------------------------------------

def frame(origin, x_axis, up=(0, 0, 1)) -> Matrix:
    """A placement: local X along x_axis, Z as near `up` as it can be."""
    x = Vector(x_axis).normalized()
    upv = Vector(up)
    if abs(x.dot(upv.normalized())) > 0.95:
        upv = Vector((0, 1, 0)) if abs(x.y) < 0.9 else Vector((1, 0, 0))
    y = upv.cross(x).normalized()
    z = x.cross(y)
    m = Matrix((x, y, z)).transposed().to_4x4()
    m.translation = Vector(origin)
    return m


def place(obj, m: Matrix):
    obj.data.transform(m)
    return obj


def bevelled_box(name: str, size, width: float = 0.004, segments: int = 2):
    obj = ka.box(name, size, (0, 0, 0))
    ka.add_bevel(obj, width=min(width, min(size) * 0.45), segments=segments)
    apply_modifiers(obj)
    return obj


def apply_modifiers(obj):
    bpy.context.view_layer.objects.active = obj
    for m in list(obj.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)


def cylinder(name: str, radius: float, depth: float, segments: int = 16, bevel: float = 0.0):
    """Along local X."""
    obj = ka.cylinder(name, radius, depth, (0, 0, 0), axis="X", segments=segments)
    if bevel:
        ka.add_bevel(obj, width=min(bevel, radius * 0.4, depth * 0.4), segments=1)
        apply_modifiers(obj)
    for p in obj.data.polygons:
        p.use_smooth = True
    return obj


def hex_bolt(name: str, radius: float = 0.006):
    head = cylinder(name, radius, radius * 0.9, segments=6)
    washer = cylinder(name + "_washer", radius * 1.35, radius * 0.3, segments=12)
    washer.data.transform(Matrix.Translation((-radius * 0.55, 0, 0)))
    return ka.join(name, [head, washer])


def strap_path(name: str, points, width: float, thickness: float, outward=None, closed: bool = False, smooth: int = 2):
    """A flat strap along points: its face turned to `outward(p)` (away from the body). The path is
    smoothed (Chaikin, `smooth` rounds) so a strap bends round the body instead of kinking, and
    every vertex carries `strap` (along, across, width in metres) and `strap_length` for the
    webbing's weave and stitching."""
    pts = [Vector(p) for p in points]
    for _ in range(smooth):
        out = [] if closed else [pts[0]]
        n = len(pts)
        for i in range(n if closed else n - 1):
            a, b = pts[i], pts[(i + 1) % n]
            out += [a.lerp(b, 0.25), a.lerp(b, 0.75)]
        pts = out if closed else out + [pts[-1]]
    bm = bmesh.new()
    rings = []
    n = len(pts)
    along = [0.0]
    for i in range(1, n + (1 if closed else 0)):
        along.append(along[-1] + (pts[i % n] - pts[i - 1]).length)
    total = along[-1]
    attrs = []
    # The cross-section: flat faces, the edges rolled (as woven webbing's are).
    w2, t2 = width / 2, thickness / 2
    c = min(thickness * 0.45, 0.0015)
    section = [(-w2 + c, -t2), (w2 - c, -t2), (w2, -t2 + c), (w2, t2 - c), (w2 - c, t2), (-w2 + c, t2), (-w2, t2 - c), (-w2, -t2 + c)]
    for i, p in enumerate(pts):
        a = pts[(i - 1) % n] if (closed or i > 0) else p
        b = pts[(i + 1) % n] if (closed or i < n - 1) else p
        t = (b - a).normalized()
        o = (outward(p) if outward else Vector((1, 0, 0))).normalized()
        side = t.cross(o).normalized()
        o = side.cross(t).normalized()
        ring = []
        for sx, so in section:
            ring.append(bm.verts.new(p + side * sx + o * so))
            attrs.append((along[i], sx, width))
        rings.append(ring)
    k_n = len(section)
    for i in range(n if closed else n - 1):
        r0, r1 = rings[i], rings[(i + 1) % n]
        for k in range(k_n):
            bm.faces.new((r0[k], r0[(k + 1) % k_n], r1[(k + 1) % k_n], r1[k]))
    if not closed:
        bm.faces.new(list(reversed(rings[0])))
        bm.faces.new(rings[-1])
    obj = ka.mesh_object(name, bm)
    strap_attributes(obj, attrs, total)
    for poly in obj.data.polygons:
        poly.use_smooth = True
    return obj


def strap_attributes(obj, attrs, total: float) -> None:
    import numpy as np
    me = obj.data
    a = me.attributes.new("strap", "FLOAT_VECTOR", "POINT")
    a.data.foreach_set("vector", np.asarray(attrs, np.float32).ravel())
    t = me.attributes.new("strap_length", "FLOAT", "POINT")
    t.data.foreach_set("value", np.full(len(me.vertices), total, np.float32))


def ellipse(cx, cy, cz, rx, ry, segments=28, tilt=0.0):
    return [(cx + rx * math.cos(2 * math.pi * i / segments), cy + ry * math.sin(2 * math.pi * i / segments),
             cz + tilt * math.cos(2 * math.pi * i / segments)) for i in range(segments)]


def stamp(name: str, text: str, size: float):
    """Raised stamped characters (a Ledger number, a lease tag), facing local +X."""
    curve = bpy.data.curves.new(name, "FONT")
    curve.body = text
    curve.size = size
    curve.extrude = 0.0006
    curve.align_x = "CENTER"
    curve.align_y = "CENTER"
    obj = bpy.data.objects.new(name, curve)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.convert(target="MESH")
    obj = bpy.context.view_layer.objects.active
    # Text lies in XY facing +Z; turn it to face +X.
    obj.data.transform(Matrix.Rotation(math.pi / 2, 4, "Y") @ Matrix.Rotation(math.pi / 2, 4, "Z"))
    return obj


# Hard parts -------------------------------------------------------------------------------------

def catch_plate(parts: Parts, bone: str, size, centre, normal, style: str, slot: str = "Plate", number: str | None = None,
                rake: float = 0.0, outer_y: float | None = None, bolts: int = 4):
    """A Catch Plate at its class size: lipped (Guild pattern), chamfered (HTR), convex mirror
    (Ascendancy) or salvage; bolts, a stamped number, and the 1 cm team-tint rim band."""
    w, h = size
    m = frame((0, 0, 0), normal, up=(0, 0, 1) if abs(Vector(normal).z) < 0.9 else (1, 0, 0))
    if rake:
        m = m @ Matrix.Rotation(math.radians(rake), 4, "X")
    if outer_y is not None:
        corners = [(m.to_3x3() @ Vector((x, y, z))).y for x in (-0.02, 0.02) for y in (-w / 2 - 0.01, w / 2 + 0.01)
                   for z in (-h / 2 - 0.01, h / 2 + 0.01)]
        reach = max(corners) if outer_y > 0 else min(corners)
        centre = (centre[0], outer_y - reach, centre[2])
    m.translation = Vector(centre)
    pieces = []
    if style == "mirror":
        bm = bmesh.new()
        bmesh.ops.create_grid(bm, x_segments=10, y_segments=10, size=0.5)
        for vert in bm.verts:
            gx, gy = vert.co.x, vert.co.y
            r = min(1.0, math.hypot(gx * 2, gy * 2))
            vert.co = Vector((0.012 * (1 - r * r), gx * w, gy * h))  # Convex, bulging outward
        bmesh.ops.solidify(bm, geom=bm.faces[:], thickness=0.006)
        plate = ka.mesh_object("Plate", bm)
        for p in plate.data.polygons:
            p.use_smooth = True
        pieces.append(plate)
    else:
        bevel = 0.006 if style == "chamfer" else 0.003
        pieces.append(bevelled_box("Plate", (0.022, w, h), width=bevel, segments=1 if style == "chamfer" else 2))
        if style == "lip":
            # The 3 cm lip, top and bottom, rolled out toward the Dog.
            for zs in (1, -1):
                lip = bevelled_box("Lip", (0.03, w, 0.012), width=0.004)
                lip.data.transform(Matrix.Translation((0.012, 0, zs * (h / 2 - 0.006))))
                pieces.append(lip)
    for p in pieces:
        p.data.materials.clear()
        p.data.materials.append(parts.mats[slot])
    body_plate = place(ka.join("CatchPlate", pieces), m)
    parts.add(body_plate, bone)
    # Bolts at the corners (brass on a Guild plate).
    if bolts:
        for cy, cz in ((w / 2 - 0.025, h / 2 - 0.025), (-w / 2 + 0.025, h / 2 - 0.025), (w / 2 - 0.025, -h / 2 + 0.025),
                       (-w / 2 + 0.025, -h / 2 + 0.025))[:bolts]:
            b = hex_bolt("Bolt", 0.0055)
            place(b, m @ Matrix.Translation((0.013, cy, cz)))
            parts.add(b, bone, "Accent")
    if number:
        t = stamp("Number", number, 0.028)
        place(t, m @ Matrix.Translation((0.0115, -w / 2 + 0.06, -h / 2 + 0.035)))
        parts.add(t, bone, "Stamp")
    # The rim band, just proud of the edge, all round.
    for bar, at in (((0.024, w + 0.02, 0.01), (0.003, 0.0, h / 2 + 0.005)), ((0.024, w + 0.02, 0.01), (0.003, 0.0, -h / 2 - 0.005)),
                    ((0.024, 0.01, h + 0.02), (0.003, w / 2 + 0.005, 0.0)), ((0.024, 0.01, h + 0.02), (0.003, -w / 2 - 0.005, 0.0))):
        band = bevelled_box("Rim", bar, width=0.002, segments=1)
        place(band, m @ Matrix.Translation(at))
        parts.add(band, bone, TEAM)
    return m


def buckle(parts: Parts, bone: str, m: Matrix, w: float, slot: str = "Accent"):
    """A roller buckle: a frame and its prong."""
    pieces = []
    for cy, cz, sy, sz in ((0, w * 0.45, w * 1.1, 0.006), (0, -w * 0.45, w * 1.1, 0.006), (w * 0.52, 0, 0.006, w * 0.95),
                           (-w * 0.52, 0, 0.006, w * 0.95)):
        bar = bevelled_box("BuckleBar", (0.006, sy, sz), width=0.0025)
        bar.data.transform(Matrix.Translation((0, cy, cz)))
        pieces.append(bar)
    prong = bevelled_box("Prong", (0.005, 0.004, w * 0.9), width=0.0015)
    prong.data.transform(Matrix.Translation((0.004, 0, 0)))
    pieces.append(prong)
    for p in pieces:
        p.data.materials.clear()
        p.data.materials.append(parts.mats[slot])
    parts.add(place(ka.join("Buckle", pieces), m), bone)


def dog(name: str, mats: dict):
    """A Dog: a 16 cm body, the nose, three jaws folded back (as docked)."""
    pieces = [cylinder(name, 0.016, 0.1, segments=12, bevel=0.003)]
    nose = ka.cylinder(name + "_nose", 0.016, 0.04, (0, 0, 0), axis="X", segments=12)
    for vert in nose.data.vertices:
        if vert.co.x > 0:
            vert.co.y *= 0.3
            vert.co.z *= 0.3
    nose.data.transform(Matrix.Translation((0.07, 0, 0)))
    pieces.append(nose)
    for a in range(3):
        jaw = bevelled_box(name + "_jaw", (0.06, 0.006, 0.012), width=0.002)
        jaw.data.transform(Matrix.Rotation(2 * math.pi * a / 3, 4, "X") @ Matrix.Translation((0.0, 0.0, 0.019)) @
                           Matrix.Rotation(math.radians(8), 4, "Y"))
        pieces.append(jaw)
    for p in pieces:
        p.data.materials.clear()
        p.data.materials.append(mats["Steel"])
    return ka.join(name, pieces)


def spool(parts: Parts, spec: dict, bone_frame):
    """The Spool on the right forearm at its class place and size (equipment/spool.md)."""
    head_p, tail_p = bone_frame("lowerarm_r")
    axis = (tail_p - head_p).normalized()
    at = head_p + (tail_p - head_p) * spec["spool_at"]
    length, radius = spec["spool"]
    # The housing's frame: X along the forearm toward the wrist, Z out from the body (-Y side).
    m = frame(at, axis, up=(0, -1, 0))
    drums = [0.0]
    if spec.get("twin"):
        drums = [-length * 0.26, length * 0.26]
    for i, off in enumerate(drums):
        r = radius
        body_len = length / len(drums) * (0.92 if len(drums) > 1 else 1.0)
        housing = cylinder("Housing", r, body_len, segments=20, bevel=0.004)
        place(housing, m @ Matrix.Translation((off, 0, 0)))
        parts.add(housing, "lowerarm_r", "Spool")
        # The drum seen through its slot, and the drum face with the mode ring's 12 segments.
        drum = cylinder("Drum", r * 0.86, body_len * 0.7, segments=20)
        place(drum, m @ Matrix.Translation((off, 0, r * 0.18)))
        parts.add(drum, "lowerarm_r", "Steel")
        face_m = m @ Matrix.Translation((off, 0, r * 0.98)) @ Matrix.Rotation(-math.pi / 2, 4, "Y")
        face = cylinder("DrumFace", r * 0.62, 0.006, segments=24)
        place(face, face_m)
        parts.add(face, "lowerarm_r", "Accent")
        for seg in range(12):
            a = 2 * math.pi * seg / 12
            tick = bevelled_box("ModeRing", (0.004, r * 0.17, r * 0.07), width=0.001, segments=1)
            place(tick, face_m @ Matrix.Rotation(a, 4, "X") @ Matrix.Translation((0.004, 0, r * 0.47)))
            parts.add(tick, "lowerarm_r", TEAM)
        # Two Dog cradles on the outer face, Dogs docked nose-out.
        for dz in (-1, 1):
            cradle = bevelled_box("Cradle", (body_len * 0.55, 0.026, 0.02), width=0.003)
            place(cradle, m @ Matrix.Translation((off, dz * r * 0.55, r * 0.95)))
            parts.add(cradle, "lowerarm_r", "Spool")
            d = dog("Dog", parts.mats)
            place(d, m @ Matrix.Translation((off + 0.01, dz * r * 0.55, r * 1.02)) @ Matrix.Diagonal((min(0.8, body_len / 0.16), 0.8, 0.8, 1)))
            parts.add(d, "lowerarm_r")
    # The brake lever (a brass lever on the Guild Spool, a latch on the Climber's).
    lever = bevelled_box("Brake", (length * 0.5, 0.008, 0.012), width=0.002)
    place(lever, m @ Matrix.Translation((0, -radius * 1.02, radius * 0.4)))
    parts.add(lever, "lowerarm_r", "Accent")
    # The ceramic line exit at the wrist end, with its tape bands.
    exit_m = m @ Matrix.Translation((length / 2 + 0.012, 0, radius * 0.3))
    eye = ka.ring("LineExit", 0.011, 0.005, (0, 0, 0), axis="X", segments=16, sides=8)
    place(eye, exit_m)
    parts.add(eye, "lowerarm_r", "Ceramic")
    for b in range(2):
        tape = cylinder("Tape", radius * 1.03, 0.008, segments=20)
        place(tape, m @ Matrix.Translation((length / 2 - 0.01 - b * 0.016, 0, 0)))
        parts.add(tape, "lowerarm_r", "Tape" if b == 0 else "TapeB")
    # Cradle straps round the forearm with a buckle, and the Office inspection seal.
    for sx in (-0.3, 0.3):
        strap = strap_path("CradleStrap", [m @ Vector((length * sx, math.cos(t) * radius * 1.06, math.sin(t) * radius * 1.06))
                                           for t in [2 * math.pi * i / 20 for i in range(20)]], 0.022, 0.004,
                           outward=lambda p, c=m @ Vector((length * sx, 0, 0)): p - c, closed=True)
        parts.add(strap, "lowerarm_r", "Strap")
    buckle(parts, "lowerarm_r", m @ Matrix.Translation((length * 0.3, 0, -radius * 1.1)) @ Matrix.Rotation(math.pi / 2, 4, "Y"), 0.02)
    seal = cylinder("Seal", 0.008, 0.003, segments=12)
    place(seal, m @ Matrix.Translation((-length * 0.35, radius * 0.7, radius * 0.75)) @ Matrix.Rotation(-math.pi / 2, 4, "Y"))
    parts.add(seal, "lowerarm_r", "Seal")
    if spec.get("capacitor"):  # The Scrapper's bolted-on can
        can = cylinder("Capacitor", 0.022, 0.07, segments=14, bevel=0.003)
        place(can, m @ Matrix.Translation((0, radius * 1.05, radius * 0.5)))
        parts.add(can, "lowerarm_r", "Accent")
        for sx in (-0.025, 0.025):
            clamp = cylinder("CanClamp", 0.024, 0.006, segments=14)
            place(clamp, m @ Matrix.Translation((sx, radius * 1.05, radius * 0.5)))
            parts.add(clamp, "lowerarm_r", "Steel")
    if spec.get("twin"):  # The snap guard: a flared bell at the elbow, scorched inside
        bm = bmesh.new()
        bmesh.ops.create_cone(bm, cap_ends=False, segments=20, radius1=0.07, radius2=radius * 1.05, depth=0.06)
        bmesh.ops.solidify(bm, geom=bm.faces[:], thickness=0.004)
        bell = ka.mesh_object("SnapGuard", bm)
        bell.data.transform(Matrix.Rotation(math.pi / 2, 4, "Y"))
        place(bell, m @ Matrix.Translation((-length / 2 - 0.02, 0, 0)))
        parts.add(bell, "lowerarm_r", "Spool")
    if spec.get("gauntlet"):  # The Breaker's shell, elbow to mid-forearm, two cam-lock latches
        shell = cylinder("Gauntlet", radius * 1.12, length * 1.3, segments=20, bevel=0.006)
        place(shell, m @ Matrix.Translation((-length * 0.1, 0, -radius * 0.1)))
        parts.add(shell, "lowerarm_r", "Plate")
        for sx in (-0.4, 0.3):
            latch = bevelled_box("CamLock", (0.03, 0.02, 0.012), width=0.003)
            place(latch, m @ Matrix.Translation((length * sx, -radius * 1.12, 0)))
            parts.add(latch, "lowerarm_r", "Accent")


def cutter(parts: Parts):
    """The Cutter in its sheath on the left thigh's outer face: the sheath, the guard, the grip."""
    cx, cy = parts.limb_point("thigh_l", 0.7, (0.006, 0.1))
    m = frame((cx - 0.006, cy + parts.fit(cx, cy, 0.7, reach=0.17)[1] + 0.014, 0.7), (0, 0, -1), up=(1, 0, 0))
    sheath = bevelled_box("Sheath", (0.26, 0.05, 0.022), width=0.006, segments=2)
    place(sheath, m @ Matrix.Translation((0.06, 0, 0)))
    parts.add(sheath, "thigh_l", "Boot")
    guard = bevelled_box("Guard", (0.012, 0.07, 0.03), width=0.003)
    place(guard, m @ Matrix.Translation((-0.075, 0, 0)))
    parts.add(guard, "thigh_l", "Steel")
    grip = cylinder("Grip", 0.016, 0.11, segments=12, bevel=0.003)
    place(grip, m @ Matrix.Translation((-0.135, 0, 0)))
    parts.add(grip, "thigh_l", "Strap")
    for z in (0.75, 0.65):  # The thigh straps that hold it
        cx, cy = parts.limb_point("thigh_l", z, (0.006, 0.1))
        lx, ly = parts.fit(cx, cy, z, reach=0.17)
        loop = strap_path("SheathStrap", ellipse(cx, cy, z, lx, ly, 24), 0.02, 0.004,
                          outward=lambda p, c=(cx, cy): Vector((p.x - c[0], p.y - c[1], 0)), closed=True)
        parts.add(loop, "thigh_l", "Strap")


def on_surface(field, point, lift: float = 0.0):
    """The point of a field's surface nearest `point`, and the outward normal there, lifted
    `lift` off it."""
    import numpy as np
    import harness_sdf as sdf
    p = sdf.refine(field, np.asarray([point], float), 6)
    _d, g = sdf.gradient(field, p)
    n = g[0] / max(np.linalg.norm(g[0]), 1e-9)
    return Vector(p[0] + n * lift), Vector(n)


def boot_hardware(parts: Parts, spec: dict, boots: dict):
    """What goes on the modelled boots (harness_body.Boot): eyelets and criss-cross laces with
    their bow, the Climber's hook-and-loop straps, the Breaker's shin shell, heel plate and
    buckled shaft straps."""
    import numpy as np
    for side, boot in boots.items():
        s = boot.s
        field = boot.upper
        source = f"Boot_{side}"
        bone = f"foot_{side}"
        if boot.style in ("work", "hitop") and spec.get("laces"):
            pairs = [(x, None) for x in (0.035, 0.07, 0.105)]
            pairs += [(None, z) for z in np.arange(0.14, boot.top - 0.015, 0.03)]
            holes = []
            for x, z in pairs:
                row = []
                for k in (1.0, -1.0):
                    yl = 0.019 * k * s
                    if z is None:
                        start = boot.world((x, boot.y0 + yl, 0.22))
                    else:
                        start = boot.world((0.12, boot.y0 + yl, z))
                    p, n = on_surface(field, start)
                    row.append((p, n))
                    eyelet = ka.ring("Eyelet", 0.0034, 0.0011, (0, 0, 0), axis="X", segments=12, sides=6)
                    place(eyelet, frame(p + n * 0.0006, n))
                    parts.add(eyelet, bone, "Steel" if boot.style == "work" else "Accent", skinned=True)
                    eyelet["skin_from"] = source
                holes.append(row)

            def lace(a, b, name="Lace"):
                pts = []
                for t in np.linspace(0.0, 1.0, 7):
                    q = a[0].lerp(b[0], t)
                    p, n = on_surface(field, q, lift=0.0028 + 0.0015 * math.sin(math.pi * t))
                    pts.append(tuple(p))
                obj = ka.tube(name, pts, 0.0019, sides=6)
                parts.add(obj, bone, "Accent" if spec.get("neon") else "Strap", skinned=True)
                obj["skin_from"] = source

            lace(holes[0][0], holes[0][1])
            for i in range(len(holes) - 1):
                lace(holes[i][0], holes[i + 1][1])
                lace(holes[i][1], holes[i + 1][0])
            # The bow at the top: two loops and two tails.
            top = holes[-1][0][0].lerp(holes[-1][1][0], 0.5)
            p, n = on_surface(field, top, lift=0.004)
            fwd = Vector(boot.R[:, 0]) - n * n.dot(Vector(boot.R[:, 0]))
            for k in (1.0, -1.0):
                loop = [p + Vector((0, k * s * a, 0)) * 0.02 + fwd.normalized() * b * 0.012 + n * 0.003
                        for a, b in ((0.0, 0.0), (0.5, 0.6), (1.0, 0.4), (0.9, -0.3), (0.4, -0.2), (0.0, 0.0))]
                obj = ka.tube("LaceBow", [tuple(q) for q in loop], 0.0018, sides=6)
                parts.add(obj, bone, "Accent" if spec.get("neon") else "Strap", skinned=True)
                obj["skin_from"] = source
                tail = [p, p + Vector((0.004, k * s * 0.008, -0.03)) + n * 0.004, p + Vector((0.01, k * s * 0.012, -0.055)) + n * 0.005]
                obj = ka.tube("LaceTail", [tuple(q) for q in tail], 0.0018, sides=6)
                parts.add(obj, bone, "Accent" if spec.get("neon") else "Strap", skinned=True)
                obj["skin_from"] = source
        if boot.style == "climb":  # Two hook-and-loop straps over the instep
            for x in (0.055, 0.1):
                pts = []
                for phi in np.linspace(-1.35, 1.45, 15):
                    start = boot.world((x, boot.y0 + s * 0.09 * math.sin(phi), 0.06 + 0.09 * math.cos(phi)))
                    p, n = on_surface(field, start, lift=0.0028)
                    pts.append(p)
                strap = strap_path("ShoeStrap", pts, 0.022, 0.0035, outward=lambda q, f=field: on_surface(f, q)[1], smooth=1)
                parts.add(strap, bone, "Strap", skinned=True)
                strap["skin_from"] = source
        if boot.style == "tall":
            # The hard shin shell: a plate conforming to the shaft's front.
            shell = conforming_plate(field, lambda a, z: boot.world((-0.012 + 0.2 * math.cos(a), boot.y0 + 0.2 * math.sin(a) * s, z)),
                                     np.radians(np.linspace(-62, 62, 17)), np.linspace(0.14, boot.top - 0.05, 18), 0.0035, 0.007)
            parts.add(shell, f"calf_{side}", "Plate")
            heel = conforming_plate(field, lambda a, z: boot.world((-0.2 * math.cos(a) - 0.01, boot.y0 + 0.2 * math.sin(a) * s, z)),
                                    np.radians(np.linspace(-48, 48, 13)), np.linspace(boot.sole_h + 0.006, 0.095, 9), 0.0015, 0.0045)
            parts.add(heel, bone, "Steel")
            for z in (0.27, boot.top - 0.1):  # Buckled straps round the shaft
                ring_pts = [on_surface(field, boot.world((-0.012 + 0.2 * math.cos(a), boot.y0 + 0.2 * math.sin(a), z)), lift=0.003)[0]
                            for a in np.linspace(0, 2 * math.pi, 28, endpoint=False)]
                c = Vector(boot.world((-0.012, boot.y0, z)))
                strap = strap_path("ShaftStrap", ring_pts, 0.03, 0.0035, outward=lambda q, c=c: Vector((q.x - c.x, q.y - c.y, 0)),
                                   closed=True, smooth=1)
                hi_vis = spec.get("reflective") and z < 0.3  # The Breaker's shin band
                parts.add(strap, f"calf_{side}", "Reflective" if hi_vis else "Strap")
                p, n = on_surface(field, boot.world((-0.012, boot.y0 + 0.2 * s, z)), lift=0.007)
                buckle(parts, f"calf_{side}", frame(p, n), 0.027, "Accent")


def conforming_plate(field, at, angles, heights, lift: float, thickness: float):
    """A plate shaped to a surface: a grid sampled on it (`at(angle, z)` is a point off the
    surface to start from), lifted off it, given thickness and its edges rolled."""
    import numpy as np
    bm = bmesh.new()
    grid = []
    normals = Vector()
    for z in heights:
        row = []
        for a in angles:
            p, n = on_surface(field, at(a, z), lift=lift)
            row.append(bm.verts.new(p))
            normals += n
        grid.append(row)
    for i in range(len(heights) - 1):
        for j in range(len(angles) - 1):
            bm.faces.new((grid[i][j], grid[i][j + 1], grid[i + 1][j + 1], grid[i + 1][j]))
    bm.normal_update()
    if sum((f.normal.dot(normals) for f in bm.faces)) < 0:  # Face away from the surface
        bmesh.ops.reverse_faces(bm, faces=bm.faces)
    obj = ka.mesh_object("Plate", bm)
    mod = obj.modifiers.new("Solidify", "SOLIDIFY")
    mod.thickness = thickness
    mod.offset = 1.0
    ka.add_bevel(obj, width=thickness * 0.4, segments=2)
    apply_modifiers(obj)
    for p in obj.data.polygons:
        p.use_smooth = True
    del np
    return obj


HEAD_CAP = ((-0.005, 0.0, 1.695), (0.106, 0.081, 0.124))  # The stand-in head above the eyes: centre, radii


def headgear(parts: Parts, spec: dict, fit: dict | None = None):
    """The class headgear on the stand-in head; with `fit` (another head's cap above the eyes:
    centre and radii), stretched to sit on that head instead."""
    first = len(parts.rigid)
    _headgear(parts, spec, ridge=fit is None)
    if fit is None:
        return
    import numpy as np
    c0, r0 = (np.array(x) for x in HEAD_CAP)
    c1, r1 = np.asarray(fit["centre"], float), np.asarray(fit["radii"], float)
    k = r1 / r0
    for obj in parts.rigid[first:]:
        co = np.empty(len(obj.data.vertices) * 3)
        obj.data.vertices.foreach_get("co", co)
        co = c1 + (co.reshape(-1, 3) - c0) * k
        obj.data.vertices.foreach_set("co", co.ravel())
        obj.data.update()


def _headgear(parts: Parts, spec: dict, ridge: bool = True):
    kind = spec.get("headgear", "helmet")
    centre = Vector((-0.004, 0, 1.722))
    if kind == "wrap":
        # The Climber's head wrap: bands of cloth round the crown.
        for i, z in enumerate((1.735, 1.758, 1.78)):
            band = strap_path("Wrap", ellipse(-0.004, 0, z, 0.111 - i * 0.007, 0.084 - i * 0.006, 28, tilt=0.012 * (i - 1)), 0.026,
                              0.005, outward=lambda p: Vector((p.x + 0.004, p.y, 0)), closed=True)
            parts.add(band, "head", "Strap")
        return
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=32, v_segments=16, radius=1.0)
    bmesh.ops.delete(bm, geom=[v for v in bm.verts if v.co.z < -0.12 - 0.14 * max(0.0, -v.co.x)], context="VERTS")
    sq = 0.35 if kind == "square" else 0.0  # The Rigger's head in a square frame from above
    for vert in bm.verts:
        x, y, z = vert.co
        x = math.copysign(abs(x) ** (1 - sq), x)
        y = math.copysign(abs(y) ** (1 - sq), y)
        vert.co = Vector((x * 0.122, y * 0.104, z * 0.104))
    bmesh.ops.solidify(bm, geom=bm.faces[:], thickness=0.006)
    shell = ka.mesh_object("Helmet", bm)
    for p in shell.data.polygons:
        p.use_smooth = True
    place(shell, Matrix.Translation(centre))
    parts.add(shell, "head", "Helmet")
    # The ridge down the crown, the brim or the visor, the rivets of the suspension.
    if kind in ("square", "salvage") and ridge:  # A hard hat's moulded ridge down the crown
        ridge = bmesh.new()
        bmesh.ops.create_uvsphere(ridge, u_segments=24, v_segments=12, radius=1.0)
        for vert in ridge.verts:
            vert.co = Vector((vert.co.x * 0.105, vert.co.y * 0.016, vert.co.z * 0.008))
        rid = ka.mesh_object("Ridge", ridge)
        for p in rid.data.polygons:
            p.use_smooth = True
        place(rid, Matrix.Translation(centre + Vector((0.0, 0, 0.099))))
        parts.add(rid, "head", "Helmet")
    if kind in ("square", "salvage"):
        brim = bevelled_box("Brim", (0.07, 0.19, 0.007), width=0.003)
        place(brim, Matrix.Translation((0.128, 0, 1.712)) @ Matrix.Rotation(math.radians(-10), 4, "Y"))
        parts.add(brim, "head", "Helmet")
    if kind in ("visor", "hood"):
        # A band of tinted shield in front of the eyes, curved with the shell.
        vb = bmesh.new()
        rows = []
        for zi in range(6):
            z = 1.672 + zi * 0.011
            row = []
            for ai in range(15):
                a = math.radians(-68 + ai * (136 / 14))
                row.append(vb.verts.new((centre.x + 0.136 * math.cos(a), 0.118 * math.sin(a), z)))
            rows.append(row)
        for zi in range(5):
            for ai in range(14):
                vb.faces.new((rows[zi][ai], rows[zi][ai + 1], rows[zi + 1][ai + 1], rows[zi + 1][ai]))
        bmesh.ops.solidify(vb, geom=vb.faces[:], thickness=0.004)
        visor = ka.mesh_object("Visor", vb)
        for p in visor.data.polygons:
            p.use_smooth = True
        parts.add(visor, "head", "Visor")
    for s in (1, -1):
        rivet = hex_bolt("HelmetRivet", 0.005)
        place(rivet, frame((-0.004, 0.106 * s, 1.735), (0, s, 0)))
        parts.add(rivet, "head", "Accent")


def straps(parts: Parts, spec: dict, chest_x: float):
    k = spec["bulk"]
    w = spec.get("strap_width", 0.05)
    out = lambda p: Vector((p.x, p.y, 0.0)) if abs(p.x) > 0.001 or abs(p.y) > 0.001 else Vector((1, 0, 0))
    # The waist belt and its buckle.
    bx0 = parts.torso_x(1.03)
    wx, wy = parts.fit(bx0, 0.0, 1.03, reach=0.3)
    belt = strap_path("Belt", ellipse(bx0, 0, 1.03, wx, wy, 32), w, 0.006,
                      outward=lambda p: Vector((p.x - bx0, p.y, 0.0)), closed=True)
    parts.add(belt, "pelvis", "Strap", skinned=True)
    belt["skin_bones"] = ["pelvis", "spine_01"]  # A belt rides the hips, not the thighs
    buckle(parts, "pelvis", Matrix.Translation((bx0 + wx + 0.006, 0, 1.03)), w * 0.9)
    # The chest cross: two straps from each shoulder over the chest plate's edges to the belt.
    for s in (1.0, -1.0):
        fx = lambda z: parts.fit(0.0, 0.0, z, reach=0.3)[0]
        pts = [(0.03, 0.15 * s, 1.47), (fx(1.4), 0.11 * s, 1.4), (chest_x + 0.008, 0.02 * s, 1.24),
               (fx(1.1), -0.09 * s, 1.1), (fx(1.04) * 0.9, -0.13 * s, 1.04)]
        if parts.bone_frame is not None:  # Under the chest plate, never through it
            pts = [(min(x, chest_x + 0.008) if 1.2 < z < 1.5 else x, y, z) for x, y, z in pts]
        parts.add(strap_path("ChestCross", pts, w * 0.8, 0.005, outward=out), "spine_03", "Strap", skinned=True)
        bx = lambda z: -parts.fit(0.0, 0.0, z, reach=0.3)[0]
        back = [(0.03, 0.15 * s, 1.47), (-0.04, 0.15 * s, 1.47), (bx(1.36), 0.11 * s, 1.36), (bx(1.06), 0.11 * s, 1.06)]
        parts.add(strap_path("ShoulderStrap", back, w * 0.8, 0.005, outward=out), "spine_03", "Strap", skinned=True)
    # The leg loops.
    for side, s in (("l", 1.0), ("r", -1.0)):
        cx, cy = parts.limb_point(f"thigh_{side}", 0.82, (0.006, 0.1 * s))
        lx, ly = parts.fit(cx, cy, 0.82, reach=0.17)
        loop = strap_path("LegLoop", ellipse(cx, cy, 0.82, lx, ly, 24, tilt=0.02), w * 0.7, 0.005,
                          outward=lambda p, c=(cx, cy): Vector((p.x - c[0], p.y - c[1], 0)), closed=True)
        parts.add(loop, f"thigh_{side}", "Strap")
        buckle(parts, f"thigh_{side}", Matrix.Translation((cx + lx + 0.004, cy, 0.82)), w * 0.6)


def spine_frame(parts: Parts, spec: dict, back_x: float):
    """The external spine frame (Guild rails, the Doubler's cut-down one, the Scrapper's half
    frame) with gel reservoirs and their sight glasses; or the Breaker's ribbed shell."""
    frame_kind = spec["frame"]
    x = back_x - 0.035
    if frame_kind == "shell":
        for i in range(7):
            rib = bevelled_box("Rib", (0.03, 0.34 * spec["bulk"], 0.035), width=0.01, segments=2)
            place(rib, Matrix.Translation((x + 0.01, 0, 1.06 + i * 0.05)))
            parts.add(rib, "spine_02", "Frame")
        return
    if frame_kind == "ribbons":
        for y in (-0.03, 0.03):
            ribbon = bevelled_box("Ribbon", (0.004, 0.015, 0.6), width=0.0015)
            place(ribbon, Matrix.Translation((x + 0.02, y, 1.22)))
            parts.add(ribbon, "spine_02", "Frame")
        return
    length = {"rails": 0.6, "short": 0.45, "half": 0.6}[frame_kind]
    rails = (0.07,) if frame_kind == "half" else (-0.07, 0.07)
    z0 = 0.98
    if parts.bone_frame is not None:  # Pelvis to the base of the neck on another body
        neck = parts.bone_frame("neck_01")[0].z
        length *= (neck + 0.02 - z0) / 0.6
    for y in rails:
        rail = bevelled_box("Rail", (0.025, 0.025, length), width=0.004)
        place(rail, Matrix.Translation((x, y, z0 + length / 2)))
        parts.add(rail, "spine_02", "Frame")
    braces = (z0 + 0.08, z0 + length / 2, z0 + length - 0.06)
    for z in braces[: 2 if frame_kind == "short" else 3]:
        brace = bevelled_box("Brace", (0.02, 0.16 if len(rails) == 2 else 0.12, 0.022), width=0.004)
        place(brace, Matrix.Translation((x, 0.0 if len(rails) == 2 else 0.03, z)))
        parts.add(brace, "spine_02", "Frame")
        for y in rails:
            b = hex_bolt("FrameBolt", 0.005)
            place(b, frame((x - 0.013, y, z), (-1, 0, 0)))
            parts.add(b, "spine_02", "Accent")
    # The gel reservoirs at kidney height with sight glasses.
    for y in [r * 1.55 for r in rails]:
        tank = cylinder("Gel", 0.035, 0.22, segments=16, bevel=0.006)
        place(tank, frame((x - 0.02, y, 1.11), (0, 0, 1)))
        parts.add(tank, "spine_02", "Steel")
        for cz in (1.0, 1.22):
            cap = cylinder("GelCap", 0.038, 0.014, segments=16, bevel=0.003)
            place(cap, frame((x - 0.02, y, cz), (0, 0, 1)))
            parts.add(cap, "spine_02", "Accent")
        glass = bevelled_box("SightGlass", (0.006, 0.018, 0.14), width=0.003)
        place(glass, Matrix.Translation((x - 0.055, y, 1.11)))
        parts.add(glass, "spine_02", "Gel")
