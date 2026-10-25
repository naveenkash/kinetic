"""The Yard's moving machinery, drawn from code (docs/objects/kinematics/rail-hauler.md,
gantry-crane.md; docs/arenas/the-yard/events-and-moving-parts.md): the detail over the boxes that
KineticYardDirector moves and collides with.

    blender -b --factory-startup -P art/machines/yard_machines.py

Each machine is one mesh, art/out/machines/SM_Machine_<Name>.fbx, in the frame of the kinematic
that carries it (the director's AddBox parts), in metres, X forward, Y left (the FBX import turns
that into Unreal's, so a part at Unreal local Y -28.5 is built at Y +28.5 here), Z up:
- Hauler: the pose is the nose; the train runs back along -X 14 m. A deep fish-belly underframe on
  two four-wheel bogies (the chassis box, Z -3.5 to 0), the deck at Z 0, a squat unmanned cab at
  the leading end with its windows painted over, two amber beacons and the klaxon horn, chevrons
  on both buffer beams, and a cut hull section chained on the deck (the load box, 12 m by 5 m by
  3.4 m);
- GantryBridge: the bridge girder across the Yard at Z 34.5 to 36 (along Y, 61.8 m), stiffened,
  with its crane rail, a catwalk and handrails along one side, the end carriages and the cab;
- GantryMagnet: the lifting magnet, 2.4 m across, finned, on its chain bridle;
- Lockout: the sudden-death curtain, 90 m by 36 m of ribbed steel shutter on stiffening channels;
- Shack: the control shack, 8 m by 10 m on its floor at Z 0, framed corrugated walls, the south
  windows, the door, roof plant and a parapet;
- HullSection, Column, Counterweight: the rated loads (a cut hull block, an H-column on its base
  plate, a cast counterweight with its lifting eye).
Material slots carry the art/surfaces names, as the arena kit's do, and are UV'd in metres.
"""
from __future__ import annotations

import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "..", "kit"), os.path.join(HERE, "..", "lib")]

import bpy  # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402
from kitlib import Group, seeded  # noqa: E402

OUT = os.path.join(HERE, "..", "out", "machines")


def prism(g: Group, profile, y0: float, y1: float, surface: str):
    """A plate cut to a profile: the polygon (x, z) points, counter-clockwise seen from +Y,
    extruded from y0 to y1 (a web, a gusset, a curved shell strake)."""
    near = [g.bm.verts.new((x, y0, z)) for x, z in profile]
    far = [g.bm.verts.new((x, y1, z)) for x, z in profile]
    faces = [g.bm.faces.new(list(reversed(near))), g.bm.faces.new(far)]
    n = len(profile)
    for i in range(n):
        j = (i + 1) % n
        faces.append(g.bm.faces.new((near[i], near[j], far[j], far[i])))
    g._take(faces, surface)


# The Hauler ----------------------------------------------------------------------------------------

def hauler() -> Group:
    g = Group("SM_Machine_Hauler")
    L, W = 14.0, 3.2
    # The deck: chequer plate over the frame, worn grey on the load's footprint.
    g.box((-L, -W / 2, -0.06), (0.0, W / 2, 0.0), "Ochre", bevel=0.01)
    g.box((-13.0, -1.4, -0.002), (-1.6, 1.4, 0.004), "HullSteel")  # The load's footprint, worn bare
    # The fish-belly side girders: a web cut to the belly's curve (deepest between the bogies),
    # flanges following it, and stiffeners every metre.
    def belly(x: float) -> float:
        t = min(1.0, max(0.0, (-x - 3.6) / 6.8))
        return 0.9 + 1.0 * math.sin(math.pi * t) ** 0.6
    xs = [-L + 0.45 + k * (L - 0.9) / 40 for k in range(41)]
    for side in (-1, 1):
        y = side * (W / 2 - 0.12)
        web = [(x, -0.06) for x in reversed(xs)] + [(x, -belly(x)) for x in xs]
        prism(g, list(reversed(web)), y - 0.012, y + 0.012, "Ochre")
        for xa, xb in zip(xs, xs[1:]):  # The bottom flange, plate by plate along the curve
            g.beam((xa, y, -belly(xa) - 0.015), (xb, y, -belly(xb) - 0.015), 0.03, 0.36, "Ochre", flange=0.03, web=0.03,
                   up=(0, 0, 1))
        g.box((-L + 0.45, y - 0.18, -0.12), (-0.45, y + 0.18, -0.06), "Ochre", bevel=0.005)  # Top flange
        x = -L + 1.0
        while x < -0.6:
            g.box((x - 0.012, y - 0.17 if side > 0 else y, -belly(x)), (x + 0.012, y if side > 0 else y + 0.17, -0.12), "Ochre")
            x += 1.0
    # Cross members, and the buffer beams with chevrons, buffers and the coupling.
    for x in [-L + 1.0 + 1.5 * i for i in range(9)]:
        g.beam((x, -W / 2 + 0.15, -0.5), (x, W / 2 - 0.15, -0.5), 0.4, 0.18, "Ochre")
    for x0, x1, face in ((-L + 0.45, -L + 0.75, -1), (-0.75, -0.45, 1)):  # Buffers end at the box's faces
        g.box((x0, -W / 2, -1.0), (x1, W / 2, -0.06), "Ochre", bevel=0.02)
        xf = x0 if face < 0 else x1
        for i in range(8):  # The chevrons, yellow and black
            y0 = -W / 2 + i * W / 8
            fx = (xf - 0.004, xf) if face < 0 else (xf, xf + 0.004)
            g.box((fx[0], y0, -0.95), (fx[1], y0 + W / 8, -0.11), "Hazard" if i % 2 == 0 else "DarkSteel")
        for y in (-1.0, 1.0):
            g.cylinder((xf, y, -0.6), (xf + 0.38 * face, y, -0.6), 0.16, "DarkSteel", segments=16)
            g.cylinder((xf + 0.38 * face, y, -0.6), (xf + 0.45 * face, y, -0.6), 0.25, "DarkSteel", segments=20)
        cx = (xf, xf + 0.42) if face > 0 else (xf - 0.42, xf)  # The coupling
        g.box((cx[0], -0.15, -0.75), (cx[1], 0.15, -0.45), "DarkSteel", bevel=0.02)
    # The bogies: frame, springs, axleboxes, four wheels on the 1.5 m gauge, brake blocks.
    for bx in (-2.6, -11.4):
        g.box((bx - 1.4, -1.1, -2.75), (bx + 1.4, 1.1, -2.35), "DarkSteel", bevel=0.03)
        g.box((bx - 0.5, -1.25, -2.4), (bx + 0.5, 1.25, -2.0), "DarkSteel", bevel=0.03)  # Bolster
        g.cylinder((bx, 0, -2.0), (bx, 0, -1.85), 0.45, "DarkSteel", segments=24)  # Centre pivot
        # The pedestal carrying the frame on the pivot (the deck rides 3.6 m over the rails).
        g.box((bx - 0.7, -1.3, -1.85), (bx + 0.7, 1.3, -0.85), "Ochre", bevel=0.04)
        for y in (-1.1, 1.1):
            prism(g, [(bx - 1.2, -0.85), (bx - 0.7, -0.85), (bx - 0.7, -1.7)], y - 0.012, y + 0.012, "Ochre")  # Gussets
            prism(g, [(bx + 0.7, -0.85), (bx + 1.2, -0.85), (bx + 0.7, -1.7)], y - 0.012, y + 0.012, "Ochre")
        for ax in (bx - 0.95, bx + 0.95):
            g.cylinder((ax, -0.95, -3.05), (ax, 0.95, -3.05), 0.08, "DarkSteel", segments=12)
            for side in (-1, 1):
                y = side * 0.75
                g.cylinder((ax, y - 0.07 * side, -3.05), (ax, y + 0.07 * side, -3.05), 0.45, "DarkSteel", segments=32)
                g.cylinder((ax, y + 0.07 * side, -3.05), (ax, y + 0.1 * side, -3.05), 0.5, "DarkSteel", segments=32)  # Flange
                g.box((ax - 0.18, side * 1.0 - 0.12, -3.25), (ax + 0.18, side * 1.0 + 0.12, -2.85), "DarkSteel", bevel=0.02)
                for dz in (0.0, 0.12, 0.24):  # Coil springs over the axlebox
                    g.cylinder((ax, side * 1.0, -2.85 + dz), (ax, side * 1.0, -2.8 + dz), 0.1, "DarkSteel", segments=12)
                g.box((ax - 0.62 if ax < bx else ax + 0.5, y - 0.08, -3.2), (ax - 0.5 if ax < bx else ax + 0.62, y + 0.08, -2.9),
                      "Rust", bevel=0.01)  # Brake block
    # The cab: squat, pressed steel, windows painted over, on the leading end.
    cx0, cx1 = -1.55, -0.3
    g.box((cx0, -1.4, 0.0), (cx1, 1.4, 1.5), "Ochre", bevel=0.06)
    g.box((cx0 + 0.1, -1.3, 1.5), (cx1 - 0.1, 1.3, 1.62), "DarkSteel", bevel=0.03)  # Roof
    for y in (-0.8, 0.8):
        g.box((cx1 - 0.002, y - 0.45, 0.75), (cx1 + 0.01, y + 0.45, 1.25), "DarkSteel", bevel=0.01)  # Painted-over glazing
    for side in (-1, 1):
        g.box((cx0 + 0.3, side * 1.4 - 0.002, 0.7), (cx1 - 0.3, side * 1.4 + 0.002, 1.25), "DarkSteel")
        g.cylinder((cx0 + 0.35, side * 0.9, 1.62), (cx0 + 0.35, side * 0.9, 1.78), 0.11, "DarkSteel", segments=16)
        g.cylinder((cx0 + 0.35, side * 0.9, 1.78), (cx0 + 0.35, side * 0.9, 1.98), 0.09, "Lamp", segments=16)  # Beacon
    g.cylinder((cx1 - 0.2, -0.3, 1.62), (cx1 + 0.25, -0.3, 1.75), 0.06, "DarkSteel", segments=12)  # Klaxon horn
    g.cylinder((cx1 + 0.25, -0.3, 1.75), (cx1 + 0.32, -0.3, 1.77), 0.16, "DarkSteel", segments=20)
    for x in (cx0 + 0.2, cx1 - 0.2):  # Grab rails
        g.cylinder((x, 1.44, 0.2), (x, 1.44, 1.3), 0.02, "Hazard", segments=8)
    # The load: a cut hull section, its bilge curve and frames, chained down.
    _hull_block(g, (-13.0, -2.5, 0.06), (-1.6, 2.5, 3.4), "Hauler")
    for x in (-11.5, -7.3, -3.1):
        for side in (-1, 1):
            a = Vector((x, side * 1.55, 0.0))
            b = Vector((x, side * 2.3, 3.2))
            n = 14
            for i in range(n):  # A chain of links, alternate ones turned
                p, q = a.lerp(b, i / n), a.lerp(b, (i + 1) / n)
                g.cylinder(p, q, 0.025 if i % 2 else 0.035, "DarkSteel", segments=6)
            g.box((x - 0.12, side * 1.55 - 0.1, 0.0), (x + 0.12, side * 1.55 + 0.1, 0.12), "DarkSteel", bevel=0.02)  # Lashing ring
    return g


def _hull_block(g: Group, mn, mx, label: str):
    """A torch-cut section of ship's hull, open at its cut ends: the shell plating curving into
    the bilge on the outer (+Y) side, the deck plate over deck beams, the inner bulkhead, transverse
    frames and longitudinal stiffeners seen at the cuts, plate seams with weld beads, a rust bleed."""
    (x0, y0, z0), (x1, y1, z1) = mn, mx
    r = seeded(label, 0)
    t = 0.025  # Plate
    h, w = z1 - z0, y1 - y0
    bilge = min(1.2, h * 0.4, w * 0.3)
    # The shell's cross-section, (y, z) round the outside from the deck edge down and in.
    shell = [(y1, z1)]
    shell += [(y1, z0 + bilge + (h - bilge) * (1 - k / 4)) for k in range(1, 5)]
    shell += [(y1 - bilge + bilge * math.cos(math.pi / 2 * k / 8), z0 + bilge - bilge * math.sin(math.pi / 2 * k / 8))
              for k in range(1, 9)]
    shell += [(y0 + w * (1 - k / 3) * 0.6, z0) for k in range(1, 3)] + [(y0, z0)]
    for (ya, za), (yb, zb) in zip(shell, shell[1:]):  # Each strake, plate thick, the length of the block
        n = Vector((0, zb - za, -(yb - ya))).normalized() * t
        prof = [(za, ya), (zb, yb), (zb - n.z, yb - n.y), (za - n.z, ya - n.y)]
        verts_far = None
        near = [g.bm.verts.new((x0, y, z)) for z, y in prof]
        far = [g.bm.verts.new((x1, y, z)) for z, y in prof]
        faces = [g.bm.faces.new(near), g.bm.faces.new(list(reversed(far)))]
        for i in range(4):
            j = (i + 1) % 4
            faces.append(g.bm.faces.new((near[j], near[i], far[i], far[j])))
        g._take(faces, "HullSteel")
        del verts_far
    g.box((x0, y0, z1 - t), (x1, y1, z1), "HullSteel")  # The deck plate
    g.box((x0, y0, z0), (x1, y0 + t, z1), "HullSteel")  # The inner bulkhead (the cut side)
    # Transverse frames every 0.8 m: a web ring inside the shell, seen through the open ends.
    x = x0 + 0.4
    while x < x1 - 0.1:
        for (ya, za), (yb, zb) in zip(shell, shell[1:]):
            g.beam((x, ya - 0.12 if ya > y0 + 0.1 else ya, za + 0.12 if za < z0 + 0.05 else za),
                   (x, yb - 0.12 if yb > y0 + 0.1 else yb, zb + 0.12 if zb < z0 + 0.05 else zb), 0.24, 0.1, "Rust",
                   flange=0.015, web=0.012, up=(1, 0, 0))
        g.beam((x, y0 + 0.05, z1 - 0.18), (x, y1 - 0.05, z1 - 0.18), 0.3, 0.1, "Rust", flange=0.015, web=0.012)  # Deck beam
        x += 0.8
    for k in range(1, 4):  # Longitudinals along the shell and under the deck
        y = y0 + w * k / 4
        g.beam((x0, y, z1 - 0.1), (x1, y, z1 - 0.1), 0.16, 0.08, "Rust", flange=0.012, web=0.01)
    for z in (z0 + bilge + (h - bilge) * f for f in (0.33, 0.66)):
        g.beam((x0, y1 - 0.08, z), (x1, y1 - 0.08, z), 0.14, 0.08, "Rust", flange=0.012, web=0.01, up=(0, 1, 0))
    # Seams and weld beads on the outside, and rust bleeding down from them.
    for z in (z0 + bilge + (h - bilge) * f for f in (0.33, 0.66)):
        g.box((x0, y1, z - 0.012), (x1, y1 + 0.012, z + 0.012), "DarkSteel", bevel=0.004)
    for f in (0.25, 0.5, 0.75):
        xs = x0 + (x1 - x0) * f
        g.box((xs - 0.012, y1, z0 + bilge), (xs + 0.012, y1 + 0.012, z1), "DarkSteel", bevel=0.004)
    for _ in range(r.randint(2, 4)):
        xc = r.uniform(x0 + 0.5, x1 - 0.5)
        g.box((xc - r.uniform(0.15, 0.5), y1 + 0.002, z0 + bilge + 0.2), (xc + r.uniform(0.15, 0.5), y1 + 0.008, z1 - 0.3),
              "Rust")


# The Gantry ----------------------------------------------------------------------------------------

def gantry_bridge() -> Group:
    g = Group("SM_Machine_GantryBridge")
    half = 30.9
    z0, z1 = 34.5, 36.0
    # The box girder: two webs, top and bottom flanges, diaphragm stiffeners every 2 m.
    g.box((-0.75, -half, z1 - 0.04), (0.75, half, z1), "Ochre", bevel=0.01)
    g.box((-0.75, -half, z0), (0.75, half, z0 + 0.04), "Ochre", bevel=0.01)
    for x in (-0.72, 0.72):
        g.box((x - 0.012, -half, z0), (x + 0.012, half, z1), "Ochre")
    y = -half + 1.0
    while y < half:
        for x in (-0.76, 0.76):
            g.box((x - 0.01 if x > 0 else x - 0.03, y - 0.06, z0 + 0.05), (x + 0.03 if x > 0 else x + 0.01, y + 0.06, z1 - 0.05), "Ochre")
        y += 2.0
    g.rail((0.0, -half, z1), (0.0, half, z1))  # The trolley's rail on top
    # The catwalk along the +X side, grating on brackets, with a handrail.
    g.grating((0.78, -half + 0.5, z1 - 0.05), (1.58, half - 0.5, z1 - 0.02), along=1)
    y = -half + 0.5
    while y <= half - 0.5:
        g.beam((0.76, y, z1 - 0.2), (1.58, y, z1 - 0.2), 0.12, 0.06, "Ochre")
        y += 1.5
    g.railing((1.55, -half + 0.5, z1 - 0.02), (1.55, half - 0.5, z1 - 0.02), 1.1, "Hazard")
    # The end carriages on the portal crossheads, with their wheels.
    for side in (-1, 1):
        ye = side * half
        g.box((-1.6, ye - 0.6, z0 - 0.7), (1.6, ye + 0.6, z0 + 0.2), "Ochre", bevel=0.05)
        for x in (-1.1, 1.1):
            g.cylinder((x, ye - 0.25, z0 - 0.75), (x, ye + 0.25, z0 - 0.75), 0.4, "DarkSteel", segments=24)
        g.box((-1.65, ye - 0.62, z0 - 0.5), (-1.6, ye + 0.62, z0 - 0.1), "Hazard")
        g.box((1.6, ye - 0.62, z0 - 0.5), (1.65, ye + 0.62, z0 - 0.1), "Hazard")
    # The cab at Unreal Y -28.5 (here +28.5): glazed on three sides, the operator's box.
    cy = 28.5
    g.box((-1.5, cy - 1.5, z1), (1.5, cy + 1.5, z1 + 0.3), "DarkSteel", bevel=0.03)
    g.box((-1.5, cy - 1.5, z1 + 0.3), (1.5, cy + 1.5, z1 + 3.7), "Ochre", bevel=0.05)
    g.box((-1.55, cy - 1.55, z1 + 3.7), (1.55, cy + 1.55, z1 + 3.95), "DarkSteel", bevel=0.03)
    for x in (-1.5, 1.5):
        g.box((x - 0.01, cy - 1.2, z1 + 1.4), (x + 0.01, cy + 1.2, z1 + 3.3), "Glass")
    g.box((-1.2, cy - 1.51, z1 + 1.4), (1.2, cy - 1.49, z1 + 3.3), "Glass")
    g.box((-0.6, cy - 0.6, z1 + 3.95), (0.6, cy + 0.6, z1 + 4.4), "DarkSteel", bevel=0.03)  # Air conditioner
    g.cylinder((1.2, cy + 1.2, z1 + 3.95), (1.2, cy + 1.2, z1 + 4.15), 0.08, "Lamp", segments=12)  # Beacon
    return g


def gantry_magnet() -> Group:
    g = Group("SM_Machine_GantryMagnet")
    r = 1.2
    g.cylinder((0, 0, 0.0), (0, 0, 0.08), r - 0.05, "DarkSteel", segments=48)  # Pole face
    g.cylinder((0, 0, 0.08), (0, 0, 0.55), r, "DarkSteel", segments=48)
    for k in range(16):  # Cooling fins
        a = 2 * math.pi * k / 16
        c, s = math.cos(a), math.sin(a)
        g.box((-0.075, -0.012, 0.1), (0.075, 0.012, 0.52), "DarkSteel",
              matrix=Matrix.Translation((c * (r + 0.06), s * (r + 0.06), 0)) @ Matrix.Rotation(a, 4, "Z"))
    g.cylinder((0, 0, 0.55), (0, 0, 0.62), r - 0.15, "Hazard", segments=48)  # Painted rim
    g.cylinder((0, 0, 0.62), (0, 0, 0.8), 0.35, "DarkSteel", segments=24)  # Junction box boss
    for k in range(3):  # The bridle: three chains up to the ring
        a = 2 * math.pi * k / 3
        p = Vector((math.cos(a) * 0.9, math.sin(a) * 0.9, 0.6))
        q = Vector((0, 0, 1.7))
        for i in range(12):
            g.cylinder(p.lerp(q, i / 12), p.lerp(q, (i + 1) / 12), 0.02 if i % 2 else 0.03, "DarkSteel", segments=6)
    g.cylinder((0, -0.1, 1.75), (0, 0.1, 1.75), 0.12, "DarkSteel", segments=16)  # Master ring
    g.cylinder((0.2, 0, 0.8), (0.2, 0, 2.5), 0.025, "Rubber", segments=8)  # Power cable
    return g


# The Lockout curtain -------------------------------------------------------------------------------

def lockout() -> Group:
    g = Group("SM_Machine_Lockout")
    half, z0, z1 = 45.0, -4.0, 32.0
    # The shutter skin: corrugated sheet on both faces, 200 mm pitch, 40 mm deep.
    us = [-half + 0.05 * k for k in range(int(2 * half / 0.05) + 1)]
    vs = [z0 + (z1 - z0) * k / 4 for k in range(5)]
    rib = lambda u, v: (0.0, 0.0, 0.02 + 0.02 * math.sin(2 * math.pi * u / 0.2))  # noqa: E731
    g.sheet(Matrix(((0, 0, 1, 0.06), (1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 0, 1))), us, vs, "Corrugated", shape=rib)
    g.sheet(Matrix(((0, 0, -1, -0.06), (-1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 0, 1))), [-u for u in reversed(us)], vs,
            "Corrugated", shape=lambda u, v: rib(-u, v))
    z = z0 + 1.5
    while z < z1:  # Horizontal stiffening channels both faces
        for face in (-1, 1):
            g.box((face * 0.08 if face > 0 else -0.2, -half, z - 0.1), (0.2 if face > 0 else -0.08, half, z + 0.1), "DarkSteel",
                  bevel=0.01)
        z += 3.0
    for y in [-half + 7.5 * k for k in range(13)]:  # Vertical posts
        for face in (-1, 1):
            g.box((face * 0.08 if face > 0 else -0.25, y - 0.15, z0), (0.25 if face > 0 else -0.08, y + 0.15, z1), "DarkSteel",
                  bevel=0.015)
    g.box((-0.35, -half, z0), (0.35, half, z0 + 0.4), "DarkSteel", bevel=0.03)  # The bottom beam
    for i in range(36):  # Hazard band along the bottom edge
        y0 = -half + i * 2.5
        for face in (-1, 1):
            g.box((face * 0.35 if face > 0 else -0.355, y0, z0 + 0.4), (0.355 if face > 0 else -0.35, y0 + 2.5, z0 + 1.2),
                  "Hazard" if i % 2 == 0 else "DarkSteel")
    return g


# The control shack ---------------------------------------------------------------------------------

def shack() -> Group:
    """Floor at Z 0 to 0.2, roof at 3.5 to 3.7; walls north (Unreal -Y, here +Y), west, east; the
    south side glazed above a 0.9 m sill."""
    g = Group("SM_Machine_Shack")
    hx, hy = 4.0, 5.0
    g.box((-hx, -hy, 0.0), (hx, hy, 0.2), "DarkSteel", bevel=0.02)
    g.box((-hx - 0.1, -hy - 0.1, 3.5), (hx + 0.1, hy + 0.1, 3.72), "DarkSteel", bevel=0.03)
    # The walls: a liner inside, corrugated sheet outside (76 mm pitch, ribs running up).
    def wall(m: Matrix, length: float, z0: float, z1: float):
        us = [k * 0.019 for k in range(int(length / 0.019) + 1)]
        g.sheet(m, us, [z0, z1], "Corrugated", shape=lambda u, v: (0.0, 0.0, 0.009 + 0.009 * math.sin(2 * math.pi * u / 0.076)))
    wall(Matrix(((1, 0, 0, -hx), (0, 0, 1, hy), (0, 1, 0, 0), (0, 0, 0, 1))), 2 * hx, 0.2, 3.5)  # North (+Y here)
    wall(Matrix(((0, 0, -1, -hx), (-1, 0, 0, hy), (0, 1, 0, 0), (0, 0, 0, 1))), 2 * hy, 0.2, 3.5)  # West
    wall(Matrix(((0, 0, 1, hx), (1, 0, 0, -hy), (0, 1, 0, 0), (0, 0, 0, 1))), 2 * hy, 0.2, 3.5)  # East
    wall(Matrix(((-1, 0, 0, hx), (0, 0, -1, -hy), (0, 1, 0, 0), (0, 0, 0, 1))), 2 * hx, 0.2, 1.1)  # The south sill
    for (a, b) in (((-hx, hy - 0.2), (hx, hy - 0.01)), ((-hx + 0.01, -hy), (-hx + 0.2, hy)), ((hx - 0.2, -hy), (hx - 0.01, hy))):
        g.box((a[0], a[1], 0.2), (b[0], b[1], 3.5), "DarkSteel")  # The liner
    g.box((-hx, -hy + 0.01, 0.2), (hx, -hy + 0.2, 1.1), "DarkSteel")
    # Framing posts at the corners and every 2 m, and the window frames on the south.
    for x in (-hx, -2.0, 0.0, 2.0, hx):
        for y in (-hy, hy):
            g.box((x - 0.08, y - 0.08, 0.2), (x + 0.08, y + 0.08, 3.5), "Ochre", bevel=0.01)
    for y in (-2.5, 0.0, 2.5):
        for x in (-hx, hx):
            g.box((x - 0.08, y - 0.08, 0.2), (x + 0.08, y + 0.08, 3.5), "Ochre", bevel=0.01)
    g.box((-hx, -hy - 0.06, 1.1), (hx, -hy + 0.06, 1.18), "Ochre", bevel=0.01)
    g.box((-hx, -hy - 0.06, 3.3), (hx, -hy + 0.06, 3.5), "Ochre", bevel=0.01)
    for x in (-hx, -2.0, 0.0, 2.0, hx):
        g.box((x - 0.05, -hy - 0.05, 1.18), (x + 0.05, -hy + 0.05, 3.3), "Ochre")
    g.box((-hx + 0.05, -hy - 0.005, 1.18), (hx - 0.05, -hy + 0.005, 3.3), "Glass")
    # The door on the east wall, a vent, the sign, and the roof plant behind a parapet.
    g.box((hx + 0.002, 1.0, 0.2), (hx + 0.03, 2.0, 2.3), "DarkSteel", bevel=0.01)
    g.cylinder((hx + 0.03, 1.15, 1.2), (hx + 0.09, 1.15, 1.2), 0.03, "DarkSteel", segments=10)
    g.grating((-hx - 0.03, -1.0, 2.4), (-hx, 1.0, 2.9), along=1, pitch=0.04)  # The vent louvre
    g.box((-1.5, hy + 0.001, 2.7), (1.5, hy + 0.02, 3.2), "Hazard")
    for (a, b) in (((-hx - 0.1, -hy - 0.1), (hx + 0.1, -hy + 0.05)), ((-hx - 0.1, hy - 0.05), (hx + 0.1, hy + 0.1)),
                   ((-hx - 0.1, -hy - 0.1), (-hx + 0.05, hy + 0.1)), ((hx - 0.05, -hy - 0.1), (hx + 0.1, hy + 0.1))):
        g.box((a[0], a[1], 3.72), (b[0], b[1], 4.0), "DarkSteel", bevel=0.01)
    g.box((-1.8, 1.0, 3.72), (-0.4, 2.6, 4.5), "Corrugated", bevel=0.03)  # Air handler
    g.grating((-1.6, 1.2, 4.5), (-0.6, 2.4, 4.53), along=0, pitch=0.05)  # The fan guard
    g.cylinder((2.5, -3.5, 3.72), (2.5, -3.5, 6.2), 0.04, "DarkSteel", segments=8)  # Radio mast
    return g


# The rated loads -----------------------------------------------------------------------------------

def hull_section() -> Group:
    g = Group("SM_Machine_HullSection")
    _hull_block(g, (-6.0, -3.0, -1.35), (6.0, 3.0, 1.35), "Rated")
    # The rated loads' mark: an ochre band round the deck edge (the director's rated colour).
    for y0, y1 in ((-3.0, -2.98), (3.012, 3.03)):
        g.box((-6.0, y0, 1.0), (6.0, y1, 1.3), "Ochre")
    return g


def column() -> Group:
    """An H-column 0.6 m by 0.6 m, 10.3 m tall, centred on its middle, on a base plate."""
    g = Group("SM_Machine_Column")
    h = 5.15
    for x in (-0.3, 0.27):
        g.box((x, -0.3, -h), (x + 0.03, 0.3, h), "Ochre", bevel=0.005)
    g.box((-0.3, -0.015, -h), (0.3, 0.015, h), "Ochre")
    g.box((-0.45, -0.45, -h), (0.45, 0.45, -h + 0.05), "DarkSteel", bevel=0.01)
    g.box((-0.45, -0.45, h - 0.05), (0.45, 0.45, h), "DarkSteel", bevel=0.01)
    for x in (-0.35, 0.35):
        for y in (-0.35, 0.35):
            g.cylinder((x, y, -h + 0.05), (x, y, -h + 0.11), 0.03, "DarkSteel", segments=6)
    for z in (-h + 0.6, 0.0, h - 0.6):
        g.box((-0.3, -0.29, z - 0.006), (0.3, 0.29, z + 0.006), "Ochre")  # Stiffener plates
    g.box((0.301, -0.2, -h + 1.0), (0.305, 0.2, -h + 1.8), "Hazard")
    return g


def counterweight() -> Group:
    """Cast iron, 2 m by 2 m by 3 m centred, stacked slabs, the lifting eye on top."""
    g = Group("SM_Machine_Counterweight")
    for k in range(6):
        z0 = -1.5 + k * 0.5
        g.box((-1.0, -1.0, z0), (1.0, 1.0, z0 + 0.49), "DarkSteel", bevel=0.03)
    for x in (-0.8, 0.8):
        g.box((x - 0.06, -1.04, -1.4), (x + 0.06, -1.0, 1.4), "Rust")  # Tie rods
        g.box((x - 0.06, 1.0, -1.4), (x + 0.06, 1.04, 1.4), "Rust")
    g.box((-0.15, -0.06, 1.5), (0.15, 0.06, 1.75), "DarkSteel", bevel=0.02)
    g.cylinder((0, -0.08, 1.85), (0, 0.08, 1.85), 0.16, "DarkSteel", segments=20)
    g.box((-1.0, -1.001, 0.2), (1.0, -1.0, 0.6), "Hazard")
    return g


MACHINES = [hauler, gantry_bridge, gantry_magnet, lockout, shack, hull_section, column, counterweight]


def materials() -> dict:
    colours = {"Concrete": "#5A5650", "Slag": "#2B2622", "HullSteel": "#4A5560", "Rust": "#6B3B1F", "Corrugated": "#8A8E92",
               "Grating": "#3B4046", "Hazard": "#F2C318", "Ochre": "#C8922A", "HTROrange": "#E8651A", "DarkSteel": "#222428",
               "Glass": "#0C0E10", "Rubber": "#141414", "Timber": "#6A5034", "Lamp": "#FFD9A0"}
    return {s: ka.flat_material(s, c, 0.6) for s, c in colours.items()}


def main() -> int:
    ka.reset_scene()
    mats = materials()
    os.makedirs(OUT, exist_ok=True)
    for make in MACHINES:
        group = make()
        obj = group.object(mats)
        tris = sum(len(p.vertices) - 2 for p in obj.data.polygons)
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, f"{obj.name}.fbx"), use_selection=True,
                                 apply_scale_options="FBX_SCALE_UNITS", object_types={"MESH"}, mesh_smooth_type="SMOOTH_GROUP",
                                 add_leaf_bones=False, bake_anim=False, path_mode="AUTO")
        print(f"{obj.name}: {tris} triangles, slots {[m.name for m in obj.data.materials]}")
        bpy.data.objects.remove(obj)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
