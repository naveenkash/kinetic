"""Folding chair (open): docs/objects/props-light/folding-chair.md.

46 x 50 x 80 cm open, seat at 44 cm: an X of 19 mm painted steel tube each side, riveted where
the legs cross, a plywood seat and back, and rubber foot caps. Pivot at the bottom centre of the
open footprint, X+ toward the sitter's knees.

Each side's X: the seat leg runs from the rear foot up to the seat's front; the back leg runs
from the front foot up under the seat's rear and on up into the back. The seat leg is inboard of
the back leg by a tube's width, so they pass, and the rivet joins them where they cross.
"""
from __future__ import annotations

import kinetic_art as ka

W, D, H, SEAT = 0.46, 0.50, 0.80, 0.44
TUBE = 0.0095
FOOT_Z = 0.012  # Tube ends stop inside the foot caps, so nothing reaches below the floor
REAR_FOOT, FRONT_FOOT = -0.20, 0.22
SEAT_FRONT, SEAT_REAR, BACK_TOP = 0.18, -0.18, -0.20


def on_line(a, b, z: float) -> float:
    """X along the leg from a to b (each (x, z)) at height z."""
    t = (z - a[1]) / (b[1] - a[1])
    return a[0] + (b[0] - a[0]) * t


def build(variant: str = "yard") -> dict:
    frame = ka.painted_steel("MI_Steel_Painted_Worn", ka.Wear(paint="#5B6670", chips=0.3, scrape_height=0.06,
                                                              streak_band=(0.0, H)))
    ply = ka.wood("MI_Plywood_Weathered", "#9C8564", weathered="#8F8B83", grain_axis="Y")
    rubber = ka.flat_material("MI_Rubber_Black", "#1E1F20", 0.8)
    steel = ka.flat_material("MI_Steel_Bare", "#5C5F63", 0.55, 1.0)
    parts = []
    hy = W / 2 - 0.02          # The back legs' plane
    inner = hy - 2 * TUBE      # The seat legs' plane
    seat_leg = ((REAR_FOOT, FOOT_Z), (SEAT_FRONT, SEAT))
    back_leg = ((FRONT_FOOT, FOOT_Z), (SEAT_REAR, SEAT))
    # Where they cross: solve the two lines in x and z.
    (ax, az), (bx, bz) = seat_leg
    (cx, cz), (dx, dz) = back_leg
    t = ((cx - ax) * (dz - cz) - (cz - az) * (dx - cx)) / ((bx - ax) * (dz - cz) - (bz - az) * (dx - cx))
    cross = (ax + (bx - ax) * t, az + (bz - az) * t)
    for sy in (-1, 1):
        parts.append(ka.tube("seat_leg", [(REAR_FOOT, sy * inner, FOOT_Z), (SEAT_FRONT, sy * inner, SEAT)], TUBE, frame))
        parts.append(ka.tube("back_leg", [(FRONT_FOOT, sy * hy, FOOT_Z), (SEAT_REAR, sy * hy, SEAT),
                                          (BACK_TOP, sy * hy, H)], TUBE, frame))
        parts.append(ka.tube("seat_rail", [(SEAT_REAR, sy * inner, SEAT), (SEAT_FRONT, sy * inner, SEAT)], TUBE, frame))
        parts.append(ka.cylinder("rivet", 0.007, 2 * TUBE + 0.016, (cross[0], sy * (hy - TUBE), cross[1]), "Y", 8, steel))
        for x, y in ((REAR_FOOT, inner), (FRONT_FOOT, hy)):
            parts.append(ka.cylinder("foot_cap", 0.013, 0.024, (x, sy * y, 0.012), "Z", 8, rubber))
    # Cross braces on the leg lines: the front one low between the back legs (which start at the
    # front feet), the rear one between the seat legs, and the back's top rail.
    z_front, z_rear = 0.15, 0.10
    x_front = on_line(*back_leg, z_front)
    x_rear = on_line(*seat_leg, z_rear)
    parts.append(ka.tube("front_brace", [(x_front, -hy, z_front), (x_front, hy, z_front)], TUBE, frame))
    parts.append(ka.tube("rear_brace", [(x_rear, -inner, z_rear), (x_rear, inner, z_rear)], TUBE, frame))
    parts.append(ka.tube("back_top", [(BACK_TOP, -hy, H), (BACK_TOP, hy, H)], TUBE, frame))
    seat = ka.box("seat", (SEAT_FRONT - SEAT_REAR + 0.02, 2 * inner + 0.01, 0.014), (0.0, 0, SEAT + TUBE + 0.007), ply)
    ka.add_bevel(seat, 0.004)
    parts.append(seat)
    back_x = (SEAT_REAR + BACK_TOP) / 2 + 0.005
    back = ka.box("back", (0.014, 2 * hy - 0.02, 0.16), (back_x, 0, H - 0.1), ply)
    ka.add_bevel(back, 0.004)
    parts.append(back)
    # The Guild's Ledger number under the seat.
    ink = ka.flat_material("MI_Stencil_Ochre", "#C8912A", 0.6)
    parts.append(ka.stencil("ledger", "LG-0412", 0.03, ink, (0.0, 0, SEAT + TUBE - 0.0008), (0, 0, -1), up=(1, 0, 0)))
    name = "SM_Light_FoldingChair"
    obj = ka.join(name, parts)
    # The doc's collision: the seat and back as boxes, a capsule (here a prism) per leg.
    ka.collision_box(name, 0, (0.40, W, 0.04), (0.0, 0, SEAT + 0.01))
    ka.collision_box(name, 1, (0.04, W, 0.36), (back_x, 0, H - 0.18))
    i = 2
    for sy in (-1, 1):
        ka.collision_segment(name, i, (REAR_FOOT, sy * inner, 0.0), (SEAT_FRONT, sy * inner, SEAT), 0.015)
        ka.collision_segment(name, i + 1, (FRONT_FOOT, sy * hy, 0.0), (SEAT_REAR, sy * hy, SEAT), 0.015)
        i += 2
    # On the surfaces: the top of the back's rail, the seat, the front of the front brace.
    for s, (loc, normal) in {"SKT_Dog_Top_01": ((BACK_TOP, 0, H + TUBE), None),
                             "SKT_Dog_Top_02": ((0.0, 0, SEAT + TUBE + 0.014), None),
                             "SKT_Dog_Frame_01": ((x_front + TUBE, 0, z_front), (1, 0, 0))}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "size": (D, W, H), "res": 1024, "budget": 2000}
