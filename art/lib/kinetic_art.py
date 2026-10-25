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
    scene.cycles.device = _cycles_device()
    scene.cycles.seed = 0
    scene.cycles.samples = 48
    scene.cycles.use_denoising = True


def _cycles_device() -> str:
    """The GPU when Blender has one (Metal on a Mac), which bakes many times faster; the CPU
    otherwise, or with KINETIC_CPU=1 in the environment. Either bakes the same maps, to within
    sampling noise."""
    if os.environ.get("KINETIC_CPU"):
        return "CPU"
    prefs = bpy.context.preferences.addons["cycles"].preferences
    for kind in ("METAL", "OPTIX", "CUDA", "HIP", "ONEAPI"):
        try:
            prefs.compute_device_type = kind
        except TypeError:
            continue
        prefs.get_devices()
        if any(dev.type == kind for dev in prefs.devices):
            for dev in prefs.devices:
                dev.use = dev.type == kind
            if hasattr(prefs, "kernel_optimization_level"):
                # Metal's specialised kernels compile on a background thread that can crash a
                # headless Blender; the generic kernel is barely slower for a bake.
                prefs.kernel_optimization_level = "OFF"
            return "GPU"
    return "CPU"


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
                     material: bpy.types.Material | None = None, rows: int = 1, bend: float = 0.0) -> bpy.types.Object:
    """A trapezoid-corrugated sheet in its local XZ plane, ribs along Z, pressed toward -Y by
    `depth`. Length along X is centred on 0; height runs from z = 0. One-sided, facing +Y. `rows`
    splits it up its height, so it can be dented; `bend` rounds every fold, splitting it into two
    that far apart, and the faces are smooth so the weighted normals round it off."""
    flat = pitch * 0.35
    slope = (pitch - 2 * flat) / 2
    corners: list[tuple[float, float]] = []
    x = -length / 2
    corners.append((x, 0.0))
    while x < length / 2 - 1e-6:
        for dx, dy in ((flat, 0.0), (slope, -depth), (flat, -depth), (slope, 0.0)):
            x = min(x + dx, length / 2)
            corners.append((x, dy))
            if x >= length / 2 - 1e-6:
                break
    profile = corners
    if bend > 0:
        profile = [corners[0]]
        for a, b, c in zip(corners, corners[1:], corners[2:]):
            # Two points either side of the fold at b, on its two runs.
            for p, q in ((b, a), (b, c)):
                run = math.hypot(q[0] - p[0], q[1] - p[1])
                t = min(0.45, bend / 2 / run) if run > 1e-9 else 0.0
                profile.append((p[0] + (q[0] - p[0]) * t, p[1] + (q[1] - p[1]) * t))
        profile.append(corners[-1])
    bm = bmesh.new()
    grid = [[bm.verts.new((px, py, height * k / rows)) for px, py in profile] for k in range(rows + 1)]
    for k in range(rows):
        for i in range(len(profile) - 1):
            bm.faces.new((grid[k][i], grid[k][i + 1], grid[k + 1][i + 1], grid[k + 1][i]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    # Make sure the panel faces +Y.
    if sum(f.normal.y for f in bm.faces) < 0:
        bmesh.ops.reverse_faces(bm, faces=bm.faces)
    obj = mesh_object(name, bm)
    if bend > 0:
        for p in obj.data.polygons:
            p.use_smooth = True
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


def cut(obj: bpy.types.Object, cutter: bpy.types.Object) -> None:
    """Cuts `cutter` out of `obj` (a casting's apertures, a pocket's slot) and deletes it. Called
    before add_bevel, so the bevel rounds the cut's rims too."""
    mod = obj.modifiers.new("Cut", "BOOLEAN")
    mod.operation = "DIFFERENCE"
    mod.solver = "EXACT"
    mod.object = cutter
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.data.objects.remove(cutter)


def _soften(obj: bpy.types.Object, angle: float = 50.0) -> None:
    """Smooth shading with the folds over `angle` kept sharp: with weighted normals, big faces
    shade flat, and bevels and bends between them shade round, as worked metal does."""
    for p in obj.data.polygons:
        p.use_smooth = True
    obj.data.set_sharp_from_angle(angle=math.radians(angle))


def _weight_normals(obj: bpy.types.Object) -> None:
    bpy.context.view_layer.objects.active = obj
    weighted = obj.modifiers.new("WeightedNormals", "WEIGHTED_NORMAL")
    weighted.keep_sharp = True
    bpy.ops.object.modifier_apply(modifier=weighted.name)


def join(name: str, objects: list[bpy.types.Object], soft: bool = False) -> bpy.types.Object:
    """Applies every part's modifiers and joins them into one mesh with weighted normals. With
    `soft`, every face is smooth-shaded but for folds sharper than 50 degrees, so bevels, bends
    and curved parts shade round and flat faces stay flat."""
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
    if soft:
        _soften(result)
    _weight_normals(result)
    return result


def duplicate(obj: bpy.types.Object, name: str | None = None) -> bpy.types.Object:
    """A copy with its own mesh, sharing materials: a part that is in both the high and low meshes."""
    copy = obj.copy()
    copy.data = obj.data.copy()
    copy.name = name or obj.name
    bpy.context.scene.collection.objects.link(copy)
    return copy


def high_low(name: str, shared: list, detail: list, proxy: list,
             soft: bool = False) -> tuple[bpy.types.Object, bpy.types.Object]:
    """Builds a game mesh and the detail it is baked from. `shared` parts are in both, `detail`
    (chain links, rope wraps, weld mesh, stencils) only in the high mesh, and `proxy` (the low
    stand-ins for that detail) only in the game mesh. Returns (low, high)."""
    copies = [duplicate(o) for o in shared]  # A copy keeps its bevel modifiers
    high = join(f"HIGH_{name}", copies + detail, soft)
    low = join(name, shared + proxy, soft)
    return low, high


def deform(objects, offset) -> None:
    """Moves every vertex of `objects` by offset(world position), a Vector, and re-derives their
    weighted normals: the dents, sag and bow of a used object. It is a field of position, so a
    game mesh and its high mesh deformed by the same field still match."""
    for obj in objects:
        to_world, to_local = obj.matrix_world, obj.matrix_world.inverted()
        for v in obj.data.vertices:
            p = to_world @ v.co
            v.co = to_local @ (p + offset(p))
        obj.data.update()
        _weight_normals(obj)


def dents(*dents) -> callable:
    """An offset field (for deform) of round dents, each (centre, push, radius): a push vector of
    the dent's depth and direction at its centre, falling off smoothly to nothing at `radius`."""
    shaped = [(Vector(c), Vector(p), r) for c, p, r in dents]

    def offset(p: Vector) -> Vector:
        out = Vector((0.0, 0.0, 0.0))
        for c, push, r in shaped:
            t = (p - c).length / r
            if t < 1.0:
                out += push * (1 - t * t) ** 2
        return out
    return offset


def waviness(amount: float, scale: float, seed: int, axes=(1.0, 1.0, 1.0)) -> callable:
    """An offset field (for deform) of smooth noise up to `amount` metres, `scale` waves a metre,
    on the axes given: a panel's oil-canning, a drum's out-of-round. Seeded, so it is the same
    every build."""
    from mathutils import noise
    shift = Vector((seed * 17.31, seed * 5.77, seed * 11.13))
    ax = Vector(axes)

    def offset(p: Vector) -> Vector:
        n = noise.noise_vector(p * scale + shift)
        return Vector((n.x * ax.x, n.y * ax.y, n.z * ax.z)) * amount
    return offset


def fields(*offsets) -> callable:
    """Several offset fields summed into one."""
    return lambda p: sum((f(p) for f in offsets), Vector((0.0, 0.0, 0.0)))


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
    """The wear story an asset doc gives, as procedural inputs. Lengths are metres, amounts run 0
    to 1, colours are hex (or linear RGBA)."""
    paint: str
    paint_rough: tuple[float, float] = (0.45, 0.70)
    bare_steel: str = "#5C5F63"
    rust: str = "#7A3F1C"
    roof_dirt: str | None = None   # The dust or dirt on upward faces (a pale dust if None)
    lower_grime: str | None = None
    chips: float = 0.3          # 0 to 1, how much paint is chipped at edges
    scrape_height: float = 0.2  # m of bare scraped steel along the bottom
    rust_streaks: float = 0.5   # 0 to 1: how far rust has got, in old chips and in streaks
    # Streaks fade in from nothing at the band's bottom to full at its top (m above the pivot):
    # set it to the asset's height, or a short prop gets none.
    streak_band: tuple[float, float] = (0.6, 2.6)
    bottom_rust: float = 0.0    # m of patchy rust rising from the base (a drum's bottom chime)
    bare_bands: tuple = ()      # (z0, z1) bands worn to bare steel all round (a drum's hoops)
    extra: dict = field(default_factory=dict)
    primer: str | None = "#6B3A2A"  # Red oxide: the ring round every chip, under the paint
    paint_metal: float = 0.0    # 1 for bare steel, whose "paint" is its mill scale
    faded: str | None = None    # What the paint chalks to on the faces the sun gets
    detail: float = 1.0         # Scales chips, scratches and pits: 1 a container, 3 a crate, 8 a tube
    edge: float = 0.03          # How far edge wear reaches in from a convex edge (m)
    grime_reach: float = 0.12   # How far into a crevice its dirt reaches (m)
    scratches: float = 0.5
    dents: float = 0.5          # Dents and dings, in the normal map
    dust: float = 0.5
    oil: float = 0.25           # Oily, glossy patches low down
    streak_from: tuple = ()     # Where rust bleeds down from: heights (seams, castings), or (x, y, z, half
                                # width) points (a lug, a bolt)
    streak_length: float = 0.35
    streaks: float | None = None  # The streaks' own amount, if not rust_streaks
    rusted: float = 0.0         # Rust over the whole surface: 1 for a rusted bolt
    drips: tuple = ()           # (x, y, top z, length): a fan of fuel or oil run down from a point
    dirt_streaks: float = 0.5   # Rain-washed dirt in grey streaks down the walls
    bare_metal: float = 1.0     # Whether what the chips bare is metal (0 for ink on wood or cloth)


def _rgba(color) -> tuple[float, float, float, float]:
    return hex_rgb(color) if isinstance(color, str) else tuple(color[:3]) + (1.0,)


def _shade(color, k: float, toward=None, t: float = 0.0) -> tuple[float, float, float, float]:
    """A colour scaled by k, after moving t of the way toward another (grey if None)."""
    c = _rgba(color)
    if t:
        lum = 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]
        o = _rgba(toward) if toward is not None else (lum, lum, lum)
        c = tuple(a + (b - a) * t for a, b in zip(c[:3], o[:3])) + (1.0,)
    return tuple(min(1.0, v * k) for v in c[:3]) + (1.0,)


class Graph:
    """A material's node tree built one value at a time. Each method takes sockets, plain numbers
    or colours and returns its output socket, so a wear layer reads as the arithmetic it is.

    Patterns are sampled at the surface's position in metres, world space: the mesh's own space
    for everything but a hinged part, which keeps its place on the asset. So a mesh bakes the
    same every time, and a high mesh and its game mesh see the same wear."""

    def __init__(self, mat: bpy.types.Material):
        mat.use_nodes = True
        self.mat = mat
        self.nodes, self.links = mat.node_tree.nodes, mat.node_tree.links
        self.bsdf = self.nodes["Principled BSDF"]
        self._count = 0
        geo = self.node("ShaderNodeNewGeometry")
        self.P, self.N = geo.outputs["Position"], geo.outputs["Normal"]
        self.x, self.y, self.z = self.separate(self.P)
        self.nx, self.ny, self.nz = self.separate(self.N)

    def node(self, kind: str, **props):
        n = self.nodes.new(kind)
        n.location = (-3000 + 220 * (self._count // 14), 1400 - 190 * (self._count % 14))
        self._count += 1
        for k, v in props.items():
            setattr(n, k, v)
        return n

    def feed(self, socket, value) -> None:
        if isinstance(value, bpy.types.NodeSocket):
            self.links.new(value, socket)
        elif isinstance(value, str):
            socket.default_value = hex_rgb(value)
        elif socket.type == "RGBA" and hasattr(value, "__len__"):
            socket.default_value = _rgba(value)
        else:
            socket.default_value = value

    def separate(self, vec):
        n = self.node("ShaderNodeSeparateXYZ")
        self.feed(n.inputs[0], vec)
        return n.outputs[0], n.outputs[1], n.outputs[2]

    def math(self, op: str, a, b=0.0, c=None, clamp: bool = False):
        n = self.node("ShaderNodeMath", operation=op, use_clamp=clamp)
        self.feed(n.inputs[0], a)
        self.feed(n.inputs[1], b)
        if c is not None:
            self.feed(n.inputs[2], c)
        return n.outputs[0]

    def add(self, a, b, clamp: bool = False):
        return self.math("ADD", a, b, clamp=clamp)

    def sub(self, a, b, clamp: bool = False):
        return self.math("SUBTRACT", a, b, clamp=clamp)

    def mul(self, a, b, clamp: bool = False):
        return self.math("MULTIPLY", a, b, clamp=clamp)

    def madd(self, a, b, c, clamp: bool = False):
        """a x b + c."""
        return self.math("MULTIPLY_ADD", a, b, c, clamp=clamp)

    def max(self, a, b):
        return self.math("MAXIMUM", a, b)

    def ramp(self, value, lo: float, hi: float, to_lo: float = 0.0, to_hi: float = 1.0, smooth: bool = False):
        """`value` mapped from lo..hi onto to_lo..to_hi and clamped; lo above hi runs it downhill."""
        n = self.node("ShaderNodeMapRange", interpolation_type="SMOOTHSTEP" if smooth else "LINEAR", clamp=True)
        ins = [i for i in n.inputs if i.type == "VALUE"]
        for socket, v in zip(ins, (value, lo, hi, to_lo, to_hi)):
            self.feed(socket, v)
        return n.outputs["Result"]

    def noise(self, vec, scale: float, detail: float = 4.0, roughness: float = 0.5, distortion: float = 0.0,
              color: bool = False):
        n = self.node("ShaderNodeTexNoise")
        self.feed(n.inputs["Vector"], self.P if vec is None else vec)
        n.inputs["Scale"].default_value = scale
        n.inputs["Detail"].default_value = detail
        n.inputs["Roughness"].default_value = roughness
        n.inputs["Distortion"].default_value = distortion
        return n.outputs[1] if color else n.outputs[0]

    def cells(self, vec, scale: float, feature: str = "F1", randomness: float = 1.0):
        """Voronoi cells: (distance, a random 0 to 1 per cell). DISTANCE_TO_EDGE gives thin lines
        along the cell borders, and no per-cell value."""
        n = self.node("ShaderNodeTexVoronoi", feature=feature)
        self.feed(n.inputs["Vector"], self.P if vec is None else vec)
        n.inputs["Scale"].default_value = scale
        n.inputs["Randomness"].default_value = randomness
        if feature == "DISTANCE_TO_EDGE":
            return n.outputs["Distance"], None
        rnd = self.node("ShaderNodeSeparateColor")
        self.links.new(n.outputs["Color"], rnd.inputs[0])
        return n.outputs["Distance"], rnd.outputs[0]

    def wave(self, vec, scale: float, direction: str = "X", kind: str = "BANDS", distortion: float = 0.0,
             detail: float = 0.0, profile: str = "SIN"):
        n = self.node("ShaderNodeTexWave", wave_type=kind, wave_profile=profile)
        if kind == "BANDS":
            n.bands_direction = direction
        else:
            n.rings_direction = direction
        self.feed(n.inputs["Vector"], self.P if vec is None else vec)
        n.inputs["Scale"].default_value = scale
        n.inputs["Distortion"].default_value = distortion
        n.inputs["Detail"].default_value = detail
        return n.outputs[1]

    def mapping(self, vec, scale=(1.0, 1.0, 1.0), rotation=(0.0, 0.0, 0.0), location=(0.0, 0.0, 0.0)):
        n = self.node("ShaderNodeMapping")
        self.feed(n.inputs["Vector"], self.P if vec is None else vec)
        n.inputs["Location"].default_value = location
        n.inputs["Rotation"].default_value = rotation
        n.inputs["Scale"].default_value = scale
        return n.outputs[0]

    def vmath(self, op: str, a, b=(0.0, 0.0, 0.0), scale: float | None = None):
        n = self.node("ShaderNodeVectorMath", operation=op)
        self.feed(n.inputs[0], self.P if a is None else a)
        self.feed(n.inputs[1], b)
        if scale is not None:
            n.inputs["Scale"].default_value = scale
        return n.outputs["Value"] if op in ("DISTANCE", "LENGTH", "DOT_PRODUCT") else n.outputs["Vector"]

    def warp(self, vec, amount: float, scale: float):
        """`vec` pushed about by noise: domain warping, so cells and lines lose their regularity."""
        c = self.noise(vec, scale, 2.0, 0.5, color=True)
        push = self.vmath("SCALE", self.vmath("SUBTRACT", c, (0.5, 0.5, 0.5)), scale=amount)
        return self.vmath("ADD", self.P if vec is None else vec, push)

    def mix(self, fac, a, b, blend: str = "MIX"):
        """Colour a where fac is 0, b where it is 1."""
        m = self.node("ShaderNodeMix", data_type="RGBA", blend_type=blend, clamp_factor=True)
        pick = lambda name: next(s for s in m.inputs if s.name == name and s.type == "RGBA")
        self.feed(m.inputs["Factor"], fac)
        self.feed(pick("A"), a)
        self.feed(pick("B"), b)
        return next(s for s in m.outputs if s.type == "RGBA")

    def mixf(self, fac, a, b):
        m = self.node("ShaderNodeMix", data_type="FLOAT", clamp_factor=True)
        pick = lambda name: next(s for s in m.inputs if s.name == name and s.type == "VALUE")
        self.feed(m.inputs["Factor"], fac)
        self.feed(pick("A"), a)
        self.feed(pick("B"), b)
        return next(s for s in m.outputs if s.type == "VALUE")

    def ao(self, distance: float, inside: bool = False, samples: int = 8):
        """Ambient occlusion on this object alone: open, the crevices; inside, the convex edges
        (low near an edge). Colour only: it is sampled, so it must not drive a bump."""
        n = self.node("ShaderNodeAmbientOcclusion", inside=inside, samples=samples, only_local=True)
        n.inputs["Distance"].default_value = distance
        return n.outputs["AO"]

    def attribute(self, name: str):
        """A mesh attribute (0 where the mesh has none)."""
        return self.node("ShaderNodeAttribute", attribute_name=name, attribute_type="GEOMETRY").outputs["Factor"]

    def bump(self, height, distance: float, normal=None, strength: float = 1.0):
        n = self.node("ShaderNodeBump")
        self.feed(n.inputs["Height"], height)
        n.inputs["Distance"].default_value = distance
        n.inputs["Strength"].default_value = strength
        if normal is not None:
            self.feed(n.inputs["Normal"], normal)
        return n.outputs["Normal"]

    def tag(self, socket, label: str):
        """Labels the node behind a mask, so a test bench can find and show it."""
        socket.node.label = label
        return socket

    def finish(self, color, roughness, metallic, normal=None) -> bpy.types.Material:
        self.feed(self.bsdf.inputs["Base Color"], color)
        self.feed(self.bsdf.inputs["Roughness"], roughness)
        self.feed(self.bsdf.inputs["Metallic"], metallic)
        if normal is not None:
            self.feed(self.bsdf.inputs["Normal"], normal)
        return self.mat


def flat_material(name: str, color: str, roughness: float, metallic: float = 0.0) -> bpy.types.Material:
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = hex_rgb(color)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic
    return mat


# Wear layers that every material shares: each returns a 0 to 1 mask.

def _scratch_lines(g: Graph, d: float, amount: float):
    """Scratches in drifts: long thin lines, most of them level (things slid along it), the rest
    at random; broken along their length. The lines are Voronoi cell borders in a stretched space.
    A scratch is as fine on a chair as on a container, so `d` only goes so far."""
    d = min(d, 3.0)
    level = g.cells(g.warp(g.mapping(None, (2.0 * d, 2.0 * d, 25.0 * d)), 0.08, 0.8), 1.0, "DISTANCE_TO_EDGE")[0]
    slant = g.cells(g.mapping(None, (20.0 * d, 2.5 * d, 2.5 * d), rotation=(0.5, 0.8, 1.2)), 1.0,
                    "DISTANCE_TO_EDGE")[0]
    drift_a = g.ramp(g.noise(None, 1.3 * d, 3.0), 0.5, 0.6)
    drift_b = g.ramp(g.noise(g.mapping(None, location=(17.0, 3.0, 5.0)), 1.9 * d, 3.0), 0.52, 0.62)
    lines = g.max(g.mul(g.ramp(level, 0.0, 0.06, 1.0, 0.0), drift_a), g.mul(g.ramp(slant, 0.0, 0.05, 1.0, 0.0), drift_b))
    broken = g.ramp(g.noise(None, 60.0 * d, 2.0), 0.4, 0.6, 0.3, 1.0)
    return g.mul(g.mul(lines, broken), min(1.0, 1.4 * amount), clamp=True)


def _edges(g: Graph, reach: float):
    """Convex edges: 1 on the edge, falling off over about `reach`. Inside AO is low near a convex
    edge, where rays into the solid soon come out through the next face."""
    return g.ramp(g.ao(reach, inside=True, samples=12), 0.6, 0.97, 1.0, 0.0, smooth=True)


def _grime(g: Graph, reach: float, breakup):
    """Dirt packed into the crevices and corners, from open AO, patchy."""
    return g.mul(g.ramp(g.ao(reach), 0.3, 0.95, 0.9, 0.0), g.ramp(breakup, 0.4, 0.6, 0.55, 1.25), clamp=True)


def _dust(g: Graph, d: float, amount: float, grime):
    """Dust on the upward faces, thicker where it banks up in the grooves."""
    up = g.ramp(g.nz, 0.5, 0.95)
    dust = g.mul(g.mul(up, g.ramp(g.noise(None, 2.2 * d, 5.0, 0.6), 0.4, 0.6, 0.4, 1.0)), min(1.0, 1.5 * amount))
    return g.max(dust, g.mul(g.mul(grime, up), 0.8 * min(1.0, amount * 2)))


def painted_steel(name: str, wear: Wear, under: str | None = None) -> bpy.types.Material:
    """Painted steel, worn the way the asset's doc tells it, a layer at a time. The paint is
    mottled and chalked where the sun gets it. Chips at the convex edges, and dings on the faces,
    are each a ring of primer round bare steel; the older ones have rusted and blistered the
    paint round them. Scratches come in drifts, the bottom is scraped, and rust bleeds down from
    seams and castings in streaks. Grime packs the crevices, dust lies on the upward faces, there
    are oily patches low down, and a hand's smudges break up the roughness. Dents, dings,
    scratches and rust pits are a bump, which the bake writes into the Normal map.

    With `under`, this is lettering or a band sprayed over paint of that colour: its edges are the
    mesh's, its overspray fades out across the stencil's `spray` attribute (ka.stencil), it flakes
    back to `under` instead of chipping at its own edges, and every other layer runs on across it."""
    mat = bpy.data.materials.new(name)
    g = Graph(mat)
    w = wear
    d = w.detail
    big = g.noise(None, 0.8 * d, 3.0, 0.55)
    mid = g.noise(None, 5.0 * d, 6.0, 0.6)
    fine = g.noise(None, 32.0 * d, 8.0, 0.65)
    grain = g.noise(None, 180.0 * d, 3.0, 0.5)
    smudge = g.noise(None, 22.0 * d, 2.0, 0.5, distortion=1.0)

    def coat(color, patches: bool = True):
        # Mottled in big soft patches and finer flecks, chalked toward `faded` on upward faces;
        # with `patches`, touched up here and there in a slightly different batch of paint.
        c = g.mix(g.ramp(big, 0.4, 0.6), _shade(color, 0.62), _shade(color, 0.95))
        c = g.mix(g.ramp(mid, 0.42, 0.58, 0.0, 0.6), c, _shade(color, 0.8, "#6A5A48", 0.2))
        c = g.mix(g.ramp(fine, 0.4, 0.62, 0.0, 0.5), c, _shade(color, 1.12))
        if patches:
            touch = g.ramp(g.noise(g.mapping(None, location=(4.0, 9.0, 2.0)), 0.7 * d, 2.0), 0.6, 0.605)
            c = g.mix(g.mul(touch, 0.85), c, _shade(color, 0.86, "#3A2E28", 0.12))
        chalk = w.faded or _shade(color, 1.1, "#9A9086", 0.3)
        sun = g.mul(g.ramp(g.nz, 0.1, 0.9), g.ramp(mid, 0.4, 0.6, 0.15, 0.7))
        c = g.mix(sun, c, chalk)
        # A film of dirt over everything, heavier low down.
        film = g.mul(g.ramp(big, 0.4, 0.6, 0.12, 0.5), g.ramp(g.z, 0.0, 1.5 / max(d, 0.5), 1.0, 0.45))
        return g.mix(film, c, _shade(color, 0.55, "#3A332A", 0.5))

    paint = coat(w.paint, w.paint_metal < 0.5)
    cover = None
    if under is not None:
        # Overspray: a mist of dots thinning out across the fringe; and flakes back to `under`.
        t = g.attribute("spray")
        dots = g.noise(None, 700.0, 2.0, 0.5)
        cover = g.ramp(g.madd(g.sub(dots, 0.5), g.mul(t, 0.9), t), 0.0, 0.8, 1.0, 0.0)
        flake = g.ramp(g.add(g.mul(fine, 0.7), g.mul(mid, 0.3)), 0.66 - 0.12 * w.chips, 0.68 - 0.12 * w.chips)
        cover = g.tag(g.mul(cover, g.sub(1.0, flake)), "cover")
        paint = g.mix(cover, coat(under, False), paint)

    # Chips: strongest at convex edges (inside AO is low there), ragged by noise; dings anywhere.
    edge = g.tag(_edges(g, w.edge), "edge") if under is None else 0.0
    breakup = g.add(g.mul(fine, 0.6), g.mul(mid, 0.4))
    chipv = g.madd(g.math("POWER", edge, 0.6), 0.6 + 0.7 * w.chips, g.mul(g.sub(breakup, 0.5), 4.0))
    dist, rnd = g.cells(g.warp(None, 0.02 / d, 30.0 * d), 6.0 * d)
    # Smaller dings on a smaller object, but not so many more of them.
    rare = 0.1 * w.chips / max(1.0, d)
    pick = g.ramp(rnd, 1.0 - rare, 1.0 - rare + 0.004)
    ding = g.mul(pick, g.ramp(dist, 0.17, 0.05))
    chipv = g.max(chipv, g.mul(ding, 0.82))
    scratch = _scratch_lines(g, d, w.scratches)
    deep = g.mul(scratch, g.ramp(mid, 0.5, 0.56))
    chipv = g.max(chipv, g.mul(deep, 0.9))

    # The scraped bottom band: bare below a ragged line, in level tine marks toward its top.
    scrape = 0.0
    if w.scrape_height > 0:
        ragged = g.madd(g.noise(None, 8.0 * d, 4.0), 0.12, w.scrape_height - 0.06)
        band = g.ramp(g.sub(ragged, g.z), -0.004, 0.004)
        tines = g.ramp(g.noise(g.mapping(None, (1.5, 1.5, 90.0)), 1.0, 3.0), 0.48, 0.56)
        scrape = g.mul(band, g.max(tines, g.ramp(g.z, w.scrape_height * 0.55, w.scrape_height * 0.3)))
    g.tag(chipv, "chipv")
    primer = g.ramp(chipv, 0.60, 0.625) if w.primer else 0.0
    bare_chip = g.ramp(chipv, 0.70, 0.712)
    bare = g.max(bare_chip, scrape)
    for z0, z1 in w.bare_bands:
        # 1 inside the band, fading out over a centimetre either side, ragged.
        z = g.madd(g.sub(fine, 0.5), 0.05, g.madd(g.sub(mid, 0.5), 0.05, g.z))
        bare = g.max(bare, g.mul(g.ramp(z, z0 - 0.01, z0), g.ramp(z, z1 + 0.01, z1)))

    grime = _grime(g, w.grime_reach, mid)

    # Rust: in the old chips (some of them), blistering the paint round them; patchy from the base.
    rl = w.rust_streaks
    old = g.ramp(g.noise(None, 3.0 * d, 4.0), 0.62 - 0.16 * rl, 0.65 - 0.16 * rl)
    if w.paint_metal >= 0.5:
        # Bare steel: flash rust in spots, mostly where dirt and damp sit.
        spots = g.ramp(g.noise(None, 7.0 * d, 6.0, 0.6), 0.6 - 0.15 * rl, 0.62 - 0.15 * rl)
        rust = g.mul(spots, g.ramp(grime, 0.0, 0.25, 0.35, 1.0))
        halo = 0.0
    else:
        rust = g.mul(bare_chip, old)
        halo = g.mul(g.mul(g.ramp(chipv, 0.48, 0.6), old), 0.45 * min(1.0, 2 * rl))
    whole = 0.0  # Rust that does not come from the sampled AO, and so may drive the bump
    if w.bottom_rust > 0:
        low = g.ramp(g.z, w.bottom_rust, 0.0)
        whole = g.mul(low, g.ramp(g.madd(mid, 0.6, g.mul(big, 0.4)), 0.35, 0.6, 0.4, 1.4), clamp=True)
    if w.rusted > 0:
        whole = g.max(whole, w.rusted)
    rust = g.max(rust, whole)

    # Rust streaks: vertical lanes of stain, fading up a band; bleeding down from seams; strongest
    # near the ends (a container's corner castings).
    stain = 0.0
    sl = w.rust_streaks if w.streaks is None else w.streaks
    if sl > 0:
        lanes = g.noise(g.mapping(None, (7.0 * d, 7.0 * d, 0.4 * d)), 1.0, 6.0, 0.65)
        reach = g.noise(g.mapping(None, (40.0 * d, 40.0 * d, 0.05)), 1.0, 2.0)
        strength = g.ramp(g.noise(g.mapping(None, (6.0 * d, 6.0 * d, 0.3)), 1.0, 2.0), 0.4, 0.6, 0.3, 1.0)
        # A faint wash of streaks up the band; the strong ones come from their sources below.
        stain = g.mul(g.ramp(lanes, 0.54 - 0.06 * sl, 0.72), g.mul(strength, 0.15 + 0.2 * sl))
        stain = g.mul(stain, g.ramp(g.z, w.streak_band[0], w.streak_band[1]))
        ends_x = w.extra.get("ends_x")
        if ends_x:
            near_end = g.ramp(g.math("ABSOLUTE", g.x), ends_x - 1.2, ends_x, 0.35, 1.6)
            stain = g.mul(stain, near_end, clamp=True)
        for source in w.streak_from:
            zs = source if isinstance(source, (int, float)) else source[2]
            below = g.sub(zs, g.z)
            run = g.ramp(g.math("DIVIDE", below, g.madd(reach, w.streak_length, 0.4 * w.streak_length)), 1.0, 0.0)
            run = g.mul(run, g.ramp(below, -0.003, 0.003))
            if not isinstance(source, (int, float)):
                # From a point: only straight below it, widening a little as it runs.
                x0, y0, _, half = source
                h = g.vmath("DISTANCE", g.mapping(None, (1.0, 1.0, 0.0)), (x0, y0, 0.0))
                run = g.mul(run, g.ramp(g.math("DIVIDE", h, g.madd(below, 0.15, half)), 1.0, 0.4))
            stain = g.max(stain, g.mul(g.mul(run, strength), g.ramp(lanes, 0.5 - 0.06 * sl, 0.64)))
        # Streaks run down walls; on a roof the same noise would only blotch.
        stain = g.mul(g.mul(stain, 0.8), g.ramp(g.math("ABSOLUTE", g.nz), 0.7, 0.35))
    # Dirt washed down the walls by rain: grey lanes, strongest under the top and the seams.
    washed = 0.0
    if w.dirt_streaks > 0:
        dirt_lanes = g.noise(g.mapping(None, (5.0 * d, 5.0 * d, 0.3 * d), location=(31.0, 7.0, 3.0)), 1.0, 5.0, 0.6)
        from_top = g.ramp(g.z, w.streak_band[0], w.streak_band[1], 0.25, 1.0)
        washed = g.mul(g.mul(g.ramp(dirt_lanes, 0.5, 0.64), from_top), g.ramp(g.math("ABSOLUTE", g.nz), 0.7, 0.35))
        washed = g.mul(washed, 0.55 * w.dirt_streaks)
    drips = 0.0
    for x0, y0, top, length in w.drips:
        # A fan of fuel run down from a point, widening as it goes, in rivulets.
        flat = g.mapping(None, (1.0, 1.0, 0.0))
        h = g.vmath("DISTANCE", flat, (x0, y0, 0.0))
        down = g.sub(top, g.z)
        fan = g.ramp(g.math("DIVIDE", h, g.madd(down, 0.35, 0.02)), 1.0, 0.55)
        along = g.mul(g.ramp(down, length, 0.0), g.ramp(down, -0.01, 0.01))
        rivulets = g.ramp(g.noise(g.mapping(None, (60.0, 60.0, 1.5)), 1.0, 3.0), 0.4, 0.62, 0.35, 1.0)
        drips = g.max(drips, g.mul(g.mul(fan, along), rivulets))

    dust = _dust(g, d, w.dust, grime)
    footprint = w.extra.get("footprint")
    if footprint:
        # Where another one stood on top: a clean, bright rectangle, rust round it.
        fx, fy, fz = footprint
        inside = g.mul(g.mul(g.ramp(g.math("ABSOLUTE", g.x), fx, fx - 0.01), g.ramp(g.math("ABSOLUTE", g.y), fy, fy - 0.01)),
                       g.mul(g.ramp(g.z, fz - 0.01, fz), g.ramp(g.nz, 0.8, 0.95)))
        dust = g.mul(dust, g.sub(1.0, g.mul(inside, 0.9)))
        rim = g.mul(g.sub(g.mul(g.ramp(g.z, fz - 0.01, fz), g.ramp(g.nz, 0.8, 0.95)), inside), g.ramp(mid, 0.45, 0.55))
        rust = g.max(rust, g.mul(rim, g.ramp(fine, 0.45, 0.6)))
    oil = g.mul(g.mul(g.ramp(g.noise(None, 1.0 * d, 3.0), 0.57, 0.6), w.oil),
                g.ramp(g.z, w.extra.get("oil_height", 0.8), 0.0))
    rv = g.noise(None, 14.0 * d, 6.0, 0.65)
    pits = g.ramp(g.cells(None, 90.0 * d)[0], 0.3, 0.08)

    # Streaks show least on bare metal, which is rubbed clean.
    keep = g.sub(1.0, g.mul(bare, 0.75))
    stain = g.mul(stain, keep)
    washed = g.mul(washed, keep)

    # Colour.
    c = g.mix(g.mul(scratch, 0.55), paint, _shade(w.paint, 1.3, t=0.35))
    c = g.mix(halo, c, "#4A2616")
    if w.primer:
        c = g.mix(primer, c, g.mix(g.ramp(fine, 0.3, 0.7), _shade(w.primer, 0.85), w.primer))
    # Bare steel is brighter than its colour in the doc, which is steel's dull average: worn edges
    # and scrapes are bright metal.
    c = g.mix(bare, c, g.mix(g.ramp(fine, 0.4, 0.6), _shade(w.bare_steel, 2.0), _shade(w.bare_steel, 3.2)))
    rust_color = g.mix(g.ramp(rv, 0.35, 0.65), "#3B1D10", w.rust)
    rust_color = g.mix(g.ramp(fine, 0.62, 0.78), rust_color, "#A65A24")
    c = g.mix(rust, c, rust_color)
    c = g.mix(g.mul(pits, g.mul(rust, 0.6)), c, "#24130B")
    c = g.mix(stain, c, g.mix(g.ramp(mid, 0.42, 0.58), "#4A2412", "#7A3C1A"))
    c = g.mix(washed, c, "#2E2A25")
    c = g.mix(grime, c, w.extra.get("crevice", "#2A2420"))
    if w.lower_grime:
        c = g.mix(g.ramp(g.z, 0.9, 0.0, 0.0, 0.7), c, w.lower_grime)
    c = g.mix(dust, c, w.roof_dirt or "#6A6255")
    c = g.mix(g.mul(oil, 0.6), c, "#141210")
    c = g.mix(g.mul(drips, 0.75), c, "#2B2622")

    # Roughness: glossier where rubbed or oily, matte where primed, rusted or dusty.
    lo, hi = w.paint_rough
    r = g.ramp(g.add(g.mul(big, 0.5), g.mul(mid, 0.5)), 0.35, 0.65, lo, hi)
    r = g.add(r, g.add(g.mul(g.sub(smudge, 0.5), 0.12), g.mul(g.sub(grain, 0.5), 0.1)))
    r = g.mixf(g.mul(scratch, 0.6), r, max(0.05, lo - 0.12))
    r = g.mixf(primer, r, 0.6)
    r = g.mixf(bare, r, g.ramp(fine, 0.4, 0.6, 0.28, 0.48))
    r = g.mixf(rust, r, g.ramp(rv, 0.3, 0.7, 0.78, 0.95))
    r = g.mixf(g.mul(stain, 0.7), r, 0.75)
    r = g.mixf(g.mul(g.max(grime, washed), 0.6), r, 0.82)
    r = g.mixf(dust, r, 0.93)
    r = g.mixf(g.max(oil, drips), r, 0.2)
    m = g.mixf(primer, w.paint_metal, 0.0)
    m = g.mixf(bare, m, w.bare_metal)
    m = g.mixf(rust, m, 0.25)
    m = g.mixf(g.mul(stain, 0.6), m, 0.15)
    m = g.mixf(g.mul(grime, 0.7), m, 0.0)
    m = g.mixf(dust, m, 0.0)

    # Relief. Dents and dings over the panel; then the paint's orange peel, scratches and dings
    # cut into it, the scraped band and the rust's pitted crust.
    dn = g.noise(None, 1.2 * d, 2.0, 0.4)
    dent_dist, dent_rnd = g.cells(None, 2.5 * d)
    dent = g.mul(g.ramp(dent_rnd, 1.0 - 0.3 * w.dents, 1.0 - 0.3 * w.dents + 0.01),
                 g.ramp(dent_dist, 0.35, 0.0, smooth=True))
    macro = g.sub(dn, g.mul(dent, 0.6))
    micro = g.mul(g.sub(grain, 0.5), 0.12)
    micro = g.sub(micro, g.add(g.mul(scratch, 0.45), g.add(g.mul(ding, 0.35), g.mul(scrape, 0.3))))
    if w.bottom_rust > 0 or w.rusted > 0:
        micro = g.add(micro, g.mul(whole, g.sub(g.mul(rv, 0.6), g.mul(pits, 0.8))))
    if cover is not None:
        micro = g.add(micro, g.mul(cover, 0.25))  # The paint film stands proud
    normal = g.bump(macro, 0.02 * w.dents / d) if w.dents > 0 else None
    normal = g.bump(micro, 0.0005 / d, normal)
    return g.finish(c, g.math("MAXIMUM", r, 0.04), m, normal)


def stencil_paint(name: str, ink: str, wear: Wear) -> bpy.types.Material:
    """Stencilled lettering or a painted band in `ink` over the paint `wear` describes: it shares
    every wear layer with the paint under it, so its chips, streaks and dust carry on across it."""
    from dataclasses import replace
    return painted_steel(name, replace(wear, paint=ink, faded=None), under=wear.paint)


def ink(name: str, color: str, under: str, detail: float = 6.0, wear: float = 0.3) -> bpy.types.Material:
    """Paint, ink or tape on a surface that is not painted steel (plywood, canvas, paper, plastic,
    zinc): it wears and flakes back to `under`, fades out across a stencil's overspray fringe, and
    takes a little dust and grime."""
    return painted_steel(name, Wear(paint=color, primer=None, bare_steel=under, bare_metal=0.0, chips=wear,
                                    scrape_height=0.0, rust_streaks=0.0, streaks=0.0, dents=0.0, dust=0.25,
                                    dirt_streaks=0.0, oil=0.0, scratches=0.3, detail=detail, grime_reach=0.02),
                         under=under)


def bare_steel(name: str = "MI_Steel_Bare", color: str = "#5C5F63", rough: tuple[float, float] = (0.5, 0.65),
               rust: float = 0.3, detail: float = 3.0, **wear) -> bpy.types.Material:
    """Unpainted steel: mill scale mottling the grey, polished bright on the edges that hands and
    loads rub, flash rust in spots where the dirt sits, scratched, dusty on top."""
    w = dict(paint=color, paint_rough=rough, paint_metal=1.0, primer=None, bare_steel=_shade(color, 1.35),
             chips=0.45, scrape_height=0.0, rust_streaks=rust, streaks=0.0, detail=detail, dust=0.35,
             edge=min(0.008, 0.03 / detail), grime_reach=0.05, dents=0.25, faded=_shade(color, 1.05))
    w.update(wear)
    return painted_steel(name, Wear(**w))


def rusted(name: str = "MI_Steel_Rusted", color: str = "#7A3F1C", detail: float = 4.0) -> bpy.types.Material:
    """Steel rusted right through: a crust in dark and orange browns, pitted, matte."""
    return painted_steel(name, Wear(paint=color, paint_rough=(0.8, 0.95), primer=None, rust=color, chips=0.2,
                                    scrape_height=0.0, rust_streaks=0.8, streaks=0.0, rusted=1.0, detail=detail,
                                    dust=0.3, edge=0.006, grime_reach=0.04, bare_steel="#4A4C4F", dents=0.2))


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
         high: bpy.types.Object | None = None, cage: float = 0.02, mask: bool = False,
         normal: bool = False) -> dict[str, str]:
    """Bakes base color, roughness, metallic and AO to PNGs and packs the last three into an ORM
    texture (R occlusion, G roughness, B metallic), which Unreal and glTF's metallic-roughness
    convention read the same way; returns every map's path.

    With `high`, every pass is projected from that detailed mesh onto `obj` (rays start `cage`
    outside it and run 2 x `cage`), and a tangent-space normal map is baked too, OpenGL
    convention (Y up): what glTF expects. game/Tools/unreal/import_props.py flips green for
    Unreal. With `mask`, texels the rays miss are transparent: BaseColor carries the alpha for a
    masked card (weld mesh). A glowing material adds an Emissive map. With `normal`, a mesh with
    no high mesh gets a Normal map too, of its materials' own bumps (dents, scratches, grain)."""
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
    if high is not None or normal:
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
                bpy.data.materials.get("PreviewConcrete") or concrete("PreviewConcrete"))
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
            cylinder: tuple | None = None, spray: float = 0.0, grid: float | None = None,
            resolution: int = 2) -> bpy.types.Object:
    """Lettering as a mesh decal centred on `center`, facing `normal`.

    With `cylinder` = (a point on the axis, the axis direction, radius), the flat lettering is
    first bent round that cylinder, keeping its length along the curve: a drum's or a mast's.
    With `wrap_onto`, it is then shrink-wrapped 2 mm proud of that surface. A wrap alone suits a
    nearly flat face; on a tight curve it pulls a line's ends in toward the middle. With `grid`,
    the glyphs are first cut into cells that many metres across, so a wrap can follow corrugation
    or dents rather than bridge them.

    With `spray`, a fringe that many metres wide is outset round every glyph, 0.3 mm under it,
    carrying a `spray` attribute from 0 at the glyph to 1 at its outer edge: stencil_paint fades
    its overspray out across it. `resolution` is the glyphs' curve steps (more for big letters)."""
    curve = bpy.data.curves.new(name, "FONT")
    curve.body = body
    curve.size = size
    curve.resolution_u = resolution
    obj = bpy.data.objects.new(name, curve)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.convert(target="MESH")
    obj = bpy.context.view_layer.objects.active
    mesh = obj.data
    from mathutils import Matrix
    bm = bmesh.new()
    bm.from_mesh(mesh)
    # Centre the glyphs on the origin, in their own plane (the text faces +Z).
    lo = Vector([min(v.co[i] for v in bm.verts) for i in range(3)])
    hi = Vector([max(v.co[i] for v in bm.verts) for i in range(3)])
    bmesh.ops.translate(bm, verts=bm.verts, vec=-(lo + hi) / 2)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    if spray > 0:
        # Each outline's vertices pushed out along the outline's outward normal; a strip of
        # faces between the two.
        layer = bm.verts.layers.float.new("spray")
        boundary = [e for e in bm.edges if len(e.link_faces) == 1]
        out_dir: dict = {}
        for e in boundary:
            a, b = e.verts
            d = (b.co - a.co).normalized()
            n = Vector((d.y, -d.x, 0.0))
            if n.dot((a.co + b.co) / 2 - e.link_faces[0].calc_center_median()) < 0:
                n = -n
            for v in (a, b):
                out_dir[v] = out_dir.get(v, Vector((0.0, 0.0, 0.0))) + n
        rim = {}
        for v, n in out_dir.items():
            if n.length > 1e-9:
                rim[v] = bm.verts.new(v.co + n.normalized() * spray - Vector((0.0, 0.0, 0.0003)))
                rim[v][layer] = 1.0
        for e in boundary:
            a, b = e.verts
            if a in rim and b in rim:
                f = bm.faces.new((a, b, rim[b], rim[a]))
                f.normal_update()
                if f.normal.z < 0:
                    f.normal_flip()
    if grid:
        half = (hi - lo) / 2 + Vector((spray, spray, 0.0))
        for axis, extent in ((Vector((1, 0, 0)), half.x), (Vector((0, 1, 0)), half.y)):
            k = -extent + grid
            while k < extent:
                geom = bm.verts[:] + bm.edges[:] + bm.faces[:]
                bmesh.ops.bisect_plane(bm, geom=geom, plane_co=axis * k, plane_no=axis)
                k += grid
        bmesh.ops.triangulate(bm, faces=bm.faces)
    # Turn +Z (the text's face) to `normal`, keeping `up` up.
    n = Vector(normal).normalized()
    u = Vector(up).normalized()
    x = u.cross(n).normalized() if abs(u.dot(n)) < 0.99 else Vector((1, 0, 0))
    y = n.cross(x).normalized()
    basis = Matrix((x, y, n)).transposed().to_4x4()
    bmesh.ops.transform(bm, verts=bm.verts, matrix=Matrix.Translation(Vector(center)) @ basis)
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
        # Each vertex to its nearest point on the surface, 2 mm out (the fringe 0.3 mm less).
        target = wrap_onto.evaluated_get(bpy.context.evaluated_depsgraph_get())
        inv = target.matrix_world.inverted()
        sprayed = mesh.attributes.get("spray")
        for v in mesh.vertices:
            ok, at, nrm, _ = target.closest_point_on_mesh(inv @ v.co)
            if ok:
                lift = 0.002 - (0.0003 if sprayed is not None and sprayed.data[v.index].value > 0.5 else 0.0)
                v.co = target.matrix_world @ (at + nrm * lift)
    return obj


# More materials --------------------------------------------------------------------------------

def wood(name: str, color: str, weathered: str | None = None, rough=(0.78, 0.9), grain_axis: str = "X",
         detail: float = 1.0, ply_axis: str | None = None, checks: float = 0.4, stains: tuple = (),
         walked: bool = False) -> bpy.types.Material:
    """Sawn timber or plywood. Growth rings along `grain_axis` (bands on the faces, rings on the end
    grain, which is darker and rougher), fibres, a knot or two, and checks: thin splits along the
    grain. Weathered silver-grey on the upward faces, the latewood raised. With `ply_axis`, the
    edges show the veneers and glue lines across that axis. `stains` are (centre, radius) rings (a
    mug's), and `walked` darkens and polishes a plank's centre line where boots go."""
    mat = bpy.data.materials.new(name)
    g = Graph(mat)
    d = detail
    along = {"X": 0, "Y": 1, "Z": 2}[grain_axis]
    stretch = lambda a, b: tuple(a if i == along else b for i in range(3))
    rings = g.wave(g.mapping(None, stretch(0.6, 14.0)), 2.0, "SPHERICAL", "RINGS", distortion=6.0, detail=3.0)
    fibre = g.noise(g.mapping(None, stretch(1.5 * d, 260.0 * d)), 1.0, 3.0, 0.6)
    figure = g.noise(g.mapping(None, stretch(2.0 * d, 18.0 * d)), 1.0, 4.0, 0.6)
    knot_dist, knot_rnd = g.cells(g.mapping(None, stretch(2.5, 9.0)), 1.0)
    knot = g.mul(g.ramp(knot_rnd, 0.93, 0.935), g.ramp(knot_dist, 0.22, 0.04))
    split = g.cells(g.warp(g.mapping(None, stretch(0.8 * d, 30.0 * d)), 0.05, 3.0), 1.0, "DISTANCE_TO_EDGE")[0]
    split = g.mul(g.ramp(split, 0.0, 0.03, 1.0, 0.0), g.ramp(g.noise(None, 3.0 * d, 3.0), 0.62 - 0.1 * checks, 0.68))
    split = g.mul(split, min(1.0, 2 * checks))
    normal_along = (g.nx, g.ny, g.nz)[along]
    end_grain = g.ramp(g.math("ABSOLUTE", normal_along), 0.6, 0.9)
    mid = g.noise(None, 4.0 * d, 5.0, 0.6)

    late = g.ramp(rings, 0.25, 0.75)
    c = g.mix(late, color, _shade(color, 0.6))
    c = g.mix(g.ramp(figure, 0.35, 0.65, 0.0, 0.6), c, _shade(color, 1.12))
    c = g.mix(g.mul(g.ramp(fibre, 0.45, 0.7), 0.35), c, _shade(color, 0.72))
    c = g.mix(knot, c, _shade(color, 0.35, "#3A2414", 0.5))
    c = g.mix(end_grain, c, _shade(color, 0.7))
    if ply_axis is not None:
        ply_normal = {"X": g.nx, "Y": g.ny, "Z": g.nz}[ply_axis]
        veneers = g.wave(g.mapping(None, (1.0, 1.0, 1.0)), 0.314 / 0.0028, ply_axis, "BANDS")
        edge_face = g.ramp(g.math("ABSOLUTE", ply_normal), 0.5, 0.2)
        glue = g.mul(edge_face, g.ramp(veneers, 0.82, 0.92))
        c = g.mix(g.mul(edge_face, g.ramp(veneers, 0.4, 0.6, 0.0, 0.35)), c, _shade(color, 1.15))
        c = g.mix(glue, c, "#2E2419")

    if weathered:
        up = g.mul(g.ramp(g.nz, 0.3, 1.0), g.ramp(mid, 0.3, 0.65, 0.6, 1.0))
        silver = g.mix(g.ramp(fibre, 0.4, 0.7), weathered, _shade(weathered, 0.78))
        c = g.mix(g.mul(up, 0.88), c, silver)
    for center, radius in stains:
        ring = g.ramp(g.math("ABSOLUTE", g.sub(g.vmath("DISTANCE", None, center), radius)), 0.006, 0.0)
        inside = g.ramp(g.vmath("DISTANCE", None, center), radius, radius - 0.01, 0.0, 0.25)
        c = g.mix(g.mul(g.max(ring, inside), g.ramp(mid, 0.3, 0.6, 0.5, 1.0)), c, _shade(color, 0.45, "#2A1D12", 0.4))
    if walked:
        lane = g.mul(g.ramp(g.math("ABSOLUTE", g.y), 0.07, 0.02), g.ramp(g.nz, 0.7, 0.95))
        boots = g.mul(lane, g.ramp(g.noise(None, 6.0, 4.0), 0.35, 0.65, 0.3, 0.9))
        c = g.mix(boots, c, "#3A332B")
    else:
        boots = 0.0
    grime = _grime(g, 0.03, mid)
    c = g.mix(split, c, "#1E1610")
    c = g.mix(grime, c, "#2A231C")
    dust = _dust(g, d, 0.25, grime)
    c = g.mix(dust, c, "#6A6255")

    r = g.ramp(rings, 0.2, 0.8, rough[0], rough[1])
    r = g.add(r, g.mul(g.sub(fibre, 0.5), 0.12))
    r = g.mixf(end_grain, r, min(1.0, rough[1] + 0.05))
    r = g.mixf(boots, r, rough[0] - 0.2)
    r = g.mixf(dust, r, 0.95)
    # Weathered wood stands its latewood proud; fibres, splits and the end grain's pores cut in.
    h = g.add(g.mul(late, 0.35), g.mul(g.sub(fibre, 0.5), 0.5))
    h = g.sub(h, g.add(g.mul(split, 1.5), g.mul(knot, 0.2)))
    h = g.add(h, g.mul(end_grain, g.mul(g.noise(None, 300.0 * d, 2.0), 0.6)))
    normal = g.bump(g.noise(None, 1.5 * d, 2.0), 0.004 / d)
    normal = g.bump(h, 0.0006 / d, normal)
    return g.finish(c, r, 0.0, normal)


def fabric(name: str, color: str, rough: float = 0.95, dirt: str = "#2B2622",
           grab_z: float | None = None, weave: float = 0.0022, detail: float = 4.0) -> bpy.types.Material:
    """Canvas and cloth: a plain weave of `weave`-metre threads in the normal map, mottled and
    faded pale on the faces the sun gets, water marks, dirt toward the bottom and in the creases,
    soft folds, no shine. With `grab_z`, it is dark with hand grime in the 6 cm under that height
    (a bag's top edge)."""
    mat = bpy.data.materials.new(name)
    g = Graph(mat)
    d = detail
    s = 0.314 / weave
    wx, wy, wz = (g.wave(None, s, axis, "BANDS") for axis in ("X", "Y", "Z"))
    threads = g.add(g.add(wx, wy), wz)
    slub = g.noise(g.mapping(None, (1.0, 1.0, 30.0)), 40.0, 2.0)
    mottle = g.noise(None, 9.0, 6.0, 0.6)
    mid = g.noise(None, 3.0 * d, 5.0, 0.6)
    c = g.mix(g.ramp(mottle, 0.3, 0.7), _shade(color, 0.78), color)
    c = g.mix(g.ramp(slub, 0.4, 0.7, 0.0, 0.3), c, _shade(color, 0.85))
    c = g.mix(g.ramp(threads, 0.9, 1.5, 0.0, 0.1), c, _shade(color, 0.88))
    sun = g.mul(g.ramp(g.nz, -0.2, 0.8), g.ramp(mid, 0.35, 0.7, 0.1, 0.45))
    c = g.mix(sun, c, _shade(color, 1.2, t=0.3))
    # Water marks: the tide line of a dried puddle, a ring of the noise's level set.
    tide = g.mul(g.ramp(g.math("ABSOLUTE", g.sub(g.noise(None, 2.0 * d, 3.0), 0.58)), 0.012, 0.0), 0.5)
    c = g.mix(tide, c, _shade(color, 0.7))
    low = g.ramp(g.z, 0.12, 0.0, 0.0, 0.6)
    c = g.mix(low, c, dirt)
    if grab_z is not None:
        grab = g.mul(g.ramp(g.z, grab_z - 0.06, grab_z, 0.0, 0.6), g.ramp(mottle, 0.3, 0.6, 0.6, 1.0))
        c = g.mix(grab, c, dirt)
    grime = _grime(g, 0.04, mid)
    c = g.mix(grime, c, dirt)
    dust = _dust(g, d, 0.3, grime)
    c = g.mix(dust, c, "#7A7263")
    r = g.add(rough, g.mul(g.sub(mottle, 0.5), 0.08))
    r = g.math("MINIMUM", r, 1.0)
    folds = g.noise(g.mapping(None, (1.0, 3.0, 1.0)), 6.0, 3.0, 0.5)
    normal = g.bump(folds, 0.006)
    normal = g.bump(g.add(threads, g.mul(slub, 0.6)), 0.00012, normal)
    return g.finish(c, r, 0.0, normal)


def plastic(name: str, color: str, rough=(0.4, 0.55), scuff: str = "#D8D4C8", detail: float = 4.0,
            parting: bool = False, edge: float = 0.008, scratches: float = 0.6) -> bpy.types.Material:
    """Moulded plastic: slightly waxy, with a faint texture; scuffed pale at the edges and in
    drifts of fine scratches where it is dragged; dirt in the creases and dust on top. With
    `parting`, the mould's parting line runs up the X axis' plane (y = 0) and the gate mark is a
    ring under the base. `edge` is how far the edge scuffs reach (keep it well under a small
    part's radius, or the whole part reads as an edge)."""
    mat = bpy.data.materials.new(name)
    g = Graph(mat)
    d = detail
    n = g.noise(None, 30.0, 6.0, 0.6)
    mid = g.noise(None, 3.0 * d, 5.0, 0.6)
    edge = _edges(g, edge)
    scratch = _scratch_lines(g, d, scratches)
    scuffed = g.mul(g.max(g.mul(edge, g.ramp(n, 0.35, 0.65, 0.3, 1.0)), g.mul(scratch, 0.35)), 0.8)
    c = g.mix(g.ramp(mid, 0.3, 0.7), _shade(color, 0.9), color)
    c = g.mix(scuffed, c, scuff)
    seam = 0.0
    if parting:
        seam = g.ramp(g.math("ABSOLUTE", g.y), 0.0012, 0.0004)
        gate = g.mul(g.ramp(g.math("ABSOLUTE", g.sub(g.vmath("LENGTH", g.mapping(None, (1.0, 1.0, 0.0))), 0.008)),
                            0.002, 0.0), g.ramp(g.z, 0.02, 0.01))
        seam = g.max(seam, gate)
        c = g.mix(g.mul(seam, 0.4), c, _shade(color, 1.25))
    grime = _grime(g, 0.03, mid)
    c = g.mix(grime, c, "#2A2420")
    dust = _dust(g, d, 0.3, grime)
    c = g.mix(dust, c, "#7A7263")
    r = g.ramp(n, 0.3, 0.7, rough[0], rough[1])
    r = g.mixf(scuffed, r, min(1.0, rough[1] + 0.2))
    r = g.mixf(dust, r, 0.9)
    h = g.sub(g.add(g.mul(g.noise(None, 400.0, 2.0), 0.3), g.mul(seam, 0.8)), g.mul(scratch, 0.6))
    normal = g.bump(g.noise(None, 2.0 * d, 2.0), 0.0008)
    normal = g.bump(h, 0.0003, normal)
    return g.finish(c, r, 0.0, normal)


def rubber(name: str = "MI_Rubber_Black", color: str = "#1E1F20", rough: float = 0.8,
           detail: float = 4.0) -> bpy.types.Material:
    """Black rubber: matte, scuffed grey where it is knocked and dragged, dusty."""
    return plastic(name, color, (rough - 0.06, rough + 0.08), "#4A4845", detail, edge=0.003, scratches=0.25)


def galvanized(name: str = "MI_Steel_Galvanized", color: str = "#9EA3A6", detail: float = 3.0) -> bpy.types.Material:
    """Galvanized steel: the zinc's spangle, each crystal its own shade and sheen; dulled by
    patches of white zinc bloom and a darker patina; bright where scratched or rubbed at the edges;
    red rust in a few spots where the zinc has gone; dirt in the corners, dust on top."""
    mat = bpy.data.materials.new(name)
    g = Graph(mat)
    d = detail
    big = g.noise(None, 1.5 * d, 3.0, 0.55)
    mid = g.noise(None, 8.0 * d, 6.0, 0.6)
    fine = g.noise(None, 50.0 * d, 6.0, 0.6)
    spangle_dist, spangle = g.cells(g.warp(None, 0.004, 60.0), 22.0 * d)
    bloom = g.ramp(g.noise(None, 6.0 * d, 8.0, 0.65), 0.55, 0.7)
    patina = g.ramp(big, 0.45, 0.7, 0.0, 0.6)
    scratch = _scratch_lines(g, d, 0.5)
    edge = g.ramp(g.ao(0.006, inside=True), 0.5, 0.95, 1.0, 0.0)
    polish = g.max(g.mul(edge, g.ramp(fine, 0.4, 0.6)), scratch)
    grime = _grime(g, 0.04, mid)
    red = g.mul(g.ramp(g.noise(None, 9.0 * d, 6.0, 0.6), 0.7, 0.73), g.ramp(grime, 0.05, 0.3))
    c = g.mix(spangle, _shade(color, 0.88), _shade(color, 1.08))
    c = g.mix(patina, c, _shade(color, 0.7))
    c = g.mix(bloom, c, "#D4D6D2")
    c = g.mix(polish, c, _shade(color, 1.25))
    c = g.mix(red, c, "#7A3F1C")
    c = g.mix(grime, c, "#2A2622")
    dust = _dust(g, d, 0.3, grime)
    c = g.mix(dust, c, "#7A7263")
    r = g.ramp(spangle, 0.0, 1.0, 0.3, 0.48)
    r = g.mixf(patina, r, 0.55)
    r = g.mixf(bloom, r, 0.72)
    r = g.mixf(polish, r, 0.22)
    r = g.mixf(red, r, 0.85)
    r = g.mixf(g.max(dust, g.mul(grime, 0.6)), r, 0.9)
    m = g.mixf(bloom, 1.0, 0.4)
    m = g.mixf(red, m, 0.2)
    m = g.mixf(g.max(dust, g.mul(grime, 0.7)), m, 0.0)
    h = g.sub(g.add(g.mul(spangle, 0.15), g.mul(g.ramp(spangle_dist, 0.0, 0.05, 0.0, 0.1), 1.0)), g.mul(scratch, 0.5))
    normal = g.bump(g.noise(None, 2.0 * d, 2.0), 0.002)
    normal = g.bump(h, 0.0003, normal)
    return g.finish(c, r, m, normal)


def emissive(name: str, color: str, strength: float) -> bpy.types.Material:
    mat = flat_material(name, color, 0.05)
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Emission Color"].default_value = hex_rgb(color)
    bsdf.inputs["Emission Strength"].default_value = strength
    return mat


def concrete(name: str = "Concrete", color: str = "#4A4743") -> bpy.types.Material:
    """The previews' floor: concrete with aggregate, stains and wear."""
    mat = bpy.data.materials.new(name)
    g = Graph(mat)
    big = g.noise(None, 0.3, 4.0, 0.55)
    mid = g.noise(None, 2.0, 6.0, 0.6)
    grit = g.cells(None, 260.0)[1]
    c = g.mix(g.ramp(big, 0.3, 0.7), _shade(color, 0.75), _shade(color, 1.1))
    c = g.mix(g.ramp(mid, 0.55, 0.7, 0.0, 0.6), c, _shade(color, 0.6, "#2A2218", 0.3))
    c = g.mix(g.ramp(grit, 0.7, 1.0, 0.0, 0.5), c, _shade(color, 1.25))
    r = g.ramp(mid, 0.3, 0.7, 0.78, 0.95)
    normal = g.bump(g.add(g.mul(grit, 0.3), g.noise(None, 40.0, 4.0)), 0.0015)
    return g.finish(c, r, 0.0, normal)
