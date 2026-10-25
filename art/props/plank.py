"""Plank: docs/objects/props-light/plank.md.

A 240 x 23 x 4 cm sawn softwood scaffold board with galvanized end bands and a few rusted nail
heads; the top face weathered grey and walked dark down its middle, the ends split 5 to 10 cm
back from the bands. Pivot at the bottom centre, X+ along the length.

The C4PW brand is burned into the top near one end: the high mesh, baked into the board.
"""
from __future__ import annotations

import random

from mathutils import Matrix, Vector

import kinetic_art as ka

L, W, T = 2.40, 0.23, 0.04


def build(variant: str = "yard") -> dict:
    timber = ka.wood("MI_Timber_Rough", "#7A5C3A", weathered="#9C8564", detail=2.0, checks=0.6, walked=True)
    galv = ka.galvanized(detail=4.0)
    rust = ka.rusted("MI_Steel_Rusted", detail=12.0)
    parts, detail = [], []
    board = ka.box("board", (L - 0.002, W, T), (0, 0, T / 2), timber)
    # The ends split back past the bands: thin cuts through the board, along the grain.
    rng = random.Random(2400)
    for sx in (-1, 1):
        for y in (rng.uniform(-0.08, -0.03), rng.uniform(0.02, 0.07)):
            length = rng.uniform(0.05, 0.1)
            split = ka.box("split", (length * 2, rng.uniform(0.0012, 0.0022), T * 0.8), (sx * L / 2, y, T * 0.45))
            ka.cut(board, split)
    ka.add_bevel(board, 0.004, 2)
    parts.append(board)
    for sx in (-1, 1):
        band = ka.box("band", (0.04, W + 0.004, T + 0.002), (sx * (L / 2 - 0.03), 0, (T + 0.002) / 2), galv)
        ka.add_bevel(band, 0.0015, 1)
        parts.append(band)
    for x in (-0.9, -0.3, 0.4, 1.0):
        for y in (-0.07, 0.07):
            parts.append(ka.lathe("nail", [(0.0, T - 0.001), (0.005, T - 0.001), (0.0048, T + 0.0006), (0.003, T + 0.0012),
                                           (0.0, T + 0.0014)], 8, rust))
            parts[-1].data.transform(Matrix.Translation((x + y * 0.4, y, 0.0)))
    # The Yard's planks are branded C4PW (the doc's PROPOSED supplier), burned in near one end.
    burn = ka.ink("MI_Timber_Burned", "#2B1E14", "#7A5C3A", wear=0.15)
    detail.append(ka.stencil("brand", "C4PW", 0.05, burn, (L / 2 - 0.16, 0, T + 0.0006), (0, 0, 1), up=(0, 1, 0),
                             spray=0.002, resolution=4))
    name = "SM_Light_Plank"
    obj, high = ka.high_low(name, parts, detail, [], soft=True)
    # A touch of twist: two opposite corners lift 3 mm off the floor, the board rocking on the
    # other two.
    ka.deform([obj, high], lambda p: Vector((0.0, 0.0, 0.003 * max(0.0, (p.x / (L / 2)) * (p.y / (W / 2))))))
    ka.collision_box(name, 0, (L, W, T), (0, 0, T / 2))
    for s, (loc, normal) in {"SKT_Dog_Top_01": ((0, 0, T), None), "SKT_Dog_End_01": ((L / 2 - 0.001, 0, T / 2), (1, 0, 0)),
                             "SKT_Dog_End_02": ((-L / 2 + 0.001, 0, T / 2), (-1, 0, 0))}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.003, "size": (L, W, T), "res": 2048,
            "budget": 1200}

