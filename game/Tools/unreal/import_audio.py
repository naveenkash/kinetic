"""Imports the generated sound cues (art/out/audio, art/audio/build_audio.py) as /Game/Audio/SW_<Name>.

    make -C game import-audio

(a headless editor running this with -ExecutePythonScript, after the cues are generated). The
hazards and the Yard director play them by name (UKineticHazardWorld::PlayCue); a missing cue is
silent, so a level plays without them.
"""
from __future__ import annotations

import glob
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.normpath(os.path.join(HERE, "..", "..", "..", "art", "out", "audio"))
DESTINATION = "/Game/Audio"

assets = unreal.AssetToolsHelpers.get_asset_tools()


def main() -> None:
    files = sorted(glob.glob(os.path.join(SOURCE, "SW_*.wav")))
    if not files:
        raise RuntimeError(f"no cues in {SOURCE} (python3 art/audio/build_audio.py)")
    tasks = []
    for wav in files:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", wav)
        task.set_editor_property("destination_path", DESTINATION)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
    assets.import_asset_tasks(tasks)
    imported = [p for t in tasks for p in t.get_editor_property("imported_object_paths")]
    if len(imported) != len(files):
        raise RuntimeError(f"imported {len(imported)} of {len(files)} cues")
    unreal.log(f"Kinetic: imported {len(imported)} sound cues into {DESTINATION}")


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
