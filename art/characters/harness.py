"""Builds the five harness characters (docs/characters/harnesses/*.md, docs/characters/README.md)
as skinned meshes on one shared skeleton, with their animations, from code:

    blender -b --factory-startup -P art/characters/harness.py -- [--class Rigger] [--out art/out/characters]
        [--preview-only]

For each class it writes art/out/characters/<Class>/: SK_Harness_<Class>.fbx (the mesh, the
skeleton and every animation as its own take), a preview render, and report.json (triangles,
the material slots with their colours, the silhouette's shoulder width, the takes and their
lengths, the bones).

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
- the faction palettes for plates, frame, straps and boots.

What is PROPOSED here: a low-poly grey-box figure (a few thousand triangles against the
85k-triangle LOD0 in the specs), rigid skinning (each part follows one bone), no sockets on the
mesh (the game places the plates from code), and six procedural takes: Idle, Run, Climb, Fall,
Brace and Stagger, keyed as rotations of the rest pose.
"""
from __future__ import annotations

import argparse
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "..", "lib")]

import bpy  # noqa: E402
from mathutils import Matrix, Quaternion, Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402

# Bones: name, parent, head, tail (metres; X forward, Y the character's left, Z up).
BONES = [
    ("root", None, (0, 0, 0), (0, 0, 0.1)),
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

# Per class: silhouette (shoulder width with plates, m), body bulk, plates (chest, back,
# shoulder; width, height in m), the Spool's place on the right forearm (0 elbow, 1 wrist), its
# length and radius, and the livery: (hex, roughness, metallic) per material slot.
CLASSES = {
    "Climber": dict(
        shoulders=0.48, bulk=0.85, limb=0.92, chest=(0.26, 0.22), back=(0.24, 0.30), shoulder=(0.14, 0.12), oval=True,
        plate_style="mirror", spool_at=0.88, spool=(0.09, 0.03), headgear="wrap", frame="ribbons", wraps=True, lean=0.0,
        boot_top=0.14, steel_toe=False, strap_width=0.03, tape=("#C9CDD2", "#F4F1EA"), helmet_colour="#F4F1EA",
        skin="#C69C7E", number="31.4", lease="SPONSORED", run_speed=7.5,
        colours={"Plate": ("#D8DADC", 0.08, 1.0), "Frame": ("#9BA0A4", 0.4, 1.0), "Strap": ("#F4F1EA", 0.95, 0.0),
                 "Body": ("#E9E4D8", 0.9, 0.0), "Boot": ("#E9E4D8", 0.8, 0.0), "Spool": ("#C9CDD2", 0.35, 1.0),
                 "Accent": ("#C9CDD2", 0.4, 1.0)}),
    "Scrapper": dict(
        shoulders=0.54, bulk=0.95, chest=(0.28, 0.24), back=(0.30, 0.28), shoulder=(0.15, 0.13), shoulder_r=(0.13, 0.11),
        plate_style="salvage", spool_at=0.55, spool=(0.16, 0.045), capacitor=True, headgear="salvage", frame="half",
        clutter=True, lean=6.0, boot_top=0.22, laces=True, strap_width=0.045, tape=("#E8C23A", "#3DA38C"),
        helmet_colour="#3DA38C", skin="#8D5A3B", number="K-77", lease="KESSL 2201", run_speed=7.0,
        colours={"Plate": ("#7E8A86", 0.6, 0.9), "Frame": ("#C8862B", 0.6, 0.3), "Strap": ("#7A3B22", 0.9, 0.0),
                 "Body": ("#3A4A45", 0.85, 0.0), "Boot": ("#4A3A2C", 0.7, 0.0), "Spool": ("#A67C3B", 0.4, 1.0),
                 "Accent": ("#E8C23A", 0.6, 0.0)}),
    "Rigger": dict(
        shoulders=0.62, bulk=1.0, chest=(0.30, 0.26), back=(0.32, 0.30), shoulder=(0.16, 0.14), plate_style="lip",
        spool_at=0.55, spool=(0.16, 0.05), headgear="square", frame="rails", lean=0.0, boot_top=0.2, laces=True,
        strap_width=0.05, tape=("#C8862B", "#141211"), helmet_colour="#C8862B", skin="#A8785A", number="L-0412",
        lease="CLEAR", run_speed=6.2,
        colours={"Plate": ("#7E786C", 0.6, 0.9), "Frame": ("#C8862B", 0.6, 0.3), "Strap": ("#B9A98A", 0.9, 0.0),
                 "Body": ("#2A2622", 0.85, 0.0), "Boot": ("#4A3A2C", 0.7, 0.0), "Spool": ("#A67C3B", 0.35, 1.0),
                 "Accent": ("#A67C3B", 0.35, 1.0)}),
    "Doubler": dict(
        shoulders=0.62, bulk=1.0, chest=(0.30, 0.26), back=(0.32, 0.30), shoulder=(0.16, 0.14), plate_style="lip",
        spool_at=0.5, spool=(0.28, 0.055), twin=True, headgear="hood", frame="short", neon=True, lean=0.0, boot_top=0.24,
        laces=True, strap_width=0.045, tape=("#39FF14", "#FF2BD6"), helmet_colour="#0E0E10", skin="#B98A6A",
        number="RED 9", lease="", run_speed=6.2,
        colours={"Plate": ("#16161A", 0.5, 0.9), "Frame": ("#16161A", 0.5, 0.6), "Strap": ("#1A1A1E", 0.6, 0.0),
                 "Body": ("#0E0E10", 0.85, 0.0), "Boot": ("#0E0E10", 0.6, 0.0), "Spool": ("#6A5A4A", 0.45, 1.0),
                 "Accent": ("#FFE600", 0.4, 0.0)}),
    "Breaker": dict(
        shoulders=0.82, bulk=1.3, limb=1.1, chest=(0.36, 0.30), back=(0.40, 0.36), shoulder=(0.22, 0.18), rake=15.0,
        plate_style="chamfer", spool_at=0.12, spool=(0.22, 0.07), gauntlet=True, headgear="visor", frame="shell",
        lean=5.0, boot_top=0.5, shin_shell=True, strap_width=0.07, tape=("#FF5A1F", "#2B2E31"), helmet_colour="#FF5A1F",
        skin="#7A4E36", number="HTR 118", lease="KESSL 9930", run_speed=5.0,
        colours={"Plate": ("#5A5E62", 0.45, 0.85), "Frame": ("#2B2E31", 0.7, 0.2), "Strap": ("#FF5A1F", 0.85, 0.0),
                 "Body": ("#2B2E31", 0.85, 0.0), "Boot": ("#2B2E31", 0.6, 0.0), "Spool": ("#5A5E62", 0.45, 0.85),
                 "Accent": ("#FF5A1F", 0.5, 0.0)}),
}
TEAM_SLOT = "TeamTint"  # The rim bands and the shoulder lamp: Unreal drives the colour


def bone_frame(name: str):
    head, tail = next((Vector(b[2]), Vector(b[3])) for b in BONES if b[0] == name)
    return head, tail


def materials(cls: str, spec: dict) -> dict:
    """The class's livery as procedural materials (the faction palettes in characters/README.md):
    worn paint, canvas and leather, brass, mirror steel and anodised aluminium, skin. They are
    baked to textures before export."""
    c = spec["colours"]
    wear = lambda paint, chips=0.35: ka.Wear(paint=paint, chips=chips, rust_streaks=0.08, scrape_height=0.0,
                                             streak_band=(1.0, 1.9), roof_dirt=None, lower_grime="#2B2622")
    style = spec["plate_style"]
    plate = {
        "mirror": lambda: ka.flat_material(f"M_{cls}_Plate", c["Plate"][0], 0.08, 1.0),
        "chamfer": lambda: ka.flat_material(f"M_{cls}_Plate", c["Plate"][0], 0.45, 0.85),
    }.get(style, lambda: ka.painted_steel(f"M_{cls}_Plate", wear(c["Plate"][0], 0.55 if style == "salvage" else 0.35)))()
    m = {
        "Plate": plate,
        "Frame": ka.painted_steel(f"M_{cls}_Frame", wear(c["Frame"][0], 0.5)),
        "Strap": ka.fabric(f"M_{cls}_Strap", c["Strap"][0]),
        "Body": ka.fabric(f"M_{cls}_Body", c["Body"][0]),
        "Boot": ka.flat_material(f"M_{cls}_Boot", c["Boot"][0], 0.65),
        "Sole": ka.flat_material(f"M_{cls}_Sole", "#1A1A1A", 0.9),
        "Spool": ka.flat_material(f"M_{cls}_Spool", c["Spool"][0], c["Spool"][1], c["Spool"][2]),
        "Accent": ka.flat_material(f"M_{cls}_Accent", c["Accent"][0], c["Accent"][1], c["Accent"][2]),
        "Steel": ka.flat_material(f"M_{cls}_Steel", "#9EA3A6", 0.35, 1.0),
        "Stamp": ka.flat_material(f"M_{cls}_Stamp", "#B8B4AC", 0.4, 1.0),
        "Ceramic": ka.flat_material(f"M_{cls}_Ceramic", "#E8E6E0", 0.3),
        "Tape": ka.flat_material(f"M_{cls}_Tape", spec["tape"][0], 0.6),
        "TapeB": ka.flat_material(f"M_{cls}_TapeB", spec["tape"][1], 0.6),
        "Seal": ka.flat_material(f"M_{cls}_Seal", "#F2F2F0", 0.5),
        "Helmet": ka.painted_steel(f"M_{cls}_Helmet", wear(spec["helmet_colour"], 0.3)),
        "Visor": ka.flat_material(f"M_{cls}_Visor", "#101418", 0.05, 0.5),
        "Gel": ka.flat_material(f"M_{cls}_Gel", "#E8D9B0", 0.1),
        "Skin": ka.flat_material(f"M_{cls}_Skin", spec["skin"], 0.55),
        "Dark": ka.flat_material(f"M_{cls}_Dark", "#0B0B0C", 0.2),
        TEAM_SLOT: ka.emissive(f"M_{cls}_{TEAM_SLOT}", "#E69F00", 6.0),
    }
    return m


def build_skeleton() -> bpy.types.Object:
    data = bpy.data.armatures.new("Armature")
    rig = bpy.data.objects.new("Armature", data)  # Unreal drops a node named Armature
    bpy.context.scene.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode="EDIT")
    for name, parent, head, tail in BONES:
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


def build_mesh(name: str, spec: dict, mats: dict, rig):
    import harness_parts as hp

    parts = hp.Parts(mats)
    figure = hp.body(parts, spec)
    weight_body(figure, rig)
    hp.head(parts)
    # Where the torso's surface is, measured, so the plates sit on it.
    band = [v.co for v in figure.data.vertices if 1.3 < v.co.z < 1.4 and abs(v.co.y) < 0.06]
    chest_x = max(c.x for c in band) + 0.011  # The plate's back face on the chest
    back_x = min(c.x for c in band) - 0.011
    shoulder_z = max(v.co.z for v in figure.data.vertices if 0.12 < abs(v.co.y) < 0.2 and v.co.z < 1.55) + 0.012
    style = spec["plate_style"]
    hp.catch_plate(parts, "spine_03", spec["chest"], (chest_x, 0, 1.35), (1, 0, 0), style, number=spec.get("number"))
    hp.catch_plate(parts, "spine_03", spec["back"], (back_x, 0, 1.32 if spec.get("clutter") else 1.36), (-1, 0, 0), style)
    half = spec["shoulders"] / 2
    for side, s in (("l", 1.0), ("r", -1.0)):
        size = spec["shoulder"] if side == "l" or "shoulder_r" not in spec else spec["shoulder_r"]
        normal = Vector((0, 0.35 * s, 1)).normalized()
        hp.catch_plate(parts, f"clavicle_{side}", size, (0, 0, shoulder_z), tuple(normal), "mirror" if spec.get("oval") else style,
                       rake=spec.get("rake", 0.0) * s, outer_y=half * s, bolts=0 if spec.get("oval") else 2)
    hp.straps(parts, spec, chest_x - 0.012)
    hp.spine_frame(parts, spec, back_x + 0.012)
    hp.spool(parts, spec, bone_frame)
    hp.cutter(parts)
    hp.boots(parts, spec)
    hp.headgear(parts, spec)
    # The telemetry pack under the back plate with its Office seal, and the lease plate.
    pack = hp.bevelled_box("Telemetry", (0.04, 0.14, 0.1), width=0.006)
    hp.place(pack, Matrix.Translation((back_x - 0.02, 0.0, 1.2)))
    parts.add(pack, "spine_02", "Frame" if spec["frame"] == "shell" else "Dark")
    seal = hp.cylinder("TelemetrySeal", 0.015, 0.004, segments=16)
    hp.place(seal, hp.frame((back_x - 0.042, 0.0, 1.2), (-1, 0, 0)))
    parts.add(seal, "spine_02", "Seal")
    lease = hp.bevelled_box("Lease", (0.004, 0.06, 0.03), width=0.0015)
    hp.place(lease, Matrix.Translation((chest_x + 0.014, 0.1, 1.25)))
    parts.add(lease, "spine_03", "Accent")
    tag = hp.stamp("LeaseText", spec.get("lease", "CLEAR"), 0.012)
    hp.place(tag, Matrix.Translation((chest_x + 0.017, 0.1, 1.25)))
    parts.add(tag, "spine_03", "Stamp")
    # The shoulder lamp in team tint.
    lamp = hp.bevelled_box("ShoulderLamp", (0.04, 0.035, 0.03), width=0.006)
    hp.place(lamp, Matrix.Translation((0.06, half * 0.62, 1.56)))
    parts.add(lamp, "clavicle_l", TEAM_SLOT)
    extras(parts, spec, chest_x, back_x)
    return ka.join(name, [figure] + parts.rigid)


def extras(parts, spec: dict, chest_x: float, back_x: float) -> None:
    """Each class's own: the Scrapper's hip clutter, the Doubler's neon tape, the Climber's wraps."""
    import harness_parts as hp

    if spec.get("clutter"):
        for i, (dx, dz) in enumerate(((0.06, 0.98), (-0.05, 0.96), (0.005, 0.9))):
            pouch = hp.bevelled_box("Pouch", (0.085, 0.07, 0.1), width=0.015, segments=2)
            hp.place(pouch, Matrix.Translation((dx, 0.2, dz)))
            parts.add(pouch, "pelvis", "Strap" if i else "Accent")
            flap = hp.bevelled_box("Flap", (0.09, 0.075, 0.03), width=0.008)
            hp.place(flap, Matrix.Translation((dx, 0.2, dz + 0.05)))
            parts.add(flap, "pelvis", "Body")
        for i in range(8):  # A coil of chain, link by link
            a = 2 * math.pi * i / 8
            link = ka.ring("Link", 0.012, 0.004, (0, 0, 0), axis="Y" if i % 2 else "X", segments=10, sides=6, stretch=0.012)
            hp.place(link, Matrix.Translation((-0.06 + 0.055 * math.cos(a), 0.225, 0.92 + 0.055 * math.sin(a))))
            parts.add(link, "pelvis", "Steel")
        wrench = hp.bevelled_box("Wrench", (0.02, 0.012, 0.2), width=0.004)
        hp.place(wrench, Matrix.Translation((0.08, 0.19, 0.88)) @ Matrix.Rotation(math.radians(12), 4, "X"))
        parts.add(wrench, "pelvis", "Steel")
    if spec.get("neon"):
        for s in (1.0, -1.0):  # The X of neon tape across the back
            pts = [(back_x - 0.005, 0.15 * s, 1.45), (back_x - 0.012, 0.0, 1.27), (back_x - 0.005, -0.14 * s, 1.08)]
            parts.add(hp.strap_path("Neon", pts, 0.025, 0.003, outward=lambda p: Vector((-1, 0, 0))), "spine_02", "Tape")
    if spec.get("wraps"):
        for side, s in (("l", 1.0), ("r", -1.0)):
            for z in (0.92, 0.97, 1.02):
                wx, wy = parts.fit(0.004, 0.245 * s, z, pad=0.003, reach=0.08)
                wrap = hp.strap_path("Wrap", hp.ellipse(0.004, 0.245 * s, z, wx, wy, 18), 0.03, 0.004,
                                     outward=lambda p, c=0.245 * s: Vector((p.x - 0.004, p.y - c, 0)), closed=True)
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


def export(path: str, rig, mesh) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    rig.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS",
                             object_types={"ARMATURE", "MESH"}, mesh_smooth_type="FACE", add_leaf_bones=False,
                             primary_bone_axis="Y", secondary_bone_axis="X", armature_nodetype="NULL",
                             bake_anim=True, bake_anim_use_all_actions=True, bake_anim_use_nla_strips=False,
                             bake_anim_force_startend_keying=True, bake_anim_simplify_factor=0.0, path_mode="AUTO")


def build(cls: str, out_root: str, preview_only: bool, res: int = 2048) -> dict:
    spec = CLASSES[cls]
    ka.reset_scene()
    bpy.context.scene.render.fps = FPS
    mats = materials(cls, spec)
    rig = build_skeleton()
    name = f"SK_Harness_{cls}"
    mesh = build_mesh(name, spec, mats, rig)
    ka.triangulate(mesh)
    out = os.path.join(out_root, cls)
    os.makedirs(out, exist_ok=True)
    # Bake the livery to textures: one set for everything but the team-tint parts, which keep a
    # flat slot of their own so the game can colour them per crew.
    team = [p.index for p in mesh.data.polygons if mesh.data.materials[p.material_index].name.endswith(TEAM_SLOT)]
    ka.smart_uv(mesh, margin=0.003)
    tint = mats[TEAM_SLOT]
    stand_in = ka.flat_material(f"M_{cls}_TeamBake", "#E69F00", 0.3)
    for slot in mesh.material_slots:
        if slot.material == tint:
            slot.material = stand_in  # Baked as plain amber; it is not seen
    maps = ka.bake(mesh, out, f"T_{cls}", resolution=res, samples=16)
    baked = ka.baked_material(f"M_{cls}_Harness", maps)
    mesh.data.materials.clear()
    mesh.data.materials.append(baked)
    mesh.data.materials.append(tint)
    for p in mesh.data.polygons:
        p.material_index = 1 if p.index in set(team) else 0
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
        # Slot 0 reads the baked maps; slot 1 is the team tint (Unreal: M_KineticHarness).
        "textures": {k: os.path.basename(v) for k, v in maps.items() if k in ("BaseColor", "ORM", "Emissive")},
        "slots": [f"M_{cls}_Harness", f"M_{cls}_{TEAM_SLOT}"],
        "bones": [b[0] for b in BONES],
        "takes": clips,
        "run_speed_ms": spec["run_speed"],  # The Run take's stride matches this speed at play rate 1 (PROPOSED)
    }
    with open(os.path.join(out, "report.json"), "w") as f:
        json.dump(report, f, indent=2)
    ka.preview(os.path.join(out, f"{name}_preview.png"), (1.0, 1.0, 1.9), samples=48, resolution=(900, 1200))
    return report


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="art/characters/harness.py")
    parser.add_argument("--class", dest="cls", default=None, choices=list(CLASSES))
    parser.add_argument("--out", default=os.path.join(HERE, "..", "out", "characters"))
    parser.add_argument("--preview-only", action="store_true")
    parser.add_argument("--res", type=int, default=2048, help="texture size")
    args = parser.parse_args(argv)
    sys.path.insert(0, HERE)
    for cls in [args.cls] if args.cls else list(CLASSES):
        r = build(cls, os.path.abspath(args.out), args.preview_only, args.res)
        print(f"{r['name']}: {r['triangles']} triangles, {r['height_m']} m tall, shoulders {r['shoulder_width_m']} m "
              f"(spec {r['shoulder_width_spec_m']}), {len(r['takes'])} takes")
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    raise SystemExit(main(argv))
