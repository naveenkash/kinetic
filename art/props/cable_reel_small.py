"""Cable reel (small): docs/objects/props-light/cable-reel-small.md.

60 cm plywood flanges with galvanized rim bands on an axle, 40 cm wide, half-wound with 12 mm
wire rope whose end comes over a flange rim and is U-bolted to its face, 30 cm hanging free.
Authored rolling: the pivot is the axle centre, X+ along the axle.

The outer layer of wraps is the high mesh; the game mesh carries a drum of rope in its place,
the wraps baked into its normal, colour and AO. The steel core and the inner layers are under
the rope, so neither mesh has them, and the collision's core is the rope's outside (18 cm), not
the doc's 10 cm bare core: the reel is half wound.
"""
from __future__ import annotations

import math

import kinetic_art as ka

R, WIDTH = 0.30, 0.40
FLANGE_T = 0.018
WOUND = 0.19   # Rope radius at half wound


def build(variant: str = "htr") -> dict:
    # The doc's wear: the flanges grey outside and still brown inside, the rims dented from
    # grates, the axle's ends bright where the stand's bearings ran.
    ply = ka.wood("MI_Plywood_Weathered", "#9C8564", weathered="#8F8B83", grain_axis="Y", detail=3.0, ply_axis="X",
                  checks=0.3)
    galv = ka.galvanized(detail=8.0)
    core_paint = ka.painted_steel("MI_Steel_Painted_Worn", ka.Wear(paint="#E8620C", chips=0.4, scrape_height=0.0,
                                                                   streak_band=(-0.05, 0.05), detail=8.0, edge=0.004,
                                                                   grime_reach=0.03))
    rope = ka.bare_steel("MI_Chain_Oiled", "#3C3E42", rough=(0.3, 0.42), rust=0.1, detail=12.0, oil=0.8, dust=0.2)
    rust = ka.rusted("MI_Steel_Rusted", detail=12.0)
    orange = ka.ink("MI_Stencil_Orange", "#E8620C", "#9EA3A6", detail=10.0)
    parts = []
    hx = WIDTH / 2 - FLANGE_T / 2
    for sx in (-1, 1):
        parts.append(ka.cylinder("flange", R - 0.006, FLANGE_T, (sx * hx, 0, 0), "X", 32, ply))
        parts.append(ka.ring("rim", R - 0.007, 0.007, (sx * hx, 0, 0), "X", 32, 4, galv))  # Flush with R
        for k in range(6):
            a = 2 * math.pi * k / 6
            parts.append(ka.cylinder("tie_bolt", 0.009, 0.006, (sx * (hx + FLANGE_T / 2 + 0.002), 0.2 * math.cos(a),
                                                                0.2 * math.sin(a)), "X", 6, rust))
    parts.append(ka.cylinder("axle", 0.025, WIDTH + 0.06, (0, 0, 0), "X", 12, core_paint))
    # The outer layer of rope wraps across the drum at the half-wound radius.
    wraps = []
    inner = WIDTH - 2 * FLANGE_T
    for i in range(int(inner / 0.013)):
        x = -hx + FLANGE_T / 2 + 0.007 + i * 0.013
        wraps.append(ka.ring("wrap", WOUND - 0.012, 0.0062, (x, 0, 0), "X", 28, 5, rope))
    # The proxy is as big as the wraps' outside, so the free end sits on it as on the wraps.
    drum = [ka.cylinder("rope_drum", WOUND - 0.006, inner, (0, 0, 0), "X", 32, rope)]
    # The free end: off the top wrap, up and over the flange's rim, down its outer face past the
    # U-bolt, 30 cm hanging. FACE is the flange's outer face.
    face = hx + FLANGE_T / 2
    out = face + 0.008
    parts.append(ka.tube("free_end", [(hx - 0.03, 0.0, WOUND), (hx - 0.012, 0.02, R + 0.008), (face, 0.035, R + 0.012),
                                      (out, 0.05, R - 0.01), (out, 0.08, 0.16), (out, 0.10, -0.02),
                                      (out + 0.004, 0.12, -0.14)], 0.0062, rope))
    parts.append(ka.tube("u_bolt", [(face, 0.064, 0.13), (out + 0.009, 0.064, 0.13), (out + 0.009, 0.098, 0.13),
                                    (face, 0.098, 0.13)], 0.004, rust))
    parts.append(ka.box("plate", (0.003, 0.12, 0.06), (hx + FLANGE_T / 2 + 0.002, -0.12, -0.12), galv))
    parts.append(ka.stencil("lot", "HTR-RC-3340", 0.022, orange, (hx + FLANGE_T / 2 + 0.005, -0.12, -0.12), (1, 0, 0)))
    name = "SM_Light_CableReel_HTR"
    obj, high = ka.high_low(name, parts, wraps, drum, soft=True)
    # The doc's collision: the two flanges and the core, all on the X axis.
    for i, sx in enumerate((-1, 1)):
        ka.collision_cylinder(name, i, R, sx * hx - FLANGE_T / 2, sx * hx + FLANGE_T / 2, 16, axis="X")
    ka.collision_cylinder(name, 2, WOUND - 0.006, -hx + FLANGE_T / 2, hx - FLANGE_T / 2, 16, axis="X")
    for s, (loc, normal) in {"SKT_Dog_End_01": ((WIDTH / 2 + 0.03, 0, 0), (1, 0, 0)),
                             "SKT_Dog_End_02": ((-WIDTH / 2 - 0.03, 0, 0), (-1, 0, 0)),
                             "SKT_Dog_Rim_01": ((hx, 0, R), (0, 0, 1)),
                             "SKT_Dog_Top_01": ((0, 0, WOUND - 0.006), None)}.items():
        ka.socket(s, loc, obj, normal)
    # Previewed standing on its flanges; the pivot stays on the axle.
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.01, "size": (WIDTH, 2 * R, 2 * R),
            "res": 2048, "preview_lift": R, "budget": 2000}
