"""Tool bag: docs/objects/props-light/tool-bag.md.

A 55 x 30 x 32 cm open-top 18 oz canvas rigger's bag on a rubber base, two steel-cored leather
handles standing over the opening, and tool handles showing. A loaf: bellied sides pinched in at
the mouth, sagging toward the heavy end. Pivot at the bottom centre. It is modelled along X and
turned a quarter at the end, so its stencilled end panel faces east (Y-), where the doc puts
SideE_01 (README-props: X+ is north). Variants: guild (natural canvas, the hitch glyph, LG-1183
and a taped name, a blue tarp patch on one corner) and htr (grey, HTR and CRIB 04 in orange).
"""
from __future__ import annotations

import math

import bmesh

import kinetic_art as ka

L, W, H = 0.55, 0.30, 0.32
BASE = 0.035


def loaf(name: str, material, sections: int = 9, around: int = 20):
    """The canvas body: rounded-rectangle sections along X, wider at the belly than the mouth,
    the ends rounded off and the top lower at the heavy end."""
    bm = bmesh.new()
    rings = []
    for i in range(sections):
        u = -1 + 2 * i / (sections - 1)
        end = math.sqrt(max(0.0, 1 - abs(u) ** 6)) * 0.12 + 0.88  # Rounded ends, never a point
        top = H - BASE - 0.025 * (u + 1) / 2                         # Sags 2.5 cm toward X+
        ring = []
        for k in range(around):
            t = 2 * math.pi * k / around
            c, s = math.cos(t), math.sin(t)
            y = math.copysign(abs(c) ** 0.5, c)
            z = math.copysign(abs(s) ** 0.5, s)
            h = (z + 1) / 2                                          # 0 at the base, 1 at the mouth
            belly = 1.0 + 0.06 * math.sin(math.pi * h) - 0.14 * h ** 3
            ring.append(bm.verts.new((u * L / 2 * (0.96 if abs(u) == 1 else 1.0),
                                      y * W / 2 * belly * end, BASE + h * top * end + (1 - end) * top / 2)))
        rings.append(ring)
    for i in range(sections - 1):
        a, b = rings[i], rings[i + 1]
        for k in range(around):
            j = (k + 1) % around
            bm.faces.new((a[k], a[j], b[j], b[k]))
    bm.faces.new(list(reversed(rings[0])))
    bm.faces.new(rings[-1])
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    obj = ka.mesh_object(name, bm)
    obj.data.materials.append(material)
    for p in obj.data.polygons:
        p.use_smooth = True
    return obj


def patch(name: str, body, material):
    """The tarp patch on the light end's front corner, shrink-wrapped onto the canvas."""
    bm = bmesh.new()
    bmesh.ops.create_grid(bm, x_segments=5, y_segments=4, size=0.05)
    from mathutils import Matrix
    bmesh.ops.transform(bm, verts=bm.verts, matrix=Matrix.Rotation(math.pi / 2, 4, "X"))
    bmesh.ops.translate(bm, verts=bm.verts, vec=(-L / 2 + 0.1, -W / 2 - 0.02, 0.12))
    obj = ka.mesh_object(name, bm)
    obj.data.materials.append(material)
    wrap = obj.modifiers.new("Wrap", "SHRINKWRAP")
    wrap.target = body
    wrap.wrap_method = "NEAREST_SURFACEPOINT"
    wrap.offset = 0.0015
    return obj


def build(variant: str = "guild") -> dict:
    guild = variant == "guild"
    canvas = ka.fabric("MI_Canvas_Natural", "#B9AE93" if guild else "#4A4D50", grab_z=H - 0.01, weave=0.0018)
    rubber = ka.rubber("MI_Rubber_Black", "#1E1F20", 0.8)
    # The doc's stand-in for the leather wrap over the steel core: black with hand grease, worn
    # pale at the grip's edges.
    leather = ka.plastic("MI_Timber_Dark", "#4A3522", rough=(0.55, 0.75), scuff="#2A1E14", detail=10.0, edge=0.003,
                         scratches=0.4)
    inside = ka.fabric("MI_Bag_Inside", "#2B2622", rough=0.95)
    timber = ka.wood("MI_Timber_Rough", "#7A5C3A", grain_axis="Z", detail=6.0, checks=0.2)
    tool_paint = ka.painted_steel("MI_Steel_Painted_Worn", ka.Wear(paint="#C8912A" if guild else "#E8620C",
                                                                   chips=0.5, scrape_height=0.0,
                                                                   streak_band=(H, H + 0.15), detail=10.0,
                                                                   edge=0.004, grime_reach=0.02))
    steel = ka.bare_steel("MI_Steel_Bare", rough=(0.5, 0.6), rust=0.3, detail=10.0)
    glyph = ka.ink("MI_Stencil_Ochre" if guild else "MI_Stencil_Orange", "#C8912A" if guild else "#E8620C",
                   "#B9AE93" if guild else "#4A4D50", detail=8.0, wear=0.4)
    parts = []
    body = loaf("bag", canvas)
    parts.append(body)
    # The mouth: the dark inside showing along the top, just above the canvas ridge, sagging with
    # it and kept short of the rounded ends, with a rolled canvas lip round it.
    sag = math.atan2(0.025, L)
    ridge = H - 0.0125
    mouth = ka.box("opening", (0.75 * L, 0.14, 0.003), (0, 0, 0), inside)
    ka.transform(mouth, (0, 0, ridge + 0.002), rotation_y=sag)
    parts.append(mouth)
    mx, my = 0.75 * L / 2, 0.07
    lip = [(x, y, ridge + 0.002 - x * math.tan(sag)) for x, y in ((-mx, -my), (mx, -my), (mx, my), (-mx, my))]
    parts.append(ka.tube("lip", lip, 0.006, canvas, closed=True))
    base = ka.box("base", (L - 0.05, W * 0.86, BASE), (0, 0, BASE / 2), rubber)
    ka.add_bevel(base, 0.012, 2)
    parts.append(base)
    # Handles stand up over the long sides, riveted through the canvas where it is 13 cm out.
    for sy in (-1, 1):
        y = sy * 0.125
        parts.append(ka.tube("handle", [(-0.12, y, H - 0.04), (-0.09, y, H + 0.07), (0.09, y, H + 0.07),
                                        (0.12, y, H - 0.045)], 0.011, leather))
        for x in (-0.12, 0.12):
            parts.append(ka.cylinder("rivet", 0.008, 0.008, (x, sy * 0.132, H - 0.05), "Y", 8, steel))
    # Tools out of the top: the hammer and spanner at the heavy end, the podger at the other.
    parts.append(ka.tube("hammer_handle", [(0.15, 0.03, 0.12), (0.19, 0.05, H + 0.12)], 0.014, timber))
    head = ka.box("hammer_head", (0.035, 0.12, 0.035), (0, 0, 0), steel)
    ka.add_bevel(head, 0.004, 1)
    ka.transform(head, (0.19, 0.05, H + 0.13), rotation_z=0.3)
    parts.append(head)
    parts.append(ka.tube("podger", [(-0.17, -0.04, 0.1), (-0.21, -0.06, H + 0.1)], 0.012, tool_paint))
    spanner = ka.box("spanner", (0.02, 0.05, 0.22), (0, 0, 0), steel)
    ka.transform(spanner, (0.05, -0.05, H), rotation_x=0.2)
    parts.append(spanner)
    # The end panel: the Guild's hitch-knot glyph and the owner's Ledger number, with a name on
    # a strip of tape; or HTR's block letters and tool-crib number.
    end = 0.96 * L / 2 + 0.002  # loaf() pulls the end panel in to 96% of the half-length
    if guild:
        parts.append(ka.ring("hitch_glyph", 0.022, 0.0035, (end + 0.002, 0, 0.21), "X", 16, 4, glyph))
        parts.append(ka.tube("hitch_glyph_tail", [(end + 0.002, -0.03, 0.18), (end + 0.002, 0.0, 0.205),
                                                  (end + 0.002, 0.03, 0.18)], 0.0035, glyph, sides=4))
        parts.append(ka.stencil("ledger", "LG-1183", 0.035, glyph, (end, 0, 0.14), (1, 0, 0), body))
        tape = ka.plastic("MI_Tape_Office_White", "#E6E4DD", rough=(0.45, 0.6), scuff="#8A8578", detail=10.0, edge=0.002)
        parts.append(ka.decal_plate("name_tape", 0.12, 0.025, tape, (end, 0, 0.09), (1, 0, 0), body))
        parts.append(ka.stencil("name", "D. ORRIN", 0.016, ka.ink("MI_Marker_Black", "#141414", "#E6E4DD", wear=0.15),
                                (end + 0.002, 0, 0.09), (1, 0, 0), body))
        parts.append(patch("patch", body, ka.fabric("MI_Tarp_Blue", "#2F5D8A", 0.8)))
    else:
        parts.append(ka.stencil("crib", "HTR", 0.06, glyph, (end, 0, 0.19), (1, 0, 0), body))
        parts.append(ka.stencil("crib_number", "CRIB 04", 0.03, glyph, (end, 0, 0.12), (1, 0, 0), body))
    name = "SM_Light_ToolBag_" + ("Guild" if guild else "HTR")
    obj = ka.join(name, parts, soft=True)
    # The quarter turn: the end panel (X+) goes to east (Y-).
    from mathutils import Matrix
    obj.data.transform(Matrix.Rotation(-math.pi / 2, 4, "Z"))

    def turned(x, y, z):
        return (y, -x, z)

    ka.collision_box(name, 0, (W, L, H), (0, 0, H / 2), chamfer=0.04)
    for s, (loc, normal) in {"SKT_Dog_Handle_01": (turned(0, 0.125, H + 0.081), (0, 0, 1)),
                             "SKT_Dog_Handle_02": (turned(0, -0.125, H + 0.081), (0, 0, 1)),
                             "SKT_Dog_Top_01": (turned(0, 0, ridge + 0.0035), None),
                             "SKT_Dog_SideE_01": (turned(end - 0.002, 0, 0.17), (0, -1, 0))}.items():
        ka.socket(s, loc, obj, normal)
    return {"name": name, "meshes": [obj], "size": (W, L, H + 0.15), "res": 2048, "budget": 2000}
