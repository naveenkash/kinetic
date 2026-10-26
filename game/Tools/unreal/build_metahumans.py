"""The five harness classes as MetaHumans (MetaHuman Creator, in the editor): faces and bodies
from Epic's MetaHuman, the harness on them code-built (art/characters). Two stages:

    make -C game metahumans            creates and shapes /Game/Characters/MetaHumans/MH_<Class>
    (in the editor, signed in to MetaHuman with an Epic account, in the Output Log's Cmd box:
     py "<repo>/game/Tools/unreal/metahumans_signed_in.py"; or KINETIC_MH_BUILD=1 headless)

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

# Each class's hair, from its she/her face doc (docs/characters/faces; the bodies are the female
# template): the nearest of MetaHuman's grooms, with eyebrows and eyelashes.
GROOMS = "/MetaHumanCharacter/Optional/Grooms/Bindings"
HAIR = {
    "Rigger": ("Hair_M_TwistedBraids", "Eyebrows_M_Thin"),  # Teodora Quispe: a long white braid
    "Breaker": ("Hair_S_Updo", "Eyebrows_M_Full"),  # Mei-Lin Haldane: a tight low black bun
    "Climber": ("Hair_S_UpdoBraids", "Eyebrows_M_Natural"),  # Paloma Reyes-Okafor: locs tied up and back
    "Doubler": ("Hair_S_LowPonytail", "Eyebrows_M_Shaded"),  # Odile Lacroix: dark brown greying, a low knot
    "Scrapper": ("Hair_M_SideSweptFringe", "Eyebrows_M_Unruly"),  # Dela Santos: half shaved, long on the right
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


def groom(cls: str, mh) -> None:
    """Puts the class's hair, eyebrows and eyelashes on the character (its wardrobe), selected."""
    collection = mh.get_editor_property("internal_collection")
    instance = collection.get_editor_property("default_instance")
    hair, brows = HAIR[cls]
    for slot, item in (("Hair", f"{GROOMS}/Hair/WI_{hair}"), ("Eyebrows", f"{GROOMS}/Eyebrows/WI_{brows}"),
                       ("Eyelashes", f"{GROOMS}/Eyelashes/WI_Eyelashes_M_Natural")):
        wardrobe = unreal.load_asset(item)
        if wardrobe is None:
            unreal.log_warning(f"Kinetic: {cls}: no {item} (MetaHuman Creator Core Data installed?)")
            continue
        keys = collection.get_item_keys_for_wardrobe_item(wardrobe)
        key = keys[0] if keys else collection.try_add_item_from_wardrobe_item(slot, wardrobe)
        if key is not None:
            instance.set_single_slot_selection(slot, key)


def build(cls: str, mh, sub) -> bool:
    if os.environ.get("KINETIC_MH_BUILD") == "assemble":
        return assemble(cls, mh, sub)  # Rig and textures already on the character
    rig = unreal.MetaHumanCharacterAutoRiggingRequestParams()
    rig.rig_type = unreal.MetaHumanRigType.JOINTS_AND_BLEND_SHAPES
    rig.blocking = True
    rig.report_progress = False
    sub.request_auto_rigging(mh, rig)
    textures = unreal.MetaHumanCharacterTextureRequestParams()
    textures.blocking = True
    textures.report_progress = False
    sub.request_texture_sources(mh, textures)
    return assemble(cls, mh, sub)


def assemble(cls: str, mh, sub) -> bool:
    # A build needs the rig and the skin textures (both Epic's services, fetched above).
    if not sub.can_build_meta_human(mh, True):
        unreal.log_error(f"Kinetic: {cls}: not buildable (no rig or textures yet: build signed in first)")
        return False
    params = unreal.MetaHumanCharacterEditorBuildParameters()
    params.pipeline_type = unreal.MetaHumanDefaultPipelineType.OPTIMIZED
    params.pipeline_quality = unreal.MetaHumanQualityLevel.MEDIUM
    params.absolute_build_path = f"{BUILD}/MH_{cls}"
    params.common_folder_path = f"{BUILD}/Common"
    params.enable_wardrobe_item_validation = False
    try:
        sub.build_meta_human(mh, params)
    except RuntimeError as e:  # The assembly's errors come back raised: report them, go on to the next class
        unreal.log_error(f"Kinetic: {cls}: build failed: {str(e).splitlines()[0]}")
        return False
    unreal.log(f"Kinetic: {cls}: built into {BUILD}/MH_{cls}")
    return True


def run_all(with_build: bool) -> int:
    sub = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)
    failed = 0
    only = [c for c in os.environ.get("KINETIC_MH_CLASSES", "").split(",") if c]
    for cls in BODIES:
        if only and cls not in only:
            continue
        mh = character(cls)
        # An asset open in MetaHuman Creator's tab cannot be edited from here: close it first.
        unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).close_all_editors_for_asset(mh)
        if not sub.try_add_object_to_edit(mh):
            unreal.log_error(f"Kinetic: {cls}: could not edit {mh.get_path_name()} (open in an editor tab?)")
            failed += 1
            continue
        try:
            shape(cls, mh, sub)
            groom(cls, mh)
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
        status = 1 if run_all(os.environ.get("KINETIC_MH_BUILD") in ("1", "assemble")) else 0
        if os.environ.get("KINETIC_MH_BUILD"):
            eal.save_directory(BUILD, only_if_is_dirty=True, recursive=True)
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
