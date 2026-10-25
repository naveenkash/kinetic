"""Cylinder rack: docs/objects/volatiles/cylinder-rack.md.

120 x 80 x 150 cm, a welded cage of 40 mm angle iron holding six propane cylinders in two rows of
three, each row kept by a chain through the cylinders' guards, hooked to the end frames; fork
pockets in the base, open to the front; the Office placard on the front. Pivot at the bottom
centre of the base plate, X+ the front (a short end). The rack is the frame alone: the cylinders
are their own assets, placed at the Load sockets (the preview shows them in place).
"""
from __future__ import annotations

import kinetic_art as ka
import propane_cylinder

L, W, H = 1.20, 0.80, 1.50
A = 0.04  # Angle iron section
BASE = 0.08
SLOTS = [(x, y) for y in (-0.2, 0.2) for x in (-0.4, 0.0, 0.4)]
POCKETS = (-0.2, 0.2)  # Fork pockets, right and left of the front
CHAIN_Z = 1.15         # Through the cylinders' valve guards (1.04 to 1.2)


def chain(name: str, a, b, material, link: float = 0.035) -> list:
    """A run of alternating links from a to b."""
    import math
    dx, dy, dz = (b[i] - a[i] for i in range(3))
    length = math.sqrt(dx * dx + dy * dy + dz * dz)
    n = max(2, int(length / (link * 0.8)))
    links = []
    for i in range(n):
        t = (i + 0.5) / n
        c = (a[0] + dx * t, a[1] + dy * t, a[2] + dz * t)
        axis = "Z" if i % 2 == 0 else "Y"
        links.append(ka.ring(f"{name}_{i}", 0.008, 0.003, c, axis, 6, 3, material, stretch=link * 0.5))
    return links


def build(variant: str = "yard") -> dict:
    # The doc's wear: paint gone on the top rail where cylinders are lifted over it, rust at the
    # feet and in the fork pockets.
    frame_paint = ka.painted_steel("MI_Steel_Painted_Worn", ka.Wear(
        paint="#5B6670", chips=0.12, scrape_height=0.1, rust_streaks=0.5, streak_band=(0.1, H), detail=4.0,
        edge=0.008, grime_reach=0.05, bottom_rust=0.1, bare_bands=((H - 0.035, H + 0.01),), streak_from=(0.75, H - A)))
    steel = ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.6), rust=0.6, detail=5.0, bottom_rust=0.06)
    oiled = ka.bare_steel("MI_Chain_Oiled", "#3C3E42", rough=(0.3, 0.45), rust=0.15, detail=12.0, oil=0.8)
    placard = ka.Wear(paint="#E6E4DD", chips=0.3, scrape_height=0.0, rust_streaks=0.3, streak_band=(0.6, 0.85),
                      detail=8.0, edge=0.003, grime_reach=0.02)
    white = ka.painted_steel("MI_Stencil_White", placard)
    ink = ka.stencil_paint("MI_Stencil_Black", "#141414", placard)

    parts = []
    hx, hy = L / 2, W / 2
    for sx in (-1, 1):
        for sy in (-1, 1):
            parts.append(ka.box("post", (A, A, H - BASE), (sx * (hx - A / 2), sy * (hy - A / 2), BASE + (H - BASE) / 2),
                                frame_paint))
    for z in (BASE + A / 2, 0.75, H - A / 2):
        for sy in (-1, 1):
            parts.append(ka.box("rail_x", (L, A, A), (0, sy * (hy - A / 2), z), frame_paint))
        for sx in (-1, 1):
            parts.append(ka.box("rail_y", (A, W, A), (sx * (hx - A / 2), 0, z), frame_paint))
    # The end rails the chains hook to, at the cylinders' guards.
    for sx in (-1, 1):
        parts.append(ka.box("chain_rail", (A, W, A), (sx * (hx - A / 2), 0, CHAIN_Z), frame_paint))
    parts.append(ka.box("base_plate", (L, W, 0.012), (0, 0, BASE + 0.006), steel))
    # Fork pockets along X under the base, open to the front and back.
    for y in POCKETS:
        parts.append(ka.box("fork_pocket", (L, 0.22, BASE), (0, y, BASE / 2), steel))
    # A divider between the two rows.
    parts.append(ka.box("divider", (L - 2 * A, 0.02, 0.05), (0, 0, 0.75), frame_paint))
    # A chain along each row through the cylinders' guards, a hook over the end rail at each end.
    for y in (-0.2, 0.2):
        parts += chain("chain", (-hx + A + 0.02, y, CHAIN_Z), (hx - A - 0.02, y, CHAIN_Z), oiled)
        for sx in (-1, 1):
            parts.append(ka.tube("hook", [(sx * (hx - A - 0.02), y, CHAIN_Z),
                                          (sx * (hx - A / 2), y, CHAIN_Z + A / 2 + 0.008),
                                          (sx * (hx + 0.004), y, CHAIN_Z + 0.01)], 0.006, steel))
    # The Office placard wired to the front of the middle rail, with the prep crew's tally.
    parts.append(ka.box("placard", (0.004, 0.30, 0.18), (hx + 0.002, 0.0, 0.75), white))
    for k, line in enumerate(("OFFICE OF THE LAY", "SITE PROPERTY", "M-1180   6 / 6")):
        parts.append(ka.stencil("placard_text", line, 0.024, ink, (hx + 0.005, 0.0, 0.8 - 0.045 * k), (1, 0, 0)))

    name = "SM_Volatile_CylinderRack"
    obj = ka.join(name, parts, soft=True)
    # The doc's collision: the posts and rails as 4 cm boxes (the four posts, the top rails and
    # the chain rails) and the base (plate and fork pockets). The cylinders are their own bodies.
    boxes = [((A, A, H - BASE), (sx * (hx - A / 2), sy * (hy - A / 2), BASE + (H - BASE) / 2))
             for sx in (-1, 1) for sy in (-1, 1)]
    boxes += [((L, A, A), (0, sy * (hy - A / 2), H - A / 2)) for sy in (-1, 1)]
    boxes += [((A, W, A), (sx * (hx - A / 2), 0, z)) for sx in (-1, 1) for z in (H - A / 2, CHAIN_Z)]
    boxes.append(((L, W, BASE + 0.012), (0, 0, (BASE + 0.012) / 2)))
    for i, (size, center) in enumerate(boxes):
        ka.collision_box(name, i, size, center)
    # Load sockets at each cylinder's own Top socket (its guard ring's top); the rest on the
    # frame, the front being X+. Left and right are as seen from the front.
    top = propane_cylinder.TOP_SOCKET
    sockets = {f"SKT_Dog_Load_{i + 1:02d}": ((x + top[0], y + top[1], BASE + 0.012 + top[2]), (0, 0, 1))
               for i, (x, y) in enumerate(SLOTS)}
    sockets["SKT_Dog_Valve_01"] = ((hx + 0.004, 0.2, CHAIN_Z + 0.01), (1, 0, 0))
    sockets["SKT_Dog_Top_01"] = ((hx - A / 2, 0, H), None)
    sockets["SKT_Dog_Top_02"] = ((-hx + A / 2, 0, H), None)
    sockets["SKT_Dog_Hook_01"] = ((hx, POCKETS[1], BASE / 2), (1, 0, 0))
    sockets["SKT_Dog_Hook_02"] = ((hx, POCKETS[0], BASE / 2), (1, 0, 0))
    sockets["SKT_Dog_Frame_01"] = ((hx, -hy + A / 2, 1.0), (1, 0, 0))
    sockets["SKT_Dog_Frame_02"] = ((-hx, hy - A / 2, 1.0), (-1, 0, 0))
    for s, (loc, normal) in sockets.items():
        ka.socket(s, loc, obj, normal)

    # Preview only: the six cylinders in their slots, kept out of the bake and the export.
    cyl = propane_cylinder.materials()
    shown = []
    for x, y in SLOTS:
        c = ka.join("PREVIEW_cylinder", propane_cylinder.build_parts(**cyl))
        c.location = (x, y, BASE + 0.012)
        shown.append(c)
    return {"name": name, "meshes": [obj], "size": (L, W, H), "res": 2048, "budget": 5000, "preview_only": shown}
