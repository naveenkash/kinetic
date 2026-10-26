"""Retargets the classes' six takes (art/characters, make import-characters) from the harness
skeleton onto each class MetaHuman (build_metahumans.py):

    make -C game retarget-metahumans

For each class it makes, under /Game/Characters/MetaHumans/<Class>:
- IKR_MH_<Class>, an IK Rig on the MetaHuman body, its retarget chains and full-body IK
  auto-generated (the harness skeleton's IKR_Harness is made once, the same way);
- RTG_Harness_To_<Class>, a retargeter between the two, its chains auto-mapped and the
  MetaHuman's retarget pose auto-aligned to the harness skeleton's (our rest pose has the arms
  down, the MetaHuman's is an A-pose);
- AS_<Class>_<Take>_MH, the six takes retargeted onto metahuman_base_skel.
"""
from __future__ import annotations

import os

import unreal

CLASSES = ["Rigger", "Breaker", "Climber", "Doubler", "Scrapper"]
TAKES = ["Idle", "Run", "Climb", "Fall", "Brace", "Stagger"]
ROOT = "/Game/Characters/MetaHumans"

assets = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def fresh(name: str, folder: str, cls, factory):
    """The asset, made if it is not there yet (its setup below is regenerated either way)."""
    path = f"{folder}/{name}"
    return unreal.load_asset(path) if eal.does_asset_exist(path) else assets.create_asset(name, folder, cls, factory)


FINGER_CURL = (30.0, 45.0, 28.0)  # Degrees at the knuckle, middle and tip joints: a relaxed hand


def curl(degrees: float) -> unreal.Quat:
    """A finger joint's bend in its bone's local frame (the MetaHuman's fingers bend about Z)."""
    return unreal.Rotator(roll=0.0, pitch=0.0, yaw=FINGER_YAW_SIGN * degrees).quaternion()


FINGER_YAW_SIGN = 1.0


def ik_rig(name: str, folder: str, mesh) -> unreal.IKRigDefinition:
    rig = fresh(name, folder, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    c = unreal.IKRigController.get_controller(rig)
    c.set_skeletal_mesh(mesh)
    c.apply_auto_generated_retarget_definition()
    c.apply_auto_fbik()
    unreal.log(f"Kinetic: {name}: chains {[str(ch.get_editor_property('chain_name')) for ch in c.get_retarget_chains()]}")
    eal.save_loaded_asset(rig)
    return rig


def retargeter(name: str, folder: str, source_rig, target_rig, source_mesh, target_mesh) -> unreal.IKRetargeter:
    rtg = fresh(name, folder, unreal.IKRetargeter, unreal.IKRetargetFactory())
    c = unreal.IKRetargeterController.get_controller(rtg)
    c.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
    c.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_rig)
    c.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, source_mesh)
    c.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, target_mesh)
    c.remove_all_ops()
    c.add_default_ops()
    c.assign_ik_rig_to_all_ops(unreal.RetargetSourceOrTarget.TARGET, target_rig)
    c.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
    # The MetaHuman stands in an A-pose; bring its retarget pose to ours, chain by chain.
    c.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET, unreal.RetargetAutoAlignMethod.CHAIN_TO_CHAIN)
    # The takes have no fingers: the MetaHuman's stay in its retarget pose, so curl them there,
    # relaxed (knuckle, middle and tip joints), the thumb a little across the palm.
    for side in ("l", "r"):
        for finger in ("index", "middle", "ring", "pinky"):
            for joint, degrees in (("01", FINGER_CURL[0]), ("02", FINGER_CURL[1]), ("03", FINGER_CURL[2])):
                c.set_rotation_offset_for_retarget_pose_bone(f"{finger}_{joint}_{side}", curl(degrees), unreal.RetargetSourceOrTarget.TARGET)
        for joint, degrees in (("01", 8.0), ("02", 12.0), ("03", 10.0)):
            c.set_rotation_offset_for_retarget_pose_bone(f"thumb_{joint}_{side}", curl(degrees), unreal.RetargetSourceOrTarget.TARGET)
    eal.save_loaded_asset(rtg)
    return rtg


def main() -> int:
    failed = 0
    harness = unreal.load_asset("/Game/Characters/Rigger/SK_Harness_Rigger")
    if harness is None:
        raise RuntimeError("no harness characters (make import-characters)")
    source_rig = ik_rig("IKR_Harness", ROOT, harness)
    for cls in CLASSES:
        folder = f"{ROOT}/{cls}"
        body = unreal.load_asset(f"/Game/MetaHumans/MH_{cls}/MH_{cls}/Body/SKM_MH_{cls}_BodyMesh")
        source = unreal.load_asset(f"/Game/Characters/{cls}/SK_Harness_{cls}")
        if body is None or source is None:
            unreal.log_error(f"Kinetic: {cls}: no MetaHuman body or no harness mesh")
            failed += 1
            continue
        target_rig = ik_rig(f"IKR_MH_{cls}", folder, body)
        rtg = retargeter(f"RTG_Harness_To_{cls}", folder, source_rig, target_rig, source, body)
        takes = [eal.find_asset_data(f"/Game/Characters/{cls}/AS_{cls}_{t}") for t in TAKES]
        made = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
            takes, source, body, rtg, suffix="_MH", target_path=folder, include_referenced_assets=False,
            overwrite_existing_files=True)
        names = []
        for data in made:
            path = str(data.get_editor_property("package_name"))
            name = path.rsplit("/", 1)[-1]
            if not name.startswith("AS_"):
                continue
            target = f"{folder}/{name}"
            if path != target:
                if eal.does_asset_exist(target):
                    eal.delete_asset(target)
                eal.rename_asset(path, target)
            eal.save_asset(target, only_if_is_dirty=False)
            names.append(name)
        missing = [t for t in TAKES if f"AS_{cls}_{t}_MH" not in names]
        if missing:
            unreal.log_error(f"Kinetic: {cls}: takes not retargeted: {missing} (made {names})")
            failed += 1
        unreal.log(f"Kinetic: {cls}: {len(names)} takes retargeted onto {body.get_name()}")
    return failed


def run() -> None:
    status = 1
    try:
        status = 1 if main() else 0
    except Exception:
        import traceback
        for line in traceback.format_exc().splitlines():
            unreal.log_error(line)
    if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.execute_console_command(None, "FLUSHLOG")
        os._exit(status)


if __name__ == "__main__":
    run()
