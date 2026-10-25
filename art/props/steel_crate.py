"""Steel crate: docs/objects/props-medium/steel-crate.md.

120 x 100 x 90 cm, a squat welded box with rolled corners, a hinged lid with two hasps, two
forklift channels in the base and four lifting lugs at the top corners. Pivot at the bottom
centre, X+ along the long axis and toward the front (the hasps). Variants: default grey, htr,
guild.

The box is pressed sheet: two stiffening ribs round it, a lid with an overhanging lip, open
C-channels for the forks, lugs welded on through plates, hasps on hinged leaves over their
staples, three-knuckle hinges. The weld beads along every seam, the bolt heads, the markings and
the chalk tally are the high mesh, baked into the body; the panels carry a dent or two, in both
meshes.
"""
from __future__ import annotations

import math
import random

import bmesh
from mathutils import Vector

import kinetic_art as ka

L, W, H = 1.20, 1.00, 0.90
FORK = 0.10   # The channels raise the box
LID = 0.06
LUG_Z = H - LID - 0.05

# Wear is the doc's: HTR 0.3, Guild 0.7. HTR has an orange band at mid height and an asset tag;
# Guild's code is ochre on a black field with the hitch glyph; Yard crates carry a chalk tally.
VARIANTS = {
    "default": dict(paint="#5B6670", text=None, ink="#E6E4DD", chips=0.35, seed=1),
    "htr": dict(paint="#4A4D50", text=("HTR-RC-4471", "RECLAIMED / DO NOT RETURN"), ink="#E8620C", chips=0.3, seed=4471),
    "guild": dict(paint="#C8912A", text=("LG-0932", None), ink="#C8912A", chips=0.7, seed=932),
}


def panel_box(name: str, size, center, material, cuts: int) -> object:
    """A box with its faces split `cuts` times each way, so it can dent."""
    obj = ka.box(name, size, center, material)
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.subdivide_edges(bm, edges=bm.edges[:], cuts=cuts, use_grid_fill=True)
    bm.to_mesh(obj.data)
    bm.free()
    return obj


def bead(name: str, points, material) -> object:
    """A weld bead: a thin tube along a seam."""
    return ka.tube(name, points, 0.004, material, sides=6)


def band(name: str, z0: float, z1: float, material, out: float = 0.002) -> object:
    """A band round the body at `out` off its rolled corners: a rounded rectangle, extruded."""
    hx, hy, r = L / 2 + out, W / 2 + out, 0.03 + out
    ring = []
    for cx, cy, a0 in ((hx - r, hy - r, 0.0), (-hx + r, hy - r, 90.0), (-hx + r, -hy + r, 180.0), (hx - r, -hy + r, 270.0)):
        for k in range(4):
            a = math.radians(a0 + 90.0 * k / 3)
            ring.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    bm = bmesh.new()
    lo = [bm.verts.new((x, y, z0)) for x, y in ring]
    hi = [bm.verts.new((x, y, z1)) for x, y in ring]
    for i in range(len(ring)):
        j = (i + 1) % len(ring)
        bm.faces.new((lo[i], lo[j], hi[j], hi[i]))
    obj = ka.mesh_object(name, bm)
    obj.data.materials.append(material)
    return obj


def build(variant: str = "htr") -> dict:
    v = VARIANTS[variant]
    lugs = [(sx * (L / 2 - 0.08), sy * (W / 2 + 0.006)) for sx in (-1, 1) for sy in (-1, 1)]
    wear = ka.Wear(paint=v["paint"], chips=v["chips"], scrape_height=0.0, rust_streaks=0.55, roof_dirt="#1B1A18",
                   streak_band=(0.1, H), detail=3.0, edge=0.02, grime_reach=0.06, dents=0.6, dirt_streaks=0.2,
                   lower_grime="#3A4A3C" if variant == "default" else None,
                   streak_from=tuple((x, y, LUG_Z - 0.03, 0.03) for x, y in lugs),
                   extra={"footprint": (L / 2 - 0.06, W / 2 - 0.06, H)})
    paint = ka.painted_steel("MI_Steel_Painted_Worn", wear)
    steel = ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.65), rust=0.45, detail=5.0, streak_band=(0.0, H))
    ink = ka.stencil_paint("MI_Stencil", v["ink"], wear)
    parts, detail = [], []
    body = panel_box("body", (L, W, H - FORK - LID), (0, 0, FORK + (H - FORK - LID) / 2), paint, 5)
    ka.add_bevel(body, 0.03, 3)
    parts.append(body)
    lid = panel_box("lid", (L + 0.02, W + 0.02, LID), (0, 0, H - LID / 2), paint, 4)
    ka.add_bevel(lid, 0.012, 2)
    parts.append(lid)
    # Panel seams: two pressed ribs round the body.
    for z in (0.35, 0.6):
        rib = ka.box("rib", (L + 0.012, W + 0.012, 0.03), (0, 0, z), paint)
        ka.add_bevel(rib, 0.012, 2)
        parts.append(rib)
    # Fork channels along Y: a C-channel each, open at both ends, its web under the box.
    for x in (-0.33, 0.33):
        for piece in (ka.box("fork_web", (0.24, W, 0.008), (x, 0, FORK - 0.004), steel),
                      ka.box("fork_flange", (0.008, W, FORK - 0.008), (x - 0.116, 0, (FORK - 0.008) / 2), steel),
                      ka.box("fork_flange", (0.008, W, FORK - 0.008), (x + 0.116, 0, (FORK - 0.008) / 2), steel)):
            ka.add_bevel(piece, 0.002, 1)
            parts.append(piece)
    # Lifting lugs at the top corners, each welded on through a plate.
    for x, y in lugs:
        sy = 1 if y > 0 else -1
        parts.append(ka.ring("lug", 0.035, 0.009, (x, y, LUG_Z), "Y", 20, 8, steel))
        plate = ka.box("lug_plate", (0.09, 0.008, 0.05), (x, sy * (W / 2 + 0.004), LUG_Z - 0.035), steel)
        ka.add_bevel(plate, 0.002, 1)
        parts.append(plate)
        detail.append(bead("weld", [(x - 0.045, sy * (W / 2 + 0.002), LUG_Z - 0.06),
                                    (x + 0.045, sy * (W / 2 + 0.002), LUG_Z - 0.06)], steel))
    # Two hasps on the lid's front edge (X+): a leaf hinged on the lid, slotted over a staple.
    for y in (-0.25, 0.25):
        leaf = ka.box("hasp", (0.008, 0.06, 0.12), (L / 2 + 0.016, y, H - LID - 0.03), steel)
        ka.cut(leaf, ka.box("slot", (0.03, 0.012, 0.035), (L / 2 + 0.016, y, H - LID - 0.07)))
        ka.add_bevel(leaf, 0.002, 1)
        parts.append(leaf)
        parts.append(ka.cylinder("hasp_knuckle", 0.008, 0.06, (L / 2 + 0.016, y, H - LID + 0.02), "Y", 8, steel))
        parts.append(ka.tube("staple", [(L / 2 + 0.001, y, H - LID - 0.06), (L / 2 + 0.03, y, H - LID - 0.06),
                                        (L / 2 + 0.03, y, H - LID - 0.08), (L / 2 + 0.001, y, H - LID - 0.08)],
                             0.004, steel, sides=6))
        staple_plate = ka.box("staple_plate", (0.006, 0.05, 0.05), (L / 2 + 0.003, y, H - LID - 0.07), steel)
        parts.append(staple_plate)
    # Hinges at the back: three knuckles each, a leaf on the lid and one on the body.
    for y in (-0.3, 0.3):
        for k, dy in enumerate((-0.05, 0.0, 0.05)):
            parts.append(ka.cylinder("hinge", 0.014, 0.045, (-L / 2 - 0.012, y + dy, H - LID), "Y", 10, steel))
        parts.append(ka.cylinder("hinge_pin", 0.006, 0.16, (-L / 2 - 0.012, y, H - LID), "Y", 6, steel))
        for z in (H - LID + 0.025, H - LID - 0.03):
            hinge_leaf = ka.box("hinge_leaf", (0.006, 0.14, 0.05), (-L / 2 - 0.004, y, z), steel)
            parts.append(hinge_leaf)
            for dy in (-0.045, 0.045):
                detail.append(ka.cylinder("bolt", 0.007, 0.004, (-L / 2 - 0.009, y + dy, z), "X", 6, steel))
    # Markings between the ribs: the code high on the east side (Y-) and low on the front (X+),
    # below the hasps; the line low on the east side.
    if variant == "htr":
        detail.append(band("band", 0.445, 0.505, ka.stencil_paint("MI_Paint_HTR_Orange", "#E8620C", wear)))
        tag = ka.box("asset_tag", (0.003, 0.08, 0.05), (L / 2 + 0.0015, 0.3, 0.27), steel)
        parts.append(tag)
        for dy in (-0.032, 0.032):
            detail.append(ka.cylinder("rivet", 0.004, 0.003, (L / 2 + 0.003, 0.3 + dy, 0.27), "X", 6, steel))
        detail.append(ka.stencil("tag_stamp", "4471", 0.016, ka.stencil_paint("MI_Stamp", "#2A2C2E", wear),
                                 (L / 2 + 0.0033, 0.3, 0.27), (1, 0, 0), resolution=3))
    if variant == "guild":
        field = ka.stencil_paint("MI_Paint_Black", "#1C1B19", wear)
        detail.append(ka.box("code_field", (0.86, 0.003, 0.2), (0.0, -W / 2 - 0.0015, 0.725), field))
        detail.append(ka.box("code_field_e", (0.003, 0.6, 0.13), (L / 2 + 0.0015, 0, 0.225), field))
        detail.append(ka.ring("hitch_glyph", 0.035, 0.006, (-0.35, -W / 2 - 0.004, 0.725), "Y", 16, 4, ink))
    if variant in ("htr", "guild"):
        chalk = ka.stencil_paint("MI_Chalk", "#D8D4C8", ka.Wear(**{**wear.__dict__, "chips": 0.9}))
        for k in range(5):
            stroke = ka.box("chalk_tally", (0.006, 0.002, 0.08), (0, 0, 0), chalk)
            ka.transform(stroke, (-L / 2 + 0.25 + 0.025 * k, W / 2 + 0.0012, 0.25), rotation_y=0.0 if k < 4 else 1.1)
            detail.append(stroke)
    if v["text"]:
        code, line = v["text"]
        detail.append(ka.stencil("code", code, 0.12, ink, (0.04 if variant == "guild" else 0.0, -W / 2 - 0.0035, 0.725),
                                 (0, -1, 0), spray=0.003, resolution=4))
        detail.append(ka.stencil("code_e", code, 0.08, ink, (L / 2 + 0.0035, 0, 0.225), (1, 0, 0), spray=0.002,
                                 resolution=4))
        if line:
            detail.append(ka.stencil("line", line, 0.045, ink, (0.0, -W / 2 - 0.0035, 0.25), (0, -1, 0), spray=0.0015,
                                     resolution=3))

    name = {"default": "SM_Medium_SteelCrate", "htr": "SM_Medium_SteelCrate_HTR", "guild": "SM_Medium_SteelCrate_Guild"}[variant]
    obj, high = ka.high_low(name, parts, detail, [], soft=True)
    # Knocks: one in the lid, one in a long side, one in the front, kept clear of the sockets.
    rng = random.Random(v["seed"])
    side = 1 if rng.random() < 0.5 else -1
    knocks = ka.dents(((rng.uniform(-0.12, 0.12), rng.uniform(-0.3, 0.3), H), (0, 0, -0.008), 0.14),
                      ((rng.uniform(-0.45, -0.15), side * W / 2, rng.uniform(0.2, 0.3)), (0, -side * 0.01, 0), 0.12),
                      ((L / 2, rng.uniform(-0.35, -0.2), 0.48), (-0.007, 0, 0), 0.1))
    ka.deform([obj, high], knocks)
    ka.collision_box(name, 0, (L, W, H), (0, 0, H / 2))
    # Sides by README-props: X+ is N, so E is Y- and W is Y+.
    sockets = {
        "SKT_Dog_Top_01": ((0.3, 0, H), None), "SKT_Dog_Top_02": ((-0.3, 0, H), None),
        "SKT_Dog_SideN_01": ((L / 2, 0, 0.5), (1, 0, 0)), "SKT_Dog_SideS_01": ((-L / 2, 0, 0.5), (-1, 0, 0)),
        "SKT_Dog_SideE_01": ((0, -W / 2, 0.5), (0, -1, 0)), "SKT_Dog_SideW_01": ((0, W / 2, 0.5), (0, 1, 0)),
        "SKT_Dog_Lid_01": ((L / 2 + 0.01, 0, H - LID / 2), (1, 0, 0)),
    }
    i = 1
    for sx in (-1, 1):
        for sy in (-1, 1):
            sockets[f"SKT_Dog_Hook_{i:02d}"] = ((sx * (L / 2 - 0.08), sy * (W / 2 + 0.015), LUG_Z), (0, sy, 0))
            i += 1
    for s, (loc, normal) in sockets.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.006, "size": (L, W, H), "res": 2048,
            "budget": 6000}
