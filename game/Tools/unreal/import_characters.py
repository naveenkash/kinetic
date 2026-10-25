"""Imports the generated harness characters (art/out/characters, art/characters/harness.py).

    make -C game import-characters

(a headless editor running this with -ExecutePythonScript). For each class:

- the skinned mesh goes to /Game/Characters/<Class>/SK_Harness_<Class>, all five on one skeleton
  (SK_Harness_Skeleton, made by the first import: the specs' one skeleton for every class);
- each take becomes an animation sequence, AS_<Class>_<Take> (Idle, Run, Climb, Fall, Brace,
  Stagger), which AKineticCharacter plays;
- the baked textures (BaseColor, ORM, Emissive) are imported beside the mesh, and the harness slot
  gets an instance of M_KineticProp (import_props.py's master, enabled for skinned meshes) reading
  them; the team-tint slot (the plate rim bands, the Spool's mode ring, the shoulder lamp) gets an
  instance of M_KineticHarness, coloured by its TeamTint parameter, which the game sets per crew.

It checks the import: every bone of the report on the skeleton, every take imported, and the
figure facing +X (its toes ahead of its heels) with its left side on -Y, as an Unreal character
faces. It writes game/Saved/Kinetic/character_report.json.
"""
from __future__ import annotations

import glob
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
SOURCE = os.path.join(REPO, "art", "out", "characters")
ROOT = "/Game/Characters"
MATERIALS = f"{ROOT}/Materials"
SKELETON = f"{ROOT}/SK_Harness_Skeleton"

assets = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def hex_linear(hex_color: str) -> unreal.LinearColor:
    h = hex_color.lstrip("#")
    srgb = [int(h[i:i + 2], 16) / 255.0 for i in (0, 2, 4)]
    lin = [c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4 for c in srgb]
    return unreal.LinearColor(lin[0], lin[1], lin[2], 1.0)


def master_material() -> unreal.Material:
    """BaseColor, Roughness and Metallic parameters; UseTeamTint (0 or 1) swaps the base colour for
    TeamTint and makes it glow at EmissiveStrength."""
    name = "M_KineticHarness"
    path = f"{MATERIALS}/{name}"
    if eal.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = assets.create_asset(name, MATERIALS, unreal.Material, unreal.MaterialFactoryNew())

    def vector(param: str, x: int, y: int, default):
        node = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, x, y)
        node.set_editor_property("parameter_name", param)
        node.set_editor_property("default_value", default)
        return node

    def scalar(param: str, x: int, y: int, default: float):
        node = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
        node.set_editor_property("parameter_name", param)
        node.set_editor_property("default_value", default)
        return node

    base = vector("BaseColor", -800, -300, unreal.LinearColor(0.5, 0.5, 0.5, 1.0))
    tint = vector("TeamTint", -800, -100, hex_linear("#E69F00"))
    use = scalar("UseTeamTint", -800, 100, 0.0)
    pick = lib.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -450, -200)
    lib.connect_material_expressions(base, "", pick, "A")
    lib.connect_material_expressions(tint, "", pick, "B")
    lib.connect_material_expressions(use, "", pick, "Alpha")
    lib.connect_material_property(pick, "", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_property(scalar("Roughness", -800, 300, 0.6), "", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.connect_material_property(scalar("Metallic", -800, 450, 0.0), "", unreal.MaterialProperty.MP_METALLIC)
    strength = scalar("EmissiveStrength", -800, 600, 6.0)
    glow = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -450, 400)
    lib.connect_material_expressions(tint, "", glow, "A")
    lib.connect_material_expressions(use, "", glow, "B")
    emit = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 500)
    lib.connect_material_expressions(glow, "", emit, "A")
    lib.connect_material_expressions(strength, "", emit, "B")
    lib.connect_material_property(emit, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mat.set_editor_property("used_with_skeletal_mesh", True)
    errors = lib.recompile_material(mat)
    if errors:
        raise RuntimeError(f"{name} does not compile: {list(errors)}")
    eal.save_loaded_asset(mat)
    return mat


def instance(master, name: str, folder: str, spec: dict) -> unreal.MaterialInstanceConstant:
    path = f"{folder}/{name}"
    if eal.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = assets.create_asset(name, folder, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, master)
    lib.set_material_instance_vector_parameter_value(mi, "BaseColor", hex_linear(spec["color"]))
    lib.set_material_instance_scalar_parameter_value(mi, "Roughness", spec["roughness"])
    lib.set_material_instance_scalar_parameter_value(mi, "Metallic", spec["metallic"])
    lib.set_material_instance_scalar_parameter_value(mi, "UseTeamTint", 1.0 if spec["team_tint"] else 0.0)
    eal.save_loaded_asset(mi)
    return mi


PROP_MASTER = "/Game/Core/Materials/M_KineticProp"


def import_texture(png: str, folder: str, kind: str) -> unreal.Texture2D:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", png)
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    assets.import_asset_tasks([task])
    texture = unreal.load_asset(list(task.get_editor_property("imported_object_paths"))[0])
    if kind == "ORM":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property("srgb", False)
    eal.save_loaded_asset(texture)
    return texture


def textured_instance(name: str, folder: str, textures: dict) -> unreal.MaterialInstanceConstant:
    master = unreal.load_asset(PROP_MASTER)
    if master is None:
        raise RuntimeError(f"{PROP_MASTER} is missing (make import-props)")
    if not master.get_editor_property("used_with_skeletal_mesh"):
        master.set_editor_property("used_with_skeletal_mesh", True)
        lib.recompile_material(master)
        eal.save_loaded_asset(master)
    path = f"{folder}/{name}"
    mi = unreal.load_asset(path) if eal.does_asset_exist(path) else assets.create_asset(
        name, folder, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, master)
    for kind, texture in textures.items():
        lib.set_material_instance_texture_parameter_value(mi, kind, texture)
    if "Emissive" in textures:
        lib.set_material_instance_scalar_parameter_value(mi, "EmissiveStrength", 4.0)
    eal.save_loaded_asset(mi)
    return mi


def import_fbx(fbx: str, folder: str, skeleton) -> list[str]:
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    ui.set_editor_property("import_animations", True)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("create_physics_asset", False)
    ui.set_editor_property("automated_import_should_detect_type", False)
    if skeleton is not None:
        ui.set_editor_property("skeleton", skeleton)
    mesh_data = ui.get_editor_property("skeletal_mesh_import_data")
    mesh_data.set_editor_property("import_morph_targets", False)
    mesh_data.set_editor_property("convert_scene", True)
    mesh_data.set_editor_property("force_front_x_axis", False)
    anim_data = ui.get_editor_property("anim_sequence_import_data")
    anim_data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    anim_data.set_editor_property("import_bone_tracks", True)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fbx)
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", ui)
    assets.import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


def bone_position(skeleton, bone: str) -> unreal.Vector | None:
    """The bone's reference-pose position in the component frame, if the editor can tell us."""
    try:
        pose = unreal.AnimPoseExtensions.get_reference_pose(skeleton)
        return unreal.AnimPoseExtensions.get_bone_pose(pose, bone, unreal.AnimPoseSpaces.WORLD).translation
    except Exception:  # Not every build exposes the pose library to Python
        return None


def main() -> None:
    master = master_material()
    reports = sorted(glob.glob(os.path.join(SOURCE, "*", "report.json")))
    if not reports:
        raise RuntimeError(f"no characters in {SOURCE} (blender -b -P art/characters/harness.py)")
    problems: list[str] = []
    out: dict = {}
    # The Rigger first: its import makes the skeleton the rest share.
    reports.sort(key=lambda p: 0 if os.sep + "Rigger" + os.sep in p else 1)
    skeleton = unreal.load_asset(SKELETON) if eal.does_asset_exist(SKELETON) else None
    for report_path in reports:
        report = json.load(open(report_path))
        cls, name = report["class"], report["name"]
        folder = f"{ROOT}/{cls}"
        fbx = os.path.join(os.path.dirname(report_path), f"{name}.fbx")
        imported = import_fbx(fbx, folder, skeleton)
        mesh_path = next((p for p in imported if p.split(".")[0].endswith(name)), None)
        mesh = unreal.load_asset(mesh_path) if mesh_path else None
        if not isinstance(mesh, unreal.SkeletalMesh):
            problems.append(f"{cls}: no skeletal mesh imported ({imported})")
            continue
        if skeleton is None:
            # The first import's skeleton, under the shared name.
            skeleton = mesh.get_editor_property("skeleton")
            if eal.rename_asset(skeleton.get_path_name().split(".")[0], SKELETON):
                skeleton = unreal.load_asset(SKELETON)
        # The takes, by name.
        takes = {}
        for path in imported:
            asset = unreal.load_asset(path)
            if isinstance(asset, unreal.AnimSequence):
                for t in report["takes"]:
                    if asset.get_name().endswith(f"_{t['name']}"):  # "Climb" is in "Climber": the suffix only
                        target = f"{folder}/AS_{cls}_{t['name']}"
                        if asset.get_path_name().split(".")[0] != target:
                            if eal.does_asset_exist(target):
                                eal.delete_asset(target)
                            eal.rename_asset(asset.get_path_name().split(".")[0], target)
                        takes[t["name"]] = target
        for t in report["takes"]:
            if t["name"] not in takes:
                problems.append(f"{cls}: take {t['name']} not imported")
        # The baked harness slot and the team-tint slot.
        textures = {kind: import_texture(os.path.join(os.path.dirname(report_path), png), folder, kind)
                    for kind, png in report["textures"].items()}
        harness = textured_instance(f"MI_{cls}_Harness", folder, textures)
        team = instance(master, f"MI_{cls}_TeamTint", folder, {"color": "#E69F00", "roughness": 0.3, "metallic": 0.0,
                                                              "team_tint": True})
        materials = mesh.get_editor_property("materials")
        slots = []
        for m in materials:
            slot = str(m.get_editor_property("material_slot_name"))
            m.set_editor_property("material_interface", team if slot.endswith("TeamTint") else harness)
            slots.append(slot)
        mesh.set_editor_property("materials", materials)
        eal.save_loaded_asset(mesh)
        # Bones, and which way it faces.
        bones = [str(n) for n in unreal.SkeletalMeshEditorSubsystem.get_bone_names(mesh)] if hasattr(
            unreal.SkeletalMeshEditorSubsystem, "get_bone_names") else []
        missing = [b for b in report["bones"] if bones and b not in bones]
        if missing:
            problems.append(f"{cls}: bones missing {missing}")
        toe, heel, left = bone_position(skeleton, "ball_l"), bone_position(skeleton, "foot_l"), bone_position(skeleton, "thigh_l")
        facing = None
        if toe is not None and heel is not None and left is not None:
            facing = {"forward": toe.x > heel.x, "left_on_minus_y": left.y < 0.0}
            if not all(facing.values()):
                problems.append(f"{cls}: faces wrong {facing}")
        out[cls] = {"mesh": mesh.get_path_name(), "takes": takes, "slots": slots, "facing": facing,
                    "triangles": report["triangles"], "shoulder_width_m": report["shoulder_width_m"]}
        unreal.log(f"Kinetic: {cls}: {name}, {len(takes)} takes, {len(slots)} material slots, facing {facing}")
    saved = os.path.join(REPO, "game", "Saved", "Kinetic")
    os.makedirs(saved, exist_ok=True)
    with open(os.path.join(saved, "character_report.json"), "w") as f:
        json.dump(out, f, indent=2)
    for p in problems:
        unreal.log_error(p)
    if problems:
        raise RuntimeError(f"{len(problems)} character import problems")


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
