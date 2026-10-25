"""Fuel drum: docs/objects/volatiles/fuel-drum.md.

A 200 L closed-head drum, 58 cm across and 88 cm tall, pivot at the bottom centre. One pressed
shell: the heads set into rolled chimes top and bottom, the body with two rolling hoops formed
out 1.5 cm and a swage rib near each end; a 2 inch bung and a 3/4 inch vent in their flanges on
the top head. Red paint worn to bare steel in a 10 cm band at each hoop and on the chimes, rust up
from the bottom chime, diesel run in a fan down from the bung, a couple of dents and the shell a
little out of round; the Office's SITE PROPERTY, DIESEL, a UN class 3 diamond and the supplier's
band.

The markings, the body's welded side seam and the plugs' slots and threads are the high mesh,
baked onto the shell, so the drum keeps to the doc's 5,000 tris.
"""
from __future__ import annotations

import math

from mathutils import Matrix, Vector

import kinetic_art as ka

R, H = 0.29, 0.88
BODY = 0.275  # The rolling hoops stand 1.5 cm proud, so the drum is 58 cm across them
HOOPS = (0.29, 0.59)
SWAGES = (0.19, 0.69)
TOP = 0.866   # The top head; the chime's rim stands 1.4 cm over it
SEGMENTS = 52


def shell_profile() -> list[tuple[float, float]]:
    """(radius, z) from the bottom head's centre, out through the bottom chime, up the body over
    its swages and hoops, through the top chime to the top head's centre."""
    chime = [(0.250, 0.012), (0.262, 0.008), (0.266, 0.002), (0.274, 0.0), (0.283, 0.004), (0.286, 0.012),
             (0.283, 0.021), (0.277, 0.027)]
    body = [(BODY, 0.035)]
    for z in sorted(HOOPS + SWAGES):
        if z in HOOPS:
            body += [(BODY, z - 0.03), (BODY + 0.006, z - 0.017), (R, z - 0.006), (R, z + 0.006),
                     (BODY + 0.006, z + 0.017), (BODY, z + 0.03)]
        else:
            body += [(BODY, z - 0.01), (BODY - 0.005, z), (BODY, z + 0.01)]
    body.append((BODY, H - 0.035))
    top = [(r, H - z) for r, z in reversed(chime)]
    top[-1] = (0.250, TOP)
    return [(0.0, 0.012)] + chime + body + top + [(0.0, TOP)]


def body_radius(z: float) -> float:
    """The shell's radius at height z (the profile's outside), for wrapping markings."""
    prof = [p for p in shell_profile() if 0.03 < p[1] < H - 0.03]
    for (r0, z0), (r1, z1) in zip(prof, prof[1:]):
        if z0 <= z <= z1 and z1 > z0:
            return r0 + (r1 - r0) * (z - z0) / (z1 - z0)
    return BODY


def build(variant: str = "yard") -> dict:
    wear = ka.Wear(paint="#8E2A1F", faded="#B04A3A", chips=0.55, scrape_height=0.07, rust_streaks=0.35,
                   roof_dirt="#2B2622", streak_band=(0.2, H), bottom_rust=0.12, detail=3.0, edge=0.012,
                   grime_reach=0.05, dust=0.6, scratches=0.6, dents=0.6,
                   bare_bands=tuple((z - 0.05, z + 0.05) for z in HOOPS) + ((-0.01, 0.01), (TOP + 0.003, H + 0.01)),
                   streak_from=(H - 0.035,) + tuple(z - 0.03 for z in HOOPS), streak_length=0.25,
                   drips=((BODY, 0.02, H - 0.03, 0.4),), extra={"oil_height": 0.3})
    paint = ka.painted_steel("MI_Drum_Fuel_Red", wear)
    steel = ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.65), rust=0.4, detail=8.0, roof_dirt="#2B2622")
    white = ka.stencil_paint("MI_Stencil_White", "#E6E4DD", wear)
    black = ka.stencil_paint("MI_Stencil_Black", "#141414", wear)
    band = ka.stencil_paint("MI_Paint_Black", "#1C1B19", wear)
    hazard = ka.stencil_paint("MI_Decal_Flammable", "#C8302A", wear)

    parts, detail = [], []
    shell = ka.lathe("shell", shell_profile(), SEGMENTS, paint)
    parts.append(shell)
    # The bung and the vent, each a plug in a raised flange; the vent's is smaller.
    for x, plug, flange in ((0.17, 0.027, 0.036), (-0.17, 0.014, 0.021)):
        at = Matrix.Translation((x, 0.0, 0.0))
        ring = ka.lathe("flange", [(plug, TOP + 0.004), (flange - 0.004, TOP + 0.006), (flange, TOP + 0.003),
                                   (flange + 0.004, TOP)], 14, steel, cap_bottom=False, cap_top=False)
        cap = ka.lathe("plug", [(plug, TOP), (plug, H - 0.002), (plug - 0.002, H), (0.0, H)], 14, steel,
                       cap_bottom=False)
        for o in (ring, cap):
            o.data.transform(at)
        parts += [ring, cap]
        # The plug's slots and its thread showing at the flange: baked.
        for k in range(2 if plug > 0.02 else 1):
            slot = ka.box("plug_slot", (plug * 1.4, 0.006, 0.003), (0.0, 0.0, 0.0), steel)
            ka.transform(slot, (x, 0.0, H + 0.0005), rotation_z=math.pi / 2 * k)
            detail.append(slot)
        for k in range(3):
            detail.append(ka.ring("thread", plug + 0.0008, 0.0009, (x, 0.0, TOP + 0.0012 + 0.0016 * k), "Z", 20, 4, steel))
    # The welded side seam, up the back of the body.
    a = math.radians(160)
    seam = [((body_radius(z) + 0.0006) * math.cos(a), (body_radius(z) + 0.0006) * math.sin(a), z)
            for z in [0.03 + k * 0.01 for k in range(83)]]
    detail.append(ka.tube("side_seam", seam, 0.0016, paint, sides=4))
    # Markings, bent round the body so a line keeps its length across the curve, then wrapped
    # onto the shell. The Office's on the X- face, DIESEL on Y-, the diamond beside it.
    drum = ((0, 0, 0), (0, 0, 1), BODY)
    detail.append(ka.stencil("office", "OFFICE OF THE LAY", 0.05, white, (-BODY, 0, 0.5), (-1, 0, 0), shell,
                             cylinder=drum, spray=0.002, grid=0.006, resolution=4))
    detail.append(ka.stencil("site", "SITE PROPERTY  M-2204", 0.04, white, (-BODY, 0, 0.44), (-1, 0, 0), shell,
                             cylinder=drum, spray=0.002, grid=0.006, resolution=4))
    detail.append(ka.stencil("diesel", "DIESEL", 0.09, black, (0, -BODY, 0.45), (0, -1, 0), shell, cylinder=drum,
                             spray=0.003, grid=0.008, resolution=5))
    # The UN class 3 diamond: a white border round the red, a 3 at its foot.
    a = math.radians(-30)
    out = Vector((math.cos(a), math.sin(a), 0.0))
    detail.append(ka.decal_plate("diamond", 0.125, 0.125, white, tuple(out * BODY + Vector((0, 0, 0.45))), tuple(out),
                                 shell, roll=math.pi / 4))
    inner = ka.decal_plate("diamond", 0.113, 0.113, hazard, tuple(out * (BODY + 0.0004) + Vector((0, 0, 0.45))),
                           tuple(out), shell, roll=math.pi / 4)
    inner.modifiers["Wrap"].offset = 0.0019
    detail.append(inner)
    detail.append(ka.stencil("class", "3", 0.022, white, tuple(out * BODY + Vector((0, 0, 0.405))), tuple(out), shell,
                             cylinder=drum, resolution=4))
    # The supplier's band (PROPOSED in the doc): white on black, between the bottom hoop and the
    # lower swage, on the X+ side.
    detail.append(ka.lathe("supplier_band", [(BODY + 0.001, 0.212), (BODY + 0.001, 0.252)], SEGMENTS * 2, band,
                           cap_bottom=False, cap_top=False))
    detail.append(ka.stencil("supplier", "BASIN PETROL CO-OP", 0.022, white, (BODY + 0.001, 0, 0.2325), (1, 0, 0),
                             cylinder=((0, 0, 0), (0, 0, 1), BODY + 0.001), resolution=4))

    name = "SM_Volatile_FuelDrum"
    obj, high = ka.high_low(name, parts, detail, [], soft=True)
    # A dent low on the Y+ side where it was dropped, one in the upper body, the top chime
    # knocked in, and the shell a little out of round: all inward, inside the collision.
    dented = ka.dents(((-0.1, 0.27, 0.16), (0.0, -0.012, 0.0), 0.09),
                      ((0.19, -0.2, 0.74), (-0.006, 0.005, 0.0), 0.07),
                      ((-0.2, -0.2, H), (0.006, 0.006, -0.004), 0.06))
    wobble = ka.waviness(0.0015, 2.0, 2204, (1.0, 1.0, 0.0))

    def offset(p: Vector) -> Vector:
        radial = Vector((p.x, p.y, 0.0))
        if radial.length < 0.2:
            return Vector((0.0, 0.0, 0.0))  # The heads' middles and the plugs stay put
        w = wobble(p)
        inward = -radial.normalized() * abs(w.dot(radial.normalized()))
        return dented(p) + inward
    ka.deform([obj, high], offset)
    ka.collision_cylinder(name, 0, R, 0.0, H)
    # On the surfaces: the lid, the bung's top, the body between the hoops, the top chime.
    for s, (loc, normal) in {
        "SKT_Dog_Top_01": ((0, 0, TOP), None),
        "SKT_Dog_Valve_01": ((0.17, 0, H), (0, 0, 1)),
        "SKT_Dog_SideN_01": ((BODY, 0, 0.44), (1, 0, 0)),
        "SKT_Dog_SideS_01": ((-BODY, 0, 0.44), (-1, 0, 0)),
        "SKT_Dog_Rim_01": ((0, BODY - 0.002 + 0.012, TOP), (0, 1, 0)),
        "SKT_Dog_Bot_01": ((0, 0, 0), None),
    }.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.006, "size": (2 * R, 2 * R, H),
            "res": 2048, "budget": 5000}

