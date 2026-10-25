"""Steel crate: docs/objects/props-medium/steel-crate.md.

120 x 100 x 90 cm, a squat welded box with rolled corners, a hinged lid with two hasps, two
forklift channels in the base and four lifting lugs at the top corners. Pivot at the bottom
centre, X+ along the long axis and toward the front (the hasps). Variants: default grey, htr,
guild.
"""
from __future__ import annotations

import kinetic_art as ka

L, W, H = 1.20, 1.00, 0.90
FORK = 0.10   # The channels raise the box
LID = 0.06

# Wear is the doc's: HTR 0.3, Guild 0.7. HTR has an orange band at mid height and an asset tag;
# Guild's code is ochre on a black field with the hitch glyph; Yard crates carry a chalk tally.
VARIANTS = {
    "default": dict(paint="#5B6670", text=None, ink="#E6E4DD", chips=0.35),
    "htr": dict(paint="#4A4D50", text=("HTR-RC-4471", "RECLAIMED / DO NOT RETURN"), ink="#E8620C", chips=0.3),
    "guild": dict(paint="#C8912A", text=("LG-0932", None), ink="#C8912A", chips=0.7),
}


def build(variant: str = "htr") -> dict:
    v = VARIANTS[variant]
    paint = ka.painted_steel("MI_Steel_Painted_Worn", ka.Wear(paint=v["paint"], chips=v["chips"], scrape_height=0.0,
                                                              rust_streaks=0.55, roof_dirt="#1B1A18",
                                                              streak_band=(0.1, H)))
    steel = ka.flat_material("MI_Steel_Bare", "#5C5F63", 0.55, 1.0)
    ink = ka.flat_material("MI_Stencil", v["ink"], 0.6)
    parts = []
    body = ka.box("body", (L, W, H - FORK - LID), (0, 0, FORK + (H - FORK - LID) / 2), paint)
    ka.add_bevel(body, 0.03, 3)
    parts.append(body)
    lid = ka.box("lid", (L + 0.02, W + 0.02, LID), (0, 0, H - LID / 2), paint)
    ka.add_bevel(lid, 0.02, 2)
    parts.append(lid)
    # Panel seams: two welded ribs around the body.
    for z in (0.35, 0.6):
        rib = ka.box("rib", (L + 0.012, W + 0.012, 0.03), (0, 0, z), paint)
        ka.add_bevel(rib, 0.006)
        parts.append(rib)
    # Fork channels along Y.
    for x in (-0.33, 0.33):
        ch = ka.box("fork_channel", (0.24, W, FORK), (x, 0, FORK / 2), steel)
        ka.add_bevel(ch, 0.006)
        parts.append(ch)
    # Lifting lugs at the top corners, and two hasps on the lid's front edge (X+).
    for sx in (-1, 1):
        for sy in (-1, 1):
            parts.append(ka.ring("lug", 0.035, 0.009, (sx * (L / 2 - 0.08), sy * (W / 2 + 0.006), H - LID - 0.05), "Y",
                                 16, 6, steel))
    for y in (-0.25, 0.25):
        parts.append(ka.box("hasp", (0.02, 0.06, 0.12), (L / 2 + 0.02, y, H - LID - 0.02), steel))
        parts.append(ka.box("staple", (0.03, 0.03, 0.04), (L / 2 + 0.015, y, H - LID - 0.09), steel))
    # Hinges at the back.
    for y in (-0.3, 0.3):
        parts.append(ka.cylinder("hinge", 0.018, 0.14, (-L / 2 - 0.01, y, H - LID), "Y", 12, steel))
    # Markings between the ribs: the code high on the east side (Y-) and low on the front (X+),
    # below the hasps; the line low on the east side.
    if variant == "htr":
        band = ka.flat_material("MI_Paint_HTR_Orange", "#E8620C", 0.55)
        parts.append(ka.box("band", (L + 0.004, W + 0.004, 0.06), (0, 0, 0.475), band))
        parts.append(ka.box("asset_tag", (0.003, 0.08, 0.05), (L / 2 + 0.003, 0.3, 0.27), steel))
    if variant == "guild":
        field = ka.flat_material("MI_Paint_Black", "#1C1B19", 0.6)
        parts.append(ka.box("code_field", (0.86, 0.003, 0.2), (0.0, -W / 2 - 0.0015, 0.725), field))
        parts.append(ka.box("code_field_e", (0.003, 0.6, 0.13), (L / 2 + 0.0015, 0, 0.225), field))
        parts.append(ka.ring("hitch_glyph", 0.035, 0.006, (-0.35, -W / 2 - 0.006, 0.725), "Y", 16, 4, ink))
    if variant in ("htr", "guild"):
        chalk = ka.flat_material("MI_Chalk", "#D8D4C8", 0.95)
        for k in range(5):
            stroke = ka.box("chalk_tally", (0.006, 0.002, 0.08), (0, 0, 0), chalk)
            ka.transform(stroke, (-L / 2 + 0.25 + 0.025 * k, W / 2 + 0.003, 0.25), rotation_y=0.0 if k < 4 else 1.1)
            parts.append(stroke)
    if v["text"]:
        code, line = v["text"]
        parts.append(ka.stencil("code", code, 0.12, ink, (0.04 if variant == "guild" else 0.0, -W / 2 - 0.004, 0.725),
                                (0, -1, 0)))
        parts.append(ka.stencil("code_e", code, 0.08, ink, (L / 2 + 0.004, 0, 0.225), (1, 0, 0)))
        if line:
            parts.append(ka.stencil("line", line, 0.045, ink, (0.0, -W / 2 - 0.004, 0.25), (0, -1, 0)))

    name = {"default": "SM_Medium_SteelCrate", "htr": "SM_Medium_SteelCrate_HTR", "guild": "SM_Medium_SteelCrate_Guild"}[variant]
    obj = ka.join(name, parts)
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
            sockets[f"SKT_Dog_Hook_{i:02d}"] = ((sx * (L / 2 - 0.08), sy * (W / 2 + 0.015), H - LID - 0.05), (0, sy, 0))
            i += 1
    for s, (loc, normal) in sockets.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "size": (L, W, H), "res": 2048, "budget": 6000}
