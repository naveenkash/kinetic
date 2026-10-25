"""Chain coil: docs/objects/props-light/chain-coil.md.

6 m of 10 mm grade-80 lifting chain coiled into a 40 cm ring, a clevis grab hook at each end.
Oiled dark links, rust in the bottom layer that never moves, hooks in HTR orange with a steel
rating tag (WLL 3.15 T, HTR-RC-2208). Pivot at the bottom centre.

The 200 links are the high mesh; the game mesh is a masked tube along the chain's path with the
links baked into its normal, colour and AO, and the gaps between them cut out of it (2,000 tris
in the doc, against 13,000 for the links).
"""
from __future__ import annotations

import math

import kinetic_art as ka

PITCH = 0.03   # Link pitch of 10 mm chain
LINK = 0.026   # A link's envelope across: 2 x (8.5 mm + 4.5 mm)
LENGTH = 6.0


def hook(name: str, at, heading: float, paint) -> list:
    """A clevis grab hook lying flat, its eye at `at`, opening along `heading`."""
    c, s = math.cos(heading), math.sin(heading)
    local = [(0.0, 0.0), (0.035, 0.0), (0.075, 0.012), (0.09, 0.04), (0.075, 0.065), (0.05, 0.06)]
    pts = [(at[0] + x * c - y * s, at[1] + x * s + y * c, at[2]) for x, y in local]
    # The eye is thinner than the shank: it lies on the same floor, so it sits 6 mm lower.
    return [ka.tube(name, pts, 0.011, paint),
            ka.ring(f"{name}_eye", 0.012, 0.005, (at[0], at[1], at[2] - 0.006), "Z", 8, 4, paint)]


def build(variant: str = "htr") -> dict:
    oiled = ka.flat_material("MI_Chain_Oiled", "#3C3E42", 0.38, 1.0)
    rusted = ka.flat_material("MI_Steel_Rusted", "#7A3F1C", 0.85, 0.3)
    hook_paint = ka.painted_steel("MI_Steel_Painted_Worn", ka.Wear(paint="#E8620C", chips=0.5, scrape_height=0.0,
                                                                   streak_band=(0.0, 0.05)))
    galv = ka.galvanized()
    ink = ka.flat_material("MI_Stencil_Black", "#141414", 0.6)
    links, path = [], []
    n = int(LENGTH / PITCH)
    # Laid down by hand in layers of two turns, spiralling in on one layer and out on the next,
    # each layer half a turn's spacing across from the one below so it sits in its grooves. A
    # link's envelope is 2.6 cm, so a layer rests 1.9 cm above the last; 6 m makes three and a
    # bit layers, about 9 cm high (the doc's 18 cm is a loose heap's). The bottom layer, which
    # never leaves the pile, is rusted. Flat links lie on their layer, the standing ones on edge.
    outer, inner, turns_per_layer = 0.185, 0.115, 2.0
    pitch = (outer - inner) / turns_per_layer
    rise = math.sqrt(LINK ** 2 - (pitch / 2) ** 2)
    angle = 0.0
    for i in range(n):
        turns = angle / (2 * math.pi)
        layer = int(turns / turns_per_layer)
        f = turns / turns_per_layer - layer      # 0 to 1 across this layer
        span = f * (outer - inner)
        offset = pitch / 2 * (layer % 2)
        radius = outer - span - offset if layer % 2 == 0 else inner + span + offset
        base = layer * rise
        if f > 0.9:                              # Climb onto the next layer over its last tenth
            base += rise * (f - 0.9) / 0.1
        angle += PITCH / radius
        x, y = radius * math.cos(angle), radius * math.sin(angle)
        lying = i % 2 == 0
        z = base + (0.0045 if lying else LINK / 2)
        mat = rusted if layer == 0 and i % 3 else oiled
        link = ka.ring("link", 0.0085, 0.0045, (0, 0, 0), "Z" if lying else "Y", 8, 4, mat, stretch=0.014)
        ka.transform(link, (x, y, z), rotation_z=angle + math.pi / 2)
        links.append(link)
        path.append((x, y, base, angle))
    first, last = path[0], path[-1]
    # The proxy: a tube through every other link, as fat as a link's envelope and resting where
    # the links rest.
    proxy = [ka.tube("chain_proxy", [(p[0], p[1], p[2] + LINK / 2) for p in path[::2]]
                     + [(last[0], last[1], last[2] + LINK / 2)], LINK / 2, oiled)]
    # Hook A off the first link, on the floor; hook B off the last, lying on the layer below.
    def ahead(p, d):
        return (p[0] - d * math.sin(p[3]), p[1] + d * math.cos(p[3]))
    ax, ay = ahead(first, -0.03)
    shared = hook("hook_a", (ax, ay, 0.011), first[3] + math.pi, hook_paint)
    bx, by = ahead(last, 0.03)
    top_z = last[2] - rise + LINK
    shared += hook("hook_b", (bx, by, top_z + 0.011), last[3], hook_paint)
    # The rating tag, wired to hook A's eye, flat on the floor; its stamping is baked on.
    tx, ty = ahead(first, -0.075)
    shared.append(ka.box("tag", (0.05, 0.03, 0.003), (tx, ty, 0.0015), galv))
    detail = list(links)
    for k, line in enumerate(("WLL 3.15 T", "HTR-RC-2208")):
        detail.append(ka.stencil("tag_stamp", line, 0.006, ink, (tx, ty + 0.006 - 0.01 * k, 0.0034), (0, 0, 1),
                                 up=(0, 1, 0)))
    name = "SM_Light_ChainCoil_HTR"
    obj, high = ka.high_low(name, shared, detail, proxy)
    height = max(p[2] for p in path) + LINK
    ka.collision_cylinder(name, 0, outer + LINK / 2, 0.0, height)
    top = max(path, key=lambda p: p[2])
    for s, (loc, normal) in {"SKT_Dog_Top_01": ((top[0], top[1], top[2] + LINK), None),
                             "SKT_Dog_Hook_01": ((ax, ay, 0.022), (0, 0, 1)),
                             "SKT_Dog_Hook_02": ((bx, by, top_z + 0.022), (0, 0, 1))}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "high": {name: high}, "cage": 0.012, "masked": [name],
            "size": (0.5, 0.5, 0.2), "res": 1024, "budget": 2000}
