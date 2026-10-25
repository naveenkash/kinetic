"""Builds the five harness characters (docs/characters/harnesses/*.md, docs/characters/README.md)
as skinned meshes on one shared skeleton, with their animations, from code:

    blender -b --factory-startup -P art/characters/harness.py -- [--class Rigger] [--out art/out/characters]
        [--preview-only] [--res 4096]

For each class it writes art/out/characters/<Class>/: SK_Harness_<Class>.fbx (the mesh, the
skeleton and every animation as its own take), the baked textures (BaseColor, ORM and a
tangent-space Normal map, OpenGL convention), a preview render, and report.json (triangles,
the material slots, the textures, the silhouette's shoulder width, the takes and their lengths,
the bones).

What the specs fix, and this follows:
- one skeleton, Unreal Mannequin bone names (root, pelvis, spine_01 to 03, neck_01, head,
  clavicle, upperarm, lowerarm, hand, thigh, calf, foot, ball), identical bone lengths for every
  class; 1.9 m tall in a 0.4 m capsule, the eye at 1.7 m; facing +X, so Unreal's import gives
  a character facing its actor's forward;
- the silhouette rules: shoulder width with plates on (Climber 0.48 m, Scrapper 0.54, Rigger and
  Doubler 0.62, Breaker 0.82), bulk where the class carries it (the Rigger's spine frame, the
  Doubler's twin-drum right forearm, the Scrapper's low left hip, the Breaker's blade
  shoulders), the Spool on the right forearm at its class position, four Catch Plates at their
  class sizes, never covered, each with a team-tint rim band;
- the faction palettes and wear stories for plates, frame, straps and boots.

How it is made (harness_body.py, harness_sdf.py, harness_parts.py, harness_look.py):
- the soft parts are signed distance fields meshed from code: a work coverall over an adult
  figure (modelled in an A-pose, bone-heat weighted there, then posed down onto the rest
  skeleton), the head and neck, gloved hands and the boots with their soles. Each is meshed
  dense and decimated: the dense mesh is what the textures are baked from;
- the hard parts (plates, frame, Spool, Dogs, Cutter, buckles, helmet) follow one bone each; the
  soft gear (straps, belt, leg loops, wraps, laces) takes its weights from the surface under it;
- every surface is a procedural material with relief (cloth grain and seams, leather grain,
  chipped and scratched paint, brushed metal, webbing weave and stitching), baked from the
  detailed meshes onto the game mesh at 4K (Cycles, on the GPU when there is one).

What is PROPOSED here: about 95k triangles (the specs' LOD0 is 85k), no sockets on the mesh
(the game places the plates from code), and six procedural takes: Idle, Run, Climb, Fall,
Brace and Stagger, keyed as rotations of the rest pose. The faces and bodies are stand-ins until
MetaHuman supplies them; the harness is fitted onto those.
"""
from __future__ import annotations

import argparse
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "..", "lib")]

sys.path[:0] = [HERE]

import bmesh  # noqa: E402
import bpy  # noqa: E402
import numpy as np  # noqa: E402
from mathutils import Matrix, Quaternion, Vector  # noqa: E402

import harness_sdf as sdf  # noqa: E402
import kinetic_art as ka  # noqa: E402

# Bones: name, parent, head, tail (metres; X forward, Y the character's left, Z up).
BONES = [
    ("root", None, (0, 0, 0), (0, 0.1, 0)),  # Along +Y, roll 0: an identity rest matrix, so Unreal's root is clean
    ("pelvis", "root", (0, 0, 0.95), (0, 0, 1.05)),
    ("spine_01", "pelvis", (0, 0, 1.05), (0, 0, 1.18)),
    ("spine_02", "spine_01", (0, 0, 1.18), (0, 0, 1.32)),
    ("spine_03", "spine_02", (0, 0, 1.32), (0, 0, 1.46)),
    ("neck_01", "spine_03", (0, 0, 1.46), (0, 0, 1.56)),
    ("head", "neck_01", (0, 0, 1.56), (0, 0, 1.80)),
]
for side, s in (("l", 1.0), ("r", -1.0)):
    BONES += [
        (f"clavicle_{side}", "spine_03", (0, 0.03 * s, 1.43), (0, 0.18 * s, 1.44)),
        (f"upperarm_{side}", f"clavicle_{side}", (0, 0.19 * s, 1.43), (0, 0.23 * s, 1.15)),
        (f"lowerarm_{side}", f"upperarm_{side}", (0, 0.23 * s, 1.15), (0, 0.25 * s, 0.89)),
        (f"hand_{side}", f"lowerarm_{side}", (0, 0.25 * s, 0.89), (0, 0.26 * s, 0.72)),
        (f"thigh_{side}", "pelvis", (0, 0.10 * s, 0.95), (0, 0.10 * s, 0.52)),
        (f"calf_{side}", f"thigh_{side}", (0, 0.10 * s, 0.52), (0, 0.10 * s, 0.10)),
        (f"foot_{side}", f"calf_{side}", (0, 0.10 * s, 0.10), (0.13, 0.10 * s, 0.03)),
        (f"ball_{side}", f"foot_{side}", (0.13, 0.10 * s, 0.03), (0.21, 0.10 * s, 0.02)),
    ]

# Per class: silhouette (shoulder width with plates, m; the specs'. The fitted pauldrons make the
# real shoulders narrower, and report.json measures them), body bulk, plates (chest, back,
# shoulder; width, height in m), the Spool's place on the right forearm (0 elbow, 1 wrist), its
# length and radius, the livery: (hex, roughness, metallic) per material slot, and the workwear:
# PROPOSED coverall colours (the specs give none) and the Breaker's retro-reflective bands (z, m).
CLASSES = {
    "Climber": dict(
        workwear="#5E6B45", wear=0.3, boots=("climb", "climb"), glove=None, thread="#C9C3B4", dirt="#8A8070", iris="#3E5A6A",
        suit=dict(ease=0.55, pockets=(), knee_pads=False, folds=0.6, shoulder=0.55), 
        shoulders=0.48, bulk=0.85, limb=0.92, chest=(0.26, 0.22), back=(0.24, 0.30), shoulder=(0.14, 0.12), oval=True,
        plate_style="mirror", spool_at=0.88, spool=(0.09, 0.03), headgear="wrap", frame="ribbons", wraps=True, lean=0.0,
        boot_top=0.14, steel_toe=False, strap_width=0.03, tape=("#C9CDD2", "#F4F1EA"), helmet_colour="#F4F1EA",
        skin="#C69C7E", number="31.4", lease="SPONSORED", run_speed=7.5,
        colours={"Plate": ("#D8DADC", 0.08, 1.0), "Frame": ("#9BA0A4", 0.4, 1.0), "Strap": ("#F4F1EA", 0.95, 0.0),
                 "Body": ("#E9E4D8", 0.9, 0.0), "Boot": ("#E9E4D8", 0.8, 0.0), "Spool": ("#C9CDD2", 0.35, 1.0),
                 "Accent": ("#C9CDD2", 0.4, 1.0)}),
    "Scrapper": dict(
        workwear="#2F7F72", wear=0.9, boots=("work", "hitop"), glove="#6A5A3A", thread="#C8862B", dirt="#3A3026", iris="#3A2618",
        suit=dict(ease=1.0, pockets=("thigh_r", "seat"), knee_pads=True, folds=1.1), 
        shoulders=0.54, bulk=0.95, chest=(0.28, 0.24), back=(0.30, 0.28), shoulder=(0.15, 0.13), shoulder_r=(0.13, 0.11),
        plate_style="salvage", spool_at=0.55, spool=(0.16, 0.045), capacitor=True, headgear="salvage", frame="half",
        clutter=True, lean=6.0, boot_top=0.22, laces=True, strap_width=0.045, tape=("#E8C23A", "#3DA38C"),
        helmet_colour="#3DA38C", skin="#8D5A3B", number="K-77", lease="KESSL 2201", run_speed=7.0,
        colours={"Plate": ("#7E8A86", 0.6, 0.9), "Frame": ("#C8862B", 0.6, 0.3), "Strap": ("#7A3B22", 0.9, 0.0),
                 "Body": ("#3A4A45", 0.85, 0.0), "Boot": ("#4A3A2C", 0.7, 0.0), "Spool": ("#A67C3B", 0.4, 1.0),
                 "Accent": ("#E8C23A", 0.6, 0.0)}),
    "Rigger": dict(
        workwear="#2C3E5C", wear=0.6, boots=("work", "work"), glove="#6B5238", thread="#B9A98A", dirt="#3A3026", iris="#4A3524",
        suit=dict(ease=1.0, pockets=("thigh_r", "seat"), knee_pads=True, folds=1.0), 
        shoulders=0.62, bulk=1.0, chest=(0.30, 0.26), back=(0.32, 0.30), shoulder=(0.16, 0.14), plate_style="lip",
        spool_at=0.55, spool=(0.16, 0.05), headgear="square", frame="rails", lean=0.0, boot_top=0.2, laces=True,
        strap_width=0.05, tape=("#C8862B", "#141211"), helmet_colour="#C8862B", skin="#A8785A", number="L-0412",
        lease="CLEAR", run_speed=6.2,
        colours={"Plate": ("#7E786C", 0.6, 0.9), "Frame": ("#C8862B", 0.6, 0.3), "Strap": ("#B9A98A", 0.9, 0.0),
                 "Body": ("#2A2622", 0.85, 0.0), "Boot": ("#4A3A2C", 0.7, 0.0), "Spool": ("#A67C3B", 0.35, 1.0),
                 "Accent": ("#A67C3B", 0.35, 1.0)}),
    "Doubler": dict(
        workwear="#6E2A2A", wear=0.45, boots=("work", "work"), glove="#141416", thread="#39FF14", dirt="#2A2622", iris="#2A1C14",
        suit=dict(ease=0.85, pockets=("thigh_r",), knee_pads=False, folds=0.9), 
        shoulders=0.62, bulk=1.0, chest=(0.30, 0.26), back=(0.32, 0.30), shoulder=(0.16, 0.14), plate_style="lip",
        spool_at=0.5, spool=(0.28, 0.055), twin=True, headgear="hood", frame="short", neon=True, lean=0.0, boot_top=0.24,
        laces=True, strap_width=0.045, tape=("#39FF14", "#FF2BD6"), helmet_colour="#0E0E10", skin="#B98A6A",
        number="RED 9", lease="", run_speed=6.2,
        colours={"Plate": ("#16161A", 0.5, 0.9), "Frame": ("#16161A", 0.5, 0.6), "Strap": ("#1A1A1E", 0.6, 0.0),
                 "Body": ("#0E0E10", 0.85, 0.0), "Boot": ("#0E0E10", 0.6, 0.0), "Spool": ("#6A5A4A", 0.45, 1.0),
                 "Accent": ("#FFE600", 0.4, 0.0)}),
    "Breaker": dict(
        workwear="#E0662A", reflective=((1.125, 1.165), (0.535, 0.575)), wear=0.15, boots=("tall", "tall"), glove="#33363A", thread="#3A3A3A", dirt="#4A4238", iris="#3A2A1E",
        suit=dict(ease=1.1, pockets=("thigh_r", "seat"), knee_pads=True, folds=0.8), 
        shoulders=0.82, bulk=1.3, limb=1.1, chest=(0.36, 0.30), back=(0.40, 0.36), shoulder=(0.22, 0.18), rake=15.0,
        plate_style="chamfer", spool_at=0.12, spool=(0.22, 0.07), gauntlet=True, headgear="visor", frame="shell",
        lean=5.0, boot_top=0.5, shin_shell=True, strap_width=0.07, tape=("#FF5A1F", "#2B2E31"), helmet_colour="#FF5A1F",
        skin="#7A4E36", number="HTR 118", lease="KESSL 9930", run_speed=5.0,
        colours={"Plate": ("#5A5E62", 0.45, 0.85), "Frame": ("#2B2E31", 0.7, 0.2), "Strap": ("#1F2124", 0.85, 0.0),  # PROPOSED: black webbing (the spec's orange vanishes on the orange suit)
                 "Body": ("#2B2E31", 0.85, 0.0), "Boot": ("#2B2E31", 0.6, 0.0), "Spool": ("#5A5E62", 0.45, 0.85),
                 "Accent": ("#FF5A1F", 0.5, 0.0)}),
}
TEAM_SLOT = "TeamTint"  # The rim bands and the shoulder lamp: Unreal drives the colour


def bone_frame(name: str):
    head, tail = next((Vector(b[2]), Vector(b[3])) for b in BONES if b[0] == name)
    return head, tail


def materials(cls: str, spec: dict) -> dict:
    """The class's livery as procedural materials (the faction palettes in characters/README.md),
    worn as its spec says (harness_look.py): painted, bare, mirror or anodised metal, webbing,
    leather, rubber, polymer, ceramic, tape. They are baked to textures before export."""
    import harness_look as hl

    c = spec["colours"]
    wear = spec["wear"]
    style = spec["plate_style"]
    n = lambda slot: f"M_{cls}_{slot}"
    plate = {
        "mirror": lambda: hl.mirror(n("Plate"), c["Plate"][0], wear),
        "chamfer": lambda: hl.anodised(n("Plate"), c["Plate"][0], wear),
        "salvage": lambda: hl.worn_paint(n("Plate"), c["Plate"][0], "#B8B4AC", wear, scratch=1.0),
    }.get(style, lambda: (hl.worn_paint(n("Plate"), c["Plate"][0], "#8C8F92", wear) if spec.get("neon") else
                          hl.bare_metal(n("Plate"), c["Plate"][0], 0.6, wear, patina="#2E2A22", brushed=False, polish=0.9)))()
    frame = {
        "ribbons": lambda: hl.bare_metal(n("Frame"), c["Frame"][0], 0.4, wear),
        "shell": lambda: hl.polymer(n("Frame"), c["Frame"][0], wear, (0.62, 0.78)),
    }.get(spec["frame"], lambda: hl.worn_paint(n("Frame"), c["Frame"][0], "#A8AAAD", wear))()
    metal = lambda slot, colour, rough: hl.bare_metal(n(slot), colour, rough, wear, patina="#3A2E1E" if colour in ("#A67C3B",) else "#2E2C2A")
    accent = c["Accent"]
    m = {
        "Plate": plate,
        "Frame": frame,
        "Strap": hl.webbing(n("Strap"), c["Strap"][0], wear, spec.get("dirt", "#3A3026")),
        "Body": hl.tape(n("Body"), c["Body"][0], wear),
        "Boot": hl.leather(n("Boot"), c["Boot"][0], wear),
        "Sole": hl.rubber(n("Sole"), "#1A1A1A", wear),
        "Spool": (hl.anodised(n("Spool"), c["Spool"][0], wear) if style == "chamfer" else
                  hl.heat_scored(n("Spool"), "#8C8F92", _line_exits(spec), 0.06, wear) if spec.get("twin") else
                  metal("Spool", c["Spool"][0], c["Spool"][1])),
        "Accent": (metal("Accent", accent[0], accent[1]) if accent[2] > 0.5 else hl.polymer(n("Accent"), accent[0], wear, (0.45, 0.6))),
        "Steel": metal("Steel", "#9EA3A6", 0.33),
        "Stamp": metal("Stamp", "#B8B4AC", 0.38),
        "Ceramic": hl.glazed(n("Ceramic"), "#E8E6E0", 0.3),
        "Tape": hl.tape(n("Tape"), spec["tape"][0], wear),
        "TapeB": hl.tape(n("TapeB"), spec["tape"][1], wear),
        "Seal": hl.polymer(n("Seal"), "#F2F2F0", wear * 0.5, (0.45, 0.55)),
        "Helmet": hl.worn_paint(n("Helmet"), spec["helmet_colour"], "#6C6F73", wear),
        "Visor": ka.flat_material(n("Visor"), "#101418", 0.06, 0.4),  # A tinted shield: glossy, clean
        "Gel": hl.glazed(n("Gel"), "#E8D9B0", 0.1),
        "Dark": hl.polymer(n("Dark"), "#2B2E31", wear),
        TEAM_SLOT: ka.emissive(f"M_{cls}_{TEAM_SLOT}", "#E69F00", 6.0),
        # The soft parts.
        "Skin": hl.skin(n("Skin")),
        "Eye": hl.eye(n("Eye"), spec.get("iris", "#4A3524")),
        "Glove": hl.leather(n("Glove"), spec["glove"] or "#6B5238", wear * 0.8),
        "Upper": (hl.tape(n("Upper"), c["Boot"][0], wear) if spec["boots"][0] == "climb" else
                  hl.leather(n("Upper"), c["Boot"][0], wear, toe_steel=spec.get("steel_toe", True))),
        "Canvas": hl.tape(n("Canvas"), "#B8452A", wear),
        "Rand": hl.rubber(n("Rand"), "#1A1A1A", wear),
        "Foxing": hl.rubber(n("Foxing"), "#D8D2C4", wear),
        "Reflective": hl.tape(n("Reflective"), "#C8CCCE", wear * 0.5),
    }
    return m


def build_skeleton(bones=None, name: str = "Armature") -> bpy.types.Object:
    data = bpy.data.armatures.new(name)
    rig = bpy.data.objects.new(name, data)  # Unreal drops a node named Armature
    bpy.context.scene.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode="EDIT")
    for name, parent, head, tail in bones or BONES:
        b = data.edit_bones.new(name)
        b.head, b.tail = head, tail
        b.roll = 0.0
        if parent:
            b.parent = data.edit_bones[parent]
            b.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    return rig


def weight_body(body, rig) -> None:
    """Bone-heat weights for the body (it bends at every joint); the root does not deform."""
    rig.data.bones["root"].use_deform = False
    bpy.ops.object.select_all(action="DESELECT")
    body.select_set(True)
    rig.select_set(True)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.parent_set(type="ARMATURE_AUTO")
    # Keep the groups, drop the parenting: the joined mesh is parented once, at the end.
    for m in list(body.modifiers):
        body.modifiers.remove(m)
    body.parent = None


# Weights ---------------------------------------------------------------------------------------

DEFORM = [b[0] for b in BONES if b[0] != "root"]


def weights_of(obj) -> np.ndarray:
    """The vertex groups as a (vertices, bones) matrix in DEFORM order, rows normalised."""
    index = {g.index: DEFORM.index(g.name) for g in obj.vertex_groups if g.name in DEFORM}
    W = np.zeros((len(obj.data.vertices), len(DEFORM)))
    for v in obj.data.vertices:
        for g in v.groups:
            if g.group in index:
                W[v.index, index[g.group]] = g.weight
    total = W.sum(1)
    W[total < 1e-6, DEFORM.index("pelvis")] = 1.0
    return W / W.sum(1)[:, None]


def set_weights(obj, W: np.ndarray, top: int = 4) -> None:
    """Writes a weight matrix as vertex groups, keeping each vertex's `top` strongest bones."""
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)
    W = W.copy()
    if top < W.shape[1]:
        cut = np.sort(W, 1)[:, -top][:, None]
        W[W < cut] = 0.0
    W /= np.maximum(W.sum(1), 1e-9)[:, None]
    for b, name in enumerate(DEFORM):
        col = W[:, b]
        idx = np.nonzero(col > 1e-4)[0]
        if not len(idx):
            continue
        group = obj.vertex_groups.new(name=name)
        for level in np.unique(np.round(col[idx], 3)):
            members = idx[np.round(col[idx], 3) == level]
            group.add(members.tolist(), float(level), "REPLACE")


def transfer(source, W: np.ndarray, points: np.ndarray) -> np.ndarray:
    """Weights at `points` from the nearest place on `source`'s surface (barycentric in its
    triangle): what a strap or a dense copy of the body takes from the body under it."""
    from mathutils.bvhtree import BVHTree

    me = source.data
    me.calc_loop_triangles()
    tris = np.empty(len(me.loop_triangles) * 3, np.int32)
    me.loop_triangles.foreach_get("vertices", tris)
    tris = tris.reshape(-1, 3)
    co = sdf.verts_of(source)
    tree = BVHTree.FromPolygons(co.tolist(), tris.tolist())
    hit_tri = np.zeros(len(points), np.int64)
    hit = np.zeros((len(points), 3))
    for i, p in enumerate(points):
        loc, _n, idx, _d = tree.find_nearest(Vector(p))
        hit_tri[i] = idx if idx is not None else 0
        hit[i] = loc if loc is not None else p
    a, b, c = (co[tris[hit_tri, k]] for k in range(3))
    v0, v1, v2 = b - a, c - a, hit - a
    d00, d01, d11 = sdf.dot(v0, v0), sdf.dot(v0, v1), sdf.dot(v1, v1)
    d20, d21 = sdf.dot(v2, v0), sdf.dot(v2, v1)
    den = np.maximum(d00 * d11 - d01 * d01, 1e-18)
    wv = (d11 * d20 - d01 * d21) / den
    ww = (d00 * d21 - d01 * d20) / den
    bary = np.clip(np.stack([1 - wv - ww, wv, ww], 1), 0, 1)
    bary /= bary.sum(1)[:, None]
    out = sum(bary[:, k, None] * W[tris[hit_tri, k]] for k in range(3))
    return out / out.sum(1)[:, None]


def deform(verts: np.ndarray, W: np.ndarray, matrices: np.ndarray) -> np.ndarray:
    """Linear blend skinning: the vertices moved by the bones' deformation matrices."""
    out = np.zeros_like(verts)
    for b in range(W.shape[1]):
        w = W[:, b]
        if w.max() <= 0:
            continue
        m = matrices[b]
        out += w[:, None] * (verts @ m[:3, :3].T + m[:3, 3])
    return out


def bone_weights(n: int, **columns) -> np.ndarray:
    W = np.zeros((n, len(DEFORM)))
    for name, col in columns.items():
        W[:, DEFORM.index(name)] = col
    return W / np.maximum(W.sum(1), 1e-9)[:, None]


# The soft parts ---------------------------------------------------------------------------------

def soft_pieces(cls: str, spec: dict, mats: dict) -> dict:
    """The suit, head, eyes, hands, boots and soles (harness_body.py): each a weighted game mesh
    and the dense mesh it is baked from, in the rest pose. Returns the pieces and what the gear
    is fitted to."""
    import harness_body as hb

    pieces = {"low": [], "high": [], "weights": {}}
    # The suit, modelled and weighted in the A-pose, then posed down onto the rest skeleton.
    suit = hb.Suit(spec, BONES)
    v, q = sdf.mesh(suit, *suit.bounds(), 0.004)
    v = sdf.refine(suit, v, 2)
    high = sdf.to_object(f"HIGH_Suit", v, q)
    low = sdf.decimated(high, 30000, "Suit")
    arig = build_skeleton(hb.apose_bones(BONES), "ARig")
    weight_body(low, arig)
    W = clean_suit_weights(weights_of(low), sdf.verts_of(low))
    Wh = transfer(low, W, v)
    pose(arig, {"upperarm_l": (-hb.APOSE, 0, 0), "upperarm_r": (hb.APOSE, 0, 0)})
    bpy.context.view_layer.update()
    mats_ = np.array([np.array(pb.matrix @ pb.bone.matrix_local.inverted()) for pb in (arig.pose.bones[n] for n in DEFORM)])
    sdf.set_verts(low, deform(sdf.verts_of(low), W, mats_))
    v = deform(v, Wh, mats_)
    sdf.set_verts(high, v)
    bpy.data.objects.remove(arig)
    set_weights(low, W)
    grime, sweat = suit.grime(v, BONES)
    sdf.set_attribute(high, "grime", grime)
    sdf.set_attribute(high, "sweat", sweat)
    high.data.materials.append(mats["Suit"])
    low.data.materials.append(mats["Suit"])
    pieces["suit"], pieces["suit_field"] = low, suit
    pieces["low"].append(low)
    pieces["high"].append(high)
    pieces["weights"]["Suit"] = weights_of(low)

    # The head and neck: the head bone, the neck into the collar.
    head = hb.Head(spec)
    v, q = sdf.project_sphere(head, head.centre, subdivisions=96, reach=0.26)
    v = sdf.relax(v, q, head, iterations=8)
    high = sdf.to_object("HIGH_Head", v, q)
    sdf.set_attribute(high, "skin", head.colour(v, ka.hex_rgb(spec["skin"])), "FLOAT_COLOR")
    sdf.set_attribute(high, "hair", head.hair_mask(v - head.o))
    low = sdf.decimated(high, 9000, "Head", symmetric=True)

    def head_weights(pts):
        z = pts[:, 2]
        top = smooth(1.56, 1.625, z)
        neck = (1 - top) * smooth(1.47, 1.53, z)
        return bone_weights(len(pts), head=top, neck_01=neck, spine_03=1 - top - neck)

    set_weights(low, head_weights(sdf.verts_of(low)))
    for o in (high, low):
        o.data.materials.append(mats["Skin"])
    pieces["low"].append(low)
    pieces["high"].append(high)
    for i, c in enumerate(head.world_eyes()):
        for dense, target in ((True, pieces["high"]), (False, pieces["low"])):
            bm = bmesh.new()
            bmesh.ops.create_uvsphere(bm, u_segments=48 if dense else 18, v_segments=32 if dense else 12, radius=head.eye_r)
            bmesh.ops.rotate(bm, verts=bm.verts, cent=(0, 0, 0), matrix=Matrix.Rotation(math.pi / 2, 3, "Y"))
            eye = ka.mesh_object(("HIGH_" if dense else "") + "Eye", bm)
            local = sdf.verts_of(eye)
            sdf.set_attribute(eye, "eye", local / head.eye_r, "FLOAT_COLOR")
            sdf.set_verts(eye, local + c)
            for poly in eye.data.polygons:
                poly.use_smooth = True
            eye.data.materials.append(mats["Eye"])
            if not dense:
                set_weights(eye, bone_weights(len(local), head=np.ones(len(local))))
            target.append(eye)

    # The hands: the hand bone from the wrist down, the cuff with the forearm.
    climber = spec["glove"] is None
    for side in "lr":
        hand = hb.Hand(side, BONES, glove=0.0008 if climber else 0.0025, missing=0)
        v, q = sdf.mesh(hand, *hand.bounds(), 0.0015)
        v = sdf.refine(hand, v, 2)
        high = sdf.to_object(f"HIGH_Hand_{side}", v, q)
        low = sdf.decimated(high, 3600, f"Hand_{side}")
        for o in (high, low):
            if climber:  # Skin, the palm and knuckles taped
                o.data.materials.append(mats["Skin"])
                o.data.materials.append(mats["Strap"])
                co = sdf.verts_of(o)
                along = hand.local(co)[:, 2]
                taped = np.zeros(len(o.data.polygons), np.int32)
                o.data.polygons.foreach_get("material_index", taped)
                centres = np.array([np.mean(along[list(poly.vertices)]) for poly in o.data.polygons])
                o.data.polygons.foreach_set("material_index", ((centres > -0.04) & (centres < 0.088)).astype(np.int32))
                if o is high:
                    sdf.set_attribute(o, "skin", np.tile(np.array(ka.hex_rgb(spec["skin"]))[None, :3], (len(co), 1)), "FLOAT_COLOR")
            else:
                o.data.materials.append(mats["Glove"])
        lw = hand.local(sdf.verts_of(low))[:, 2]
        h_w = smooth(-0.018, 0.022, lw)
        set_weights(low, bone_weights(len(lw), **{f"hand_{side}": h_w, f"lowerarm_{side}": 1 - h_w}))
        pieces["low"].append(low)
        pieces["high"].append(high)

    # The boots and their soles: the shaft with the calf, the foot and toes with theirs.
    boots = {}
    for side, style in zip("lr", spec["boots"]):
        boot = hb.Boot(side, spec, suit, style)
        boots[side] = boot
        lo, hi = boot.bounds()
        v, q = sdf.mesh(boot.upper, lo, hi, 0.0022)
        v = sdf.refine(boot.upper, v, 2)
        high = sdf.to_object(f"HIGH_Boot_{side}", v, q)
        low = sdf.decimated(high, 4200 if style == "tall" else 3400, f"Boot_{side}")
        sv, sq = sdf.mesh(boot.sole, *boot.bounds(top=boot.sole_h + 0.004), 0.0013)
        sv = sdf.refine(boot.sole, sv, 2)
        shigh = sdf.to_object(f"HIGH_Sole_{side}", sv, sq)
        slow = sdf.decimated(shigh, 1800, f"Sole_{side}")
        for o in (high, low):
            if style in ("climb", "hitop"):
                o.data.materials.append(mats["Upper" if style == "climb" else "Canvas"])
                o.data.materials.append(mats["Rand" if style == "climb" else "Foxing"])
                co = sdf.verts_of(o)
                cen = np.array([co[list(poly.vertices)].mean(0) for poly in o.data.polygons])
                band = cen[:, 2] < boot.sole_h + (0.012 if style == "climb" else 0.016)
                toe = (cen[:, 0] > 0.165) & (cen[:, 2] < boot.sole_h + 0.05)
                o.data.polygons.foreach_set("material_index", (band | toe).astype(np.int32))
            else:
                o.data.materials.append(mats["Upper"])
        for o in (shigh, slow):
            o.data.materials.append(mats["Sole"])
        co = sdf.verts_of(low)
        calf = smooth(0.12, 0.185, co[:, 2]) * smooth(0.09, 0.03, co[:, 0])
        ball = (1 - calf) * smooth(0.12, 0.17, co[:, 0])
        set_weights(low, bone_weights(len(co), **{f"calf_{side}": calf, f"ball_{side}": ball, f"foot_{side}": 1 - calf - ball}))
        sco = sdf.verts_of(slow)
        ball = smooth(0.1, 0.16, sco[:, 0])
        set_weights(slow, bone_weights(len(sco), **{f"ball_{side}": ball, f"foot_{side}": 1 - ball}))
        pieces["low"] += [low, slow]
        pieces["high"] += [high, shigh]
        pieces["weights"][low.name] = weights_of(low)
        pieces[f"boot_{side}"] = low
    pieces["boots"] = boots
    return pieces


def clean_suit_weights(W: np.ndarray, co: np.ndarray) -> np.ndarray:
    """Heat weights leak across the crotch and up the back of the waist: a leg's bones move only
    its own side, and the thighs give way to the pelvis above the hip joints (where the belt sits)."""
    W = W.copy()
    y, z = co[:, 1], co[:, 2]
    for side, sign in (("l", 1.0), ("r", -1.0)):
        cols = [DEFORM.index(f"{b}_{side}") for b in ("thigh", "calf", "foot", "ball")]
        other = y * sign < -0.004
        W[np.ix_(other, cols)] = 0.0
        fade = smooth(1.04, 0.96, z)
        W[:, cols[0]] *= fade
    W[W.sum(1) < 1e-6, DEFORM.index("pelvis")] = 1.0
    return W / W.sum(1)[:, None]


def smooth(e0: float, e1: float, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def harness_parts_on(parts, spec: dict, boots: dict, head: dict | None = None) -> None:
    """The harness on a body (parts.body, weighted): the plates curved to the chest and back,
    pauldrons, the fall-arrest webbing and spine hardware (harness_armour.py), the Spool, Cutter,
    the boots' hardware, lease plate, shoulder lamp and the class's extras. parts.bone_frame
    places it on another skeleton. `head` would size the headgear to another head (none for now)."""
    import harness_parts as hp

    frame_of = parts.bone_frame or bone_frame
    import harness_armour as ha

    # The plates curved to the torso, pauldrons, the fall-arrest webbing and the spine hardware.
    fit = ha.fitted(parts, spec, frame_of, boots)
    hp.spool(parts, spec, frame_of)
    hp.cutter(parts)
    hp.boot_hardware(parts, spec, boots)
    # PROPOSED: no headgear. The class specs call for helmets, hoods and the Climber's head wrap
    # (harness_parts.headgear builds them, sized to any head); they are left off for now.
    del head
    # The lease plate, riveted to the chest plate's lower left.
    chest = fit["chest"]
    p, n = chest.outer(chest.w / 2 - 0.06, chest.zc - chest.h / 2 + 0.04)
    lease = hp.bevelled_box("Lease", (0.004, 0.06, 0.03), width=0.0015)
    hp.place(lease, hp.frame(p + n * 0.002, n))
    parts.add(lease, "spine_03", "Accent")
    tag = hp.stamp("LeaseText", spec.get("lease", "CLEAR"), 0.011)
    hp.place(tag, hp.frame(p + n * 0.0042, n))
    parts.add(tag, "spine_03", "Stamp")
    # The shoulder lamp in team tint, on the left pauldron.
    p, n = fit["caps"]["l"].at(0.0, 0.35)
    lamp = hp.bevelled_box("ShoulderLamp", (0.026, 0.04, 0.03), width=0.006)
    hp.place(lamp, hp.frame(p + n * 0.012, n))
    parts.add(lamp, "upperarm_l", TEAM_SLOT)
    ha.share(lamp, "upperarm_l", "clavicle_l")
    extras(parts, spec, fit)


def build_mesh(name: str, spec: dict, mats: dict, rig):
    """The game mesh and the dense mesh its textures are baked from."""
    import harness_look as hl
    import harness_parts as hp

    parts = hp.Parts(mats)
    pieces = soft_pieces(name, spec, mats)
    figure = pieces["suit"]
    parts.set_body(figure)
    harness_parts_on(parts, spec, pieces["boots"])
    # The soft gear takes its weights from the surface under it: the suit, or its boot.
    sources = {"Suit": figure, **{pieces[f"boot_{s}"].name: pieces[f"boot_{s}"] for s in "lr"}}
    for obj in parts.skinned:
        src = sources.get(obj.get("skin_from", "Suit"), figure)
        W = transfer(src, pieces["weights"][src.name] if src.name in pieces["weights"] else weights_of(src), sdf.verts_of(obj))
        if obj.get("skin_bones"):  # Only these bones move it
            keep = np.zeros(len(DEFORM), bool)
            keep[[DEFORM.index(b) for b in obj["skin_bones"]]] = True
            W[:, ~keep] = 0.0
            W[W.sum(1) < 1e-6, DEFORM.index(obj["skin_bones"][0])] = 1.0
        set_weights(obj, W)
    # The dense mesh the soft parts are baked from (the hard parts are their own detail).
    for piece in pieces["low"]:
        sdf.set_attribute(piece, "soft", np.ones(len(piece.data.vertices)))
    soft = hl.join(f"HIGH_Soft_{name}", pieces["high"])
    low = hl.join(name, pieces["low"] + parts.rigid)
    return low, soft


def extras(parts, spec: dict, fit: dict) -> None:
    """Each class's own: the Scrapper's hip clutter, the Doubler's neon tape, the Climber's wraps."""
    import harness_parts as hp

    body = fit["body"]
    if spec.get("clutter"):  # Pouches, a coil of chain and a wrench, hung on the left hip
        for i, (dx, dz) in enumerate(((0.05, 0.96), (-0.055, 0.95), (0.0, 0.88))):
            loc, n = body.side(parts.torso_x(dz) + dx, dz, 1.0)
            if loc is None:
                continue
            pouch = hp.bevelled_box("Pouch", (0.045, 0.085, 0.1), width=0.012, segments=2)
            hp.place(pouch, hp.frame(loc + n * 0.028, n))
            parts.add(pouch, "pelvis", "Strap" if i else "Accent")
            flap = hp.bevelled_box("Flap", (0.05, 0.09, 0.03), width=0.008)
            hp.place(flap, hp.frame(loc + n * 0.03 + Vector((0, 0, 0.05)), n))
            parts.add(flap, "pelvis", "Body")
        loc, n = body.side(parts.torso_x(0.9) - 0.04, 0.9, 1.0)
        if loc is not None:
            m = hp.frame(loc + n * 0.02, n)
            for i in range(8):  # A coil of chain, link by link
                a = 2 * math.pi * i / 8
                link = ka.ring("Link", 0.011, 0.0036, (0, 0, 0), axis="Y" if i % 2 else "Z", segments=10, sides=6, stretch=0.011)
                hp.place(link, m @ Matrix.Translation((0.0, 0.045 * math.cos(a), 0.045 * math.sin(a))))
                parts.add(link, "pelvis", "Steel")
        loc, n = body.side(parts.torso_x(0.9) + 0.07, 0.9, 1.0)
        if loc is not None:
            wrench = hp.bevelled_box("Wrench", (0.012, 0.02, 0.2), width=0.004)
            hp.place(wrench, hp.frame(loc + n * 0.014, n) @ Matrix.Rotation(math.radians(12), 4, "X"))
            parts.add(wrench, "pelvis", "Steel")
    if spec.get("neon"):
        for s in (1.0, -1.0):  # The X of neon tape across the back, laid on the coverall
            ctl = [body.back(0.15 * s, 1.45)[0], body.back(0.0, 1.27)[0], body.back(-0.14 * s, 1.08)[0]]
            pts, nrm = body.hug([c for c in ctl if c is not None], 0.003)
            import harness_armour as ha
            ha.strap(parts, "Neon", pts, nrm, 0.025, 0.003, slot="Tape")
    if spec.get("wraps"):  # Three bands of cloth round each forearm above the Spool's place
        frame_of = bone_frame if parts.bone_frame is None else parts.bone_frame
        for side in ("l", "r"):
            head, tail = frame_of(f"lowerarm_{side}")
            axis = (tail - head).normalized()
            for t in (0.62, 0.71, 0.8):
                o = head + (tail - head) * t
                pts = parts.ring(o, axis, pad=0.003)
                wrap = hp.strap_path("Wrap", pts, 0.03, 0.004, outward=lambda p, o=o, a=axis: (p - o) - a * (p - o).dot(a),
                                     closed=True, smooth=1)
                parts.add(wrap, f"lowerarm_{side}", "Strap")


# Animation ------------------------------------------------------------------------------------

def pose(rig, angles: dict, pelvis_drop: float = 0.0) -> None:
    """Sets a pose from rotations about armature axes (degrees, X forward, Y left, Z up) in each
    bone's rest frame. Positive Y on a bone that points down swings it back; on one that points
    up, it leans it forward."""
    for pb in rig.pose.bones:
        pb.rotation_mode = "QUATERNION"
        pb.rotation_quaternion = Quaternion()
        pb.location = Vector()
    for bone, (ax, ay, az) in angles.items():
        pb = rig.pose.bones[bone]
        rest = pb.bone.matrix_local.to_3x3()
        world = (Matrix.Rotation(math.radians(az), 3, "Z") @ Matrix.Rotation(math.radians(ay), 3, "Y") @
                 Matrix.Rotation(math.radians(ax), 3, "X"))
        pb.rotation_quaternion = (rest.inverted() @ world @ rest).to_quaternion()
    if pelvis_drop:
        pb = rig.pose.bones["pelvis"]
        pb.location = pb.bone.matrix_local.to_3x3().inverted() @ Vector((0, 0, -pelvis_drop))


def key(rig, frame: int) -> None:
    for pb in rig.pose.bones:
        pb.keyframe_insert("rotation_quaternion", frame=frame)
        pb.keyframe_insert("location", frame=frame)


FPS = 30


def take(rig, name: str, seconds: float, at) -> dict:
    """A take: at(t) gives (angles, pelvis drop) for t in [0, seconds]; keyed every frame."""
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    rig.animation_data_create()
    rig.animation_data.action = action
    frames = int(round(seconds * FPS))
    for f in range(frames + 1):
        angles, drop = at(f / FPS)
        pose(rig, angles, drop)
        key(rig, f)
    return {"name": name, "seconds": seconds, "frames": frames}


def takes(rig, spec: dict) -> list[dict]:
    lean = spec["lean"]
    wide = 6.0 if spec.get("rake") else 3.0  # The Breaker's wide stance

    def idle(t):
        b = math.sin(2 * math.pi * t / 3.0)
        return ({"spine_02": (0, lean * 0.5 + 1.5 * b, 0), "upperarm_l": (4, 0, 0), "upperarm_r": (-4, 0, 0),
                 "lowerarm_l": (0, -10 - 2 * b, 0), "lowerarm_r": (0, -10 - 2 * b, 0),
                 "thigh_l": (-wide, 0, 0), "thigh_r": (wide, 0, 0)}, 0.0)

    def run(t):
        p = 2 * math.pi * t / 0.7
        s = math.sin(p)
        return ({"spine_01": (0, 8 + lean, 0), "spine_03": (0, 0, 8 * s),
                 "thigh_l": (0, -38 * s, 0), "thigh_r": (0, 38 * s, 0),
                 "calf_l": (0, 15 + 45 * max(0.0, -math.cos(p)), 0), "calf_r": (0, 15 + 45 * max(0.0, math.cos(p)), 0),
                 "upperarm_l": (6, 32 * s, 0), "upperarm_r": (-6, -32 * s, 0),
                 "lowerarm_l": (0, -60, 0), "lowerarm_r": (0, -60, 0)}, 0.03 * abs(math.cos(p)))

    def climb(t):
        s = math.sin(2 * math.pi * t / 1.2)
        return ({"upperarm_l": (0, -150 + 20 * s, 0), "upperarm_r": (0, -150 - 20 * s, 0),
                 "lowerarm_l": (0, -25, 0), "lowerarm_r": (0, -25, 0),
                 "thigh_l": (0, -45 - 25 * s, 0), "thigh_r": (0, -45 + 25 * s, 0),
                 "calf_l": (0, 70, 0), "calf_r": (0, 70, 0), "spine_01": (0, 5, 0)}, 0.0)

    def fall(t):
        f = math.sin(2 * math.pi * t / 0.5)
        return ({"upperarm_l": (75 + 8 * f, -20, 0), "upperarm_r": (-75 - 8 * f, -20, 0),
                 "lowerarm_l": (0, -30, 0), "lowerarm_r": (0, -30, 0),
                 "thigh_l": (-8, -25, 0), "thigh_r": (8, -10, 0), "calf_l": (0, 40, 0), "calf_r": (0, 25, 0)}, 0.0)

    def brace(t):
        # Rigger and Breaker (05 5.4): hips down, feet wide, a hand to the Spool.
        return ({"thigh_l": (-12 - wide, -45, 0), "thigh_r": (12 + wide, -45, 0), "calf_l": (0, 80, 0), "calf_r": (0, 80, 0),
                 "foot_l": (0, -35, 0), "foot_r": (0, -35, 0), "spine_01": (0, 22 + lean, 0),
                 "upperarm_r": (-10, -40, 0), "lowerarm_r": (0, -70, 0), "upperarm_l": (10, -30, 0), "lowerarm_l": (0, -50, 0)},
                0.25)

    def stagger(t):
        a = math.sin(math.pi * min(t / 0.6, 1.0))
        return ({"spine_01": (0, -20 * a, 0), "spine_03": (0, -10 * a, 6 * a), "upperarm_l": (40 * a, 10, 0),
                 "upperarm_r": (-40 * a, 10, 0), "thigh_l": (0, -15 * a, 0), "calf_l": (0, 25 * a, 0)}, 0.05 * a)

    return [take(rig, "Idle", 3.0, idle), take(rig, "Run", 0.7, run), take(rig, "Climb", 1.2, climb),
            take(rig, "Fall", 1.0, fall), take(rig, "Brace", 1.0, brace), take(rig, "Stagger", 0.6, stagger)]


CM = 100.0  # Blender metres to the FBX's centimetres


def _action_curves(action):
    """Every F-curve of an action (slotted actions keep them in channel bags)."""
    curves = list(getattr(action, "fcurves", []) or [])
    for layer in getattr(action, "layers", []):
        for strip in layer.strips:
            for bag in getattr(strip, "channelbags", []):
                curves += list(bag.fcurves)
    return curves


def _scale_scene(rig, mesh, k: float) -> None:
    """Scales the skeleton, the mesh and the takes' bone translations by k, in place."""
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode="EDIT")
    for eb in rig.data.edit_bones:
        eb.head *= k
        eb.tail *= k
    bpy.ops.object.mode_set(mode="OBJECT")
    mesh.data.transform(Matrix.Scale(k, 4))
    mesh.data.update()
    for action in bpy.data.actions:
        for fc in _action_curves(action):
            if fc.data_path.endswith(".location"):
                for kp in fc.keyframe_points:
                    kp.co.y *= k
                    kp.handle_left.y *= k
                    kp.handle_right.y *= k
                fc.update()
    for pb in rig.pose.bones:
        pb.location = pb.location * k


def export(path: str, rig, mesh) -> None:
    """The mesh, skeleton and takes as FBX in centimetres, Z up, the armature node at identity,
    so that in Unreal the root bone has unit scale and no rotation and every bone's translation
    is in centimetres (pelvis 95 cm above root along +Z), the character facing +X, its left on -Y.

    The scene is written as it is in Blender's own axes (forward Y, up Z: no axis conversion on
    the root objects) with the scene unit set to the centimetre and the data scaled by 100 for
    the export (then put back), so the FBX's unit scale is 1 and nothing has to be scaled on
    import. Before this, the metre-to-centimetre scale and the Y-up turn sat on the Armature node,
    which Unreal folds into the root bone (scale 100, a 90 degree roll)."""
    scene = bpy.context.scene
    unit = scene.unit_settings.scale_length
    frame = scene.frame_current
    _scale_scene(rig, mesh, CM)
    scene.unit_settings.scale_length = 1.0 / CM
    try:
        bpy.ops.object.select_all(action="DESELECT")
        rig.select_set(True)
        mesh.select_set(True)
        bpy.context.view_layer.objects.active = rig
        bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS",
                                 global_scale=1.0, axis_forward="Y", axis_up="Z", bake_space_transform=False,
                                 object_types={"ARMATURE", "MESH"}, mesh_smooth_type="FACE", add_leaf_bones=False,
                                 primary_bone_axis="Y", secondary_bone_axis="X", armature_nodetype="NULL",
                                 bake_anim=True, bake_anim_use_all_actions=True, bake_anim_use_nla_strips=False,
                                 bake_anim_force_startend_keying=True, bake_anim_simplify_factor=0.0, path_mode="AUTO")
    finally:
        scene.unit_settings.scale_length = unit
        _scale_scene(rig, mesh, 1.0 / CM)
        scene.frame_set(frame)


def build(cls: str, out_root: str, preview_only: bool, res: int = 4096) -> dict:
    import harness_look as hl

    spec = CLASSES[cls]
    ka.reset_scene()
    device = hl.use_gpu()
    bpy.context.scene.render.fps = FPS
    mats = materials(cls, spec)
    mats["Suit"] = hl.suit_fabric(f"M_{cls}_Suit", spec, _suit_layout(spec))
    rig = build_skeleton()
    name = f"SK_Harness_{cls}"
    mesh, soft = build_mesh(name, spec, mats, rig)
    for obj in (mesh, soft):  # The same triangles in both, so a hard part's copy shades as it does
        ka.triangulate(obj)
    out = os.path.join(out_root, cls)
    os.makedirs(out, exist_ok=True)
    # Bake the livery to textures: one set for everything but the team-tint parts, which keep a
    # flat slot of their own so the game can colour them per crew.
    team = [p.index for p in mesh.data.polygons if mesh.data.materials[p.material_index].name.endswith(TEAM_SLOT)]
    # One UV map for the whole sheet (the parts arrive with none, one, or several of their own).
    while mesh.data.uv_layers:
        mesh.data.uv_layers.remove(mesh.data.uv_layers[0])
    mesh.data.uv_layers.new(name="UVMap")
    ka.smart_uv(mesh, margin=0.002)
    tint = mats[TEAM_SLOT]
    stand_in = ka.flat_material(f"M_{cls}_TeamBake", "#E69F00", 0.3)
    for slot in mesh.material_slots:
        if slot.material == tint:
            slot.material = stand_in  # Baked as plain amber; it is not seen
    # Bake with the arms raised and the legs apart, so a hand or the Spool beside the hip does not
    # print onto the cloth under it: the dense mesh takes the game mesh's weights and both pose.
    set_weights(soft, transfer(mesh, weights_of(mesh), sdf.verts_of(soft)))
    for obj in (mesh, soft):
        mod = obj.modifiers.new("BakePose", "ARMATURE")
        mod.object = rig
    import harness_body as hb
    pose(rig, {"upperarm_l": (hb.APOSE + 5, 0, 0), "upperarm_r": (-hb.APOSE - 5, 0, 0), "thigh_l": (10, 0, 0), "thigh_r": (-10, 0, 0)})
    bpy.context.view_layer.update()
    maps = hl.bake(mesh, soft, out, f"T_{cls}", resolution=res, samples=16)
    mesh.modifiers.remove(mesh.modifiers["BakePose"])
    pose(rig, {})
    bpy.data.objects.remove(soft)
    baked = ka.baked_material(f"M_{cls}_Harness", maps)
    mesh.data.materials.clear()
    mesh.data.materials.append(baked)
    mesh.data.materials.append(tint)
    team = set(team)
    for p in mesh.data.polygons:
        p.material_index = 1 if p.index in team else 0
    mesh.parent = rig
    modifier = mesh.modifiers.new("Armature", "ARMATURE")
    modifier.object = rig
    clips = takes(rig, spec)
    rig.animation_data.action = bpy.data.actions["Idle"]
    bpy.context.scene.frame_set(0)
    if not preview_only:
        export(os.path.join(out, f"{name}.fbx"), rig, mesh)
    ys = [v.co.y for v in mesh.data.vertices if v.co.z > 1.3]
    report = {
        "name": name,
        "class": cls,
        "triangles": len(mesh.data.polygons),
        "height_m": round(max(v.co.z for v in mesh.data.vertices), 3),
        "shoulder_width_m": round(max(ys) - min(ys), 3),
        "shoulder_width_spec_m": spec["shoulders"],
        # Slot 0 reads the baked maps (Normal: tangent space, OpenGL, green up); slot 1 is the
        # team tint (Unreal: M_KineticHarness).
        "textures": {k: os.path.basename(v) for k, v in maps.items() if k in ("BaseColor", "ORM", "Normal", "Emissive")},
        "texture_size": res,
        "normal_map": "tangent space, OpenGL convention (green up): flip green in Unreal",
        "slots": [f"M_{cls}_Harness", f"M_{cls}_{TEAM_SLOT}"],
        "bones": [b[0] for b in BONES],
        "takes": clips,
        "run_speed_ms": spec["run_speed"],  # The Run take's stride matches this speed at play rate 1 (PROPOSED)
        "bake_device": device,
    }
    with open(os.path.join(out, "report.json"), "w") as f:
        json.dump(report, f, indent=2)
    ka.preview(os.path.join(out, f"{name}_preview.png"), (1.0, 1.0, 1.9), samples=48, resolution=(900, 1200))
    return report


def _line_exits(spec: dict) -> list:
    """Where the Spool's line leaves it, and its elbow end: the Doubler's heat scoring centres."""
    head_p, tail_p = bone_frame("lowerarm_r")
    axis = (tail_p - head_p).normalized()
    at = head_p + (tail_p - head_p) * spec["spool_at"]
    length = spec["spool"][0]
    return [tuple(at + axis * (length / 2 + 0.012)), tuple(at - axis * (length / 2))]


def _suit_layout(spec: dict) -> dict:
    import harness_body as hb
    return hb.Suit(spec, BONES).layout(BONES)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="art/characters/harness.py")
    parser.add_argument("--class", dest="cls", default=None, choices=list(CLASSES))
    parser.add_argument("--out", default=os.path.join(HERE, "..", "out", "characters"))
    parser.add_argument("--preview-only", action="store_true")
    parser.add_argument("--res", type=int, default=4096, help="texture size")
    parser.add_argument("--metahuman", action="store_true", help="the gear fitted to each class's MetaHuman (harness_mh.py)")
    args = parser.parse_args(argv)
    if args.metahuman:
        import harness_mh
        for cls in [args.cls] if args.cls else list(CLASSES):
            r = harness_mh.build(cls, args.res, preview=not args.preview_only)
            print(f"{r['name']}: {r['triangles']} triangles on {len(r['bones'])} bones, textures {sorted(r['textures'])}")
        return 0
    for cls in [args.cls] if args.cls else list(CLASSES):
        r = build(cls, os.path.abspath(args.out), args.preview_only, args.res)
        print(f"{r['name']}: {r['triangles']} triangles, {r['height_m']} m tall, shoulders {r['shoulder_width_m']} m "
              f"(spec {r['shoulder_width_spec_m']}), {len(r['takes'])} takes")
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    raise SystemExit(main(argv))
