"""Shipping container (20 ft): docs/objects/heavy/shipping-container.md.

ISO 606 x 244 x 259 cm, pivot at the bottom centre, X+ toward the door end. Corrugated sides and
blank end, a beaded roof, corner posts, rails and eight corner castings; on the door end two
leaves with four lock bars, handles, cam keepers, hinges and gaskets; stencils on both long sides
and the doors. The doors are separate meshes hinged at their outer edge (the blown state swings
them). Dog sockets and the collision box are named as the doc gives them.

Variants (the doc's table): htr (the Yard, Anchor Rim), meridian (Ballast), guild (the Yard).
"""
from __future__ import annotations

import math

import bpy
from mathutils import Matrix, Vector

import kinetic_art as ka

L, W, H = 6.058, 2.438, 2.591
POST = 0.16           # Corner post section
CAST = (0.178, 0.162, 0.118)  # Corner casting (x, y, z)
RAIL_BOTTOM = 0.16
RAIL_TOP = 0.10
SILL, HEADER = 0.20, 0.18     # Door end bottom and top rails
LEAF = (W - 2 * 0.012) / 2    # Each door leaf, minus the gasket gap

VARIANTS = {
    "htr": dict(mesh="SM_Heavy_Container20_HTR", paint="#E8620C", code="HTR-RC-0417", owner="HTR",
                wear=ka.Wear(paint="#E8620C", chips=0.3, rust_streaks=0.45, roof_dirt="#1B1A18", extra={"ends_x": L / 2})),
    "meridian": dict(mesh="SM_Heavy_Container20_Meridian", paint="#2A9FB5", code="MC-7-2291", owner="MERIDIAN",
                     wear=ka.Wear(paint="#2A9FB5", chips=0.8, rust_streaks=0.9, lower_grime="#3A4A3C",
                                  roof_dirt="#C9CBC4", extra={"ends_x": L / 2})),
    "guild": dict(mesh="SM_Heavy_Container20_Guild", paint="#5B6670", code="LG-3306", owner="LAY GUILD",
                  wear=ka.Wear(paint="#5B6670", chips=0.45, rust_streaks=0.5, roof_dirt="#1B1A18",
                                  extra={"ends_x": L / 2})),
}


def text_mesh(name: str, body: str, size: float, material, center, rotation_z: float) -> bpy.types.Object:
    """Stencil lettering as a flat mesh decal centred on `center` (a point 2 mm proud of the face
    it sits on), turned to face the way rotation_z gives."""
    curve = bpy.data.curves.new(name, "FONT")
    curve.body = body
    curve.size = size
    curve.align_x = "LEFT"
    curve.resolution_u = 2
    obj = bpy.data.objects.new(name, curve)
    bpy.context.scene.collection.objects.link(obj)
    obj.rotation_euler = (math.pi / 2, 0.0, rotation_z)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.ops.object.convert(target="MESH")
    mesh_obj = bpy.context.view_layer.objects.active
    mesh_obj.data.materials.clear()
    mesh_obj.data.materials.append(material)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    corners = [v.co for v in mesh_obj.data.vertices]
    mid = Vector([(min(c[i] for c in corners) + max(c[i] for c in corners)) / 2 for i in range(3)])
    mesh_obj.data.transform(Matrix.Translation(Vector(center) - mid))
    return mesh_obj


def build_body(v: dict, paint, steel, rubber, stencil) -> list[bpy.types.Object]:
    parts: list[bpy.types.Object] = []
    add = parts.append
    hx, hy = L / 2, W / 2
    # Corner posts and castings.
    for sx in (-1, 1):
        for sy in (-1, 1):
            post = ka.box("post", (POST, POST, H - 2 * CAST[2]),
                          (sx * (hx - POST / 2), sy * (hy - POST / 2), H / 2), paint)
            ka.add_bevel(post)
            add(post)
            for z in (CAST[2] / 2, H - CAST[2] / 2):
                c = ka.box("casting", CAST, (sx * (hx - CAST[0] / 2), sy * (hy - CAST[1] / 2), z), steel)
                ka.add_bevel(c, 0.008)
                add(c)
    # Side rails along X.
    inner_x = L - 2 * CAST[0]
    for sy in (-1, 1):
        add(ka.box("rail_bottom", (inner_x, 0.10, RAIL_BOTTOM), (0, sy * (hy - 0.05), RAIL_BOTTOM / 2), paint))
        add(ka.box("rail_top", (inner_x, 0.08, RAIL_TOP), (0, sy * (hy - 0.04), H - RAIL_TOP / 2), paint))
    # End rails along Y: the blank end, and the door end's sill and header.
    inner_y = W - 2 * CAST[1]
    add(ka.box("end_bottom", (0.10, inner_y, RAIL_BOTTOM), (-(hx - 0.05), 0, RAIL_BOTTOM / 2), paint))
    add(ka.box("end_top", (0.08, inner_y, RAIL_TOP), (-(hx - 0.04), 0, H - RAIL_TOP / 2), paint))
    add(ka.box("door_sill", (0.12, inner_y, SILL), (hx - 0.06, 0, SILL / 2), paint))
    add(ka.box("door_header", (0.12, inner_y, HEADER), (hx - 0.06, 0, H - HEADER / 2), paint))
    for p in parts[-6:]:
        ka.add_bevel(p)

    # Corrugated long sides (pitch 0.2 m: the doc's alternating 10 cm bands) and blank end.
    side_len = L - 2 * POST
    side_h = H - RAIL_BOTTOM - RAIL_TOP
    for sy, rot in ((1, 0.0), (-1, math.pi)):
        panel = ka.corrugated_panel("side", side_len, side_h, 0.2, 0.036, paint)
        ka.transform(panel, (0, sy * (hy - 0.004), RAIL_BOTTOM), rot)
        add(panel)
    end = ka.corrugated_panel("end", W - 2 * POST, side_h, 0.2, 0.036, paint)
    ka.transform(end, (-(hx - 0.004), 0, RAIL_BOTTOM), math.pi / 2)
    add(end)
    # Roof with shallow beads; a floor plate closes the underside.
    add(ka.box("roof", (L - 2 * 0.06, W - 2 * 0.06, 0.03), (0, 0, H - 0.015), paint))
    for i in range(14):
        x = -L / 2 + 0.35 + i * (L - 0.7) / 13
        add(ka.box("roof_bead", (0.10, W - 0.3, 0.012), (x, 0, H + 0.004), paint))
    add(ka.box("floor", (L - 0.2, W - 0.2, 0.03), (0, 0, 0.11), steel))
    # Gasket frame around the door opening.
    add(ka.box("gasket_top", (0.02, inner_y, 0.03), (hx - 0.005, 0, H - HEADER - 0.015), rubber))
    add(ka.box("gasket_bottom", (0.02, inner_y, 0.03), (hx - 0.005, 0, SILL + 0.015), rubber))

    # Stencils: owner mark top left, the 40 cm asset code bottom right, SITE PROPERTY under it.
    for sy, rot in ((1, math.pi), (-1, 0.0)):
        face_y = sy * (hy + 0.002)
        right = 1 if sy < 0 else -1  # The +X or -X way, as seen from outside that face
        add(text_mesh("owner", v["owner"], 0.34, stencil, (-right * (hx - 1.0), face_y, H - 0.5), rot))
        add(text_mesh("code", v["code"], 0.4, stencil, (right * 1.6, face_y, 0.85), rot))
        add(text_mesh("site", "SITE PROPERTY", 0.16, stencil, (right * 1.6, face_y, 0.5), rot))
    return parts


def build_leaf(side: int, paint, steel, rubber, stencil, code: str) -> bpy.types.Object:
    """One door leaf, side +1 (left, +Y) or -1 (right, -Y), hinged at its outer edge."""
    hx = L / 2
    height = H - SILL - HEADER
    cy = side * (0.006 + LEAF / 2)
    parts: list[bpy.types.Object] = []
    leaf = ka.box("leaf", (0.04, LEAF, height), (hx - 0.01, cy, SILL + height / 2), paint)
    ka.add_bevel(leaf)
    parts.append(leaf)
    for i in range(5):
        z = SILL + 0.25 + i * (height - 0.5) / 4
        parts.append(ka.box("rib", (0.018, LEAF - 0.08, 0.07), (hx + 0.018, cy, z), paint))
    for y_off in (0.30, 0.90):
        y = side * y_off
        parts.append(ka.cylinder("lockbar", 0.022, height + 0.1, (hx + 0.05, y, SILL + height / 2), "Z", 12, steel))
        for z in (SILL + 0.3, SILL + height / 2, SILL + height - 0.3):
            parts.append(ka.box("guide", (0.05, 0.07, 0.05), (hx + 0.035, y, z), steel))
        parts.append(ka.box("handle", (0.05, 0.06, 0.38), (hx + 0.09, y + side * 0.05, 1.1), steel))
        for z in (SILL + 0.03, SILL + height - 0.03):
            parts.append(ka.box("cam_keeper", (0.08, 0.10, 0.08), (hx + 0.04, y, z), steel))
    outer = side * (W / 2 - 0.17)
    for z in (0.45, 1.0, 1.55, 2.1):
        parts.append(ka.cylinder("hinge", 0.03, 0.14, (hx + 0.02, outer, z), "Z", 10, steel))
    parts.append(ka.box("gasket_edge", (0.02, 0.012, height), (hx - 0.005, side * 0.006, SILL + height / 2), rubber))
    if side < 0:
        parts.append(text_mesh("door_code", code, 0.085, stencil, (hx + 0.03, side * 0.6, 2.05), math.pi / 2))
    for p in parts:
        if p.name.startswith(("guide", "cam_keeper", "handle")):
            ka.add_bevel(p, 0.004)
    mesh_name = "Door_L" if side > 0 else "Door_R"
    obj = ka.join(mesh_name, parts)
    # Pivot on the hinge line, at the sill.
    pivot = Vector((hx + 0.02, outer, SILL))
    obj.data.transform(Matrix.Translation(-pivot))
    obj.location = pivot
    return obj


def build(variant: str = "htr") -> dict:
    v = VARIANTS[variant]
    paint = ka.painted_steel("MI_Steel_Painted_Worn", v["wear"])
    steel = ka.flat_material("MI_Steel_Bare", "#5C5F63", 0.55, 1.0)
    rubber = ka.flat_material("MI_Rubber_Black", "#1E1F20", 0.78)
    stencil = ka.flat_material("MI_Stencil_White", "#E8E6E0", 0.6)

    body = ka.join(v["mesh"], build_body(v, paint, steel, rubber, stencil))
    doors = [build_leaf(side, paint, steel, rubber, stencil, v["code"]) for side in (1, -1)]
    for d in doors:
        d.name = f"{v['mesh']}_{d.name}"
        d.data.name = d.name

    # Collision (the doc: a box for the body; a box per door leaf).
    ka.collision_box(v["mesh"], 0, (L, W, H), (0, 0, H / 2))
    height = H - SILL - HEADER
    for d in doors:
        side = 1 if d.name.endswith("_L") else -1
        ka.collision_box(d.name, 0, (0.05, LEAF, height), (L / 2 - 0.01, side * (0.006 + LEAF / 2), SILL + height / 2))

    hx, hy = L / 2, W / 2
    sockets = {
        "SKT_Dog_Top_01": (-1.5, 0, H + 0.01),
        "SKT_Dog_Top_02": (1.5, 0, H + 0.01),
        "SKT_Dog_SideN_01": (0, hy, 1.3),
        "SKT_Dog_SideS_01": (0, -hy, 1.3),
        "SKT_Dog_End_01": (-hx, 0, 1.3),
        "SKT_Dog_Door_01": (hx + 0.05, 0.6, 1.3),
        "SKT_Dog_Door_02": (hx + 0.05, -0.6, 1.3),
    }
    i = 1
    for sx in (-1, 1):
        for sy in (-1, 1):
            sockets[f"SKT_Dog_Hook_{i:02d}"] = (sx * (hx - CAST[0] / 2), sy * (hy - CAST[1] / 2), H)
            i += 1
    # The doc names the long sides N and S (X is the container's length), the blank end, the
    # doors and the top corner castings; each socket faces out of its face.
    normals = {"SKT_Dog_SideN_01": (0, 1, 0), "SKT_Dog_SideS_01": (0, -1, 0), "SKT_Dog_End_01": (-1, 0, 0),
               "SKT_Dog_Door_01": (1, 0, 0), "SKT_Dog_Door_02": (1, 0, 0)}
    for name, loc in sockets.items():
        ka.socket(name, loc, body, normals.get(name, (0, 0, 1) if "_Hook_" in name else None))
    # Its walls are single sheets, seen from inside once the doors are open: two-sided.
    return {"name": v["mesh"], "meshes": [body, *doors], "size": (L, W, H), "budget": 16000,
            "double_sided": [body.name, *(d.name for d in doors)]}
