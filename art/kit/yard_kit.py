"""The Yard's visual kit (docs/arenas/the-yard/layout.md, prop-manifest.md, lighting-and-mood.md): the
detail drawn over the grey-box level's boxes, from code.

    blender -b --factory-startup -P art/kit/yard_kit.py

It reads art/kit/yard_layout.json (written by game/Tools/unreal/build_yard_greybox.py: every box
with its role and label, every ladder) and builds, per role:
- the islands: 4 m concrete slabs with their joints and steel lip angles; and on every trench wall
  cast concrete in form panels (stepped, bowed, grout fins, plugged tie holes), spalled in places
  back to rusted rebar, pilasters every 6 m, a bolted channel rub rail, a bar-grating drain;
- the trench beds: twin rails on sleepers down each trench (the Haulers' rail bed);
- the Stacks: cut hull sections, one per 2.7 m layer, the shell plate sagged between its T-bar
  frames and seams (with their weld beads) and dented, the odd doubler plate welded on, a seam
  band between sections, a seam-welded deck and a cut-down gunwale; an inspection lamp per mesa;
- the perimeter walls in real corrugated sheet (lapped, bowed, dented low, rusted at the foot,
  screwed at the girts) on I-beam posts; the portal legs as stiffened box columns with bolted
  flanges; the crossheads as box girders; the rail walks as bar grating on edge angles with
  handrails; the masts as lattice towers with their flood heads; the press frame in I-beams; the
  Well's rusted walls; ladders as welded rungs, the tall ones caged in flat-bar hoops;
- the grate bridges on channel stringers with cross beams and open bar-grating decks; their
  landings as open steel frames on posts, braced, railed; their stairs as real steel stairs
  (channel stringers, grating treads with striped nosings, handrails), open underneath;
- the placeholders as objects: the Haul trucks (cab, glass, wheels, beacons, flatbed, ramp), the
  billet stacks on dunnage, the welding carts with their bottles, pallets, trolleys, the scaffold
  boards, the laundry line, the claim marker and its ring light, the Haul spool; and the scrap
  heaps as settled mounds of torn and folded plate, section and pipe ends, rebar, a crushed drum
  and cable on a bed of scale;
- and clutter where nobody walks: cable runs along the lips, puddles, barriers, pipe stacks and
  notices on the walls.
Each group is one mesh, art/out/kit/SM_Yard_<Group>.fbx, its material slots named after the
art/surfaces surfaces, UV'd in metres, chamfered and hard-edged as kitlib describes, with smoothing
groups. Every random choice is seeded from the piece it shapes, so a rebuild is the same. Open
things (stairs, landings, heaps, grating) stay inside the boxes they draw. art/kit/covered.json
lists the boxes the kit draws over; the level keeps them for collision and the navmesh and stops
drawing them. Boxes that move or break (the press ram, the plank bridges) are not covered.
"""
from __future__ import annotations

import bisect
import json
import math
import os
import random
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [HERE, os.path.join(HERE, "..", "lib")]

import bmesh  # noqa: E402
import bpy  # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402
from kitlib import SURFACES, Group  # noqa: E402

OUT = os.path.join(HERE, "..", "out", "kit")
TRENCHES = (-30.0, 0.0, 30.0)
NOT_COVERED = {"Press_Ram", "PlankBridge"}


def rng(*key) -> random.Random:
    return random.Random(zlib.crc32(repr(key).encode()))


def smoothstep(e0: float, e1: float, x: float) -> float:
    t = min(1.0, max(0.0, (x - e0) / (e1 - e0)))
    return t * t * (3 - 2 * t)


def waves(r: random.Random, amp: float, scale: float, n: int = 3):
    """A smooth random field over (u, v): n plane waves about `scale` metres long, within ±amp."""
    ws = []
    for _ in range(n):
        a = r.uniform(0, 2 * math.pi)
        ws.append((math.cos(a), math.sin(a), 2 * math.pi / (scale * r.uniform(0.6, 1.4)), r.uniform(0, 2 * math.pi)))
    return lambda u, v: amp * sum(math.sin(kk * (u * ca + v * sa) + p) for ca, sa, kk, p in ws) / n


def lin(a: float, b: float, step: float) -> list[float]:
    """a to b in equal steps no longer than `step`, both ends included."""
    n = max(1, math.ceil((b - a) / step - 1e-9))
    return [a + (b - a) * i / n for i in range(n + 1)]


def held(lines, step: float) -> list[float]:
    """Grid coordinates through every held line (a seam, a frame, a joint), `step` apart between."""
    lines = sorted(set(round(x, 5) for x in lines))
    out = []
    for a, b in zip(lines, lines[1:]):
        out.extend(lin(a, b, step)[:-1])
    out.append(lines[-1])
    return out


def bay(lines: list, x: float):
    """Which pair of held lines x lies between, and how far across (0 to 1)."""
    i = min(max(bisect.bisect_right(lines, x) - 1, 0), len(lines) - 2)
    a, b = lines[i], lines[i + 1]
    return i, (x - a) / (b - a) if b > a else 0.0


def face(x0, y0, x1, y1, side: str):
    """One side of a footprint as a frame, u along it (left to right seen from outside), v the
    world's Z, z out of the face; and the side's length."""
    out, origin, length = {"y0": ((0, -1, 0), (x0, y0, 0), x1 - x0), "y1": ((0, 1, 0), (x1, y1, 0), x1 - x0),
                           "x1": ((1, 0, 0), (x1, y0, 0), y1 - y0), "x0": ((-1, 0, 0), (x0, y1, 0), y1 - y0)}[side]
    o = Vector(out)
    m = Matrix((Vector((0, 0, 1)).cross(o), Vector((0, 0, 1)), o)).transposed().to_4x4()
    m.translation = origin
    return m, length


def bolt(g: Group, at, axis, size: float = 0.012, surface: str = "DarkSteel"):
    """A washer, a hex nut and the bolt's end through it, on a face at `at` looking along `axis`."""
    at, ax = Vector(at), Vector(axis).normalized()
    g.cylinder(at, at + ax * 0.003, size * 1.8, surface, segments=12, bevel=0.0)
    g.cylinder(at + ax * 0.003, at + ax * (0.003 + size), size * 1.15, surface, segments=6, bevel=0.0)
    g.cylinder(at + ax * (0.003 + size), at + ax * (0.004 + size * 1.7), size * 0.62, surface, segments=8, bevel=0.0)


def bead(g: Group, points, radius: float = 0.006, surface: str = "HullSteel"):
    """A weld bead along a polyline."""
    ring = [(math.cos(a) * radius, math.sin(a) * radius) for a in (i * math.pi / 3 for i in range(6))]
    g.sweep(points, ring, surface, soft=True)


class Kit:
    def __init__(self, layout: dict):
        self.layout = layout
        self.groups: dict[str, Group] = {}
        self.covered: set[int] = set()
        self.solids = [(b["min"], b["max"]) for b in layout["boxes"] if b["role"] not in ("floor", "trench", "ladder")]

    def g(self, name: str) -> Group:
        if name not in self.groups:
            self.groups[name] = Group(f"SM_Yard_{name}")
        return self.groups[name]

    def free(self, x0, y0, x1, y1, z0=0.0, z1=2.0) -> bool:
        """Nothing built stands in this footprint (for clutter)."""
        for (a, b) in self.solids:
            if x0 < b[0] and a[0] < x1 and y0 < b[1] and a[1] < y1 and z0 < b[2] and a[2] < z1:
                return False
        return True


# Islands, trench walls and beds ---------------------------------------------------------------------

def island(k: Kit, box: dict):
    g = k.g("Floor")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    # The island's long faces that stand over a trench or the pit (the perimeter has walls).
    trench_faces = [(y, out) for y, out in ((y0, -1.0), (y1, 1.0)) if abs(y) <= 44.9]
    # The body of the island below the slabs, set back behind the trench walls' own face.
    by0 = y0 + (0.06 if any(out < 0 for _, out in trench_faces) else 0.0)
    by1 = y1 - (0.06 if any(out > 0 for _, out in trench_faces) else 0.0)
    g.box((x0, by0, z0), (x1, by1, z1 - 0.2), "Concrete", bevel=0.0)
    # 4 m slabs, a centimetre apart, the odd one sunk or lifted a little.
    r = rng(box["label"], x0, y0)
    nx, ny = max(1, round((x1 - x0) / 4.0)), max(1, round((y1 - y0) / 4.0))
    sx, sy = (x1 - x0) / nx, (y1 - y0) / ny
    for i in range(nx):
        for j in range(ny):
            dz = r.choice((0.0, 0.0, 0.0, -0.008, 0.006))
            g.box((x0 + i * sx + 0.006, y0 + j * sy + 0.006, z1 - 0.2), (x0 + (i + 1) * sx - 0.006, y0 + (j + 1) * sy - 0.006, z1 + dz),
                  "Concrete", bevel=0.012)
    # The steel lip angle round the top edge, where the island drops away.
    for (a, b) in (((x0, y0), (x1, y0)), ((x0, y1), (x1, y1)), ((x0, y0), (x0, y1)), ((x1, y0), (x1, y1))):
        horizontal = a[1] == b[1]
        inward = (0, 1 if a[1] == y0 else -1) if horizontal else (1 if a[0] == x0 else -1, 0)
        lo = Vector((min(a[0], b[0]), min(a[1], b[1]), z1 - 0.1))
        hi = Vector((max(a[0], b[0]), max(a[1], b[1]), z1 + 0.004))
        if horizontal:
            g.box((lo.x, lo.y - 0.004, lo.z), (hi.x, hi.y + 0.004, hi.z), "DarkSteel")
            g.box((lo.x, lo.y + (0 if inward[1] > 0 else -0.1), z1), (hi.x, hi.y + (0.1 if inward[1] > 0 else 0), z1 + 0.006), "DarkSteel")
        else:
            g.box((lo.x - 0.004, lo.y, lo.z), (hi.x + 0.004, hi.y, hi.z), "DarkSteel")
            g.box((lo.x + (0 if inward[0] > 0 else -0.1), lo.y, z1), (hi.x + (0.1 if inward[0] > 0 else 0), hi.y, z1 + 0.006), "DarkSteel")
    for y, out in trench_faces:
        trench_wall(k, box, y, out)
    k.covered.add(box["_i"])


def trench_wall(k: Kit, box: dict, y: float, out: float):
    """One trench-facing side of an island: concrete cast in 1.2 m by 2.4 m form panels, each a few
    millimetres proud of or shy of the next and bowed a little, with grout fins at the joints and
    plugged tie holes; spalled here and there back to its rusted rebar (under the lip where the
    loads swing, at the foot where the Haulers' skirts ride); pilasters every 6 m, a bolted steel
    rub rail, and a drain grate along the lip."""
    walls = k.g("TrenchWalls")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    r = rng("trench wall", box["label"], y)
    m, L = face(x0, y0, x1, y1, "y0" if out < 0 else "y1")
    top, pw, lift = z1 - 0.1, 1.2, z0 + 2.4
    npan = int(L / pw) + 1
    offs = {(i, j): r.uniform(-0.004, 0.004) for i in range(npan + 1) for j in range(2)}
    bows = {(i, j): r.uniform(0.001, 0.007) for i in range(npan + 1) for j in range(2)}

    def base(u, v):
        i, j = min(int(u / pw), npan), 0 if v < lift else 1
        su = min(1.0, max(0.0, (u - i * pw) / pw))
        sv = (v - z0) / (lift - z0) if j == 0 else (v - lift) / (top - lift)
        w = offs[i, j] + bows[i, j] * math.sin(math.pi * su) * math.sin(math.pi * min(1.0, max(0.0, sv)))
        if min(abs(u - round(u / pw) * pw), abs(v - lift)) < 0.0035:
            w += 0.002  # The grout that ran out between two panels
        return w

    # The spalls: none overlapping, each in a rectangle the coarse face leaves open for it.
    spalls = []
    for _ in range(max(1, int(L / 7))):
        if r.random() < 0.25:
            continue
        uc, zone = r.uniform(1.0, L - 1.0), r.random()
        ru, rv = r.uniform(0.14, 0.42), r.uniform(0.1, 0.26)
        vc = r.uniform(top - 0.32, top - rv * 0.6) if zone < 0.5 else r.uniform(z0 + 0.3, z0 + 0.9) if zone < 0.8 else r.uniform(z0 + 1.0, top - 0.6)
        rect = (max(0.05, uc - ru * 1.5), min(L - 0.05, uc + ru * 1.5), max(z0 + 0.05, vc - rv * 1.5), min(top - 0.02, vc + rv * 1.5))
        if any(rect[0] < o[1] + 0.1 and o[0] - 0.1 < rect[1] for o in (s[0] for s in spalls)):
            continue
        spalls.append((rect, uc, vc, ru, rv, r.uniform(0.02, 0.055), waves(r, 0.28, 0.14, 4), waves(r, 0.006, 0.035, 4)))

    def crater(u, v):
        d = 0.0
        for rect, uc, vc, ru, rv, depth, rim, rough in spalls:
            if not (rect[0] <= u <= rect[1] and rect[2] <= v <= rect[3]):
                continue
            q = ((u - uc) / ru) ** 2 + ((v - vc) / rv) ** 2
            lim = (1.0 + rim(u, v)) ** 2  # A jagged rim, broken back further in places
            if q < lim:
                d = max(d, depth * (1.0 - q / lim) ** 0.35 + rough(u, v) * 0.5 + 0.003)
        return -d

    joints_u = [i * pw for i in range(npan) if 0 < i * pw < L]
    lines_u = [0.0, L] + joints_u + [j - 0.0035 for j in joints_u] + [j + 0.0035 for j in joints_u] + [c for s in spalls for c in s[0][:2]]
    lines_v = [z0, top, lift, lift - 0.0035, lift + 0.0035] + [c for s in spalls for c in s[0][2:]]
    us, vs = held(lines_u, 0.3), held(lines_v, 0.3)

    def open_for_spall(i, j):
        cu, cv = (us[i] + us[i + 1]) / 2, (vs[j] + vs[j + 1]) / 2
        return not any(s[0][0] < cu < s[0][1] and s[0][2] < cv < s[0][3] for s in spalls)

    walls.sheet(m, us, vs, "Concrete", lambda u, v: (0.0, 0.0, base(u, v)), keep=open_for_spall)
    for rect, uc, vc, ru, rv, depth, rim, rough in spalls:
        fu = held([rect[0], rect[1]] + [x for x in lines_u if rect[0] < x < rect[1]], 0.025)
        fv = held([rect[2], rect[3]] + [x for x in lines_v if rect[2] < x < rect[3]], 0.025)
        walls.sheet(m, fu, fv, "Concrete", lambda u, v: (0.0, 0.0, base(u, v) + crater(u, v)))
        if depth > 0.028:  # The cover's gone: the bars show, rusted
            vb = vc + r.uniform(-0.3, 0.3) * rv
            walls.cylinder(m @ Vector((uc - ru * 0.9, vb, -0.032)), m @ Vector((uc + ru * 0.9, vb, -0.032)), 0.008, "Rust", segments=8)
            if r.random() < 0.6:
                ub = uc + r.uniform(-0.3, 0.3) * ru
                walls.cylinder(m @ Vector((ub, vc - rv * 0.9, -0.042)), m @ Vector((ub, vc + rv * 0.9, -0.042)), 0.008, "Rust", segments=8)
    # Tie holes, plugged with dark mortar, two to a panel width on three rows and the upper lift.
    for v in (z0 + 0.3, z0 + 1.2, z0 + 2.1, (lift + top) / 2):
        for i in range(npan):
            for u in (i * pw + 0.3, i * pw + 0.9):
                if u > L - 0.2 or any(s[0][0] - 0.05 < u < s[0][1] + 0.05 and s[0][2] - 0.05 < v < s[0][3] + 0.05 for s in spalls):
                    continue
                w = base(u, v)
                walls.cylinder(m @ Vector((u, v, w - 0.004)), m @ Vector((u, v, w + 0.001)), 0.017, "Slag", segments=10, bevel=0.0)
    # Pilasters every 6 m.
    for i in range(int((x1 - x0) // 6.0) + 1):
        x = x0 + 1.5 + i * 6.0
        if x > x1 - 0.5:
            break
        walls.box((x - 0.2, y + (0 if out > 0 else -0.12), z0), (x + 0.2, y + (0.12 if out > 0 else 0), z1 - 0.12), "Concrete", bevel=0.02)
    # The rub rail: a 200 mm channel, toes to the wall, bolted on at a metre and a half under the lip.
    zc = z1 - 1.6
    channel = [(0.0, -0.1), (0.1, -0.1), (0.1, 0.1), (0.0, 0.1), (0.0, 0.088), (0.088, 0.088), (0.088, -0.088), (0.0, -0.088)]
    walls.sweep([(x0, y, zc), (x1, y, zc)], [(-out * o, z) for o, z in channel], "DarkSteel")
    x = x0 + 0.6
    while x < x1 - 0.4:
        bolt(walls, (x, y + out * 0.1, zc), (0, out, 0), size=0.016)
        x += 1.2
    # A drain grate along the lip, 45 cm in, over its dark slot.
    lo, hi = sorted((y - out * 0.75, y - out * 0.45))
    walls.box((x0 + 0.5, lo, z1), (x1 - 0.5, hi, z1 + 0.002), "Slag", bevel=0.0)
    walls.grating((x0 + 0.5, lo, z1 - 0.022), (x1 - 0.5, hi, z1 + 0.008), along=0, rod_pitch=0.1, panel=1.0)


def trench_bed(k: Kit, box: dict):
    g = k.g("TrenchBeds")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z0), (x1, y1, z1), "Slag")
    for yc in TRENCHES:
        if not (y0 < yc < y1):
            continue
        # Sleepers every 70 cm, the twin rails 1.5 m apart on them.
        x = x0 + 0.35
        r = rng("sleepers", yc, x0)
        while x < x1:
            jitter = r.uniform(-0.03, 0.03)
            g.box((x - 0.11 + jitter, yc - 1.2, z1), (x + 0.11 + jitter, yc + 1.2, z1 + 0.14), "Timber", bevel=0.01)
            x += 0.7
        for side in (-0.75, 0.75):
            g.rail((x0, yc + side, z1 + 0.14), (x1, yc + side, z1 + 0.14))
    k.covered.add(box["_i"])


# The Stacks --------------------------------------------------------------------------------------

def mesa(k: Kit, box: dict):
    g = k.g("Stacks")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    r = rng(box["label"], x0, y0, z1)
    layers = max(1, round((z1 - z0) / 2.7))
    h = (z1 - z0) / layers
    for li in range(layers):
        za, zb = z0 + li * h, z0 + (li + 1) * h
        inset = r.uniform(0.0, 0.06)  # Each cut section a little out of true with the next
        bx0, by0, bx1, by1 = x0 + inset, y0 + inset, x1 - inset, y1 - inset
        g.box((bx0 + 0.05, by0 + 0.05, za), (bx1 - 0.05, by1 - 0.05, zb - 0.06), "HullSteel", bevel=0.0)  # Behind the plate
        # The seam band where two sections meet.
        g.box((x0 - 0.03, y0 - 0.03, zb - 0.08), (x1 + 0.03, y1 + 0.03, zb), "DarkSteel", bevel=0.01)
        pitch = r.choice((0.8, 0.9, 1.0, 1.2))  # This section's own frame spacing
        for side in ("y0", "x1", "y1", "x0"):
            m, L = face(bx0, by0, bx1, by1, side)
            hull_side(g, r, m, L, za, zb - 0.08, pitch)
        for (cx, cy) in ((bx0, by0), (bx1, by0), (bx1, by1), (bx0, by1)):
            bead(g, [(cx, cy, za), (cx, cy, zb - 0.08)], 0.01)  # The corner weld
    # The deck on top, its plates in their own weld lines, and a cut-down gunwale at the edge.
    g.box((x0 + 0.05, y0 + 0.05, z1 - 0.02), (x1 - 0.05, y1 - 0.05, z1 + 0.01), "HullSteel")
    yy = y0 + r.uniform(1.6, 2.4)
    while yy < y1 - 0.5:
        bead(g, [(x0 + 0.1, yy, z1 + 0.005), (x1 - 0.1, yy, z1 + 0.005)], 0.008)
        yy += r.uniform(1.8, 2.4)
    xx = x0 + r.uniform(3.0, 6.0)
    while xx < x1 - 0.5:
        bead(g, [(xx, y0 + 0.1, z1 + 0.005), (xx, y1 - 0.1, z1 + 0.005)], 0.008)
        xx += r.uniform(5.0, 8.0)
    for (a, b) in (((x0, y0), (x1, y0 + 0.08)), ((x0, y1 - 0.08), (x1, y1)), ((x0, y0), (x0 + 0.08, y1)), ((x1 - 0.08, y0), (x1, y1))):
        g.box((a[0], a[1], z1), (b[0], b[1], z1 + 0.1), "DarkSteel", bevel=0.01)
    if box["role"] == "mesa":
        # A chalk-mark inspection lamp clipped to the face (lighting-and-mood: one per mesa).
        lamp = k.g("Lamps")
        lamp.box((x0 - 0.18, (y0 + y1) / 2 - 0.12, z1 - 1.2), (x0 - 0.02, (y0 + y1) / 2 + 0.12, z1 - 0.95), "DarkSteel", bevel=0.01)
        lamp.box((x0 - 0.19, (y0 + y1) / 2 - 0.09, z1 - 1.17), (x0 - 0.17, (y0 + y1) / 2 + 0.09, z1 - 0.98), "Lamp")
    k.covered.add(box["_i"])


def hull_side(g: Group, r: random.Random, m: Matrix, L: float, va: float, vb: float, pitch: float):
    """One side of a cut hull section, in the face frame m (u along, v up, z out): the shell plate
    sagged in between its frames and seams the way an old hull is ("hungry horse"), with a dent or
    two; T-bar frames at the section's pitch with their fillet welds; butt and strake seams with
    their weld caps; and now and then a doubler plate welded over a thin spot."""
    frames = []
    t = pitch / 2
    while t < L - 0.2:
        if r.random() >= 0.08:  # A frame cut away
            frames.append(t)
        t += pitch
    butts = []
    t = r.uniform(2.0, 6.0)
    while t < L - 1.0:
        i = bisect.bisect_left(frames, t)
        if 0 < i < len(frames):
            t = (frames[i - 1] + frames[i]) / 2  # Butts fall between frames
        butts.append(t)
        t += r.uniform(3.5, 8.0)
    strakes = [r.uniform(va + 0.9, vb - 0.9)] if vb - va > 2.0 and r.random() < 0.55 else []
    hu = sorted(set(round(x, 5) for x in [0.0, L] + frames + butts))
    hv = [va] + strakes + [vb]
    sag = {(i, j): r.uniform(0.008, 0.028) for i in range(len(hu)) for j in range(len(hv))}
    dents = []
    for _ in range(r.choice((0, 0, 1, 1, 2))):
        i, j = r.randrange(len(hu) - 1), r.randrange(len(hv) - 1)
        rad = min(hu[i + 1] - hu[i], hv[j + 1] - hv[j]) * r.uniform(0.3, 0.5)
        if rad > 0.12:
            dents.append((r.uniform(hu[i] + rad, hu[i + 1] - rad), r.uniform(hv[j] + rad, hv[j + 1] - rad), rad, r.uniform(0.015, 0.045)))

    def plate(u, v):
        i, s = bay(hu, u)
        j, t_ = bay(hv, v)
        window = max(0.0, math.sin(math.pi * s)) * max(0.0, math.sin(math.pi * t_)) ** 0.6
        w = -sag[i, j] * window
        for (uc, vc, rad, depth) in dents:
            q = ((u - uc) ** 2 + (v - vc) ** 2) / (rad * rad)
            if q < 4.0:
                w -= depth * math.exp(-1.5 * q) * math.sqrt(max(0.0, window))
        return w

    g.sheet(m, held(hu, 0.2), held(hv, 0.2), "HullSteel", lambda u, v: (0.0, 0.0, plate(u, v)))
    # The frames: a 120 mm web and an 80 mm face flat, fillet-welded both sides.
    for t in frames:
        g.box((t - 0.006, va + 0.1, 0.0), (t + 0.006, vb - 0.1, 0.11), "HullSteel", matrix=m)
        g.box((t - 0.04, va + 0.13, 0.11), (t + 0.04, vb - 0.13, 0.124), "HullSteel", matrix=m)
        for d in (-0.008, 0.008):
            bead(g, [m @ Vector((t + d, va + 0.1, 0.0)), m @ Vector((t + d, vb - 0.1, 0.0))], 0.005)
    # Butt and strake seams: the weld cap stands 3 mm proud.
    for t in butts:
        bead(g, [m @ Vector((t, va, -0.005)), m @ Vector((t, vb, -0.005))], 0.008)
    for v in strakes:
        bead(g, [m @ Vector((0.0, v, -0.005)), m @ Vector((L, v, -0.005))], 0.008)
    # A doubler plate welded over a thin spot, inside one bay.
    if r.random() < 0.35:
        i = r.randrange(len(hu) - 1)
        a, b = hu[i] + 0.05, hu[i + 1] - 0.05
        hh = r.uniform(0.4, min(1.1, vb - va - 0.4))
        c = r.uniform(va + 0.15, vb - 0.15 - hh)
        d = c + hh
        if b - a > 0.3:
            g.sheet(m, lin(a, b, 0.1), lin(c, d, 0.1), r.choice(("Rust", "HullSteel", "HullSteel")),
                    lambda u, v: (0.0, 0.0, plate(u, v) + 0.01), thickness=0.01)
            for path in ([(u, c) for u in lin(a, b, 0.1)], [(u, d) for u in lin(a, b, 0.1)],
                         [(a, v) for v in lin(c, d, 0.1)], [(b, v) for v in lin(c, d, 0.1)]):
                bead(g, [m @ Vector((u, v, plate(u, v) + 0.002)) for u, v in path], 0.005)


# Steel ---------------------------------------------------------------------------------------------

def wall(k: Kit, box: dict):
    g = k.g("Walls")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    r = rng("wall", x0, y0)
    long_x = (x1 - x0) > (y1 - y0)
    # The girts' line behind the cladding, set back from both faces.
    if long_x:
        g.box((x0, y0 + 0.03, z0 + 0.3), (x1, y1 - 0.03, z1), "Corrugated", bevel=0.0)
    else:
        g.box((x0 + 0.03, y0, z0 + 0.3), (x1 - 0.03, y1, z1), "Corrugated", bevel=0.0)
    g.box((x0 - 0.05, y0 - 0.05, z0), (x1 + 0.05, y1 + 0.05, z0 + 0.3), "Concrete", bevel=0.02)  # The kicker
    centre = Vector(((x0 + x1) / 2, (y0 + y1) / 2, 0))
    for side in (("y0", "y1") if long_x else ("x0", "x1")):
        m, L = face(x0, y0, x1, y1, side)
        inner = (m.col[2].xyz).dot(-centre) > 0  # The face the Yard sees
        cladding(g, r, m, L, z0 + 0.3, z1, inner)
    n = max(1, round((x1 - x0 if long_x else y1 - y0) / 3.0))
    for i in range(n + 1):
        t = i / n
        if long_x:
            x = x0 + (x1 - x0) * t
            for y in (y0 - 0.12, y1 + 0.12):
                g.beam((x, y, z0), (x, y, z1 + 0.15), 0.2, 0.12, "DarkSteel", up=(1, 0, 0))
        else:
            y = y0 + (y1 - y0) * t
            for x in (x0 - 0.12, x1 + 0.12):
                g.beam((x, y, z0), (x, y, z1 + 0.15), 0.2, 0.12, "DarkSteel", up=(0, 1, 0))
    g.box((x0 - 0.15, y0 - 0.15, z1), (x1 + 0.15, y1 + 0.15, z1 + 0.12), "DarkSteel", bevel=0.01)  # Cap rail
    k.covered.add(box["_i"])


def cladding(g: Group, r: random.Random, m: Matrix, L: float, za: float, zb: float, inner: bool):
    """Corrugated sheet over one face (frame m: u along, v up, z out): 76 mm pitch, 18 mm deep,
    sheets 762 mm cover lapping one corrugation (every other sheet over its neighbour), each bowed a
    little between the girts, kicked out at the foot, the odd one rusted through there; forklift
    dents low down; and on the Yard side the fixings, a self-drilling screw in every third valley at
    each girt."""
    pitch, depth = 0.0762, 0.018
    cover = 10 * pitch
    if inner:
        rows = [za, za + 0.12, za + 0.3, za + 0.6, za + 0.9, za + 1.2, za + 1.55, za + 2.0, za + 2.6, za + 3.1, zb]
    else:
        rows = [za, za + 0.12, (za + zb) / 2, zb]
    rows = [v for v in rows if v <= zb]
    girts = [za + 0.15, za + 1.2, za + 2.4, zb - 0.15]
    dents = []
    t = r.uniform(2.0, 8.0)
    while inner and t < L - 0.5:
        dents.append((t, r.uniform(za + 0.2, za + 1.3), r.uniform(0.18, 0.5), r.uniform(0.015, 0.05)))
        t += r.uniform(4.0, 14.0)
    n = math.ceil(L / cover - 1e-6)
    for s in range(n):
        u0, u1 = s * cover, min(L, s * cover + cover + pitch)
        lap = 0.004 * (s % 2)
        bow = r.uniform(0.0, 0.006)
        kick = r.uniform(0.002, 0.01)
        rusty = r.random()
        mine = [d for d in dents if u0 - 0.6 < d[0] < u1 + 0.6]

        def shape(u, v, lap=lap, bow=bow, kick=kick, mine=mine):
            w = depth * (0.5 - 0.5 * math.cos(2 * math.pi * u / pitch)) + lap
            gi = max(0, bisect.bisect_right(girts, v) - 1)
            if gi < len(girts) - 1:  # Bowed between girts
                w += bow * math.sin(math.pi * (v - girts[gi]) / (girts[gi + 1] - girts[gi]))
            if v < za + 0.12:
                w += kick * (1 - (v - za) / 0.12)
            for (uc, vc, rad, dd) in mine:
                q = ((u - uc) ** 2 + (v - vc) ** 2) / (rad * rad)
                if q < 4.0:
                    w -= dd * math.exp(-2.0 * q)
            return 0.0, 0.0, w

        us = [u0 + i * pitch / 4 for i in range(int((u1 - u0) / (pitch / 4) + 1e-6) + 1)]
        if us[-1] < u1 - 1e-4:
            us.append(u1)
        foot = "Rust" if rusty < 0.25 else None
        whole = "Rust" if rusty > 0.99 else None
        g.sheet(m, us, rows, "Corrugated", shape, cell_surface=lambda i, j, foot=foot, whole=whole: whole or (foot if j == 0 else None))
        if inner:
            for gz in girts:
                c = 1
                while u0 + c * pitch < u1 - 0.02:
                    u = u0 + c * pitch
                    w = shape(u, gz)[2]
                    g.cylinder(m @ Vector((u, gz, w)), m @ Vector((u, gz, w + 0.007)), 0.0075, "DarkSteel", segments=6, bevel=0.0)
                    c += 3


def column(k: Kit, box: dict, surface: str = "HullSteel"):
    """A box column: corner angles, stiffener bands every 3 m, flanged top and foot, bolt rows."""
    g = k.g("Rails")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z0), (x1, y1, z1), surface)
    for (cx, cy) in ((x0, y0), (x0, y1), (x1, y0), (x1, y1)):
        g.box((cx - 0.05, cy - 0.05, z0), (cx + 0.05, cy + 0.05, z1), "DarkSteel", bevel=0.008)
    z = z0 + 3.0
    while z < z1 - 0.5:
        g.box((x0 - 0.03, y0 - 0.03, z - 0.06), (x1 + 0.03, y1 + 0.03, z + 0.06), "DarkSteel", bevel=0.008)
        z += 3.0
    for zz in (z0, z1 - 0.06):
        g.box((x0 - 0.1, y0 - 0.1, zz), (x1 + 0.1, y1 + 0.1, zz + 0.06), "DarkSteel", bevel=0.01)
    k.covered.add(box["_i"])


def girder(k: Kit, box: dict):
    """A box girder or beam: top and bottom flanges proud, web stiffeners every metre."""
    g = k.g("Rails")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z0 + 0.04), (x1, y1, z1 - 0.04), "HullSteel")
    for zz in (z0, z1 - 0.04):
        g.box((x0 - 0.03, y0 - 0.03, zz), (x1 + 0.03, y1 + 0.03, zz + 0.04), "DarkSteel", bevel=0.006)
    long_x = (x1 - x0) >= (y1 - y0)
    span = (x0, x1) if long_x else (y0, y1)
    t = span[0] + 0.5
    while t < span[1] - 0.3:
        if long_x:
            for y, d in ((y0, -1), (y1, 1)):
                g.box((t - 0.01, min(y, y + d * 0.05), z0 + 0.04), (t + 0.01, max(y, y + d * 0.05), z1 - 0.04), "DarkSteel")
        else:
            for x, d in ((x0, -1), (x1, 1)):
                g.box((min(x, x + d * 0.05), t - 0.01, z0 + 0.04), (max(x, x + d * 0.05), t + 0.01, z1 - 0.04), "DarkSteel")
        t += 1.0
    k.covered.add(box["_i"])


def walkway(k: Kit, box: dict, group: str = "Rails"):
    """A bar-grating deck on its edge angles."""
    g = k.g(group)
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    long_x = (x1 - x0) >= (y1 - y0)
    g.grating((x0, y0, z1 - 0.03), (x1, y1, z1), along=0 if long_x else 1, panel=1.0)
    for y in ((y0, y1) if long_x else ()):
        g.box((x0, y - 0.04 if y == y0 else y - 0.005, z1 - 0.18), (x1, y + 0.005 if y == y0 else y + 0.04, z1 - 0.03), "DarkSteel")
    for x in ((x0, x1) if not long_x else ()):
        g.box((x - 0.04 if x == x0 else x - 0.005, y0, z1 - 0.18), (x + 0.005 if x == x0 else x + 0.04, y1, z1 - 0.03), "DarkSteel")
    k.covered.add(box["_i"])


def handrail(k: Kit, box: dict, group: str = "Rails"):
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    long_x = (x1 - x0) >= (y1 - y0)
    yc, xc = (y0 + y1) / 2, (x0 + x1) / 2
    a, b = ((x0, yc, z0 - 0.94), (x1, yc, z0 - 0.94)) if long_x else ((xc, y0, z0 - 0.94), (xc, y1, z0 - 0.94))
    k.g(group).railing(a, b, 1.0, "Ochre", posts_every=1.6)
    k.covered.add(box["_i"])


def mast(k: Kit, box: dict):
    """A floodlight mast: a lattice tower of four corner angles, rungs of bracing and X braces,
    and three flood heads on the platform aimed inboard."""
    g = k.g("Masts")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    hw = (x1 - x0) / 2
    corners = [(cx - hw, cy - hw), (cx + hw, cy - hw), (cx + hw, cy + hw), (cx - hw, cy + hw)]
    for (px, py) in corners:
        g.box((px - 0.05, py - 0.05, z0), (px + 0.05, py + 0.05, z1), "DarkSteel", bevel=0.006)
    z = z0 + 1.5
    while z < z1:
        for i in range(4):
            a, b = corners[i], corners[(i + 1) % 4]
            g.cylinder((a[0], a[1], z), (b[0], b[1], z), 0.02, "DarkSteel", segments=6)
            g.cylinder((a[0], a[1], z - 1.5), (b[0], b[1], z), 0.014, "DarkSteel", segments=6)
            g.cylinder((b[0], b[1], z - 1.5), (a[0], a[1], z), 0.014, "DarkSteel", segments=6)
        z += 1.5
    # The platform's railing, round its 4 m square.
    top = 24.0
    for (a, b) in (((cx - 2, cy - 2), (cx + 2, cy - 2)), ((cx + 2, cy - 2), (cx + 2, cy + 2)), ((cx + 2, cy + 2), (cx - 2, cy + 2)),
                   ((cx - 2, cy + 2), (cx - 2, cy - 2))):
        g.railing((a[0], a[1], top), (b[0], b[1], top), 1.05, "Ochre", posts_every=1.0)
    # Three flood heads, each a housing and its lens, facing the Yard.
    toward = Vector((-cx, -cy, 0)).normalized()
    side = Vector((-toward.y, toward.x, 0))
    for k_ in (-1, 0, 1):
        base = Vector((cx, cy, top + 1.2)) + toward * 1.4 + side * (k_ * 1.0)
        m = Matrix.Translation(base) @ (toward.to_track_quat("X", "Z").to_matrix().to_4x4()) @ Matrix.Rotation(math.radians(35), 4, "Y")
        g.box((-0.25, -0.35, -0.3), (0.25, 0.35, 0.3), "DarkSteel", bevel=0.03, matrix=m)
        k.g("Lamps").box((0.25, -0.3, -0.25), (0.27, 0.3, 0.25), "Lamp", matrix=m)
        g.cylinder(base - Vector((0, 0, 0.3)), base - Vector((0, 0, 1.2)), 0.04, "DarkSteel", segments=8)
    k.covered.add(box["_i"])


def press_piece(k: Kit, box: dict):
    g = k.g("Press")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    label = box["label"]
    if label == "Press_Leg":
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        g.beam((cx, cy, z0), (cx, cy, z1), 0.9, 0.8, "DarkSteel", flange=0.06, web=0.05, up=(1, 0, 0))
        g.box((x0 - 0.2, y0 - 0.2, z0), (x1 + 0.2, y1 + 0.2, z0 + 0.08), "DarkSteel", bevel=0.01)
        for zz in (3.0, 6.0, 9.0):
            g.box((x0 - 0.05, y0 - 0.05, zz), (x1 + 0.05, y1 + 0.05, zz + 0.05), "Hazard")
    elif label in ("Press_Beam", "Press_CounterweightArm"):
        long_x = (x1 - x0) >= (y1 - y0)
        yc, xc, zc = (y0 + y1) / 2, (x0 + x1) / 2, (z0 + z1) / 2
        a, b = ((x0, yc, zc), (x1, yc, zc)) if long_x else ((xc, y0, zc), (xc, y1, zc))
        g.beam(a, b, z1 - z0, min(x1 - x0, y1 - y0), "DarkSteel", flange=0.05, web=0.04)
    elif label == "Press_Crosshead":
        g.box((x0, y0, z0), (x1, y1, z1), "DarkSteel", bevel=0.04)
        # The hydraulic cylinders over the ram, rods down.
        for (cx, cy) in ((40.5, -3.0), (40.5, 3.0), (43.0, -3.0), (43.0, 3.0)):
            g.cylinder((cx, cy, z1), (cx, cy, z1 + 2.2), 0.35, "DarkSteel", segments=20)
            g.cylinder((cx, cy, z1 + 2.2), (cx, cy, z1 + 2.35), 0.4, "Hazard", segments=20)
            g.cylinder((cx, cy, z0), (cx, cy, z0 - 1.5), 0.12, "DarkSteel", segments=12)
    else:
        g.box((x0, y0, z0), (x1, y1, z1), "DarkSteel", bevel=0.02)
    k.covered.add(box["_i"])


def pit_piece(k: Kit, box: dict):
    g = k.g("Pit")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z0), (x1, y1, z1), "Rust")
    if box["label"] != "Pit_Floor":
        long_x = (x1 - x0) >= (y1 - y0)
        t = (x0 if long_x else y0) + 0.6
        end = x1 if long_x else y1
        while t < end - 0.3:
            if long_x:
                g.box((t - 0.05, y0 - 0.08, z0), (t + 0.05, y1 + 0.08, z1), "DarkSteel")
            else:
                g.box((x0 - 0.08, t - 0.05, z0), (x1 + 0.08, t + 0.05, z1), "DarkSteel")
            t += 1.2
        # The ram lamps in the pit walls at -5 m (lighting-and-mood).
        if long_x:
            k.g("Lamps").box(((x0 + x1) / 2 - 0.3, y0 - 0.1, -5.1), ((x0 + x1) / 2 + 0.3, y1 + 0.1, -4.9), "Lamp")
    k.covered.add(box["_i"])


def pit_rail(k: Kit, box: dict):
    g = k.g("Press")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    if box["label"] == "PitLipRail_Post":
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        g.cylinder((cx, cy, z0), (cx, cy, z1), 0.03, "Hazard", segments=8)
    else:
        g.cylinder((x0, (y0 + y1) / 2, (z0 + z1) / 2), (x1, (y0 + y1) / 2, (z0 + z1) / 2), 0.03, "Hazard", segments=8)
        g.cylinder((x0, (y0 + y1) / 2, 0.55), (x1, (y0 + y1) / 2, 0.55), 0.022, "Hazard", segments=8)
    k.covered.add(box["_i"])


# Bridges -------------------------------------------------------------------------------------------

def grate_deck(k: Kit, box: dict):
    """A footbridge deck: two 300 mm channel stringers, toes out to the deck's edges, cross beams
    every metre bolted through their webs, open bar-grating panels between, and kick plates."""
    g = k.g("Bridges")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    for xo, xi in ((x0, x0 + 0.09), (x1, x1 - 0.09)):
        lo, hi = min(xo, xi), max(xo, xi)
        wlo, whi = (xi - 0.01, xi) if xi > xo else (xi, xi + 0.01)
        g.box((wlo, y0, z1 - 0.3), (whi, y1, z1), "DarkSteel")              # Web
        g.box((lo, y0, z1 - 0.015), (hi, y1, z1), "DarkSteel")              # Top flange
        g.box((lo, y0, z1 - 0.3), (hi, y1, z1 - 0.285), "DarkSteel")        # Bottom flange
    y = y0 + 0.5
    while y < y1:
        g.beam((x0 + 0.09, y, z1 - 0.085), (x1 - 0.09, y, z1 - 0.085), 0.11, 0.055, "DarkSteel", flange=0.008, web=0.006)
        for x, d in ((x0 + 0.08, -1), (x1 - 0.08, 1)):
            for dz in (-0.05, -0.12):
                bolt(g, (x, y, z1 + dz), (d, 0, 0))
        y += 1.0
    g.grating((x0 + 0.09, y0, z1 - 0.03), (x1 - 0.09, y1, z1), along=1, panel=1.0)
    for x in (x0 + 0.09, x1 - 0.096):
        g.box((x, y0, z1), (x + 0.006, y1, z1 + 0.1), "DarkSteel")  # Kick plate
    k.covered.add(box["_i"])


def grate_rail(k: Kit, box: dict):
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    xc = (x0 + x1) / 2
    k.g("Bridges").railing((xc, y0, z0), (xc, y1, z0), z1 - z0, "Ochre", posts_every=1.33)
    k.covered.add(box["_i"])


def abuts(box: dict, other: dict, side: str) -> bool:
    """`other` stands against this side of `box` (within a centimetre, overlapping along it)."""
    (ax0, ay0, _), (ax1, ay1, _) = box["min"], box["max"]
    (bx0, by0, _), (bx1, by1, _) = other["min"], other["max"]
    if side in ("x0", "x1"):
        edge = abs(bx1 - ax0) < 0.01 if side == "x0" else abs(bx0 - ax1) < 0.01
        return edge and min(ay1, by1) - max(ay0, by0) > 0.05
    edge = abs(by1 - ay0) < 0.01 if side == "y0" else abs(by0 - ay1) < 0.01
    return edge and min(ax1, bx1) - max(ax0, bx0) > 0.05


def grate_landing(k: Kit, box: dict):
    """A stair-head landing, open underneath (the collision under it is solid, as it was): a
    channel frame on four square hollow posts with base plates and anchor bolts, X-braced in angle
    where nothing meets it, a bar-grating deck, handrails with kick plates on its open sides, and
    the amber lamp at the stair head."""
    g = k.g("Bridges")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    others = [b for b in k.layout["boxes"] if b["role"] == "bridge" and b["label"] in ("Grate_Step", "Grate_Deck")]
    joined = {s: [b["label"] for b in others if abuts(box, b, s)] for s in ("x0", "x1", "y0", "y1")}
    edges = {"y0": ((x0, y0), (x1, y0)), "y1": ((x1, y1), (x0, y1)), "x0": ((x0, y1), (x0, y0)), "x1": ((x1, y0), (x1, y1))}
    # The frame: a 150 mm channel round the edge, toes in.
    for side, ((ax, ay), (bx, by)) in edges.items():
        lo_x, hi_x = min(ax, bx), max(ax, bx)
        lo_y, hi_y = min(ay, by), max(ay, by)
        if side in ("y0", "y1"):
            inn = 1 if side == "y0" else -1
            web = sorted((ay, ay + inn * 0.007))
            fl = sorted((ay, ay + inn * 0.065))
            g.box((lo_x, web[0], z1 - 0.15), (hi_x, web[1], z1), "DarkSteel")
            g.box((lo_x, fl[0], z1 - 0.011), (hi_x, fl[1], z1), "DarkSteel")
            g.box((lo_x, fl[0], z1 - 0.15), (hi_x, fl[1], z1 - 0.139), "DarkSteel")
        else:
            inn = 1 if side == "x0" else -1
            web = sorted((ax, ax + inn * 0.007))
            fl = sorted((ax, ax + inn * 0.065))
            g.box((web[0], lo_y, z1 - 0.15), (web[1], hi_y, z1), "DarkSteel")
            g.box((fl[0], lo_y, z1 - 0.011), (fl[1], hi_y, z1), "DarkSteel")
            g.box((fl[0], lo_y, z1 - 0.15), (fl[1], hi_y, z1 - 0.139), "DarkSteel")
    g.grating((x0 + 0.007, y0 + 0.007, z1 - 0.03), (x1 - 0.007, y1 - 0.007, z1), along=0 if (x1 - x0) < (y1 - y0) else 1,
              panel=max(x1 - x0, y1 - y0))
    # Posts, 100 mm square hollow, on base plates.
    posts = [(x0 + 0.06, y0 + 0.06), (x1 - 0.06, y0 + 0.06), (x1 - 0.06, y1 - 0.06), (x0 + 0.06, y1 - 0.06)]
    for (px, py) in posts:
        g.tube([(px, py, 0.016), (px, py, z1 - 0.15)], 0.05 * math.sqrt(2), 0.006, "DarkSteel", segments=4, phase=math.pi / 4)
        g.box((px - 0.11, py - 0.11, 0.0), (px + 0.11, py + 0.11, 0.016), "DarkSteel")
        for dx in (-0.075, 0.075):
            for dy in (-0.075, 0.075):
                bolt(g, (px + dx, py + dy, 0.016), (0, 0, 1), size=0.014)
    # X-bracing in 60 mm angle on the sides nothing stands against.
    angle = [(0.0, 0.0), (0.06, 0.0), (0.06, 0.006), (0.006, 0.006), (0.006, 0.06), (0.0, 0.06)]
    pairs = {"y0": (0, 1), "x1": (1, 2), "y1": (2, 3), "x0": (3, 0)}
    outward = {"y0": Vector((0, -1, 0)), "y1": Vector((0, 1, 0)), "x0": Vector((-1, 0, 0)), "x1": Vector((1, 0, 0))}
    for side, (i, j) in pairs.items():
        if joined[side]:
            continue
        (ax, ay), (bx, by) = posts[i], posts[j]
        o = outward[side] * 0.055
        for k_, (za_, zb_) in enumerate(((0.35, z1 - 0.3), (z1 - 0.3, 0.35))):
            off = o + outward[side] * 0.007 * k_
            a3, b3 = Vector((ax, ay, za_)) + off, Vector((bx, by, zb_)) + off
            g.sweep([a3, b3], angle, "DarkSteel", up=outward[side])
    # Handrails on the open sides.
    for side, ((ax, ay), (bx, by)) in edges.items():
        if joined[side]:
            continue
        inn = -outward[side] * 0.03
        a3, b3 = Vector((ax, ay, z1)) + inn, Vector((bx, by, z1)) + inn
        g.railing(a3, b3, 1.1, "Ochre", posts_every=1.2, toe=True)
    # The amber lamp at the stair head, on a stub post over the handrail's end.
    for side, labels in joined.items():
        if "Grate_Step" in labels:
            (ax, ay), (bx, by) = edges[side]
            along = (Vector((bx, by, 0)) - Vector((ax, ay, 0))).normalized()
            p = Vector((ax, ay, z1 + 1.12)) + along * 0.03 - outward[side] * 0.03
            g.cylinder(p, p + Vector((0, 0, 0.3)), 0.024, "Ochre", segments=12)
            top = p + Vector((0, 0, 0.3))
            lamps = k.g("Lamps")
            lamps.box(top + Vector((-0.08, -0.08, 0.0)), top + Vector((0.08, 0.08, 0.15)), "DarkSteel", bevel=0.02)
            lens = top + outward[side] * 0.082 + Vector((0, 0, 0.075))
            half = Vector((0.05, 0.05, 0.05)) - Vector([abs(c) for c in outward[side]]) * 0.046
            lamps.box(lens - half, lens + half, "Lamp", bevel=0.0)
    k.covered.add(box["_i"])


def clip_above(poly, z: float = 0.0):
    """A polygon [(s, z), ...] cut off below z."""
    out = []
    for p, q in zip(poly, poly[1:] + poly[:1]):
        if p[1] >= z:
            out.append(p)
        if (p[1] >= z) != (q[1] >= z):
            t = (z - p[1]) / (q[1] - p[1])
            out.append((p[0] + t * (q[0] - p[0]), z))
    return out


def stairs(k: Kit, steps: list):
    """The footbridges' stair flights, drawn open over their stepped collision as real steel stairs.
    The steps come as boxes; each run of boxes side by side is a flight."""
    g = k.g("Bridges")
    runs = {}
    for b in steps:
        (x0, y0, _), (x1, y1, _) = b["min"], b["max"]
        along_x = (x1 - x0) < (y1 - y0)
        key = (along_x, round(y0, 3), round(y1, 3)) if along_x else (along_x, round(x0, 3), round(x1, 3))
        runs.setdefault(key, []).append(b)
    for (along_x, c0, c1), bs in runs.items():
        a = 0 if along_x else 1
        bs.sort(key=lambda b: b["min"][a])
        flight = [bs[0]]
        for b in bs[1:]:
            if abs(b["min"][a] - flight[-1]["max"][a]) < 0.01:
                flight.append(b)
            else:
                stair_flight(g, flight, a, c0, c1)
                flight = [b]
        stair_flight(g, flight, a, c0, c1)
        for b in bs:
            k.covered.add(b["_i"])


def stair_flight(g: Group, flight: list, a: int, c0: float, c1: float):
    """One flight: two 250 mm channel stringers on base plates, toes out, their tops 7 cm over the
    nosing line; a bar-grating tread at each step on bolted carrier plates, its nosing a striped
    angle; and a handrail with a mid rail each side, posts bolted into the stringers. Built in the
    flight's own frame: s down the run from the top edge, c across, z up."""
    sign = 1.0
    if flight[0]["max"][2] < flight[-1]["max"][2]:
        flight, sign = flight[::-1], -1.0
    s_top = flight[0]["min"][a] if sign > 0 else flight[0]["max"][a]
    run = flight[0]["max"][a] - flight[0]["min"][a]
    tops = [b["max"][2] for b in flight]
    rise = tops[0] - tops[1] if len(tops) > 1 else tops[0]
    zl = tops[0] + rise
    es = Vector((sign, 0, 0)) if a == 0 else Vector((0, sign, 0))
    ec = Vector((0, 0, 1)).cross(es)
    cmin = c0 if (ec.y if a == 0 else ec.x) > 0 else c1
    M = Matrix((es, ec, Vector((0, 0, 1)))).transposed().to_4x4()
    M.translation = Vector((s_top, cmin, 0)) if a == 0 else Vector((cmin, s_top, 0))
    W = c1 - c0
    slope = rise / run
    cos_a = 1.0 / math.sqrt(1.0 + slope * slope)
    s_end = len(flight) * run
    D, tf, tw, fw = 0.25, 0.012, 0.009, 0.089
    zt = lambda s: zl - slope * s + 0.07          # noqa: E731  The stringer's top edge
    zb = lambda s: zt(s) - D / cos_a              # noqa: E731  ... and its bottom edge
    s_toe = s_end + 0.02

    def band(lo: float, hi: float):
        """The stringer's side outline between lo and hi above its bottom edge, cut level at the floor."""
        return clip_above([(0.0, zb(0.0) + lo), (s_toe, zb(s_toe) + lo), (s_toe, zb(s_toe) + hi), (0.0, zb(0.0) + hi)])

    lc = lambda s, c, z: M @ Vector((s, c, z))  # noqa: E731
    across = M.to_3x3() @ Vector((0, 1, 0))
    for side in (0, 1):
        c_web = fw - tw if side == 0 else W - fw
        c_fl = 0.0 if side == 0 else W - fw
        g.prism(band(0.0, D / cos_a), M @ Matrix.Translation((0, c_web, 0)), tw, "DarkSteel")
        g.prism(band(D / cos_a - tf / cos_a, D / cos_a), M @ Matrix.Translation((0, c_fl, 0)), fw, "DarkSteel")
        g.prism(band(0.0, tf / cos_a), M @ Matrix.Translation((0, c_fl, 0)), fw, "DarkSteel")
        # The foot: a base plate and two anchors; the head: two bolts into the landing frame.
        p_lo, p_hi = (0.0, fw + 0.05) if side == 0 else (W - fw - 0.05, W)
        g.box((s_end - 0.16, p_lo, 0.0), (s_toe + 0.03, p_hi, 0.014), "DarkSteel", matrix=M)
        for sb in (s_end - 0.1, s_toe - 0.01):
            bolt(g, lc(sb, fw + 0.025 if side == 0 else W - fw - 0.025, 0.014), (0, 0, 1), size=0.012)
        out_c = -1.0 if side == 0 else 1.0
        for dz in (0.08, 0.17):
            bolt(g, lc(0.06, c_web if side == 0 else c_web + tw, zt(0.06) - dz), across * out_c)
    along_c = 1 if a == 0 else 0
    for i, z in enumerate(tops):
        s0, s1 = i * run + 0.005, (i + 1) * run - 0.005
        p0, p1 = lc(s0, fw, z - 0.03), lc(s1 - 0.05, W - fw, z)
        g.grating(tuple(map(min, p0, p1)), tuple(map(max, p0, p1)), along=along_c, panel=W)
        g.box((s1 - 0.05, fw, z - 0.006), (s1, W - fw, z), "Hazard", matrix=M)        # Nosing, striped
        g.box((s1 - 0.006, fw, z - 0.05), (s1, W - fw, z - 0.006), "DarkSteel", matrix=M)
        for c_lo in (fw, W - fw - 0.006):
            g.box((s0, c_lo, z - 0.07), (s1, c_lo + 0.006, z - 0.004), "DarkSteel", matrix=M)  # Carrier plates
        for c, d in ((fw - tw, -1.0), (W - fw + tw, 1.0)):
            for sb in (s0 + 0.1, s1 - 0.1):
                bolt(g, lc(sb, c, z - 0.04), across * d, size=0.01)
    # Handrails: posts in the stringers' channels, a top rail 0.95 m over the stringer, a mid rail.
    for c in (0.035, W - 0.035):
        for s in lin(0.25, s_end - 0.2, 1.5):
            g.cylinder(lc(s, c, zb(s) + 0.06), lc(s, c, zt(s) + 1.0), 0.024, "Ochre", segments=12)
        for h, rad in ((0.95, 0.0265), (0.48, 0.021)):
            g.cylinder(lc(0.0, c, zt(0.0) + h), lc(s_end - 0.1, c, zt(s_end - 0.1) + h), rad, "Ochre", segments=14)


# Ladders ---------------------------------------------------------------------------------------------

def ladder(k: Kit, lad: dict):
    g = k.g("Ladders")
    (x0, y0, z0), (x1, y1, z1) = lad["min"], lad["max"]
    ix, iy = lad["into"]
    label = lad["label"]
    caged = "Cage" in label or "Mast" in label
    across_x = ix == 0  # The rungs run across the face
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    half = 0.25
    out = -0.12  # Rungs stand off the face
    if across_x:
        face_y = y1 if iy > 0 else y0
        fy = face_y - iy * 0.0 + (-iy) * 0.0
        sy = fy - iy * (-out)
        for s in (-1, 1):
            g.box((cx + s * half - 0.025, sy - 0.03, z0), (cx + s * half + 0.025, sy + 0.03, z1 + (1.0 if caged else 0.3)), "DarkSteel")
        z = z0 + 0.3
        while z < z1:
            g.cylinder((cx - half, sy, z), (cx + half, sy, z), 0.014, "DarkSteel", segments=6)
            z += 0.3
        if caged:
            g.ring_cage((cx, sy + (-iy) * 0.35, 0), 0.38, z0, z1, (0, iy), "Ochre")
    else:
        face_x = x1 if ix > 0 else x0
        sx = face_x - ix * (-out)
        for s in (-1, 1):
            g.box((sx - 0.03, cy + s * half - 0.025, z0), (sx + 0.03, cy + s * half + 0.025, z1 + (1.0 if caged else 0.3)), "DarkSteel")
        z = z0 + 0.3
        while z < z1:
            g.cylinder((sx, cy - half, z), (sx, cy + half, z), 0.014, "DarkSteel", segments=6)
            z += 0.3
        if caged:
            g.ring_cage((sx + (-ix) * 0.35, cy, 0), 0.38, z0, z1, (ix, 0), "Ochre")


# The placeholders as objects -------------------------------------------------------------------------

def wheel(g: Group, centre, radius: float, width: float, axis_y: bool = True):
    c = Vector(centre)
    d = Vector((0, width / 2, 0)) if axis_y else Vector((width / 2, 0, 0))
    g.cylinder(c - d, c + d, radius, "Rubber", segments=20)
    g.cylinder(c - d * 1.05, c + d * 1.05, radius * 0.55, "DarkSteel", segments=12)


def truck(k: Kit, box: dict):
    g = k.g("Trucks")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    paint = "Ochre" if x0 < 0 else "HTROrange"  # Truck A Guild ochre, Truck B HTR orange
    label = box["label"]
    yc = (y0 + y1) / 2
    if label.endswith("Cab"):
        g.box((x0, y0 + 0.1, 0.9), (x1, y1 - 0.1, z1 - 0.1), paint, bevel=0.08)
        g.box((x0 + 0.05, y0 + 0.08, z1 - 1.1), (x1 - 0.05, y1 - 0.08, z1 - 0.35), "Glass")  # The cab's glass band
        g.box((x0, y0, 0.45), (x1, y1, 0.9), "DarkSteel", bevel=0.03)  # Chassis and bumpers
        for y in (y0 + 0.4, y1 - 0.4):
            wheel(g, ((x0 + x1) / 2, y, 0.55), 0.55, 0.45)
        g.cylinder((x1 - 0.3 if x0 < 0 else x0 + 0.3, y1 - 0.2, 1.5), (x1 - 0.3 if x0 < 0 else x0 + 0.3, y1 - 0.2, z1 + 0.6), 0.08,
                   "DarkSteel")  # Exhaust stack
        k.g("Lamps").box(((x0 + x1) / 2 - 0.15, yc - 0.6, z1 - 0.1), ((x0 + x1) / 2 + 0.15, yc - 0.3, z1 + 0.05), "Lamp")
        k.g("Lamps").box(((x0 + x1) / 2 - 0.15, yc + 0.3, z1 - 0.1), ((x0 + x1) / 2 + 0.15, yc + 0.6, z1 + 0.05), "Lamp")
    elif label.endswith("Bed"):
        g.box((x0, y0, z1 - 0.15), (x1, y1, z1), "Timber", bevel=0.01)
        g.box((x0, y0 + 0.4, 0.4), (x1, y0 + 0.7, z1 - 0.15), "DarkSteel")
        g.box((x0, y1 - 0.7, 0.4), (x1, y1 - 0.4, z1 - 0.15), "DarkSteel")
        for (a, b) in (((x0, y0), (x1, y0 + 0.06)), ((x0, y1 - 0.06), (x1, y1))):
            g.box((a[0], a[1], z1), (b[0], b[1], z1 + 0.3), paint, bevel=0.01)
        for x in (x0 + 1.5, x0 + 2.8, x1 - 1.5) if x1 - x0 > 4 else ((x0 + x1) / 2,):
            for y in (y0 + 0.4, y1 - 0.4):
                wheel(g, (x, y, 0.55), 0.55, 0.45)
        k.g("Lamps").box((x0 + 0.2, y0 - 0.01, z1 - 0.1), (x0 + 0.5, y0 + 0.01, z1 - 0.02), "Lamp")
    else:  # A ramp step: drawn as one sloping steel plate over the steps
        g.box((x0, y0, 0.0), (x1, y1, max(0.02, z1)), "DarkSteel", bevel=0.01)
    k.covered.add(box["_i"])


def billets(k: Kit, box: dict):
    """A billet stack: 20 cm square billets in layers on timber dunnage, banded."""
    g = k.g("Props")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    r = rng(box["label"], x0, y0)
    z = z0
    layer = 0
    while z < z1 - 0.15:
        g.box((x0, y0 + 0.1, z), (x0 + 0.1, y1 - 0.1, z + 0.08), "Timber")
        g.box((x1 - 0.1, y0 + 0.1, z), (x1, y1 - 0.1, z + 0.08), "Timber")
        z += 0.08
        n = int((y1 - y0) / 0.22)
        for i in range(n):
            yy = y0 + 0.01 + i * 0.22
            g.box((x0 + r.uniform(0, 0.05), yy, z), (x1 - r.uniform(0, 0.05), yy + 0.2, z + 0.2), "Rust" if r.random() < 0.3 else "DarkSteel",
                  bevel=0.008)
        z += 0.2
        layer += 1
    for x in (x0 + 0.5, x1 - 0.5):
        g.box((x - 0.02, y0 - 0.005, z0), (x + 0.02, y1 + 0.005, z), "DarkSteel")
    k.covered.add(box["_i"])


def welding_cart(k: Kit, box: dict):
    g = k.g("Props")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, 0.25), (x1, y1, 0.3), "DarkSteel")
    for (x, y) in ((x0 + 0.05, y0 + 0.05), (x1 - 0.05, y0 + 0.05), (x0 + 0.05, y1 - 0.05), (x1 - 0.05, y1 - 0.05)):
        g.cylinder((x, y, 0.3), (x, y, 1.1), 0.015, "DarkSteel", segments=6)
    for (x, y) in ((x0 + 0.1, y0 + 0.08), (x1 - 0.1, y0 + 0.08)):
        wheel(g, (x, y, 0.12), 0.12, 0.05)
    g.box((x0 + 0.1, y0 + 0.1, 0.3), (x1 - 0.5, y1 - 0.1, 0.75), "HTROrange", bevel=0.02)  # The welder
    for x, paint in ((x1 - 0.35, "Rust"), (x1 - 0.15, "Ochre")):
        g.cylinder((x, (y0 + y1) / 2, 0.3), (x, (y0 + y1) / 2, 1.35), 0.1, paint, segments=14)
        g.cylinder((x, (y0 + y1) / 2, 1.35), (x, (y0 + y1) / 2, 1.45), 0.03, "DarkSteel", segments=8)
    k.g("Lamps").box((x0 + 0.2, y0 - 0.02, 1.1), (x0 + 0.4, y0 + 0.02, 1.25), "Lamp")  # Arc lamp (static)
    k.covered.add(box["_i"])


def pallet(g: Group, x0, y0, x1, y1, z0=0.0):
    for x in (x0 + 0.05, (x0 + x1) / 2, x1 - 0.05):
        g.box((x - 0.05, y0, z0), (x + 0.05, y1, z0 + 0.1), "Timber")
    i = 0
    y = y0
    while y < y1 - 0.08:
        g.box((x0, y, z0 + 0.1), (x1, y + 0.1, z0 + 0.13), "Timber")
        y += 0.15
        i += 1


def small_prop(k: Kit, box: dict):
    g = k.g("Props")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    label = box["label"]
    r = rng(label, x0, y0)
    if label == "PH_BrickPallet":
        pallet(g, x0, y0, x1, y1)
        z = 0.13
        while z < z1 - 0.07:
            for i in range(int((x1 - x0) / 0.22)):
                for j in range(int((y1 - y0) / 0.11)):
                    g.box((x0 + i * 0.22 + 0.005, y0 + j * 0.11 + 0.005, z), (x0 + (i + 1) * 0.22 - 0.005, y0 + (j + 1) * 0.11 - 0.005, z + 0.07),
                          "Rust" if r.random() < 0.8 else "Concrete")
            z += 0.075
    elif label == "PH_ScaffoldBoardStack":
        z = z0
        while z < z1 - 0.03:
            for j in range(int((y1 - y0) / 0.23)):
                g.box((x0 + r.uniform(0, 0.08), y0 + j * 0.23, z), (x1 - r.uniform(0, 0.08), y0 + j * 0.23 + 0.225, z + 0.04),
                      "Timber", bevel=0.005)
            z += 0.045
    elif label == "PH_CartTrolley":
        g.box((x0, y0, 0.25), (x1, y1, 0.3), "DarkSteel")
        g.box((x0, y0, 0.3), (x1, y0 + 0.03, z1), "Ochre")
        g.cylinder((x1, y0, z1), (x1, y1, z1), 0.02, "Ochre")
        for (x, y) in ((x0 + 0.1, y0 + 0.1), (x1 - 0.1, y0 + 0.1), (x0 + 0.1, y1 - 0.1), (x1 - 0.1, y1 - 0.1)):
            wheel(g, (x, y, 0.1), 0.1, 0.05, axis_y=False)
    elif label == "PH_HandTruck":
        for y in (y0 + 0.05, y1 - 0.05):
            g.cylinder((x0 + 0.1, y, 0.1), (x1 - 0.1, y, z1), 0.018, "Ochre", segments=8)
        g.box((x0, y0, 0.0), (x0 + 0.3, y1, 0.02), "DarkSteel")
        for y in (y0, y1):
            wheel(g, (x1 - 0.15, y, 0.12), 0.12, 0.06)
    elif label == "PH_HaulSpool":
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        g.cylinder((x0 + 0.1, cy, 0.8), (x1 - 0.1, cy, 0.8), 0.45, "Rust", segments=20)
        for x in (x0 + 0.05, x1 - 0.05):
            g.cylinder((x - 0.04, cy, 0.8), (x + 0.04, cy, 0.8), 0.78, "DarkSteel", segments=24)
        g.box((x0, y0, 0.0), (x1, y1, 0.12), "DarkSteel", bevel=0.01)
    elif label == "PH_ClaimMarker":
        pallet(g, x0, y0, x1, y1)
        g.box((x0 + 0.05, y0 + 0.1, 0.13), (x1 - 0.05, y1 - 0.1, z1), "DarkSteel", bevel=0.01)  # The Weave billet
    elif label == "PH_ClaimMarker_Beacon":
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        g.cylinder((cx, cy, z0), (cx, cy, z1), 0.04, "DarkSteel", segments=8)
        k.g("Lamps").cylinder((cx, cy, z1 - 0.05), (cx, cy, z1 + 0.08), 0.18, "Lamp", segments=16)  # LayCast ring light
    elif label == "PH_LaundryLine":
        a, b = Vector((x0, (y0 + y1) / 2, z1)), Vector((x1, (y0 + y1) / 2, z1))
        g.cylinder(a, b, 0.006, "DarkSteel", segments=4)
        for x in (x0, x1):
            g.cylinder((x, (y0 + y1) / 2, 8.0), (x, (y0 + y1) / 2, z1 + 0.1), 0.03, "DarkSteel", segments=8)
        t = x0 + 0.4
        while t < x1 - 0.3:
            w = r.uniform(0.3, 0.6)
            g.box((t, (y0 + y1) / 2 - 0.01, z1 - r.uniform(0.4, 0.9)), (t + w, (y0 + y1) / 2 + 0.01, z1), r.choice(("Ochre", "HTROrange", "Hazard")))
            t += w + 0.15
    else:
        g.box((x0, y0, z0), (x1, y1, z1), "DarkSteel", bevel=0.02)
    k.covered.add(box["_i"])


HEAP_PAINT = (("Rust", 0.55), ("HullSteel", 0.2), ("DarkSteel", 0.19), ("Ochre", 0.03), ("HTROrange", 0.03))
HEAP_PIECES = (("plate", 0.42), ("section", 0.18), ("pipe", 0.13), ("chunk", 0.17), ("rebar", 0.06), ("drum", 0.04))
ROUND = [(math.cos(a), math.sin(a)) for a in (i * 2 * math.pi / 8 for i in range(8))]


def pick(r: random.Random, table) -> str:
    x = r.random() * sum(w for _, w in table)
    for name, w in table:
        x -= w
        if x <= 0:
            return name
    return table[-1][0]


def scrap_plate(g: Group, r: random.Random):
    """A plate offcut, lying in XY at the origin: torch-cut or torn at its edges (the torn ones
    ragged, a corner gone), crumpled, and often folded over a bend."""
    w, l = 0.15 + 1.35 * r.random() ** 1.6, 0.12 + 0.9 * r.random() ** 1.4  # Mostly small, the odd big one
    thick = r.choice((0.006, 0.008, 0.01, 0.012, 0.016, 0.02, 0.025))
    step = 0.06 if max(w, l) < 0.8 else 0.085
    us, vs = lin(-w / 2, w / 2, step), lin(-l / 2, l / 2, step)
    du, dv = us[1] - us[0], vs[1] - vs[0]
    crumple = waves(r, r.uniform(0.0, 0.03) * (0.5 + w), r.uniform(0.25, 0.7), 4)
    torn = {side: r.random() < 0.45 for side in ("u0", "u1", "v0", "v1")}
    tears = {side: [r.uniform(0.0, 0.75) for _ in range(max(len(us), len(vs)))] for side in torn}
    folds = []
    if r.random() < 0.6:
        folds.append((r.uniform(-w * 0.3, w * 0.3), r.choice((-1, 1)) * r.uniform(0.25, 1.7), r.uniform(0.03, 0.12)))
    cut_corner = r.choice((None, None, (1, 1), (-1, 1), (1, -1), (-1, -1)))
    cut = r.uniform(0.15, 0.45) * min(w, l)

    def shape(u, v):
        i, j = round((u - us[0]) / du), round((v - vs[0]) / dv)
        pu = pv = 0.0
        if torn["u0"] and i == 0:
            pu += tears["u0"][j] * du
        if torn["u1"] and i == len(us) - 1:
            pu -= tears["u1"][j] * du
        if torn["v0"] and j == 0:
            pv += tears["v0"][i] * dv
        if torn["v1"] and j == len(vs) - 1:
            pv -= tears["v1"][i] * dv
        z = crumple(u, v)
        x = u + pu
        for (uf, theta, rb) in folds:
            s = x - uf
            if s > 0:
                rad = rb / theta
                if s < rb:
                    phi = theta * s / rb
                    x, z = uf + rad * math.sin(phi), z + rad * (1 - math.cos(phi))
                else:
                    ex, ez = uf + rad * math.sin(theta), rad * (1 - math.cos(theta))
                    x, z = ex + (s - rb) * math.cos(theta), z + ez + (s - rb) * math.sin(theta)
        return x - u, pv, z

    def keep(i, j):
        cu, cv = (us[i] + us[i + 1]) / 2, (vs[j] + vs[j + 1]) / 2
        if cut_corner and (cu * cut_corner[0] - w / 2) + (cv * cut_corner[1] - l / 2) > -cut:
            return False  # A corner burnt off on the diagonal
        edge = i == 0 or j == 0 or i == len(us) - 2 or j == len(vs) - 2
        torn_here = (i == 0 and torn["u0"]) or (i == len(us) - 2 and torn["u1"]) or (j == 0 and torn["v0"]) or (j == len(vs) - 2 and torn["v1"])
        return not (edge and torn_here and r.random() < 0.15)

    g.sheet(Matrix.Identity(4), us, vs, pick(r, HEAP_PAINT), shape, keep=keep, thickness=thick)


def scrap_section(g: Group, r: random.Random):
    """An offcut of rolled section (I, channel, angle, tee, flat or square hollow), its ends
    torch-cut on the skew, about half of them bent."""
    kind = r.choice(("I", "I", "C", "L", "L", "T", "flat", "box"))
    length = r.uniform(0.5, 2.3)
    d, b = r.uniform(0.1, 0.32), r.uniform(0.06, 0.16)
    tf, tw = max(0.008, d * 0.07), max(0.006, d * 0.045)
    if kind == "I":
        prof = [(-b / 2, -d / 2), (b / 2, -d / 2), (b / 2, -d / 2 + tf), (tw / 2, -d / 2 + tf), (tw / 2, d / 2 - tf), (b / 2, d / 2 - tf),
                (b / 2, d / 2), (-b / 2, d / 2), (-b / 2, d / 2 - tf), (-tw / 2, d / 2 - tf), (-tw / 2, -d / 2 + tf), (-b / 2, -d / 2 + tf)]
    elif kind == "C":
        prof = [(-b / 2, -d / 2), (b / 2, -d / 2), (b / 2, -d / 2 + tf), (-b / 2 + tw, -d / 2 + tf), (-b / 2 + tw, d / 2 - tf),
                (b / 2, d / 2 - tf), (b / 2, d / 2), (-b / 2, d / 2)]
    elif kind == "L":
        a, t = r.uniform(0.05, 0.15), r.uniform(0.006, 0.014)
        prof = [(0, 0), (a, 0), (a, t), (t, t), (t, a), (0, a)]
        prof = [(x - a / 3, y - a / 3) for x, y in prof]
        d = a
    elif kind == "T":
        prof = [(-b / 2, d / 2 - tf), (-tw / 2, d / 2 - tf), (-tw / 2, -d / 2), (tw / 2, -d / 2), (tw / 2, d / 2 - tf), (b / 2, d / 2 - tf),
                (b / 2, d / 2), (-b / 2, d / 2)]
    elif kind == "flat":
        t = r.uniform(0.01, 0.03)
        prof = [(-b, -t / 2), (b, -t / 2), (b, t / 2), (-b, t / 2)]
        d = t
    else:
        s = r.uniform(0.06, 0.2)
        n = 9 if r.random() < 0.4 else 2
        path = bent_path(r, length, n, 0.9)
        g.tube([p + Vector((0, 0, s / 2)) for p in path], s / math.sqrt(2), max(0.004, s * 0.06), pick(r, HEAP_PAINT), segments=4,
               phase=math.pi / 4, cut0=(r.uniform(-0.5, 0.5), 0.0), cut1=(r.uniform(-0.5, 0.5), 0.0))
        return
    n = 9 if r.random() < 0.45 else 2
    path = [p + Vector((0, 0, d / 2)) for p in bent_path(r, length, n, 1.0)]
    g.sweep(path, prof, pick(r, HEAP_PAINT), up=(0, 0, 1), cut0=(r.uniform(-0.6, 0.6), r.uniform(-0.3, 0.3)),
            cut1=(r.uniform(-0.6, 0.6), r.uniform(-0.3, 0.3)))


def bent_path(r: random.Random, length: float, n: int, most: float):
    """A path along X through the origin, straight (n = 2) or bent by up to `most` radians in a
    random plane, the bend gathered about its middle or towards one end."""
    if n <= 2:
        return [Vector((-length / 2, 0, 0)), Vector((length / 2, 0, 0))]
    total = r.uniform(0.25, most) * r.choice((-1, 1))
    where, plane = r.choice((0.5, 0.5, 0.3, 0.7)), r.uniform(0, math.pi)
    weights = [math.exp(-((i / (n - 2) - where) / 0.2) ** 2) for i in range(n - 1)]
    pts, p, heading = [Vector((0, 0, 0))], Vector((0, 0, 0)), 0.0
    for wgt in weights:
        heading += total * wgt / sum(weights)
        p = p + Vector((math.cos(heading), math.sin(heading) * math.cos(plane), math.sin(heading) * math.sin(plane))) * (length / (n - 1))
        pts.append(p.copy())
    mid = sum(pts, Vector((0, 0, 0))) / len(pts)
    return [q - mid for q in pts]


def scrap_pipe(g: Group, r: random.Random):
    """A length of pipe, cut square or skew, one end often crushed flat by the press."""
    rad = r.uniform(0.03, 0.17)
    wall = max(0.004, rad * r.uniform(0.06, 0.12))
    length = r.uniform(0.4, 2.0)
    path = [Vector((-length / 2 + length * i / 6, 0, rad)) for i in range(7)]
    crushed = r.random() < 0.35
    flat = r.uniform(0.25, 0.6)

    def squash(i):
        if not crushed or i < 4:
            return 1.0, 1.0
        k_ = (i - 3) / 3 * (1 - flat)
        return 1.0 + k_ * 0.55, 1.0 - k_
    g.tube(path, rad, wall, r.choice(("Rust", "Rust", "Rust", "DarkSteel", "HullSteel")), segments=16 if rad > 0.06 else 12,
           scale=squash, cut0=(r.uniform(-0.5, 0.5), 0.0), cut1=(0.0, 0.0) if crushed else (r.uniform(-0.5, 0.5), 0.0))


def scrap_rebar(g: Group, r: random.Random):
    """A tangle of reinforcing bar: three to seven bars wandering through each other, some hooked."""
    rad = r.choice((0.006, 0.008, 0.01, 0.012, 0.016))
    surface = "Rust" if r.random() < 0.75 else "DarkSteel"
    for _ in range(r.randint(3, 7)):
        p = Vector((r.uniform(-0.35, 0.35), r.uniform(-0.35, 0.35), r.uniform(0.02, 0.15)))
        heading, turn, climb = r.uniform(0, 2 * math.pi), r.uniform(-0.25, 0.25), r.uniform(0, 2 * math.pi)
        pts = [p.copy()]
        for i in range(int(r.uniform(0.8, 2.6) / 0.08)):
            turn += r.uniform(-0.12, 0.12)
            turn = max(-0.4, min(0.4, turn))
            heading += turn
            p = p + Vector((math.cos(heading) * 0.08, math.sin(heading) * 0.08, 0.025 * math.sin(climb + i * 0.4)))
            pts.append(p.copy())
        if r.random() < 0.4:  # A hook bent on the end
            hd = pts[-1] - pts[-2]
            pts.append(pts[-1] + Vector((0, 0, 0.06)) + hd * 0.3)
            pts.append(pts[-1] + Vector((0, 0, 0.06)) - hd * 0.8)
        g.sweep(pts, [(x * rad, y * rad) for x, y in ROUND], surface, soft=True)


def scrap_drum(g: Group, r: random.Random):
    """A 200 litre drum crushed in the press: squashed oval, dented, its rolling hoops and chimes
    still showing; lying on its side or not."""
    R, H = 0.286, 0.88
    zs = sorted(set([0.0, 0.012, 0.03, H - 0.03, H - 0.012, H] + [H / 3 + d for d in (-0.03, 0, 0.03)] + [2 * H / 3 + d for d in (-0.03, 0, 0.03)]
                    + lin(0.0, H, 0.08)))
    g.hold()
    g.sweep([Vector((0, 0, z)) for z in zs], [(math.cos(a) * R, math.sin(a) * R) for a in (i * 2 * math.pi / 28 for i in range(28))],
            r.choice(("HTROrange", "Ochre", "Rust", "Rust", "DarkSteel")), up=(1, 0, 0), soft=True)
    crush = r.uniform(0.35, 0.9)
    dents = [(r.uniform(0, 2 * math.pi), r.uniform(0.1, H - 0.1), r.uniform(0.02, 0.08), r.uniform(0.08, 0.2)) for _ in range(r.randint(2, 6))]
    verts = g.release()
    for v in verts:
        x, y, z = v.co
        rr, ang = math.hypot(x, y), math.atan2(y, x)
        if rr > 1e-4:
            grow = 0.012 * (math.exp(-((z - H / 3) / 0.02) ** 2) + math.exp(-((z - 2 * H / 3) / 0.02) ** 2))  # Rolling hoops
            grow += 0.008 if (z < 0.013 or z > H - 0.013) and rr > R * 0.99 else 0.0                             # Chimes
            for (a0, zc, depth, size) in dents:
                da = (ang - a0 + math.pi) % (2 * math.pi) - math.pi
                grow -= depth * math.exp(-((da * R / size) ** 2 + ((z - zc) / size) ** 2)) * (rr / R)
            k_ = (rr + grow * rr / R) / rr
            x, y = x * k_, y * k_
        x *= crush
        y *= 1.0 + (1.0 - crush) * 0.45
        z *= 1.0 - (1.0 - crush) * 0.25 * (0.5 + 0.5 * math.cos(ang * 2))
        v.co = Vector((x, y, z))
    if r.random() < 0.65:  # On its side
        bmesh.ops.rotate(g.bm, verts=verts, cent=(0, 0, 0), matrix=Matrix.Rotation(math.pi / 2, 3, "Y"))


def scrap_chunk(g: Group, r: random.Random):
    """Small stuff: a billet end, a cut flange ring, a lug or a short angle cleat."""
    kind = r.random()
    if kind < 0.4:
        s, l = r.uniform(0.1, 0.25), r.uniform(0.12, 0.6)
        g.box((-l / 2, -s / 2, 0.0), (l / 2, s / 2, s), r.choice(("Rust", "DarkSteel", "Rust")), bevel=r.uniform(0.004, 0.015))
    elif kind < 0.7:
        rad = r.uniform(0.1, 0.3)
        g.tube([Vector((0, 0, 0)), Vector((0, 0, r.uniform(0.02, 0.05)))], rad, rad * r.uniform(0.25, 0.5), r.choice(("Rust", "DarkSteel")),
               segments=24)
    elif kind < 0.85:
        t, w = r.uniform(0.015, 0.03), r.uniform(0.12, 0.25)
        lug = [(-w / 2, 0.0), (w / 2, 0.0), (w / 2, w * 0.5)] + [(math.cos(a) * w / 2, w * 0.5 + math.sin(a) * w / 2)
                                                                 for a in (i * math.pi / 8 for i in range(1, 8))] + [(-w / 2, w * 0.5)]
        g.prism(lug, Matrix.Rotation(-math.pi / 2, 4, "X"), t, pick(r, HEAP_PAINT))
    else:
        a, t, l = r.uniform(0.06, 0.12), 0.008, r.uniform(0.1, 0.35)
        g.sweep([Vector((-l / 2, 0, 0)), Vector((l / 2, 0, 0))], [(0, 0), (a, 0), (a, t), (t, t), (t, a), (0, a)], pick(r, HEAP_PAINT))


def scrap_heap(k: Kit, box: dict):
    """A heap of press offcuts within its box (the collision): a bed of rust scale and swarf, and
    on it, settled one on another and mounded flat-topped to the box, torn and folded plate,
    beam, channel and angle ends, pipe (some crushed), rebar tangles, the odd crushed drum, small
    cut pieces, and a cable or two thrown over the top."""
    g = k.g("Heaps")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    rx, ry, H = (x1 - x0) / 2, (y1 - y0) / 2, z1 - z0
    r = rng(box["label"], cx, cy, z0)
    lump = waves(r, 0.12, 1.3, 4)
    ramp = min(rx, ry) * 0.45

    def target(x, y):
        """The heap's settled height: a lumpy flat top falling away over the outer `ramp`."""
        q = ((abs(x - cx) / rx) ** 4 + (abs(y - cy) / ry) ** 4) ** 0.25
        e = (1.0 - q) * min(rx, ry)
        return z0 + H * (0.05 + 0.95 * smoothstep(0.0, ramp, e) ** 0.8) * (0.9 + lump(x, y))

    cell = 0.08
    nx, ny = int(2 * rx / cell) + 1, int(2 * ry / cell) + 1
    hm = [[z0] * ny for _ in range(nx)]

    def at(x, y):
        i, j = int((x - x0) / cell), int((y - y0) / cell)
        return hm[i][j] if 0 <= i < nx and 0 <= j < ny else z0

    # The bed: scale, swarf and fines, settled to about half the heap's height.
    grit = waves(r, 0.02, 0.15, 5)

    def bed(x, y):
        q = ((abs(x - cx) / rx) ** 4 + (abs(y - cy) / ry) ** 4) ** 0.25
        edge = smoothstep(0.0, 0.18, (1.0 - q) * min(rx, ry))
        return (z0 - 0.03) + (0.5 * (target(x, y) - z0) + grit(x, y) + 0.03) * edge

    for i in range(nx):
        for j in range(ny):
            hm[i][j] = bed(x0 + (i + 0.5) * cell, y0 + (j + 0.5) * cell)
    bus, bvs = lin(x0 + 0.01, x1 - 0.01, 0.07), lin(y0 + 0.01, y1 - 0.01, 0.07)
    g.sheet(Matrix.Identity(4), bus, bvs, "Rust", lambda u, v: (0.0, 0.0, bed(u, v)))
    # The pieces, biggest first, each dropped where the heap is still short of its shape.
    count = int(4 * rx * ry * 17)
    kinds = [pick(r, HEAP_PIECES) for _ in range(count)]
    drums = 0
    order = {"drum": 0, "section": 1, "plate": 2, "pipe": 3, "chunk": 4, "rebar": 5}
    kinds.sort(key=lambda kd: order[kd] + r.random() * 2.5)
    for kind in kinds:
        if kind == "drum":
            if drums or H < 1.0:
                continue
            drums += 1
        g.hold()
        {"plate": scrap_plate, "section": scrap_section, "pipe": scrap_pipe, "rebar": scrap_rebar, "drum": scrap_drum,
         "chunk": scrap_chunk}[kind](g, r)
        verts = g.release()
        if not verts:
            continue
        best = None
        for _ in range(4):
            x, y = r.uniform(x0 + 0.12, x1 - 0.12), r.uniform(y0 + 0.12, y1 - 0.12)
            short = target(x, y) - at(x, y)
            if best is None or short > best[0]:
                best = (short, x, y)
        _, x, y = best
        gx = (target(x + 0.1, y) - target(x - 0.1, y)) / 0.2
        gy = (target(x, y + 0.1) - target(x, y - 0.1)) / 0.2
        slope = math.atan(math.hypot(gx, gy))
        tilt = slope * r.uniform(0.5, 1.15) + r.uniform(-0.25, 0.25)
        if kind == "plate" and r.random() < 0.15:
            tilt += r.uniform(0.6, 1.2)  # Stood on edge, leaning in
        axis = Vector((-gy, gx, 0.0)) if slope > 0.05 else Vector((math.cos(r.uniform(0, 6.3)), math.sin(r.uniform(0, 6.3)), 0.0))
        rot = Matrix.Rotation(-tilt, 4, axis.normalized()) @ Matrix.Rotation(r.uniform(0, 2 * math.pi), 4, "Z")
        bmesh.ops.transform(g.bm, matrix=Matrix.Translation((x, y, 0.0)) @ rot, verts=verts)
        lo_x, hi_x = min(v.co.x for v in verts), max(v.co.x for v in verts)
        lo_y, hi_y = min(v.co.y for v in verts), max(v.co.y for v in verts)
        if hi_x - lo_x > 2 * rx + 0.04 or hi_y - lo_y > 2 * ry + 0.04:
            bmesh.ops.delete(g.bm, geom=verts, context="VERTS")
            continue
        dx = max(0.0, x0 - 0.02 - lo_x) - max(0.0, hi_x - (x1 + 0.02))
        dy = max(0.0, y0 - 0.02 - lo_y) - max(0.0, hi_y - (y1 + 0.02))
        # Settle it: lifted until most of it rests on what is already there, kept under the top.
        sample = verts[::max(1, len(verts) // 160)]
        lifts = sorted(at(v.co.x + dx, v.co.y + dy) - v.co.z for v in sample)
        lift = lifts[int(len(lifts) * 0.8)] - r.uniform(0.0, 0.04)
        high = max(v.co.z for v in verts)
        lift = min(lift, z1 - 0.01 - high)
        bmesh.ops.translate(g.bm, vec=(dx, dy, lift), verts=verts)
        for v in verts:
            i, j = int((v.co.x - x0) / cell), int((v.co.y - y0) / cell)
            if 0 <= i < nx and 0 <= j < ny and v.co.z > hm[i][j]:
                hm[i][j] = v.co.z
    # A cable or two thrown over it all.
    for _ in range(r.randint(1, 2)):
        rad = r.uniform(0.012, 0.025)
        p = Vector((r.uniform(x0 + 0.3, x1 - 0.3), r.uniform(y0 + 0.3, y1 - 0.3), 0))
        heading, turn = r.uniform(0, 2 * math.pi), 0.0
        pts = []
        for _ in range(int(r.uniform(2.5, 5.0) / 0.1)):
            turn = max(-0.35, min(0.35, turn + r.uniform(-0.15, 0.15)))
            heading += turn
            q = p + Vector((math.cos(heading), math.sin(heading), 0)) * 0.1
            if not (x0 + 0.1 < q.x < x1 - 0.1 and y0 + 0.1 < q.y < y1 - 0.1):
                heading += math.pi * 0.6
                continue
            p = q
            pts.append(Vector((p.x, p.y, min(z1 - rad, at(p.x, p.y) + rad * 0.7))))
        if len(pts) > 3:
            for i in range(1, len(pts) - 1):  # Sag between the high points
                pts[i].z = max(pts[i].z, (pts[i - 1].z + pts[i + 1].z) / 2 - 0.02)
            g.sweep(pts, [(x * rad, y * rad) for x, y in ROUND], "Rubber", soft=True)
    k.covered.add(box["_i"])


# Clutter -----------------------------------------------------------------------------------------------

def clutter(k: Kit):
    """Things where nobody walks: cable runs along the trench lips, puddles on the docks, water
    barriers at the Haul Lane's ends, pipe stacks and notices against the perimeter walls."""
    g = k.g("Clutter")
    r = rng("clutter")
    for yc in TRENCHES:
        for lip in (-3.6, 3.6):
            y = yc + lip
            x = -78.0
            while x < 78.0:
                l = r.uniform(4.0, 12.0)
                if k.free(x, y - 0.1, x + l, y + 0.1, 0.0, 0.3):
                    g.cylinder((x, y, 0.03), (x + l, y + r.uniform(-0.15, 0.15), 0.03), 0.025, "Rubber", segments=6)
                x += l + r.uniform(1.0, 6.0)
    for _ in range(40):
        x, y = r.uniform(-78, 78), r.choice((r.uniform(-44, -34), r.uniform(-26, -4), r.uniform(4, 26), r.uniform(34, 44)))
        w, d = r.uniform(0.6, 2.2), r.uniform(0.4, 1.4)
        if k.free(x - w, y - d, x + w, y + d, 0.0, 0.3):
            g.box((x - w, y - d, 0.0), (x + w, y + d, 0.004), "Glass")  # A puddle
    for x in (-74.0, -70.0, 70.0, 74.0):
        for y in (-15.5, -8.5):
            g.box((x - 0.9, y - 0.3, 0.0), (x + 0.9, y + 0.3, 0.8), "HTROrange", bevel=0.12)  # Water barrier
    # Pipe stacks and notices along the north and south walls.
    for y, d in ((-44.4, 1), (44.4, -1)):
        x = -76.0
        while x < 76.0:
            if r.random() < 0.35 and k.free(x - 1.5, min(y, y + d * 1.2), x + 1.5, max(y, y + d * 1.2), 0.0, 1.0):
                for i in range(6):
                    row, col = (0, i) if i < 3 else (1, i - 3)
                    zz = 0.12 + row * 0.22
                    yy = y + d * (0.15 + col * 0.24 + row * 0.12)
                    g.cylinder((x - 1.5, yy, zz), (x + 1.5, yy, zz), 0.11, r.choice(("Rust", "DarkSteel", "Ochre")), segments=12)
            elif r.random() < 0.25:
                g.box((x - 0.8, y - 0.02, 1.4), (x + 0.8, y + 0.02, 2.4), r.choice(("Ochre", "HTROrange")), bevel=0.01)  # A notice
            x += r.uniform(5.0, 12.0)


# The build -------------------------------------------------------------------------------------------

def build(layout: dict) -> Kit:
    k = Kit(layout)
    steps = []
    for i, box in enumerate(layout["boxes"]):
        box["_i"] = i
        role, label = box["role"], box["label"]
        if label in NOT_COVERED:
            continue
        if role == "floor":
            island(k, box)
        elif role == "trench":
            trench_bed(k, box)
        elif role in ("mesa", "ledge"):
            mesa(k, box)
        elif role == "pit":
            pit_piece(k, box)
        elif role == "ladder":
            k.covered.add(i)  # The ladders are drawn from the ladder list
        elif role == "bridge":
            if label == "Grate_Step":
                steps.append(box)  # Drawn by flight, below
            else:
                {"Grate_Deck": grate_deck, "Grate_Rail": grate_rail, "Grate_Landing": grate_landing}[label](k, box)
        elif role == "placeholder":
            if label.startswith("PH_HaulTruck"):
                truck(k, box)
            elif label == "PH_BilletStack":
                billets(k, box)
            elif label == "PH_WeldingCart":
                welding_cart(k, box)
            elif label == "PH_ScrapHeap":
                scrap_heap(k, box)
            else:
                small_prop(k, box)
        elif label.startswith("Wall"):
            wall(k, box)
        elif label in ("PortalLeg", "PortalLeg_Head"):
            column(k, box)
        elif label in ("Portal_Crosshead", "Gantry_Beam", "Pit_Stringer"):
            girder(k, box)
        elif label in ("PortalLeg_Landing", "RailWalk", "MastPlatform"):
            walkway(k, box, "Masts" if label == "MastPlatform" else "Rails")
        elif label == "RailWalk_Handrail":
            handrail(k, box)
        elif label == "GantryRail":
            (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
            k.g("Rails").box((x0, y0, z0), (x1, y1, z1 - 0.1), "DarkSteel")
            k.g("Rails").rail((x0, (y0 + y1) / 2, z1 - 0.16), (x1, (y0 + y1) / 2, z1 - 0.16))
            k.covered.add(i)
        elif label == "FloodlightMast":
            mast(k, box)
        elif label.startswith("Press_"):
            press_piece(k, box)
        elif label.startswith("PitLipRail"):
            pit_rail(k, box)
        else:
            k.g("Rails").box(box["min"], box["max"], "HullSteel", bevel=0.02)
            k.covered.add(i)
    stairs(k, steps)
    for lad in layout["ladders"]:
        ladder(k, lad)
    clutter(k)
    return k


def materials() -> dict:
    """Stand-in materials, one per surface, so the slots carry the surface names (Unreal assigns
    the real ones by name)."""
    colours = {"Concrete": "#5A5650", "Slag": "#2B2622", "HullSteel": "#4A5560", "Rust": "#6B3B1F", "Corrugated": "#8A8E92",
               "Grating": "#3B4046", "Hazard": "#F2C318", "Ochre": "#C8922A", "HTROrange": "#E8651A", "DarkSteel": "#222428",
               "Glass": "#0C0E10", "Rubber": "#141414", "Timber": "#6A5034", "Lamp": "#FFD9A0"}
    return {s: ka.flat_material(s, colours[s], 0.6) for s in SURFACES}


def main() -> int:
    ka.reset_scene()
    layout = json.load(open(os.path.join(HERE, "yard_layout.json")))
    k = build(layout)
    mats = materials()
    os.makedirs(OUT, exist_ok=True)
    report = {}
    for name, group in k.groups.items():
        obj = group.object(mats)
        tris = sum(len(p.vertices) - 2 for p in obj.data.polygons)
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        path = os.path.join(OUT, f"{obj.name}.fbx")
        bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS", object_types={"MESH"},
                                 mesh_smooth_type="SMOOTH_GROUP", add_leaf_bones=False, bake_anim=False, path_mode="AUTO")
        report[obj.name] = {"triangles": tris, "slots": [m.name for m in obj.data.materials]}
        print(f"{obj.name}: {tris} triangles, slots {report[obj.name]['slots']}")
    with open(os.path.join(HERE, "covered.json"), "w") as f:
        json.dump(sorted(k.covered), f)
    with open(os.path.join(OUT, "kit.json"), "w") as f:
        json.dump(report, f, indent=2)
    print(f"{len(k.covered)} of {len(layout['boxes'])} boxes covered")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
