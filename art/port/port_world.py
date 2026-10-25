"""The port round The Yard (docs/bible/03-arenas.md, docs/arenas/the-yard/overview.md): the Collar Four
Breaking Yard is one walled lot inside a working port terminal on the Basin's water, and this is that
port, from code, so that a camera pulled back from the arena sees quays, cranes, ships and a town
instead of an empty plane.

    blender -b --factory-startup -P art/port/port_world.py [-- --only Apron,Rail] [--no-texture]

Everything is in the layout's frame (metres, X east, Y north, Z up; the Yard's 4 m walls at X ±80.5,
Y ±45.5, its floor at Z 0) and nothing is drawn inside the walls. It builds:
- the apron: concrete slabs out to 500 m round the Yard (jointed and chamfered near it, big panels
  farther off) at the Yard's floor level, asphalt roads with their centre lines, container bay and
  RTG lane markings, slot drains, manholes, kerbs, the hazard kerb along the Yard's walls;
- the rail: each of the three trenches carried on from the end walls in a cut-and-cover tunnel
  under a concrete podium to X ±128 (it hides the Haulers' plenums at X ±95, Z -10), then a walled
  cutting climbing to grade by X ±330, the track out to buffer stops at the apron's edge with
  sidings, flat wagons loaded with cut hull blocks, road bridges over the cuttings, signals;
- the harbour: the quay 140 m north of the Yard, a vertical concrete wall with its coping, edge
  kerb, cell fenders, ladders, crane rails and cable trench, mooring lines to the ships; the water
  (a flat plane, slot Water) out to the horizon; two rubble breakwaters with armour, crown walls, a
  lighthouse and a beacon; rock revetments where the quay meets the shore;
- the ships at the quay: a 290 m container ship (lofted hull with bulb, flare and transom; hatch
  covers, lashing bridges, accommodation with its windows, bridge wings, funnel, lifeboat) and a
  bulk carrier cut in half for the breakers (open cut face showing the shell, frames, decks and
  bulkhead), its cut-off blocks on the quay beside a harbour mobile crane;
- the terminal: container stacks in RTG blocks and an empty depot, ship-to-shore gantry cranes,
  rubber-tyred gantries, light towers, warehouses (corrugated walls, portal frames, roller doors),
  workshops and site cabins, the gate with its canopy and booths, the office and its car park, a
  tank farm in its bund, cement silos, the perimeter fence, tractors, trailers and reach stackers;
- the land beyond: a terrain mesh to 6 km with the harbour basin, the far shore 3 km off and a ring
  of low hills, and the town's silhouettes on it (blocks, slabs, houses, sheds, a power station's
  chimneys, a cable-stayed bridge across the bay, wind turbines);
- a seamless tiling water normal map (OpenGL) and roughness map for Unreal to pan.

Each world group is one mesh, art/out/port/SM_Port_<Group>.fbx, in the layout's frame. Things that
repeat (containers in six paints and two lengths, the STS cranes, RTGs, light towers, bollards) are
one mesh each in their own frame, and art/out/port/instances.json places them. Shapes come from
art/kit/kitlib.py: material slots named after the art/surfaces surfaces plus Water, UV'd in metres,
chamfered and hard-edged near the Yard, plainer far off. Every random choice is seeded from what it
shapes, so a rebuild is the same.
"""
from __future__ import annotations

import json
import math
import os
import random
import struct
import sys
import time
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "..", "kit"), os.path.join(HERE, "..", "lib")]

import bmesh  # noqa: E402
import bpy  # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402
from kitlib import SURFACES, Group  # noqa: E402

OUT = os.path.join(HERE, "..", "out", "port")
SLOTS = SURFACES + ["Water"]

YARD = (-80.5, -45.5, 80.5, 45.5)
TRENCHES = (-30.0, 0.0, 30.0)
QUAY_Y = 185.0                    # The quay's face, 140 m north of the Yard's north wall
QUAY_X = 520.0                    # The quay runs X ±520, and the apron with it
APRON = (-QUAY_X, -500.0, QUAY_X, QUAY_Y)
WATER_Z = -3.5
SEABED_Z = -16.0
PODIUM_X = 128.0                  # The tunnels run from the end walls to here
CUTTING_X = 330.0                 # The cutting climbs from the trench floor to grade by here
TRACK_END = 505.0
RAIL_LAND, RAIL_SEA = QUAY_Y - 33.0, QUAY_Y - 3.0   # The STS cranes' rails, 30 m apart
CRANE_Y = QUAY_Y - 18.0           # Between them
NEAR = 60.0                       # Within this of the walls nothing stands taller than LOW_TOP
LOW_TOP = 38.0
PAINTS = (("HullSteel", 0.24), ("Rust", 0.2), ("HTROrange", 0.15), ("Ochre", 0.14), ("Corrugated", 0.15),
          ("DarkSteel", 0.12))
CONTAINER_H, CONTAINER_W = 2.591, 2.438
BAY, ROW = 12.6, 2.79             # A 40 ft slot and a row in a block, gaps included

ROADS = [(-160, 52, 160, 66), (-160, -66, 160, -52), (-154, -66, -140, 66), (140, -66, 154, 66),
         (-520, 94, 520, 104), (-520, 134, 520, 148), (-520, -342, 520, -330), (-12, -500, 12, -66), (-40, -500, 40, -430),
         (-404, -330, -389, 134), (389, -330, 404, 134), (-288, -330, -273, -52), (273, -330, 288, -52),
         (-288, 52, -273, 134), (273, 52, 288, 134)]


def rng(*key) -> random.Random:
    return random.Random(zlib.crc32(repr(key).encode()))


def smoothstep(e0: float, e1: float, x: float) -> float:
    t = min(1.0, max(0.0, (x - e0) / (e1 - e0)))
    return t * t * (3 - 2 * t)


def pick(r: random.Random, table) -> str:
    x = r.random() * sum(w for _, w in table)
    for name, w in table:
        x -= w
        if x <= 0:
            return name
    return table[-1][0]


def lin(a: float, b: float, step: float) -> list[float]:
    """a to b in equal steps no longer than `step`, both ends included."""
    n = max(1, math.ceil(abs(b - a) / step - 1e-9))
    return [a + (b - a) * i / n for i in range(n + 1)]


def place(x: float, y: float, z: float = 0.0, yaw: float = 0.0) -> Matrix:
    return Matrix.Translation((x, y, z)) @ Matrix.Rotation(math.radians(yaw), 4, "Z")


def frame(origin, u, v) -> Matrix:
    """A frame at origin with its X along u, Y along v and Z out of them (u x v)."""
    u, v = Vector(u).normalized(), Vector(v).normalized()
    m = Matrix((u, v, u.cross(v))).transposed().to_4x4()
    m.translation = Vector(origin)
    return m


def strut(g: Group, a, b, w: float, h: float, surface: str, up=(0, 0, 1), bevel=None):
    """A box section w by h from a to b, its h along `up` as far as the direction allows."""
    a, b = Vector(a), Vector(b)
    x = b - a
    length = x.length
    if length < 1e-4:
        return
    x.normalize()
    u = Vector(up)
    if abs(x.dot(u)) > 0.95:
        u = Vector((1, 0, 0)) if abs(x.x) < 0.9 else Vector((0, 1, 0))
    y = u.cross(x).normalized()
    m = Matrix((x, y, x.cross(y))).transposed().to_4x4()
    m.translation = a
    g.box((0, -w / 2, -h / 2), (length, w / 2, h / 2), surface, bevel=bevel, matrix=m)


def mesh_grid(g: Group, m: Matrix, pts, surface: str, cell=None, keep=None, thickness: float = 0.0):
    """A sheet through a grid of points pts[i][j] (in m's frame); a cell faces (P[i+1]-P[i]) x
    (P[j+1]-P[j]). cell(i, j) may give a cell its own surface, keep(i, j) False drops it."""
    g.sheet(m, list(range(len(pts))), list(range(len(pts[0]))), surface,
            shape=lambda i, j: (pts[i][j][0] - i, pts[i][j][1] - j, pts[i][j][2]),
            keep=keep, cell_surface=cell, thickness=thickness)


def overlap(a, b) -> bool:
    return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]


def clip(a, b):
    r = (max(a[0], b[0]), max(a[1], b[1]), min(a[2], b[2]), min(a[3], b[3]))
    return r if r[0] < r[2] and r[1] < r[3] else None


def rect_minus(rects, holes) -> list:
    """The rectangles left of `rects` once every hole is cut out (each split in up to four)."""
    out = list(rects)
    for h in holes:
        nxt = []
        for r in out:
            if not overlap(r, h):
                nxt.append(r)
                continue
            x0, y0, x1, y1 = r
            ya, yb = max(y0, h[1]), min(y1, h[3])
            for piece in ((x0, y0, x1, h[1]), (x0, h[3], x1, y1), (x0, ya, h[0], yb), (h[2], ya, x1, yb)):
                if piece[0] < piece[2] - 1e-6 and piece[1] < piece[3] - 1e-6 and clip(piece, r):
                    nxt.append(clip(piece, r))
        out = nxt
    return out


def near_walls(x: float, y: float) -> float:
    """How far a point is outside the Yard's walls (0 inside)."""
    dx = max(abs(x) - YARD[2], 0.0)
    dy = max(abs(y) - YARD[3], 0.0)
    return math.hypot(dx, dy)


def trap(u: float, pitch: float) -> float:
    """A trapezoidal sheet profile, 0 on the crowns and 1 in the pans."""
    t = (u / pitch) % 1.0
    if t < 0.15 or t > 0.85:
        return 0.0
    if t < 0.35:
        return (t - 0.15) / 0.2
    if t > 0.65:
        return (0.85 - t) / 0.2
    return 1.0


def trap_us(length: float, pitch: float) -> list[float]:
    """Sample points along a trapezoidal profile: every corner of it, and the ends."""
    out, k = [0.0], 0
    while True:
        for f in (0.15, 0.35, 0.65, 0.85, 1.0):
            u = (k + f) * pitch
            if u >= length - 1e-4:
                out.append(length)
                return out
            out.append(u)
        k += 1


def clad(g: Group, m: Matrix, length: float, rows, surface: str, pitch: float = 0.2, depth: float = 0.03,
         roof=None, keep=None, cell=None):
    """Profiled sheet over one face (frame m: u along, v up, z out), pans pressed in; `roof(u)`
    trims the top to a gable."""
    us = trap_us(length, pitch)

    def shape(u, v):
        top = roof(u) if roof else None
        dv = (min(v, top) - v) if top is not None else 0.0
        return 0.0, dv, -depth * trap(u, pitch)

    g.sheet(m, us, rows, surface, shape, keep=keep, cell_surface=cell)


def ring_rail(g: Group, centre, radius: float, z: float, surface: str = "Ochre", n: int = 24, height: float = 1.1):
    """A round handrail: posts and two rails round a circle at z."""
    c = Vector(centre)
    pts = [c + Vector((math.cos(2 * math.pi * k / n) * radius, math.sin(2 * math.pi * k / n) * radius, z)) for k in range(n + 1)]
    for p in pts[:-1]:
        g.cylinder(p, p + Vector((0, 0, height)), 0.03, surface, segments=6, bevel=0.0)
    for h in (height, height * 0.5):
        ring = [p + Vector((0, 0, h)) for p in pts]
        g.sweep(ring, [(math.cos(a) * 0.03, math.sin(a) * 0.03) for a in (i * math.pi / 3 for i in range(6))], surface,
                soft=True)


class Port:
    def __init__(self):
        self.groups: dict[str, Group] = {}
        self.instances: list[dict] = []
        self.blocks: list[tuple] = []

    def g(self, name: str) -> Group:
        if name not in self.groups:
            self.groups[name] = Group(f"SM_Port_{name}")
        return self.groups[name]

    def put(self, kind: str, x: float, y: float, z: float = 0.0, yaw: float = 0.0, scale: float = 1.0):
        self.instances.append({"type": f"SM_Port_{kind}", "location": [round(x, 3), round(y, 3), round(z, 3)],
                               "yaw": round(yaw, 2), "scale": scale})


# The plan ----------------------------------------------------------------------------------------

def terminal_plan(p: Port):
    """The container blocks: (x0, y0, kind), each 8 bays of 40 ft along X and 6 rows across, with
    its truck lane on the north side; an RTG spans each, the depot's are stacked by reach stacker."""
    for cx in (-505.0, -389.0, -273.0, 172.0, 288.0, 404.0):
        p.blocks.append((cx, 60.0, "rtg"))
    for cx in (-505.0, -389.0, -273.0, 172.0, 288.0, 404.0):
        p.blocks.append((cx, 106.0, "rtg"))
    for k in range(9):
        for cx in (-505.0, -389.0, -273.0):
            p.blocks.append((cx, -84.0 - 30.0 * k, "rtg"))
    for k in range(6):
        for cx in (288.0, 404.0):
            p.blocks.append((cx, -84.0 - 30.0 * k, "rtg"))
    for k in range(5):
        for cx in (-505.0, -389.0, -273.0):
            p.blocks.append((cx, -362.0 - 24.0 * k, "depot"))
    for cx in (-150.0, -45.0, 60.0):  # Export stacks just north of the Yard, kept low
        p.blocks.append((cx, 68.0, "low"))


BUILDINGS = {"WH1": (-145, -160, -35, -100), "WH2": (-145, -270, -35, -205), "WS": (35, -160, 145, -100),
             "WH3": (35, -270, 145, -205), "WH4": (180, -205, 268, -95), "WH5": (180, -320, 268, -225),
             "Office": (45, -445, 115, -420), "TankFarm": (330, -488, 510, -285), "Silos": (180, -420, 250, -365),
             "Park": (-150, -485, -55, -395)}


def solids(p: Port) -> list:
    """Every footprint a light tower or a manhole must keep off."""
    out = [YARD, (-PODIUM_X, -41, PODIUM_X, 41), (-520, -50, -PODIUM_X, 50), (PODIUM_X, -50, 520, 50),
           (-520, 148, 520, QUAY_Y)]
    out += [(x0 - 1, y0 - 1, x0 + 8 * BAY + 1, y0 + 24 + 1) for x0, y0, _ in p.blocks]
    out += list(BUILDINGS.values()) + ROADS
    return out


# Ground ------------------------------------------------------------------------------------------

def slabs(g: Group, rect, size: float, bevel: float, gap: float, key):
    """Concrete slabs on a grid `size` apart (aligned to the origin, so joints run on), each
    a few millimetres off level, over a dark bed showing in the joints."""
    x0, y0, x1, y1 = rect
    g.box((x0, y0, -0.7), (x1, y1, -0.03), "Slag", bevel=0.0)
    xs = sorted({x0, x1} | {k * size for k in range(math.ceil(x0 / size), math.floor(x1 / size) + 1)})
    ys = sorted({y0, y1} | {k * size for k in range(math.ceil(y0 / size), math.floor(y1 / size) + 1)})
    r = rng("slabs", key)
    for xa, xb in zip(xs, xs[1:]):
        if xb - xa < 0.05:
            continue
        for ya, yb in zip(ys, ys[1:]):
            if yb - ya < 0.05:
                continue
            dz = r.uniform(-0.004, 0.004)
            surface = "Slag" if r.random() < 0.012 else "Concrete"  # The odd patched, oil-black slab
            g.box((xa + gap / 2, ya + gap / 2, -0.4), (xb - gap / 2, yb - gap / 2, dz), surface, bevel=bevel)


def apron(p: Port):
    g = p.g("Apron")
    holes = [YARD, (80.5, -40.0, PODIUM_X, 40.0), (-PODIUM_X, -40.0, -80.5, 40.0)]
    for yt in TRENCHES:
        holes += [(PODIUM_X, yt - 3.6, CUTTING_X, yt + 3.6), (-CUTTING_X, yt - 3.6, -PODIUM_X, yt + 3.6)]
    near = (-235.0, -200.0, 235.0, QUAY_Y)
    for i, rect in enumerate(rect_minus([APRON], holes)):
        inner = clip(rect, near)
        if inner:
            slabs(g, inner, 6.0, 0.012, 0.0, ("near", i))
        for j, far in enumerate(rect_minus([rect], [near])):
            slabs(g, far, 20.0, 0.0, 0.04, ("far", i, j))
    roads(p, g, holes)
    bay_markings(p, g)
    wall_kerb(g)
    drains(p, g)


def roads(p: Port, g: Group, holes):
    """Asphalt over the slabs, with dashed centre lines and stop lines at the junctions."""
    for k, road in enumerate(ROADS):
        for piece in rect_minus([road], holes):
            x0, y0, x1, y1 = piece
            g.box((x0, y0, -0.1), (x1, y1, 0.012), "Slag", bevel=0.0)
        x0, y0, x1, y1 = road
        along_x = (x1 - x0) > (y1 - y0)
        c = (y0 + y1) / 2 if along_x else (x0 + x1) / 2
        a, b = (x0, x1) if along_x else (y0, y1)
        t = a + 2.0
        while t < b - 5.0:
            mn, mx = ((t, c - 0.075, 0.012), (t + 3.0, c + 0.075, 0.017)) if along_x else ((c - 0.075, t, 0.012), (c + 0.075, t + 3.0, 0.017))
            if not any(overlap((mn[0], mn[1], mx[0], mx[1]), h) for h in holes):
                g.box(mn, mx, "Ochre", bevel=0.0)
            t += 9.0
        for edge in ((y0 + 0.4, y1 - 0.55) if along_x else (x0 + 0.4, x1 - 0.55)):  # Edge lines
            mn, mx = ((x0, edge, 0.012), (x1, edge + 0.15, 0.016)) if along_x else ((edge, y0, 0.012), (edge + 0.15, y1, 0.016))
            for piece in rect_minus([(mn[0], mn[1], mx[0], mx[1])], holes + ROADS[:k] + ROADS[k + 1:]):
                g.box((piece[0], piece[1], 0.012), (piece[2], piece[3], 0.016), "Ochre", bevel=0.0)
    # The gate's lanes: islands and stop bars.
    for x in (-24.0, 0.0, 24.0):
        g.box((x - 1.0, -472.0, 0.0), (x + 1.0, -452.0, 0.18), "Concrete", bevel=0.03)
        g.box((x - 1.02, -472.02, 0.18), (x + 1.02, -471.0, 0.2), "Hazard", bevel=0.0)
    for x in (-36.0, -12.0, 12.0):
        g.box((x, -449.0, 0.012), (x + 24.0, -448.4, 0.018), "Hazard", bevel=0.0)


def bay_markings(p: Port, g: Group):
    """HTR orange slot lines at every bay of every block, its row lines, and the RTGs' yellow tyre
    lanes."""
    for x0, y0, kind in p.blocks:
        L, D = 8 * BAY, 6 * ROW
        for i in range(9):
            x = x0 + i * BAY
            g.box((x - 0.06, y0 + 0.6, 0.0), (x + 0.06, y0 + 0.6 + D, 0.006), "HTROrange", bevel=0.0)
        for j in range(7):
            y = y0 + 0.6 + j * ROW
            g.box((x0, y - 0.05, 0.0), (x0 + L, y + 0.05, 0.006), "HTROrange", bevel=0.0)
        if kind == "rtg":
            for y in (y0 - 0.5, y0 + 23.6):
                g.box((x0 - 6.0, y, 0.0), (x0 + L + 6.0, y + 0.3, 0.006), "Ochre", bevel=0.0)


def wall_kerb(g: Group):
    """A striped kerb along the outside of the Yard's walls, broken at the trench tunnels."""
    x0, y0, x1, y1 = YARD
    g.box((x0 - 0.3, y0 - 0.3, 0.0), (x1 + 0.3, y0, 0.22), "Hazard", bevel=0.02)
    g.box((x0 - 0.3, y1, 0.0), (x1 + 0.3, y1 + 0.3, 0.22), "Hazard", bevel=0.02)


def drains(p: Port, g: Group):
    """A slot drain along each block's truck lane and the quay road; manholes between."""
    for x0, y0, kind in p.blocks:
        y = y0 + 22.0 if kind == "rtg" else y0 - 1.5
        g.box((x0, y - 0.15, -0.05), (x0 + 8 * BAY, y + 0.15, 0.003), "Grating", bevel=0.0)
    g.box((-QUAY_X, 148.6, -0.05), (QUAY_X, 148.9, 0.003), "Grating", bevel=0.0)
    r = rng("manholes")
    keep_off = solids(p)
    n = 0
    while n < 160:
        x, y = r.uniform(-510, 510), r.uniform(-495, 135)
        if any(a[0] - 1 < x < a[2] + 1 and a[1] - 1 < y < a[3] + 1 for a in keep_off[:5] + list(BUILDINGS.values())):
            continue
        g.cylinder((x, y, -0.02), (x, y, 0.008), 0.36, "DarkSteel", segments=16, bevel=0.004)
        n += 1


# Rail --------------------------------------------------------------------------------------------

def track(g: Group, xa: float, xb: float, y: float, za: float, zb: float, sleeper: str = "Timber",
          pitch: float = 0.7, ballast: bool = True, near: bool = True):
    """Sleepers and twin rails on the Haulers' 1.5 m gauge from xa to xb, the bed rising from za to
    zb (sleeper tops), on a shouldered ballast bed."""
    if xa > xb:
        xa, xb, za, zb = xb, xa, zb, za
    length = xb - xa

    def z(x):
        return za + (zb - za) * (x - xa) / length

    r = rng("sleepers", round(xa, 1), round(y, 1))
    x = xa + pitch / 2
    while x < xb:
        j = r.uniform(-0.03, 0.03) if near else 0.0
        zz = z(x)
        g.box((x - 0.12 + j, y - 1.25, zz - 0.16), (x + 0.12 + j, y + 1.25, zz), sleeper, bevel=0.01 if near else 0.0)
        x += pitch
    for side in (-0.75, 0.75):
        g.rail((xa, y + side, za), (xb, y + side, zb))
    if ballast:
        prof = [(-2.4, -0.55), (2.4, -0.55), (1.7, -0.1), (-1.7, -0.1)]
        g.sweep([Vector((xa, y, za)), Vector((xb, y, zb))], prof, "Slag", up=(0, 0, 1))


def rail(p: Port):
    g = p.g("Rail")
    for s in (-1, 1):
        podium(g, s)
        for yt in TRENCHES:
            tunnel(g, s, yt)
            cutting(g, s, yt)
        sidings(p, g, s)
    for s in (-1, 1):
        for yt in TRENCHES:
            road_bridge(g, s * 147.0, yt)


def podium(g: Group, s: int):
    """The concrete podium over the three tunnels: deck slab, retaining walls, parapet, and on top
    a relay hut and cable troughs."""
    xa, xb = sorted((s * 80.5, s * PODIUM_X))
    g.box((xa, -40.0, 3.2), (xb, 40.0, 3.95), "Concrete", bevel=0.03)
    for y0, y1 in ((-40.0, -39.4), (39.4, 40.0)):
        g.box((xa, y0, -0.3), (xb, y1, 3.2), "Concrete", bevel=0.02)
    xo = s * PODIUM_X
    face_x = (xo - 0.6, xo) if s > 0 else (xo, xo + 0.6)
    for ya, yb in ((-40, -33.6), (-26.4, -3.6), (3.6, 26.4), (33.6, 40)):
        g.box((face_x[0], ya, -0.3), (face_x[1], yb, 3.2), "Concrete", bevel=0.02)
    # Parapet with a hazard band, all round the open sides.
    for (pa, pb) in (((xa + 0.3, -40.0), (xb - 0.3, -40.0)), ((xa + 0.3, 40.0), (xb - 0.3, 40.0)), ((xo, -40.0), (xo, 40.0))):
        a, b = Vector((*pa, 3.95)), Vector((*pb, 3.95))
        g.railing(a, b, 1.1, "Ochre", posts_every=2.0)
    hx = s * 104.0
    g.box((hx - 4.0, -12.0, 3.95), (hx + 4.0, -6.0, 6.8), "Corrugated", bevel=0.02)  # Relay hut
    g.box((hx - 4.2, -12.2, 6.8), (hx + 4.2, -5.8, 7.0), "DarkSteel", bevel=0.01)
    g.box((hx - 0.6, -6.02, 3.95), (hx + 0.4, -5.98, 6.0), "DarkSteel", bevel=0.0)
    for yt in TRENCHES:
        g.box((xa, yt + 4.5, 3.95), (xb, yt + 5.0, 4.15), "Concrete", bevel=0.01)  # Cable trough


def tunnel(g: Group, s: int, yt: float):
    """One trench's cut-and-cover tunnel: side walls on its lips, the soffit, the slab track, the
    portal lintel at the Yard's end wall with its hazard band, lamps and a cable tray."""
    xa, xb = sorted((s * 80.5, s * PODIUM_X))
    xe = sorted((s * 80.0, s * 80.5))  # Under the end wall's line the trench is the Yard's, but its sides are open
    for y0, y1 in ((yt - 3.6, yt - 3.0), (yt + 3.0, yt + 3.6)):
        g.box((xa, y0, -4.6), (xb, y1, 3.2), "Concrete", bevel=0.02)
        g.box((xe[0], y0, -4.6), (xe[1], y1, 0.0), "Concrete", bevel=0.0)
    g.box((xa, yt - 3.0, -4.6), (xb, yt + 3.0, -4.0), "Concrete", bevel=0.0)
    far = s * PODIUM_X
    fx = (far - 0.7, far) if s > 0 else (far, far + 0.7)
    g.box((fx[0], yt - 3.6, 2.6), (fx[1], yt + 3.6, 3.95), "Concrete", bevel=0.03)  # The far portal's lintel
    ff = fx[1] if s > 0 else fx[0] - 0.005
    g.box((ff, yt - 3.0, 2.6), (ff + 0.005, yt + 3.0, 3.1), "Hazard", bevel=0.0)
    yard_end = s * 80.5
    lx = (yard_end, yard_end + 0.9) if s > 0 else (yard_end - 0.9, yard_end)
    g.box((lx[0], yt - 3.6, 2.6), (lx[1], yt + 3.6, 3.95), "Concrete", bevel=0.03)  # The lintel
    face = lx[0] - 0.005 if s > 0 else lx[1]
    g.box((face, yt - 3.0, 2.6), (face + 0.005, yt + 3.0, 3.1), "Hazard", bevel=0.0)
    track(g, xa, xb, yt, -3.86, -3.86, ballast=False)
    x = xa + 6.0
    while x < xb - 2.0:
        for y, d in ((yt - 3.0, 1), (yt + 3.0, -1)):
            g.box((x - 0.3, y if d > 0 else y - 0.12, 1.8), (x + 0.3, y + 0.12 if d > 0 else y, 2.0), "Lamp", bevel=0.0)
        x += 9.0
    g.box((xa, yt + 2.75, 2.3), (xb, yt + 3.0, 2.4), "DarkSteel", bevel=0.0)  # Cable tray


def cutting(g: Group, s: int, yt: float):
    """The open cutting from the far portal to grade: walls with a parapet and fence, the track
    climbing on its ballast, then on at grade to the buffer stop."""
    xa, xb = s * PODIUM_X, s * CUTTING_X
    lo, hi = sorted((xa, xb))
    for y0, y1 in ((yt - 3.6, yt - 3.0), (yt + 3.0, yt + 3.6)):
        g.sweep([Vector((lo, (y0 + y1) / 2, 0.0)), Vector((hi, (y0 + y1) / 2, 0.0))],
                [(-0.3, -4.7), (0.3, -4.7), (0.3, 0.9), (-0.3, 0.9)], "Concrete", up=(0, 0, 1))
        for ya, yb in ((lo, s * 140.0), (s * 154.0, hi)):
            a, b = sorted((ya, yb))
            yy = y1 if y0 > yt else y0
            g.railing(Vector((a + 0.5, yy, 0.9)), Vector((b - 0.5, yy, 0.9)), 1.2, "DarkSteel", posts_every=2.5,
                      mid=True)
    g.sweep([Vector((xa, yt, -4.0)), Vector((xb, yt, 0.0))], [(-3.0, -0.6), (3.0, -0.6), (3.0, 0.0), (-3.0, 0.0)], "Concrete",
            up=(0, 0, 1))
    xm = s * 180.0  # Chamfered sleepers where the Yard can see them, plain beyond
    track(g, xa, xm, yt, -3.86, -3.86 + 4.0 * (180.0 - PODIUM_X) / (CUTTING_X - PODIUM_X), sleeper="Concrete", pitch=0.65)
    track(g, xm, xb, yt, -3.86 + 4.0 * (180.0 - PODIUM_X) / (CUTTING_X - PODIUM_X), 0.14, sleeper="Concrete", pitch=0.65,
          near=False)
    track(g, xb, s * TRACK_END, yt, 0.14, 0.14, sleeper="Concrete", pitch=0.65, near=False)
    buffer_stop(g, s * TRACK_END, yt, s)
    signal(g, s * (PODIUM_X + 6.0), yt + 3.0 if s > 0 else yt - 3.0, s)


def buffer_stop(g: Group, x: float, y: float, s: int):
    g.box((x - 0.6, y - 1.6, 0.0), (x + 0.6, y + 1.6, 1.3), "DarkSteel", bevel=0.03)
    f = x - s * 0.65
    g.box((min(f, f - s * 0.08), y - 1.4, 0.75), (max(f, f - s * 0.08), y + 1.4, 1.05), "Hazard", bevel=0.0)
    for yy in (y - 0.9, y + 0.9):
        g.cylinder((f, yy, 0.9), (f - s * 0.4, yy, 0.9), 0.17, "DarkSteel", segments=12)


def signal(g: Group, x: float, y: float, s: int):
    """A cutting-side signal post with two aspects."""
    g.cylinder((x, y, 0.9), (x, y, 5.2), 0.09, "DarkSteel", segments=10)
    g.box((x - 0.18, y - 0.25, 4.0), (x + 0.18, y + 0.25, 5.4), "DarkSteel", bevel=0.02)
    for z in (4.4, 5.0):
        g.cylinder((x - s * 0.18, y, z), (x - s * 0.2, y, z), 0.11, "Lamp", segments=12, bevel=0.0)


def road_bridge(g: Group, xc: float, yt: float):
    """The ring road's bridge over a cutting: deck, edge beams, parapets with a hazard band."""
    g.box((xc - 7.5, yt - 4.0, -1.0), (xc + 7.5, yt + 4.0, -0.05), "Concrete", bevel=0.02)
    g.box((xc - 7.0, yt - 4.0, -0.05), (xc + 7.0, yt + 4.0, 0.012), "Slag", bevel=0.0)
    for x0, x1 in ((xc - 7.5, xc - 7.0), (xc + 7.0, xc + 7.5)):
        g.box((x0, yt - 4.0, -0.1), (x1, yt + 4.0, 1.0), "Concrete", bevel=0.02)
        g.box((x0 - 0.005, yt - 4.0, 0.7), (x1 + 0.005, yt + 4.0, 0.9), "Hazard", bevel=0.0)


def sidings(p: Port, g: Group, s: int):
    """Two sidings off the outer tracks at grade, with flat wagons of cut hull blocks waiting."""
    for yt, ys in ((-30.0, -42.0), (30.0, 42.0)):
        xa, xb = s * 350.0, s * 380.0
        track(g, s * 380.0, s * TRACK_END, ys, 0.14, 0.14, sleeper="Concrete", pitch=0.65, near=False)
        for side in (-0.75, 0.75):  # The turnout's diverging rails
            g.rail((xa, yt + side, 0.14), (xb, ys + side, 0.14))
        n = 0
        while n < 6:
            x = s * (392.0 + 15.0 * n)
            wagon(g, x, ys, s, rng("wagon", s, ys, n))
            n += 1
    for n in range(4):
        wagon(g, s * (420.0 + 15.0 * n), 0.0, s, rng("wagon", s, 0, n))


def wagon(g: Group, x: float, y: float, s: int, r: random.Random):
    """A 14 m flat wagon on two bogies with a cut hull block chained on dunnage."""
    a, b = sorted((x, x + s * 14.0))
    g.box((a, y - 1.5, 1.2), (b, y + 1.5, 1.4), "Ochre", bevel=0.01)
    for yy in (y - 1.38, y + 1.38):
        g.beam((a + 0.3, yy, 0.95), (b - 0.3, yy, 0.95), 0.5, 0.2, "Ochre")
    for bx in (a + 2.2, b - 2.2):
        g.box((bx - 1.3, y - 1.1, 0.6), (bx + 1.3, y + 1.1, 0.95), "DarkSteel", bevel=0.01)
        for wx in (bx - 0.9, bx + 0.9):
            for side in (-0.75, 0.75):
                g.cylinder((wx, y + side - 0.07, 0.76), (wx, y + side + 0.07, 0.76), 0.46, "DarkSteel", segments=16)
    for dx in (a + 3.0, b - 3.0):
        g.box((dx - 0.15, y - 1.4, 1.4), (dx + 0.15, y + 1.4, 1.6), "Timber", bevel=0.01)
    if r.random() < 0.85:
        hull_block(g, place((a + b) / 2, y, 1.6, 0.0), r.uniform(8.0, 11.0), r.uniform(2.4, 2.8), r.uniform(1.8, 2.7), r)


def hull_block(g: Group, m: Matrix, length: float, width: float, height: float, r: random.Random):
    """A block cut from a ship's side: shell plate rolled round the bilge, frames inside it every
    0.8 m showing at the cut ends, a deck plate on top, rust breaking through the paint."""
    hl, hw = length / 2, width / 2
    rad = min(height, width) * 0.7
    prof = []
    for k in range(9):
        a = math.pi / 2 * k / 8
        prof.append((hw - rad + math.sin(a) * rad, rad - math.cos(a) * rad))  # Bottom round the bilge
    prof = [(-hw, 0.0)] + prof + [(hw, height)]
    paint = r.choice(("HullSteel", "DarkSteel", "HTROrange", "Rust"))
    us = lin(-hl, hl, 1.0)
    rust = {(i, j): r.random() < 0.3 for i in range(len(us)) for j in range(len(prof))}
    pts = [[(u, py, pz) for (py, pz) in reversed(prof)] for u in us]
    mesh_grid(g, m, pts, paint, cell=lambda i, j: "Rust" if rust[i, j] else None, thickness=0.025)
    g.box((-hl, -hw, height - 0.03), (hl, hw, height), "Rust", bevel=0.0, matrix=m)
    for x in lin(-hl + 0.3, hl - 0.3, 0.8):
        ring = [(py - 0.12 if py > 0 else py, pz + 0.12 if pz < height - 0.1 else pz) for (py, pz) in prof[1:]]
        path = [m @ Vector((x, py, pz)) for (py, pz) in ring]
        g.sweep(path, [(-0.06, -0.01), (0.06, -0.01), (0.06, 0.01), (-0.06, 0.01)], "Rust", up=tuple(m.col[0].xyz))


# Harbour -----------------------------------------------------------------------------------------

def quay(p: Port):
    """The quay wall and its furniture, X ±520 along Y 140."""
    g = p.g("Quay")
    r = rng("quay")
    x = -QUAY_X
    while x < QUAY_X - 1e-3:  # The wall in 5 m cast panels, a wet band at the waterline
        x1 = min(QUAY_X, x + 5.0)
        g.box((x + 0.01, QUAY_Y - 1.6, SEABED_Z), (x1 - 0.01, QUAY_Y - r.uniform(0.0, 0.006), -1.2), "Concrete",
              bevel=0.02)
        g.box((x + 0.01, QUAY_Y - 0.004, WATER_Z - 0.8), (x1 - 0.01, QUAY_Y + 0.004, WATER_Z + r.uniform(0.6, 1.0)), "Slag",
              bevel=0.0)
        x = x1
    for side in (-1, 1):
        g.box((side * QUAY_X - 0.8, QUAY_Y - 30.0, SEABED_Z), (side * QUAY_X + 0.8, QUAY_Y, 0.0), "Concrete", bevel=0.02)
    g.box((-QUAY_X, QUAY_Y - 2.2, -1.2), (QUAY_X, QUAY_Y, -0.01), "Concrete", bevel=0.03)  # Coping beam
    g.box((-QUAY_X, QUAY_Y - 0.06, -0.3), (QUAY_X, QUAY_Y + 0.02, 0.02), "DarkSteel", bevel=0.005)  # Edge angle
    g.box((-QUAY_X, QUAY_Y - 0.55, 0.0), (QUAY_X, QUAY_Y - 0.05, 0.25), "Hazard", bevel=0.02)  # Wheel kerb
    k = 0
    x = -QUAY_X + 8.0
    while x < QUAY_X - 4.0:
        fender(g, x)
        if k % 3 == 1:
            quay_ladder(g, x + 4.0)
        x += 16.0
        k += 1
    for y in (RAIL_LAND, RAIL_SEA):  # The STS cranes' rails, hazard lines either side
        g.box((-QUAY_X, y - 0.5, -0.4), (QUAY_X, y + 0.5, 0.0), "Concrete", bevel=0.0)
        g.rail((-QUAY_X, y, -0.16), (QUAY_X, y, -0.16))
        for d in (-0.85, 0.7):
            g.box((-QUAY_X, y + d, 0.0), (QUAY_X, y + d + 0.15, 0.005), "Hazard", bevel=0.0)
        for xs in (-QUAY_X + 1.0, QUAY_X - 2.0):
            g.box((xs, y - 0.6, 0.0), (xs + 1.0, y + 0.6, 1.4), "Hazard", bevel=0.03)  # End stops
    g.box((-QUAY_X, RAIL_LAND - 2.2, -0.3), (QUAY_X, RAIL_LAND - 1.2, 0.004), "Grating", bevel=0.0)  # Cable trench lids
    revetment(g)


def fender(g: Group, x: float):
    """A cell fender: rubber cone on the wall, steel frontal frame with its pads, chains."""
    y0 = QUAY_Y
    g.box((x - 1.0, y0, -2.4), (x + 1.0, y0 + 0.08, -0.2), "DarkSteel", bevel=0.01)
    g.cylinder((x, y0 + 0.08, -1.3), (x, y0 + 1.1, -1.3), 0.75, "Rubber", segments=20)
    g.box((x - 1.4, y0 + 1.1, -3.0), (x + 1.4, y0 + 1.45, 0.1), "DarkSteel", bevel=0.02)
    for zz in (-2.6, -1.8, -1.0, -0.2):
        g.box((x - 1.3, y0 + 1.45, zz - 0.25), (x + 1.3, y0 + 1.55, zz + 0.25), "Rubber", bevel=0.02)
    for d in (-1.2, 1.2):
        g.cylinder((x + d, y0 + 0.1, -0.15), (x + d * 0.9, y0 + 1.1, -0.3), 0.035, "Rust", segments=6)


def quay_ladder(g: Group, x: float):
    """A steel ladder down the quay face from the coping to below low water, with grab hoops."""
    y0 = QUAY_Y
    for d in (-0.22, 0.22):
        g.box((x + d - 0.03, y0, -6.5), (x + d + 0.03, y0 + 0.16, -0.05), "Ochre", bevel=0.005)
        g.sweep([Vector((x + d, y0 - 0.35, 0.0)), Vector((x + d, y0 - 0.35, 0.9)), Vector((x + d, y0 + 0.05, 1.05)),
                 Vector((x + d, y0 + 0.12, 0.0))], [(math.cos(a) * 0.025, math.sin(a) * 0.025) for a in
                                                    (i * math.pi / 4 for i in range(8))], "Ochre", soft=True)
    z = -6.2
    while z < -0.2:
        g.cylinder((x - 0.22, y0 + 0.12, z), (x + 0.22, y0 + 0.12, z), 0.016, "Ochre", segments=8)
        z += 0.3


def revetment(g: Group):
    """Rock armour on the bank where the quay ends and the shore runs on north."""
    for s in (-1, 1):
        r = rng("revetment", s)
        x = QUAY_X + 2.0
        while x < 940.0:
            y = shore_y(x) + r.uniform(-6.0, 4.0)
            for _ in range(2):
                size = r.uniform(1.0, 2.4)
                z = ground(s * x, y) + size * 0.2
                m = place(s * x, y, z, r.uniform(0, 360)) @ Matrix.Rotation(r.uniform(-0.5, 0.5), 4, "X")
                g.box((-size / 2, -size * 0.4, -size * 0.35), (size / 2, size * 0.4, size * 0.35),
                      r.choice(("Slag", "Concrete", "Slag")), bevel=0.04, matrix=m)
                y += r.uniform(2.0, 6.0)
            x += r.uniform(1.6, 3.0)


def bollard(g: Group):
    """A tee-head bollard: base plate and bolts, barrel, the tee head with its lip."""
    g.box((-0.45, -0.45, 0.0), (0.45, 0.45, 0.06), "DarkSteel", bevel=0.01)
    for x in (-0.36, 0.36):
        for y in (-0.36, 0.36):
            g.cylinder((x, y, 0.06), (x, y, 0.1), 0.03, "DarkSteel", segments=6, bevel=0.0)
    g.cylinder((0, 0, 0.06), (0, 0, 0.62), 0.24, "DarkSteel", segments=20)
    g.cylinder((0, 0, 0.62), (0, 0, 0.72), 0.3, "Ochre", segments=20)
    g.box((-0.55, -0.13, 0.5), (0.55, 0.13, 0.66), "Ochre", bevel=0.03)  # The tee's horns


def water(p: Port):
    """The water: one flat plane at WATER_Z from the quay face north to the horizon, 500 m cells."""
    g = p.g("Water")
    xs = lin(-6000.0, 6000.0, 500.0)
    ys = [QUAY_Y] + lin(500.0, 6500.0, 500.0)
    pts = [[(x, y, WATER_Z) for y in ys] for x in xs]
    mesh_grid(g, Matrix(), pts, "Water")


def breakwater(p: Port):
    g = p.g("Breakwater")
    east = [(900.0, 300.0), (900.0, 900.0), (250.0, 1150.0)]
    west = [(-900.0, 300.0), (-900.0, 780.0), (-450.0, 980.0)]
    for name, line, sea in (("east", east, 1), ("west", west, -1)):
        mound(g, line, sea, name)
    lighthouse(g, Vector((250.0, 1150.0, 3.0)))
    beacon(g, Vector((-450.0, 980.0, 3.0)))


def mound(g: Group, line, sea: int, name: str):
    """A rubble mound along a polyline: crest at +3 m with a concrete road and a crown wall on the
    sea side, slopes at 1 in 1.5 to the bed, armour cubes on the sea slope, a round head at the end."""
    r = rng("mound", name)
    crest, half = 3.0, 6.0

    def prof(s):
        a = abs(s)
        return crest if a < half else crest - (a - half) / 1.5

    width = half + (crest - SEABED_Z) * 1.5
    for k, (a, b) in enumerate(zip(line, line[1:])):
        a3, b3 = Vector((*a, 0.0)), Vector((*b, 0.0))
        d = (b3 - a3)
        length = d.length
        m = frame(a3, d, Vector((-d.y, d.x, 0.0)))
        ss = lin(-width, width, 2.0)
        us = lin(-8.0, length + 8.0, 4.0)
        bumps = {}
        pts = []
        for u in us:
            row = []
            for s in ss:
                key = (round(u), round(s))
                if key not in bumps:
                    bumps[key] = r.uniform(-0.35, 0.35) if abs(s) > half else 0.0
                row.append((u, s, prof(s) + bumps[key]))
            pts.append(row)
        mesh_grid(g, m, pts, "Slag", cell=lambda i, j, ss=ss: "Concrete" if abs(ss[j]) < half - 0.5 else None)
        side = -sea  # The sea side: right of the leg for the east arm, left for the west
        g.box((-8.0, side * half - 1.2 if side > 0 else side * half, crest), (length + 8.0, side * half if side > 0 else side * half + 1.2, crest + 3.6),
              "Concrete", bevel=0.04, matrix=m)
        u = 2.0
        while u < length:
            for row in range(3):
                s = side * (half + 1.5 + row * 2.2 + r.uniform(-0.4, 0.4))
                size = r.uniform(1.8, 2.4)
                mm = m @ Matrix.Translation((u + r.uniform(-1, 1), s, prof(s) + size * 0.25)) @ \
                    Matrix.Rotation(r.uniform(0, 6.28), 4, "Z") @ Matrix.Rotation(r.uniform(-0.6, 0.6), 4, "X")
                g.box((-size / 2,) * 3, (size / 2,) * 3, "Concrete", bevel=0.04, matrix=mm)
            u += r.uniform(2.6, 3.4)
        joint = Vector((*b, 0.0))
        roundhead(g, joint, width, prof)


def roundhead(g: Group, c: Vector, width: float, prof):
    """A cone of rubble at a joint or the head, the same section as the mound."""
    angs = lin(0.0, 2 * math.pi, math.radians(10.0))
    rads = lin(0.0, width, 2.0)
    pts = [[(c.x + math.cos(a) * rr, c.y + math.sin(a) * rr, prof(rr)) for a in angs] for rr in rads]
    mesh_grid(g, Matrix(), pts, "Slag", cell=lambda i, j: "Concrete" if rads[i] < 5.0 else None)


def lighthouse(g: Group, base: Vector):
    """A round concrete tower in red and grey bands, gallery and railing, lantern, dome, door."""
    g.cylinder(base, base + Vector((0, 0, 1.2)), 5.0, "Concrete", segments=32)
    z = 1.2
    k = 0
    while z < 22.0:
        g.cylinder(base + Vector((0, 0, z)), base + Vector((0, 0, min(22.0, z + 3.6))), 3.2 - z * 0.03,
                   "HTROrange" if k % 2 == 0 else "Concrete", segments=32, bevel=0.0)
        z += 3.6
        k += 1
    g.cylinder(base + Vector((0, 0, 22.0)), base + Vector((0, 0, 22.4)), 3.4, "Concrete", segments=32)
    ring_rail(g, base, 3.2, 22.4, "DarkSteel", n=24)
    g.cylinder(base + Vector((0, 0, 22.4)), base + Vector((0, 0, 25.0)), 1.7, "Glass", segments=16, bevel=0.0)
    g.cylinder(base + Vector((0, 0, 23.0)), base + Vector((0, 0, 24.2)), 0.6, "Lamp", segments=12)
    for k in range(8):
        a = 2 * math.pi * k / 8
        p = base + Vector((math.cos(a) * 1.72, math.sin(a) * 1.72, 22.4))
        g.cylinder(p, p + Vector((0, 0, 2.6)), 0.05, "DarkSteel", segments=6, bevel=0.0)
    g.cylinder(base + Vector((0, 0, 25.0)), base + Vector((0, 0, 25.6)), 1.9, "DarkSteel", segments=16)
    g.cylinder(base + Vector((0, 0, 25.6)), base + Vector((0, 0, 26.6)), 1.0, "DarkSteel", segments=16)
    g.cylinder(base + Vector((0, 0, 26.6)), base + Vector((0, 0, 27.8)), 0.05, "DarkSteel", segments=6)
    g.box(base + Vector((-0.6, -3.25, 1.2)), base + Vector((0.6, -3.05, 3.4)), "DarkSteel", bevel=0.01)


def beacon(g: Group, base: Vector):
    """The west mole's head beacon: a lattice of four legs, a day mark and a lamp."""
    for dx, dy in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
        g.cylinder(base + Vector((dx * 1.6, dy * 1.6, 0)), base + Vector((dx * 0.5, dy * 0.5, 11.0)), 0.12, "Ochre",
                   segments=8)
    for z in (3.0, 6.0, 9.0):
        g.box(base + Vector((-1.4 + z * 0.09, -1.4 + z * 0.09, z)), base + Vector((1.4 - z * 0.09, 1.4 - z * 0.09, z + 0.12)),
              "Ochre", bevel=0.0)
    g.box(base + Vector((-0.9, -0.9, 11.0)), base + Vector((0.9, 0.9, 11.2)), "Grating", bevel=0.0)
    g.box(base + Vector((-0.6, -0.05, 11.2)), base + Vector((0.6, 0.05, 13.0)), "HTROrange", bevel=0.01)
    g.cylinder(base + Vector((0, 0.4, 11.2)), base + Vector((0, 0.4, 12.0)), 0.25, "Lamp", segments=12)


# Ships -------------------------------------------------------------------------------------------

class Hull:
    """A ship's hull form, lofted: u from the transom (0) to the stem (1), v round the section from
    one deck edge (-1) through the keel (0) to the other (+1). Sections are superellipses, boxy
    amidships and fining to V forward and aft; the keel lifts aft to the transom; the stem rakes;
    a bulb swells forward of it under the waterline."""

    def __init__(self, L, B, D, T, bow=0.74, stern=0.16, rake=7.0, bulb=0.0, transom=0.5):
        self.L, self.B, self.D, self.T = L, B, D, T
        self.bow, self.stern, self.rake, self.transom = bow, stern, rake, transom
        self.bulb_len = bulb
        self.zb, self.rb, self.rz = 0.3 * D, 0.11 * B, 0.2 * D

    def width(self, u):
        if u < self.stern:
            return 0.84 + 0.16 * math.sin(math.pi / 2 * u / self.stern)
        if u > self.bow:
            t = (u - self.bow) / (1 - self.bow)
            return max(0.0, 1 - t ** 2.2) ** 0.9
        return 1.0

    def fullness(self, u):
        n = 10.0
        n = n + (2.0 - n) * smoothstep(self.bow - 0.08, 1.0, u)
        return n + (2.6 - n) * smoothstep(self.stern + 0.1, 0.0, u)

    def keel(self, u):
        return self.D * self.transom * smoothstep(self.stern + 0.1, 0.0, u) ** 1.4

    def stem(self, z):
        bulb = self.bulb_len * math.exp(-((z - self.zb) / self.rz) ** 2) if self.bulb_len else 0.0
        return self.L - self.rake * (1 - z / self.D) ** 1.3 + bulb

    def half(self, u):
        """Half-breadth at deck level for station u."""
        return self.B / 2 * self.width(u)

    def point(self, u, v, top=None):
        side = 1.0 if v >= 0 else -1.0
        th = min(abs(v), 1.0) * math.pi / 2
        n = self.fullness(u)
        zk = self.keel(u)
        b = (top if top is not None else self.D) - zk
        a = self.half(u)
        s, c = math.sin(th), math.cos(th)
        y = a * s ** (2 / n)
        z = zk + b * (1 - c ** (2 / n))
        if self.bulb_len and u > 0.75:
            e = (1 - u) * self.stem(z)
            q = 1 - ((z - self.zb) / self.rz) ** 2
            if q > 0:
                yb = self.rb * math.sqrt(q) * math.sqrt(min(1.0, e / (1.4 * self.rb)))
                y = (y ** 4 + yb ** 4) ** 0.25
        y = max(y, 0.05)
        return u * self.stem(z), side * y, z

    def deck_edge(self, u):
        return self.point(u, 1.0)


def stations(a: float, b: float, n: int, ends: float = 0.0) -> list[float]:
    """n+1 stations from a to b, closer together at the ends by `ends` (0 even)."""
    out = []
    for i in range(n + 1):
        t = i / n
        t = t + ends * (math.sin(2 * math.pi * t) / (2 * math.pi)) * -1.0
        out.append(a + (b - a) * t)
    return out


def hull_shell(g: Group, m: Matrix, h: Hull, us, paint: str, boot: float, streaks: int = 0, key="hull",
               thickness: float = 0.0, cut_jitter: float = 0.0):
    """The shell over stations us: antifouling red below the boot-top line `boot` metres above the
    keel (a row of the grid runs along it, so the line is clean), the paint above it with rust
    running down from the deck edge in `streaks` places; and the transom. `thickness` opens it:
    an inner skin in rusted primer and the cut edge at the last station, ragged by `cut_jitter`."""
    r = rng(key)
    nb, na = 16, 6

    def boot_v(u):
        q = min(1.0, max(0.0, 1 - (boot - h.keel(u)) / (h.D - h.keel(u))))
        return math.acos(q ** (h.fullness(u) / 2)) / (math.pi / 2)

    jitter = {j: r.uniform(-cut_jitter, cut_jitter) for j in range(2 * (nb + na) + 1)}
    pts = []
    for i, u in enumerate(us):
        fb = boot_v(u)
        side = [fb + (1 - fb) * k / na for k in range(na, 0, -1)] + [fb * k / nb for k in range(nb, -1, -1)]
        vs = side + [-v for v in reversed(side[:-1])]
        row = []
        for j, v in enumerate(vs):
            x, y, z = h.point(u, v)
            if cut_jitter and i == len(us) - 1:
                x += jitter[j]
            row.append((x, y, z))
        pts.append(row)
    ncol = len(pts[0])
    rusty = set()
    for _ in range(streaks):
        i0, run = r.randrange(len(us) - 1), r.randint(1, na + 3)
        for j in range(run):
            rusty.add((i0, j) if r.random() < 0.5 else (i0, ncol - 2 - j))

    def cell(i, j):
        if na <= j < ncol - 1 - na:
            return "HTROrange"
        return "Rust" if (i, j) in rusty else None

    mesh_grid(g, m, pts, paint, cell=cell)
    if thickness:
        t = thickness

        def inward(i, x, y, z):
            a, zk = h.half(us[i]), h.keel(us[i])
            return x, y * (1 - t / max(a, 1.0)), zk + t + (z - zk) * (1 - t / (h.D - zk))

        inner = [[inward(i, *q) for q in row] for i, row in enumerate(pts)]
        mesh_grid(g, m, [row[::-1] for row in inner], "Rust")
        last = len(us) - 1
        mesh_grid(g, m, [pts[last], inner[last]], "Rust")
    half = pts[0][:nb + na + 1]  # The transom: deck edge to keel on one side, mirrored
    cap = [[(x, -y, z) for (x, y, z) in half], [(x, y, z) for (x, y, z) in half]]
    mesh_grid(g, m, cap, paint, cell=lambda i, j: "HTROrange" if j >= na else None)
    return pts


def deck_plate(g: Group, m: Matrix, h: Hull, us, surface: str, z=None):
    pts = []
    for u in us:
        x, y, zz = h.deck_edge(u)
        pts.append([(x, w * y * 0.998, z if z is not None else zz) for w in (-1.0, -0.5, 0.0, 0.5, 1.0)])
    mesh_grid(g, m, pts, surface)


def ship_rail(g: Group, m: Matrix, h: Hull, u0: float, u1: float, surface: str = "Ochre"):
    """Handrails along both deck edges from u0 to u1."""
    for side in (-1, 1):
        us = lin(u0, u1, 6.0 / h.L)
        for a, b in zip(us, us[1:]):
            pa = Vector(h.point(a, side)) + Vector((0, -side * 0.15, 0))
            pb = Vector(h.point(b, side)) + Vector((0, -side * 0.15, 0))
            g.railing(m @ pa, m @ pb, 1.1, surface, posts_every=2.0)


def windows(g: Group, m: Matrix, x0, y0, x1, y1, z0, z1, deck_h: float, pitch: float, lit: float, r: random.Random):
    """Window rows on the four faces of a block, one row a deck, glass a few cm proud."""
    z = z0 + deck_h * 0.45
    while z + 1.1 < z1:
        for (ax, ay, bx, by, nx, ny) in ((x0, y0, x1, y0, 0, -1), (x1, y0, x1, y1, 1, 0), (x1, y1, x0, y1, 0, 1),
                                         (x0, y1, x0, y0, -1, 0)):
            length = math.hypot(bx - ax, by - ay)
            n = int(length / pitch)
            for k in range(n):
                t = (k + 0.5) / n
                cx, cy = ax + (bx - ax) * t, ay + (by - ay) * t
                w = min(pitch * 0.55, 1.3)
                dx, dy = (bx - ax) / length * w / 2, (by - ay) / length * w / 2
                mn = (min(cx - dx, cx + dx) + min(nx, 0) * 0.04, min(cy - dy, cy + dy) + min(ny, 0) * 0.04, z)
                mx = (max(cx - dx, cx + dx) + max(nx, 0) * 0.04, max(cy - dy, cy + dy) + max(ny, 0) * 0.04, z + 1.0)
                g.box(mn, mx, "Lamp" if r.random() < lit else "Glass", bevel=0.0, matrix=m)
        z += deck_h


CONTAINER_SHIP = dict(L=290.0, B=42.0, D=24.0, T=12.5, x0=-302.0, y0=QUAY_Y + 1.6 + 21.0)
HULK = dict(L=190.0, B=32.0, D=17.0, T=6.5, cut=0.6, x0=166.0, y0=QUAY_Y + 1.6 + 16.0)


def container_ship_hull():
    s = CONTAINER_SHIP
    return Hull(s["L"], s["B"], s["D"], s["T"], bow=0.72, stern=0.15, rake=8.0, bulb=7.0, transom=0.5)


def container_ship(p: Port):
    """The container ship at the west berths, bow east."""
    g = p.g("ShipContainer")
    s = CONTAINER_SHIP
    h = container_ship_hull()
    m = place(s["x0"], s["y0"], WATER_Z - s["T"])
    us = sorted(set(stations(0.0, 1.0, 70, ends=0.6) + [0.995, 0.99, 0.98, 0.97, 0.003, 0.01]))
    hull_shell(g, m, h, us, "DarkSteel", boot=s["T"] + 0.6, streaks=10, key="container ship")
    deck_plate(g, m, h, us, "HullSteel")
    L, B, D = h.L, h.B, h.D
    # Forecastle bulwark and deck, mooring gear fore and aft.
    fus = [u for u in us if u >= 0.9]
    for side in (-1.0, 1.0):
        row0 = [h.point(u, side) for u in fus]
        pts = [[(x, y, z) for (x, y, z) in row0], [(x, y, z + 2.6) for (x, y, z) in row0]]
        if side < 0:
            pts = pts[::-1]
        mesh_grid(g, m, pts, "DarkSteel", thickness=0.06)
    for x, y in ((L * 0.95, 6.0), (L * 0.95, -6.0), (L * 0.93, 10.0), (L * 0.93, -10.0), (8.0, 9.0), (8.0, -9.0)):
        g.cylinder(m @ Vector((x - 1.2, y, D + 1.0)), m @ Vector((x + 1.2, y, D + 1.0)), 0.9, "DarkSteel", segments=16)
        g.box((x - 1.6, y - 1.0, D), (x + 1.6, y + 1.0, D + 0.4), "HullSteel", bevel=0.02, matrix=m)
    for side in (-1, 1):  # Hawse pipes
        x, y, z = h.point(0.965, side * 0.95)
        g.cylinder(m @ Vector((x, y, z - 1.0)), m @ Vector((x - 1.0, y - side * 0.4, z - 1.0)), 0.7, "DarkSteel", segments=16)
    # Hatch covers and lashing bridges per bay.
    for x0, x1, rows in container_bays(h):
        hw = rows * 2.5 / 2 + 0.6
        g.box((x0, -hw, D), (x1, hw, D + 1.6), "HullSteel", bevel=0.03, matrix=m)
        g.box((x0 - 0.2, -hw - 0.3, D), (x0, hw + 0.3, D + 1.0), "DarkSteel", bevel=0.01, matrix=m)
        lx = x1 + 0.5
        for zz in (D + 1.6, D + 4.2, D + 6.8):
            g.box((lx, -hw, zz - 0.1), (lx + 0.8, hw, zz), "Grating", bevel=0.0, matrix=m)
        for y in lin(-hw, hw, 5.0):
            g.box((lx, y - 0.1, D), (lx + 0.8, y + 0.1, D + 6.8), "Ochre", bevel=0.0, matrix=m)
    accommodation(g, m, h, x0=36.0, x1=54.0, z0=D, decks=7, beam=B - 3.0, key="container ship")
    funnel(g, m, 24.0, 32.0, D, D + 26.0, 7.0, "HTROrange")
    lifeboat(g, m, 12.0, 0.0, D + 6.0)
    ship_rail(g, m, h, 0.01, 0.9)
    gangway(g, m @ Vector((60.0, -h.B / 2, D + 0.5)))
    mooring(g, m, h, [(0.03, -1), (0.06, -1), (0.2, -1), (0.85, -1), (0.94, -1), (0.97, -1)])


def container_bays(h: Hull):
    """The 40 ft bays along the ship: (x0, x1, rows)."""
    out = []
    x = 60.0
    while x + 12.4 < h.L * 0.86:
        u = (x + 6.0) / h.L
        a = min(h.half(u), h.half(min(1.0, (x + 12.4) / h.L)))
        rows = max(4, int((2 * a - 2.4) / 2.5))
        out.append((x, x + 12.4, rows))
        x += 13.8
    return out


def ship_containers(p: Port):
    """The deck cargo, as instances: 4 to 6 high amidships, lower forward and aft."""
    s = CONTAINER_SHIP
    h = container_ship_hull()
    r = rng("deck cargo")
    zb = WATER_Z - s["T"] + h.D + 1.6
    for k, (x0, x1, rows) in enumerate(container_bays(h)):
        t = (x0 + x1) / 2 / h.L
        top = 6 if 0.3 < t < 0.7 else 5 if 0.2 < t < 0.8 else 4
        for j in range(rows):
            y = (j - (rows - 1) / 2) * 2.5
            tiers = max(0, top - r.choice((0, 0, 0, 1, 1, 2)) - (1 if abs(j - (rows - 1) / 2) > rows / 2 - 1.5 else 0))
            for tz in range(tiers):
                paint = pick(r, PAINTS)
                p.put(f"Container40_{paint}", s["x0"] + (x0 + x1) / 2, s["y0"] + y, zb + tz * (CONTAINER_H + 0.02), 0.0)


def accommodation(g: Group, m: Matrix, h: Hull, x0, x1, z0, decks, beam, key):
    """A deckhouse: decks of 2.9 m in light grey with their window rows, the bridge on top with
    wings to the ship's sides, a radar mast."""
    r = rng("house", key)
    dh = 2.9
    hw = beam / 2 * 0.62
    z1 = z0 + decks * dh
    g.box((x0, -hw, z0), (x1, hw, z1), "Corrugated", bevel=0.05, matrix=m)
    windows(g, m, x0, -hw, x1, hw, z0, z1, dh, 2.2, 0.15, r)
    for k in range(1, decks):
        g.box((x1, -hw, z0 + k * dh - 0.08), (x1 + 0.9, hw, z0 + k * dh), "Corrugated", bevel=0.0, matrix=m)
    bw = h.B / 2
    g.box((x0 + 2.0, -bw, z1), (x1 + 1.0, bw, z1 + 3.2), "Corrugated", bevel=0.05, matrix=m)  # Bridge with wings
    g.box((x1 + 1.0, -bw + 0.5, z1 + 1.0), (x1 + 1.06, bw - 0.5, z1 + 2.6), "Glass", bevel=0.0, matrix=m)
    for side in (-1, 1):
        g.box((x0 + 2.0, side * bw - (0.06 if side > 0 else 0), z1 + 1.0), (x1 + 1.0, side * bw + (0.06 if side < 0 else 0), z1 + 2.6),
              "Glass", bevel=0.0, matrix=m)
    g.box((x0 + 2.0, -bw, z1 + 3.2), (x1 + 1.0, bw, z1 + 3.5), "DarkSteel", bevel=0.0, matrix=m)
    xm = (x0 + x1) / 2
    for dz, rr in ((0.0, 0.25), (6.0, 0.15)):
        g.cylinder(m @ Vector((xm, 0, z1 + 3.5 + dz)), m @ Vector((xm, 0, z1 + 9.5 + dz * 0.5)), rr, "Corrugated", segments=10)
    g.box((xm - 0.2, -3.0, z1 + 8.0), (xm + 0.2, 3.0, z1 + 8.2), "Corrugated", bevel=0.0, matrix=m)
    g.box((xm - 0.3, -1.6, z1 + 9.6), (xm + 0.3, 1.6, z1 + 9.8), "DarkSteel", bevel=0.0, matrix=m)  # Radar


def funnel(g: Group, m: Matrix, x0, x1, z0, z1, w, paint):
    g.box((x0, -w / 2, z0), (x1, w / 2, z1 - 2.0), paint, bevel=0.3, matrix=m)
    g.box((x0 - 0.05, -w / 2 - 0.05, z1 - 2.0), (x1 + 0.05, w / 2 + 0.05, z1), "DarkSteel", bevel=0.1, matrix=m)
    for y in (-1.4, 1.4):
        g.cylinder(m @ Vector(((x0 + x1) / 2, y, z1 - 1.0)), m @ Vector(((x0 + x1) / 2, y, z1 + 1.2)), 0.7, "DarkSteel", segments=14)


def lifeboat(g: Group, m: Matrix, x, y, z):
    """A free-fall lifeboat on its ramp over the stern, bright orange."""
    path = [m @ Vector((x + dx, y, z + dx * 0.55)) for dx in lin(-4.5, 4.5, 0.75)]
    prof = [(math.cos(a) * 1.5, math.sin(a) * 1.3) for a in (i * 2 * math.pi / 12 for i in range(12))]
    g.sweep(path, prof, "HTROrange", soft=True, scale=lambda i: (0.35 + 0.65 * math.sin(math.pi * (i + 0.6) / 13.2) ** 0.5,) * 2)
    for side in (-1.9, 1.9):
        g.beam(m @ Vector((x - 6.0, y + side, z - 3.6)), m @ Vector((x + 5.0, y + side, z + 2.4)), 0.4, 0.2, "DarkSteel")


def gangway(g: Group, top: Vector):
    """An accommodation ladder from the deck down to the quay."""
    bottom = Vector((top.x + 10.0, QUAY_Y - 1.8, 0.3))
    strut(g, top, bottom, 1.0, 0.12, "Grating")
    for side in (-0.5, 0.5):
        a, b = top + Vector((0, side, 0)), bottom + Vector((0, side, 0))
        g.railing(a, b, 1.0, "Corrugated", posts_every=1.5)


def mooring(g: Group, m: Matrix, h: Hull, points):
    """Mooring lines from fairleads at the deck edge to the quay's bollards, sagging."""
    for u, side in points:
        x, y, z = h.point(u, side * 1.0)
        a = m @ Vector((x, y, z - 0.6))
        lead = m.col[0].x * (-14.0 if u < 0.12 else 14.0 if u > 0.9 else 6.0 if u < 0.5 else -6.0)
        bx = -QUAY_X + 16.0 + 16.0 * round((a.x + lead + QUAY_X - 16.0) / 16.0)
        b = Vector((bx, QUAY_Y - 1.3, 0.62))
        pts = []
        for k in range(17):
            t = k / 16
            q = a.lerp(b, t)
            q.z -= 1.8 * math.sin(math.pi * t)
            pts.append(q)
        g.sweep(pts, [(math.cos(a_) * 0.045, math.sin(a_) * 0.045) for a_ in (i * math.pi / 3 for i in range(6))], "Rubber",
                soft=True)


def hulk(p: Port):
    """The bulk carrier come in for the breakers, bow already cut away: what is left from the
    transom to the cut, the cut face open on its shell, frames, decks and a hold bulkhead, deck
    cranes over the holds left, the house aft; its cut-off blocks on the quay."""
    g = p.g("ShipHulk")
    s = HULK
    h = Hull(s["L"], s["B"], s["D"], s["T"], bow=0.78, stern=0.14, rake=5.0, bulb=0.0, transom=0.45)
    m = place(s["x0"], s["y0"], WATER_Z - s["T"], 180.0)
    cut = s["cut"]
    us = sorted(set(stations(0.0, cut, 40, ends=0.0) + [0.003, 0.01]))
    hull_shell(g, m, h, us, "HullSteel", boot=11.0, streaks=70, key="hulk", thickness=0.22, cut_jitter=0.35)
    deck_plate(g, m, h, [u for u in us if u <= cut - 0.004], "Rust")
    L, B, D = h.L, h.B, h.D
    xc = cut * L
    # The cut face: a hold bulkhead a frame back, the tank top, side tanks, frames at the cut.
    xb = xc - 3.0
    nv = 12
    vs = [math.sin(math.pi / 2 * k / nv) for k in range(nv, -1, -1)]
    sect = [h.point(xb / L, v) for v in vs]
    cap = [[(xb, max(abs(y) - 0.3, 0.05), z) for (x, y, z) in sect], [(xb, -max(abs(y) - 0.3, 0.05), z) for (x, y, z) in sect]]
    mesh_grid(g, m, cap, "Rust")  # Facing the cut; nobody sees its back
    for z in (1.8, D - 5.0):
        y = h.point(xb / L, 1.0)[1] - 0.4
        g.box((xb, -y, z - 0.03), (xc - 0.4, y, z), "Rust", bevel=0.0, matrix=m)
    for x in lin(xb + 0.6, xc - 0.6, 0.8):
        ring = [h.point(x / L, v) for v in lin(-1.0, 1.0, 0.08)]
        path = [m @ Vector((x, y * 0.985, z + (0.25 if z < 1 else 0))) for (_, y, z) in ring]
        g.sweep(path, [(-0.18, -0.012), (0.18, -0.012), (0.18, 0.012), (-0.18, 0.012)], "Rust", up=tuple(m.col[0].xyz))
    for side in (-1, 1):  # Hopper tanks
        y0 = side * (h.B / 2 - 0.4)
        g.prism([(0, 1.8), (side * 6.0, 1.8), (0, 7.0)], m @ Matrix.Translation((xb, y0, 0)) @ Matrix.Rotation(-math.pi / 2, 4, "Z"),
                xc - xb - 0.4, "Rust")
    # Hatch coamings over the holds left, one cover gone.
    for k, x0 in enumerate((44.0, 66.0, 88.0)):
        hw = B * 0.3
        g.box((x0, -hw, D), (x0 + 16.0, hw, D + 1.8), "Rust" if k == 2 else "Ochre", bevel=0.04, matrix=m)
        if k != 2:
            g.box((x0 + 0.2, -hw - 0.6, D + 1.8), (x0 + 15.8, hw + 0.6, D + 2.6), "HullSteel", bevel=0.05, matrix=m)
        else:
            g.box((x0 + 0.5, -hw + 0.5, D + 1.0), (x0 + 15.5, hw - 0.5, D + 1.8), "DarkSteel", bevel=0.0, matrix=m)
    for x in (62.0, 84.0):
        deck_crane(g, m, x, D)
    accommodation(g, m, h, x0=10.0, x1=24.0, z0=D, decks=5, beam=B - 2.0, key="hulk")
    funnel(g, m, 4.0, 10.0, D, D + 20.0, 5.0, "DarkSteel")
    ship_rail(g, m, h, 0.01, cut - 0.02, "Rust")
    mooring(g, m, h, [(0.03, 1), (0.07, 1), (0.25, 1), (0.5, 1), (0.56, 1)])
    g = p.g("Laydown")
    laydown(g)
    corridor_laydown(g)
    r = rng("hull blocks")
    for k, (x, y, yaw) in enumerate(((22.0, 172.0, 0.0), (40.0, 173.0, 8.0), (30.0, 161.0, -4.0))):
        for sx in (-3.0, 3.0):
            g.box((x + sx - 0.4, y - 2.0, 0.0), (x + sx + 0.4, y + 2.0, 0.8), "Timber", bevel=0.01)
        hull_block(g, place(x, y, 0.8, yaw), r.uniform(10.0, 14.0), r.uniform(3.6, 4.4), r.uniform(3.0, 4.0), r)


def laydown(g: Group):
    """The breakers' laydown between the Yard and the quay: cut hull blocks waiting for the rail on
    timber dunnage in painted bays, low, so the Yard still sees the quay over them."""
    r = rng("laydown")
    for i in range(7):
        for j in range(2):
            if r.random() < 0.2:
                continue
            x, y = -140.0 + i * 42.0 + r.uniform(-3, 3), 112.0 + j * 13.0 + r.uniform(-1.5, 1.5)
            yaw = r.uniform(-6, 6)
            m = place(x, y, 0.0, yaw)
            for dx in (-4.0, 0.0, 4.0):
                g.box((dx - 0.3, -2.4, 0.0), (dx + 0.3, 2.4, 0.5), "Timber", bevel=0.01, matrix=m)
            hull_block(g, place(x, y, 0.5, yaw), r.uniform(10.0, 15.0), r.uniform(3.4, 4.6), r.uniform(2.2, 3.6), r)
            for d in (-1, 1):
                g.box((-9.0, d * 4.0 - 0.06, 0.0), (9.0, d * 4.0 + 0.06, 0.006), "HTROrange", bevel=0.0, matrix=m)


def corridor_laydown(g: Group):
    """Blocks waiting by the cuttings for the Yard's trains, either side of the Yard."""
    r = rng("corridor laydown")
    for s in (-1, 1):
        for yc in (-15.0, 15.0):
            x = 175.0
            while x < 310.0:
                if r.random() < 0.75:
                    m = place(s * x, yc + r.uniform(-4, 4), 0.0, r.uniform(-8, 8) + 90.0 * (r.random() < 0.3))
                    for dx in (-3.5, 0.0, 3.5):
                        g.box((dx - 0.3, -2.2, 0.0), (dx + 0.3, 2.2, 0.45), "Timber", bevel=0.01, matrix=m)
                    hull_block(g, m @ Matrix.Translation((0, 0, 0.45)), r.uniform(8.0, 12.0), r.uniform(3.0, 4.2),
                               r.uniform(1.8, 3.2), r)
                x += r.uniform(16.0, 24.0)


def deck_crane(g: Group, m: Matrix, x: float, deck: float):
    """A bulker's deck crane: pedestal, slewing house, jib luffed up and stowed in its rest."""
    base = m @ Vector((x, 0, deck))
    g.cylinder(base, base + Vector((0, 0, 7.0)), 1.6, "Ochre", segments=20)
    hm = m @ Matrix.Translation((x, 0, deck + 7.0))
    g.box((-2.5, -2.2, 0.0), (3.0, 2.2, 4.0), "Ochre", bevel=0.06, matrix=hm)
    g.box((2.6, -1.0, 1.2), (3.05, 1.0, 2.6), "Glass", bevel=0.0, matrix=hm)
    for side in (-0.9, 0.9):
        strut(g, hm @ Vector((2.0, side, 1.0)), hm @ Vector((18.0, side * 0.4, 16.0)), 0.6, 0.8, "Ochre")
    g.cylinder(hm @ Vector((18.0, 0, 16.0)), hm @ Vector((18.0, 0, 6.0)), 0.04, "DarkSteel", segments=6)
    g.box(hm @ Vector((17.6, -0.4, 5.0)), hm @ Vector((18.4, 0.4, 6.0)), "Hazard", bevel=0.02)


# Cranes and machines (one mesh each, instanced) --------------------------------------------------

def sts_crane(g: Group, raised: bool = False):
    """A ship-to-shore gantry crane in its own frame (X along the quay, Y toward the water, origin
    on the ground midway between its rails at Y ∓15): four box legs on rail bogies, portal and sill
    beams with their diagonals, twin box girders with the backreach, the hinged boom (raised or
    down), the A-frame with fore and back stays, the machinery house, trolley, cab and spreader,
    walkways with handrails."""
    P = "HTROrange"
    land, sea, lx, top = -15.0, 15.0, 9.0, 42.0
    gw, gh, gx = 1.6, 3.2, 6.5
    for y in (land, sea):
        for sx in (-1, 1):
            x = sx * lx
            g.box((x - 4.6, y - 0.9, 0.95), (x + 4.6, y + 0.9, 2.4), "DarkSteel", bevel=0.03)
            for k in range(4):
                wx = x - 3.6 + k * 2.4
                g.cylinder((wx, y - 0.25, 0.48), (wx, y + 0.25, 0.48), 0.45, "DarkSteel", segments=16)
            g.box((x - 1.0, y - 1.0, 2.4), (x + 1.0, y + 1.0, top - 2.4), P, bevel=0.05)
            g.box((x - 1.02, y - 1.02, 2.4), (x + 1.02, y + 1.02, 4.4), "Hazard", bevel=0.02)
        g.box((-lx - 1.2, y - 1.2, top - 2.6), (lx + 1.2, y + 1.2, top), P, bevel=0.05)  # Portal beam
        g.box((-lx + 1.0, y - 0.7, 12.0), (lx - 1.0, y + 0.7, 13.6), P, bevel=0.03)  # Sill beam
        for sx in (-1, 1):
            strut(g, (sx * (lx - 1.0), y, 13.6), (0.0, y, top - 2.6), 0.7, 0.7, P)
    for sx in (-1, 1):  # Side ties, land to sea legs
        x = sx * lx
        g.box((x - 0.6, land + 1.0, 26.0), (x + 0.6, sea - 1.0, 27.4), P, bevel=0.03)
        strut(g, (x, land + 1.0, 27.4), (x, sea - 1.0, top - 2.6), 0.6, 0.6, P, up=(1, 0, 0))
    for sx in (-1, 1):  # The fixed girders and backreach
        x = sx * gx
        g.box((x - gw / 2, -44.0, top), (x + gw / 2, sea + 1.2, top + gh), P, bevel=0.05)
        wa, wb = sorted((x + sx * gw / 2, x + sx * (gw / 2 + 0.9)))
        g.box((wa, -44.0, top + gh - 0.08), (wb, sea + 1.2, top + gh), "Grating", bevel=0.0)
        wx = x + sx * (gw / 2 + 0.85)
        g.railing(Vector((wx, -44.0, top + gh)), Vector((wx, sea + 1.2, top + gh)), 1.1, P, posts_every=2.5)
    for y in lin(-42.0, sea, 7.0):
        g.box((-gx, y - 0.3, top + gh - 0.7), (gx, y + 0.3, top + gh - 0.1), P, bevel=0.0)
    for sx in (-1, 1):  # The A-frame
        strut(g, (sx * gx, sea, top + gh), (sx * 2.6, 12.0, 77.0), 1.3, 1.3, P)
        strut(g, (sx * gx, -13.0, top + gh), (sx * 2.6, 10.6, 77.0), 1.3, 1.3, P)
        strut(g, (sx * gx, 0.0, top + gh), (sx * 2.6, 11.0, 66.0), 0.6, 0.6, P)
    g.box((-3.4, 9.8, 76.0), (3.4, 12.8, 79.2), P, bevel=0.05)
    g.box((-1.2, 10.3, 79.2), (1.2, 12.3, 80.0), "Lamp", bevel=0.02)  # Aircraft warning light housing
    for sx in (-1, 1):
        g.cylinder((sx * 2.6, 10.6, 77.5), (sx * gx, -43.0, top + gh), 0.28, P, segments=12)  # Backstays
    # Machinery house on the backreach, and the electrical room under it.
    g.box((-7.6, -43.5, top + gh), (7.6, -28.0, top + gh + 8.0), "Corrugated", bevel=0.05)
    g.box((-7.7, -43.6, top + gh + 8.0), (7.7, -27.9, top + gh + 8.3), "DarkSteel", bevel=0.02)
    for vx in (-4.0, 0.0, 4.0):
        g.box((vx - 0.8, -40.0, top + gh + 8.3), (vx + 0.8, -38.0, top + gh + 9.3), "DarkSteel", bevel=0.02)
    g.box((-5.0, -40.0, top - 4.5), (5.0, -32.0, top), "Corrugated", bevel=0.04)
    # The boom, from its hinge out over the water.
    hinge = Vector((0.0, sea + 1.2, top + 1.6))
    g.hold()
    for sx in (-1, 1):
        x = sx * gx
        g.box((x - gw / 2, sea + 1.25, top + 0.3), (x + gw / 2, sea + 56.0, top + gh - 0.2), P, bevel=0.05)
        g.box((x - gw / 2 - 0.9 if sx < 0 else x + gw / 2, sea + 1.25, top + gh - 0.2),
              (x - gw / 2 if sx < 0 else x + gw / 2 + 0.9, sea + 56.0, top + gh - 0.12), "Grating", bevel=0.0)
    for y in lin(sea + 4.0, sea + 55.0, 7.0):
        g.box((-gx, y - 0.3, top + gh - 0.9), (gx, y + 0.3, top + gh - 0.3), P, bevel=0.0)
    g.box((-gx - 1.0, sea + 55.0, top + 0.3), (gx + 1.0, sea + 57.5, top + gh + 0.6), P, bevel=0.05)
    g.box((-0.6, sea + 57.4, top + gh), (0.6, sea + 57.6, top + gh + 0.6), "Lamp", bevel=0.0)
    boom = g.release()
    tip_a, tip_b = Vector((0, sea + 30.0, top + gh - 0.2)), Vector((0, sea + 54.0, top + gh - 0.2))
    if raised:
        rot = Matrix.Translation(hinge) @ Matrix.Rotation(math.radians(78.0), 4, "X") @ Matrix.Translation(-hinge)
        bmesh.ops.transform(g.bm, matrix=rot, verts=boom)
        tip_a, tip_b = rot @ tip_a, rot @ tip_b
    for sx in (-1, 1):  # Forestays
        for t in (tip_a, tip_b):
            g.cylinder((sx * 2.6, 12.0, 77.5), (sx * gx, t.y, t.z), 0.22, P, segments=12)
    # Trolley, cab, spreader; parked on the backreach when the boom is up.
    ty = -20.0 if raised else 36.0
    g.box((-gx - 0.8, ty - 3.0, top + gh), (gx + 0.8, ty + 3.0, top + gh + 2.2), "Ochre", bevel=0.04)
    g.box((-3.0, ty - 1.2, top - 4.0), (-0.4, ty + 1.6, top - 0.2), "Corrugated", bevel=0.04)
    g.box((-3.02, ty + 1.6, top - 3.4), (-0.38, ty + 1.65, top - 1.0), "Glass", bevel=0.0)
    g.box((-2.98, ty - 1.18, top - 4.05), (-0.42, ty + 1.58, top - 4.0), "Glass", bevel=0.0)
    sz = top - 6.0 if raised else 28.0
    for x in (-5.0, -3.0, 3.0, 5.0):
        for y in (ty - 0.8, ty + 0.8):
            g.cylinder((x * 0.6, y, top), (x, y, sz + 1.6), 0.04, "DarkSteel", segments=6, bevel=0.0)
    g.box((-2.2, ty - 1.4, sz + 0.8), (2.2, ty + 1.4, sz + 1.6), "DarkSteel", bevel=0.03)  # Headblock
    g.box((-6.1, ty - 1.25, sz), (6.1, ty + 1.25, sz + 0.8), "Ochre", bevel=0.03)  # Spreader
    for x in (-6.1, 6.1):
        g.box((x - 0.15, ty - 1.3, sz - 0.3), (x + 0.15, ty + 1.3, sz + 0.8), "Hazard", bevel=0.01)
    # Stairs up the landside leg: a ladder with landings every 6 m.
    for z in range(6, int(top) - 2, 6):
        g.box((-lx + 1.0, land - 1.8, z - 0.05), (-lx + 3.2, land - 1.0, z + 0.05), "Grating", bevel=0.0)
        g.railing(Vector((-lx + 1.0, land - 1.8, z + 0.05)), Vector((-lx + 3.2, land - 1.8, z + 0.05)), 1.0, P, posts_every=1.1)
        strut(g, (-lx + 1.2, land - 1.4, z - 6.0 + 0.05), (-lx + 3.0, land - 1.4, z), 0.8, 0.1, "Grating")


def rtg(g: Group):
    """A rubber-tyred gantry in its own frame (X along its block, Y across, origin on the ground at
    the block's centre line; tyres at Y ±11.75): sill beams on twin-tyred bogies, raked legs, top
    girders, the trolley and its cab, the spreader, a diesel house on one sill."""
    P, half, H = "Ochre", 11.75, 19.0
    for y in (-half, half):
        g.box((-7.6, y - 0.6, 1.7), (7.6, y + 0.6, 2.8), P, bevel=0.04)
        for x in (-6.2, 6.2):
            for dy in (-0.35, 0.35):
                g.cylinder((x, y + dy - 0.27, 0.82), (x, y + dy + 0.27, 0.82), 0.82, "Rubber", segments=20)
                g.cylinder((x, y + dy - 0.28, 0.82), (x, y + dy + 0.28, 0.82), 0.4, "DarkSteel", segments=12)
            g.box((x - 1.0, y - 0.8, 1.2), (x + 1.0, y + 0.8, 1.7), "DarkSteel", bevel=0.02)
            strut(g, (x, y, 2.8), (x * 0.82, y, H), 1.0, 1.1, P, up=(0, 1, 0))
            g.box((x - 0.52, y - 0.57, 2.8), (x + 0.52, y + 0.57, 4.2), "Hazard", bevel=0.01)
        g.box((-6.2, y - 0.6, H - 1.4), (6.2, y + 0.6, H), P, bevel=0.04)
        strut(g, (-5.5, y, 4.0), (5.5, y, H - 1.4), 0.4, 0.4, P)
    for x in (-5.2, 5.2):
        g.box((x - 0.6, -half - 0.9, H), (x + 0.6, half + 0.9, H + 1.6), P, bevel=0.04)
        g.railing(Vector((x, -half, H + 1.6)), Vector((x, half, H + 1.6)), 1.0, P, posts_every=2.0)
    g.box((-5.9, 2.0, H + 1.6), (5.9, 6.4, H + 3.4), P, bevel=0.04)  # Trolley
    g.box((-5.9, -half - 2.2, 2.8), (-1.0, -half - 0.6, 5.6), "Corrugated", bevel=0.04)  # Diesel house
    g.cylinder((-2.0, -half - 1.4, 5.6), (-2.0, -half - 1.4, 6.6), 0.15, "DarkSteel", segments=8)
    g.box((3.4, 2.6, H - 2.6), (5.4, 5.0, H + 0.2), "Corrugated", bevel=0.03)  # Cab
    g.box((3.38, 2.58, H - 2.0), (5.42, 2.62, H - 0.4), "Glass", bevel=0.0)
    for x in (-4.0, 4.0):
        for y in (3.4, 5.0):
            g.cylinder((x, y, H + 1.6), (x * 1.4, y, 12.8), 0.035, "DarkSteel", segments=6, bevel=0.0)
    g.box((-6.1, 3.0, 12.0), (6.1, 5.4, 12.8), P, bevel=0.03)


def light_tower(g: Group):
    """A 30 m galvanised mast on a plinth: tapered sections, ladder, head frame with eight floods."""
    g.box((-1.0, -1.0, -0.4), (1.0, 1.0, 0.5), "Concrete", bevel=0.03)
    g.cylinder((0, 0, 0.5), (0, 0, 2.0), 0.47, "Hazard", segments=16)
    g.box((-0.6, -0.6, 0.5), (0.6, 0.6, 0.55), "DarkSteel", bevel=0.005)
    for (z0, z1, rr) in ((0.55, 10.0, 0.42), (10.0, 20.0, 0.34), (20.0, 30.0, 0.26)):
        g.cylinder((0, 0, z0), (0, 0, z1), rr, "Corrugated", segments=12)
        g.cylinder((0, 0, z1 - 0.15), (0, 0, z1 + 0.05), rr + 0.06, "Corrugated", segments=12)
    for dy in (-0.2, 0.2):
        g.box((0.5, dy - 0.02, 2.5), (0.54, dy + 0.02, 29.8), "Corrugated", bevel=0.0)
    for z in lin(2.8, 29.6, 0.6):
        g.cylinder((0.52, -0.2, z), (0.52, 0.2, z), 0.012, "Corrugated", segments=6, bevel=0.0)
    g.box((-2.4, -1.0, 29.8), (2.4, 1.0, 30.0), "Grating", bevel=0.0)
    g.railing(Vector((-2.4, -1.0, 30.0)), Vector((2.4, -1.0, 30.0)), 1.0, "Corrugated", posts_every=1.2)
    g.box((-2.4, 0.9, 30.0), (2.4, 1.0, 31.6), "Corrugated", bevel=0.0)
    for k in range(8):
        x = -2.1 + k * 0.6
        hm = Matrix.Translation((x, 1.1, 31.0)) @ Matrix.Rotation(math.radians(-25.0 if k % 2 else -35.0), 4, "X")
        g.box((-0.26, 0.0, -0.3), (0.26, 0.42, 0.3), "DarkSteel", bevel=0.02, matrix=hm)
        g.box((-0.22, 0.42, -0.25), (0.22, 0.44, 0.25), "Lamp", bevel=0.0, matrix=hm)


def container(g: Group, length: float, paint: str):
    """A dry box in its own frame (X along it, origin at the floor's centre): corner posts and
    castings, side and end rails, trapezoid-corrugated sides, front end and roof, the doors with
    four locking bars, cams and handles, cross members under the floor."""
    L, W, H = length, CONTAINER_W, CONTAINER_H
    hx, hy, post, rh = L / 2, W / 2, 0.15, 0.16
    for x in (-hx, hx - post):
        for y in (-hy, hy - post):
            g.box((x, y, 0.0), (x + post, y + post, H), paint, bevel=0.008)
    for x in (-hx - 0.004, hx - 0.174):
        for y in (-hy - 0.004, hy - 0.158):
            for z in (-0.004, H - 0.114):
                g.box((x, y, z), (x + 0.178, y + 0.162, z + 0.118), paint, bevel=0.01)
    for y0, y1 in ((-hy, -hy + 0.07), (hy - 0.07, hy)):
        g.box((-hx + post, y0, 0.0), (hx - post, y1, rh), paint, bevel=0.006)
        g.box((-hx + post, y0, H - 0.1), (hx - post, y1, H), paint, bevel=0.006)
    for x0, x1 in ((-hx, -hx + 0.1), (hx - 0.1, hx)):
        g.box((x0, -hy + post, 0.0), (x1, hy - post, rh), paint, bevel=0.006)
        g.box((x0, -hy + post, H - 0.24), (x1, hy - post, H), paint, bevel=0.006)
    rows = [0.0, H - rh - 0.1]
    clad(g, frame((-hx + post, -hy + 0.035, rh), (1, 0, 0), (0, 0, 1)), L - 2 * post, rows, paint, pitch=0.278, depth=0.036)
    clad(g, frame((hx - post, hy - 0.035, rh), (-1, 0, 0), (0, 0, 1)), L - 2 * post, rows, paint, pitch=0.278, depth=0.036)
    clad(g, frame((hx - 0.06, -hy + post, rh), (0, 1, 0), (0, 0, 1)), W - 2 * post, [0.0, H - rh - 0.24], paint, pitch=0.25,
         depth=0.03)
    clad(g, frame((-hx + post, -hy + 0.07, H - 0.03), (1, 0, 0), (0, 1, 0)), L - 2 * post, [0.0, W - 0.14], paint, pitch=0.6,
         depth=0.012)
    # The doors.
    for y0, y1 in ((-hy + post, -0.005), (0.005, hy - post)):
        clad(g, frame((-hx + 0.05, y1, rh), (0, -1, 0), (0, 0, 1)), y1 - y0, [0.0, H - rh - 0.24], paint, pitch=0.3, depth=0.015)
    for y in (-0.95, -0.35, 0.35, 0.95):
        g.cylinder((-hx - 0.03, y, rh - 0.05), (-hx - 0.03, y, H - 0.19), 0.021, paint, segments=8)
        for z in (rh + 0.02, H - 0.26):
            g.box((-hx - 0.06, y - 0.05, z - 0.04), (-hx + 0.0, y + 0.05, z + 0.04), "DarkSteel", bevel=0.005)
        g.box((-hx - 0.07, y - 0.02, 1.0), (-hx - 0.03, y + 0.2 * (1 if y < 0 else -1) + 0.02, 1.05), paint, bevel=0.004)
    g.box((-hx + post, -hy + 0.07, 0.02), (hx - post, hy - 0.07, 0.15), "DarkSteel", bevel=0.0)


# Placement of the instanced kit ------------------------------------------------------------------

def stacks(p: Port):
    """Containers in every block (heights seeded per stack, the odd 20 ft pair, gaps), an RTG on
    each RTG block; the low export stacks before the Yard kept to three high."""
    for x0, y0, kind in p.blocks:
        r = rng("block", x0, y0)
        top = {"rtg": 5, "depot": 6, "low": 2, "mid": 3}[kind]
        for i in range(8):
            for j in range(6):
                if r.random() < 0.08:
                    continue
                n = min(top, max(1, int(r.triangular(1, top + 1, top * 0.75))))
                x = x0 + (i + 0.5) * BAY
                y = y0 + 0.6 + (j + 0.5) * ROW
                twenty = r.random() < 0.18
                for k in range(n):
                    z = k * (CONTAINER_H + 0.01)
                    if twenty:
                        for dx in (-3.1, 3.1):
                            p.put(f"Container20_{pick(r, PAINTS)}", x + dx, y, z, 0.0)
                    else:
                        p.put(f"Container40_{pick(r, PAINTS)}", x, y, z, r.choice((0.0, 180.0)))
        if kind == "rtg":
            p.put("RTG", x0 + r.uniform(0.2, 0.8) * 8 * BAY, y0 + 12.0, 0.0, 0.0)


def cranes(p: Port):
    for x, raised in ((-255.0, False), (-190.0, False), (-125.0, False), (-52.0, True), (300.0, True), (420.0, False)):
        p.put("STSCraneRaised" if raised else "STSCrane", x, CRANE_Y, 0.0, 0.0)


def lights(p: Port):
    """Light towers: in the gaps between block rows, along the quay road, round the Yard's ring."""
    spots = []
    for x0, y0, kind in p.blocks:
        if kind == "rtg" and y0 < 100.0 and int(round((y0 + 84.0) / 30.0)) % 2 == 0:
            spots.append((x0 + 4 * BAY, y0 - 3.0))
    for x in range(-500, 521, 100):
        spots.append((x + 0.0, 132.0))
    spots += [(-170.0, 46.0), (170.0, 46.0), (-170.0, -46.0), (170.0, -46.0), (-100.0, -76.0),
              (100.0, -76.0), (-100.0, 48.0), (100.0, 48.0), (-60.0, 116.0), (60.0, 116.0), (-25.0, -300.0), (25.0, -400.0),
              (-20.0, -180.0), (160.0, -360.0), (-200.0, -350.0), (300.0, -270.0), (450.0, -270.0)]
    for k, (x, y) in enumerate(spots):
        p.put("LightTower", x, y, 0.0, 90.0 * (k % 4))


# Buildings ---------------------------------------------------------------------------------------

def warehouse(g: Group, rect, eave: float, ridge: float, key, doors=(0.3, 0.65), open_door: int = 0):
    """A portal-frame shed over rect, its length along the longer side: concrete plinth wall,
    box-profile cladding to the eaves and up the gables, a corrugated roof with rooflights, ridge
    cap, gutters and downpipes, roller doors on the long north/east side (one open on its dark
    inside, the frames showing), personnel doors."""
    r = rng("warehouse", key)
    x0, y0, x1, y1 = rect
    along_x = (x1 - x0) >= (y1 - y0)
    L, S = (x1 - x0, y1 - y0) if along_x else (y1 - y0, x1 - x0)
    m = place(x0, (y0 + y1) / 2, 0.0, 0.0) if along_x else place((x0 + x1) / 2, y0, 0.0, 90.0)
    hs = S / 2

    def roof_z(s):
        return eave + (ridge - eave) * (1 - abs(s) / hs)

    door_w, door_h = 6.0, 6.0
    door_us = [L * f for f in doors]  # Door centres, measured from the +X end along the +Y side
    g.box((0.0, -hs, -0.2), (L, hs, 0.05), "Concrete", bevel=0.0, matrix=m)
    g.box((0.0, -hs, 0.0), (L, -hs + 0.25, 1.2), "Concrete", bevel=0.02, matrix=m)
    g.box((0.0, -hs, 0.0), (0.25, hs, 1.2), "Concrete", bevel=0.02, matrix=m)
    g.box((L - 0.25, -hs, 0.0), (L, hs, 1.2), "Concrete", bevel=0.02, matrix=m)
    edges = sorted([0.0, L] + [L - c + d * door_w / 2 for c in door_us for d in (-1, 1)])
    for a, b in zip(edges[0::2], edges[1::2]):
        g.box((a, hs - 0.25, 0.0), (b, hs, 1.2), "Concrete", bevel=0.02, matrix=m)
    # The long sides (the +Y side carries the doors) and the gables.
    for side in (-1, 1):
        o = Vector((L, hs, 1.2)) if side > 0 else Vector((0.0, -hs, 1.2))
        fm = m @ frame(o, (-1, 0, 0) if side > 0 else (1, 0, 0), (0, 0, 1))
        rows = [0.0, door_h - 1.2, eave - 1.2]

        def keep(i, j, side=side, us=trap_us(L, 0.2)):
            if side < 0 or j != 0:
                return True
            u = (us[i] + us[i + 1]) / 2
            return not any(c - door_w / 2 < u < c + door_w / 2 for c in door_us)

        clad(g, fm, L, rows, "Corrugated", keep=keep)
    for gx, d in ((0.0, (0, -1, 0)), (L, (0, 1, 0))):
        o = Vector((gx, -hs if d[1] > 0 else hs, 1.2))
        fm = m @ frame(o, d, (0, 0, 1))
        clad(g, fm, S, [0.0, eave - 1.2, ridge - 1.2], "Corrugated", roof=lambda u: roof_z(u - hs) - 1.2)
    for side in (-1, 1):  # Roof slopes, corrugated down the fall, every sixth sheet a rooflight
        a = Vector((-0.3, 0.0, ridge)) if side > 0 else Vector((L + 0.3, 0.0, ridge))
        d = Vector((1, 0, 0)) if side > 0 else Vector((-1, 0, 0))
        fall = Vector((0, side * (hs + 0.4), eave - 0.08 - ridge))
        us = trap_us(L + 0.6, 0.3)
        fm = m @ frame(a, d, fall)
        lights = {i for i in range(len(us)) if int(us[i] / 6.0) % 6 == 3}
        g.sheet(fm, us, [0.0, fall.length], "Corrugated", lambda u, v: (0.0, 0.0, 0.035 * (1 - trap(u, 0.3))),
                cell_surface=lambda i, j, lights=lights: "Glass" if i in lights else None)
    g.box((-0.4, -0.35, ridge - 0.05), (L + 0.4, 0.35, ridge + 0.25), "Corrugated", bevel=0.02, matrix=m)
    for side in (-1, 1):
        y = side * (hs + 0.5)
        g.box((-0.3, min(y, y - side * 0.3), eave - 0.4), (L + 0.3, max(y, y - side * 0.3), eave - 0.05), "DarkSteel",
              bevel=0.01, matrix=m)
        for x in lin(2.0, L - 2.0, 15.0):
            g.cylinder(m @ Vector((x, y - side * 0.15, 0.1)), m @ Vector((x, y - side * 0.15, eave - 0.3)), 0.06, "DarkSteel",
                       segments=8)
    for k, c in enumerate(door_us):  # Roller doors, frames and roller boxes
        g.box((L - c - door_w / 2 - 0.25, hs - 0.1, 0.0), (L - c - door_w / 2, hs + 0.15, door_h + 0.4), "DarkSteel", bevel=0.01,
              matrix=m)
        g.box((L - c + door_w / 2, hs - 0.1, 0.0), (L - c + door_w / 2 + 0.25, hs + 0.15, door_h + 0.4), "DarkSteel", bevel=0.01,
              matrix=m)
        g.box((L - c - door_w / 2, hs - 0.1, door_h), (L - c + door_w / 2, hs + 0.6, door_h + 0.7), "Corrugated", bevel=0.02,
              matrix=m)
        g.box((L - c - door_w / 2 - 0.3, hs, 0.0), (L - c - door_w / 2 - 0.05, hs + 0.3, 1.0), "Hazard", bevel=0.01, matrix=m)
        if k == open_door:
            g.box((L - c - door_w / 2, hs - 0.2, door_h - 0.6), (L - c + door_w / 2, hs - 0.05, door_h), "Corrugated", bevel=0.0,
                  matrix=m)
            continue
        fm = m @ frame((L - c + door_w / 2, hs + 0.02, 0.0), (-1, 0, 0), (0, 0, 1))
        g.sheet(fm, [0.0, door_w], lin(0.0, door_h, 0.1), "Corrugated", lambda u, v: (0.0, 0.0, -0.012 * ((v / 0.1) % 1.0 > 0.5)))
    for x in lin(5.0, L - 5.0, 7.5):  # The portal frames inside
        for side in (-1, 1):
            g.beam(m @ Vector((x, side * (hs - 0.3), 0.05)), m @ Vector((x, side * (hs - 0.3), eave - 0.3)), 0.45, 0.2, "DarkSteel",
                   up=(m.col[1].xyz * side))
            g.beam(m @ Vector((x, side * (hs - 0.3), eave - 0.3)), m @ Vector((x, 0.0, ridge - 0.4)), 0.5, 0.2, "DarkSteel")
    if open_door is not None:  # Stock inside the open door: pallets and coils
        c = door_us[open_door]
        for k in range(10):
            px, py = L - c + r.uniform(-8, 8), hs - r.uniform(3, 14)
            g.box(m @ Vector((px - 0.6, py - 0.5, 0.05)), m @ Vector((px + 0.6, py + 0.5, 0.15)), "Timber", bevel=0.0)
            g.box(m @ Vector((px - 0.55, py - 0.45, 0.15)), m @ Vector((px + 0.55, py + 0.45, r.uniform(0.8, 1.8))),
                  r.choice(("Corrugated", "DarkSteel", "HullSteel")), bevel=0.01)
    for x in lin(6.0, L - 6.0, 30.0):  # Personnel doors on the south side with their canopies
        g.box((x - 0.5, -hs - 0.08, 0.0), (x + 0.5, -hs + 0.02, 2.2), "DarkSteel", bevel=0.01, matrix=m)
        g.box((x - 0.9, -hs - 1.0, 2.6), (x + 0.9, -hs, 2.75), "Corrugated", bevel=0.01, matrix=m)


def cabin(g: Group, x: float, y: float, yaw: float, r: random.Random, storeys: int = 1):
    """A site cabin: steel box on blocks, windows, door, steps; stacked two high sometimes."""
    m = place(x, y, 0.0, yaw)
    for k in range(storeys):
        z = 0.3 + k * 2.75
        g.box((-6.0, -1.5, z), (6.0, 1.5, z + 2.7), r.choice(("Corrugated", "Ochre", "HullSteel")), bevel=0.03, matrix=m)
        for wx in (-4.0, -1.5, 3.5):
            g.box((wx - 0.6, -1.53, z + 1.0), (wx + 0.6, -1.49, z + 2.0), "Glass", bevel=0.0, matrix=m)
        g.box((1.0, -1.53, z + 0.05), (1.9, -1.49, z + 2.1), "DarkSteel", bevel=0.0, matrix=m)
    for wx in (-5.5, 5.5):
        g.box((wx - 0.3, -1.4, 0.0), (wx + 0.3, 1.4, 0.3), "Concrete", bevel=0.01, matrix=m)
    g.box((0.8, -2.4, 0.0), (2.1, -1.5, 0.3), "Grating", bevel=0.0, matrix=m)


def tank(g: Group, x: float, y: float, rad: float, h: float, r: random.Random):
    """A fuel tank: shell in courses, a cone roof, a stair winding up the shell, a roof handrail,
    nozzles and a pipe to the manifold."""
    c = Vector((x, y, 0.0))
    g.cylinder(c + Vector((0, 0, -0.3)), c + Vector((0, 0, 0.3)), rad + 0.8, "Concrete", segments=48)
    z = 0.3
    while z < h:
        g.cylinder(c + Vector((0, 0, z)), c + Vector((0, 0, min(h, z + 2.4))), rad, "Rust" if r.random() < 0.1 else "Corrugated",
                   segments=64, bevel=0.0)
        z += 2.4
    angs = lin(0.0, 2 * math.pi, math.radians(6.0))
    rads = [rad + 0.15, rad * 0.7, rad * 0.35, 0.0]
    pts = [[(x + math.cos(a) * rr, y + math.sin(a) * rr, h + 0.1 + (rad - rr) * 0.18) for a in angs] for rr in rads]
    mesh_grid(g, Matrix(), pts[::-1], "Corrugated")
    ring_rail(g, c, rad - 0.4, h + 0.1 + 0.4 * 0.18, "Ochre", n=40)
    a0 = r.uniform(0, 2 * math.pi)
    path = []
    for k in range(41):
        t = k / 40
        a = a0 + t * math.pi * 0.9
        path.append(c + Vector((math.cos(a) * (rad + 0.6), math.sin(a) * (rad + 0.6), 0.3 + t * h)))
    g.sweep(path, [(-0.45, -0.05), (0.45, -0.05), (0.45, 0.05), (-0.45, 0.05)], "Grating", up=(0, 0, 1))
    g.sweep([q + Vector((0, 0, 1.0)) + (q - c).normalized() * 0.4 for q in path],
            [(math.cos(b) * 0.03, math.sin(b) * 0.03) for b in (i * math.pi / 3 for i in range(6))], "Ochre", soft=True)
    for k in range(0, 41, 4):
        q = path[k] + (path[k] - c).normalized() * 0.4
        g.cylinder(q, q + Vector((0, 0, 1.0)), 0.025, "Ochre", segments=6, bevel=0.0)
    d = Vector((math.cos(a0 + 3.5), math.sin(a0 + 3.5), 0))
    g.cylinder(c + d * rad + Vector((0, 0, 1.2)), c + d * (rad + 6.0) + Vector((0, 0, 1.2)), 0.3, "HullSteel", segments=12)


def tank_farm(g: Group, rect):
    """Six tanks in a concrete bund with its wall, steps over it and a pipe rack."""
    x0, y0, x1, y1 = rect
    for (a, b) in (((x0, y0), (x1, y0)), ((x1, y0), (x1, y1)), ((x1, y1), (x0, y1)), ((x0, y1), (x0, y0))):
        g.sweep([Vector((*a, 0.0)), Vector((*b, 0.0))], [(-0.3, 0.0), (0.3, 0.0), (0.25, 1.6), (-0.25, 1.6)], "Concrete",
                up=(0, 0, 1))
    r = rng("tanks")
    for i, tx in enumerate((x0 + 36.0, x0 + 90.0, x0 + 144.0)):
        for j, ty in enumerate((y0 + 50.0, y0 + 150.0)):
            tank(g, tx, ty, 20.0, 18.0 + 2.4 * ((i + j) % 2), r)
    yp = (y0 + y1) / 2
    for x in lin(x0 + 8.0, x1 - 8.0, 6.0):
        for dy in (-1.5, 1.5):
            g.box((x - 0.15, yp + dy - 0.15, 0.0), (x + 0.15, yp + dy + 0.15, 3.0), "HullSteel", bevel=0.01)
        g.box((x - 0.12, yp - 1.7, 3.0), (x + 0.12, yp + 1.7, 3.25), "HullSteel", bevel=0.01)
    for k, dy in enumerate((-1.1, -0.4, 0.3, 1.0)):
        g.cylinder((x0 + 8.0, yp + dy, 3.25 + 0.2 + 0.05 * k), (x1 - 8.0, yp + dy, 3.25 + 0.2 + 0.05 * k), 0.18 + 0.03 * k,
                   ("HullSteel", "Ochre", "HullSteel", "DarkSteel")[k], segments=10)


def silos(g: Group, rect):
    """Four cement silos on a steel frame: hopper cones, a roof walkway with its railing, a stair
    tower, the loading bay under them."""
    x0, y0, x1, y1 = rect
    yc = (y0 + y1) / 2
    for k in range(4):
        c = Vector((x0 + 9.0 + k * 13.0, yc, 0.0))
        for dx, dy in ((-4, -4), (4, -4), (4, 4), (-4, 4)):
            g.box(c + Vector((dx - 0.3, dy - 0.3, 0.0)), c + Vector((dx + 0.3, dy + 0.3, 7.0)), "HullSteel", bevel=0.01)
        rads = [0.6, 2.5, 5.0]
        angs = lin(0.0, 2 * math.pi, math.radians(10.0))
        pts = [[(c.x + math.cos(a) * rr, c.y + math.sin(a) * rr, 4.0 + (rr - 0.6) * 0.9) for a in angs] for rr in rads]
        mesh_grid(g, Matrix(), pts[::-1], "Corrugated")
        g.cylinder(c + Vector((0, 0, 8.0)), c + Vector((0, 0, 32.0)), 5.0, "Corrugated", segments=40)
        g.cylinder(c + Vector((0, 0, 32.0)), c + Vector((0, 0, 32.6)), 5.1, "Corrugated", segments=40)
    g.box((x0 + 4.0, yc - 1.0, 32.6), (x0 + 53.0, yc + 1.0, 32.8), "Grating", bevel=0.0)
    for d in (-1.0, 1.0):
        g.railing(Vector((x0 + 4.0, yc + d, 32.8)), Vector((x0 + 53.0, yc + d, 32.8)), 1.1, "Ochre", posts_every=2.0)
    g.box((x0 + 56.0, yc - 2.0, 0.0), (x0 + 60.0, yc + 2.0, 34.0), "Ochre", bevel=0.03)


def office(g: Group, rect):
    """The terminal office: three floors in concrete with ribbon windows, a flat roof with its
    parapet and plant, a glazed entrance under a canopy, a car park with its bays and cars."""
    r = rng("office")
    x0, y0, x1, y1 = rect
    g.box((x0, y0, 0.0), (x1, y1, 12.5), "Concrete", bevel=0.05)
    for k in range(3):
        z = 1.0 + k * 3.8
        g.box((x0 - 0.05, y0 - 0.05, z), (x1 + 0.05, y1 + 0.05, z + 1.7), "Glass", bevel=0.0)
    g.box((x0 - 0.1, y0 - 0.1, 12.5), (x1 + 0.1, y1 + 0.1, 13.4), "Concrete", bevel=0.02)
    for k in range(3):
        g.box((x0 + 8.0 + k * 9.0, y0 + 6.0, 12.5), (x0 + 14.0 + k * 9.0, y0 + 12.0, 14.6), "Corrugated", bevel=0.03)
    xm = (x0 + x1) / 2
    g.box((xm - 4.0, y0 - 0.06, 0.0), (xm + 4.0, y0 + 0.0, 3.2), "Glass", bevel=0.0)
    g.box((xm - 6.0, y0 - 5.0, 3.4), (xm + 6.0, y0, 3.7), "DarkSteel", bevel=0.02)
    for dx in (-5.5, 5.5):
        g.cylinder((xm + dx, y0 - 4.6, 0.0), (xm + dx, y0 - 4.6, 3.4), 0.12, "DarkSteel", segments=10)
    for dx in (-9.0, -10.5, -12.0):
        g.cylinder((xm + dx, y0 - 6.0, 0.0), (xm + dx, y0 - 6.0, 9.0), 0.05, "Corrugated", segments=8)
    for k in range(14):  # Car park, north of the office
        x = x0 + 2.0 + k * 2.7
        g.box((x - 0.05, y1 + 4.0, 0.0), (x + 0.05, y1 + 9.0, 0.006), "Ochre", bevel=0.0)
        if r.random() < 0.7:
            car(g, x + 1.35, y1 + 6.6, 90.0 + r.choice((0.0, 180.0)), r)


def car(g: Group, x: float, y: float, yaw: float, r: random.Random):
    m = place(x, y, 0.0, yaw)
    paint = r.choice(("Corrugated", "DarkSteel", "HullSteel", "HTROrange", "Ochre", "Rust"))
    g.box((-2.2, -0.88, 0.3), (2.2, 0.88, 0.95), paint, bevel=0.08, matrix=m)
    g.box((-1.2, -0.8, 0.95), (1.0, 0.8, 1.45), paint, bevel=0.1, matrix=m)
    g.box((-1.1, -0.82, 1.0), (0.9, 0.82, 1.38), "Glass", bevel=0.0, matrix=m)
    for wx in (-1.4, 1.4):
        for wy in (-0.8, 0.8):
            g.cylinder(m @ Vector((wx, wy - 0.1, 0.32)), m @ Vector((wx, wy + 0.1, 0.32)), 0.32, "Rubber", segments=12)


def gate(g: Group):
    """The gate: a canopy over six lanes on steel columns, the booths, barrier arms, the fence's
    return to the gate posts, a sign gantry."""
    y0, y1 = -472.0, -455.0
    g.box((-38.0, y0, 6.4), (38.0, y1, 7.4), "Corrugated", bevel=0.04)
    g.box((-38.1, y0 - 0.1, 6.2), (38.1, y1 + 0.1, 6.45), "HTROrange", bevel=0.01)
    for x in (-36.0, -12.0, 12.0, 36.0):
        for y in (y0 + 1.0, y1 - 1.0):
            g.box((x - 0.25, y - 0.25, 0.0), (x + 0.25, y + 0.25, 6.2), "DarkSteel", bevel=0.01)
    for x in (-24.0, 0.0, 24.0):
        g.box((x - 0.9, -469.0, 0.18), (x + 0.9, -464.0, 2.8), "Corrugated", bevel=0.03)
        g.box((x - 0.94, -468.6, 1.2), (x + 0.94, -464.4, 2.4), "Glass", bevel=0.0)
        g.box((x - 1.0, -469.1, 2.8), (x + 1.0, -463.9, 3.0), "DarkSteel", bevel=0.01)
        for d in (-1, 1):
            bx = x + d * 1.3
            g.box((bx - 0.2, -460.0, 0.18), (bx + 0.2, -459.6, 1.2), "DarkSteel", bevel=0.01)
            g.box((min(bx, bx + d * 10.0), -459.85, 1.0), (max(bx, bx + d * 10.0), -459.75, 1.12), "Hazard", bevel=0.0)
    g.box((-30.0, -440.0, 6.0), (30.0, -439.4, 7.6), "DarkSteel", bevel=0.02)
    for x in (-30.0, 30.0):
        g.cylinder((x, -439.7, 0.0), (x, -439.7, 7.6), 0.2, "DarkSteel", segments=10)


def fence(g: Group, a, b, height: float = 2.6):
    """Security fence: posts every 3 m, a top and a bottom rail, seven line wires, a barbed outrigger."""
    a, b = Vector((*a, 0.0)), Vector((*b, 0.0))
    length = (b - a).length
    d = (b - a).normalized()
    out = Vector((d.y, -d.x, 0.0))
    n = max(1, round(length / 3.0))
    for k in range(n + 1):
        q = a.lerp(b, k / n)
        g.cylinder(q, q + Vector((0, 0, height)), 0.035, "Corrugated", segments=8, bevel=0.0)
        g.cylinder(q + Vector((0, 0, height)), q + Vector((0, 0, height + 0.45)) + out * 0.35, 0.02, "Corrugated", segments=6,
                   bevel=0.0)
    for z in (0.1, height - 0.05):
        g.cylinder(a + Vector((0, 0, z)), b + Vector((0, 0, z)), 0.022, "Corrugated", segments=8, bevel=0.0)
    for k in range(1, 8):
        z = 0.1 + (height - 0.2) * k / 8
        g.cylinder(a + Vector((0, 0, z)), b + Vector((0, 0, z)), 0.004, "Corrugated", segments=4, bevel=0.0)
    for k in range(3):
        o = out * (0.12 * (k + 1)) + Vector((0, 0, height + 0.15 * (k + 1)))
        g.cylinder(a + o, b + o, 0.006, "DarkSteel", segments=4, bevel=0.0)


def buildings(p: Port):
    g = p.g("Buildings")
    B = BUILDINGS
    warehouse(g, B["WH1"], 9.0, 12.5, "WH1", open_door=0)
    warehouse(g, B["WH2"], 10.0, 13.5, "WH2", open_door=1)
    warehouse(g, B["WS"], 11.0, 14.0, "WS", doors=(0.2, 0.45, 0.7), open_door=1)
    warehouse(g, B["WH3"], 10.0, 13.5, "WH3", open_door=None)
    warehouse(g, B["WH4"], 10.0, 13.0, "WH4", open_door=0)
    warehouse(g, B["WH5"], 10.0, 13.0, "WH5", open_door=None)
    office(g, B["Office"])
    gate(g)
    tank_farm(g, B["TankFarm"])
    silos(g, B["Silos"])
    r = rng("cabins")
    for k, x in enumerate((-130.0, -110.0, -60.0, -40.0, 40.0, 60.0, 115.0)):  # Site cabins south of the ring
        cabin(g, x, -78.0 - (k % 2) * 4.0, 0.0, r, storeys=2 if k in (2, 4) else 1)
    for x in (-126.0, -96.0):
        g.box((x - 6.0, -98.0, 0.0), (x + 6.0, -86.0, 4.5), "Corrugated", bevel=0.03)  # A lean-to store
        g.box((x - 6.3, -98.3, 4.5), (x + 6.3, -85.7, 4.8), "DarkSteel", bevel=0.01)
    for k in range(10):  # Parked trucks' bays in the truck park
        x = B["Park"][0] + 6.0 + k * 9.0
        g.box((x - 0.06, B["Park"][1] + 4.0, 0.0), (x + 0.06, B["Park"][1] + 24.0, 0.006), "Ochre", bevel=0.0)
    x0, y0, x1, y1 = APRON
    fence(g, (x0, y0), (-40.0, y0))
    fence(g, (40.0, y0), (x1, y0))
    fence(g, (x0, y1 - 30.0), (x0, y0))
    fence(g, (x1, y0), (x1, y1 - 30.0))
    fence(g, (-40.0, y0), (-40.0, -472.0))
    fence(g, (40.0, -472.0), (40.0, y0))
    for (a, b) in (((330.0, -290.0), (510.0, -290.0)), ((330.0, -290.0), (330.0, -486.0))):
        fence(g, a, b, 2.2)


def tractor(g: Group, x: float, y: float, yaw: float, r: random.Random):
    """A terminal tractor and its skeletal trailer."""
    m = place(x, y, 0.0, yaw)
    paint = r.choice(("Ochre", "HTROrange", "Corrugated"))
    g.box((4.6, -1.25, 0.6), (7.6, 1.25, 1.4), paint, bevel=0.04, matrix=m)
    g.box((6.2, -1.2, 1.4), (7.6, 1.2, 3.2), paint, bevel=0.06, matrix=m)
    g.box((7.58, -1.0, 2.0), (7.64, 1.0, 3.0), "Glass", bevel=0.0, matrix=m)
    g.cylinder(m @ Vector((6.0, -1.0, 1.4)), m @ Vector((6.0, -1.0, 3.6)), 0.08, "DarkSteel", segments=8)
    for wx in (5.0, 7.0):
        for wy in (-1.1, 1.1):
            g.cylinder(m @ Vector((wx, wy - 0.25, 0.55)), m @ Vector((wx, wy + 0.25, 0.55)), 0.55, "Rubber", segments=14)
    for wy in (-0.5, 0.5):
        g.beam(m @ Vector((-6.2, wy, 1.2)), m @ Vector((5.0, wy, 1.2)), 0.3, 0.15, "DarkSteel")
    for wx in (-4.8, -3.6):
        for wy in (-1.05, 1.05):
            g.cylinder(m @ Vector((wx, wy - 0.2, 0.5)), m @ Vector((wx, wy + 0.2, 0.5)), 0.5, "Rubber", segments=14)
    return m @ Vector((-0.6, 0.0, 1.35))


TRACTORS = [(-220.0, 141.0, 0.0), (-160.0, 138.0, 180.0), (-200.0, 162.0, 0.0), (-140.0, 166.0, 180.0), (-150.0, 59.0, 180.0),
            (60.0, -62.0, 0.0), (0.0, -250.0, 90.0), (5.0, -380.0, -90.0), (-396.0, -150.0, 90.0), (280.0, -200.0, -90.0),
            (396.0, 30.0, 90.0), (-280.0, 95.0, 0.0)]
TRACTORS += [(BUILDINGS["Park"][0] + 10.5 + k * 9.0, BUILDINGS["Park"][1] + 14.0, 90.0) for k in range(6)]  # In the park


def machines(p: Port):
    """The harbour mobile crane at the hulk, terminal tractors on the roads and in the truck park,
    reach stackers at the depot."""
    g = p.g("Machines")
    mobile_crane(g, Vector((72.0, 166.0, 0.0)))
    r = rng("tractors")
    for x, y, yaw in TRACTORS:
        tractor(g, x, y, yaw, r)
    for x, y, yaw in ((-430.0, -350.0, 90.0), (-300.0, -345.0, -90.0), (-200.0, -480.0, 0.0)):
        reach_stacker(g, x, y, yaw)


def loads(p: Port):
    """The boxes on the tractors' trailers, and the bollards along the quay."""
    r = rng("loads")
    for x, y, yaw in TRACTORS:
        if r.random() < 0.65:
            top = place(x, y, 0.0, yaw) @ Vector((-0.6, 0.0, 1.35))
            p.put(f"Container40_{pick(r, PAINTS)}", top.x, top.y, top.z, yaw)
    x = -QUAY_X + 8.0
    while x < QUAY_X - 4.0:
        p.put("Bollard", x + 8.0, QUAY_Y - 1.3, 0.0, 0.0)
        x += 16.0


def reach_stacker(g: Group, x: float, y: float, yaw: float):
    m = place(x, y, 0.0, yaw)
    g.box((-4.0, -2.0, 0.6), (4.5, 2.0, 2.4), "Ochre", bevel=0.06, matrix=m)
    g.box((0.0, -1.9, 2.4), (2.0, -0.4, 4.6), "Ochre", bevel=0.06, matrix=m)
    g.box((0.05, -1.92, 3.0), (1.95, -0.38, 4.4), "Glass", bevel=0.0, matrix=m)
    for wx in (-2.8, 3.0):
        for wy in (-1.8, 1.8):
            g.cylinder(m @ Vector((wx, wy - 0.4, 0.9)), m @ Vector((wx, wy + 0.4, 0.9)), 0.9, "Rubber", segments=16)
    strut(g, m @ Vector((-3.5, 0.4, 3.0)), m @ Vector((6.5, 0.4, 11.0)), 0.9, 1.1, "Ochre")
    g.box((5.4, -6.1, 10.0), (7.4, 6.1, 10.8), "Ochre", bevel=0.03, matrix=m)


def mobile_crane(g: Group, base: Vector):
    """A harbour mobile crane: undercarriage on outriggers, tower, the slewing house and cab, a
    lattice-braced luffing boom over the hulk with its hook block."""
    m = Matrix.Translation(base) @ Matrix.Rotation(math.radians(15.0), 4, "Z")
    g.box((-8.0, -3.0, 1.0), (8.0, 3.0, 3.2), "Ochre", bevel=0.06, matrix=m)
    for wx in lin(-6.5, 6.5, 2.2):
        for wy in (-2.6, 2.6):
            g.cylinder(m @ Vector((wx, wy - 0.3, 0.6)), m @ Vector((wx, wy + 0.3, 0.6)), 0.6, "Rubber", segments=14)
    for sx in (-1, 1):
        for sy in (-1, 1):
            strut(g, m @ Vector((sx * 5.0, sy * 2.5, 2.0)), m @ Vector((sx * 9.0, sy * 9.0, 0.6)), 0.8, 0.8, "Ochre")
            g.box(m @ Vector((sx * 9.0 - 1.2, sy * 9.0 - 1.2, 0.0)), m @ Vector((sx * 9.0 + 1.2, sy * 9.0 + 1.2, 0.3)), "Timber",
                  bevel=0.01)
    g.cylinder(m @ Vector((0, 0, 3.2)), m @ Vector((0, 0, 22.0)), 2.0, "Ochre", segments=20)
    hm = m @ Matrix.Translation((0, 0, 22.0)) @ Matrix.Rotation(math.radians(70.0), 4, "Z")
    g.box((-9.0, -3.0, 0.0), (5.0, 3.0, 4.5), "Ochre", bevel=0.06, matrix=hm)
    g.box((-11.0, -2.8, -1.0), (-9.0, 2.8, 3.0), "DarkSteel", bevel=0.04, matrix=hm)  # Counterweight
    g.box((2.0, 3.0, 8.0), (4.5, 5.0, 11.0), "Ochre", bevel=0.05, matrix=hm)  # The tower cab
    g.box((4.48, 3.02, 9.0), (4.55, 4.98, 10.8), "Glass", bevel=0.0, matrix=hm)
    g.box((2.8, 3.5, 4.5), (3.6, 4.3, 8.0), "Ochre", bevel=0.02, matrix=hm)
    tip = hm @ Vector((44.0, 0.0, 36.0))
    for dy in (-1.4, 1.4):
        foot = hm @ Vector((3.0, dy, 3.0))
        strut(g, foot, tip + (hm.to_3x3() @ Vector((0, dy * 0.3, 0))), 0.9, 1.3, "Ochre")
    for k in range(1, 12):  # The boom's lacing
        a = hm @ Vector((3.0 + 41.0 * k / 12, -1.4 + 1.0 * k / 12, 3.0 + 33.0 * k / 12))
        b = hm @ Vector((3.0 + 41.0 * (k + 0.5) / 12, 1.4 - 1.0 * k / 12, 3.0 + 33.0 * (k + 0.5) / 12))
        g.cylinder(a, b, 0.1, "Ochre", segments=8)
    strut(g, hm @ Vector((-6.0, 0.0, 4.5)), hm @ Vector((-2.0, 0.0, 14.0)), 0.8, 0.8, "Ochre")
    g.cylinder(hm @ Vector((-2.0, 0.0, 14.0)), tip, 0.08, "DarkSteel", segments=6)
    g.cylinder(tip, tip - Vector((0, 0, 30.0)), 0.05, "DarkSteel", segments=6)
    g.box(tip - Vector((0.6, 0.6, 31.4)), tip + Vector((0.6, 0.6, -30.0)), "Hazard", bevel=0.03)


# Land and town -----------------------------------------------------------------------------------

def shore_y(x: float) -> float:
    """The near shore: the quay's line, curving north beyond its ends to the river's south bank
    2 km north, which runs on east and west."""
    t = max(abs(x) - QUAY_X, 0.0) / 1400.0
    return QUAY_Y + 2000.0 * (1 - math.exp(-t * t))


def shore_slope(x: float) -> float:
    t = max(abs(x) - QUAY_X, 0.0) / 1400.0
    return 2000.0 * 2 * t * math.exp(-t * t) / 1400.0


def far_shore_y(x: float) -> float:
    return 3300.0 + 250.0 * math.sin(x / 700.0) + 110.0 * math.sin(x / 230.0 + 1.0)


_WAVES = None


def field(x: float, y: float, which: int) -> float:
    """Seeded smooth noise, about -1 to 1: a sum of plane waves from 3 km to 200 m."""
    global _WAVES
    if _WAVES is None:
        r = rng("terrain waves")
        _WAVES = []
        for _ in range(3):
            ws = []
            for k in range(7):
                a = r.uniform(0, 2 * math.pi)
                lam = 3000.0 / (1.7 ** k) * r.uniform(0.8, 1.2)
                ws.append((math.cos(a), math.sin(a), 2 * math.pi / lam, r.uniform(0, 6.28), 0.75 ** k))
            _WAVES.append(ws)
    ws = _WAVES[which]
    return sum(w * math.sin(kk * (x * ca + y * sa) + ph) for ca, sa, kk, ph, w in ws) / 2.2


def ground(x: float, y: float) -> float:
    """The terrain's height: the dredged basin and the bay's bed under the water, beaches up from
    it, the port's flat plain, and hills rising round it from 1.5 km to 4 km."""
    sy, fy = shore_y(x), far_shore_y(x)
    slope = shore_slope(x)
    if sy < y < fy:
        d = min((y - sy) / math.sqrt(1 + slope * slope), fy - y)
        natural = WATER_Z - min(14.0, 0.6 + d * 0.1)
        basin = smoothstep(1100.0, 900.0, abs(x)) * smoothstep(1200.0, 1000.0, y)
        return min(natural, SEABED_Z * basin + natural * (1 - basin)) if basin > 0 else natural
    if y <= sy:
        d = (sy - y) / math.sqrt(1 + slope * slope) if abs(x) > QUAY_X + 20.0 else 1e9
    else:
        d = y - fy
    r = math.hypot(x, y * 0.95)
    hills = smoothstep(1300.0, 2800.0, r) * (70.0 + 70.0 * field(x, y, 0)) + smoothstep(3000.0, 5000.0, r) * (90.0 + 110.0 * field(x, y, 1))
    hills += smoothstep(700.0, 1300.0, r) * 2.5 * field(x * 3.0, y * 3.0, 2)
    if y > fy:
        hills += smoothstep(0.0, 900.0, d) * (25.0 + 30.0 * field(x, y, 2))
    bank = WATER_Z + 0.5 + d * 0.12
    return min(max(hills, 0.0), bank)


def terrain(p: Port):
    """The land and the sea bed out to 6 km: a grid 20 m apart near the port, opening to 150 m at
    the edge, with the apron's rectangle left out (its edges on the grid)."""
    g = p.g("Terrain")

    def axis(a, keep):
        out = set(keep)
        v, step = 0.0, 20.0
        while v < 6200.0:
            out.add(v)
            out.add(-v)
            if v > 1200.0:
                step = min(150.0, step * 1.05)
            v += step
        return sorted(c for c in out if all(abs(c - k) > 4.0 or c == k for k in keep))

    xs = axis(0, [-QUAY_X, QUAY_X])
    ys = axis(0, [APRON[1], QUAY_Y])
    pts = [[(x, y, ground(x, y)) for y in ys] for x in xs]

    def inside(i, j):
        return APRON[0] <= xs[i] and xs[i + 1] <= APRON[2] and APRON[1] <= ys[j] and ys[j + 1] <= APRON[3]

    def cell(i, j):
        """Mud and shingle at the water, rock where it is steep or high, scrub over the rest, and
        the port's gravel near it."""
        z = pts[i][j][2]
        if z < WATER_Z + 1.2:
            return None
        steep = abs(pts[i + 1][j][2] - z) / (xs[i + 1] - xs[i]) + abs(pts[i][j + 1][2] - z) / (ys[j + 1] - ys[j])
        n = field(xs[i] * 4.0, ys[j] * 4.0, 2)
        if steep > 0.2 or (z > 60.0 and n > 0.25):
            return "Concrete"
        if math.hypot(xs[i], ys[j]) < 800.0 or n > 0.45:
            return None
        return "Timber"

    mesh_grid(g, Matrix(), pts, "Slag", cell=cell, keep=lambda i, j: not inside(i, j))


DISTRICTS = [  # (centre x, y, radius, kind, plot size, how full)
    (0.0, -2100.0, 950.0, "town", 42.0, 0.85), (-1900.0, -1300.0, 650.0, "slabs", 64.0, 0.7),
    (1800.0, -1150.0, 600.0, "industry", 80.0, 0.6), (-1500.0, 450.0, 450.0, "houses", 24.0, 0.75),
    (1500.0, 420.0, 450.0, "houses", 24.0, 0.75), (0.0, 3900.0, 1700.0, "town", 50.0, 0.8),
    (-2900.0, 3500.0, 700.0, "houses", 30.0, 0.7), (2900.0, 3400.0, 700.0, "houses", 30.0, 0.7),
    (-900.0, -1000.0, 350.0, "industry", 80.0, 0.6), (900.0, -950.0, 420.0, "houses", 24.0, 0.7),
    (-3200.0, -2600.0, 900.0, "houses", 32.0, 0.6), (3000.0, -2500.0, 900.0, "slabs", 70.0, 0.6),
    (-850.0, -700.0, 280.0, "industry", 75.0, 0.65), (850.0, -700.0, 280.0, "industry", 75.0, 0.65),
    (0.0, -900.0, 260.0, "industry", 75.0, 0.6), (-820.0, 40.0, 220.0, "industry", 75.0, 0.6),
    (820.0, 40.0, 220.0, "industry", 75.0, 0.6), (-1350.0, -2900.0, 600.0, "town", 46.0, 0.6),
    (-790.0, -260.0, 250.0, "industry", 70.0, 0.7), (790.0, -260.0, 250.0, "industry", 70.0, 0.7),
    (-420.0, -790.0, 230.0, "industry", 70.0, 0.7), (420.0, -790.0, 230.0, "industry", 70.0, 0.7),
]
HIGHWAYS = [[(0.0, -500.0), (0.0, -640.0)], [(-4500.0, -640.0), (4500.0, -640.0)],
            [(0.0, -640.0), (180.0, -1100.0), (0.0, -1600.0), (-100.0, -2800.0)],
            [(-QUAY_X, -150.0), (-900.0, -250.0), (-1500.0, 250.0), (-2300.0, 1200.0), (-4500.0, 1500.0)],
            [(QUAY_X, -150.0), (900.0, -250.0), (1500.0, 250.0), (2300.0, 1200.0), (4500.0, 1500.0)]]


def town(p: Port):
    """The town's silhouettes on the terrain: districts of blocks, slab flats, houses with pitched
    roofs and industrial sheds; windows as glass bands (some lit); a power station with two
    banded chimneys; a cable-stayed bridge over the bay's west end; wind turbines on the hills."""
    g = p.g("Town")
    for k, district in enumerate(DISTRICTS):
        district_plots(g, k, *district)
    for line in HIGHWAYS:
        highway(g, line)
    power_station(g, Vector((1950.0, -1350.0, 0.0)))
    bridge(g)
    r = rng("turbines")
    for k in range(9):
        a = math.radians(200.0 + k * 16.0)
        x, y = math.cos(a) * 3600.0, math.sin(a) * 3300.0
        turbine(g, Vector((x, y, ground(x, y) - 1.0)), r)


def district_plots(g: Group, k: int, cx, cy, rad, kind, plot, fill):
    """A district on a street grid turned to its own bearing: every fifth row and column of plots
    is a street (drawn over the ground), the rest built on, fuller and taller to the centre."""
    r = rng("district", k)
    yaw = r.uniform(-35, 35)
    ca, sa = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
    n = int(rad / plot) + 1

    def at(i, j):
        return cx + (i * ca - j * sa) * plot, cy + (i * sa + j * ca) * plot

    for i in range(-n, n + 1):
        for j in range(-n, n + 1):
            if i % 5 == 0 or j % 5 == 0:
                continue
            x, y = at(i + r.uniform(-0.08, 0.08), j + r.uniform(-0.08, 0.08))
            d = math.hypot(x - cx, y - cy) / rad
            if d > 1.0 or r.random() > fill * (1 - d * d) ** 0.35:
                continue
            if APRON[0] - 150 < x < APRON[2] + 150 and APRON[1] - 150 < y < QUAY_Y + 200:
                continue
            z = ground(x, y)
            if z < WATER_Z + 1.5 or abs(ground(x + 20, y) - ground(x - 20, y)) > 12.0:
                continue
            town_building(g, x, y, z, yaw + r.choice((0.0, 0.0, 90.0)) * (kind == "houses") + r.uniform(-2, 2), kind, r,
                          centrality=1 - d, plot=plot)
    for i in range(-n, n + 1):
        if i % 5:
            continue
        for along in (0, 1):
            half = math.sqrt(max(0.0, rad * rad - (i * plot) ** 2)) / plot
            if half < 2:
                continue
            a = at(i, -half) if along else at(-half, i)
            b = at(i, half) if along else at(half, i)
            street(g, [a, b])


def footing(x: float, y: float, rad: float) -> tuple[float, float]:
    """The ground under a footprint: its lowest and its middle height."""
    zs = [ground(x + dx * rad, y + dy * rad) for dx, dy in ((0, 0), (-1, -1), (1, -1), (1, 1), (-1, 1))]
    return min(zs), sum(zs) / len(zs)


def town_building(g: Group, x, y, z, yaw, kind, r: random.Random, centrality: float, plot: float):
    m = place(x, y, 0.0, yaw)
    low, z = footing(x, y, plot * 0.4)
    base = low - 2.0
    if kind == "houses":
        w, d, h = r.uniform(0.4, 0.6) * plot, r.uniform(0.32, 0.45) * plot, r.uniform(5.5, 8.5)
        wall = r.choice(("Concrete", "Concrete", "Corrugated", "Ochre"))
        g.box((-w / 2, -d / 2, base), (w / 2, d / 2, z + h), wall, bevel=0.0, matrix=m)
        g.box((-w / 2 - 0.03, -d / 2 - 0.03, z + 1.0), (w / 2 + 0.03, d / 2 + 0.03, z + 2.0),
              "Lamp" if r.random() < 0.25 else "Glass", bevel=0.0, matrix=m)
        roof = r.choice(("Rust", "HTROrange", "DarkSteel", "Rust"))
        g.prism([(-d / 2 - 0.4, 0.0), (d / 2 + 0.4, 0.0), (0.0, d * 0.4)],
                m @ Matrix.Translation((-w / 2 - 0.3, 0, z + h)) @ Matrix.Rotation(-math.pi / 2, 4, "Z"), w + 0.6, roof)
        return
    if kind == "industry":
        w, d, h = r.uniform(0.4, 0.8) * plot, r.uniform(0.3, 0.6) * plot, r.uniform(7, 14)
        g.box((-w / 2, -d / 2, base), (w / 2, d / 2, z + h), r.choice(("Corrugated", "Corrugated", "HullSteel", "Rust")), bevel=0.0,
              matrix=m)
        g.box((-w / 2 - 0.2, -d / 2 - 0.2, z + h), (w / 2 + 0.2, d / 2 + 0.2, z + h + 0.6), "DarkSteel", bevel=0.0, matrix=m)
        if r.random() < 0.15:
            g.cylinder(m @ Vector((w / 2 - 3, 0, z + h)), m @ Vector((w / 2 - 3, 0, z + h + r.uniform(15, 45))), r.uniform(0.8, 1.6),
                       "Concrete", segments=12, bevel=0.0)
        return
    if kind == "slabs":
        w, d, storeys = r.uniform(0.6, 0.85) * plot, r.uniform(12, 16), r.randint(5, 14)
    else:
        tall = centrality ** 2
        w, d = r.uniform(0.45, 0.75) * plot, r.uniform(0.4, 0.7) * plot
        storeys = max(2, int(r.triangular(2, 5 + 24 * tall, 3 + 9 * tall)))
    h = storeys * 3.1
    g.box((-w / 2, -d / 2, base), (w / 2, d / 2, z + h), r.choice(("Concrete", "Concrete", "Concrete", "Corrugated", "Ochre")),
          bevel=0.0, matrix=m)
    lit = r.uniform(0.05, 0.4)
    for s in range(storeys):
        zz = z + 0.9 + s * 3.1
        g.box((-w / 2 - 0.04, -d / 2 - 0.04, zz), (w / 2 + 0.04, d / 2 + 0.04, zz + 1.4), "Lamp" if r.random() < lit else "Glass",
              bevel=0.0, matrix=m)
    g.box((-w / 2 - 0.15, -d / 2 - 0.15, z + h), (w / 2 + 0.15, d / 2 + 0.15, z + h + 0.8), "Concrete", bevel=0.0, matrix=m)
    if r.random() < 0.5:
        g.box((-2.0, -2.0, z + h), (2.0, 2.0, z + h + 3.0), "Corrugated", bevel=0.0, matrix=m)


def street(g: Group, line):
    """A town street over the ground."""
    pts = []
    a, b = line
    for t in lin(0.0, 1.0, 40.0 / max(1.0, math.dist(a, b))):
        x, y = a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t
        z = ground(x, y)
        if z < WATER_Z + 1.0:
            if len(pts) > 1:
                g.sweep(pts, [(-6.0, -2.0), (6.0, -2.0), (4.5, 0.0), (-4.5, 0.0)], "Slag", up=(0, 0, 1))
            pts = []
            continue
        pts.append(Vector((x, y, z + 0.2)))
    if len(pts) > 1:
        g.sweep(pts, [(-6.0, -2.0), (6.0, -2.0), (4.5, 0.0), (-4.5, 0.0)], "Slag", up=(0, 0, 1))


def highway(g: Group, line):
    """A two-lane road over the ground along a polyline: asphalt on a shallow bank, its centre
    line; over water it rides on a causeway."""
    pts = []
    for a, b in zip(line, line[1:]):
        for t in lin(0.0, 1.0, 40.0 / math.dist(a, b))[:-1]:
            x, y = a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t
            pts.append(Vector((x, y, max(ground(x, y), WATER_Z + 2.0) + 0.25)))
    pts.append(Vector((*line[-1], max(ground(*line[-1]), WATER_Z + 2.0) + 0.25)))
    g.sweep(pts, [(-9.0, -3.0), (9.0, -3.0), (6.0, 0.0), (-6.0, 0.0)], "Slag", up=(0, 0, 1))
    g.sweep([q + Vector((0, 0, 0.02)) for q in pts], [(-0.08, -0.01), (0.08, -0.01), (0.08, 0.0), (-0.08, 0.0)], "Ochre",
            up=(0, 0, 1))


def power_station(g: Group, c: Vector):
    """Turbine hall, boiler house, two chimneys banded at the top, cooling plant and a coal shed."""
    low, mid = footing(c.x, c.y, 120.0)
    c = Vector((c.x, c.y, mid))
    g.box(c + Vector((-60, -25, low - mid - 3)), c + Vector((60, 25, 28)), "Corrugated", bevel=0.0)
    g.box(c + Vector((-50, 25, -3)), c + Vector((40, 55, 50)), "Concrete", bevel=0.0)
    for k, dx in enumerate((-30.0, 20.0)):
        b = c + Vector((dx, 85.0, -3.0))
        g.cylinder(b, b + Vector((0, 0, 150.0)), 5.0, "Concrete", segments=24, bevel=0.0)
        for s in range(5):
            g.cylinder(b + Vector((0, 0, 120.0 + s * 6.0)), b + Vector((0, 0, 126.0 + s * 6.0)), 5.05,
                       "HTROrange" if s % 2 == 0 else "Concrete", segments=24, bevel=0.0)
        g.cylinder(b + Vector((0, 0, 150.0)), b + Vector((0, 0, 151.0)), 5.4, "DarkSteel", segments=24, bevel=0.0)
        g.cylinder(b + Vector((0, 0, 148.5)), b + Vector((0, 0, 149.0)), 5.2, "Lamp", segments=24, bevel=0.0)
    g.box(c + Vector((80, -60, -3)), c + Vector((220, 0, 22)), "Rust", bevel=0.0)


def bridge(g: Group):
    """A cable-stayed bridge across the bay's west end, along Y at X -2300: two H pylons, the deck
    girder, fans of stays, approach viaduct piers."""
    x = -2300.0
    y0, y1 = shore_y(x) - 160.0, far_shore_y(x) + 160.0
    deck = 42.0
    towers = (y0 + (y1 - y0) * 0.36, y0 + (y1 - y0) * 0.64)
    g.box((x - 13.0, y0, deck - 3.0), (x + 13.0, y1, deck), "Concrete", bevel=0.0)
    g.box((x - 13.2, y0, deck), (x + 13.2, y1, deck + 1.2), "DarkSteel", bevel=0.0)
    for yp in lin(y0 + 40.0, y1 - 40.0, 70.0):
        if any(abs(yp - t) < 120 for t in towers):
            continue
        zb = min(ground(x, yp), WATER_Z) - 4.0
        for dx in (-7.0, 7.0):
            g.box((x + dx - 2.0, yp - 2.0, zb), (x + dx + 2.0, yp + 2.0, deck - 3.0), "Concrete", bevel=0.0)
    for yt in towers:
        zb = min(ground(x, yt), WATER_Z) - 4.0
        g.box((x - 20.0, yt - 10.0, zb), (x + 20.0, yt + 10.0, WATER_Z + 2.0), "Concrete", bevel=0.0)
        for dx in (-15.0, 15.0):
            strut(g, (x + dx, yt, WATER_Z + 2.0), (x + dx * 0.45, yt, deck + 120.0), 5.0, 7.0, "Concrete", bevel=0.0, up=(0, 1, 0))
        g.box((x - 15.0, yt - 3.0, deck - 8.0), (x + 15.0, yt + 3.0, deck - 3.0), "Concrete", bevel=0.0)
        g.box((x - 9.0, yt - 3.0, deck + 95.0), (x + 9.0, yt + 3.0, deck + 101.0), "Concrete", bevel=0.0)
        for k in range(1, 12):
            for sgn in (-1, 1):
                for dx in (-12.0, 12.0):
                    top = Vector((x + dx * 0.55, yt, deck + 70.0 + k * 4.0))
                    foot = Vector((x + dx, yt + sgn * (20.0 + k * 19.0), deck + 0.5))
                    g.cylinder(top, foot, 0.25, "Corrugated", segments=6, bevel=0.0)
        g.box((x - 4.0, yt - 1.0, deck + 120.0), (x + 4.0, yt + 1.0, deck + 121.0), "Lamp", bevel=0.0)


def turbine(g: Group, base: Vector, r: random.Random):
    h = 95.0
    g.cylinder(base, base + Vector((0, 0, h)), 2.2, "Corrugated", segments=16, bevel=0.0)
    g.box(base + Vector((-2.0, -6.0, h - 1.5)), base + Vector((2.0, 4.0, h + 2.5)), "Corrugated", bevel=0.0)
    hub = base + Vector((0, -6.5, h + 0.5))
    phase = r.uniform(0, 120)
    for k in range(3):
        a = math.radians(phase + k * 120.0)
        d = Vector((math.cos(a), 0, math.sin(a)))
        g.sweep([hub, hub + d * 20.0, hub + d * 52.0], [(-1.6, -0.25), (1.6, -0.25), (1.6, 0.25), (-1.6, 0.25)], "Corrugated",
                up=(0, 1, 0), scale=lambda i: ((1.0, 1.0), (1.0, 0.8), (0.25, 0.4))[i])
    g.cylinder(hub, hub + Vector((0, -1.5, 0)), 1.4, "Corrugated", segments=12, bevel=0.0)


# The water texture -------------------------------------------------------------------------------

def write_png(path: str, pixels, channels: int):
    """An 8-bit PNG from a numpy array (rows top to bottom)."""
    import numpy as np
    h, w = pixels.shape[:2]
    raw = b"".join(b"\x00" + pixels[row].astype(np.uint8).tobytes() for row in range(h))
    colour = {1: 0, 3: 2, 4: 6}[channels]

    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        return c + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, colour, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        f.write(chunk(b"IEND", b""))


def water_texture(size: int = 1024):
    """A seamless tile of small waves: a height field summed from plane waves whose wave numbers are
    whole cycles per tile (so it tiles), steeper short chop on longer swell; its normal (OpenGL,
    green up) and a roughness that is low on the faces and higher in the chop. Meant to tile every
    8 m in Unreal, panned in two directions at two scales."""
    import numpy as np
    r = rng("water texture")
    t = np.arange(size) / size
    u, v = np.meshgrid(t, 1.0 - t)  # Rows top to bottom, v up
    hgt = np.zeros((size, size))
    dhu = np.zeros((size, size))
    dhv = np.zeros((size, size))
    for _ in range(48):
        k = r.uniform(2.0, 30.0)
        a = r.gauss(0.6, 0.7)  # Mostly from one quarter: wind-driven
        kx, ky = round(math.cos(a) * k), round(math.sin(a) * k)
        if kx == 0 and ky == 0:
            continue
        kk = math.hypot(kx, ky)
        amp = 0.02 / kk ** 1.1 * r.uniform(0.6, 1.4)
        ph = r.uniform(0, 2 * math.pi)
        arg = 2 * np.pi * (kx * u + ky * v) + ph
        s = np.sin(arg)
        c = np.cos(arg)
        sharp = 1.0 + 0.6 * s  # A little peaked
        hgt += amp * s * sharp
        dhu += amp * 2 * np.pi * kx * c * (1.0 + 1.2 * s)
        dhv += amp * 2 * np.pi * ky * c * (1.0 + 1.2 * s)
    scale = 0.35  # Heights are in tiles; flatten to taste
    nx, ny, nz = -dhu * scale, -dhv * scale, np.ones_like(dhu)
    n = np.sqrt(nx * nx + ny * ny + nz * nz)
    rgb = np.stack([(nx / n) * 0.5 + 0.5, (ny / n) * 0.5 + 0.5, (nz / n) * 0.5 + 0.5], axis=-1)
    write_png(os.path.join(OUT, "T_Water_Normal.png"), np.clip(rgb * 255.0 + 0.5, 0, 255), 3)
    chop = np.clip(np.sqrt(dhu ** 2 + dhv ** 2) * scale, 0.0, 1.0)
    rough = 0.04 + 0.16 * chop + 0.04 * (hgt - hgt.min()) / (np.ptp(hgt) + 1e-9)
    write_png(os.path.join(OUT, "T_Water_Roughness.png"), np.clip(rough * 255.0 + 0.5, 0, 255)[..., None], 1)


# Build -------------------------------------------------------------------------------------------

INSTANCE_TYPES = {
    "STSCrane": lambda g: sts_crane(g, raised=False),
    "STSCraneRaised": lambda g: sts_crane(g, raised=True),
    "RTG": rtg,
    "LightTower": light_tower,
    "Bollard": bollard,
}
for _paint, _ in PAINTS:
    INSTANCE_TYPES[f"Container40_{_paint}"] = (lambda paint: lambda g: container(g, 12.192, paint))(_paint)
    INSTANCE_TYPES[f"Container20_{_paint}"] = (lambda paint: lambda g: container(g, 6.058, paint))(_paint)

BUILDERS = {"Apron": apron, "Rail": rail, "Quay": quay, "Water": water, "Breakwater": breakwater,
            "ShipContainer": container_ship, "ShipHulk": hulk, "Buildings": buildings, "Machines": machines,
            "Terrain": terrain, "Town": town}
PLACERS = (stacks, cranes, lights, ship_containers, loads)

# The height of each instanced type, for the check against the Yard (it only matters for the tall).
TYPE_TOP = {"STSCrane": 80.0, "STSCraneRaised": 104.0, "RTG": 22.5, "LightTower": 31.6, "Bollard": 0.8}


def materials() -> dict:
    """Stand-in materials, one per slot, so the slots carry the surface names (Unreal assigns the
    real ones by name)."""
    colours = {"Concrete": "#5A5650", "Slag": "#2B2622", "HullSteel": "#4A5560", "Rust": "#6B3B1F", "Corrugated": "#8A8E92",
               "Grating": "#3B4046", "Hazard": "#F2C318", "Ochre": "#C8922A", "HTROrange": "#E8651A", "DarkSteel": "#222428",
               "Glass": "#0C0E10", "Rubber": "#141414", "Timber": "#6A5034", "Lamp": "#FFD9A0", "Water": "#10262C"}
    return {s: ka.flat_material(s, colours[s], 0.6) for s in SLOTS}


def check_heights(objects: dict, instances: list):
    """Warn about anything taller than LOW_TOP within NEAR of the walls, and anything inside them."""
    bad = []
    for name, obj in objects.items():
        if name.startswith("SM_Port_Container") or name[8:] in INSTANCE_TYPES:
            continue
        for v in obj.data.vertices:
            x, y, z = v.co
            if abs(x) < YARD[2] - 0.01 and abs(y) < YARD[3] - 0.01 and not (abs(x) >= 79.99 and z <= 0.01):
                bad.append(f"{name}: vertex inside the Yard at {tuple(round(c, 1) for c in v.co)}")
                break
            if z > LOW_TOP and near_walls(x, y) < NEAR:
                bad.append(f"{name}: {z:.1f} m tall at ({x:.0f}, {y:.0f}), {near_walls(x, y):.0f} m from the walls")
                break
    for inst in instances:
        kind = inst["type"][8:]
        x, y, z = inst["location"]
        top = z + TYPE_TOP.get(kind, CONTAINER_H)
        reach = {"STSCrane": 60.0, "STSCraneRaised": 46.0}.get(kind, 12.0)
        if top > LOW_TOP and near_walls(x, y) - reach < NEAR:
            bad.append(f"{kind} at ({x:.0f}, {y:.0f}) reaches {top:.0f} m within {NEAR:.0f} m of the walls")
        if abs(x) < YARD[2] and abs(y) < YARD[3]:
            bad.append(f"{kind} inside the Yard at ({x:.0f}, {y:.0f})")
    for b in bad[:20]:
        print("CHECK:", b)
    print(f"Height check: {len(bad)} problems")


def export(obj, path: str):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS", object_types={"MESH"},
                             mesh_smooth_type="SMOOTH_GROUP", add_leaf_bones=False, bake_anim=False, path_mode="AUTO")


def main() -> int:
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    only = None
    if "--only" in argv:
        only = set(argv[argv.index("--only") + 1].split(","))
    t0 = time.time()
    ka.reset_scene()
    os.makedirs(OUT, exist_ok=True)
    p = Port()
    terminal_plan(p)
    for name, fn in BUILDERS.items():
        if only is None or name in only:
            t = time.time()
            fn(p)
            print(f"built {name} in {time.time() - t:.1f} s")
    for fn in PLACERS:
        fn(p)
    for kind, fn in INSTANCE_TYPES.items():
        if only is None or kind in only or ("Types" in only):
            fn(p.g(kind))
    if "--no-texture" not in argv and (only is None or "Texture" in only):
        t = time.time()
        water_texture()
        print(f"water texture in {time.time() - t:.1f} s")
    mats = materials()
    report_path = os.path.join(OUT, "port.json")
    report = json.load(open(report_path)) if (only is not None and os.path.exists(report_path)) else {}
    objects = {}
    for name, group in p.groups.items():
        t = time.time()
        obj = group.object(mats)
        objects[obj.name] = obj
        tris = sum(len(poly.vertices) - 2 for poly in obj.data.polygons)
        export(obj, os.path.join(OUT, f"{obj.name}.fbx"))
        report[obj.name] = {"triangles": tris, "slots": [m.name for m in obj.data.materials],
                            "instanced": name in INSTANCE_TYPES}
        print(f"{obj.name}: {tris} triangles, slots {report[obj.name]['slots']} ({time.time() - t:.1f} s)")
    check_heights(objects, p.instances)
    with open(report_path, "w") as f:
        json.dump(report, f, indent=2)
    counts = {}
    for inst in p.instances:
        counts[inst["type"]] = counts.get(inst["type"], 0) + 1
    with open(os.path.join(OUT, "instances.json"), "w") as f:
        head = {"frame": "layout: metres, X east, Y north, Z up; yaw in degrees counter-clockwise about Z from +X; "
                         "location is the mesh's origin (each type's own frame is in port_world.py)", "counts": counts}
        f.write(json.dumps(head)[:-1] + ',\n"instances": [\n')
        f.write(",\n".join(json.dumps(i) for i in p.instances))
        f.write("\n]}\n")
    print(f"{len(p.instances)} instances: {counts}")
    print(f"port built in {time.time() - t0:.1f} s")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
