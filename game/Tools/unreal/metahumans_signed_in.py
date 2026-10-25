"""Rigs, textures and builds the five class MetaHumans in the editor you are signed in to
(build_metahumans.py's second stage). In the Output Log's Cmd box:

    py "/Users/naveenkashyap/Documents/Projects/Kinetic/game/Tools/unreal/metahumans_signed_in.py"

The editor is busy while Epic's services rig each one (minutes per class). Progress and the
result go to the Output Log as "Kinetic:" lines.
"""
import importlib
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import unreal  # noqa: E402

import build_metahumans  # noqa: E402

importlib.reload(build_metahumans)
failed = build_metahumans.build_all()
unreal.log(f"Kinetic: MetaHumans done, {5 - failed} of 5 built" if not failed else f"Kinetic: MetaHumans: {failed} of 5 failed")
