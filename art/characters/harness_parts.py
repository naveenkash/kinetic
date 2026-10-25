"""The detailed parts of a harness character (art/characters/harness.py builds and rigs them).

Every part is made in the rest pose, in the skeleton's frame (metres; X forward, Y the
character's left, Z up), and either follows one bone rigidly (hard parts: plates, the Spool,
buckles, the helmet) or is weighted to the skeleton automatically (the body, which bends).

- The body: a skin-modifier figure over the skeleton's joints (torso, arms, five-fingered hands,
  legs), subdivided twice, so it reads as a person and bends at the joints.
- The head: a face (jaw, cheekbones, brow, nose, ears, eyes) on a subdivided head shape.
- The harness: Catch Plates at their class sizes with lips or chamfers, rolled edges, bolts,
  stamped numbers and the team-tint rim band; straps that follow the body (the chest cross, the
  shoulder straps, the waist belt with its buckle, the leg loops); the spine frame with its gel
  reservoirs and sight glasses; the telemetry pack and its seal; the lease plate.
- The Spool on the right forearm: housing, drum slot, drum face with the 12-segment mode ring,
  two Dog cradles with Dogs in them, the ceramic line exit and its tape bands, the brake, the
  cradle straps and buckle, the inspection seal; and each class's variant (the Climber's cuff,
  the Scrapper's capacitor can, the Doubler's twin drums and snap guard, the Breaker's gauntlet).
- The Cutter in its sheath, boots and their soles, the class headgear, and each class's
  extras (the Scrapper's pouches, chain and wrench; the Doubler's neon tape; the Climber's wraps;
  the Breaker's ribbed shell and blade shoulders).
"""
from __future__ import annotations

import math

import bmesh
import bpy
from mathutils import Matrix, Vector

import kinetic_art as ka

TEAM = "TeamTint"


class Parts:
    """Collects parts; each records which bone it follows (None: weighted automatically)."""

    def __init__(self, mats: dict):
        self.mats = mats
        self.rigid: list = []   # (object, bone)
        self.body = None        # The figure, once made: straps are sized to it

    def fit(self, cx: float, cy: float, z: float, pad: float = 0.006, reach: float = 0.2):
        """The body's half-widths (front-back, side-side) in a 4 cm slice at z around (cx, cy),
        plus pad: a strap that goes round it sits on it."""
        pts = [v.co for v in self.body.data.vertices
               if abs(v.co.z - z) < 0.02 and math.hypot(v.co.x - cx, v.co.y - cy) < reach
               and (abs(cy) < 1e-6 or v.co.y * cy > 0)]  # A limb's slice stays on its own side
        if not pts:
            return 0.08, 0.08
        return (max(abs(p.x - cx) for p in pts) + pad, max(abs(p.y - cy) for p in pts) + pad)

    def add(self, obj, bone: str, slot: str | None = None):
        if slot is not None:
            obj.data.materials.clear()
            obj.data.materials.append(self.mats[slot])
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


def strap_path(name: str, points, width: float, thickness: float, outward=None, closed: bool = False):
    """A flat strap along points: its face turned to `outward(p)` (away from the body)."""
    pts = [Vector(p) for p in points]
    bm = bmesh.new()
    rings = []
    n = len(pts)
    for i, p in enumerate(pts):
        a = pts[(i - 1) % n] if (closed or i > 0) else p
        b = pts[(i + 1) % n] if (closed or i < n - 1) else p
        t = (b - a).normalized()
        o = (outward(p) if outward else Vector((1, 0, 0))).normalized()
        side = t.cross(o).normalized()
        o = side.cross(t).normalized()
        ring = []
        for sx, so in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
            ring.append(bm.verts.new(p + side * sx * width / 2 + o * so * thickness / 2))
        rings.append(ring)
    for i in range(n if closed else n - 1):
        r0, r1 = rings[i], rings[(i + 1) % n]
        for k in range(4):
            bm.faces.new((r0[k], r0[(k + 1) % 4], r1[(k + 1) % 4], r1[k]))
    if not closed:
        bm.faces.new(list(reversed(rings[0])))
        bm.faces.new(rings[-1])
    return ka.mesh_object(name, bm)


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


# The body ---------------------------------------------------------------------------------------

def body(parts: Parts, spec: dict, slot: str = "Body"):
    """A skin-modifier figure: joints with radii (front-back, side-side), two subdivisions."""
    k, limb = spec["bulk"], spec.get("limb", 1.0)
    verts, radii, edges = [], [], []

    def v(p, r, parent=None):
        verts.append(Vector(p))
        radii.append((r[0] * 1.32, r[1] * 1.32))  # The subdivision shrinks a skin by about a quarter
        i = len(verts) - 1
        if parent is not None:
            edges.append((parent, i))
        return i

    pel = v((0, 0, 0.95), (0.125 * k, 0.16 * k))
    s1 = v((0.0, 0, 1.06), (0.11 * k, 0.145 * k), pel)
    s2 = v((0.008, 0, 1.19), (0.12 * k, 0.155 * k), s1)
    ch = v((0.014, 0, 1.32), (0.13 * k, 0.17 * k), s2)
    up = v((0.004, 0, 1.43), (0.105 * k, 0.16 * k), ch)
    nk = v((0.006, 0, 1.5), (0.05, 0.056), up)
    v((0.012, 0, 1.57), (0.047, 0.052), nk)
    for s in (1.0, -1.0):
        sh = v((0.0, 0.17 * s, 1.425), (0.058 * k, 0.06 * k), up)
        arm = v((0.0, 0.205 * s, 1.29), (0.05 * k * limb, 0.05 * k * limb), sh)
        el = v((0.0, 0.23 * s, 1.15), (0.042 * k * limb, 0.042 * k * limb), arm)
        fa = v((0.004, 0.243 * s, 1.03), (0.04 * k * limb, 0.037 * k * limb), el)
        wr = v((0.006, 0.25 * s, 0.9), (0.028, 0.024), fa)
        palm = v((0.012, 0.255 * s, 0.835), (0.022, 0.04), wr)
        # Four fingers hanging from the palm's front edge, the thumb off its front.
        for dx in (-0.022, -0.007, 0.008, 0.022):
            kn = v((0.012 + dx, 0.257 * s, 0.8), (0.0095, 0.0095), palm)
            mid = v((0.012 + dx * 1.05, 0.259 * s, 0.77), (0.0085, 0.0085), kn)
            v((0.014 + dx * 1.1, 0.259 * s, 0.742 + abs(dx) * 0.4), (0.007, 0.007), mid)
        th0 = v((0.034, 0.247 * s, 0.84), (0.011, 0.011), palm)
        th1 = v((0.05, 0.243 * s, 0.815), (0.009, 0.009), th0)
        v((0.058, 0.242 * s, 0.792), (0.0075, 0.0075), th1)
        hip = v((0.0, 0.095 * s, 0.9), (0.085 * k, 0.082 * k), pel)
        th = v((0.006, 0.1 * s, 0.72), (0.072 * k * limb, 0.07 * k * limb), hip)
        kn = v((0.014, 0.1 * s, 0.52), (0.054 * k * limb, 0.052 * k * limb), th)
        cf = v((0.0, 0.1 * s, 0.32), (0.055 * k * limb, 0.05 * k * limb), kn)
        an = v((-0.005, 0.1 * s, 0.11), (0.037, 0.034), cf)
        heel = v((0.02, 0.1 * s, 0.05), (0.04, 0.04), an)
        v((0.17, 0.1 * s, 0.035), (0.03, 0.042), heel)
    mesh = bpy.data.meshes.new("Body")
    mesh.from_pydata([tuple(p) for p in verts], edges, [])
    obj = bpy.data.objects.new("Body", mesh)
    bpy.context.scene.collection.objects.link(obj)
    skin = obj.modifiers.new("Skin", "SKIN")
    skin.use_smooth_shade = True
    skin.branch_smoothing = 0.4
    for i, r in enumerate(radii):
        obj.data.skin_vertices[0].data[i].radius = r
    obj.data.skin_vertices[0].data[pel].use_root = True
    sub = obj.modifiers.new("Subdivision", "SUBSURF")
    sub.levels = sub.render_levels = 2
    apply_modifiers(obj)
    obj.data.materials.append(parts.mats[slot])
    parts.body = obj
    return obj


def head(parts: Parts, skin_slot: str = "Skin"):
    """A face on a head: jaw and chin forward and down, cheekbones, brow ridge, a nose, ears and
    eyes, subdivided."""
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=20, v_segments=14, radius=1.0)
    for vert in bm.verts:
        x, y, z = vert.co
        sx, sy, sz = 0.094, 0.078, 0.112
        # The jaw: the lower front narrows and comes forward; the chin is a little square.
        if z < -0.1:
            sy *= 1.0 - 0.18 * (-z - 0.1)
            if x > 0:
                x *= 1.0 + 0.12 * (-z)
        # Cheekbones and the back of the skull.
        if 0.0 < z < 0.35 and x > 0.3:
            sy *= 1.04
        if x < -0.2:
            sx *= 1.06
        vert.co = Vector((x * sx, y * sy, z * sz))
    obj = ka.mesh_object("Head", bm)
    sub = obj.modifiers.new("Subdivision", "SUBSURF")
    sub.levels = 1
    apply_modifiers(obj)
    for p in obj.data.polygons:
        p.use_smooth = True
    pieces = [obj]
    nose = bevelled_box("Nose", (0.03, 0.026, 0.045), width=0.008, segments=3)
    nose.data.transform(Matrix.Translation((0.092, 0, -0.005)) @ Matrix.Rotation(math.radians(-12), 4, "Y"))
    brow = bevelled_box("Brow", (0.02, 0.12, 0.016), width=0.007, segments=3)
    brow.data.transform(Matrix.Translation((0.083, 0, 0.032)))
    pieces += [nose, brow]
    for s in (1.0, -1.0):
        ear = cylinder("Ear", 0.022, 0.012, segments=12)
        ear.data.transform(Matrix.Translation((-0.005, 0.078 * s, 0.005)) @ Matrix.Rotation(math.pi / 2, 4, "Z") @
                           Matrix.Diagonal((1.0, 0.8, 1.35, 1.0)))
        pieces.append(ear)
    for p in pieces:
        p.data.materials.clear()
        p.data.materials.append(parts.mats[skin_slot])
    face = ka.join("Head", pieces)
    face.data.transform(Matrix.Translation((0.012, 0, 1.675)))
    parts.add(face, "head")
    for s in (1.0, -1.0):
        eye = ka.cylinder("Eye", 0.011, 0.006, (0, 0, 0), axis="X", segments=12)
        place(eye, Matrix.Translation((0.012 + 0.083, 0.03 * s, 1.675 + 0.012)))
        parts.add(eye, "head", "Dark")


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
    m = frame((0.0, 0.1 + parts.fit(0.006, 0.1, 0.7, reach=0.11)[1] + 0.014, 0.7), (0, 0, -1), up=(1, 0, 0))
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
        lx, ly = parts.fit(0.006, 0.1, z, reach=0.11)
        loop = strap_path("SheathStrap", ellipse(0.006, 0.1, z, lx, ly, 24), 0.02, 0.004,
                          outward=lambda p: Vector((p.x - 0.006, p.y - 0.1, 0)), closed=True)
        parts.add(loop, "thigh_l", "Strap")


def boots(parts: Parts, spec: dict):
    for side, s in (("l", 1.0), ("r", -1.0)):
        y = 0.1 * s
        tall = spec.get("boot_top", 0.2)
        leg = max(parts.fit(0.0, y, z, pad=0.008, reach=0.12)[0] for z in (0.1, min(tall - 0.02, 0.45)))
        shaft = cylinder("BootShaft", max(0.058, leg), tall - 0.06, segments=16, bevel=0.006)
        place(shaft, frame((0.0, y, 0.06 + (tall - 0.06) / 2), (0, 0, 1)))
        parts.add(shaft, f"calf_{side}", "Boot")
        foot = bevelled_box("BootFoot", (0.27, 0.105, 0.085), width=0.03, segments=3)
        place(foot, Matrix.Translation((0.055, y, 0.055)))
        parts.add(foot, f"foot_{side}", "Boot")
        sole = bevelled_box("Sole", (0.285, 0.115, 0.025), width=0.008, segments=2)
        place(sole, Matrix.Translation((0.055, y, 0.0125)))
        parts.add(sole, f"foot_{side}", "Sole")
        toe = bevelled_box("ToeCap", (0.07, 0.1, 0.05), width=0.018, segments=3)
        place(toe, Matrix.Translation((0.16, y, 0.045)))
        parts.add(toe, f"ball_{side}", "Steel" if spec.get("steel_toe", True) else "Boot")
        if spec.get("shin_shell"):  # The Breaker's knee-high hard shell and heel plate
            shell = bevelled_box("ShinShell", (0.03, 0.1, tall - 0.12), width=0.012, segments=2)
            place(shell, Matrix.Translation((max(0.058, leg) + 0.012, y, 0.1 + (tall - 0.12) / 2)))
            parts.add(shell, f"calf_{side}", "Plate")
            heel = bevelled_box("HeelPlate", (0.04, 0.09, 0.06), width=0.006)
            place(heel, Matrix.Translation((-0.06, y, 0.05)))
            parts.add(heel, f"foot_{side}", "Steel")
        if spec.get("laces"):
            for i in range(4):
                lace = bevelled_box("Lace", (0.008, 0.07, 0.006), width=0.002, segments=1)
                place(lace, Matrix.Translation((0.07 + i * 0.012, y, 0.1 + i * 0.022)) @ Matrix.Rotation(math.radians(-35), 4, "Y"))
                parts.add(lace, f"foot_{side}", "Accent")


def headgear(parts: Parts, spec: dict):
    kind = spec.get("headgear", "helmet")
    centre = Vector((0.006, 0, 1.73))
    if kind == "wrap":
        # The Climber's head wrap: bands of cloth round the crown.
        for i, z in enumerate((1.7, 1.725, 1.75)):
            band = strap_path("Wrap", ellipse(0.008, 0, z, 0.098 - i * 0.004, 0.083 - i * 0.004, 24, tilt=0.01 * (i - 1)), 0.03,
                              0.006, outward=lambda p: Vector((p.x - 0.008, p.y, 0)), closed=True)
            parts.add(band, "head", "Strap")
        return
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=24, v_segments=12, radius=1.0)
    bmesh.ops.delete(bm, geom=[v for v in bm.verts if v.co.z < -0.15], context="VERTS")
    sq = 0.35 if kind == "square" else 0.0  # The Rigger's head in a square frame from above
    for vert in bm.verts:
        x, y, z = vert.co
        x = math.copysign(abs(x) ** (1 - sq), x)
        y = math.copysign(abs(y) ** (1 - sq), y)
        vert.co = Vector((x * 0.112, y * 0.098, z * 0.09))
    bmesh.ops.solidify(bm, geom=bm.faces[:], thickness=0.006)
    shell = ka.mesh_object("Helmet", bm)
    for p in shell.data.polygons:
        p.use_smooth = True
    place(shell, Matrix.Translation(centre))
    parts.add(shell, "head", "Helmet")
    # A brim or a visor, and the shoulder of the shell's lamp clip.
    if kind in ("square", "salvage"):
        brim = bevelled_box("Brim", (0.07, 0.19, 0.008), width=0.003)
        place(brim, Matrix.Translation((0.1, 0, 1.712)) @ Matrix.Rotation(math.radians(-8), 4, "Y"))
        parts.add(brim, "head", "Helmet")
    if kind in ("visor", "hood"):
        visor = bevelled_box("Visor", (0.02, 0.17, 0.05), width=0.008, segments=2)
        place(visor, Matrix.Translation((0.1, 0, 1.69)))
        parts.add(visor, "head", "Visor")
    for s in (1, -1):
        rivet = hex_bolt("HelmetRivet", 0.005)
        place(rivet, frame((0.0, 0.098 * s, 1.73), (0, s, 0)))
        parts.add(rivet, "head", "Accent")


def straps(parts: Parts, spec: dict, chest_x: float):
    k = spec["bulk"]
    w = spec.get("strap_width", 0.05)
    out = lambda p: Vector((p.x, p.y, 0.0)) if abs(p.x) > 0.001 or abs(p.y) > 0.001 else Vector((1, 0, 0))
    # The waist belt and its buckle.
    wx, wy = parts.fit(0.0, 0.0, 1.03, reach=0.3)
    belt = strap_path("Belt", ellipse(0.0, 0, 1.03, wx, wy, 32), w, 0.006, outward=out, closed=True)
    parts.add(belt, "pelvis", "Strap")
    buckle(parts, "pelvis", Matrix.Translation((wx + 0.006, 0, 1.03)), w * 0.9)
    # The chest cross: two straps from each shoulder over the chest plate's edges to the belt.
    for s in (1.0, -1.0):
        fx = lambda z: parts.fit(0.0, 0.0, z, reach=0.3)[0]
        pts = [(0.03, 0.15 * s, 1.47), (fx(1.4), 0.11 * s, 1.4), (chest_x + 0.008, 0.02 * s, 1.24),
               (fx(1.1), -0.09 * s, 1.1), (fx(1.04) * 0.9, -0.13 * s, 1.04)]
        parts.add(strap_path("ChestCross", pts, w * 0.8, 0.005, outward=out), "spine_03", "Strap")
        bx = lambda z: -parts.fit(0.0, 0.0, z, reach=0.3)[0]
        back = [(0.03, 0.15 * s, 1.47), (-0.04, 0.15 * s, 1.47), (bx(1.36), 0.11 * s, 1.36), (bx(1.06), 0.11 * s, 1.06)]
        parts.add(strap_path("ShoulderStrap", back, w * 0.8, 0.005, outward=out), "spine_03", "Strap")
    # The leg loops.
    for side, s in (("l", 1.0), ("r", -1.0)):
        lx, ly = parts.fit(0.006, 0.1 * s, 0.82, reach=0.11)
        loop = strap_path("LegLoop", ellipse(0.006, 0.1 * s, 0.82, lx, ly, 24, tilt=0.02), w * 0.7, 0.005,
                          outward=lambda p, c=0.1 * s: Vector((p.x - 0.006, p.y - c, 0)), closed=True)
        parts.add(loop, f"thigh_{side}", "Strap")
        buckle(parts, f"thigh_{side}", Matrix.Translation((0.006 + lx + 0.004, 0.1 * s, 0.82)), w * 0.6)


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
