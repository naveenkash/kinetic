"""Fuel drum: docs/objects/volatiles/fuel-drum.md.

A 200 L closed-head drum, 58 cm across and 88 cm tall, pivot at the bottom centre: a ribbed body,
two rolling hoops, top and bottom chimes, a 2 inch bung and a 3/4 inch vent. Red paint worn to
bare steel in a 10 cm band at each hoop, the hoops and chimes bare, rust up from the bottom
chime; the Office's SITE PROPERTY, DIESEL, a flammable diamond and the supplier's band.
"""
from __future__ import annotations

import kinetic_art as ka

R, H = 0.29, 0.88
BODY = 0.275  # The rolling hoops stand 1.5 cm proud, so the drum is 58 cm across them
HOOPS = (0.29, 0.59)


def build(variant: str = "yard") -> dict:
    paint = ka.painted_steel("MI_Drum_Fuel_Red", ka.Wear(paint="#8E2A1F", chips=0.55, scrape_height=0.07,
                                                         rust_streaks=0.3, roof_dirt="#2B2622",
                                                         streak_band=(0.2, H), bottom_rust=0.12,
                                                         bare_bands=tuple((z - 0.05, z + 0.05) for z in HOOPS)))
    steel = ka.flat_material("MI_Steel_Bare", "#5C5F63", 0.55, 1.0)
    white = ka.flat_material("MI_Stencil_White", "#E6E4DD", 0.6)
    black = ka.flat_material("MI_Stencil_Black", "#141414", 0.6)
    hazard = ka.flat_material("MI_Decal_Flammable", "#C8302A", 0.55)

    parts = []
    # The body with its swaged ribs, between the chimes.
    profile = [(0.0, 0.012), (BODY, 0.012), (BODY, 0.18), (BODY - 0.006, 0.19), (BODY, 0.2)]
    profile += [(BODY, 0.68), (BODY - 0.006, 0.69), (BODY, 0.70), (BODY, 0.866), (0.0, 0.866)]
    body = ka.lathe("body", profile, 40, paint, cap_bottom=False, cap_top=False)
    parts.append(body)
    # Chimes: the rolled rims top and bottom.
    for z in (0.012, 0.866):
        parts.append(ka.ring("chime", BODY - 0.002, 0.012, (0, 0, z), "Z", 40, 8, steel))
    for z in HOOPS:
        parts.append(ka.ring("hoop", R - 0.011, 0.011, (0, 0, z), "Z", 40, 8, steel))
    parts.append(ka.cylinder("bung", 0.035, 0.016, (0.17, 0.0, 0.872), "Z", 16, steel))
    parts.append(ka.cylinder("vent", 0.02, 0.014, (-0.17, 0.0, 0.871), "Z", 12, steel))
    # Markings on the X- face, wrapped onto the body.
    # Lettering bent round the body, so a line keeps its length across the curve.
    drum = ((0, 0, 0), (0, 0, 1), BODY)
    parts.append(ka.stencil("office", "OFFICE OF THE LAY", 0.05, white, (-BODY, 0, 0.5), (-1, 0, 0), cylinder=drum))
    parts.append(ka.stencil("site", "SITE PROPERTY  M-2204", 0.04, white, (-BODY, 0, 0.44), (-1, 0, 0),
                            cylinder=drum))
    parts.append(ka.stencil("diesel", "DIESEL", 0.09, black, (0, -BODY, 0.45), (0, -1, 0), cylinder=drum))
    # The supplier's band (PROPOSED in the doc): white on black, between the bottom hoop and the
    # body's ribs, on the X+ side.
    band = ka.flat_material("MI_Paint_Black", "#1C1B19", 0.6)
    parts.append(ka.lathe("supplier_band", [(BODY + 0.001, 0.215), (BODY + 0.001, 0.25)], 40, band,
                          cap_bottom=False, cap_top=False))
    parts.append(ka.stencil("supplier", "BASIN PETROL CO-OP", 0.022, white, (BODY + 0.001, 0, 0.2325), (1, 0, 0),
                            cylinder=((0, 0, 0), (0, 0, 1), BODY + 0.001)))
    parts.append(ka.decal_plate("diamond", 0.12, 0.12, hazard, (BODY * 0.70, -BODY * 0.71, 0.45), (0.70, -0.71, 0),
                                body, roll=0.785398))

    name = "SM_Volatile_FuelDrum"
    obj = ka.join(name, parts)
    ka.collision_cylinder(name, 0, R, 0.0, H)
    # On the surfaces: the lid, the bung's top, the body between the hoops, the top chime.
    for s, (loc, normal) in {
        "SKT_Dog_Top_01": ((0, 0, 0.866), None),
        "SKT_Dog_Valve_01": ((0.17, 0, 0.88), (0, 0, 1)),
        "SKT_Dog_SideN_01": ((BODY, 0, 0.44), (1, 0, 0)),
        "SKT_Dog_SideS_01": ((-BODY, 0, 0.44), (-1, 0, 0)),
        "SKT_Dog_Rim_01": ((0, BODY - 0.002 + 0.012, 0.866), (0, 1, 0)),
        "SKT_Dog_Bot_01": ((0, 0, 0), None),
    }.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "size": (2 * R, 2 * R, H), "res": 2048, "budget": 5000}
