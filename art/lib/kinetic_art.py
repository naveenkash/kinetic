"""Shared helpers for KINETIC's generated assets (art/README.md).

Every asset is a Python script that builds its meshes from its spec in docs/objects, gives them
procedural materials, bakes those to textures, and exports FBX for Unreal and a GLB preview
client. Runs headless: blender -b --factory-startup -P art/build.py -- <asset>.

Conventions (docs/objects/README-props.md): metres, Z up, X+ toward the object's front (and
north, for side sockets), pivot at the bottom centre of the footprint. Unreal static-mesh sockets
are empties named SOCKET_<name>, their X axis the outward surface normal and Z up where it can be;
simple collision is convex UCX_<mesh>_<nn> hulls.
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


def transform(obj: bpy.types.Object, location=(0, 0, 0), rotation_z: float = 0.0, rotation_x: float = 0.0,
              rotation_y: float = 0.0) -> None:
    """Bakes a placement into the mesh (rotation in X, then Y, then Z) and resets the object."""
    obj.rotation_euler = (rotation_x, rotation_y, rotation_z)
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


def duplicate(obj: bpy.types.Object, name: str | None = None) -> bpy.types.Object:
    """A copy with its own mesh, sharing materials: a part that is in both the high and low meshes."""
    copy = obj.copy()
    copy.data = obj.data.copy()
    copy.name = name or obj.name
    bpy.context.scene.collection.objects.link(copy)
    return copy


def high_low(name: str, shared: list, detail: list, proxy: list) -> tuple[bpy.types.Object, bpy.types.Object]:
    """Builds a game mesh and the detail it is baked from. `shared` parts are in both, `detail`
    (chain links, rope wraps, weld mesh) only in the high mesh, and `proxy` (the low stand-ins
    for that detail) only in the game mesh. Returns (low, high)."""
    copies = [duplicate(o) for o in shared]  # A copy keeps its bevel modifiers
    high = join(f"HIGH_{name}", copies + detail)
    low = join(name, shared + proxy)
    return low, high


def triangles(obj: bpy.types.Object) -> int:
    return sum(len(p.vertices) - 2 for p in obj.data.polygons)


def socket(name: str, location: tuple[float, float, float], parent: bpy.types.Object,
           normal: tuple[float, float, float] | None = None) -> bpy.types.Object:
    """A Dog socket (README-props, Socket orientation): X along the outward surface `normal`,
    Z object-up where possible (north, X+, when the normal itself is vertical). Top and Bot
    sockets default to straight up and down; every other socket names its normal."""
    from mathutils import Matrix
    if normal is None:
        kind = name.split("_")[2] if name.startswith("SKT_Dog_") else ""
        if kind == "Top":
            normal = (0.0, 0.0, 1.0)
        elif kind == "Bot":
            normal = (0.0, 0.0, -1.0)
        else:
            raise ValueError(f"{name}: give the socket's outward normal")
    x = Vector(normal).normalized()
    up = Vector((0.0, 0.0, 1.0))
    z = up - x * x.dot(up)
    if z.length < 1e-6:
        z = Vector((1.0, 0.0, 0.0)) - x * x.x
    z.normalize()
    y = z.cross(x)
    empty = bpy.data.objects.new(f"SOCKET_{name}", None)
    empty.empty_display_type = "ARROWS"
    empty.empty_display_size = 0.15
    empty.matrix_world = Matrix.Translation(Vector(location)) @ Matrix((x, y, z)).transposed().to_4x4()
    empty.parent = parent
    bpy.context.scene.collection.objects.link(empty)
    return empty


def _collision(obj: bpy.types.Object) -> bpy.types.Object:
    obj.display_type = "WIRE"
    obj.hide_render = True
    return obj


def collision_cylinder(mesh_name: str, index: int, radius: float, z0: float, z1: float, sides: int = 16,
                       top_radius: float | None = None, axis: str = "Z", center=(0.0, 0.0)):
    """A convex cylinder (or, with `top_radius`, a truncated cone) collision hull. The polygon
    is circumscribed, so its flats sit at `radius` and nothing round pokes through them. Along
    X or Y, z0 and z1 run along that axis and `center` is the other two coordinates."""
    grow = 1.0 / math.cos(math.pi / sides)
    top = radius if top_radius is None else top_radius
    obj = lathe(f"UCX_{mesh_name}_{index:02d}", [(radius * grow, z0), (top * grow, z1)], sides)
    from mathutils import Matrix
    if axis == "X":
        obj.data.transform(Matrix.Rotation(math.pi / 2, 4, "Y"))
    elif axis == "Y":
        obj.data.transform(Matrix.Rotation(-math.pi / 2, 4, "X"))
    if axis in ("X", "Y"):
        a, b = center
        obj.data.transform(Matrix.Translation((0.0, a, b) if axis == "X" else (a, 0.0, b)))
    else:
        obj.data.transform(Matrix.Translation((center[0], center[1], 0.0)))
    return _collision(obj)


def collision_hull(mesh_name: str, index: int, points) -> bpy.types.Object:
    """The convex hull of `points` as a collision hull: a rotated box, a capsule's stand-in."""
    bm = bmesh.new()
    verts = [bm.verts.new(p) for p in points]
    bmesh.ops.convex_hull(bm, input=verts)
    return _collision(mesh_object(f"UCX_{mesh_name}_{index:02d}", bm))


def collision_segment(mesh_name: str, index: int, a, b, radius: float, sides: int = 8,
                      floor: float | None = 0.0) -> bpy.types.Object:
    """A tube or a leg's collision: an n-sided prism around the segment a to b, circumscribing
    a capsule of `radius` without its caps. Points below `floor` are lifted to it, so a leg's
    hull stops at the floor rather than poking through it."""
    a, b = Vector(a), Vector(b)
    axis = (b - a).normalized()
    side = axis.orthogonal().normalized()
    other = axis.cross(side)
    r = radius / math.cos(math.pi / sides)
    ring = [side * (r * math.cos(2 * math.pi * k / sides)) + other * (r * math.sin(2 * math.pi * k / sides))
            for k in range(sides)]
    points = [a + o for o in ring] + [b + o for o in ring]
    if floor is not None:
        points = [Vector((p.x, p.y, max(p.z, floor))) for p in points]
    return collision_hull(mesh_name, index, points)


def collision_box(mesh_name: str, index: int, size, center, chamfer: float = 0.0) -> bpy.types.Object:
    """A box collision hull; with `chamfer`, its eight corners and twelve edges are cut back."""
    if chamfer <= 0:
        return _collision(box(f"UCX_{mesh_name}_{index:02d}", size, center))
    hx, hy, hz = (d / 2 for d in size)
    c = chamfer
    points = []
    for sx in (-1, 1):
        for sy in (-1, 1):
            for sz in (-1, 1):
                points += [(sx * (hx - c), sy * hy, sz * (hz - c)), (sx * hx, sy * (hy - c), sz * (hz - c)),
                           (sx * (hx - c), sy * (hy - c), sz * hz)]
    return collision_hull(mesh_name, index, [Vector(p) + Vector(center) for p in points])


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
    # Streaks fade in from nothing at the band's bottom to full at its top (m above the pivot):
    # set it to the asset's height, or a short prop gets none.
    streak_band: tuple[float, float] = (0.6, 2.6)
    bottom_rust: float = 0.0    # m of patchy rust rising from the base (a drum's bottom chime)
    bare_bands: tuple = ()      # (z0, z1) bands worn to bare steel all round (a drum's hoops)
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
    height_fade.inputs["From Min"].default_value = wear.streak_band[0]
    height_fade.inputs["From Max"].default_value = wear.streak_band[1]
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
    if wear.bottom_rust > 0:
        # Patchy rust up from the base, strongest at the bottom edge.
        low = _node(nodes, "ShaderNodeMapRange", -1000, 1250)
        low.inputs["From Min"].default_value = wear.bottom_rust
        low.inputs["From Max"].default_value = 0.0
        links.new(sep.outputs["Z"], low.inputs["Value"])
        patch = _node(nodes, "ShaderNodeMath", -850, 1250)
        patch.operation = "MULTIPLY"
        patch.use_clamp = True
        links.new(low.outputs["Result"], patch.inputs[0])
        patchy = _node(nodes, "ShaderNodeMapRange", -1000, 1400)
        patchy.inputs["From Min"].default_value = 0.35
        patchy.inputs["From Max"].default_value = 0.6
        patchy.inputs["To Min"].default_value = 0.4
        patchy.inputs["To Max"].default_value = 1.4
        links.new(streak_noise.outputs["Fac"], patchy.inputs["Value"])
        links.new(patchy.outputs["Result"], patch.inputs[1])
        either = _node(nodes, "ShaderNodeMath", -650, 1150)
        either.operation = "MAXIMUM"
        links.new(rust.outputs[0], either.inputs[0])
        links.new(patch.outputs[0], either.inputs[1])
        rust = either

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
    for i, (z0, z1) in enumerate(wear.bare_bands):
        # 1 inside the band, fading out over a centimetre either side.
        rise = _node(nodes, "ShaderNodeMapRange", -900, -500 - 150 * i)
        rise.inputs["From Min"].default_value = z0 - 0.01
        rise.inputs["From Max"].default_value = z0
        links.new(sep.outputs["Z"], rise.inputs["Value"])
        fall = _node(nodes, "ShaderNodeMapRange", -900, -575 - 150 * i)
        fall.inputs["From Min"].default_value = z1 + 0.01
        fall.inputs["From Max"].default_value = z1
        links.new(sep.outputs["Z"], fall.inputs["Value"])
        band = _node(nodes, "ShaderNodeMath", -750, -500 - 150 * i)
        band.operation = "MULTIPLY"
        links.new(rise.outputs["Result"], band.inputs[0])
        links.new(fall.outputs["Result"], band.inputs[1])
        wider = _node(nodes, "ShaderNodeMath", -650, -450 - 150 * i)
        wider.operation = "MAXIMUM"
        links.new(bare.outputs[0], wider.inputs[0])
        links.new(band.outputs[0], wider.inputs[1])
        bare = wider
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

def triangulate(obj: bpy.types.Object) -> None:
    """Fixes the triangulation before the bake: a normal map is only right on the triangles it
    was baked on, so Unreal and the glTF exporter must not pick their own."""
    mod = obj.modifiers.new("Triangulate", "TRIANGULATE")
    mod.quad_method = "BEAUTY"
    mod.ngon_method = "BEAUTY"
    if hasattr(mod, "keep_custom_normals"):
        mod.keep_custom_normals = True
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=mod.name)


def _gltf_output(nt, occlusion_socket) -> None:
    """The glTF exporter reads occlusion from a node group of this name; Blender ignores it."""
    group = bpy.data.node_groups.get("glTF Material Output")
    if group is None:
        group = bpy.data.node_groups.new("glTF Material Output", "ShaderNodeTree")
        group.interface.new_socket("Occlusion", in_out="INPUT", socket_type="NodeSocketFloat")
    node = nt.nodes.new("ShaderNodeGroup")
    node.node_tree = group
    node.location = (300, -300)
    nt.links.new(occlusion_socket, node.inputs["Occlusion"])


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


def _emit_from(objects, socket_of) -> list:
    """Points every material on `objects` at an emission shader fed by `socket_of(bsdf)` (a socket
    to link, or a value): the trick for passes Cycles cannot bake directly. Returns the undo list."""
    undo = []
    for mat in {slot.material for o in objects for slot in o.material_slots if slot.material}:
        nt = mat.node_tree
        bsdf = nt.nodes.get("Principled BSDF")
        out = nt.nodes.get("Material Output")
        emit = nt.nodes.new("ShaderNodeEmission")
        src = socket_of(bsdf)
        if isinstance(src, bpy.types.NodeSocket) and src.is_linked:
            nt.links.new(src.links[0].from_socket, emit.inputs["Color"])
        else:
            v = src.default_value if isinstance(src, bpy.types.NodeSocket) else src
            emit.inputs["Color"].default_value = tuple(v)[:3] + (1.0,) if hasattr(v, "__len__") else (v, v, v, 1.0)
        old = out.inputs["Surface"].links[0].from_socket
        nt.links.new(emit.outputs["Emission"], out.inputs["Surface"])
        undo.append((nt, old, out, emit))
    return undo


def _restore(undo: list) -> None:
    for nt, old, out, emit in undo:
        nt.links.new(old, out.inputs["Surface"])
        nt.nodes.remove(emit)


def emission_strength(objects) -> float:
    """The strongest emission on these objects' materials (0 if nothing glows)."""
    strengths = [0.0]
    for o in objects:
        for slot in o.material_slots:
            bsdf = slot.material.node_tree.nodes.get("Principled BSDF") if slot.material else None
            if bsdf is not None:
                strengths.append(bsdf.inputs["Emission Strength"].default_value)
    return max(strengths)


def bake(obj: bpy.types.Object, out_dir: str, prefix: str, resolution: int = 2048, samples: int = 16,
         high: bpy.types.Object | None = None, cage: float = 0.02, mask: bool = False) -> dict[str, str]:
    """Bakes base color, roughness, metallic and AO to PNGs and packs the last three into an ORM
    texture (R occlusion, G roughness, B metallic), which Unreal and glTF's metallic-roughness
    convention read the same way; returns every map's path.

    With `high`, every pass is projected from that detailed mesh onto `obj` (rays start `cage`
    outside it and run 2 x `cage`), and a tangent-space normal map is baked too, OpenGL
    convention (Y up): what glTF expects. game/Tools/unreal/import_props.py flips green for
    Unreal. With `mask`, texels the rays miss are transparent: BaseColor carries the alpha for a
    masked card (weld mesh). A glowing material adds an Emissive map."""
    import numpy as np

    scene = bpy.context.scene
    scene.cycles.samples = samples
    bpy.ops.object.select_all(action="DESELECT")
    obj.hide_render = False  # The procedural preview may have hidden it behind its high mesh
    if high is not None:
        # The game mesh's own materials do not matter to a projected bake.
        assign_only(obj, flat_material(f"{prefix}_BakeLow", "#000000", 0.5))
        high.hide_render = False
        high.select_set(True)
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    sources = [high] if high is not None else [obj]
    projected = {"use_selected_to_active": True, "cage_extrusion": cage, "max_ray_distance": 2 * cage} if high else {}
    paths: dict[str, str] = {}
    pixels: dict[str, np.ndarray] = {}

    def run(kind: str, bake_type: str, colorspace: str, margin: int = 8, project: bool = True, **kwargs):
        # BaseColor of a masked card keeps an alpha channel for the mask.
        alpha = mask and kind == "BaseColor"
        img = bpy.data.images.new(f"{prefix}_{kind}", resolution, resolution, alpha=alpha, float_buffer=False)
        img.colorspace_settings.name = colorspace
        img.generated_color = (0.0, 0.0, 0.0, 1.0)
        _bake_target(obj, img)
        bpy.ops.object.bake(type=bake_type, margin=margin, **(projected if project else {}), **kwargs)
        path = os.path.join(out_dir, f"{prefix}_{kind}.png")
        img.filepath_raw = path
        img.file_format = "PNG"
        img.save()
        paths[kind] = path
        pixels[kind] = np.array(img.pixels[:], dtype=np.float32).reshape(resolution, resolution, 4)
        return img

    # Base colour goes through emission too: Cycles' diffuse colour pass is black on metal.
    undo = _emit_from(sources, lambda bsdf: bsdf.inputs["Base Color"])
    run("BaseColor", "EMIT", "sRGB")
    _restore(undo)
    run("Roughness", "ROUGHNESS", "Non-Color")
    run("AO", "AO", "Non-Color")
    # Metallic has no bake pass: route it through emission for one bake.
    undo = _emit_from(sources, lambda bsdf: bsdf.inputs["Metallic"])
    run("Metallic", "EMIT", "Non-Color")
    _restore(undo)
    if high is not None:
        run("Normal", "NORMAL", "Non-Color", normal_space="TANGENT")
    if emission_strength(sources) > 0:
        # Emission colour only where a material glows; the strength goes on the material.
        undo = _emit_from(sources, lambda bsdf: bsdf.inputs["Emission Color"]
                          if bsdf.inputs["Emission Strength"].default_value > 0 else (0.0, 0.0, 0.0, 1.0))
        run("Emissive", "EMIT", "sRGB")
        _restore(undo)
    if mask:
        # White wherever a ray hits the detail, with no margin so the gaps stay clear; and the
        # texels the game mesh covers at all. Texels outside every UV island stay opaque, so the
        # padding around a solid part's island cannot open a seam.
        undo = _emit_from(sources, lambda bsdf: 1.0)
        run("Mask", "EMIT", "Non-Color", margin=0)
        _restore(undo)
        if high is not None:
            high.select_set(False)
            undo = _emit_from([obj], lambda bsdf: 1.0)
            run("Covered", "EMIT", "Non-Color", margin=0, project=False)
            _restore(undo)
            high.select_set(True)
            os.remove(paths.pop("Covered"))
        base = bpy.data.images[f"{prefix}_BaseColor"]
        rgba = pixels["BaseColor"].copy()
        hit = pixels["Mask"][..., 0] > 0.5
        covered = pixels["Covered"][..., 0] > 0.5 if "Covered" in pixels else np.ones_like(hit)
        rgba[..., 3] = (hit | ~covered).astype(np.float32)
        base.alpha_mode = "STRAIGHT"
        base.pixels[:] = rgba.ravel()
        base.save()
        os.remove(paths.pop("Mask"))

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


def baked_material(name: str, maps: dict[str, str], masked: bool = False,
                   emission: float = 0.0, two_sided: bool = False) -> bpy.types.Material:
    """A material reading only the baked textures: what Unreal and the GLB preview get."""
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
    _gltf_output(nt, sep.outputs["Red"])
    if "Normal" in maps:
        nrm = nt.nodes.new("ShaderNodeTexImage")
        nrm.image = bpy.data.images.load(maps["Normal"], check_existing=True)
        nrm.image.colorspace_settings.name = "Non-Color"
        nrm.location = (-600, -450)
        nmap = nt.nodes.new("ShaderNodeNormalMap")
        nmap.location = (-300, -450)
        nt.links.new(nrm.outputs["Color"], nmap.inputs["Color"])
        nt.links.new(nmap.outputs["Normal"], bsdf.inputs["Normal"])
    if "Emissive" in maps:
        glow = nt.nodes.new("ShaderNodeTexImage")
        glow.image = bpy.data.images.load(maps["Emissive"], check_existing=True)
        glow.location = (-600, -750)
        nt.links.new(glow.outputs["Color"], bsdf.inputs["Emission Color"])
        bsdf.inputs["Emission Strength"].default_value = emission
    # One-sided, as Unreal's are by default; a masked card is seen from both sides, and so is a
    # single sheet seen from inside (a container's walls once its doors are open).
    mat.use_backface_culling = not (masked or two_sided)
    if masked:
        # Round on alpha is what the glTF exporter reads as alphaMode MASK at 0.5.
        clip = nt.nodes.new("ShaderNodeMath")
        clip.operation = "ROUND"
        clip.location = (-300, 150)
        nt.links.new(base.outputs["Alpha"], clip.inputs[0])
        nt.links.new(clip.outputs["Value"], bsdf.inputs["Alpha"])
    return mat


def assign_only(obj: bpy.types.Object, mat: bpy.types.Material) -> None:
    obj.data.materials.clear()
    obj.data.materials.append(mat)


def _fbx(path: str, objects: list[bpy.types.Object]) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    for o in objects:
        o.select_set(True)
        for child in o.children:
            child.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS",
                             object_types={"MESH", "EMPTY"}, mesh_smooth_type="FACE", add_leaf_bones=False,
                             bake_anim=False, path_mode="COPY", embed_textures=False)


def export(objects: list[bpy.types.Object], out_dir: str, name: str) -> dict:
    """FBX for Unreal and a GLB preview. A part with its own pivot (a container door, on
    its hinge) gets an FBX of its own with that pivot at the origin, since Unreal's importer
    bakes node positions into the vertices; the body carries an ATTACH_ socket where it goes."""
    meshes = [o for o in objects if not o.name.startswith("UCX_")]
    collision = [o for o in objects if o.name.startswith("UCX_")]

    def hulls(mesh):
        return [c for c in collision if c.name.startswith(f"UCX_{mesh.name}_")]

    pivoted = [m for m in meshes[1:] if m.location.length > 1e-6]
    fbx = os.path.join(out_dir, f"{name}.fbx")
    _fbx(fbx, [m for m in meshes if m not in pivoted] + [c for m in meshes if m not in pivoted for c in hulls(m)])
    parts = {}
    for m in pivoted:
        at = m.location.copy()
        m.location = (0.0, 0.0, 0.0)
        for c in hulls(m):
            c.location -= at
        parts[m.name] = os.path.join(out_dir, f"{m.name}.fbx")
        _fbx(parts[m.name], [m] + hulls(m))
        m.location = at
        for c in hulls(m):
            c.location += at
    glb = os.path.join(out_dir, f"{name}.glb")
    # The GLB preview gets the render meshes only, not collision or sockets.
    bpy.ops.object.select_all(action="DESELECT")
    for o in objects:
        if not o.name.startswith("UCX_"):
            o.select_set(True)
    bpy.ops.export_scene.gltf(filepath=glb, use_selection=True, export_format="GLB", export_apply=True,
                              export_tangents=True)
    return {"fbx": fbx, "glb": glb, "fbx_parts": parts}


# Preview render --------------------------------------------------------------------------------

PREVIEW_OBJECTS = ("PreviewFloor", "Camera", "Key", "Fill", "Rim")


def clear_preview() -> None:
    for o in [o for o in bpy.data.objects if o.name.split(".")[0] in PREVIEW_OBJECTS]:
        bpy.data.objects.remove(o)


def preview(out_path: str, size: tuple[float, float, float], sky: str = "#2B2622", key: str = "#F4E7C8",
            fill: str = "#B4551E", resolution=(1600, 1000), samples: int = 64) -> None:
    """Renders the asset under a sodium-lit three-point rig scaled to its size: the camera frames
    the whole object, and the lights keep the same exposure at any distance."""
    scene = bpy.context.scene
    scene.cycles.samples = samples
    scene.render.resolution_x, scene.render.resolution_y = resolution
    world = bpy.data.worlds.new("Preview")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = hex_rgb(sky)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 2.5
    scene.world = world

    lens = 40.0
    hfov = 2 * math.atan(36.0 / 2 / lens)
    vfov = 2 * math.atan(36.0 / 2 / lens * resolution[1] / resolution[0])
    radius = 0.5 * math.sqrt(size[0] ** 2 + size[1] ** 2 + size[2] ** 2)
    distance = 1.15 * max(radius / math.tan(hfov / 2), (size[2] * 0.6) / math.tan(vfov / 2))
    target = Vector((0.0, 0.0, size[2] * 0.45))
    exposure = (distance / 9.0) ** 2  # The rig was tuned at 9 m

    def light(name, kind, color, energy, direction, light_size):
        data = bpy.data.lights.new(name, kind)
        data.color = hex_rgb(color)[:3]
        data.energy = energy * exposure
        data.size = light_size * distance / 9.0
        obj = bpy.data.objects.new(name, data)
        obj.location = target + Vector(direction).normalized() * distance
        obj.rotation_euler = (target - obj.location).to_track_quat("-Z", "Y").to_euler()
        scene.collection.objects.link(obj)

    light("Key", "AREA", key, 2600, (0.6, -0.8, 0.8), 3.0)
    light("Fill", "AREA", fill, 900, (-1.0, 0.4, 0.5), 6.0)
    light("Rim", "AREA", "#9FB8D8", 700, (-0.3, 1.0, 0.2), 2.0)
    floor = box("PreviewFloor", (max(40, distance * 8), max(40, distance * 8), 0.02), (0, 0, -0.01),
                flat_material("Concrete", "#4A4743", 0.9))
    floor.hide_select = True
    cam_data = bpy.data.cameras.new("Camera")
    cam_data.lens = lens
    cam = bpy.data.objects.new("Camera", cam_data)
    cam.location = target + Vector((0.62, -0.70, 0.34)).normalized() * distance
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()
    scene.collection.objects.link(cam)
    scene.camera = cam
    scene.render.filepath = out_path
    scene.render.image_settings.file_format = "PNG"
    bpy.ops.render.render(write_still=True)


# More shapes -----------------------------------------------------------------------------------

def decal_plate(name: str, width: float, height: float, material, center, normal, wrap_onto=None,
                roll: float = 0.0, up=(0, 0, 1)) -> bpy.types.Object:
    """A flat decal (a hazard diamond, a placard) centred on `center` facing `normal`, rolled by
    `roll` radians about the normal, and with `wrap_onto` shrink-wrapped 1.5 mm proud of that
    surface so a curved body does not leave its corners standing off."""
    from mathutils import Matrix
    bm = bmesh.new()
    bmesh.ops.create_grid(bm, x_segments=8, y_segments=8, size=0.5)
    bmesh.ops.scale(bm, vec=(width, height, 1.0), verts=bm.verts)
    n = Vector(normal).normalized()
    u = Vector(up).normalized()
    x = u.cross(n).normalized() if abs(u.dot(n)) < 0.99 else Vector((1, 0, 0))
    y = n.cross(x).normalized()
    basis = Matrix((x, y, n)).transposed().to_4x4()
    bmesh.ops.transform(bm, verts=bm.verts, matrix=Matrix.Translation(Vector(center)) @ basis @ Matrix.Rotation(roll, 4, "Z"))
    obj = mesh_object(name, bm)
    obj.data.materials.append(material)
    if wrap_onto is not None:
        mod = obj.modifiers.new("Wrap", "SHRINKWRAP")
        mod.target = wrap_onto
        mod.wrap_method = "NEAREST_SURFACEPOINT"
        mod.offset = 0.0015
    return obj


def lathe(name: str, profile: list[tuple[float, float]], segments: int = 32, material=None,
          cap_bottom: bool = True, cap_top: bool = True) -> bpy.types.Object:
    """Revolves a (radius, z) profile about Z: drums, cylinders, buckets, cans, valves."""
    bm = bmesh.new()
    rings = []
    for r, z in profile:
        ring = []
        for i in range(segments):
            a = 2 * math.pi * i / segments
            ring.append(bm.verts.new((r * math.cos(a), r * math.sin(a), z)))
        rings.append(ring)
    for k in range(len(rings) - 1):
        a, b = rings[k], rings[k + 1]
        for i in range(segments):
            j = (i + 1) % segments
            bm.faces.new((a[i], a[j], b[j], b[i]))
    if cap_bottom and profile[0][0] > 1e-6:
        bm.faces.new(list(reversed(rings[0])))
    if cap_top and profile[-1][0] > 1e-6:
        bm.faces.new(rings[-1])
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-6)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    obj = mesh_object(name, bm)
    if material:
        obj.data.materials.append(material)
    for p in obj.data.polygons:
        p.use_smooth = True
    return obj


def ring(name: str, major: float, minor: float, center=(0, 0, 0), axis: str = "Z", segments: int = 24,
         sides: int = 8, material=None, stretch: float = 0.0) -> bpy.types.Object:
    """A torus (hoops, chain links). `stretch` lengthens it along X into a link."""
    bm = bmesh.new()
    loops = []
    for i in range(segments):
        a = 2 * math.pi * i / segments
        loop = []
        for k in range(sides):
            b = 2 * math.pi * k / sides
            rr = major + minor * math.cos(b)
            x = rr * math.cos(a) + (math.copysign(stretch / 2, math.cos(a)) if abs(math.cos(a)) > 1e-9 else 0.0)
            loop.append(bm.verts.new((x, rr * math.sin(a), minor * math.sin(b))))
        loops.append(loop)
    for i in range(segments):
        a, b = loops[i], loops[(i + 1) % segments]
        for k in range(sides):
            m = (k + 1) % sides
            bm.faces.new((a[k], b[k], b[m], a[m]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    from mathutils import Matrix
    if axis == "X":
        bmesh.ops.rotate(bm, verts=bm.verts, cent=(0, 0, 0), matrix=Matrix.Rotation(math.pi / 2, 3, "Y"))
    elif axis == "Y":
        bmesh.ops.rotate(bm, verts=bm.verts, cent=(0, 0, 0), matrix=Matrix.Rotation(math.pi / 2, 3, "X"))
    bmesh.ops.translate(bm, verts=bm.verts, vec=Vector(center))
    obj = mesh_object(name, bm)
    if material:
        obj.data.materials.append(material)
    for p in obj.data.polygons:
        p.use_smooth = True
    return obj


def tube(name: str, points: list[tuple[float, float, float]], radius: float, material=None,
         sides: int = 6, closed: bool = False) -> bpy.types.Object:
    """A round bar along a polyline (chair frames, bails, valve guards, tripod legs)."""
    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.bevel_depth = radius
    curve.bevel_resolution = max(0, (sides - 4) // 2)  # A round bevel has 4 + 2 x resolution sides
    curve.use_fill_caps = True
    spline = curve.splines.new("POLY")
    spline.points.add(len(points) - 1)
    for p, co in zip(spline.points, points):
        p.co = (co[0], co[1], co[2], 1.0)
    spline.use_cyclic_u = closed
    obj = bpy.data.objects.new(name, curve)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.convert(target="MESH")
    obj = bpy.context.view_layer.objects.active
    obj.data.materials.clear()
    if material:
        obj.data.materials.append(material)
    for p in obj.data.polygons:
        p.use_smooth = True
    return obj


def stencil(name: str, body: str, size: float, material, center, normal: tuple[float, float, float],
            wrap_onto: bpy.types.Object | None = None, up: tuple[float, float, float] = (0, 0, 1),
            cylinder: tuple | None = None) -> bpy.types.Object:
    """Lettering as a mesh decal centred on `center`, facing `normal`.

    With `cylinder` = (a point on the axis, the axis direction, radius), the flat lettering is
    first bent round that cylinder, keeping its length along the curve: a drum's or a mast's.
    With `wrap_onto`, it is then shrink-wrapped 2 mm proud of that surface. A wrap alone suits a
    nearly flat face; on a tight curve it pulls a line's ends in toward the middle."""
    curve = bpy.data.curves.new(name, "FONT")
    curve.body = body
    curve.size = size
    curve.resolution_u = 2
    obj = bpy.data.objects.new(name, curve)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.convert(target="MESH")
    obj = bpy.context.view_layer.objects.active
    mesh = obj.data
    # Centre the glyphs on the origin, then turn +Z (the text's face) to `normal`, keeping `up` up.
    co = [v.co.copy() for v in mesh.vertices]
    mid = Vector([(min(c[i] for c in co) + max(c[i] for c in co)) / 2 for i in range(3)])
    from mathutils import Matrix
    mesh.transform(Matrix.Translation(-mid))
    n = Vector(normal).normalized()
    u = Vector(up).normalized()
    x = u.cross(n).normalized() if abs(u.dot(n)) < 0.99 else Vector((1, 0, 0))
    y = n.cross(x).normalized()
    basis = Matrix((x, y, n)).transposed().to_4x4()
    mesh.transform(Matrix.Translation(Vector(center)) @ basis)
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    if cylinder is not None:
        # Arc length along the tangent t is kept: s = r * angle. A glyph's straight edges become
        # chords, sinking at most L^2 / 8r into the curve (0.4 mm for a 2 cm stroke on 14 cm).
        origin, axis, radius = Vector(cylinder[0]), Vector(cylinder[1]).normalized(), cylinder[2]
        out = n - axis * axis.dot(n)
        out.normalize()
        t = axis.cross(out)
        for v in bm.verts:
            d = v.co - origin
            h, s, q = d.dot(axis), d.dot(t), d.dot(out) - radius
            angle = s / radius
            v.co = origin + axis * h + (out * math.cos(angle) + t * math.sin(angle)) * (radius + 0.002 + q)
    bm.to_mesh(mesh)
    bm.free()
    mesh.materials.clear()
    mesh.materials.append(material)
    if wrap_onto is not None:
        mod = obj.modifiers.new("Wrap", "SHRINKWRAP")
        mod.target = wrap_onto
        mod.wrap_method = "NEAREST_SURFACEPOINT"
        mod.offset = 0.002
        bpy.ops.object.modifier_apply(modifier=mod.name)
    return obj


# More materials --------------------------------------------------------------------------------

def _noise_value(nodes, links, coord, x, y, scale, detail=4.0, roughness=0.5):
    n = _node(nodes, "ShaderNodeTexNoise", x, y, Scale=scale, Detail=detail, Roughness=roughness)
    links.new(coord.outputs["Object"], n.inputs["Vector"])
    return n


def wood(name: str, color: str, weathered: str | None = None, rough=(0.78, 0.9), grain_axis: str = "X") -> bpy.types.Material:
    """Sawn timber: grain bands along `grain_axis`, a weathered grey on the upward faces."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    bsdf = nodes["Principled BSDF"]
    coord = _node(nodes, "ShaderNodeTexCoord", -1200, 0)
    mapping = _node(nodes, "ShaderNodeMapping", -1000, 0)
    scale = {"X": (0.6, 14.0, 14.0), "Y": (14.0, 0.6, 14.0), "Z": (14.0, 14.0, 0.6)}[grain_axis]
    mapping.inputs["Scale"].default_value = scale
    links.new(coord.outputs["Object"], mapping.inputs["Vector"])
    grain = _node(nodes, "ShaderNodeTexWave", -800, 0, Scale=2.0, Distortion=6.0, Detail=3.0)
    grain.wave_type = "RINGS"
    links.new(mapping.outputs["Vector"], grain.inputs["Vector"])
    m, a, b, out = _mix(nodes, -500, 0)
    a.default_value = hex_rgb(color)
    b.default_value = tuple(c * 0.62 for c in hex_rgb(color)[:3]) + (1.0,)
    links.new(grain.outputs["Fac"], m.inputs["Factor"])
    last = out
    if weathered:
        geo = _node(nodes, "ShaderNodeNewGeometry", -800, -300)
        sep = _node(nodes, "ShaderNodeSeparateXYZ", -600, -300)
        links.new(geo.outputs["Normal"], sep.inputs[0])
        up = _node(nodes, "ShaderNodeMapRange", -450, -300)
        up.inputs["From Min"].default_value = 0.3
        up.inputs["From Max"].default_value = 1.0
        up.inputs["To Max"].default_value = 0.85
        links.new(sep.outputs["Z"], up.inputs["Value"])
        w, wa, wb, wout = _mix(nodes, -250, -100)
        links.new(up.outputs["Result"], w.inputs["Factor"])
        links.new(out, wa)
        wb.default_value = hex_rgb(weathered)
        last = wout
    links.new(last, bsdf.inputs["Base Color"])
    r = _node(nodes, "ShaderNodeMapRange", -250, -350)
    r.inputs["To Min"].default_value, r.inputs["To Max"].default_value = rough
    links.new(grain.outputs["Fac"], r.inputs["Value"])
    links.new(r.outputs["Result"], bsdf.inputs["Roughness"])
    return mat


def fabric(name: str, color: str, rough: float = 0.95, dirt: str = "#2B2622",
           grab_z: float | None = None) -> bpy.types.Material:
    """Canvas and cloth: mottled colour, dirt toward the bottom, no shine. With `grab_z`, it is
    dark with hand grime in the 6 cm under that height (a bag's top edge)."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    bsdf = nodes["Principled BSDF"]
    coord = _node(nodes, "ShaderNodeTexCoord", -1000, 0)
    n = _noise_value(nodes, links, coord, -800, 0, 9.0, 6.0, 0.6)
    m, a, b, out = _mix(nodes, -500, 0)
    a.default_value = hex_rgb(color)
    b.default_value = tuple(c * 0.75 for c in hex_rgb(color)[:3]) + (1.0,)
    links.new(n.outputs["Fac"], m.inputs["Factor"])
    sep = _node(nodes, "ShaderNodeSeparateXYZ", -800, -300)
    links.new(coord.outputs["Object"], sep.inputs[0])
    low = _node(nodes, "ShaderNodeMapRange", -600, -300)
    low.inputs["From Min"].default_value = 0.12
    low.inputs["From Max"].default_value = 0.0
    low.inputs["To Max"].default_value = 0.6
    links.new(sep.outputs["Z"], low.inputs["Value"])
    d, da, db, dout = _mix(nodes, -300, -100)
    links.new(low.outputs["Result"], d.inputs["Factor"])
    links.new(out, da)
    db.default_value = hex_rgb(dirt)
    if grab_z is not None:
        grab = _node(nodes, "ShaderNodeMapRange", -600, -500)
        grab.inputs["From Min"].default_value = grab_z - 0.06
        grab.inputs["From Max"].default_value = grab_z
        grab.inputs["To Max"].default_value = 0.55
        links.new(sep.outputs["Z"], grab.inputs["Value"])
        g, ga, gb, gout = _mix(nodes, -100, -200)
        links.new(grab.outputs["Result"], g.inputs["Factor"])
        links.new(dout, ga)
        gb.default_value = hex_rgb(dirt)
        dout = gout
    links.new(dout, bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = rough
    return mat


def plastic(name: str, color: str, rough=(0.4, 0.55), scuff: str = "#D8D4C8") -> bpy.types.Material:
    """HDPE: a slightly waxy plastic with pale scuffs at the edges."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    bsdf = nodes["Principled BSDF"]
    coord = _node(nodes, "ShaderNodeTexCoord", -1000, 0)
    n = _noise_value(nodes, links, coord, -800, 0, 30.0, 6.0, 0.6)
    edge = nodes.new("ShaderNodeAmbientOcclusion")
    edge.location = (-800, -250)
    edge.inside = True
    edge.inputs["Distance"].default_value = 0.01
    inv = _node(nodes, "ShaderNodeMapRange", -600, -250)
    inv.inputs["From Min"].default_value = 0.0
    inv.inputs["From Max"].default_value = 0.3
    inv.inputs["To Min"].default_value = 0.6
    inv.inputs["To Max"].default_value = 0.0
    links.new(edge.outputs["AO"], inv.inputs["Value"])
    s = _node(nodes, "ShaderNodeMath", -450, -200)
    s.operation = "MULTIPLY"
    links.new(inv.outputs["Result"], s.inputs[0])
    links.new(n.outputs["Fac"], s.inputs[1])
    m, a, b, out = _mix(nodes, -300, 0)
    a.default_value = hex_rgb(color)
    b.default_value = hex_rgb(scuff)
    links.new(s.outputs[0], m.inputs["Factor"])
    links.new(out, bsdf.inputs["Base Color"])
    r = _node(nodes, "ShaderNodeMapRange", -300, -350)
    r.inputs["To Min"].default_value, r.inputs["To Max"].default_value = rough
    links.new(n.outputs["Fac"], r.inputs["Value"])
    links.new(r.outputs["Result"], bsdf.inputs["Roughness"])
    return mat


def galvanized(name: str = "MI_Steel_Galvanized", color: str = "#9EA3A6") -> bpy.types.Material:
    """Galvanized steel: metallic, dulled by patches of white zinc bloom."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    bsdf = nodes["Principled BSDF"]
    coord = _node(nodes, "ShaderNodeTexCoord", -1000, 0)
    n = _noise_value(nodes, links, coord, -800, 0, 6.0, 8.0, 0.65)
    bloom = _node(nodes, "ShaderNodeMapRange", -600, 0)
    bloom.inputs["From Min"].default_value = 0.55
    bloom.inputs["From Max"].default_value = 0.7
    links.new(n.outputs["Fac"], bloom.inputs["Value"])
    m, a, b, out = _mix(nodes, -400, 0)
    a.default_value = hex_rgb(color)
    b.default_value = hex_rgb("#D4D6D2")
    links.new(bloom.outputs["Result"], m.inputs["Factor"])
    links.new(out, bsdf.inputs["Base Color"])
    rough, ra, rb, rout = _mix(nodes, -400, -250, "FLOAT")
    links.new(bloom.outputs["Result"], rough.inputs["Factor"])
    ra.default_value = 0.38
    rb.default_value = 0.7
    links.new(rout, bsdf.inputs["Roughness"])
    met, ma, mb, mout = _mix(nodes, -400, -450, "FLOAT")
    links.new(bloom.outputs["Result"], met.inputs["Factor"])
    ma.default_value = 1.0
    mb.default_value = 0.4
    links.new(mout, bsdf.inputs["Metallic"])
    return mat


def emissive(name: str, color: str, strength: float) -> bpy.types.Material:
    mat = flat_material(name, color, 0.05)
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Emission Color"].default_value = hex_rgb(color)
    bsdf.inputs["Emission Strength"].default_value = strength
    return mat
