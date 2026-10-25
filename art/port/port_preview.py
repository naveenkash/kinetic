"""Daylight preview renders of the port (art/port/port_world.py) round a stand-in Yard, to judge it
by: it imports every art/out/port/SM_Port_*.fbx, places the instanced types from instances.json,
draws the Yard's grey boxes from art/kit/yard_layout.json, gives each slot its art/surfaces textures
(UV'd in metres, so at their true size) and the water its normal map, and renders with Cycles under a
sun and a physical sky.

    blender -b --factory-startup -P art/port/port_preview.py -- --out DIR [--views aerial,north,south,harbour]
        [--res 1600] [--samples 48] [--sun-elev 32] [--sun-az 215]
"""
from __future__ import annotations

import glob
import json
import math
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "..", "kit"), os.path.join(HERE, "..", "lib")]

import bmesh  # noqa: E402
import bpy  # noqa: E402
from mathutils import Euler, Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402

PORT = os.path.join(HERE, "..", "out", "port")
SURFACES = os.path.join(HERE, "..", "out", "surfaces")
LAYOUT = os.path.join(HERE, "..", "kit", "yard_layout.json")

FLAT = {"Glass": ("#0A0D10", 0.04, 0.0), "Rubber": ("#151515", 0.85, 0.0), "Timber": ("#5E4A32", 0.85, 0.0),
        "Lamp": ("#FFD9A0", 0.4, 0.0)}

VIEWS = {  # name: (camera, target, lens mm)
    "aerial": ((-700.0, -1000.0, 560.0), (-40.0, 160.0, 0.0), 30.0),
    "north": ((10.0, 12.0, 8.0), (-20.0, 600.0, 22.0), 24.0),
    "south": ((0.0, -12.0, 8.0), (0.0, -2000.0, 40.0), 24.0),
    "harbour": ((700.0, 1100.0, 200.0), (-60.0, 0.0, 0.0), 30.0),
    "zoom": ((-160.0, -260.0, 120.0), (0.0, 60.0, 0.0), 28.0),
    "portal": ((55.0, -26.0, 2.5), (130.0, -30.0, -1.0), 30.0),
    "ship": ((-60.0, 140.0, 22.0), (-200.0, 205.0, 12.0), 28.0),
    "hulk": ((10.0, 150.0, 12.0), (110.0, 205.0, 4.0), 28.0),
    "gate": ((0.0, -560.0, 14.0), (0.0, -420.0, 4.0), 28.0),
    "east": ((150.0, -80.0, 25.0), (330.0, 20.0, 0.0), 28.0),
    "bow": ((40.0, 290.0, 8.0), (-40.0, 205.0, 2.0), 30.0),
    "stern": ((-360.0, 270.0, 12.0), (-280.0, 205.0, 4.0), 30.0),
    "cut": ((20.0, 245.0, 14.0), (60.0, 202.0, 2.0), 30.0),
}


def surface_material(name: str) -> bpy.types.Material:
    mat = bpy.data.materials.new(f"P_{name}")
    mat.use_nodes = True
    nt = mat.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    if name in FLAT:
        col, rough, metal = FLAT[name]
        bsdf.inputs["Base Color"].default_value = ka.hex_rgb(col)
        bsdf.inputs["Roughness"].default_value = rough
        if name == "Lamp":
            bsdf.inputs["Emission Color"].default_value = ka.hex_rgb(col)
            bsdf.inputs["Emission Strength"].default_value = 2.0
        if name == "Glass":
            bsdf.inputs["Specular IOR Level"].default_value = 0.8
        return mat
    if name == "Water":
        return water_material(mat)
    info = json.load(open(os.path.join(SURFACES, name, "surface.json")))
    tile = info["tile_m"]
    tc = nt.nodes.new("ShaderNodeTexCoord")
    mp = nt.nodes.new("ShaderNodeMapping")
    mp.inputs["Scale"].default_value = (1 / tile, 1 / tile, 1.0)
    nt.links.new(tc.outputs["UV"], mp.inputs["Vector"])

    def image(kind, colour):
        node = nt.nodes.new("ShaderNodeTexImage")
        node.image = bpy.data.images.load(os.path.join(SURFACES, name, info["maps"][kind]))
        node.image.colorspace_settings.name = "sRGB" if colour else "Non-Color"
        nt.links.new(mp.outputs["Vector"], node.inputs["Vector"])
        return node

    nt.links.new(image("BaseColor", True).outputs["Color"], bsdf.inputs["Base Color"])
    orm = image("ORM", False)
    sep = nt.nodes.new("ShaderNodeSeparateColor")
    nt.links.new(orm.outputs["Color"], sep.inputs["Color"])
    nt.links.new(sep.outputs["Green"], bsdf.inputs["Roughness"])
    nt.links.new(sep.outputs["Blue"], bsdf.inputs["Metallic"])
    nm = nt.nodes.new("ShaderNodeNormalMap")
    nt.links.new(image("Normal", False).outputs["Color"], nm.inputs["Color"])
    nt.links.new(nm.outputs["Normal"], bsdf.inputs["Normal"])
    return mat


def water_material(mat):
    nt = mat.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.012, 0.03, 0.035, 1.0)
    bsdf.inputs["IOR"].default_value = 1.33
    tc = nt.nodes.new("ShaderNodeTexCoord")
    mp = nt.nodes.new("ShaderNodeMapping")
    mp.inputs["Scale"].default_value = (1 / 8.0, 1 / 8.0, 1.0)
    nt.links.new(tc.outputs["UV"], mp.inputs["Vector"])
    nrm = nt.nodes.new("ShaderNodeTexImage")
    nrm.image = bpy.data.images.load(os.path.join(PORT, "T_Water_Normal.png"))
    nrm.image.colorspace_settings.name = "Non-Color"
    rough = nt.nodes.new("ShaderNodeTexImage")
    rough.image = bpy.data.images.load(os.path.join(PORT, "T_Water_Roughness.png"))
    rough.image.colorspace_settings.name = "Non-Color"
    for n in (nrm, rough):
        nt.links.new(mp.outputs["Vector"], n.inputs["Vector"])
    nm = nt.nodes.new("ShaderNodeNormalMap")
    nm.inputs["Strength"].default_value = 0.6
    nt.links.new(nrm.outputs["Color"], nm.inputs["Color"])
    nt.links.new(nm.outputs["Normal"], bsdf.inputs["Normal"])
    nt.links.new(rough.outputs["Color"], bsdf.inputs["Roughness"])
    return mat


def import_port(mats: dict) -> dict:
    """Every port mesh; the instanced types are returned hidden, by type name."""
    types = {}
    for path in sorted(glob.glob(os.path.join(PORT, "SM_Port_*.fbx"))):
        before = set(bpy.data.objects)
        bpy.ops.import_scene.fbx(filepath=path)
        for obj in set(bpy.data.objects) - before:
            for slot in obj.material_slots:
                base = slot.material.name.split(".")[0] if slot.material else "Concrete"
                slot.material = mats.get(base, mats["Concrete"])
            name = os.path.splitext(os.path.basename(path))[0]
            types[name] = obj
    return types


def place_instances(types: dict):
    data = json.load(open(os.path.join(PORT, "instances.json")))
    used = {i["type"] for i in data["instances"]}
    col = bpy.data.collections.new("Instances")
    bpy.context.scene.collection.children.link(col)
    for kind in used:
        types[kind].hide_render = True
        types[kind].hide_set(True)
    for i, inst in enumerate(data["instances"]):
        src = types[inst["type"]]
        obj = bpy.data.objects.new(f"I{i}", src.data)
        obj.location = inst["location"]
        obj.rotation_euler = Euler((0.0, 0.0, math.radians(inst["yaw"])))
        obj.scale = (inst["scale"],) * 3
        col.objects.link(obj)
    print(f"placed {len(data['instances'])} instances")


def stand_in_yard(mats: dict):
    """The Yard's grey boxes, coloured by role."""
    layout = json.load(open(LAYOUT))
    role_mat = {"floor": "Concrete", "trench": "Slag", "mesa": "Rust", "ledge": "Rust", "pit": "DarkSteel", "bridge": "Grating",
                "rails": "Grating", "placeholder": "HTROrange", "ladder": "Ochre"}
    bm = bmesh.new()
    names = list(mats)
    for b in layout["boxes"]:
        mn, mx = Vector(b["min"]), Vector(b["max"])
        res = bmesh.ops.create_cube(bm, size=1.0)
        for v in res["verts"]:
            v.co = Vector(((mn.x + mx.x) / 2 + v.co.x * (mx.x - mn.x), (mn.y + mx.y) / 2 + v.co.y * (mx.y - mn.y),
                           (mn.z + mx.z) / 2 + v.co.z * (mx.z - mn.z)))
        label = b["label"]
        surf = role_mat.get(b["role"], "HullSteel")
        if label.startswith("Wall"):
            surf = "Corrugated"
        idx = names.index(surf)
        for f in {f for v in res["verts"] for f in v.link_faces}:
            f.material_index = idx
    mesh = bpy.data.meshes.new("Yard")
    bm.to_mesh(mesh)
    bm.free()
    uv = mesh.uv_layers.new()
    for poly in mesh.polygons:
        n = poly.normal
        ax = max(range(3), key=lambda k: abs(n[k]))
        a, c = {0: (1, 2), 1: (0, 2), 2: (0, 1)}[ax]
        for li in poly.loop_indices:
            co = mesh.vertices[mesh.loops[li].vertex_index].co
            uv.data[li].uv = (co[a], co[c])
    for s in names:
        mesh.materials.append(mats[s])
    obj = bpy.data.objects.new("Yard", mesh)
    bpy.context.scene.collection.objects.link(obj)


def world(sun_elev: float = 32.0, sun_az: float = 215.0):
    scene = bpy.context.scene
    w = bpy.data.worlds.new("Sky")
    scene.world = w
    w.use_nodes = True
    nt = w.node_tree
    sky = nt.nodes.new("ShaderNodeTexSky")
    sky.sky_type = "MULTIPLE_SCATTERING"
    sky.sun_elevation = math.radians(sun_elev)
    sky.sun_rotation = math.radians(sun_az)
    sky.sun_disc = False
    for attr, val in (("air_density", 1.0), ("aerosol_density", 2.0), ("dust_density", 2.0), ("ozone_density", 1.0)):
        if hasattr(sky, attr):
            setattr(sky, attr, val)
    bg = nt.nodes["Background"]
    bg.inputs["Strength"].default_value = 0.35
    nt.links.new(sky.outputs["Color"], bg.inputs["Color"])
    haze()
    sun = bpy.data.lights.new("Sun", "SUN")
    sun.energy = 4.0
    sun.angle = math.radians(0.6)
    sun.color = (1.0, 0.96, 0.9)
    so = bpy.data.objects.new("Sun", sun)
    so.rotation_euler = Euler((math.radians(90 - sun_elev), 0.0, math.radians(sun_az + 90.0)))
    scene.collection.objects.link(so)
    scene.view_settings.view_transform = "AgX"
    scene.view_settings.look = "AgX - Medium High Contrast" if "AgX - Medium High Contrast" in [
        i.identifier for i in scene.view_settings.bl_rna.properties["look"].enum_items] else "None"


def haze(density: float = 0.00006):
    """A thin haze for the distance: a scattering volume in a slab of air over the whole scene (a
    world volume would put out the sun, whose rays never leave it)."""
    mesh = bpy.data.meshes.new("Haze")
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector((v.co.x * 24000.0, v.co.y * 24000.0, (v.co.z + 0.5) * 1600.0 - 30.0))
    bm.to_mesh(mesh)
    bm.free()
    mat = bpy.data.materials.new("Haze")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.remove(nt.nodes["Principled BSDF"])
    vol = nt.nodes.new("ShaderNodeVolumePrincipled")
    vol.inputs["Density"].default_value = density
    vol.inputs["Color"].default_value = (0.78, 0.84, 0.92, 1.0)
    vol.inputs["Anisotropy"].default_value = 0.4
    nt.links.new(vol.outputs["Volume"], nt.nodes["Material Output"].inputs["Volume"])
    mesh.materials.append(mat)
    obj = bpy.data.objects.new("Haze", mesh)
    bpy.context.scene.collection.objects.link(obj)


def camera(name: str, eye, target, lens: float):
    cam = bpy.data.cameras.new(name)
    cam.lens = lens
    cam.clip_start = 0.5
    cam.clip_end = 30000.0
    obj = bpy.data.objects.new(name, cam)
    bpy.context.scene.collection.objects.link(obj)
    obj.location = eye
    d = Vector(target) - Vector(eye)
    obj.rotation_euler = d.to_track_quat("-Z", "Y").to_euler()
    return obj


def main() -> int:
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []

    def arg(name, default):
        return argv[argv.index(name) + 1] if name in argv else default

    out = arg("--out", os.path.join(PORT, "preview"))
    views = arg("--views", "aerial,north,south,harbour").split(",")
    res = int(arg("--res", "1600"))
    samples = int(arg("--samples", "48"))
    t0 = time.time()
    ka.reset_scene()
    scene = bpy.context.scene
    names = ["Concrete", "Slag", "HullSteel", "Rust", "Corrugated", "Grating", "Hazard", "Ochre", "HTROrange", "DarkSteel",
             "Glass", "Rubber", "Timber", "Lamp", "Water"]
    mats = {n: surface_material(n) for n in names}
    types = import_port(mats)
    place_instances(types)
    stand_in_yard(mats)
    world(float(arg("--sun-elev", "32")), float(arg("--sun-az", "215")))
    scene.render.resolution_x = res
    scene.render.resolution_y = int(res * 9 / 16)
    scene.cycles.samples = samples
    scene.cycles.use_denoising = True
    scene.cycles.max_bounces = 4
    scene.cycles.volume_bounces = 0
    scene.cycles.volume_step_rate = 8.0
    os.makedirs(out, exist_ok=True)
    print(f"scene ready in {time.time() - t0:.0f} s")
    for v in views:
        eye, target, lens = VIEWS[v]
        scene.camera = camera(v, eye, target, lens)
        scene.render.filepath = os.path.join(out, f"port_{v}.png")
        t = time.time()
        bpy.ops.render.render(write_still=True)
        print(f"rendered {v} in {time.time() - t:.0f} s")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
