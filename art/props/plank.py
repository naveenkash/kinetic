"""Plank: docs/objects/props-light/plank.md.

A 240 x 23 x 4 cm sawn softwood scaffold board with galvanized end bands and a few rusted nail
heads; the top face weathered grey. Pivot at the bottom centre, X+ along the length.
"""
from __future__ import annotations

import kinetic_art as ka

L, W, T = 2.40, 0.23, 0.04


def build(variant: str = "yard") -> dict:
    timber = ka.wood("MI_Timber_Rough", "#7A5C3A", weathered="#9C8564")
    galv = ka.galvanized()
    rust = ka.flat_material("MI_Steel_Rusted", "#7A3F1C", 0.85, 0.3)
    parts = []
    board = ka.box("board", (L - 0.002, W, T), (0, 0, T / 2), timber)
    ka.add_bevel(board, 0.004, 2)
    parts.append(board)
    for sx in (-1, 1):
        parts.append(ka.box("band", (0.04, W + 0.004, T + 0.002), (sx * (L / 2 - 0.03), 0, (T + 0.002) / 2), galv))
    for x in (-0.9, -0.3, 0.4, 1.0):
        for y in (-0.07, 0.07):
            parts.append(ka.cylinder("nail", 0.005, 0.002, (x + (y * 0.4), y, T + 0.001), "Z", 6, rust))
    # The Yard's planks are branded C4PW (the doc's PROPOSED supplier), burned in near one end.
    burn = ka.flat_material("MI_Timber_Burned", "#2B1E14", 0.9)
    parts.append(ka.stencil("brand", "C4PW", 0.05, burn, (L / 2 - 0.16, 0, T + 0.0006), (0, 0, 1), up=(0, 1, 0)))
    name = "SM_Light_Plank"
    obj = ka.join(name, parts)
    ka.collision_box(name, 0, (L, W, T), (0, 0, T / 2))
    for s, (loc, normal) in {"SKT_Dog_Top_01": ((0, 0, T), None), "SKT_Dog_End_01": ((L / 2 - 0.001, 0, T / 2), (1, 0, 0)),
                             "SKT_Dog_End_02": ((-L / 2 + 0.001, 0, T / 2), (-1, 0, 0))}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "size": (L, W, T), "res": 1024, "budget": 1200}
