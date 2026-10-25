"""Imports the port round The Yard (art/port, built with Blender) into Unreal:

    make -C game import-port

(a headless editor running this, after make import-kit has made the surfaces). It makes:
- M_PortWater: open water from art/out/port's tiling wave normal and roughness, the normal panned
  at two scales and directions so the swell never repeats visibly, under a deep green-grey;
- every art/out/port/SM_Port_*.fbx as a Nanite static mesh with no collision (the port is
  backdrop: players never reach it), each slot given the kit surface it is named after, the
  Water slot the water.
build_yard_greybox.py then places the meshes, and instances the repeated ones (containers,
cranes, light towers, bollards) from art/out/port/instances.json.
"""
from __future__ import annotations

import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import unreal  # noqa: E402

import import_kit as kit  # noqa: E402

PORT = os.path.join(kit.REPO, "art", "out", "port")
DEST = "/Game/Arenas/YRD_Yard/Port"
lib = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def texture(png: str, normal: bool) -> unreal.Texture2D:
    t = unreal.load_asset(kit.task(os.path.join(PORT, png), f"{DEST}/Water")[0])
    t.set_editor_property("srgb", False)
    if normal:
        t.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        t.set_editor_property("flip_green_channel", True)  # Blender bakes OpenGL
    else:
        t.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_GRAYSCALE)
    eal.save_loaded_asset(t)
    return t


def water_material(normal_tex, rough_tex) -> unreal.Material:
    path = f"{DEST}/M_PortWater"
    if eal.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = kit.assets.create_asset("M_PortWater", DEST, unreal.Material, unreal.MaterialFactoryNew())
    coord = lib.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1400, 0)

    def swell(tile_m: float, speed, y: int):
        uv = lib.create_material_expression(mat, unreal.MaterialExpressionDivide, -1200, y)
        lib.connect_material_expressions(coord, "", uv, "A")
        uv.set_editor_property("const_b", tile_m)
        pan = lib.create_material_expression(mat, unreal.MaterialExpressionPanner, -1000, y)
        pan.set_editor_property("speed_x", speed[0])
        pan.set_editor_property("speed_y", speed[1])
        lib.connect_material_expressions(uv, "", pan, "Coordinate")
        n = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -700, y)
        n.set_editor_property("texture", normal_tex)
        n.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        lib.connect_material_expressions(pan, "", n, "UVs")
        r = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -700, y + 200)
        r.set_editor_property("texture", rough_tex)
        r.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE)
        lib.connect_material_expressions(pan, "", r, "UVs")
        return n, r

    n1, r1 = swell(8.0, (0.012, 0.004), -400)
    n2, r2 = swell(21.0, (-0.004, 0.007), 100)
    added = lib.create_material_expression(mat, unreal.MaterialExpressionAdd, -450, -200)  # Two gentle swells: sum, renormalise
    lib.connect_material_expressions(n1, "RGB", added, "A")
    lib.connect_material_expressions(n2, "RGB", added, "B")
    blend = lib.create_material_expression(mat, unreal.MaterialExpressionNormalize, -300, -200)
    lib.connect_material_expressions(added, "", blend, "VectorInput")
    lib.connect_material_property(blend, "", unreal.MaterialProperty.MP_NORMAL)
    rough = lib.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -400, 200)
    lib.connect_material_expressions(r1, "R", rough, "A")
    lib.connect_material_expressions(r2, "R", rough, "B")
    rough.set_editor_property("const_alpha", 0.5)
    lib.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    colour = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 400)
    colour.set_editor_property("constant", unreal.LinearColor(0.012, 0.03, 0.032, 1.0))  # Harbour water, deep green-grey
    lib.connect_material_property(colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
    spec = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 500)
    spec.set_editor_property("r", 0.5)
    lib.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)
    mat.set_editor_property("used_with_nanite", True)
    errors = lib.recompile_material(mat)
    if errors:
        raise RuntimeError(f"M_PortWater does not compile: {list(errors)}")
    eal.save_loaded_asset(mat)
    return mat


def main() -> None:
    fbxs = sorted(glob.glob(os.path.join(PORT, "SM_Port_*.fbx")))
    if not fbxs:
        raise RuntimeError(f"no port in {PORT} (blender -b -P art/port/port_world.py)")
    mis = {}
    for path in eal.list_assets(f"{kit.DEST}", recursive=False):
        name = path.rsplit("/", 1)[-1].split(".")[0]
        if name.startswith("MI_Surface_"):
            mis[name[len("MI_Surface_"):]] = unreal.load_asset(path)
    if not mis:
        raise RuntimeError("no kit surfaces (make import-kit first)")
    mis["Water"] = water_material(texture("T_Water_Normal.png", True), texture("T_Water_Roughness.png", False))
    problems: list[str] = []
    kit.import_meshes(fbxs, DEST, mis, problems)
    for p in problems:
        unreal.log_error(p)
    if problems:
        raise RuntimeError(f"{len(problems)} port import problems")


def run() -> None:
    status = 0
    try:
        main()
    except Exception:
        import traceback
        for line in traceback.format_exc().splitlines():
            unreal.log_error(line)
        status = 1
    if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.execute_console_command(None, "FLUSHLOG")
        os._exit(status)


if __name__ == "__main__":
    run()
