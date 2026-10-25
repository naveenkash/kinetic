"""The five harness classes as MetaHumans (MetaHuman Creator, in the editor): faces and bodies
from Epic's MetaHuman, the harness on them code-built (art/characters). Two stages:

    make -C game metahumans            creates and shapes /Game/Characters/MetaHumans/MH_<Class>
    (in the editor, signed in to MetaHuman with an Epic account:
     Tools > Execute Python Script > game/Tools/unreal/build_metahumans.py, with KINETIC_MH_BUILD=1
     in the environment, or run build_all() from the Python console)

Shaping runs anywhere. Rigging and the high-resolution skin textures are Epic's cloud services and
need the editor signed in to MetaHuman; the build needs the rig. The body targets come from the
class silhouettes (docs/characters/README.md; PROPOSED as measurements): every class inside the
1.9 m capsule, the Breaker broadest, the Climber longest and leanest.
"""
from __future__ import annotations

import os

import unreal

FOLDER = "/Game/Characters/MetaHumans"
BUILD = "/Game/MetaHumans"

# Height, across the shoulders, chest and waist (cm), muscularity and fat on the editor's 0 to 1.
BODIES = {
    "Climber": dict(height=184.0, across_shoulder=40.0, chest=88.0, waist=70.0, muscularity=0.55, fat=0.1),
    "Scrapper": dict(height=172.0, across_shoulder=43.0, chest=96.0, waist=80.0, muscularity=0.5, fat=0.3),
    "Rigger": dict(height=180.0, across_shoulder=46.0, chest=104.0, waist=86.0, muscularity=0.6, fat=0.35),
    "Doubler": dict(height=178.0, across_shoulder=45.0, chest=100.0, waist=82.0, muscularity=0.55, fat=0.25),
    "Breaker": dict(height=186.0, across_shoulder=54.0, chest=124.0, waist=104.0, muscularity=0.85, fat=0.45),
}

tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def character(cls: str):
    path = f"{FOLDER}/MH_{cls}"
    if eal.does_asset_exist(path):
        return unreal.load_asset(path)
    return tools.create_asset(f"MH_{cls}", FOLDER, unreal.MetaHumanCharacter,
                              unreal.new_object(type=unreal.MetaHumanCharacterFactoryNew))


def shape(cls: str, mh, sub) -> None:
    targets = BODIES[cls]
    constraints = {str(c.name).lower().replace(" ", "_"): c for c in sub.get_body_constraints(mh)}
    for key, value in targets.items():
        c = constraints.get(key)
        if c is None:
            unreal.log_warning(f"Kinetic: {cls}: no body constraint {key}")
            continue
        c.is_active = True
        c.target_measurement = value
    sub.set_body_constraints(mh, list(constraints.values()))
    sub.commit_body_state(mh)


def build(cls: str, mh, sub) -> bool:
    rig = unreal.MetaHumanCharacterAutoRiggingRequestParams()
    rig.rig_type = unreal.MetaHumanRigType.JOINTS_AND_BLEND_SHAPES
    rig.blocking = True
    rig.report_progress = False
    sub.request_auto_rigging(mh, rig)
    if not sub.can_build_meta_human(mh, True):
        unreal.log_error(f"Kinetic: {cls}: not rigged (sign in to MetaHuman in the editor first)")
        return False
    textures = unreal.MetaHumanCharacterTextureRequestParams()
    textures.blocking = True
    textures.report_progress = False
    sub.request_texture_sources(mh, textures)
    params = unreal.MetaHumanCharacterEditorBuildParameters()
    params.pipeline_type = unreal.MetaHumanDefaultPipelineType.OPTIMIZED
    params.pipeline_quality = unreal.MetaHumanQualityLevel.MEDIUM
    params.absolute_build_path = f"{BUILD}/MH_{cls}"
    params.common_folder_path = f"{BUILD}/Common"
    params.enable_wardrobe_item_validation = False
    sub.build_meta_human(mh, params)
    unreal.log(f"Kinetic: {cls}: built into {BUILD}/MH_{cls}")
    return True


def run_all(with_build: bool) -> int:
    sub = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)
    failed = 0
    for cls in BODIES:
        mh = character(cls)
        if not sub.try_add_object_to_edit(mh):
            unreal.log_error(f"Kinetic: {cls}: could not edit {mh.get_path_name()} (open in an editor tab?)")
            failed += 1
            continue
        try:
            shape(cls, mh, sub)
            if with_build and not build(cls, mh, sub):
                failed += 1
        finally:
            sub.remove_object_to_edit(mh)
        eal.save_loaded_asset(mh)
        unreal.log(f"Kinetic: {cls}: {mh.get_path_name()} shaped")
    return failed


def build_all() -> int:
    """From the editor's Python console, signed in: rig, texture and build all five."""
    return run_all(True)


def run() -> None:
    unattended = "-unattended" in unreal.SystemLibrary.get_command_line().lower()
    status = 0
    try:
        status = 1 if run_all(os.environ.get("KINETIC_MH_BUILD") == "1") else 0
    except Exception:
        import traceback
        for line in traceback.format_exc().splitlines():
            unreal.log_error(line)
        status = 1
    if unattended:
        unreal.SystemLibrary.execute_console_command(None, "FLUSHLOG")
        os._exit(status)


if __name__ == "__main__":
    run()
