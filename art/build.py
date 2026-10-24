"""Builds one generated asset (art/README.md).

    blender -b --factory-startup -P art/build.py -- <asset> [--variant v] [--out dir]
        [--preview-only] [--res 2048]

Writes to art/out/<asset>/: the preview renders (procedural and baked), the baked textures, an FBX
for Unreal and a GLB for the browser client, and report.json (triangle counts, sizes, sockets).
"""
from __future__ import annotations

import argparse
import importlib
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "lib"), os.path.join(HERE, "props")]

import bpy  # noqa: E402
from mathutils import Vector  # noqa: E402

import kinetic_art as ka  # noqa: E402


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="art/build.py")
    parser.add_argument("asset")
    parser.add_argument("--variant", default=None)
    parser.add_argument("--out", default=os.path.join(HERE, "out"))
    parser.add_argument("--preview-only", action="store_true")
    parser.add_argument("--res", type=int, default=2048)
    args = parser.parse_args(argv)

    module = importlib.import_module(args.asset)
    ka.reset_scene()
    built = module.build(args.variant) if args.variant else module.build()
    name = built["name"]
    out = os.path.join(args.out, args.asset, name)
    os.makedirs(out, exist_ok=True)
    meshes = [built["body"], *built.get("doors", [])]
    size = built["size"]
    target = Vector((0, 0, size[2] * 0.45))
    distance = max(size) * 1.25

    report = {
        "name": name,
        "triangles": {m.name: ka.triangles(m) for m in meshes},
        "size_m": size,
        "sockets": sorted(c.name.removeprefix("SOCKET_") for c in built["body"].children),
    }
    ka.preview(os.path.join(out, "preview.png"), target, distance)
    ka.clear_preview()
    if not args.preview_only:
        for m in meshes:
            ka.smart_uv(m)
            maps = ka.bake(m, out, m.name, args.res if m is built["body"] else args.res // 2)
            ka.assign_only(m, ka.baked_material(f"M_{m.name}", maps))
        ka.preview(os.path.join(out, "preview_baked.png"), target, distance)
        ka.clear_preview()
        collision = [o for o in bpy.data.objects if o.name.startswith("UCX_")]
        report["exports"] = ka.export(meshes + collision, out, name)
    with open(os.path.join(out, "report.json"), "w") as f:
        json.dump(report, f, indent=2)
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    sys.exit(main(argv))
