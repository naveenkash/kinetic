"""The harness fitted to the body it is worn on: a rope-access / fall-arrest harness with armour
plates, as harness.harness_parts_on puts it on the stand-in figure or a MetaHuman.

- Chest and back Catch Plates at the specs' sizes (docs/characters/harnesses: Dogs seat on them),
  each a rigid shell curved to the torso under it (a smooth fit of the body's surface, then
  lifted clear of it), with a rolled edge in team tint, bolts, the stamped number, and a buckled
  tab at each corner tying it into the harness.
- Shoulder plates as small pauldrons: caps of a sphere round the shoulder joint, hugging the top
  of the deltoid and riding the upper arm (they pivot where the arm does, so it never cuts
  through them).
- The harness webbing: two shoulder straps from the dorsal D-ring over the shoulders and down the
  front to the waist belt, the belt snug at the hips, leg loops snug on the thighs with their
  front connectors; every strap is laid on the coverall's surface.
- The spine hardware close on the back: the Guild's tube rails and braces (the Doubler's cut-down
  frame, the Scrapper's half frame), the Climber's ribbons, the Breaker's shell ribs; the gel
  reservoirs at the kidneys and the telemetry pack under the back plate.
"""
from __future__ import annotations

import math

import bmesh
import numpy as np
from mathutils import Matrix, Vector
from mathutils.bvhtree import BVHTree

import kinetic_art as ka
import harness_parts as hp

TEAM = "TeamTint"


class Body:
    """Ray and nearest-point queries on the body the harness goes on (parts.body, the coverall):
    the torso alone (the arms out of the way) or all of it."""

    def __init__(self, parts):
        self.parts = parts
        me = parts.body.data
        me.calc_loop_triangles()
        tris = np.empty(len(me.loop_triangles) * 3, np.int32)
        me.loop_triangles.foreach_get("vertices", tris)
        tris = tris.reshape(-1, 3)
        co = parts._co
        self.full = BVHTree.FromPolygons(co.tolist(), tris.tolist())
        torso = tris[parts._torso[tris].all(1)]
        self.torso = BVHTree.FromPolygons(co.tolist(), torso.tolist())

    def ray(self, origin, direction, torso: bool = True):
        tree = self.torso if torso else self.full
        loc, n, _i, _d = tree.ray_cast(Vector(origin), Vector(direction).normalized(), 5.0)
        if loc is None:
            return None, None
        if n.dot(Vector(direction)) > 0:
            n = -n
        return loc, n

    def front(self, y, z):
        return self.ray((2.0, y, z), (-1, 0, 0))

    def back(self, y, z):
        return self.ray((-2.0, y, z), (1, 0, 0))

    def top(self, x, y):
        return self.ray((x, y, 3.0), (0, 0, -1))

    def side(self, x, z, s):
        return self.ray((x, 2.0 * s, z), (0, -s, 0))

    def hug(self, points, lift: float, step: float = 0.015, torso: bool = True):
        """A path laid on the surface: the polyline resampled every `step` and every sample put
        `lift` off the nearest surface point. Returns the points and their outward normals."""
        pts = [Vector(p) for p in points]
        out = [pts[0]]
        for a, b in zip(pts, pts[1:]):
            n = max(1, int((b - a).length / step))
            out += [a.lerp(b, (i + 1) / n) for i in range(n)]
        tree = self.torso if torso else self.full
        laid, normals = [], []
        for p in out:
            loc, n, _i, _d = tree.find_nearest(p)
            if loc is None:
                laid.append(p)
                normals.append(Vector((1, 0, 0)))
                continue
            if n.dot(p - loc) < 0 and (p - loc).length > 1e-5:
                n = -n
            laid.append(loc + n * lift)
            normals.append(n)
        return laid, normals


def outline(u, v, e: float):
    """The square (u, v) in [-1, 1] mapped onto a superellipse of exponent e (a plate's outline:
    e 2 an oval, 6 a rounded rectangle)."""
    m = max(abs(u), abs(v))
    if m < 1e-9:
        return 0.0, 0.0
    r = (abs(u) ** e + abs(v) ** e) ** (1.0 / e)
    return u * m / r, v * m / r


def strap(parts, name, pts, normals, width, thickness, slot="Strap", closed=False, bone=None, skinned=True):
    lookup = {i: n for i, n in enumerate(normals)}
    pts = list(pts)

    def outward(p, pts=pts):
        i = min(range(len(pts)), key=lambda k: (pts[k] - p).length_squared)
        return lookup[i]

    obj = hp.strap_path(name, pts, width, thickness, outward=outward, closed=closed, smooth=1)
    parts.add(obj, bone, slot, skinned=skinned or bone is None)
    return obj


# Plates -----------------------------------------------------------------------------------------

class Plate:
    """A rigid plate curved to the torso: x(y, z) is a quadratic fit of the body under it, lifted
    so its inner face clears the body by `gap` everywhere."""

    def __init__(self, body: Body, size, zc: float, facing: float, gap: float, thick: float, bulge: float = 0.0,
                 shape: float = 6.0):
        self.w, self.h = size
        self.zc, self.s, self.thick, self.bulge, self.shape = zc, facing, thick, bulge, shape
        ys = np.linspace(-self.w / 2 - 0.01, self.w / 2 + 0.01, 11)
        zs = np.linspace(zc - self.h / 2 - 0.01, zc + self.h / 2 + 0.01, 11)
        rows = []
        for y in ys:
            for z in zs:
                loc, _n = (body.front if facing > 0 else body.back)(y, z)
                if loc is not None:
                    rows.append((y, z, loc.x))
        rows = np.array(rows)
        y, z, x = rows[:, 0], rows[:, 1] - zc, rows[:, 2]
        A = np.stack([np.ones_like(y), y, z, y * y, y * z, z * z], 1)
        self.c = np.linalg.lstsq(A, x, rcond=None)[0]
        fit = A @ self.c
        self.c[0] += facing * (max(0.0, float((facing * (x - fit)).max())) + gap)

    def inner(self, y, z):
        zz = z - self.zc
        c = self.c
        x = c[0] + c[1] * y + c[2] * zz + c[3] * y * y + c[4] * y * zz + c[5] * zz * zz
        dx_dy = c[1] + 2 * c[3] * y + c[4] * zz
        dx_dz = c[2] + c[4] * y + 2 * c[5] * zz
        n = Vector((self.s, -self.s * dx_dy, -self.s * dx_dz)).normalized()
        return Vector((x, y, z)), n

    def outer(self, y, z):
        p, n = self.inner(y, z)
        un, vn = 2 * y / self.w, 2 * (z - self.zc) / self.h
        t = self.thick + self.bulge * max(0.0, 1 - un * un - vn * vn)
        return p + n * t, n

    def mesh(self, name: str, nu: int = 19, nv: int = 17):
        bm = bmesh.new()
        grid = []
        for i in range(nu):
            row = []
            for j in range(nv):
                u, v = -1 + 2 * i / (nu - 1), -1 + 2 * j / (nv - 1)
                u, v = outline(u, v, self.shape)
                y, z = u * self.w / 2, self.zc + v * self.h / 2
                p, n = self.inner(y, z)
                q, _ = self.outer(y, z)
                row.append((bm.verts.new(p), bm.verts.new(q)))
            grid.append(row)
        for i in range(nu - 1):
            for j in range(nv - 1):
                a, b, c, d = grid[i][j], grid[i + 1][j], grid[i + 1][j + 1], grid[i][j + 1]
                bm.faces.new((a[1], b[1], c[1], d[1]))
                bm.faces.new((d[0], c[0], b[0], a[0]))
        ring = [(i, 0) for i in range(nu)] + [(nu - 1, j) for j in range(1, nv)] + \
               [(i, nv - 1) for i in range(nu - 2, -1, -1)] + [(0, j) for j in range(nv - 2, 0, -1)]
        for k in range(len(ring)):
            (i0, j0), (i1, j1) = ring[k], ring[(k + 1) % len(ring)]
            a, b = grid[i0][j0], grid[i1][j1]
            bm.faces.new((a[0], b[0], b[1], a[1]))
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        obj = ka.mesh_object(name, bm)
        for p in obj.data.polygons:
            p.use_smooth = True
        obj.data.set_sharp_from_angle(angle=math.radians(50))
        return obj

    def edge_path(self, n: int = 72, inset: float = 0.0):
        pts = []
        for k in range(n):
            a = 2 * math.pi * k / n
            u, v = math.cos(a), math.sin(a)
            m = max(abs(u), abs(v))
            u, v = outline(u / m, v / m, self.shape)
            y = u * (self.w / 2 - inset)
            z = self.zc + v * (self.h / 2 - inset)
            p, nrm = self.inner(y, z)
            pts.append(p + nrm * (self.thick / 2))
        return pts

    def corner(self, sy: float, sz: float, inset: float = 0.03):
        """A point on the plate's face near a corner, and the face's normal there."""
        return self.outer(sy * (self.w / 2 - inset), self.zc + sz * (self.h / 2 - inset))


def plate(parts, body: Body, bone: str, size, zc: float, facing: float, style: str, number: str | None,
          gap: float, thick: float = 0.008) -> Plate:
    bulge = 0.009 if style == "mirror" else 0.0
    P = Plate(body, size, zc, facing, gap, thick, bulge, shape=2.6 if style == "mirror" else 6.0)
    parts.add(P.mesh("CatchPlate"), bone, "Plate")
    # The rolled edge, carrying the team-tint band all round.
    rim = ka.tube("Rim", [tuple(p) for p in P.edge_path()], thick / 2 + 0.0025, sides=10, closed=True)
    parts.add(rim, bone, TEAM)
    if style != "mirror":
        for sy in (1, -1):
            for sz in (1, -1):
                p, n = P.corner(sy, sz, 0.028)
                b = hp.hex_bolt("Bolt", 0.0052)
                hp.place(b, hp.frame(p + n * 0.001, n))
                parts.add(b, bone, "Accent")
    if number:
        p, n = P.outer(-facing * (P.w / 2 - 0.065), P.zc - P.h / 2 + 0.035)
        t = hp.stamp("Number", number, 0.026)
        hp.place(t, hp.frame(p - n * 0.0001, n))
        parts.add(t, bone, "Stamp")
    return P


class Pauldron:
    """A cap of a sphere round the shoulder joint over the top of the deltoid: it rides the upper
    arm and pivots with it about the joint."""

    def __init__(self, body: Body, joint, s: float, size, gap: float = 0.008, thick: float = 0.007, tilt: float = 0.0):
        self.J = Vector(joint)
        # Out over the deltoid, and up; tilted out by `tilt` degrees where the rest pose holds
        # the arm away from the side (half the way: it rides half the arm's swing).
        self.c = (Matrix.Rotation(math.radians(-tilt * s), 3, "X") @ Vector((0.0, 0.6 * s, 0.8))).normalized()
        self.a1 = Vector((1.0, 0.0, 0.0))                    # Front to back
        self.a2 = self.c.cross(self.a1).normalized()         # Down the arm, over the shoulder
        self.w, self.h, self.thick = size[0], size[1], thick
        r0 = 0.075
        self.au, self.av = (self.w / 2) / r0, (self.h / 2) / r0
        hits = []
        for i in range(9):
            for j in range(9):
                d = self.dir(-1 + i / 4, -1 + j / 4)
                loc, _n = body.ray(self.J, d, torso=False)
                if loc is not None and (loc - self.J).length < 0.2:
                    hits.append((loc - self.J).length)
        self.R = (max(hits) if hits else r0) + gap

    def dir(self, u, v):
        return (self.c + self.a1 * math.tan(u * self.au) + self.a2 * math.tan(v * self.av)).normalized()

    def mesh(self, name: str, n: int = 15):
        bm = bmesh.new()
        grid = []
        for i in range(n):
            row = []
            for j in range(n):
                u, v = outline(-1 + 2 * i / (n - 1), -1 + 2 * j / (n - 1), 2.4)
                d = self.dir(u, v)
                row.append((bm.verts.new(self.J + d * self.R), bm.verts.new(self.J + d * (self.R + self.thick))))
            grid.append(row)
        for i in range(n - 1):
            for j in range(n - 1):
                a, b, c, d = grid[i][j], grid[i + 1][j], grid[i + 1][j + 1], grid[i][j + 1]
                bm.faces.new((a[1], b[1], c[1], d[1]))
                bm.faces.new((d[0], c[0], b[0], a[0]))
        ring = [(i, 0) for i in range(n)] + [(n - 1, j) for j in range(1, n)] + \
               [(i, n - 1) for i in range(n - 2, -1, -1)] + [(0, j) for j in range(n - 2, 0, -1)]
        for k in range(len(ring)):
            (i0, j0), (i1, j1) = ring[k], ring[(k + 1) % len(ring)]
            a, b = grid[i0][j0], grid[i1][j1]
            bm.faces.new((a[0], b[0], b[1], a[1]))
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        obj = ka.mesh_object(name, bm)
        for p in obj.data.polygons:
            p.use_smooth = True
        return obj

    def edge_path(self, n: int = 56):
        pts = []
        for k in range(n):
            a = 2 * math.pi * k / n
            u, v = math.cos(a), math.sin(a)
            m = max(abs(u), abs(v))
            u, v = outline(u / m, v / m, 2.4)
            pts.append(self.J + self.dir(u, v) * (self.R + self.thick / 2))
        return pts

    def at(self, u, v):
        d = self.dir(u, v)
        return self.J + d * (self.R + self.thick), d


def share(obj, a: str, b: str, k: float = 0.5) -> None:
    """A rigid part carried by two bones (a pauldron: half the upper arm's swing, half the
    shoulder's). Both turn about the shoulder joint, so it stays on its sphere round it."""
    ga = obj.vertex_groups.get(a)
    idx = list(range(len(obj.data.vertices)))
    ga.add(idx, 1.0 - k, "REPLACE")
    gb = obj.vertex_groups.get(b) or obj.vertex_groups.new(name=b)
    gb.add(idx, k, "REPLACE")


# The harness --------------------------------------------------------------------------------------

def fitted(parts, spec: dict, frame_of, boots) -> dict:
    """The plates, pauldrons, webbing and spine hardware on parts.body. Returns what the rest of
    the gear is placed against (the chest plate for the lease tag, the left pauldron for the lamp)."""
    body = Body(parts)
    style = spec["plate_style"]
    w = spec.get("strap_width", 0.05)
    webbing_t = 0.0042
    lift = webbing_t / 2 + 0.0015
    frame_kind = spec["frame"]
    railed = frame_kind in ("rails", "short", "half")
    zc_back = 1.32 if spec.get("clutter") else 1.36
    chest = plate(parts, body, "spine_03", spec["chest"], 1.35, 1.0, style, spec.get("number"), gap=0.011)
    back = plate(parts, body, "spine_03", spec["back"], zc_back, -1.0, style, None, gap=0.026 if railed else 0.011)
    # Pauldrons on the deltoids.
    caps = {}
    for side, s in (("l", 1.0), ("r", -1.0)):
        size = spec["shoulder"] if side == "l" or "shoulder_r" not in spec else spec["shoulder_r"]
        size = (size[0] * 0.85, size[1] * 0.85)
        J, tail = frame_of(f"upperarm_{side}")
        down = math.degrees((tail - J).normalized().angle(Vector((0, 0, -1))))
        cap = Pauldron(body, J, s, size, tilt=max(0.0, down - 8.0) * 0.5)
        bone = f"upperarm_{side}"
        pieces = [parts.add(cap.mesh("Pauldron"), bone, "Plate"),
                  parts.add(ka.tube("PauldronRim", [tuple(p) for p in cap.edge_path()], cap.thick / 2 + 0.002, sides=8,
                                    closed=True), bone, TEAM)]
        if style != "mirror":
            for u in (-0.6, 0.6):
                p, n = cap.at(u, -0.55)
                b = hp.hex_bolt("PauldronBolt", 0.0045)
                hp.place(b, hp.frame(p, n))
                pieces.append(parts.add(b, bone, "Accent"))
        for obj in pieces:
            share(obj, bone, f"clavicle_{side}")
        caps[side] = cap
    # The webbing: shoulder straps from the dorsal ring over the shoulders to the belt.
    belt_z = 1.03
    bx = parts.torso_x(belt_z)
    for side, s in (("l", 1.0), ("r", -1.0)):
        ctl = []
        for fn, a, b in ((body.front, 0.085 * s, belt_z + 0.03), (body.front, 0.1 * s, 1.18), (body.front, 0.108 * s, 1.36),
                         (body.front, 0.112 * s, 1.44)):
            loc, _ = fn(a, b)
            if loc is not None:
                ctl.append(loc)
        top, _ = body.top(parts.torso_x(1.45), 0.112 * s)
        if top is not None:
            ctl.append(top)
        for a, b in ((0.1 * s, 1.44), (0.075 * s, 1.4), (0.035 * s, 1.35)):
            loc, _ = body.back(a, b)
            if loc is not None:
                ctl.append(loc)
        pts, nrm = body.hug(ctl, lift)
        strap(parts, "ShoulderStrap", pts, nrm, w * 0.8, webbing_t)
    dorsal, dn = body.back(0.0, 1.35)
    if dorsal is not None:
        ring = ka.ring("DorsalRing", 0.022, 0.0045, (0, 0, 0), axis="X", segments=20, sides=8)
        hp.place(ring, hp.frame(dorsal + dn * 0.01, dn))
        parts.add(ring, "spine_03", "Steel")
        pad = hp.bevelled_box("DorsalPad", (0.006, 0.07, 0.05), width=0.002)
        hp.place(pad, hp.frame(dorsal + dn * 0.004, dn))
        parts.add(pad, "spine_03", "Strap")
    # The waist belt, snug, with its buckle; the leg loops, snug, and their front connectors.
    pts = parts.ring((bx, 0.0, belt_z), (0, 0, 1), pad=lift, reach=0.32, torso=True, n=40)
    belt = strap(parts, "Belt", pts, [(p - Vector((bx, 0, belt_z))).normalized() for p in pts], w, 0.006, closed=True)
    belt["skin_bones"] = ["pelvis", "spine_01"]
    loc, n = body.front(0.0, belt_z)
    if loc is not None:
        hp.buckle(parts, "pelvis", hp.frame(loc + n * 0.008, n), w * 0.9)
    for side, s in (("l", 1.0), ("r", -1.0)):
        head, tail = frame_of(f"thigh_{side}")
        z = head.z - 0.13
        cx, cy = parts.limb_point(f"thigh_{side}", z, (0.006, 0.1 * s))
        axis = (tail - head).normalized()
        o = Vector((cx, cy, z))
        pts = parts.ring(o, axis, pad=lift, reach=0.16, side=s, n=32)
        nrm = [((p - o) - axis * (p - o).dot(axis)).normalized() for p in pts]
        strap(parts, "LegLoop", pts, nrm, w * 0.7, webbing_t, closed=True, bone=f"thigh_{side}", skinned=False)
        front = max(pts, key=lambda p: p.x)
        nf = ((front - o) - axis * (front - o).dot(axis)).normalized()
        hp.buckle(parts, f"thigh_{side}", hp.frame(front + nf * 0.006, nf), w * 0.6)
        top, _ = body.front(0.075 * s, belt_z - 0.02)
        if top is not None:
            p2, n2 = body.hug([top, front + Vector((0, 0, 0.02))], lift)
            strap(parts, "LegStrap", p2, n2, w * 0.55, webbing_t)
    # Each plate's corners buckled into the harness: up to the shoulder straps, down to the belt.
    for P, fn in ((chest, body.front), (back, body.back)):
        for sy in (1, -1):
            for sz in (1, -1):
                p0, n0 = P.corner(sy, sz, 0.02)
                if sz > 0:
                    target, _ = fn(0.106 * sy, min(P.zc + P.h / 2 + 0.06, 1.45))
                else:
                    target, _ = fn(0.12 * sy, belt_z + 0.01)
                if target is None:
                    continue
                edge, en = P.inner(sy * (P.w / 2 + 0.006), P.zc + sz * (P.h / 2 + 0.006))
                path, nrm = body.hug([edge.lerp(target, 0.35), target], lift)
                pts = [p0 + n0 * 0.002, edge + en * (P.thick + 0.001)] + path
                strap(parts, "PlateTab", pts, [n0, en] + nrm, 0.025, webbing_t)
                hp.buckle(parts, "spine_03", hp.frame(p0 + n0 * 0.004, n0), 0.022)
    spine_hardware(parts, body, spec, back, frame_of)
    return {"chest": chest, "back": back, "caps": caps, "body": body}


def spine_hardware(parts, body: Body, spec: dict, back: Plate, frame_of) -> None:
    """The spine frame close on the back (rails and braces, ribbons or shell ribs), the gel
    reservoirs at the kidneys and the telemetry pack under the back plate."""
    kind = spec["frame"]
    z0 = 0.98
    neck = frame_of("neck_01")[0].z
    if kind in ("rails", "short", "half"):
        top = neck - 0.01 if kind != "short" else z0 + (neck - z0) * 0.75
        ys = (0.065,) if kind == "half" else (-0.065, 0.065)
        for y in ys:
            ctl = [body.back(y, z)[0] for z in np.linspace(z0, top, 12)]
            pts, _n = body.hug([p for p in ctl if p is not None], 0.012)
            parts.add(ka.tube("Rail", [tuple(p) for p in pts], 0.0085, sides=10), "spine_02", "Frame")
        braces = (z0 + 0.06, (z0 + top) / 2, top - 0.05)[: 2 if kind == "short" else 3]
        for z in braces:
            ctl = [body.back(y, z)[0] for y in np.linspace(-0.075 if len(ys) == 2 else 0.0, 0.075, 7)]
            pts, nrm = body.hug([p for p in ctl if p is not None], 0.012)
            parts.add(ka.tube("Brace", [tuple(p) for p in pts], 0.0065, sides=8), "spine_02", "Frame")
            for y in ys:
                loc, n = body.back(y, z)
                if loc is not None:
                    b = hp.hex_bolt("FrameBolt", 0.0045)
                    hp.place(b, hp.frame(loc + n * 0.0215, n))
                    parts.add(b, "spine_02", "Accent")
        # Gel reservoirs at the kidneys, on the frame's sides.
        for y in [s * 0.115 for s in ((1,) if kind == "half" else (1, -1))]:
            loc, n = body.back(y, 1.11)
            if loc is None:
                continue
            c = loc + n * 0.031
            tank = hp.cylinder("Gel", 0.026, 0.18, segments=18, bevel=0.005)
            hp.place(tank, hp.frame(c, (0, 0, 1)))
            parts.add(tank, "spine_02", "Steel")
            for dz in (-0.095, 0.095):
                cap = hp.cylinder("GelCap", 0.028, 0.012, segments=18, bevel=0.003)
                hp.place(cap, hp.frame(c + Vector((0, 0, dz)), (0, 0, 1)))
                parts.add(cap, "spine_02", "Accent")
            glass = hp.bevelled_box("SightGlass", (0.005, 0.014, 0.12), width=0.0022)
            hp.place(glass, hp.frame(c + n * 0.025, n))
            parts.add(glass, "spine_02", "Gel")
    elif kind == "ribbons":
        for y in (-0.03, 0.03):
            ctl = [body.back(y, z)[0] for z in np.linspace(z0, neck - 0.02, 10)]
            pts, nrm = body.hug([p for p in ctl if p is not None], 0.0025)
            strap(parts, "Ribbon", pts, nrm, 0.015, 0.0025, slot="Frame")
    elif kind == "shell":
        for z in np.linspace(1.06, back.zc - back.h / 2 - 0.02, 5):
            ctl = [body.back(y, z)[0] for y in np.linspace(-0.17, 0.17, 9)]
            pts, nrm = body.hug([p for p in ctl if p is not None], 0.007)
            strap(parts, "Rib", pts, nrm, 0.035, 0.011, slot="Frame", bone="spine_02", skinned=False)
    # The telemetry pack under the back plate, and its Office seal.
    loc, n = body.back(0.0, back.zc - back.h / 2 - 0.06)
    if loc is not None:
        pack = hp.bevelled_box("Telemetry", (0.035, 0.13, 0.09), width=0.006)
        hp.place(pack, hp.frame(loc + n * 0.02, n))
        parts.add(pack, "spine_02", "Frame" if kind == "shell" else "Dark")
        seal = hp.cylinder("TelemetrySeal", 0.014, 0.004, segments=16)
        hp.place(seal, hp.frame(loc + n * 0.039, n))
        parts.add(seal, "spine_02", "Seal")
