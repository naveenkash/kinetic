"""Floodlight stand: docs/objects/props-light/floodlight-stand.md.

A tripod on a 90 cm footprint, a 240 cm mast of 38 mm tube, a 100 cm galvanized crossbar (104 cm
over its end caps) with twin 30 x 20 x 10 cm LED heads on yokes, a coil of cable at the foot and a sandbag on one leg.
Pivot at the bottom centre of the tripod, the heads facing X+. The lenses glow; the bake carries
it as an Emissive map. Variants: office (default, yellow, OFFICE OF THE LAY / MATCH LIGHTING /
UNIT 14 up the mast, a LayCast sticker on a head, the Yard's Kessl Harness Leasing strip on the
crossbar), htr (orange, HTR-RC-6002) and guild (ochre, the older pattern with one head and the
hitch glyph).

The lettering, the sticker and the sponsor strip are the high mesh, baked onto the mast, head
and crossbar, so the stand keeps to the doc's 2,000 tris.
"""
from __future__ import annotations

import math

from mathutils import Matrix, Vector

import kinetic_art as ka

FOOT, MAST, HUB = 0.45, 2.40, 0.55
TUBE = 0.019                       # 38 mm mast and legs
BAR_Z = MAST - 0.04                # Crossbar height
TILT = math.radians(20)            # Heads aimed 20 degrees down
HEAD = (0.10, 0.30, 0.20)          # Depth (X), width (Y), height (Z)
VARIANTS = {
    "office": dict(mesh="SM_Light_FloodlightStand_Office", paint="#F2C230", heads=(-0.35, 0.35), bar=1.04,
                   mast_text=("OFFICE OF THE LAY", "MATCH LIGHTING", "UNIT 14")),
    "htr": dict(mesh="SM_Light_FloodlightStand_HTR", paint="#E8620C", heads=(-0.35, 0.35), bar=1.04,
                mast_text=("HTR-RC-6002",)),
    "guild": dict(mesh="SM_Light_FloodlightStand_Guild", paint="#C8912A", heads=(0.0,), bar=0.4, mast_text=()),
}


def head_frame(y: float) -> Matrix:
    """The head's placement: in front of the crossbar, below it, tilted down by TILT. A head on
    the mast's line (the Guild's single one) stands further forward, clear of the mast."""
    forward = 0.08 if abs(y) > 0.2 else 0.11
    return Matrix.Translation((forward, y, MAST - 0.12)) @ Matrix.Rotation(TILT, 4, "Y")


def placed(obj, frame: Matrix):
    obj.data.transform(frame)
    return obj


def build(variant: str = "office") -> dict:
    v = VARIANTS[variant]
    # The doc's wear: chipped at the feet and along the mast where it rides a shoulder, the mast
    # collar polished bare.
    wear = ka.Wear(paint=v["paint"], chips=0.25, scrape_height=0.04, streak_band=(0.0, MAST), detail=6.0, edge=0.003,
                   grime_reach=0.03, rust_streaks=0.3, streaks=0.15, dents=0.3, bare_bands=((HUB - 0.04, HUB + 0.04),))
    paint = ka.painted_steel("MI_Steel_Painted_Worn", wear)
    galv = ka.galvanized(detail=8.0)
    # Die-cast housings: matte grey, scorched grey-brown above the lens from the heat.
    housing = ka.painted_steel("MI_HTR_Grey_Matte", ka.Wear(paint="#4A4D50", paint_rough=(0.55, 0.7), primer=None,
                                                             chips=0.3, scrape_height=0.0, rust_streaks=0.0, streaks=0.0,
                                                             detail=8.0, edge=0.004, grime_reach=0.03, dust=0.5,
                                                             dirt_streaks=0.3, lower_grime=None,
                                                             extra={"crevice": "#1E1A16"}))
    lens = ka.emissive("MI_Glass_Lamp", "#E8EEF2", 6.0)
    rubber = ka.rubber("MI_Rubber_Black", "#1E1F20", 0.8)
    sand = ka.fabric("MI_Canvas_Natural", "#B9AE93", weave=0.003)
    white = ka.stencil_paint("MI_Stencil_White", "#E6E4DD", wear)
    parts, detail = [], []
    # Tripod legs from the hub down to rubber pads on the floor, and the stays. The legs end at
    # the pads' tops, so nothing reaches below the floor.
    feet = []
    for i in range(3):
        a = 2 * math.pi * i / 3 + math.pi / 2
        foot = (FOOT * math.cos(a), FOOT * math.sin(a), 0.012)
        feet.append(foot)
        parts.append(ka.tube("leg", [(0, 0, HUB), foot], TUBE * 0.9, paint))
        parts.append(ka.tube("stay", [(0.0, 0.0, 0.3), (foot[0] * 0.5, foot[1] * 0.5, HUB * 0.5)], 0.008, paint))
        parts.append(ka.cylinder("foot_pad", 0.03, 0.012, (foot[0], foot[1], 0.006), "Z", 10, rubber))
    mast = ka.tube("mast", [(0, 0, 0.3), (0, 0, MAST)], TUBE, paint, sides=12)
    parts.append(mast)
    parts.append(ka.cylinder("clamp", 0.03, 0.06, (0, 0, HUB), "Z", 12, galv))
    parts.append(ka.cylinder("crossbar", 0.014, v["bar"], (0, 0, BAR_Z), "Y", 12, galv))
    hx, hy, hz = (d / 2 for d in HEAD)
    for y in v["heads"]:
        frame = head_frame(y)
        fx = head_frame(y).translation.x - 0.03
        parts.append(ka.tube("yoke", [(0, y - 0.16, BAR_Z), (fx, y - 0.16, MAST - 0.09),
                                      (fx, y + 0.16, MAST - 0.09), (0, y + 0.16, BAR_Z)], 0.007, galv))
        head = ka.box("head", HEAD, (0, 0, 0), housing)
        ka.add_bevel(head, 0.012, 2)
        parts.append(placed(head, frame))
        parts.append(placed(ka.box("lens", (0.004, 0.26, 0.16), (hx + 0.002, 0, 0), lens), frame))
        # Cooling fins across the back face.
        for k in range(5):
            fin = ka.box("fin", (0.012, 0.28, 0.006), (-hx - 0.006, 0, -0.08 + 0.04 * k), housing)
            parts.append(placed(fin, frame))
    # The cable: from the crossbar down the back of the mast, touching it, out across the floor
    # to a coil clear of the feet, between the two rear legs.
    cable_r = 0.006
    back = -(TUBE + cable_r)
    parts.append(ka.tube("cable_down", [(-0.014, 0, BAR_Z - 0.012), (back, 0, BAR_Z - 0.05), (back, 0, 0.4),
                                        (-0.2, 0.0, cable_r)], cable_r, rubber))
    coil = (-0.34, 0.0)
    for k in range(4):
        parts.append(ka.ring("cable_coil", 0.12 - k * 0.012, cable_r, (coil[0], coil[1], cable_r + k * 0.011), "Z",
                             24, 5, rubber))
    # The sandbag lies over the rear leg near its foot, on the floor.
    bag = ka.box("sandbag", (0.28, 0.18, 0.09), (0, 0, 0), sand)
    ka.add_bevel(bag, 0.035, 3)
    fx, fy, _ = feet[1]
    ka.transform(bag, (fx * 0.85, fy * 0.85, 0.045), rotation_z=math.atan2(fy, fx) + math.pi / 2)
    parts.append(bag)
    # Markings, baked. Up the mast's front, reading bottom to top.
    z = 0.9
    for line in v["mast_text"]:
        length = 0.018 * 0.62 * len(line)
        detail.append(ka.stencil("mast_text", line, 0.018, white, (TUBE, 0, z + length / 2), (1, 0, 0), up=(0, -1, 0),
                                 cylinder=((0, 0, 0), (0, 0, 1), TUBE)))
        z += length + 0.05
    if variant == "guild":
        glyph = ka.stencil_paint("MI_Stencil_Black", "#1C1B19", wear)
        detail.append(ka.ring("hitch_glyph", 0.012, 0.002, (TUBE + 0.001, 0, 1.3), "X", 12, 4, glyph))
    if variant == "office":
        sticker = ka.ink("MI_Decal_LayCast", "#1C1B19", "#4A4D50", wear=0.25)
        frame = head_frame(v["heads"][1])
        at = frame @ Vector((0.0, hy + 0.001, 0.04))
        detail.append(ka.decal_plate("laycast_sticker", 0.06, 0.03, sticker, at, (0, 1, 0)))
        # The sponsor's strip, either side of the mast.
        navy = ka.ink("MI_Sponsor_Kessl_Navy", "#1F2A44", "#9EA3A6", wear=0.3)
        bar_axis = ((0, 0, BAR_Z), (0, 1, 0), 0.0155)
        for word, y in (("KESSL", -0.1), ("HARNESS LEASING", 0.11)):
            length = 0.011 * 0.62 * len(word) + 0.03
            detail.append(ka.cylinder("sponsor_strip", 0.0155, length, (0, y, BAR_Z), "Y", 12, navy))
            detail.append(ka.stencil("sponsor_text", word, 0.011, white, (0.0155, y, BAR_Z), (1, 0, 0), up=(0, 0, 1),
                                     cylinder=bar_axis))
    name = v["mesh"]
    obj, high = ka.high_low(name, parts, detail, [], soft=True)
    # The doc's collision: capsules for the mast, the three legs and the crossbar (prisms here),
    # and a box per head.
    ka.collision_segment(name, 0, (0, 0, 0.3), (0, 0, MAST), 0.02)
    for i, foot in enumerate(feet):
        ka.collision_segment(name, 1 + i, (0, 0, HUB), (foot[0], foot[1], 0.0), 0.015)
    ka.collision_segment(name, 4, (0, -v["bar"] / 2, BAR_Z), (0, v["bar"] / 2, BAR_Z), 0.015)
    for i, y in enumerate(v["heads"]):
        frame = head_frame(y)
        corners = [frame @ Vector((sx * hx, sy * hy, sz * hz)) for sx in (-1, 1) for sy in (-1, 1) for sz in (-1, 1)]
        ka.collision_hull(name, 5 + i, corners)
    # On the surfaces: the crossbar's top, the mast's front, the rear leg's outer side at mid
    # length, and the outer arm of the last head's yoke.
    hook_y = v["heads"][-1]
    out = Vector((feet[1][0], feet[1][1], 0.0)).normalized()
    mid = Vector((feet[1][0] * 0.5, feet[1][1] * 0.5, (HUB + feet[1][2]) * 0.5))
    arm_x = head_frame(hook_y).translation.x - 0.03
    for s, (loc, normal) in {"SKT_Dog_Top_01": ((0, 0, BAR_Z + 0.014), None),
                             "SKT_Dog_Frame_01": ((TUBE, 0, 1.2), (1, 0, 0)),
                             "SKT_Dog_Frame_02": (tuple(mid + out * TUBE * 0.9), tuple(out)),
                             "SKT_Dog_Hook_01": ((arm_x / 2, hook_y + 0.167, (BAR_Z + MAST - 0.09) / 2), (0, 1, 0))
                             }.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.004, "size": (0.9, 0.9, MAST),
            "res": 2048, "budget": 2000}
