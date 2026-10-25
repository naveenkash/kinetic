"""Clamp crate: docs/objects/props-light/clamp-crate.md.

60 x 40 x 30 cm open-top crate of galvanized 25 mm weld mesh on a 12 mm frame, holding twenty
scaffold couplers (one of them a different maker's pattern), with bar handles at both ends. Pivot
at the bottom centre. The Yard variant carries the C4PW tag.

The weld mesh is the high mesh, baked onto a masked card box (the doc's LOD0: a masked card box
and twenty low couplers, 2,000 tris).
"""
from __future__ import annotations

import math
import random

import bmesh

import kinetic_art as ka

L, W, H = 0.60, 0.40, 0.30
BAR = 0.006
MESH = 0.025


def coupler(name: str, at, yaw: float, pitch: float, galv, rust, odd: bool) -> tuple[list, list]:
    """A right-angle coupler: two half-rings on a galvanized body, and its rusted T-bolt, which
    is detail baked onto the body. Returns (parts, detail)."""
    parts = []
    r = 0.028 if not odd else 0.032
    # 84 tris a coupler, against the doc's 60: two 6 x 3 rings and the body.
    a = ka.ring(f"{name}_a", r, 0.007, (0, 0, 0), "X", 6, 3, galv)
    b = ka.ring(f"{name}_b", r, 0.007, (0, 0, 0.045), "Y", 6, 3, galv)
    body = ka.box(f"{name}_body", (0.05, 0.05, 0.03), (0, 0, 0.022), galv)
    bolt = ka.box(f"{name}_bolt", (0.07, 0.012, 0.012), (0, 0.027, 0.022), rust)
    for p in (a, b, body, bolt):
        ka.transform(p, at, rotation_z=yaw, rotation_x=pitch)
    return [a, b, body], [bolt]


def bounds(objects) -> tuple:
    """(min x, max x, min y, max y, min z, max z) of these objects' meshes."""
    co = [v.co for o in objects for v in o.data.vertices]
    return (min(c.x for c in co), max(c.x for c in co), min(c.y for c in co), max(c.y for c in co),
            min(c.z for c in co), max(c.z for c in co))


def card_box(name: str, material) -> object:
    """The four sides and the floor as single quads on the mesh planes: the weld mesh is baked
    onto them and masked."""
    hx, hy = L / 2, W / 2
    bm = bmesh.new()
    v = {(sx, sy, sz): bm.verts.new((sx * hx, sy * hy, 0.006 if sz == 0 else H))
         for sx in (-1, 1) for sy in (-1, 1) for sz in (0, 1)}
    for quad in (((-1, -1, 0), (1, -1, 0), (1, -1, 1), (-1, -1, 1)), ((1, 1, 0), (-1, 1, 0), (-1, 1, 1), (1, 1, 1)),
                 ((1, -1, 0), (1, 1, 0), (1, 1, 1), (1, -1, 1)), ((-1, 1, 0), (-1, -1, 0), (-1, -1, 1), (-1, 1, 1)),
                 ((-1, -1, 0), (-1, 1, 0), (1, 1, 0), (1, -1, 0))):
        bm.faces.new([v[k] for k in quad])
    obj = ka.mesh_object(name, bm)
    obj.data.materials.append(material)
    return obj


def build(variant: str = "yard") -> dict:
    galv = ka.galvanized()
    steel = ka.flat_material("MI_Steel_Bare", "#5C5F63", 0.45, 1.0)
    rust = ka.flat_material("MI_Steel_Rusted", "#7A3F1C", 0.8, 0.3)
    ochre = ka.flat_material("MI_Stencil_Ochre", "#C8912A", 0.6)
    parts, mesh = [], []
    hx, hy = L / 2, W / 2
    # Frame: the twelve edges.
    for sx in (-1, 1):
        for sy in (-1, 1):
            parts.append(ka.box("edge_z", (0.012, 0.012, H), (sx * hx, sy * hy, H / 2), galv))
    for z in (0.006, H - 0.006):
        for sy in (-1, 1):
            parts.append(ka.box("edge_x", (L, 0.012, 0.012), (0, sy * hy, z), galv))
        for sx in (-1, 1):
            parts.append(ka.box("edge_y", (0.012, W, 0.012), (sx * hx, 0, z), galv))
    # Weld mesh on the four sides and the floor.
    nx, ny, nz = round(L / MESH), round(W / MESH), round(H / MESH)
    for sy in (-1, 1):
        for i in range(1, nx):
            mesh.append(ka.box("mesh_v", (BAR, BAR, H), (-hx + i * MESH, sy * hy, H / 2), galv))
        for k in range(1, nz):
            mesh.append(ka.box("mesh_h", (L, BAR, BAR), (0, sy * hy, k * MESH), galv))
    for sx in (-1, 1):
        for j in range(1, ny):
            mesh.append(ka.box("mesh_v", (BAR, BAR, H), (sx * hx, -hy + j * MESH, H / 2), galv))
        for k in range(1, nz):
            mesh.append(ka.box("mesh_h", (BAR, W, BAR), (sx * hx, 0, k * MESH), galv))
    for i in range(1, nx):
        mesh.append(ka.box("floor_x", (BAR, W, BAR), (-hx + i * MESH, 0, 0.006), galv))
    for j in range(1, ny):
        mesh.append(ka.box("floor_y", (L, BAR, BAR), (0, -hy + j * MESH, 0.006), galv))
    # End handles.
    for sx in (-1, 1):
        parts.append(ka.tube("handle", [(sx * hx, -0.1, H - 0.03), (sx * (hx + 0.04), -0.08, H - 0.03),
                                        (sx * (hx + 0.04), 0.08, H - 0.03), (sx * hx, 0.1, H - 0.03)], 0.007, steel,
                                sides=4))
    # Twenty couplers tipped in: twelve across the floor on a jittered 4 x 3 grid, eight on the
    # half-offset grid above them. Each settles onto the floor bars (top at 9 mm) or onto the
    # highest coupler already under it, and all stay clear of the walls by more than the bake's
    # reach, so none is printed onto a side card.
    from mathutils import Matrix
    rng = random.Random(4)
    spots = [(-0.18 + 0.12 * i, -0.09 + 0.09 * j) for j in range(3) for i in range(4)]
    spots += [(-0.12 + 0.12 * i, -0.045 + 0.09 * j) for j in range(2) for i in range(4)]
    placed = []
    for n, (sx, sy) in enumerate(spots):
        at = (sx + rng.uniform(-0.015, 0.015), sy + rng.uniform(-0.015, 0.015), 0.2)
        body, bolt = coupler(f"cp{n}", at, rng.uniform(0, math.pi * 2), rng.uniform(-0.6, 0.6), galv, rust, n == 13)
        x0, x1, y0, y1, z0, _ = bounds(body)
        rest = 0.009
        for other in placed:
            ox0, ox1, oy0, oy1, _, oz1 = bounds(other)
            if x0 < ox1 and ox0 < x1 and y0 < oy1 and oy0 < y1:
                rest = max(rest, oz1)
        drop = Matrix.Translation((0, 0, rest - z0))
        for o in body + bolt:
            o.data.transform(drop)
        placed.append(body)
        parts += body
        mesh += bolt
    # The Yard's steel tag, hung by a wire from a bar of the mesh 1.5 cm out: beyond the bake's
    # 6 mm reach, so it is not printed onto the card.
    bar_x, bar_z = -hx + 17 * MESH, 11 * MESH
    parts.append(ka.box("tag", (0.09, 0.003, 0.05), (bar_x, -hy - 0.015, bar_z - 0.035), galv))
    parts.append(ka.tube("tag_wire", [(bar_x, -hy - 0.015, bar_z - 0.01), (bar_x, -hy, bar_z)], 0.001, galv, sides=4))
    parts.append(ka.stencil("tag_text", "C4PW", 0.025, ochre, (bar_x, -hy - 0.018, bar_z - 0.035), (0, -1, 0)))
    # Top_01 goes on the top of the heap's highest coupler (measured before the join).
    hx0, hx1, hy0, hy1, _, hz1 = max((bounds(p) for p in placed), key=lambda b: b[5])
    name = "SM_Light_ClampCrate"
    obj, high = ka.high_low(name, parts, mesh, [card_box("card", galv)])
    ka.collision_box(name, 0, (L, W, H), (0, 0, H / 2))
    for s, (loc, normal) in {"SKT_Dog_Top_01": (((hx0 + hx1) / 2, (hy0 + hy1) / 2, hz1), None),
                             "SKT_Dog_Handle_01": ((hx + 0.047, 0, H - 0.03), (1, 0, 0)),
                             "SKT_Dog_Handle_02": ((-hx - 0.047, 0, H - 0.03), (-1, 0, 0)),
                             "SKT_Dog_SideN_01": ((hx, 0, 0.12), (1, 0, 0))}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.006, "masked": [name],
            "size": (L + 0.08, W, H), "res": 1024, "budget": 2000}
