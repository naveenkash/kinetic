"""The Yard's visual kit (docs/arenas/the-yard/layout.md, prop-manifest.md, lighting-and-mood.md): the
detail drawn over the grey-box level's boxes, from code.

    blender -b --factory-startup -P art/kit/yard_kit.py

It reads art/kit/yard_layout.json (written by game/Tools/unreal/build_yard_greybox.py: every box
with its role and label, every ladder) and builds, per role:
- the islands: 4 m concrete slabs with their joints, steel lip angles, and on every trench wall
  pilasters every 6 m, a steel rub rail and form-tie studs; drain grates along the lips;
- the trench beds: twin rails on sleepers down each trench (the Haulers' rail bed);
- the Stacks: cut hull sections, one per 2.7 m layer, each with its seam band, external frames at
  its own pitch, a cut-down gunwale on top and the odd rusted patch; an inspection lamp per mesa;
- the perimeter walls in corrugated sheet on I-beam posts; the portal legs as stiffened box
  columns with bolted flanges; the crossheads as box girders; the rail walks as grating on
  brackets with handrails; the masts as lattice towers with their flood heads; the press frame in
  I-beams; the Well's rusted walls; ladders as welded rungs, the tall ones caged;
- the grate bridges on channel stringers with grating decks, handrails and stair treads;
- the placeholders as objects: the Haul trucks (cab, glass, wheels, beacons, flatbed, ramp), the
  billet stacks on dunnage, the welding carts with their bottles, pallets, trolleys, the scaffold
  boards, the laundry line, the claim marker and its ring light, the Haul spool, and the scrap
  heaps as mounds of cut steel;
- and clutter where nobody walks: cable runs along the lips, puddles, barriers, pipe stacks and
  notices on the walls.
Each group is one mesh, art/out/kit/SM_Yard_<Group>.fbx, its material slots named after the
art/surfaces surfaces, UV'd in metres. art/kit/covered.json lists the boxes the kit draws over;
the level keeps them for collision and the navmesh and stops drawing them. Boxes that move or
break (the press ram, the plank bridges) are not covered.
"""
from __future__ import annotations

import json
import math
import os
import random
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [HERE, os.path.join(HERE, "..", "lib")]

import bpy  # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402
from kitlib import SURFACES, Group  # noqa: E402

OUT = os.path.join(HERE, "..", "out", "kit")
TRENCHES = (-30.0, 0.0, 30.0)
NOT_COVERED = {"Press_Ram", "PlankBridge"}


def rng(*key) -> random.Random:
    return random.Random(zlib.crc32(repr(key).encode()))


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
    # The body of the island below the slabs.
    g.box((x0, y0, z0), (x1, y1, z1 - 0.2), "Concrete")
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
    # The trench walls: the island's long faces that stand over a trench or the pit.
    walls = k.g("TrenchWalls")
    for y, out in ((y0, -1.0), (y1, 1.0)):
        if abs(y) > 44.9:
            continue  # The perimeter: walls stand there
        for i in range(int((x1 - x0) // 6.0) + 1):
            x = x0 + 1.5 + i * 6.0
            if x > x1 - 0.5:
                break
            walls.box((x - 0.2, y + (0 if out > 0 else -0.12), z0), (x + 0.2, y + (0.12 if out > 0 else 0), z1 - 0.12), "Concrete",
                      bevel=0.02)
        # The rub rail, a steel channel at a metre and a half under the lip.
        walls.box((x0, y + (0 if out > 0 else -0.1), z1 - 1.7), (x1, y + (0.1 if out > 0 else 0), z1 - 1.5), "DarkSteel", bevel=0.01)
        # Form-tie studs in rows.
        for zi in (z1 - 0.8, z1 - 2.6, z1 - 3.4):
            x = x0 + 0.6
            while x < x1 - 0.4:
                walls.cylinder((x, y, zi), (x, y + out * 0.012, zi), 0.025, "DarkSteel", segments=8)
                x += 1.2
        # A drain grate along the lip, 60 cm in.
        walls.box((x0 + 0.5, y - out * 0.75, z1), (x1 - 0.5, y - out * 0.45, z1 + 0.008), "Grating")
    k.covered.add(box["_i"])


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
        g.box((x0 + inset, y0 + inset, za), (x1 - inset, y1 - inset, zb - 0.06), "HullSteel")
        # The seam band where two sections meet.
        g.box((x0 - 0.03, y0 - 0.03, zb - 0.08), (x1 + 0.03, y1 + 0.03, zb), "DarkSteel", bevel=0.01)
        # External frames at this section's own pitch, on every face.
        pitch = r.choice((0.8, 0.9, 1.0, 1.2))
        for face in ("x0", "x1", "y0", "y1"):
            along0, along1 = (y0, y1) if face.startswith("x") else (x0, x1)
            t = along0 + pitch / 2
            while t < along1 - 0.2:
                if r.random() < 0.08:
                    t += pitch
                    continue  # A frame cut away
                if face == "x0":
                    g.box((x0 + inset - 0.12, t - 0.04, za + 0.1), (x0 + inset, t + 0.04, zb - 0.18), "HullSteel", bevel=0.01)
                elif face == "x1":
                    g.box((x1 - inset, t - 0.04, za + 0.1), (x1 - inset + 0.12, t + 0.04, zb - 0.18), "HullSteel", bevel=0.01)
                elif face == "y0":
                    g.box((t - 0.04, y0 + inset - 0.12, za + 0.1), (t + 0.04, y0 + inset, zb - 0.18), "HullSteel", bevel=0.01)
                else:
                    g.box((t - 0.04, y1 - inset, za + 0.1), (t + 0.04, y1 - inset + 0.12, zb - 0.18), "HullSteel", bevel=0.01)
                t += pitch
        # A rusted patch or two.
        for _ in range(r.randint(0, 2)):
            face = r.choice(("x0", "x1", "y0", "y1"))
            w, hh = r.uniform(0.8, 2.5), r.uniform(0.5, 1.6)
            zc = r.uniform(za + 0.4, zb - 0.6)
            if face in ("x0", "x1"):
                x = x0 - 0.005 if face == "x0" else x1 + 0.005
                yc = r.uniform(y0 + w, y1 - w) if y1 - y0 > 2 * w else (y0 + y1) / 2
                g.box((x - 0.006, yc - w / 2, zc - hh / 2), (x + 0.006, yc + w / 2, zc + hh / 2), "Rust")
            else:
                y = y0 - 0.005 if face == "y0" else y1 + 0.005
                xc = r.uniform(x0 + w, x1 - w) if x1 - x0 > 2 * w else (x0 + x1) / 2
                g.box((xc - w / 2, y - 0.006, zc - hh / 2), (xc + w / 2, y + 0.006, zc + hh / 2), "Rust")
    # The deck on top, its plates in their own weld lines, and a cut-down gunwale at the edge.
    g.box((x0 + 0.05, y0 + 0.05, z1 - 0.02), (x1 - 0.05, y1 - 0.05, z1 + 0.01), "HullSteel")
    for (a, b) in (((x0, y0), (x1, y0 + 0.08)), ((x0, y1 - 0.08), (x1, y1)), ((x0, y0), (x0 + 0.08, y1)), ((x1 - 0.08, y0), (x1, y1))):
        g.box((a[0], a[1], z1), (b[0], b[1], z1 + 0.1), "DarkSteel", bevel=0.01)
    if box["role"] == "mesa":
        # A chalk-mark inspection lamp clipped to the face (lighting-and-mood: one per mesa).
        lamp = k.g("Lamps")
        lamp.box((x0 - 0.18, (y0 + y1) / 2 - 0.12, z1 - 1.2), (x0 - 0.02, (y0 + y1) / 2 + 0.12, z1 - 0.95), "DarkSteel", bevel=0.01)
        lamp.box((x0 - 0.19, (y0 + y1) / 2 - 0.09, z1 - 1.17), (x0 - 0.17, (y0 + y1) / 2 + 0.09, z1 - 0.98), "Lamp")
    k.covered.add(box["_i"])


# Steel ---------------------------------------------------------------------------------------------

def wall(k: Kit, box: dict):
    g = k.g("Walls")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z0 + 0.3), (x1, y1, z1), "Corrugated")
    g.box((x0 - 0.05, y0 - 0.05, z0), (x1 + 0.05, y1 + 0.05, z0 + 0.3), "Concrete", bevel=0.02)  # The kicker
    long_x = (x1 - x0) > (y1 - y0)
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
    """A grating deck on brackets."""
    g = k.g(group)
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z1 - 0.04), (x1, y1, z1), "Grating")
    long_x = (x1 - x0) >= (y1 - y0)
    for y in ((y0, y1) if long_x else ()):
        g.box((x0, y - 0.04 if y == y0 else y - 0.005, z1 - 0.18), (x1, y + 0.005 if y == y0 else y + 0.04, z1 - 0.04), "DarkSteel")
    for x in ((x0, x1) if not long_x else ()):
        g.box((x - 0.04 if x == x0 else x - 0.005, y0, z1 - 0.18), (x + 0.005 if x == x0 else x + 0.04, y1, z1 - 0.04), "DarkSteel")
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
    g = k.g("Bridges")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0 + 0.06, y0, z1 - 0.04), (x1 - 0.06, y1, z1), "Grating")
    for x in (x0, x1 - 0.06):  # Channel stringers
        g.box((x, y0, z1 - 0.3), (x + 0.06, y1, z1), "DarkSteel", bevel=0.006)
    y = y0 + 0.5
    while y < y1:
        g.box((x0 + 0.06, y - 0.03, z1 - 0.12), (x1 - 0.06, y + 0.03, z1 - 0.04), "DarkSteel")
        y += 1.0
    k.covered.add(box["_i"])


def grate_rail(k: Kit, box: dict):
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    xc = (x0 + x1) / 2
    k.g("Bridges").railing((xc, y0, z0), (xc, y1, z0), z1 - z0, "Ochre", posts_every=1.33)
    k.covered.add(box["_i"])


def grate_solid(k: Kit, box: dict):
    """A landing or a stair step: solid underneath (the collision is), concrete with a steel-nosed
    grating tread."""
    g = k.g("Bridges")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    g.box((x0, y0, z0), (x1, y1, z1 - 0.04), "Concrete", bevel=0.015)
    g.box((x0, y0, z1 - 0.04), (x1, y1, z1), "Grating")
    for (a, b) in (((x0, y0), (x1, y0 + 0.05)), ((x0, y1 - 0.05), (x1, y1)), ((x0, y0), (x0 + 0.05, y1)), ((x1 - 0.05, y0), (x1, y1))):
        g.box((a[0], a[1], z1 - 0.05), (b[0], b[1], z1 + 0.003), "Hazard")
    k.covered.add(box["_i"])


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


def scrap_heap(k: Kit, box: dict):
    """A mound of cut steel: plates, beams, pipes and offcuts piled to the heap's shape."""
    g = k.g("Heaps")
    (x0, y0, z0), (x1, y1, z1) = box["min"], box["max"]
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    rx, ry, h = (x1 - x0) / 2, (y1 - y0) / 2, z1 - z0
    r = rng(box["label"], cx, cy)
    g.box((cx - rx * 0.7, cy - ry * 0.7, z0), (cx + rx * 0.7, cy + ry * 0.7, z0 + h * 0.6), "Rust", bevel=0.2)  # The core
    for i in range(70):
        a = r.uniform(0, 2 * math.pi)
        d = math.sqrt(r.random())
        px, py = cx + math.cos(a) * rx * d, cy + math.sin(a) * ry * d
        pz = z0 + h * (1 - d * d) * r.uniform(0.3, 0.95)
        m = Matrix.Translation((px, py, pz)) @ Matrix.Rotation(r.uniform(0, 6.28), 4, "Z") @ Matrix.Rotation(r.uniform(-0.7, 0.7), 4, "X")
        kind = r.random()
        surf = r.choice(("Rust", "Rust", "HullSteel", "DarkSteel"))
        if kind < 0.45:
            w, l = r.uniform(0.3, 1.4), r.uniform(0.4, 1.8)
            g.box((-l / 2, -w / 2, -0.01), (l / 2, w / 2, 0.01), surf, matrix=m)
        elif kind < 0.7:
            l = r.uniform(0.8, 2.4)
            g.beam(m @ Vector((-l / 2, 0, 0)), m @ Vector((l / 2, 0, 0)), r.uniform(0.12, 0.3), r.uniform(0.08, 0.18), surf)
        elif kind < 0.9:
            l = r.uniform(0.6, 2.0)
            g.cylinder(m @ Vector((-l / 2, 0, 0)), m @ Vector((l / 2, 0, 0)), r.uniform(0.04, 0.15), surf, segments=10)
        else:
            s = r.uniform(0.15, 0.4)
            g.box((-s, -s, -s), (s, s, s), surf, bevel=0.03, matrix=m)
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
            {"Grate_Deck": grate_deck, "Grate_Rail": grate_rail}.get(label, grate_solid)(k, box)
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
                                 mesh_smooth_type="FACE", add_leaf_bones=False, bake_anim=False, path_mode="AUTO")
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
