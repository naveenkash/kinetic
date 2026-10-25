"""Shipping container (20 ft): docs/objects/heavy/shipping-container.md.

ISO 606 x 244 x 259 cm, pivot at the bottom centre, X+ toward the door end. Corrugated sides,
blank end and roof, rounded at every fold; corner posts, rails, and eight corner castings with
their twistlock apertures. On the door end two leaves, each a frame round a corrugated panel, with
two lock bars in guide brackets, their cams in keepers on the header and sill, horizontal handles
in retainers, hinges and gaskets; a data plate on the left leaf. The doors are separate meshes
hinged at their outer edge (the blown state swings them). Dog sockets and the collision box are
named as the doc gives them.

The panels are dented and oil-canned a little, and the roof sags: a seeded deformation of both
the game mesh and the high mesh. The stencils (owner mark, asset code, SITE PROPERTY, the door
code and the data plate's text) are the high mesh, painted over the corrugation and baked into
the body and doors.

Variants (the doc's table): htr (the Yard, Anchor Rim), meridian (Ballast), guild (the Yard).
"""
from __future__ import annotations

import math
import random

import bpy
from mathutils import Matrix, Vector

import kinetic_art as ka

L, W, H = 6.058, 2.438, 2.591
POST = 0.16           # Corner post section
CAST = (0.178, 0.162, 0.118)  # Corner casting (x, y, z)
RAIL_BOTTOM = 0.16
RAIL_TOP = 0.10
SILL, HEADER = 0.20, 0.18     # Door end bottom and top rails
LEAF = (W - 2 * 0.012) / 2    # Each door leaf, minus the gasket gap
PITCH, DEPTH = 0.2, 0.036     # Side corrugation: the doc's alternating 10 cm bands
BEND = 0.008                  # The radius of every fold in the sheet
BAR_X = L / 2 + 0.06          # Lock bars' axis, in front of the leaves
BARS = (0.30, 0.90)           # Lock bars' distance off the centre line, each leaf
STREAKS = (H - RAIL_TOP, H - CAST[2], H - HEADER)  # Rust bleeds from the top rail and castings

VARIANTS = {
    "htr": dict(mesh="SM_Heavy_Container20_HTR", code="HTR-RC-0417", owner="HTR", seed=417,
                wear=ka.Wear(paint="#E8620C", chips=0.3, rust_streaks=0.45, roof_dirt="#1B1A18",
                             streak_from=STREAKS, extra={"ends_x": L / 2})),
    "meridian": dict(mesh="SM_Heavy_Container20_Meridian", code="MC-7-2291", owner="MERIDIAN", seed=2291,
                     wear=ka.Wear(paint="#2A9FB5", chips=0.8, rust_streaks=0.9, lower_grime="#3A4A3C",
                                  roof_dirt="#C9CBC4", dust=0.8, dents=0.8, streak_from=STREAKS,
                                  streak_length=0.6, extra={"ends_x": L / 2})),
    "guild": dict(mesh="SM_Heavy_Container20_Guild", code="LG-3306", owner="LAY GUILD", seed=3306,
                  wear=ka.Wear(paint="#5B6670", chips=0.45, rust_streaks=0.5, roof_dirt="#1B1A18",
                               streak_from=STREAKS, extra={"ends_x": L / 2})),
}


def oval(name: str, center, axis: str, width: float, height: float, depth: float) -> bpy.types.Object:
    """An oval prism along `axis`, `width` x `height` across it (in the order of the other two
    axes, X before Y before Z): a casting aperture's cutter."""
    c = ka.cylinder(name, 0.5, depth, (0, 0, 0), axis, 10)
    across = [a for a in "XYZ" if a != axis]
    scale = {axis: 1.0, across[0]: width, across[1]: height}
    c.data.transform(Matrix.Diagonal((scale["X"], scale["Y"], scale["Z"], 1.0)))
    c.data.transform(Matrix.Translation(center))
    return c


def casting(center, sx: int, sy: int, top: bool, steel) -> bpy.types.Object:
    """A corner casting: the oval twistlock aperture in its top (or bottom), and the side and end
    apertures, each a pocket 4 cm deep; its edges rounded."""
    c = ka.box("casting", CAST, center, steel)
    # Rounded first, so the apertures' rims stay crisp and the count low.
    ka.add_bevel(c, 0.006, 1)
    bpy.context.view_layer.objects.active = c
    bpy.ops.object.modifier_apply(modifier="Bevel")
    hx, hy, hz = (d / 2 for d in CAST)
    x, y, z = center
    face_z = z + (hz if top else -hz)
    ka.cut(c, oval("aperture", (x, y, face_z), "Z", 0.124, 0.063, 0.08))
    ka.cut(c, oval("aperture", (x, y + sy * hy, z), "Y", 0.078, 0.051, 0.08))
    ka.cut(c, oval("aperture", (x + sx * hx, y, z), "X", 0.063, 0.051, 0.08))
    return c


def sheet(name: str, length: float, height: float, pitch: float, depth: float, material, rows: int,
          basis: Matrix, at) -> tuple[bpy.types.Object, bpy.types.Object]:
    """A corrugated sheet turned by `basis` (columns: where its profile, its face and its ribs go)
    and moved to `at`: (the game mesh's, with sharp folds; the high mesh's, every fold rounded,
    which the normal map carries)."""
    low = ka.corrugated_panel(name, length, height, pitch, depth, material, rows=rows)
    high = ka.corrugated_panel(name, length, height, pitch, depth, material, rows=rows, bend=BEND)
    for panel in (low, high):
        panel.data.transform(Matrix.Translation(at) @ basis.to_4x4())
    return low, high


def build_body(v: dict, paint, steel, rubber, ply, ink) -> tuple[list, list, list]:
    """The body's parts in both meshes, the high mesh's detail (the rounded sheet and the
    stencils), and the game mesh's sheet."""
    parts: list[bpy.types.Object] = []
    detail: list[bpy.types.Object] = []
    proxy: list[bpy.types.Object] = []
    add = parts.append
    hx, hy = L / 2, W / 2
    # Corner posts and castings.
    for sx in (-1, 1):
        for sy in (-1, 1):
            post = ka.box("post", (POST, POST, H - 2 * CAST[2]),
                          (sx * (hx - POST / 2), sy * (hy - POST / 2), H / 2), paint)
            ka.add_bevel(post, 0.006, 2)
            add(post)
            for z, top in ((CAST[2] / 2, False), (H - CAST[2] / 2, True)):
                add(casting((sx * (hx - CAST[0] / 2), sy * (hy - CAST[1] / 2), z), sx, sy, top, steel))
    # Side rails along X.
    inner_x = L - 2 * CAST[0]
    rails = []
    for sy in (-1, 1):
        rails.append(ka.box("rail_bottom", (inner_x, 0.10, RAIL_BOTTOM), (0, sy * (hy - 0.05), RAIL_BOTTOM / 2), paint))
        rails.append(ka.box("rail_top", (inner_x, 0.08, RAIL_TOP), (0, sy * (hy - 0.04), H - RAIL_TOP / 2), paint))
    # End rails along Y: the blank end, and the door end's sill and header.
    inner_y = W - 2 * CAST[1]
    rails.append(ka.box("end_bottom", (0.10, inner_y, RAIL_BOTTOM), (-(hx - 0.05), 0, RAIL_BOTTOM / 2), paint))
    rails.append(ka.box("end_top", (0.08, inner_y, RAIL_TOP), (-(hx - 0.04), 0, H - RAIL_TOP / 2), paint))
    rails.append(ka.box("door_sill", (0.12, inner_y, SILL), (hx - 0.06, 0, SILL / 2), paint))
    rails.append(ka.box("door_header", (0.12, inner_y, HEADER), (hx - 0.06, 0, H - HEADER / 2), paint))
    for p in rails:
        ka.add_bevel(p, 0.006, 2)
    parts += rails

    # Corrugated long sides and blank end, eight rows up so they can dent.
    side_len = L - 2 * POST
    side_h = H - RAIL_BOTTOM - RAIL_TOP
    sides = []
    for sy in (1, -1):
        # Profile along X, face out of the side, ribs up.
        basis = Matrix(((1, 0, 0), (0, sy, 0), (0, 0, 1))).transposed() if sy > 0 else \
            Matrix(((-1, 0, 0), (0, -1, 0), (0, 0, 1))).transposed()
        sides.append(sheet("side", side_len, side_h, PITCH, DEPTH, paint, 8, basis, (0, sy * (hy - 0.004), RAIL_BOTTOM)))
    end = sheet("end", W - 2 * POST, side_h, PITCH, DEPTH, paint, 6,
                Matrix(((0, 1, 0), (-1, 0, 0), (0, 0, 1))).transposed(), (-(hx - 0.004), 0, RAIL_BOTTOM))
    # The roof: a shallow corrugation across the width, just under the top rails' tops.
    roof = sheet("roof", L - 0.2, W - 0.16, 0.3, 0.02, paint, 4,
                 Matrix(((1, 0, 0), (0, 0, 1), (0, -1, 0))).transposed(), (-0.02, (W - 0.16) / 2, H - 0.008))
    for low, high in sides + [end, roof]:
        proxy.append(low)
        detail.append(high)
    # The floor's plywood, seen when the doors are open.
    add(ka.box("floor", (L - 0.2, W - 0.2, 0.03), (0, 0, 0.11), ply))
    # Gasket frame round the door opening.
    add(ka.box("gasket_top", (0.02, inner_y, 0.03), (hx - 0.005, 0, H - HEADER - 0.015), rubber))
    add(ka.box("gasket_bottom", (0.02, inner_y, 0.03), (hx - 0.005, 0, SILL + 0.015), rubber))
    # Cam keepers on the header and sill, one at each end of every lock bar: a block with the
    # slot the cam turns into.
    for side in (1, -1):
        for off in BARS:
            y = side * off
            for z, up in ((SILL - 0.06, -1), (H - HEADER + 0.06, 1)):
                keeper = ka.box("cam_keeper", (0.08, 0.10, 0.07), (hx + 0.04, y, z), steel)
                ka.cut(keeper, ka.box("slot", (0.06, 0.12, 0.034), (hx + 0.05, y, z - up * 0.005)))
                add(keeper)

    # Stencils: owner mark top left, the 40 cm asset code bottom right, SITE PROPERTY under it,
    # painted over the corrugation.
    for (_, panel), sy in zip(sides, (1, -1)):
        face_y = sy * hy
        right = 1 if sy < 0 else -1  # The +X or -X way, as seen from outside that face
        for name, body, size, at in (("owner", v["owner"], 0.34, (-right * (hx - 1.0), H - 0.5)),
                                     ("code", v["code"], 0.4, (right * 1.6, 0.85)),
                                     ("site", "SITE PROPERTY", 0.16, (right * 1.6, 0.5))):
            detail.append(ka.stencil(name, body, size, ink, (at[0], face_y, at[1]), (0, sy, 0), panel,
                                     spray=0.006, grid=0.012, resolution=6))
    return parts, detail, proxy


def build_leaf(side: int, paint, steel, rubber, ink, code: str) -> tuple[bpy.types.Object, bpy.types.Object]:
    """One door leaf, side +1 (left, +Y) or -1 (right, -Y), hinged at its outer edge: (game mesh,
    high mesh)."""
    hx = L / 2
    height = H - SILL - HEADER
    y0, y1 = side * 0.006, side * (0.006 + LEAF)
    cy = (y0 + y1) / 2
    parts: list[bpy.types.Object] = []
    detail: list[bpy.types.Object] = []
    # The frame: stiles at both edges, rails top and bottom, round a corrugated panel with
    # level ribs.
    for y in (y0 + side * 0.03, y1 - side * 0.03):
        parts.append(ka.box("stile", (0.04, 0.06, height), (hx, y, SILL + height / 2), paint))
    for z in (SILL + 0.04, SILL + height - 0.04):
        parts.append(ka.box("leaf_rail", (0.04, LEAF - 0.12, 0.08), (hx, cy, z), paint))
    for p in parts:
        ka.add_bevel(p, 0.005, 2)
    infill, infill_high = sheet("infill", height - 0.16, LEAF - 0.12, 0.25, 0.026, paint, 4,
                                Matrix(((0, 0, 1), (1, 0, 0), (0, 1, 0))).transposed(),
                                (hx + 0.005, cy - (LEAF - 0.12) / 2, SILL + height / 2))
    detail.append(infill_high)
    # Lock bars, their guide brackets, cams at the ends, and a horizontal handle on each: the
    # inner bar's points out at 1.15 m, the outer bar's in at 1.0 m, each held by a retainer.
    for k, off in enumerate(BARS):
        y = side * off
        top, bottom = H - HEADER + 0.06, SILL - 0.06
        parts.append(ka.cylinder("lockbar", 0.017, top - bottom, (BAR_X, y, (top + bottom) / 2), "Z", 12, steel))
        for z in (bottom, top):
            cam = ka.box("cam", (0.05, 0.075, 0.03), (BAR_X - 0.01, y + side * 0.012, z), steel)
            ka.add_bevel(cam, 0.004, 1)
            parts.append(cam)
        for z in (SILL + 0.35, 1.5, SILL + height - 0.35):
            parts += [ka.box("guide_plate", (0.012, 0.11, 0.07), (hx + 0.026, y, z), steel),
                      ka.box("guide_strap", (0.012, 0.07, 0.045), (BAR_X + 0.023, y, z), steel),
                      ka.box("guide_side", (0.05, 0.012, 0.045), (BAR_X, y - 0.029, z), steel),
                      ka.box("guide_side", (0.05, 0.012, 0.045), (BAR_X, y + 0.029, z), steel)]
            # Its bolts are baked: 5 mm proud, inside the bake's reach.
            for dy in (-0.042, 0.042):
                detail.append(ka.cylinder("bolt", 0.008, 0.005, (hx + 0.0345, y + dy, z), "X", 6, steel))
        z = 1.15 if k == 0 else 1.0
        way = side if k == 0 else -side
        parts.append(ka.cylinder("handle_hub", 0.028, 0.07, (BAR_X, y, z), "Z", 12, steel))
        lever = ka.box("handle", (0.03, 0.34, 0.035), (BAR_X + 0.012, y + way * 0.19, z), steel)
        ka.add_bevel(lever, 0.006, 2)
        parts.append(lever)
        retainer = ka.box("handle_retainer", (0.04, 0.05, 0.07), (hx + 0.04, y + way * 0.37, z), steel)
        ka.add_bevel(retainer, 0.004, 1)
        parts.append(retainer)
    # Hinges: a blade on the leaf, round a knuckle on the hinge line.
    outer = side * (W / 2 - 0.17)
    for z in (0.45, 1.0, 1.55, 2.1):
        blade = ka.box("hinge_blade", (0.014, 0.16, 0.07), (hx + 0.027, outer - side * 0.08, z), steel)
        ka.add_bevel(blade, 0.003, 1)
        parts.append(blade)
        parts.append(ka.cylinder("hinge", 0.03, 0.14, (hx + 0.02, outer, z), "Z", 12, steel))
        parts.append(ka.cylinder("hinge_pin", 0.012, 0.18, (hx + 0.02, outer, z), "Z", 8, steel))
    parts.append(ka.box("gasket_edge", (0.02, 0.012, height), (hx - 0.005, side * 0.006, SILL + height / 2), rubber))
    if side > 0:
        # The data plate (CSC approval and the owner's), riveted to the left leaf.
        plate = ka.box("data_plate", (0.004, 0.22, 0.13), (hx + 0.022, cy, 1.55), steel)
        ka.add_bevel(plate, 0.002, 1)
        parts.append(plate)
        for dy in (-0.1, 0.1):
            for dz in (-0.055, 0.055):
                detail.append(ka.cylinder("rivet", 0.004, 0.003, (hx + 0.0255, cy + dy, 1.55 + dz), "X", 6, steel))
        for k, line in enumerate(("CSC SAFETY APPROVAL", f"OWNER {code}", "MAX GROSS 30480 KG", "TARE 2230 KG")):
            detail.append(ka.stencil("plate_text", line, 0.014, ink, (hx + 0.0255, cy, 1.585 - 0.024 * k), (1, 0, 0),
                                     resolution=3))
    else:
        detail.append(ka.stencil("door_code", code, 0.085, ink, (hx + 0.005, side * 0.6, 2.05), (1, 0, 0), infill,
                                 spray=0.003, grid=0.006, resolution=4))
    mesh_name = "Door_L" if side > 0 else "Door_R"
    obj, high = ka.high_low(mesh_name, parts, detail, [infill], soft=True)
    # Pivot on the hinge line, at the sill.
    pivot = Vector((hx + 0.02, outer, SILL))
    for o in (obj, high):
        o.data.transform(Matrix.Translation(-pivot))
        o.location = pivot
    return obj, high


def panel_field(seed: int, dents: float):
    """The body's deformation: dents in the long sides and the roof, the sheet oil-canned
    between its ribs, and the roof sagging 1.5 cm at its middle. Only the sheet moves: the posts,
    rails and castings keep their lines."""
    rng = random.Random(seed)
    hx, hy = L / 2, W / 2
    side = []
    for sy in (1, -1):
        for _ in range(3 + int(4 * dents)):
            r = rng.uniform(0.2, 0.55)
            side.append(((rng.uniform(-hx + POST + r, hx - POST - r), sy * hy, rng.uniform(0.35 + r * 0.5, 2.2 - r * 0.5)),
                         (0, -sy * rng.uniform(0.008, 0.03) * (0.6 + dents), 0), r))
    roof = [((rng.uniform(-2.4, 2.4), rng.uniform(-0.7, 0.7), H), (0, 0, -rng.uniform(0.01, 0.025)), rng.uniform(0.3, 0.6))
            for _ in range(3)]
    roof.append(((0.0, 0.0, H), (0, 0, -0.015), 3.2))
    dent_side, dent_roof = ka.dents(*side), ka.dents(*roof)
    wave = ka.waviness(0.004, 0.9, seed, (0.0, 1.0, 0.0))

    def offset(p: Vector) -> Vector:
        in_x = abs(p.x) < hx - POST and RAIL_BOTTOM < p.z < H - RAIL_TOP
        if in_x and abs(p.y) > hy - 0.05:
            return dent_side(p) + wave(p) * math.sin(math.pi * (p.z - RAIL_BOTTOM) / (H - RAIL_BOTTOM - RAIL_TOP))
        if abs(p.x) < hx - CAST[0] and abs(p.y) < hy - 0.08 and p.z > H - 0.04:
            return dent_roof(p)
        return Vector((0.0, 0.0, 0.0))
    return offset


def leaf_field(seed: int, side: int):
    """A door leaf's knocks: two shallow dents in its panel."""
    rng = random.Random(seed + side)
    cy = side * (0.006 + LEAF / 2)
    dents = ka.dents(*[((L / 2, cy + rng.uniform(-0.3, 0.3), rng.uniform(0.6, 2.0)), (-rng.uniform(0.006, 0.015), 0, 0),
                        rng.uniform(0.15, 0.3)) for _ in range(2)])
    return lambda p: dents(p) if abs(p.y - cy) < LEAF / 2 - 0.07 and p.x < L / 2 + 0.03 else Vector((0.0, 0.0, 0.0))


def build(variant: str = "htr") -> dict:
    v = VARIANTS[variant]
    wear = v["wear"]
    paint = ka.painted_steel("MI_Steel_Painted_Worn", wear)
    steel = ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.65), rust=0.5, detail=4.0, streaks=0.4,
                          streak_band=(0.0, H), roof_dirt=wear.roof_dirt, lower_grime=wear.lower_grime)
    rubber = ka.rubber("MI_Rubber_Black", "#1E1F20", 0.78)
    ply = ka.wood("MI_Plywood_Weathered", "#9C8564", grain_axis="X", rough=(0.8, 0.92))
    ink = ka.stencil_paint("MI_Stencil_White", "#E8E6E0", wear)

    parts, detail, proxy = build_body(v, paint, steel, rubber, ply, ink)
    body, body_high = ka.high_low(v["mesh"], parts, detail, proxy, soft=True)
    ka.deform([body, body_high], panel_field(v["seed"], wear.dents))
    doors, highs = [], {body.name: body_high}
    for side in (1, -1):
        d, h = build_leaf(side, paint, steel, rubber, ink, v["code"])
        ka.deform([d, h], leaf_field(v["seed"], side))
        d.name = f"{v['mesh']}_{d.name}"
        d.data.name = d.name
        h.name = f"HIGH_{d.name}"
        doors.append(d)
        highs[d.name] = h

    # Collision (the doc: a box for the body; a box per door leaf).
    ka.collision_box(v["mesh"], 0, (L, W, H), (0, 0, H / 2))
    height = H - SILL - HEADER
    for d in doors:
        side = 1 if d.name.endswith("_L") else -1
        ka.collision_box(d.name, 0, (0.05, LEAF, height), (L / 2 - 0.01, side * (0.006 + LEAF / 2), SILL + height / 2))

    hx, hy = L / 2, W / 2
    sockets = {
        "SKT_Dog_Top_01": (-1.5, 0, H + 0.01),
        "SKT_Dog_Top_02": (1.5, 0, H + 0.01),
        "SKT_Dog_SideN_01": (0, hy, 1.3),
        "SKT_Dog_SideS_01": (0, -hy, 1.3),
        "SKT_Dog_End_01": (-hx, 0, 1.3),
        "SKT_Dog_Door_01": (hx + 0.05, 0.6, 1.3),
        "SKT_Dog_Door_02": (hx + 0.05, -0.6, 1.3),
    }
    i = 1
    for sx in (-1, 1):
        for sy in (-1, 1):
            sockets[f"SKT_Dog_Hook_{i:02d}"] = (sx * (hx - CAST[0] / 2), sy * (hy - CAST[1] / 2), H)
            i += 1
    # The doc names the long sides N and S (X is the container's length), the blank end, the
    # doors and the top corner castings; each socket faces out of its face.
    normals = {"SKT_Dog_SideN_01": (0, 1, 0), "SKT_Dog_SideS_01": (0, -1, 0), "SKT_Dog_End_01": (-1, 0, 0),
               "SKT_Dog_Door_01": (1, 0, 0), "SKT_Dog_Door_02": (1, 0, 0)}
    for name, loc in sockets.items():
        ka.socket(name, loc, body, normals.get(name, (0, 0, 1) if "_Hook_" in name else None))
    # Its walls are single sheets, seen from inside once the doors are open: two-sided. The
    # projected bake reaches 6 mm, past the stencils' 2 mm and short of the door gear's gaps.
    return {"name": v["mesh"], "meshes": [body, *doors], "size": (L, W, H), "budget": 16000, "res": 4096,
            "high": highs, "cage": 0.006, "double_sided": [body.name, *(d.name for d in doors)]}
