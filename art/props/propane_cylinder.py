"""Propane cylinder: docs/objects/volatiles/propane-cylinder.md.

30 cm across, 120 cm tall with the valve guard, pivot at the bottom centre of the foot ring. Taller
and narrower than a real 47 L cylinder on purpose: it reads as "bomb" at 30 m. A foot ring, the
body with a domed shoulder and collar, a three-prong welded guard, a brass valve with a red
handwheel, the Office stencil and a hazard diamond. Bare steel where it is rolled, 5 cm above the
foot ring.
"""
from __future__ import annotations

import math

import kinetic_art as ka

R, H = 0.15, 1.20
TOP_SOCKET = (0.085, 0, 1.199)  # On the guard ring's top (the doc's guard centre is air)


def build_parts(paint, steel, brass, red, white, hazard, body_ref=None) -> list:
    parts = []
    # The foot ring as a closed section: base, outer wall, top and inner wall.
    parts.append(ka.lathe("foot", [(0.125, 0.0), (0.135, 0.0), (0.135, 0.09), (0.125, 0.09), (0.125, 0.0)], 32, steel,
                          cap_bottom=False, cap_top=False))
    profile = [(0.0, 0.07), (0.14, 0.07), (R, 0.10), (R, 0.86), (0.135, 0.93), (0.10, 0.985), (0.06, 1.01),
               (0.05, 1.02), (0.05, 1.04), (0.0, 1.04)]
    body = ka.lathe("body", profile, 36, paint, cap_bottom=False, cap_top=False)
    parts.append(body)
    # The welded guard: three prongs up from the collar to a top ring.
    for i in range(3):
        a = 2 * math.pi * i / 3
        x, y = math.cos(a), math.sin(a)
        parts.append(ka.tube("prong", [(0.05 * x, 0.05 * y, 1.02), (0.085 * x, 0.085 * y, 1.10),
                                       (0.085 * x, 0.085 * y, 1.185)], 0.008, steel))
    parts.append(ka.ring("guard_ring", 0.085, 0.009, (0, 0, 1.19), "Z", 28, 6, steel))
    parts.append(ka.lathe("valve", [(0.0, 1.04), (0.022, 1.04), (0.022, 1.10), (0.012, 1.12), (0.0, 1.12)], 16, brass,
                          cap_bottom=False, cap_top=False))
    parts.append(ka.cylinder("outlet", 0.009, 0.05, (0.035, 0, 1.075), "X", 10, brass))
    # The handwheel sits on the valve's top (1.12).
    parts.append(ka.ring("handwheel", 0.036, 0.006, (0, 0, 1.126), "Z", 20, 6, red))
    for i in range(3):
        a = 2 * math.pi * i / 3
        parts.append(ka.tube("spoke", [(0, 0, 1.126), (0.036 * math.cos(a), 0.036 * math.sin(a), 1.126)], 0.004, red))
    target = body_ref or body
    # The Office marking on the shoulder (it tilts back 12 degrees there), the supplier band below.
    # Bent round the axis first, then wrapped onto the cone, so a line keeps its length.
    shoulder = (0, -0.98, 0.21)
    parts.append(ka.stencil("office", "OFFICE OF THE LAY", 0.03, white, (0, -0.143, 0.905), shoulder, target,
                            cylinder=((0, 0, 0), (0, 0, 1), 0.143)))
    parts.append(ka.stencil("site", "SITE PROPERTY  M-0417", 0.02, white, (0, -0.148, 0.872), shoulder, target,
                            cylinder=((0, 0, 0), (0, 0, 1), 0.148)))
    parts.append(ka.stencil("supplier", "RUCU GAS", 0.05, red, (0, -R, 0.62), (0, -1, 0), target,
                            cylinder=((0, 0, 0), (0, 0, 1), R)))
    parts.append(ka.decal_plate("diamond", 0.07, 0.07, hazard, (0.0, -R, 0.48), (0, -1, 0), target,
                                roll=math.pi / 4))
    return parts


def materials():
    # The doc's wear: scraped bare 5 cm above the foot ring where it is rolled, rust from the
    # foot ring's weld, dents on the shoulder; the diamond at 0.7 wear.
    wear = ka.Wear(paint="#B8B4A8", paint_rough=(0.4, 0.6), chips=0.4, scrape_height=0.0, rust_streaks=0.3,
                   streak_band=(0.2, H), bottom_rust=0.12, detail=4.0, edge=0.01, grime_reach=0.04, dents=0.5,
                   bare_bands=((0.1, 0.15),), streak_from=(0.93,), streak_length=0.2, roof_dirt="#5A544C")
    paint = ka.painted_steel("MI_Cylinder_Propane_Grey", wear)
    return dict(paint=paint, steel=ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.65), rust=0.45, detail=8.0),
                brass=ka.bare_steel("MI_Brass", "#A8894A", rough=(0.3, 0.45), rust=0.0, detail=12.0,
                                    bare_steel="#BA9853"),
                red=ka.painted_steel("MI_Handwheel_Red", ka.Wear(paint="#B5302A", chips=0.5, scrape_height=0.0,
                                                                 rust_streaks=0.1, streaks=0.0, detail=12.0,
                                                                 edge=0.003, dust=0.2, dirt_streaks=0.0)),
                white=ka.stencil_paint("MI_Stencil_White", "#E6E4DD", wear),
                hazard=ka.stencil_paint("MI_Decal_Hazard_Yellow", "#E0B52A", ka.Wear(**{**wear.__dict__, "chips": 0.7})))


def build(variant: str = "yard") -> dict:
    m = materials()
    name = "SM_Volatile_PropaneCylinder"
    obj = ka.join(name, build_parts(**m), soft=True)
    # The doc's capsule body as a cylinder to the shoulder and a cone over the dome, and the
    # guard's cylinder, as convex hulls.
    ka.collision_cylinder(name, 0, R, 0.0, 0.86)
    ka.collision_cylinder(name, 1, R, 0.86, 1.04, 16, top_radius=0.05)
    ka.collision_cylinder(name, 2, 0.095, 1.04, H, 12)
    # The doc's Top (guard centre) and Bot (foot-ring centre) are air: they sit on the guard
    # ring's top and the foot ring's underside instead.
    for s, (loc, normal) in {
        "SKT_Dog_Top_01": (TOP_SOCKET, None),
        "SKT_Dog_Valve_01": ((0.022, 0, 1.08), (1, 0, 0)),
        "SKT_Dog_SideN_01": ((R, 0, 0.6), (1, 0, 0)),
        "SKT_Dog_SideS_01": ((-R, 0, 0.6), (-1, 0, 0)),
        "SKT_Dog_Bot_01": ((0.13, 0, 0), None),
    }.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "size": (2 * R, 2 * R, H), "res": 2048, "budget": 5000}
