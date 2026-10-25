"""Exports the five built class MetaHumans' body and face meshes, with the MetaHuman skeleton,
to art/out/metahumans/<Class>/ as FBX, so art/characters can fit the coverall and the harness to
each body and skin them to its skeleton:

    make -C game export-metahumans

(a headless editor, after build_metahumans.py has built them into /Game/MetaHumans/MH_<Class>).
"""
from __future__ import annotations

import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.normpath(os.path.join(HERE, "..", "..", "..", "art", "out", "metahumans"))
CLASSES = ["Rigger", "Breaker", "Climber", "Doubler", "Scrapper"]


def export(mesh, path: str) -> bool:
    task = unreal.AssetExportTask()
    task.set_editor_property("object", mesh)
    task.set_editor_property("filename", path)
    task.set_editor_property("automated", True)
    task.set_editor_property("prompt", False)
    task.set_editor_property("replace_identical", True)
    options = unreal.FbxExportOption()
    options.set_editor_property("collision", False)
    options.set_editor_property("level_of_detail", False)
    options.set_editor_property("export_morph_targets", False)
    task.set_editor_property("options", options)
    return unreal.Exporter.run_asset_export_task(task)


def main() -> int:
    failed = 0
    for cls in CLASSES:
        base = f"/Game/MetaHumans/MH_{cls}/MH_{cls}"
        folder = os.path.join(OUT, cls)
        os.makedirs(folder, exist_ok=True)
        facts = {"class": cls}
        for part in ("Body", "Face"):
            mesh = unreal.load_asset(f"{base}/{part}/SKM_MH_{cls}_{part}Mesh")
            if mesh is None:
                unreal.log_error(f"Kinetic: {cls}: no {part} mesh (build the MetaHumans first)")
                failed += 1
                continue
            path = os.path.join(folder, f"SKM_MH_{cls}_{part}Mesh.fbx")
            if not export(mesh, path):
                unreal.log_error(f"Kinetic: {cls}: {part} export failed")
                failed += 1
                continue
            b = mesh.get_bounds()
            facts[part.lower()] = {"fbx": os.path.basename(path), "skeleton": mesh.get_editor_property("skeleton").get_path_name(),
                                   "bounds_cm": [b.origin.x, b.origin.y, b.origin.z, b.box_extent.x, b.box_extent.y, b.box_extent.z]}
            unreal.log(f"Kinetic: {cls}: {part} to {path}")
        with open(os.path.join(folder, "metahuman.json"), "w") as f:
            json.dump(facts, f, indent=2)
    return failed


if __name__ == "__main__":
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
