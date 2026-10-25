"""Paint can: docs/objects/props-light/paint-can.md.

A 4 L tinplate can, 18 cm across and 20 cm tall, with rim beads, a paper label around 60% of it, a
lid with a ring of dried paint, a galvanized wire bail, a run of paint down one side and a ring
of rust at the base where it stood in water. Pivot at the bottom centre. Variants are the doc's
Full_ cans, by who bought them: office (default: rated-tape yellow, OFFICE OF THE LAY / SITE
MARKING / NOT FOR SALE), guild (ochre, LAY GUILD STORES and a stores number), htr (orange, HTR-RC
and a batch code, no wear) and hanger (teal, a chalk price on the lid).

The label's lettering and the rust ring are the high mesh, baked onto the label and the can, so
the can keeps to the doc's 800 tris.
"""
from __future__ import annotations

import math

import bmesh

import kinetic_art as ka

R, H = 0.09, 0.20

VARIANTS = {
    "office": dict(mesh="SM_Light_PaintCan_Office", paint="#F2C230", label="#F2C230", ink="#1C1B19",
                   lines=("OFFICE OF THE LAY", "SITE MARKING", "NOT FOR SALE")),
    "guild": dict(mesh="SM_Light_PaintCan_Guild", paint="#C8912A", label="#C8912A", ink="#1C1B19",
                  lines=("LAY GUILD STORES", "STORES NO. 2217")),
    "htr": dict(mesh="SM_Light_PaintCan_HTR", paint="#E8620C", label="#E8620C", ink="#1C1B19",
                lines=("HTR-RC", "BATCH 26-0917")),
    "hanger": dict(mesh="SM_Light_PaintCan_Hanger", paint="#2E8B8B", label="#D8CFB8", ink="#2E8B8B",
                   lines=("HANGER TEAL",), chalk="4 SH"),
}


def label_band(name: str, material, radius: float, z0: float, z1: float, arc: float, segments: int = 24):
    """A paper label wrapping `arc` (0 to 1) of the can, centred on Y-."""
    bm = bmesh.new()
    n = max(2, int(segments * arc))
    bottom, top = [], []
    for i in range(n + 1):
        a = -math.pi * arc + 2 * math.pi * arc * i / n - math.pi / 2
        bottom.append(bm.verts.new((radius * math.cos(a), radius * math.sin(a), z0)))
        top.append(bm.verts.new((radius * math.cos(a), radius * math.sin(a), z1)))
    for i in range(n):
        bm.faces.new((bottom[i], bottom[i + 1], top[i + 1], top[i]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    obj = ka.mesh_object(name, bm)
    obj.data.materials.append(material)
    for p in obj.data.polygons:
        p.use_smooth = True
    return obj


def build(variant: str = "office") -> dict:
    v = VARIANTS[variant]
    tin = ka.flat_material("MI_Steel_Bare", "#5C5F63", 0.45, 1.0)
    label = ka.fabric("MI_Label_Paper", v["label"], rough=0.75)
    paint = ka.flat_material("MI_Paint_Dried", v["paint"], 0.35)
    galv = ka.galvanized()
    ink = ka.flat_material("MI_Stencil", v["ink"], 0.6)
    rust = ka.flat_material("MI_Steel_Rusted", "#7A3F1C", 0.85, 0.3)
    parts, detail = [], []
    # Chime, a rolled bead near each end, and the lid's rim.
    profile = [(0.0, 0.0), (R - 0.004, 0.0), (R, 0.006), (R + 0.002, 0.016), (R, 0.024), (R, 0.178),
               (R + 0.002, 0.186), (R, H - 0.004), (R - 0.004, H), (0.0, H)]
    can = ka.lathe("can", profile, 24, tin, cap_bottom=False, cap_top=False)
    parts.append(can)
    parts.append(label_band("label", label, R + 0.0012, 0.035, 0.165, 0.6))
    for k, line in enumerate(v["lines"]):
        z = 0.125 - 0.024 * k + 0.012 * (3 - len(v["lines"]))
        detail.append(ka.stencil(f"label_line_{k}", line, 0.016 if k == 0 else 0.012, ink, (0, -R - 0.0025, z),
                                 (0, -1, 0), can))
    parts.append(ka.ring("lid_paint", R - 0.012, 0.004, (0, 0, H + 0.001), "Z", 14, 3, paint))
    if "chalk" in v:
        chalk = ka.flat_material("MI_Chalk", "#D8D4C8", 0.95)
        detail.append(ka.stencil("lid_price", v["chalk"], 0.03, chalk, (0, 0, H + 0.0015), (0, 0, 1), up=(1, 0, 0)))
    # Rust where it stood in water: a band round the chime, baked onto the can.
    detail.append(ka.lathe("rust_ring", [(R + 0.0008, 0.0), (R + 0.0028, 0.012)], 24, rust, cap_bottom=False,
                           cap_top=False))
    # The run of paint from the rim, on one side.
    for a, length in ((0.4, 0.05), (1.3, 0.09), (2.9, 0.07)):
        x, y = (R + 0.0018) * math.cos(a), (R + 0.0018) * math.sin(a)
        drip = ka.box("drip", (0.006, 0.003, length), (0, 0, 0), paint)
        ka.transform(drip, (x, y, H - 0.01 - length / 2), rotation_z=a + math.pi / 2)
        parts.append(drip)
    # Bail: ears on the sides and a wire arc over the lid.
    for sy in (-1, 1):
        parts.append(ka.box("ear", (0.02, 0.006, 0.02), (0, sy * (R + 0.003), 0.17), tin))
    pts = [(0.0, -R - 0.006, 0.17)]
    for k in range(1, 10):
        a = math.pi * k / 10
        pts.append((0.0, -(R + 0.006) * math.cos(a), 0.17 + 0.12 * math.sin(a)))
    pts.append((0.0, R + 0.006, 0.17))
    parts.append(ka.tube("bail", pts, 0.0022, galv, sides=4))
    name = v["mesh"]
    obj, high = ka.high_low(name, parts, detail, [])
    ka.collision_cylinder(name, 0, R, 0.0, H, 12)
    for s, (loc, normal) in {"SKT_Dog_Top_01": ((0, 0, H), None), "SKT_Dog_Handle_01": ((0, 0, 0.29), (0, 0, 1)),
                             "SKT_Dog_Bot_01": ((0, 0, 0), None)}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.004, "size": (2 * R, 2 * R, 0.3),
            "res": 1024, "budget": 800}
