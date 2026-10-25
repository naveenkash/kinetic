"""Imports the generated props (art/out) into the Unreal project.

    make -C game import-props

(a headless editor running this with -ExecutePythonScript; in an open editor, run it from the
Python console or File > Execute Python Script)

For every art/out/<asset>/<mesh>/ that has a full build (an .fbx and its report.json):

- the static mesh goes to /Game/Objects/<Tier>/<Mesh>/, the tier taken from the name
  (SM_Light_, SM_Medium_, SM_Heavy_, SM_Rated_, SM_Volatile_). Its sockets come from the
  SOCKET_ empties, its collision from the UCX_ meshes, and its scale from the FBX's metres;
- the baked textures are imported next to it with the right settings: BaseColor sRGB, ORM as
  masks, and Normal as a normal map with green flipped, since Blender bakes OpenGL;
- a material instance of M_KineticProp is assigned to it: M_KineticProp_Masked for a masked
  card, M_KineticProp_TwoSided for a single-sheet mesh seen from inside (a container's walls);
- a part with its own pivot (a container door) comes from its own FBX, with that pivot.

The master materials are made here too, so the whole Content folder can be rebuilt from code.
It writes game/Saved/Kinetic/import_report.json: per mesh, its size in metres, collision
primitives and triangles beside Blender's, and every socket Blender made, found on the mesh at the
same place (Blender's Y is Unreal's -Y). It fails if a socket is missing or moved, or a mesh has
no collision.
"""
from __future__ import annotations

import glob
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
ART_OUT = os.path.join(REPO, "art", "out")
TIERS = {"Light", "Medium", "Heavy", "Rated", "Volatile"}
MATERIALS = "/Game/Core/Materials"

DEFAULT_ORM = f"{MATERIALS}/T_Default_ORM"

assets = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def write_png(path: str, rgb: tuple[int, int, int], size: int = 4) -> None:
    """A flat-colour PNG with the standard library only (the editor's Python has no imaging)."""
    import struct
    import zlib

    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    row = b"\x00" + bytes(rgb) * size
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(row * size)) + chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def default_orm() -> unreal.Texture2D:
    """The ORM a mesh without one falls back to: no occlusion, roughness 0.5, not metal. A masks
    texture, as the ORM sampler requires."""
    out = os.path.join(REPO, "game", "Saved", "Kinetic")
    os.makedirs(out, exist_ok=True)
    png = os.path.join(out, "T_Default_ORM.png")
    write_png(png, (255, 128, 0))
    texture = unreal.load_asset(import_task(png, MATERIALS)[0])
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
    texture.set_editor_property("srgb", False)
    eal.save_loaded_asset(texture)
    return texture


def master_material(name: str, masked: bool, two_sided: bool = False) -> unreal.Material:
    """BaseColor, ORM (R occlusion, G roughness, B metallic), Normal and Emissive texture
    parameters, and EmissiveStrength (0 unless the mesh glows)."""
    path = f"{MATERIALS}/{name}"
    # Rebuilt in place on a re-run: the instances that use it keep their parent.
    if eal.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = assets.create_asset(name, MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    if masked:
        mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
        mat.set_editor_property("opacity_mask_clip_value", 0.5)
    mat.set_editor_property("two_sided", masked or two_sided)

    def sampler(param: str, x: int, y: int, kind, default: str):
        node = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
        node.set_editor_property("parameter_name", param)
        node.set_editor_property("sampler_type", kind)
        node.set_editor_property("texture", unreal.load_asset(default))
        return node


    base = sampler("BaseColor", -600, -300, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
                   "/Engine/EngineResources/WhiteSquareTexture")
    orm = sampler("ORM", -600, 0, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, DEFAULT_ORM)
    normal = sampler("Normal", -600, 300, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
                     "/Engine/EngineMaterials/DefaultNormal")
    lib.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_property(orm, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
    lib.connect_material_property(orm, "G", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.connect_material_property(orm, "B", unreal.MaterialProperty.MP_METALLIC)
    lib.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)
    glow = sampler("Emissive", -600, 600, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
                   "/Engine/EngineResources/Black")
    strength = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 850)
    strength.set_editor_property("parameter_name", "EmissiveStrength")
    strength.set_editor_property("default_value", 0.0)
    times = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 650)
    lib.connect_material_expressions(glow, "RGB", times, "A")
    lib.connect_material_expressions(strength, "", times, "B")
    lib.connect_material_property(times, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if masked:
        lib.connect_material_property(base, "A", unreal.MaterialProperty.MP_OPACITY_MASK)
    errors = lib.recompile_material(mat)
    if errors:
        raise RuntimeError(f"{name} does not compile: {list(errors)}")
    eal.save_loaded_asset(mat)
    return mat


def import_task(filename: str, destination: str, options=None) -> list[str]:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    if options is not None:
        task.set_editor_property("options", options)
    assets.import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


def import_texture(png: str, destination: str) -> unreal.Texture2D:
    texture = unreal.load_asset(import_task(png, destination)[0])
    kind = os.path.splitext(png)[0].rsplit("_", 1)[-1]
    if kind == "ORM":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property("srgb", False)
    elif kind == "Normal":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("flip_green_channel", True)
    eal.save_loaded_asset(texture)
    return texture


def fbx_options() -> unreal.FbxImportUI:
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", False)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("import_animations", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    data = ui.get_editor_property("static_mesh_import_data")
    # Every option the import depends on is set here: the rest come from each user's saved
    # import settings.
    # One mesh per file: a part with its own pivot (a container door) comes in its own FBX with
    # that pivot at the origin (art/lib/kinetic_art.py export), so baking node transforms into
    # the vertices, which also applies the file's units, keeps every pivot.
    data.set_editor_property("combine_meshes", True)
    data.set_editor_property("transform_vertex_to_absolute", True)
    data.set_editor_property("import_uniform_scale", 1.0)
    data.set_editor_property("import_translation", unreal.Vector(0, 0, 0))
    data.set_editor_property("import_rotation", unreal.Rotator(0, 0, 0))
    data.set_editor_property("auto_generate_collision", False)  # The UCX_ meshes are the collision
    data.set_editor_property("one_convex_hull_per_ucx", True)
    data.set_editor_property("generate_lightmap_u_vs", True)
    data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS)
    data.set_editor_property("convert_scene_unit", True)
    return ui


def describe(mesh: unreal.StaticMesh, sockets: dict[str, list[float]], normals: dict[str, list[float]],
             bounds_m) -> tuple[dict, list[str]]:
    """What Unreal made of the mesh, and what disagrees with Blender: its size, its sockets'
    places and facings (X along the outward normal), and whether it has collision."""
    meshes = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    bounds = mesh.get_bounding_box()
    size = bounds.max - bounds.min
    problems = []
    found = {}
    for name, (x, y, z) in sockets.items():
        socket = mesh.find_socket(name)
        if socket is None:
            problems.append(f"socket {name} missing")
            continue
        at = socket.get_editor_property("relative_location")
        found[name] = [round(at.x / 100, 4), round(at.y / 100, 4), round(at.z / 100, 4)]
        if max(abs(at.x / 100 - x), abs(at.y / 100 + y), abs(at.z / 100 - z)) > 0.002:
            problems.append(f"socket {name} at {found[name]}, Blender put it at {[x, -y, z]} (Unreal axes)")
        if name in normals:
            facing = socket.get_editor_property("relative_rotation").get_forward_vector()
            nx, ny, nz = normals[name]
            if facing.x * nx - facing.y * ny + facing.z * nz < 0.999:
                problems.append(f"socket {name} faces {[round(facing.x, 3), round(facing.y, 3), round(facing.z, 3)]}, "
                                f"Blender's normal is {[nx, -ny, nz]} (Unreal axes)")
    if bounds_m and max(abs(a - b) for a, b in zip((size.x / 100, size.y / 100, size.z / 100), bounds_m)) > 0.002:
        problems.append(f"size {[round(size.x / 100, 3), round(size.y / 100, 3), round(size.z / 100, 3)]} m, "
                        f"Blender's {bounds_m}")
    # Simple shapes and UCX convex hulls are counted apart.
    collision = meshes.get_simple_collision_count(mesh) + meshes.get_convex_collision_count(mesh)
    if collision == 0:
        problems.append("no collision")
    return {
        "size_m": [round(size.x / 100, 3), round(size.y / 100, 3), round(size.z / 100, 3)],
        "sockets": found,
        "collision_primitives": collision,
        "triangles": mesh.get_num_triangles(0),
    }, problems


def main() -> None:
    # The legacy FBX importer honours FbxImportUI; Interchange would ignore these options.
    unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX False")
    # /Game/Objects is all generated: start it clean, so a renamed or dropped asset leaves nothing.
    if eal.does_directory_exist("/Game/Objects"):
        eal.delete_directory("/Game/Objects")
    default_orm()
    masters = {(False, False): master_material("M_KineticProp", False),
               (False, True): master_material("M_KineticProp_TwoSided", False, two_sided=True),
               (True, False): master_material("M_KineticProp_Masked", True),
               (True, True): None}
    masters[(True, True)] = masters[(True, False)]  # Masked cards are two-sided already
    report = {}
    problems = []
    for path in sorted(glob.glob(os.path.join(ART_OUT, "*", "*", "report.json"))):
        with open(path) as f:
            built = json.load(f)
        name = built["name"]
        folder = os.path.dirname(path)
        fbx = os.path.join(folder, f"{name}.fbx")
        if not os.path.exists(fbx):
            unreal.log_warning(f"{name}: no full build (preview only), skipped")
            continue
        files = [fbx] + sorted(built.get("exports", {}).get("fbx_parts", {}).values())
        tier = name.split("_")[1] if name.count("_") >= 2 else ""
        if tier not in TIERS:
            unreal.log_error(f"{name}: the name carries no tier (SM_<Tier>_...), skipped")
            continue
        destination = f"/Game/Objects/{tier}/{name}"
        imported = [unreal.load_asset(p) for f in files for p in import_task(f, destination, fbx_options())]
        meshes = [m for m in imported if isinstance(m, unreal.StaticMesh)]
        for mesh in meshes:
            mesh_name = mesh.get_name()
            textures = {}
            for png in glob.glob(os.path.join(folder, f"{mesh_name}_*.png")):
                kind = os.path.splitext(png)[0][len(os.path.join(folder, mesh_name)) + 1:]
                if kind in ("BaseColor", "ORM", "Normal", "Emissive"):
                    textures[kind] = import_texture(png, f"{destination}/Textures")
            masked = mesh_name in built.get("masked", [])
            two_sided = mesh_name in built.get("double_sided", [])
            instance_name = f"MI_{mesh_name}"
            instance_path = f"{destination}/{instance_name}"
            if eal.does_asset_exist(instance_path):
                instance = unreal.load_asset(instance_path)
            else:
                instance = assets.create_asset(instance_name, destination, unreal.MaterialInstanceConstant,
                                               unreal.MaterialInstanceConstantFactoryNew())
            lib.set_material_instance_parent(instance, masters[(masked, two_sided)])
            for kind, texture in textures.items():
                lib.set_material_instance_texture_parameter_value(instance, kind, texture)
            glow = built.get("emission_strength", {}).get(mesh_name, 0.0)
            lib.set_material_instance_scalar_parameter_value(instance, "EmissiveStrength", glow)
            eal.save_loaded_asset(instance)
            mesh.set_material(0, instance)
            eal.save_loaded_asset(mesh)
            # Sockets belong to the first mesh (an asset's body); doors and other parts carry none.
            sockets = built.get("socket_locations_m", {}) if mesh_name == name else {}
            facts, issues = describe(mesh, sockets, built.get("socket_normals", {}),
                                     built.get("bounds_m", {}).get(mesh_name))
            report[mesh_name] = facts | {"blender": {"bounds_m": built.get("bounds_m", {}).get(mesh_name),
                                                     "triangles": built["triangles"].get(mesh_name)}}
            if facts["triangles"] != built["triangles"].get(mesh_name):
                issues.append(f"{facts['triangles']} tris, Blender's {built['triangles'].get(mesh_name)}")
            problems += [f"{mesh_name}: {p}" for p in issues]
    out = os.path.join(REPO, "game", "Saved", "Kinetic")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "import_report.json"), "w") as f:
        json.dump(report, f, indent=2)
    unreal.log(f"Kinetic: imported {len(report)} meshes; report in {out}/import_report.json")
    for p in problems:
        unreal.log_error(p)
    if problems:
        raise RuntimeError(f"{len(problems)} import problems")


def run() -> None:
    """Runs main(). In an unattended editor (make), it then ends the process with main()'s
    status, 0 or 1: everything is saved by then, and the 5.8.4 editor's own shutdown, after a
    script has shown its import notifications, crashes in Slate's teardown on macOS and opens
    the crash reporter."""
    unattended = "-unattended" in unreal.SystemLibrary.get_command_line().lower()
    status = 0
    try:
        main()
    except Exception:
        import traceback
        for line in traceback.format_exc().splitlines():
            unreal.log_error(line)
        status = 1
    if unattended:
        unreal.SystemLibrary.execute_console_command(None, "FLUSHLOG")
        os._exit(status)


run()
