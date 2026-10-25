"""Bucket: docs/objects/props-light/bucket.md.

A 15 L HDPE bucket, 30 cm across the top, 26 cm at the base, 36 cm tall, with a rolled lip, a
galvanized wire bail with a black grip. Pivot at the bottom centre. Variants are the doc's:
water (default, blue HDPE filled to 4 cm under the rim), empty, sand (the Yard's fire station),
quench (black water beside the welding cart) and hanger_mustard (a mustard body). The Yard's
carry the Guild's LG STORES stencil in ochre, baked onto the side from a high mesh.
"""
from __future__ import annotations

import math

import kinetic_art as ka

H = 0.36
BLUE = "#1F4FA3"
VARIANTS = {
    "water": dict(mesh="SM_Light_Bucket_Water", body=BLUE, fill=("MI_Water", "#3A5F6E", 0.05, 0.32)),
    "empty": dict(mesh="SM_Light_Bucket_Empty", body=BLUE, fill=None),
    "sand": dict(mesh="SM_Light_Bucket_Sand", body=BLUE, fill=("MI_Sand", "#B9A27A", 0.95, 0.30)),
    "quench": dict(mesh="SM_Light_Bucket_Quench", body=BLUE, fill=("MI_Water_Quench", "#141414", 0.02, 0.32)),
    "hanger_mustard": dict(mesh="SM_Light_Bucket_HangerMustard", body="#D9A441", fill=("MI_Water", "#3A5F6E", 0.05, 0.32)),
}


def wall(z: float) -> float:
    """The outside of the wall at height z: 0.13 at 6 mm to 0.15 at 33.5 cm."""
    return 0.13 + 0.02 * (z - 0.006) / (0.335 - 0.006)


def build(variant: str = "water") -> dict:
    v = VARIANTS[variant]
    hdpe = ka.plastic("MI_HDPE", v["body"])
    black = ka.plastic("MI_HDPE_Black", "#232426", rough=(0.5, 0.55))
    galv = ka.galvanized()
    parts = []
    profile = [(0.0, 0.0), (0.126, 0.0), (0.13, 0.006), (0.15, 0.335), (0.158, 0.348), (0.158, 0.36), (0.146, 0.36),
               (0.143, 0.35), (0.124, 0.012), (0.0, 0.012)]
    parts.append(ka.lathe("body", profile, 28, hdpe, cap_bottom=False, cap_top=False))
    # Stiffening rings under the lip.
    for z in (0.30, 0.32):
        parts.append(ka.ring("rib", wall(z) + 0.001, 0.003, (0, 0, z), "Z", 28, 3, hdpe))
    if v["fill"]:
        material, color, rough, level = v["fill"]
        # The wall runs from r 0.124 at 1.2 cm to 0.143 at 35 cm on the inside.
        radius = 0.124 + (0.143 - 0.124) * (level - 0.012) / (0.35 - 0.012)
        parts.append(ka.lathe("fill", [(0.0, level), (radius, level)], 28, ka.flat_material(material, color, rough),
                              cap_bottom=False, cap_top=False))
    # Bail ears, the wire bail and its grip.
    for sy in (-1, 1):
        parts.append(ka.box("ear", (0.03, 0.012, 0.03), (0, sy * (wall(0.33) + 0.006), 0.33), hdpe))
    pts = []
    for k in range(0, 13):
        a = math.pi * k / 12
        pts.append((0.0, -0.162 * math.cos(a), 0.33 + 0.16 * math.sin(a)))
    parts.append(ka.tube("bail", pts, 0.0025, galv, sides=4))
    parts.append(ka.cylinder("grip", 0.012, 0.1, (0, 0, 0.49), "Y", 12, black))
    detail = []
    if variant != "hanger_mustard":
        ochre = ka.flat_material("MI_Stencil_Ochre", "#C8912A", 0.6)
        z = 0.2
        detail.append(ka.stencil("stores", "LG STORES", 0.04, ochre, (wall(z), 0, z), (1, 0, 0),
                                 cylinder=((0, 0, 0), (0, 0, 1), wall(z))))
    name = v["mesh"]
    obj, high = ka.high_low(name, parts, detail, [])
    ka.collision_cylinder(name, 0, 0.13, 0.0, H, 16, top_radius=0.158)  # The doc's truncated cone, out to the lip
    for s, (loc, normal) in {"SKT_Dog_Handle_01": ((0, 0, 0.502), (0, 0, 1)), "SKT_Dog_Rim_01": ((0.158, 0, H), (1, 0, 0)),
                             "SKT_Dog_Bot_01": ((0, 0, 0), None)}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.004, "size": (0.32, 0.32, 0.5),
            "res": 1024, "budget": 1200}
