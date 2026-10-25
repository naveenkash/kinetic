"""Imports an arena's visual kit (art/kit, art/surfaces) into Unreal.

    make -C game import-kit

(a headless editor running this with -ExecutePythonScript, after the surfaces and the kit are
built with Blender). It makes:
- the tiling surfaces: each art/out/surfaces/<Name> set as textures (BaseColor sRGB, ORM as masks,
  Normal with green flipped, since Blender bakes OpenGL) under /Game/Arenas/YRD_Yard/Kit/Surfaces,
  and an instance of M_KineticSurface that repeats them at the surface's own size (the kit is UV'd
  in metres);
- flat surfaces for what has no tiling set: glass, rubber, timber, and the lamps' glow;
- every art/out/kit/SM_Yard_*.fbx as a static mesh with Nanite on and no collision (the grey
  boxes under it keep the collision), each slot given the surface it is named after;
- the same for the moving machinery, art/out/machines/SM_Machine_*.fbx (art/machines), under
  /Game/Arenas/YRD_Yard/Machines; KineticYardDirector draws them over the boxes it moves.
build_yard_greybox.py then places the meshes and stops drawing the boxes they cover.
"""
from __future__ import annotations

import glob
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
SURFACES = os.path.join(REPO, "art", "out", "surfaces")
KIT = os.path.join(REPO, "art", "out", "kit")
DEST = "/Game/Arenas/YRD_Yard/Kit"
MACHINES = os.path.join(REPO, "art", "out", "machines")
MACHINES_DEST = "/Game/Arenas/YRD_Yard/Machines"

assets = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary

# Surfaces with no tiling set: (base colour, roughness, metallic, emissive strength).
FLAT = {
    "Glass": ((0.004, 0.005, 0.006), 0.05, 0.0, 0.0),
    "Rubber": ((0.012, 0.012, 0.012), 0.9, 0.0, 0.0),
    "Timber": ((0.12, 0.075, 0.04), 0.85, 0.0, 0.0),
    "Lamp": ((1.0, 0.72, 0.38), 0.3, 0.0, 30.0),
}


def task(filename: str, destination: str, options=None) -> list[str]:
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", filename)
    t.set_editor_property("destination_path", destination)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", True)
    if options is not None:
        t.set_editor_property("options", options)
    assets.import_asset_tasks([t])
    return list(t.get_editor_property("imported_object_paths"))


def material(name: str, build) -> unreal.Material:
    path = f"{DEST}/{name}"
    if eal.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = assets.create_asset(name, DEST, unreal.Material, unreal.MaterialFactoryNew())
    build(mat)
    errors = lib.recompile_material(mat)
    if errors:
        raise RuntimeError(f"{name} does not compile: {list(errors)}")
    eal.save_loaded_asset(mat)
    return mat


def surface_master(mat) -> None:
    """BaseColor, ORM and Normal textures repeated every Tile metres of UV (the kit's UVs are metres)."""
    coord = lib.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1000, 0)
    tile = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -1000, 150)
    tile.set_editor_property("parameter_name", "Tile")
    tile.set_editor_property("default_value", 2.0)
    uv = lib.create_material_expression(mat, unreal.MaterialExpressionDivide, -800, 50)
    lib.connect_material_expressions(coord, "", uv, "A")
    lib.connect_material_expressions(tile, "", uv, "B")

    def sample(param: str, y: int, kind, default: str):
        n = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -500, y)
        n.set_editor_property("parameter_name", param)
        n.set_editor_property("sampler_type", kind)
        n.set_editor_property("texture", unreal.load_asset(default))
        lib.connect_material_expressions(uv, "", n, "UVs")
        return n

    base = sample("BaseColor", -300, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, "/Engine/EngineResources/WhiteSquareTexture")
    orm = sample("ORM", 0, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, "/Engine/EngineResources/WhiteSquareTexture")
    normal = sample("Normal", 300, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, "/Engine/EngineMaterials/DefaultNormal")
    lib.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_property(orm, "G", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.connect_material_property(orm, "B", unreal.MaterialProperty.MP_METALLIC)
    lib.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)
    mat.set_editor_property("used_with_nanite", True)


def flat_master(mat) -> None:
    def vec(name, y, default):
        n = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, y)
        n.set_editor_property("parameter_name", name)
        n.set_editor_property("default_value", default)
        return n

    def scal(name, y, default):
        n = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, y)
        n.set_editor_property("parameter_name", name)
        n.set_editor_property("default_value", default)
        return n

    colour = vec("BaseColor", -200, unreal.LinearColor(0.5, 0.5, 0.5, 1))
    lib.connect_material_property(colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_property(scal("Roughness", 0, 0.6), "", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.connect_material_property(scal("Metallic", 100, 0.0), "", unreal.MaterialProperty.MP_METALLIC)
    glow = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 250)
    lib.connect_material_expressions(colour, "", glow, "A")
    lib.connect_material_expressions(scal("EmissiveStrength", 300, 0.0), "", glow, "B")
    lib.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mat.set_editor_property("used_with_nanite", True)


def instance(name: str, parent) -> unreal.MaterialInstanceConstant:
    path = f"{DEST}/{name}"
    mi = unreal.load_asset(path) if eal.does_asset_exist(path) else assets.create_asset(
        name, DEST, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, parent)
    return mi


def surfaces() -> dict:
    surface = material("M_KineticSurface", surface_master)
    flat = material("M_KineticFlat", flat_master)
    out = {}
    for info_path in sorted(glob.glob(os.path.join(SURFACES, "*", "surface.json"))):
        info = json.load(open(info_path))
        name = info["name"]
        folder = f"{DEST}/Surfaces/{name}"
        mi = instance(f"MI_Surface_{name}", surface)
        for kind, png in info["maps"].items():
            texture = unreal.load_asset(task(os.path.join(os.path.dirname(info_path), png), folder)[0])
            if kind == "ORM":
                texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
                texture.set_editor_property("srgb", False)
            elif kind == "Normal":
                texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
                texture.set_editor_property("srgb", False)
                texture.set_editor_property("flip_green_channel", True)
            eal.save_loaded_asset(texture)
            lib.set_material_instance_texture_parameter_value(mi, kind, texture)
        lib.set_material_instance_scalar_parameter_value(mi, "Tile", info["tile_m"])
        eal.save_loaded_asset(mi)
        out[name] = mi
    for name, (rgb, rough, metal, glow) in FLAT.items():
        mi = instance(f"MI_Surface_{name}", flat)
        lib.set_material_instance_vector_parameter_value(mi, "BaseColor", unreal.LinearColor(*rgb, 1.0))
        lib.set_material_instance_scalar_parameter_value(mi, "Roughness", rough)
        lib.set_material_instance_scalar_parameter_value(mi, "Metallic", metal)
        lib.set_material_instance_scalar_parameter_value(mi, "EmissiveStrength", glow)
        eal.save_loaded_asset(mi)
        out[name] = mi
    return out


def fbx_options() -> unreal.FbxImportUI:
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", False)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("import_animations", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    data = ui.get_editor_property("static_mesh_import_data")
    data.set_editor_property("combine_meshes", True)
    data.set_editor_property("transform_vertex_to_absolute", True)
    data.set_editor_property("auto_generate_collision", False)  # The grey boxes are the collision
    data.set_editor_property("generate_lightmap_u_vs", False)
    data.set_editor_property("convert_scene_unit", True)
    data.set_editor_property("build_nanite", True)
    data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
    return ui


def import_meshes(fbxs: list[str], dest: str, mis: dict, problems: list[str]) -> None:
    for fbx in fbxs:
        name = os.path.splitext(os.path.basename(fbx))[0]
        paths = task(fbx, dest, fbx_options())
        mesh = next((unreal.load_asset(p) for p in paths if p.split(".")[0].endswith(name)), None)
        if not isinstance(mesh, unreal.StaticMesh):
            problems.append(f"{name}: not imported ({paths})")
            continue
        for i, slot in enumerate(mesh.get_editor_property("static_materials")):
            key = str(slot.get_editor_property("material_slot_name"))
            mi = mis.get(key)
            if mi is None:
                problems.append(f"{name}: slot {key} has no surface")
                continue
            mesh.set_material(i, mi)
        settings = mesh.get_editor_property("nanite_settings")
        settings.set_editor_property("enabled", True)
        mesh.set_editor_property("nanite_settings", settings)
        eal.save_loaded_asset(mesh)
        unreal.log(f"Kinetic: {name}, {len(mesh.get_editor_property('static_materials'))} surfaces")


def main() -> None:
    mis = surfaces()
    meshes = sorted(glob.glob(os.path.join(KIT, "SM_Yard_*.fbx")))
    if not meshes:
        raise RuntimeError(f"no kit in {KIT} (blender -b -P art/kit/yard_kit.py)")
    problems = []
    import_meshes(meshes, DEST, mis, problems)
    # The moving machinery (art/machines), which KineticYardDirector puts over its boxes.
    import_meshes(sorted(glob.glob(os.path.join(MACHINES, "SM_Machine_*.fbx"))), MACHINES_DEST, mis, problems)
    for p in problems:
        unreal.log_error(p)
    if problems:
        raise RuntimeError(f"{len(problems)} kit import problems")


def run() -> None:
    """Runs main(); in an unattended editor, ends the process with its status (see import_props.py)."""
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
