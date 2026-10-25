"""Imports each class's gear fitted to its MetaHuman (art/characters, harness.py --metahuman):

    make -C game import-metahuman-gear

For each class, art/out/metahumans/<Class>/SK_MHGear_<Class>.fbx becomes
/Game/Characters/MetaHumans/<Class>/SK_MHGear_<Class>, on the MetaHuman body's own skeleton
(metahuman_base_skel), with its baked BaseColor, ORM and Normal on the prop master and a team-tint
instance for its TeamTint slot. AKineticCharacter then plays the class as its MetaHuman, the gear
following the body by bone name.
"""
from __future__ import annotations

import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import unreal  # noqa: E402

import import_characters as chars  # noqa: E402

OUT = os.path.join(chars.REPO, "art", "out", "metahumans")
CLASSES = ["Rigger", "Breaker", "Climber", "Doubler", "Scrapper"]
eal = unreal.EditorAssetLibrary


def import_gear(fbx: str, folder: str, skeleton) -> unreal.SkeletalMesh | None:
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    ui.set_editor_property("import_animations", False)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("create_physics_asset", False)
    ui.set_editor_property("automated_import_should_detect_type", False)
    ui.set_editor_property("skeleton", skeleton)
    data = ui.get_editor_property("skeletal_mesh_import_data")
    data.set_editor_property("import_morph_targets", False)
    data.set_editor_property("convert_scene", True)
    data.set_editor_property("force_front_x_axis", False)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fbx)
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", ui)
    chars.assets.import_asset_tasks([task])
    name = os.path.splitext(os.path.basename(fbx))[0]
    for path in task.get_editor_property("imported_object_paths"):
        asset = unreal.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh) and asset.get_name() == name:
            return asset
    return None


def main() -> int:
    master = chars.master_material()
    failed = 0
    for cls in CLASSES:
        report_path = os.path.join(OUT, cls, "report.json")
        if not os.path.exists(report_path):
            unreal.log_error(f"Kinetic: {cls}: no gear (blender -b -P art/characters/harness.py -- --metahuman)")
            failed += 1
            continue
        report = json.load(open(report_path))
        skeleton = unreal.load_asset(report["skeleton"].split(".")[0])
        folder = f"/Game/Characters/MetaHumans/{cls}"
        mesh = import_gear(os.path.join(OUT, cls, f"{report['name']}.fbx"), folder, skeleton)
        if mesh is None:
            unreal.log_error(f"Kinetic: {cls}: {report['name']} not imported")
            failed += 1
            continue
        on = mesh.get_editor_property("skeleton")
        if on != skeleton:
            unreal.log_error(f"Kinetic: {cls}: the gear is on {on.get_path_name() if on else None}, not the MetaHuman's skeleton")
            failed += 1
        textures = {kind: chars.import_texture(os.path.join(OUT, cls, png), folder, kind) for kind, png in report["textures"].items()}
        harness = chars.textured_instance(f"MI_{cls}_MHGear", folder, textures)
        team = chars.instance(master, f"MI_{cls}_MHGearTeamTint", folder,
                              {"color": "#E69F00", "roughness": 0.3, "metallic": 0.0, "team_tint": True})
        materials = mesh.get_editor_property("materials")
        for i in range(len(materials)):
            m = materials[i]  # A copy: set back into the array below
            m.set_editor_property("material_interface", team if str(m.get_editor_property("material_slot_name")).endswith("TeamTint")
                                  else harness)
            materials[i] = m
        mesh.set_editor_property("materials", materials)
        eal.save_loaded_asset(mesh, only_if_is_dirty=False)
        unreal.log(f"Kinetic: {cls}: {mesh.get_name()} on {skeleton.get_name()}, {len(materials)} slots, textures {sorted(textures)}")
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
