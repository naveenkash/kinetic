"""Shared helpers for KINETIC's generated assets (art/README.md).

Every asset is a Python script that builds its meshes from its spec in docs/objects, gives them
procedural materials, bakes those to textures, and exports FBX for Unreal and GLB for the browser
client. Runs headless: blender -b --factory-startup -P art/build.py -- <asset>.

Conventions (docs/objects/README-props.md): metres, Z up, X+ toward the object's front, pivot at
the bottom centre of the footprint. Unreal static-mesh sockets are empties named SOCKET_<name>;
simple collision is a UCX_<mesh>_<nn> box.
"""
from __future__ import annotations

import math
import os
from dataclasses import dataclass, field

import bmesh
import bpy
from mathutils import Vector


# Scene -----------------------------------------------------------------------------------------

def reset_scene() -> None:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = 48
    scene.cycles.use_denoising = True


def hex_rgb(hex_color: str) -> tuple[float, float, float, float]:
    """sRGB hex to linear RGBA, as Blender's color inputs expect."""
    h = hex_color.lstrip("#")
    srgb = [int(h[i:i + 2], 16) / 255.0 for i in (0, 2, 4)]
    lin = [c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4 for c in srgb]
    return (lin[0], lin[1], lin[2], 1.0)


# Geometry --------------------------------------------------------------------------------------

def mesh_object(name: str, bm: bmesh.types.BMesh) -> bpy.types.Object:
    mesh = bpy.data.meshes.new(name)
    bm.to_mesh(mesh)
    bm.free()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def box(name: str, size: tuple[float, float, float], center: tuple[float, float, float],
        material: bpy.types.Material | None = None) -> bpy.types.Object:
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector((v.co.x * size[0] + center[0], v.co.y * size[1] + center[1], v.co.z * size[2] + center[2]))
    obj = mesh_object(name, bm)
    if material:
        obj.data.materials.append(material)
    return obj


def cylinder(name: str, radius: float, depth: float, center: tuple[float, float, float], axis: str = "Z",
             segments: int = 12, material: bpy.types.Material | None = None) -> bpy.types.Object:
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=True, segments=segments, radius1=radius, radius2=radius, depth=depth)
    rot = {"Z": None, "X": ("Y", math.pi / 2), "Y": ("X", math.pi / 2)}[axis]
    if rot:
        from mathutils import Matrix
        bmesh.ops.rotate(bm, verts=bm.verts, cent=(0, 0, 0), matrix=Matrix.Rotation(rot[1], 3, rot[0]))
    bmesh.ops.translate(bm, verts=bm.verts, vec=Vector(center))
    obj = mesh_object(name, bm)
    if material:
        obj.data.materials.append(material)
    return obj


def corrugated_panel(name: str, length: float, height: float, pitch: float, depth: float,
                     material: bpy.types.Material | None = None) -> bpy.types.Object:
    """A trapezoid-corrugated sheet in its local XZ plane, ribs along Z, pressed toward -Y by
    `depth`. Length along X is centred on 0; height runs from z = 0. One-sided, facing +Y."""
    flat = pitch * 0.35
    slope = (pitch - 2 * flat) / 2
    profile: list[tuple[float, float]] = []
    x = -length / 2
    profile.append((x, 0.0))
    while x < length / 2 - 1e-6:
        for dx, dy in ((flat, 0.0), (slope, -depth), (flat, -depth), (slope, 0.0)):
            x = min(x + dx, length / 2)
            profile.append((x, dy))
            if x >= length / 2 - 1e-6:
                break
    bm = bmesh.new()
    bottom = [bm.verts.new((px, py, 0.0)) for px, py in profile]
    top = [bm.verts.new((px, py, height)) for px, py in profile]
    for i in range(len(profile) - 1):
        bm.faces.new((bottom[i], bottom[i + 1], top[i + 1], top[i]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    # Make sure the panel faces +Y.
    if sum(f.normal.y for f in bm.faces) < 0:
        bmesh.ops.reverse_faces(bm, faces=bm.faces)
    obj = mesh_object(name, bm)
    if material:
        obj.data.materials.append(material)
    return obj


def transform(obj: bpy.types.Object, location=(0, 0, 0), rotation_z: float = 0.0, rotation_x: float = 0.0) -> None:
    obj.rotation_euler = (rotation_x, 0.0, rotation_z)
    obj.location = location
    bpy.context.view_layer.update()
    obj.data.transform(obj.matrix_world)
    obj.matrix_world.identity()


def add_bevel(obj: bpy.types.Object, width: float = 0.006, segments: int = 1) -> None:
    mod = obj.modifiers.new("Bevel", "BEVEL")
    mod.width = width
    mod.segments = segments
    mod.limit_method = "ANGLE"
    mod.angle_limit = math.radians(40)
    mod.harden_normals = True


def join(name: str, objects: list[bpy.types.Object]) -> bpy.types.Object:
    for o in objects:
        bpy.context.view_layer.objects.active = o
        for m in list(o.modifiers):
            bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.ops.object.select_all(action="DESELECT")
    for o in objects:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    result = bpy.context.view_layer.objects.active
    result.name = name
    result.data.name = name
    weighted = result.modifiers.new("WeightedNormals", "WEIGHTED_NORMAL")
    weighted.keep_sharp = True
    bpy.ops.object.modifier_apply(modifier=weighted.name)
    return result


def triangles(obj: bpy.types.Object) -> int:
    return sum(len(p.vertices) - 2 for p in obj.data.polygons)


def socket(name: str, location: tuple[float, float, float], parent: bpy.types.Object) -> bpy.types.Object:
    empty = bpy.data.objects.new(f"SOCKET_{name}", None)
    empty.empty_display_type = "ARROWS"
    empty.empty_display_size = 0.15
    empty.location = location
    empty.parent = parent
    bpy.context.scene.collection.objects.link(empty)
    return empty


def collision_box(mesh_name: str, index: int, size, center) -> bpy.types.Object:
    obj = box(f"UCX_{mesh_name}_{index:02d}", size, center)
    obj.display_type = "WIRE"
    obj.hide_render = True
    return obj


# Materials -------------------------------------------------------------------------------------

@dataclass
class Wear:
    """The wear story an asset doc gives, as procedural inputs."""
    paint: str
    paint_rough: tuple[float, float] = (0.45, 0.70)
    bare_steel: str = "#5C5F63"
    rust: str = "#7A3F1C"
    roof_dirt: str | None = None
    lower_grime: str | None = None
    chips: float = 0.3          # 0 to 1, how much paint is chipped at edges
    scrape_height: float = 0.2  # m of bare scraped steel along the bottom
    rust_streaks: float = 0.5   # 0 to 1
    extra: dict = field(default_factory=dict)


def _node(nodes, kind: str, x: float, y: float, **inputs):
    n = nodes.new(kind)
    n.location = (x, y)
    for k, v in inputs.items():
        n.inputs[k].default_value = v
    return n


def _mix(nodes, x: float, y: float, data_type: str = "RGBA"):
    """A Mix node and its sockets of the right type (Mix has an A, B and Result per type)."""
    m = nodes.new("ShaderNodeMix")
    m.location = (x, y)
    m.data_type = data_type
    want = "RGBA" if data_type == "RGBA" else "VALUE"
    pick = lambda sockets, name: next(s for s in sockets if s.name == name and s.type == want)
    return m, pick(m.inputs, "A"), pick(m.inputs, "B"), pick(m.outputs, "Result")


def flat_material(name: str, color: str, roughness: float, metallic: float = 0.0) -> bpy.types.Material:
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = hex_rgb(color)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic
    return mat


def painted_steel(name: str, wear: Wear) -> bpy.types.Material:
    """Painted steel with chipped edges, rust streaks running down from the top, a scraped
    bottom band, and dirt on top: all from object space, so it bakes the same every time."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    nodes, links = nt.nodes, nt.links
    bsdf = nodes["Principled BSDF"]
    coord = _node(nodes, "ShaderNodeTexCoord", -1600, 0)
    sep = _node(nodes, "ShaderNodeSeparateXYZ", -1400, 200)
    links.new(coord.outputs["Object"], sep.inputs[0])
    geo = _node(nodes, "ShaderNodeNewGeometry", -1600, -400)

    # Edge wear from ambient occlusion inverted on convex edges (Cycles bakes it).
    edge = nodes.new("ShaderNodeAmbientOcclusion")
    edge.location = (-1400, -200)
    edge.inside = True
    edge.samples = 8
    edge.inputs["Distance"].default_value = 0.02
    edge_ramp = _node(nodes, "ShaderNodeValToRGB", -1200, -200)
    edge_ramp.color_ramp.elements[0].position = 0.0
    edge_ramp.color_ramp.elements[1].position = 0.25 * (1.0 - wear.chips) + 0.05
    links.new(edge.outputs["AO"], edge_ramp.inputs["Fac"])
    chip_noise = _node(nodes, "ShaderNodeTexNoise", -1400, -500, Scale=40.0, Detail=6.0)
    links.new(coord.outputs["Object"], chip_noise.inputs["Vector"])
    chips = _node(nodes, "ShaderNodeMath", -1000, -300)
    chips.operation = "MULTIPLY"
    inv_edge = _node(nodes, "ShaderNodeInvert", -1050, -150)
    links.new(edge_ramp.outputs["Color"], inv_edge.inputs["Color"])
    links.new(inv_edge.outputs["Color"], chips.inputs[0])
    chip_step = _node(nodes, "ShaderNodeMath", -1150, -500)
    chip_step.operation = "GREATER_THAN"
    chip_step.inputs[1].default_value = 0.45
    links.new(chip_noise.outputs["Fac"], chip_step.inputs[0])
    links.new(chip_step.outputs[0], chips.inputs[1])

    # Bottom scrape: bare steel below scrape_height, with ragged edge.
    scrape_noise = _node(nodes, "ShaderNodeTexNoise", -1400, 450, Scale=8.0, Detail=4.0)
    links.new(coord.outputs["Object"], scrape_noise.inputs["Vector"])
    scrape_h = _node(nodes, "ShaderNodeMath", -1150, 450)
    scrape_h.operation = "MULTIPLY_ADD"
    scrape_h.inputs[1].default_value = 0.12
    scrape_h.inputs[2].default_value = wear.scrape_height - 0.06
    links.new(scrape_noise.outputs["Fac"], scrape_h.inputs[0])
    scrape = _node(nodes, "ShaderNodeMath", -950, 450)
    scrape.operation = "LESS_THAN"
    links.new(sep.outputs["Z"], scrape.inputs[0])
    links.new(scrape_h.outputs[0], scrape.inputs[1])

    # Rust streaks: vertical noise stretched along Z, stronger toward the top.
    streak_mapping = _node(nodes, "ShaderNodeMapping", -1400, 700)
    streak_mapping.inputs["Scale"].default_value = (14.0, 14.0, 0.8)
    links.new(coord.outputs["Object"], streak_mapping.inputs["Vector"])
    streak_noise = _node(nodes, "ShaderNodeTexNoise", -1200, 700, Scale=3.0, Detail=8.0, Roughness=0.7)
    links.new(streak_mapping.outputs["Vector"], streak_noise.inputs["Vector"])
    streak_step = _node(nodes, "ShaderNodeMapRange", -1000, 700)
    streak_step.inputs["From Min"].default_value = 0.62 - 0.12 * wear.rust_streaks
    streak_step.inputs["From Max"].default_value = 0.75
    links.new(streak_noise.outputs["Fac"], streak_step.inputs["Value"])
    height_fade = _node(nodes, "ShaderNodeMapRange", -1000, 900)
    height_fade.inputs["From Min"].default_value = 0.6
    height_fade.inputs["From Max"].default_value = 2.6
    links.new(sep.outputs["Z"], height_fade.inputs["Value"])
    rust = _node(nodes, "ShaderNodeMath", -800, 800)
    rust.operation = "MULTIPLY"
    links.new(streak_step.outputs["Result"], rust.inputs[0])
    links.new(height_fade.outputs["Result"], rust.inputs[1])
    ends_x = wear.extra.get("ends_x")
    if ends_x:
        # Rust runs down from the top corners: strongest within a metre of either end.
        ax = _node(nodes, "ShaderNodeMath", -1000, 1050)
        ax.operation = "ABSOLUTE"
        links.new(sep.outputs["X"], ax.inputs[0])
        near_end = _node(nodes, "ShaderNodeMapRange", -850, 1050)
        near_end.inputs["From Min"].default_value = ends_x - 1.2
        near_end.inputs["From Max"].default_value = ends_x
        near_end.inputs["To Min"].default_value = 0.35
        near_end.inputs["To Max"].default_value = 1.6
        links.new(ax.outputs[0], near_end.inputs["Value"])
        boosted = _node(nodes, "ShaderNodeMath", -650, 950)
        boosted.operation = "MULTIPLY"
        boosted.use_clamp = True
        links.new(rust.outputs[0], boosted.inputs[0])
        links.new(near_end.outputs["Result"], boosted.inputs[1])
        rust = boosted

    # Paint with subtle mottling.
    mottle = _node(nodes, "ShaderNodeTexNoise", -1000, 200, Scale=2.5, Detail=3.0)
    links.new(coord.outputs["Object"], mottle.inputs["Vector"])
    paint, paint_a, paint_b, paint_out = _mix(nodes, -800, 200)
    paint_a.default_value = hex_rgb(wear.paint)
    paint_b.default_value = tuple(c * 0.7 for c in hex_rgb(wear.paint)[:3]) + (1.0,)
    links.new(mottle.outputs["Fac"], paint.inputs["Factor"])

    def mix(x, y, factor_socket, a_socket, b_color):
        m, a, b, out = _mix(nodes, x, y)
        links.new(factor_socket, m.inputs["Factor"])
        links.new(a_socket, a)
        b.default_value = b_color
        return out

    with_rust = mix(-600, 400, rust.outputs[0], paint_out, hex_rgb(wear.rust))
    bare = max_node = _node(nodes, "ShaderNodeMath", -600, -300)
    max_node.operation = "MAXIMUM"
    links.new(chips.outputs[0], max_node.inputs[0])
    links.new(scrape.outputs[0], max_node.inputs[1])
    color = mix(-400, 200, bare.outputs[0], with_rust, hex_rgb(wear.bare_steel))
    last = color
    if wear.roof_dirt:
        top = _node(nodes, "ShaderNodeSeparateXYZ", -600, 0)
        links.new(geo.outputs["Normal"], top.inputs[0])
        top_ramp = _node(nodes, "ShaderNodeMapRange", -450, 0)
        top_ramp.inputs["From Min"].default_value = 0.8
        top_ramp.inputs["From Max"].default_value = 1.0
        top_ramp.inputs["To Max"].default_value = 0.6
        links.new(top.outputs["Z"], top_ramp.inputs["Value"])
        last = mix(-250, 200, top_ramp.outputs["Result"], color, hex_rgb(wear.roof_dirt))
    if wear.lower_grime:
        low = _node(nodes, "ShaderNodeMapRange", -450, -100)
        low.inputs["From Min"].default_value = 0.9
        low.inputs["From Max"].default_value = 0.0
        low.inputs["To Max"].default_value = 0.7
        links.new(sep.outputs["Z"], low.inputs["Value"])
        last = mix(-150, 100, low.outputs["Result"], last, hex_rgb(wear.lower_grime))
    # Dirt settles in the grooves: open ambient occlusion darkens the corrugation and seams.
    crevice = nodes.new("ShaderNodeAmbientOcclusion")
    crevice.location = (-450, 650)
    crevice.samples = 8
    crevice.inputs["Distance"].default_value = 0.12
    crevice_ramp = _node(nodes, "ShaderNodeMapRange", -300, 650)
    crevice_ramp.inputs["From Min"].default_value = 0.35
    crevice_ramp.inputs["From Max"].default_value = 1.0
    crevice_ramp.inputs["To Min"].default_value = 0.75
    crevice_ramp.inputs["To Max"].default_value = 0.0
    links.new(crevice.outputs["AO"], crevice_ramp.inputs["Value"])
    last = mix(-100, 300, crevice_ramp.outputs["Result"], last, hex_rgb(wear.extra.get("crevice", "#2A2420")))
    links.new(last, bsdf.inputs["Base Color"])

    # Metal where bare, rough where rusted.
    metal = _node(nodes, "ShaderNodeMath", -300, -300)
    metal.operation = "MULTIPLY"
    metal.inputs[1].default_value = 1.0
    links.new(bare.outputs[0], metal.inputs[0])
    links.new(metal.outputs[0], bsdf.inputs["Metallic"])
    rough_base = _node(nodes, "ShaderNodeMapRange", -500, -600)
    rough_base.inputs["To Min"].default_value = wear.paint_rough[0]
    rough_base.inputs["To Max"].default_value = wear.paint_rough[1]
    links.new(mottle.outputs["Fac"], rough_base.inputs["Value"])
    rough_rust, rough_a, rough_b, rough_out = _mix(nodes, -300, -600, "FLOAT")
    links.new(rust.outputs[0], rough_rust.inputs["Factor"])
    links.new(rough_base.outputs["Result"], rough_a)
    rough_b.default_value = 0.88
    links.new(rough_out, bsdf.inputs["Roughness"])
    return mat


# Baking and export -----------------------------------------------------------------------------

def smart_uv(obj: bpy.types.Object, margin: float = 0.004) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=margin)
    bpy.ops.object.mode_set(mode="OBJECT")


def _bake_target(obj: bpy.types.Object, image: bpy.types.Image) -> None:
    for slot in obj.material_slots:
        nt = slot.material.node_tree
        for n in [n for n in nt.nodes if n.name == "BakeTarget"]:
            nt.nodes.remove(n)
        tex = nt.nodes.new("ShaderNodeTexImage")
        tex.name = "BakeTarget"
        tex.image = image
        tex.location = (600, 600)
        nt.nodes.active = tex


def bake(obj: bpy.types.Object, out_dir: str, prefix: str, resolution: int = 2048, samples: int = 16) -> dict[str, str]:
    """Bakes base color, roughness, metallic and AO to PNGs; returns their paths. Writes an ORM
    texture (R occlusion, G roughness, B metallic) for Unreal and the glTF metallic-roughness
    convention reads the same channels."""
    import numpy as np

    scene = bpy.context.scene
    scene.cycles.samples = samples
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    paths: dict[str, str] = {}
    pixels: dict[str, np.ndarray] = {}

    def run(kind: str, bake_type: str, colorspace: str, **kwargs):
        img = bpy.data.images.new(f"{prefix}_{kind}", resolution, resolution, alpha=False, float_buffer=False)
        img.colorspace_settings.name = colorspace
        _bake_target(obj, img)
        bpy.ops.object.bake(type=bake_type, margin=8, **kwargs)
        path = os.path.join(out_dir, f"{prefix}_{kind}.png")
        img.filepath_raw = path
        img.file_format = "PNG"
        img.save()
        paths[kind] = path
        pixels[kind] = np.array(img.pixels[:], dtype=np.float32).reshape(resolution, resolution, 4)
        return img

    run("BaseColor", "DIFFUSE", "sRGB", pass_filter={"COLOR"})
    run("Roughness", "ROUGHNESS", "Non-Color")
    run("AO", "AO", "Non-Color")
    # Metallic has no bake pass: route it through emission for one bake.
    links = []
    for slot in obj.material_slots:
        nt = slot.material.node_tree
        bsdf = nt.nodes.get("Principled BSDF")
        out = nt.nodes.get("Material Output")
        emit = nt.nodes.new("ShaderNodeEmission")
        emit.name = "MetalBake"
        src = bsdf.inputs["Metallic"]
        if src.is_linked:
            nt.links.new(src.links[0].from_socket, emit.inputs["Color"])
        else:
            v = src.default_value
            emit.inputs["Color"].default_value = (v, v, v, 1.0)
        old = out.inputs["Surface"].links[0].from_socket
        nt.links.new(emit.outputs["Emission"], out.inputs["Surface"])
        links.append((nt, old, out, emit))
    run("Metallic", "EMIT", "Non-Color")
    for nt, old, out, emit in links:
        nt.links.new(old, out.inputs["Surface"])
        nt.nodes.remove(emit)

    orm = bpy.data.images.new(f"{prefix}_ORM", resolution, resolution, alpha=False)
    orm.colorspace_settings.name = "Non-Color"
    packed = np.ones((resolution, resolution, 4), dtype=np.float32)
    packed[..., 0] = pixels["AO"][..., 0]
    packed[..., 1] = pixels["Roughness"][..., 0]
    packed[..., 2] = pixels["Metallic"][..., 0]
    orm.pixels[:] = packed.ravel()
    path = os.path.join(out_dir, f"{prefix}_ORM.png")
    orm.filepath_raw = path
    orm.file_format = "PNG"
    orm.save()
    paths["ORM"] = path
    return paths


def baked_material(name: str, maps: dict[str, str]) -> bpy.types.Material:
    """A material reading only the baked textures: what Unreal and the browser get."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    base = nt.nodes.new("ShaderNodeTexImage")
    base.image = bpy.data.images.load(maps["BaseColor"], check_existing=True)
    base.location = (-600, 300)
    nt.links.new(base.outputs["Color"], bsdf.inputs["Base Color"])
    orm = nt.nodes.new("ShaderNodeTexImage")
    orm.image = bpy.data.images.load(maps["ORM"], check_existing=True)
    orm.image.colorspace_settings.name = "Non-Color"
    orm.location = (-600, -100)
    sep = nt.nodes.new("ShaderNodeSeparateColor")
    sep.location = (-300, -100)
    nt.links.new(orm.outputs["Color"], sep.inputs["Color"])
    nt.links.new(sep.outputs["Green"], bsdf.inputs["Roughness"])
    nt.links.new(sep.outputs["Blue"], bsdf.inputs["Metallic"])
    return mat


def assign_only(obj: bpy.types.Object, mat: bpy.types.Material) -> None:
    obj.data.materials.clear()
    obj.data.materials.append(mat)


def export(objects: list[bpy.types.Object], out_dir: str, name: str) -> dict[str, str]:
    bpy.ops.object.select_all(action="DESELECT")
    for o in objects:
        o.select_set(True)
        for child in o.children:
            child.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    fbx = os.path.join(out_dir, f"{name}.fbx")
    bpy.ops.export_scene.fbx(filepath=fbx, use_selection=True, apply_scale_options="FBX_SCALE_UNITS",
                             object_types={"MESH", "EMPTY"}, mesh_smooth_type="FACE", add_leaf_bones=False,
                             bake_anim=False, path_mode="COPY", embed_textures=False)
    glb = os.path.join(out_dir, f"{name}.glb")
    # The browser gets the render meshes only, not collision or sockets.
    bpy.ops.object.select_all(action="DESELECT")
    for o in objects:
        if not o.name.startswith("UCX_"):
            o.select_set(True)
    bpy.ops.export_scene.gltf(filepath=glb, use_selection=True, export_format="GLB", export_apply=True)
    return {"fbx": fbx, "glb": glb}


# Preview render --------------------------------------------------------------------------------

PREVIEW_OBJECTS = ("PreviewFloor", "Camera", "Key", "Fill", "Rim")


def clear_preview() -> None:
    for o in [o for o in bpy.data.objects if o.name.split(".")[0] in PREVIEW_OBJECTS]:
        bpy.data.objects.remove(o)


def preview(out_path: str, target: Vector, distance: float, sky: str = "#2B2622", key: str = "#F4E7C8",
            fill: str = "#B4551E", resolution=(1600, 1000), samples: int = 64) -> None:
    scene = bpy.context.scene
    scene.cycles.samples = samples
    scene.render.resolution_x, scene.render.resolution_y = resolution
    world = bpy.data.worlds.new("Preview")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = hex_rgb(sky)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 2.5
    scene.world = world

    def light(name, kind, color, energy, location, size=1.0):
        data = bpy.data.lights.new(name, kind)
        data.color = hex_rgb(color)[:3]
        data.energy = energy
        if kind == "AREA":
            data.size = size
        obj = bpy.data.objects.new(name, data)
        obj.location = location
        direction = target - Vector(location)
        obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
        scene.collection.objects.link(obj)

    light("Key", "AREA", key, 2600, (target.x + distance * 0.7, target.y - distance * 0.9, target.z + distance * 0.9), 3.0)
    light("Fill", "AREA", fill, 900, (target.x - distance, target.y + distance * 0.4, target.z + distance * 0.5), 6.0)
    light("Rim", "AREA", "#9FB8D8", 700, (target.x - distance * 0.3, target.y + distance, target.z + distance * 0.2), 2.0)
    floor = box("PreviewFloor", (40, 40, 0.02), (0, 0, -0.01), flat_material("Concrete", "#4A4743", 0.9))
    floor.hide_select = True
    cam_data = bpy.data.cameras.new("Camera")
    cam_data.lens = 40
    cam = bpy.data.objects.new("Camera", cam_data)
    cam.location = (target.x + distance * 0.85, target.y - distance * 0.95, target.z + distance * 0.45)
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()
    scene.collection.objects.link(cam)
    scene.camera = cam
    scene.render.filepath = out_path
    scene.render.image_settings.file_format = "PNG"
    bpy.ops.render.render(write_still=True)
