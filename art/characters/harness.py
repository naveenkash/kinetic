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
        shoulders=0.48, bulk=0.85, chest=(0.26, 0.22), back=(0.24, 0.30), shoulder=(0.14, 0.12), oval=True,
        spool_at=0.9, spool=(0.09, 0.035), helmet=False, lean=0.0, frame="ribbons", run_speed=7.5,
        colours={"Plate": ("#D8DADC", 0.08, 1.0), "Frame": ("#9BA0A4", 0.4, 1.0), "Strap": ("#F4F1EA", 0.95, 0.0),
                 "Body": ("#E9E4D8", 0.9, 0.0), "Boot": ("#E9E4D8", 0.8, 0.0), "Spool": ("#C9CDD2", 0.35, 1.0),
                 "Accent": ("#A9C7E8", 0.5, 0.0)}),
    "Scrapper": dict(
        shoulders=0.54, bulk=0.95, chest=(0.28, 0.24), back=(0.30, 0.28), shoulder=(0.15, 0.13), shoulder_r=(0.13, 0.11),
        spool_at=0.55, spool=(0.14, 0.045), helmet=True, lean=6.0, frame="half", clutter=True, run_speed=7.0,
        colours={"Plate": ("#8C8F92", 0.6, 0.9), "Frame": ("#C8862B", 0.6, 0.3), "Strap": ("#7A3B22", 0.9, 0.0),
                 "Body": ("#3DA38C", 0.85, 0.0), "Boot": ("#4A3A2C", 0.7, 0.0), "Spool": ("#A67C3B", 0.4, 1.0),
                 "Accent": ("#E8C23A", 0.6, 0.0)}),
    "Rigger": dict(
        shoulders=0.62, bulk=1.0, chest=(0.30, 0.26), back=(0.32, 0.30), shoulder=(0.16, 0.14),
        spool_at=0.55, spool=(0.16, 0.05), helmet=True, lean=0.0, frame="rails", run_speed=6.2,
        colours={"Plate": ("#8C8578", 0.6, 0.9), "Frame": ("#C8862B", 0.6, 0.3), "Strap": ("#B9A98A", 0.9, 0.0),
                 "Body": ("#141211", 0.85, 0.0), "Boot": ("#4A3A2C", 0.7, 0.0), "Spool": ("#A67C3B", 0.35, 1.0),
                 "Accent": ("#A67C3B", 0.35, 1.0)}),
    "Doubler": dict(
        shoulders=0.62, bulk=1.0, chest=(0.30, 0.26), back=(0.32, 0.30), shoulder=(0.16, 0.14),
        spool_at=0.5, spool=(0.24, 0.055), twin=True, helmet=True, lean=0.0, frame="short", run_speed=6.2,
        colours={"Plate": ("#0E0E10", 0.5, 0.9), "Frame": ("#0E0E10", 0.5, 0.6), "Strap": ("#39FF14", 0.4, 0.0),
                 "Body": ("#0E0E10", 0.85, 0.0), "Boot": ("#0E0E10", 0.6, 0.0), "Spool": ("#4A2E1E", 0.45, 1.0),
                 "Accent": ("#FF2BD6", 0.4, 0.0)}),
    "Breaker": dict(
        shoulders=0.82, bulk=1.3, chest=(0.36, 0.30), back=(0.40, 0.36), shoulder=(0.22, 0.18), rake=15.0,
        spool_at=0.15, spool=(0.18, 0.075), helmet=True, lean=5.0, frame="shell", run_speed=5.0,
        colours={"Plate": ("#5A5E62", 0.45, 0.85), "Frame": ("#2B2E31", 0.7, 0.2), "Strap": ("#FF5A1F", 0.85, 0.0),
                 "Body": ("#2B2E31", 0.85, 0.0), "Boot": ("#2B2E31", 0.6, 0.0), "Spool": ("#5A5E62", 0.45, 0.85),
                 "Accent": ("#FF5A1F", 0.5, 0.0)}),
}
TEAM_SLOT = "TeamTint"  # The rim bands and the shoulder lamp: Unreal drives the colour


def bone_frame(name: str):
    head, tail = next((Vector(b[2]), Vector(b[3])) for b in BONES if b[0] == name)
    return head, tail


def part(objects: list, bone: str, obj) -> None:
    """Rigid skinning: every vertex of the part follows one bone."""
    group = obj.vertex_groups.new(name=bone)
    group.add(list(range(len(obj.data.vertices))), 1.0, "REPLACE")
    objects.append(obj)


def limb(objects, mats, bone: str, radius: float, slot: str, sides: int = 8, inset=(0.0, 0.0)):
    """A cylinder along a bone, from its head to its tail (shortened at either end by inset)."""
    head, tail = bone_frame(bone)
    axis = tail - head
    length = axis.length - inset[0] - inset[1]
    centre = head + axis.normalized() * (inset[0] + length / 2)
    obj = ka.cylinder(f"{bone}_limb", radius, length, (0, 0, 0), segments=sides)
    obj.data.materials.append(mats[slot])
    obj.matrix_world = Matrix.Translation(centre) @ axis.to_track_quat("Z", "Y").to_matrix().to_4x4()
    bpy.context.view_layer.update()
    obj.data.transform(obj.matrix_world)
    obj.matrix_world = Matrix.Identity(4)
    part(objects, bone, obj)


def block(objects, mats, bone: str, size, centre, slot: str, rotation=None):
    obj = ka.box(f"{bone}_{slot}_{len(objects)}", size, (0, 0, 0))
    obj.data.materials.append(mats[slot])
    m = Matrix.Translation(Vector(centre))
    if rotation is not None:
        m = m @ rotation
    obj.data.transform(m)
    part(objects, bone, obj)
    return obj


def plate(objects, mats, bone: str, size, centre, normal: Vector, slot: str = "Plate", oval: bool = False, rake: float = 0.0,
          outer_y: float | None = None):
    """A Catch Plate: the plate and its 1 cm team-tint rim band, facing along normal. With
    outer_y, it is moved across so its outermost edge, rim included, is there: the silhouette's
    shoulder width is the spec's exactly."""
    w, h = size
    rot = normal.to_track_quat("X", "Y" if abs(normal.z) > 0.9 else "Z").to_matrix().to_4x4()
    if rake:
        rot = rot @ Matrix.Rotation(math.radians(rake), 4, "X")
    if outer_y is not None:
        corners = [rot.to_3x3() @ Vector((x, y, z)) for x in (-0.019, 0.019) for y in (-w / 2 - 0.01, w / 2 + 0.01)
                   for z in (-h / 2 - 0.01, h / 2 + 0.01)]
        reach = max(c.y for c in corners) if outer_y > 0 else min(c.y for c in corners)
        centre = (centre[0], outer_y - reach, centre[2])
    if oval:
        obj = ka.cylinder(f"{bone}_oval", 0.5, 1.0, (0, 0, 0), axis="X", segments=12)
        obj.data.transform(Matrix.Diagonal((0.02, w, h, 1.0)))
        obj.data.materials.append(mats[slot])
        obj.data.transform(Matrix.Translation(Vector(centre)) @ rot)
        part(objects, bone, obj)
    else:
        block(objects, mats, bone, (0.025, w, h), centre, slot, rot)
    # The rim band: four 1 cm bars just proud of the plate's edge, in team tint.
    for bar, at in (((0.03, w + 0.02, 0.01), (0.004, 0.0, h / 2 + 0.005)), ((0.03, w + 0.02, 0.01), (0.004, 0.0, -h / 2 - 0.005)),
                    ((0.03, 0.01, h + 0.02), (0.004, w / 2 + 0.005, 0.0)), ((0.03, 0.01, h + 0.02), (0.004, -w / 2 - 0.005, 0.0))):
        obj = ka.box(f"{bone}_rim_{len(objects)}", bar, (0, 0, 0))
        obj.data.materials.append(mats[TEAM_SLOT])
        obj.data.transform(Matrix.Translation(Vector(centre)) @ rot @ Matrix.Translation(Vector(at)))
        part(objects, bone, obj)


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


def build_mesh(name: str, spec: dict, mats: dict):
    objects: list = []
    k = spec["bulk"]
    # The body under the harness: pelvis, the torso in three, neck and head.
    block(objects, mats, "pelvis", (0.22 * k, 0.32 * k, 0.2), (0, 0, 0.98), "Body")
    block(objects, mats, "spine_01", (0.2 * k, 0.30 * k, 0.14), (0, 0, 1.12), "Body")
    block(objects, mats, "spine_02", (0.22 * k, 0.33 * k, 0.15), (0, 0, 1.25), "Body")
    block(objects, mats, "spine_03", (0.24 * k, 0.36 * k, 0.16), (0, 0, 1.39), "Body")
    limb(objects, mats, "neck_01", 0.05 * min(k, 1.15), "Body")
    block(objects, mats, "head", (0.2, 0.17, 0.23), (0.01, 0, 1.67), "Body")
    if spec["helmet"]:
        block(objects, mats, "head", (0.23, 0.2, 0.09), (0.0, 0, 1.79), "Frame")
        block(objects, mats, "head", (0.03, 0.18, 0.05), (0.11, 0, 1.71), "Accent")  # Visor band
    else:
        block(objects, mats, "head", (0.21, 0.18, 0.06), (0.0, 0, 1.74), "Strap")  # A wrap
    # Arms and legs.
    for side, s in (("l", 1.0), ("r", -1.0)):
        limb(objects, mats, f"clavicle_{side}", 0.045 * k, "Body", inset=(0.02, 0.0))
        limb(objects, mats, f"upperarm_{side}", 0.052 * k, "Body")
        limb(objects, mats, f"lowerarm_{side}", 0.045 * k, "Strap")
        block(objects, mats, f"hand_{side}", (0.08, 0.04, 0.15), (0.0, 0.255 * s, 0.8), "Strap")
        limb(objects, mats, f"thigh_{side}", 0.075 * k, "Body")
        limb(objects, mats, f"calf_{side}", 0.06 * k, "Body", inset=(0.0, 0.05))
        boot_top = 0.5 if spec.get("rake") else 0.22  # The Breaker's knee-high boots
        block(objects, mats, f"calf_{side}", (0.13, 0.12, boot_top - 0.08), (0.0, 0.1 * s, 0.08 + (boot_top - 0.08) / 2), "Boot")
        block(objects, mats, f"foot_{side}", (0.26, 0.11, 0.08), (0.06, 0.1 * s, 0.04), "Boot")
    # Straps: the chest cross and the waist belt.
    block(objects, mats, "spine_02", (0.24 * k + 0.02, 0.34 * k + 0.02, 0.05), (0, 0, 1.2), "Strap")
    block(objects, mats, "pelvis", (0.23 * k + 0.02, 0.33 * k + 0.02, 0.06), (0, 0, 1.04), "Strap")
    # The four Catch Plates (never covered), on the chest, the back, and the shoulders.
    chest_x = 0.12 * k + 0.0125
    plate(objects, mats, "spine_03", spec["chest"], (chest_x, 0, 1.36), Vector((1, 0, 0)))
    back_z = 1.32 if spec.get("clutter") else 1.37  # The Scrapper's sits low
    plate(objects, mats, "spine_03", spec["back"], (-chest_x, 0, back_z), Vector((-1, 0, 0)), oval=False)
    half = spec["shoulders"] / 2
    for side, s in (("l", 1.0), ("r", -1.0)):
        size = spec["shoulder"] if side == "l" or "shoulder_r" not in spec else spec["shoulder_r"]
        normal = Vector((0, 0.35 * s, 1)).normalized()
        plate(objects, mats, f"clavicle_{side}", size, (0, 0, 1.5), normal, oval=spec.get("oval", False),
              rake=spec.get("rake", 0.0) * s, outer_y=half * s)
    # The spine frame and its gel reservoirs (the Rigger's rails, the Doubler's cut-down frame).
    frame = spec["frame"]
    if frame in ("rails", "short", "half"):
        length = {"rails": 0.6, "short": 0.45, "half": 0.6}[frame]
        rails = (0.07,) if frame == "half" else (-0.07, 0.07)
        for y in rails:
            block(objects, mats, "spine_02", (0.025, 0.025, length), (-chest_x - 0.03, y, 1.08 + length / 2), "Frame")
        for z in (1.12, 1.3, 1.48)[: 2 if frame == "short" else 3]:
            block(objects, mats, "spine_02", (0.025, 0.16, 0.025), (-chest_x - 0.03, 0, z), "Frame")
        for y in rails:
            gel = ka.cylinder("gel", 0.035, 0.22, (0, 0, 0), segments=10)
            gel.data.materials.append(mats["Accent"])
            gel.data.transform(Matrix.Translation((-chest_x - 0.06, y * 1.6, 1.12)))
            part(objects, "spine_02", gel)
    elif frame == "shell":
        block(objects, mats, "spine_02", (0.04, 0.4 * k, 0.4), (-chest_x - 0.01, 0, 1.25), "Frame")  # The ribbed shell
    # The Spool on the right forearm, at its class place.
    head, tail = bone_frame("lowerarm_r")
    axis = (tail - head).normalized()
    at = head + (tail - head) * spec["spool_at"]
    length, radius = spec["spool"]
    drums = (0.0, 0.07) if spec.get("twin") else (0.0,)
    for off in drums:
        drum = ka.cylinder("spool", radius, length, (0, 0, 0), segments=12)
        drum.data.materials.append(mats["Spool"])
        drum.data.transform(Matrix.Translation(at + Vector((0.035 * (1 if off else 0), 0, 0)) - axis * off) @
                            axis.to_track_quat("Z", "Y").to_matrix().to_4x4())
        part(objects, "lowerarm_r", drum)
    ring = ka.cylinder("spool_rim", radius + 0.006, 0.012, (0, 0, 0), segments=12)
    ring.data.materials.append(mats[TEAM_SLOT])
    ring.data.transform(Matrix.Translation(at + axis * (length / 2)) @ axis.to_track_quat("Z", "Y").to_matrix().to_4x4())
    part(objects, "lowerarm_r", ring)
    if spec.get("twin"):  # The snap guard, a flared bell at the elbow
        bell = ka.cylinder("bell", 0.07, 0.05, (0, 0, 0), segments=12)
        bell.data.materials.append(mats["Spool"])
        bell.data.transform(Matrix.Translation(head + axis * 0.03) @ axis.to_track_quat("Z", "Y").to_matrix().to_4x4())
        part(objects, "lowerarm_r", bell)
    # The Scrapper's clutter, low on the left hip: pouches and a coil of chain.
    if spec.get("clutter"):
        for i, (dx, dz) in enumerate(((0.06, 0.98), (-0.04, 0.96), (0.0, 0.9))):
            block(objects, mats, "pelvis", (0.09, 0.07, 0.1), (dx, 0.2, dz), "Strap" if i else "Accent")
        coil = ka.cylinder("coil", 0.08, 0.05, (0, 0, 0), axis="Y", segments=12)
        coil.data.materials.append(mats["Frame"])
        coil.data.transform(Matrix.Translation((-0.06, 0.22, 0.92)))
        part(objects, "pelvis", coil)
    # The team-tint shoulder lamp.
    block(objects, mats, "clavicle_l", (0.04, 0.04, 0.03), (0.05, half * 0.7, 1.55), TEAM_SLOT)
    mesh = ka.join(name, objects)
    return mesh


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


def build(cls: str, out_root: str, preview_only: bool) -> dict:
    spec = CLASSES[cls]
    ka.reset_scene()
    bpy.context.scene.render.fps = FPS
    mats = {slot: ka.flat_material(f"M_{cls}_{slot}", *values) for slot, values in spec["colours"].items()}
    tint = "#E69F00"  # Amber in the preview; Unreal sets each crew's tint
    mats[TEAM_SLOT] = ka.emissive(f"M_{cls}_{TEAM_SLOT}", tint, 6.0)
    rig = build_skeleton()
    name = f"SK_Harness_{cls}"
    mesh = build_mesh(name, spec, mats)
    ka.triangulate(mesh)
    mesh.parent = rig
    modifier = mesh.modifiers.new("Armature", "ARMATURE")
    modifier.object = rig
    out = os.path.join(out_root, cls)
    os.makedirs(out, exist_ok=True)
    clips = takes(rig, spec)
    rig.animation_data.action = bpy.data.actions["Idle"]
    bpy.context.scene.frame_set(0)
    if not preview_only:
        export(os.path.join(out, f"{name}.fbx"), rig, mesh)
    # The silhouette, measured: the widest point across the shoulders, in the rest pose.
    ys = [v.co.y for v in mesh.data.vertices if v.co.z > 1.3]
    report = {
        "name": name,
        "class": cls,
        "triangles": len(mesh.data.polygons),
        "height_m": round(max(v.co.z for v in mesh.data.vertices), 3),
        "shoulder_width_m": round(max(ys) - min(ys), 3),
        "shoulder_width_spec_m": spec["shoulders"],
        "materials": {slot.removeprefix(f"M_{cls}_"): {"color": spec["colours"].get(slot.removeprefix(f"M_{cls}_"), (tint, 0.5, 0.0))[0],
                                                       "roughness": spec["colours"].get(slot.removeprefix(f"M_{cls}_"), (tint, 0.5, 0.0))[1],
                                                       "metallic": spec["colours"].get(slot.removeprefix(f"M_{cls}_"), (tint, 0.5, 0.0))[2],
                                                       "team_tint": slot.endswith(TEAM_SLOT)}
                      for slot in [m.name for m in mesh.data.materials]},
        "bones": [b[0] for b in BONES],
        "takes": clips,
        "run_speed_ms": spec["run_speed"],  # The Run take's stride matches this speed at play rate 1 (PROPOSED)
    }
    with open(os.path.join(out, "report.json"), "w") as f:
        json.dump(report, f, indent=2)
    ka.preview(os.path.join(out, f"{name}_preview.png"), (1.0, 1.0, 1.9), samples=32, resolution=(800, 1000))
    return report


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="art/characters/harness.py")
    parser.add_argument("--class", dest="cls", default=None, choices=list(CLASSES))
    parser.add_argument("--out", default=os.path.join(HERE, "..", "out", "characters"))
    parser.add_argument("--preview-only", action="store_true")
    args = parser.parse_args(argv)
    for cls in [args.cls] if args.cls else list(CLASSES):
        r = build(cls, os.path.abspath(args.out), args.preview_only)
        print(f"{r['name']}: {r['triangles']} triangles, {r['height_m']} m tall, shoulders {r['shoulder_width_m']} m "
              f"(spec {r['shoulder_width_spec_m']}), {len(r['takes'])} takes")
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    raise SystemExit(main(argv))
