"""Makes the materials Claim with Tag-Out draws its tells with (docs/proposals/claim-tag-out.md
sections 6 and 8), in /Game/TagOut:

    make -C game import-tagout

(a headless editor running this, after make import-port, whose wave normal the haze borrows):
- M_TagGlint: an unlit, very bright white for the sighting level's lens glint, a plate's sun
  flare and the beacon's reflector flash (it blooms);
- M_TagDust: a dust bag's cloud, unlit translucent dust, thinning to nothing at its edge;
- M_TagHaze: heat haze over hot steel, a clear refracting shimmer from the port's wave normal
  panned upward.
FKineticTagOut loads them by path and falls back to the engine's basic shape material without them.
"""
from __future__ import annotations

import os

import unreal

DEST = "/Game/TagOut"
WAVES = "/Game/Arenas/YRD_Yard/Port/Water/T_Water_Normal"
lib = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary
assets = unreal.AssetToolsHelpers.get_asset_tools()


def fresh(name: str) -> unreal.Material:
    path = f"{DEST}/{name}"
    if eal.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
        return mat
    return assets.create_asset(name, DEST, unreal.Material, unreal.MaterialFactoryNew())


def constant3(mat, colour, x: int, y: int):
    c = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, x, y)
    c.set_editor_property("constant", unreal.LinearColor(*colour, 1.0))
    return c


def constant(mat, value: float, x: int, y: int):
    c = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, x, y)
    c.set_editor_property("r", value)
    return c


def finish(mat: unreal.Material, name: str) -> None:
    errors = lib.recompile_material(mat)
    if errors:
        raise RuntimeError(f"{name} does not compile: {list(errors)}")
    eal.save_loaded_asset(mat)
    unreal.log(f"Kinetic: {name}")


def glint() -> None:
    mat = fresh("M_TagGlint")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    lib.connect_material_property(constant3(mat, (40.0, 40.0, 36.0), -300, 0), "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    finish(mat, "M_TagGlint")


def dust() -> None:
    mat = fresh("M_TagDust")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("two_sided", True)
    lib.connect_material_property(constant3(mat, (0.30, 0.26, 0.20), -300, -100), "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    # Thick through the middle of the sphere, thin where you look along its skin.
    edge = lib.create_material_expression(mat, unreal.MaterialExpressionFresnel, -600, 100)
    edge.set_editor_property("exponent", 1.5)
    inner = lib.create_material_expression(mat, unreal.MaterialExpressionOneMinus, -450, 100)
    lib.connect_material_expressions(edge, "", inner, "")
    thick = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    lib.connect_material_expressions(inner, "", thick, "A")
    thick.set_editor_property("const_b", 0.92)
    lib.connect_material_property(thick, "", unreal.MaterialProperty.MP_OPACITY)
    finish(mat, "M_TagDust")


def haze() -> None:
    mat = fresh("M_TagHaze")
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("two_sided", True)
    waves = unreal.load_asset(WAVES)
    lib.connect_material_property(constant3(mat, (0.0, 0.0, 0.0), -300, -200), "", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_property(constant(mat, 0.03, -300, -100), "", unreal.MaterialProperty.MP_OPACITY)
    lib.connect_material_property(constant(mat, 1.04, -300, 0), "", unreal.MaterialProperty.MP_REFRACTION)
    if waves:
        coord = lib.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1000, 200)
        coord.set_editor_property("u_tiling", 3.0)
        coord.set_editor_property("v_tiling", 3.0)
        pan = lib.create_material_expression(mat, unreal.MaterialExpressionPanner, -800, 200)
        pan.set_editor_property("speed_x", 0.0)
        pan.set_editor_property("speed_y", -0.35)  # The shimmer rises
        lib.connect_material_expressions(coord, "", pan, "Coordinate")
        n = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -550, 200)
        n.set_editor_property("texture", waves)
        n.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        lib.connect_material_expressions(pan, "", n, "UVs")
        lib.connect_material_property(n, "RGB", unreal.MaterialProperty.MP_NORMAL)
    else:
        unreal.log_warning("Kinetic: no port waves (make import-port): the haze is a flat tint")
    finish(mat, "M_TagHaze")


def main() -> None:
    glint()
    dust()
    haze()


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
