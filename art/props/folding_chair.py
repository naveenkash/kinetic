"""Folding chair (open): docs/objects/props-light/folding-chair.md.

46 x 50 x 80 cm open, seat at 44 cm: an X of 19 mm painted steel tube each side, riveted where
the legs cross, a plywood seat and back with rounded corners, and moulded rubber foot caps. Pivot
at the bottom centre of the open footprint, X+ toward the sitter's knees.

Each side's X: the seat leg runs from the rear foot up to the seat's front; the back leg runs
from the front foot up under the seat's rear and on up into the back. The seat leg is inboard of
the back leg by a tube's width, so they pass, and the rivet joins them where they cross. The seat
pivots on the back legs at hinge knuckles under its rear edge, and the back is a curved panel.

Wear (the doc's): the back's top rail polished bare where hands fold it, chips at the feet and
along the seat edge, rust only in the hinge knuckles, a mug ring on the seat; the Guild's Ledger
number under the seat, baked from the high mesh.
"""
from __future__ import annotations

import math

import bmesh
from mathutils import Matrix, Vector

import kinetic_art as ka

W, D, H, SEAT = 0.46, 0.50, 0.80, 0.44
TUBE = 0.0095
FOOT_Z = 0.012  # Tube ends stop inside the foot caps, so nothing reaches below the floor
REAR_FOOT, FRONT_FOOT = -0.20, 0.22
SEAT_FRONT, SEAT_REAR, BACK_TOP = 0.18, -0.18, -0.20
SIDES = 10      # The tube's sides


def on_line(a, b, z: float) -> float:
    """X along the leg from a to b (each (x, z)) at height z."""
    t = (z - a[1]) / (b[1] - a[1])
    return a[0] + (b[0] - a[0]) * t


def slab(name: str, size, center, radius: float, material, axis: str = "Z") -> object:
    """A board with rounded corners: a rounded rectangle `size[0]` x `size[1]` across, `size[2]`
    thick along `axis` (Z for the seat, X for the back), its edges eased by a small bevel."""
    sx, sy, t = size
    bm = bmesh.new()
    ring = []
    for cx, cy, a0 in ((sx / 2 - radius, sy / 2 - radius, 0.0), (-sx / 2 + radius, sy / 2 - radius, 90.0),
                       (-sx / 2 + radius, -sy / 2 + radius, 180.0), (sx / 2 - radius, -sy / 2 + radius, 270.0)):
        for k in range(5):
            a = math.radians(a0 + 90.0 * k / 4)
            ring.append(bm.verts.new((cx + radius * math.cos(a), cy + radius * math.sin(a), -t / 2)))
    face = bm.faces.new(ring)
    ext = bmesh.ops.extrude_face_region(bm, geom=[face])
    bmesh.ops.translate(bm, verts=[v for v in ext["geom"] if isinstance(v, bmesh.types.BMVert)], vec=(0, 0, t))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    if axis == "X":
        bmesh.ops.transform(bm, verts=bm.verts, matrix=Matrix(((0, 0, 1), (0, 1, 0), (-1, 0, 0))).to_4x4())
    bmesh.ops.translate(bm, verts=bm.verts, vec=Vector(center))
    obj = ka.mesh_object(name, bm)
    obj.data.materials.append(material)
    ka.add_bevel(obj, 0.002, 2)
    obj.modifiers["Bevel"].angle_limit = math.radians(60)
    return obj


def foot_cap(x: float, y: float, rubber) -> object:
    """A moulded rubber cap over a tube's end: a domed foot, a collar up the tube."""
    cap = ka.lathe("foot_cap", [(0.0, 0.0), (0.0125, 0.0), (0.0145, 0.003), (0.0145, 0.012), (0.012, 0.02),
                                (0.0115, 0.028), (0.0105, 0.03), (0.0, 0.03)], 12, rubber)
    cap.data.transform(Matrix.Translation((x, y, 0.0)))
    return cap


def build(variant: str = "yard") -> dict:
    wear = ka.Wear(paint="#5B6670", chips=0.3, scrape_height=0.06, streak_band=(0.0, H), detail=8.0, edge=0.0025,
                   grime_reach=0.03, rust_streaks=0.15, streaks=0.0, dents=0.2, dust=0.4,
                   bare_bands=((H - 0.02, H + 0.02),), extra={"oil_height": 0.2})
    frame = ka.painted_steel("MI_Steel_Painted_Worn", wear)
    ply = ka.wood("MI_Plywood_Weathered", "#9C8564", weathered="#8F8B83", grain_axis="X", detail=3.0, ply_axis="Z",
                  checks=0.2, stains=(((0.03, 0.06, SEAT + TUBE + 0.014), 0.04),), rough=(0.8, 0.92))
    back_ply = ka.wood("MI_Plywood_Weathered", "#9C8564", weathered="#8F8B83", grain_axis="Y", detail=3.0,
                       ply_axis="X", checks=0.2, rough=(0.8, 0.92))
    rubber = ka.rubber("MI_Rubber_Black", "#1E1F20", 0.8)
    steel = ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.6), rust=0.3, detail=10.0)
    rust = ka.rusted("MI_Steel_Rusted", detail=10.0)
    parts, detail = [], []
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
        parts.append(ka.tube("seat_leg", [(REAR_FOOT, sy * inner, FOOT_Z), (SEAT_FRONT, sy * inner, SEAT)], TUBE,
                             frame, sides=SIDES))
        # The back leg bends at the seat's rear in a short arc, not a kink.
        bend = [(SEAT_REAR + 0.012, SEAT - 0.03), (SEAT_REAR, SEAT), (SEAT_REAR - 0.004, SEAT + 0.03)]
        parts.append(ka.tube("back_leg", [(FRONT_FOOT, sy * hy, FOOT_Z)] + [(x, sy * hy, z) for x, z in bend]
                             + [(BACK_TOP, sy * hy, H)], TUBE, frame, sides=SIDES))
        parts.append(ka.tube("seat_rail", [(SEAT_REAR, sy * inner, SEAT), (SEAT_FRONT, sy * inner, SEAT)], TUBE, frame,
                             sides=SIDES))
        # The rivet's domed heads either side of the crossing, its shank through both tubes.
        parts.append(ka.cylinder("rivet", 0.004, 2 * TUBE + 0.012, (cross[0], sy * (hy - TUBE), cross[1]), "Y", 8, steel))
        for side_y, out in ((sy * (hy + TUBE - 0.001), sy), (sy * (inner - TUBE + 0.001), -sy)):
            head = ka.lathe("rivet_head", [(0.0, -0.002), (0.0075, -0.002), (0.0068, 0.0005), (0.004, 0.0022),
                                           (0.0, 0.0028)], 10, steel)
            head.data.transform(Matrix.Translation((cross[0], side_y, cross[1])) @ Matrix.Rotation(-math.pi / 2 * out, 4, "X"))
            parts.append(head)
        # The seat's hinge: two knuckles on the back leg under the seat's rear, rusted in.
        parts.append(ka.cylinder("hinge_knuckle", 0.006, 0.03, (SEAT_REAR + 0.015, sy * (inner + TUBE), SEAT + 0.002),
                                 "Y", 8, rust))
        for x, y in ((REAR_FOOT, inner), (FRONT_FOOT, hy)):
            parts.append(foot_cap(x, sy * y, rubber))
    # Cross braces on the leg lines: the front one low between the back legs (which start at the
    # front feet), the rear one between the seat legs, and the back's top rail.
    z_front, z_rear = 0.15, 0.10
    x_front = on_line(*back_leg, z_front)
    x_rear = on_line(*seat_leg, z_rear)
    parts.append(ka.tube("front_brace", [(x_front, -hy, z_front), (x_front, hy, z_front)], TUBE, frame, sides=SIDES))
    parts.append(ka.tube("rear_brace", [(x_rear, -inner, z_rear), (x_rear, inner, z_rear)], TUBE, frame, sides=SIDES))
    parts.append(ka.tube("back_top", [(BACK_TOP, -hy, H), (BACK_TOP, hy, H)], TUBE, frame, sides=SIDES))
    seat = slab("seat", (SEAT_FRONT - SEAT_REAR + 0.02, 2 * inner + 0.01, 0.014), (0.0, 0, SEAT + TUBE + 0.007), 0.03, ply)
    parts.append(seat)
    back_x = (SEAT_REAR + BACK_TOP) / 2 + 0.005
    back = slab("back", (0.16, 2 * hy - 0.02, 0.014), (back_x, 0, H - 0.1), 0.025, back_ply, axis="X")
    parts.append(back)
    # The Guild's Ledger number under the seat, stencilled on the plywood.
    ink = ka.ink("MI_Stencil_Ochre", "#C8912A", "#9C8564", wear=0.5)
    detail.append(ka.stencil("ledger", "LG-0412", 0.03, ink, (0.0, 0, SEAT + TUBE - 0.0008), (0, 0, -1), up=(1, 0, 0),
                             spray=0.0015, resolution=3))
    name = "SM_Light_FoldingChair"
    obj, high = ka.high_low(name, parts, detail, [], soft=True)
    # The back is curved: bowed 1.2 cm back at its middle, as a pressed panel is.
    def bow(p: Vector) -> Vector:
        if abs(p.x - back_x) < 0.012 and p.z > H - 0.19 and abs(p.y) < hy - 0.005:
            return Vector((-0.012 * math.cos(math.pi * p.y / (2 * hy)), 0.0, 0.0))
        return Vector((0.0, 0.0, 0.0))
    ka.deform([obj, high], bow)
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
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.003, "size": (D, W, H), "res": 2048,
            "budget": 2000}
