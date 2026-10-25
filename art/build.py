"""Builds one generated asset (art/README.md).

    blender -b --factory-startup -P art/build.py -- <asset> [--variant v] [--out dir]
        [--preview-only] [--res N]

Writes to art/out/<asset>/<mesh>/: the preview renders (procedural and baked), the baked textures, an FBX
for Unreal and a GLB preview, and report.json (triangle counts, sizes, sockets).
"""
from __future__ import annotations

import argparse
import importlib
import json
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "lib"), os.path.join(HERE, "props")]

import bpy  # noqa: E402

import kinetic_art as ka  # noqa: E402


def bounds(mesh) -> list[float]:
    """The mesh's extent along each axis in its own space, metres, from its vertices (an
    object's cached bounding box can be stale after a script transforms its data)."""
    co = [v.co for v in mesh.data.vertices]
    return [round(max(c[i] for c in co) - min(c[i] for c in co), 4) for i in range(3)]


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="art/build.py")
    parser.add_argument("asset")
    parser.add_argument("--variant", default=None)
    parser.add_argument("--out", default=os.path.join(HERE, "out"))
    parser.add_argument("--preview-only", action="store_true")
    parser.add_argument("--res", type=int, default=None, help="body texture size; overrides the asset's")
    args = parser.parse_args(argv)
    started = time.time()

    module = importlib.import_module(args.asset)
    ka.reset_scene()
    built = module.build(args.variant) if args.variant else module.build()
    name = built["name"]
    out = os.path.join(args.out, args.asset, name)
    os.makedirs(out, exist_ok=True)
    # An asset returns its meshes (the first carries the sockets), its size, its LOD0 triangle
    # budget from the spec, and optionally: a texture size; objects shown in the previews only
    # (a rack's cylinders), kept out of the bake and the export; a lift for the previews (a reel
    # whose pivot is its axle); and for a mesh baked from detail (ka.high_low) its high mesh,
    # the cage distance and whether it is a masked card.
    meshes = built["meshes"]
    size = built["size"]
    res = args.res or built.get("res", 2048)
    preview_only = built.get("preview_only", [])
    highs = built.get("high", {})
    masked = set(built.get("masked", ()))
    two_sided = set(built.get("double_sided", ()))
    # A part with its own pivot is exported on its own (ka.export); the body says where it goes.
    for m in meshes[1:]:
        if m.location.length > 1e-6:
            ka.socket(f"ATTACH_{m.name}", tuple(m.location), meshes[0], normal=(1.0, 0.0, 0.0))

    report = {
        "name": name,
        "triangles": {m.name: ka.triangles(m) for m in meshes},
        "budget": built.get("budget"),
        "masked": sorted(masked),  # Masked cards: the Unreal import gives them the masked material
        "double_sided": sorted(two_sided),
        # Each mesh's bounding box in its own space, metres, for checking the import's scale.
        "bounds_m": {m.name: bounds(m) for m in meshes},
        "size_m": size,
        "sockets": sorted(c.name.removeprefix("SOCKET_") for c in meshes[0].children),
        # Mesh-local, metres, Blender axes (Unreal's import flips Y).
        "socket_locations_m": {c.name.removeprefix("SOCKET_"): [round(v, 4) for v in c.matrix_local.translation]
                               for c in meshes[0].children},
        # Each socket's X axis, the outward normal (Blender axes).
        "socket_normals": {c.name.removeprefix("SOCKET_"): [round(v, 4) for v in c.matrix_local.col[0][:3]]
                           for c in meshes[0].children},
    }
    if highs:
        report["high_triangles"] = {k: ka.triangles(h) for k, h in highs.items()}
    over = report["budget"] is not None and sum(report["triangles"].values()) > report["budget"]
    if over:
        print(f"WARNING: {name} is {sum(report['triangles'].values())} tris, over its {report['budget']} budget")
    lift = built.get("preview_lift", 0.0)

    def show(baked: bool) -> None:
        # The procedural preview shows the detail; the baked one shows the game mesh.
        for m in meshes:
            if m.name in highs:
                m.hide_render = not baked
                highs[m.name].hide_render = baked

    def render(path: str) -> None:
        # A pivot off the floor (an axle) is lifted for the render only, then put back.
        for o in bpy.data.objects:
            if o.parent is None:
                o.location.z += lift
        ka.preview(path, size)
        ka.clear_preview()
        for o in bpy.data.objects:
            if o.parent is None:
                o.location.z -= lift

    show(baked=False)
    render(os.path.join(out, "preview.png"))
    if not args.preview_only:
        for m in meshes:
            ka.triangulate(m)
            ka.smart_uv(m)
            # Preview-only objects would shade the bake (AO, the edge-wear nodes).
            for o in preview_only:
                o.hide_render = True
            sources = [highs[m.name]] if m.name in highs else [m]
            glow = ka.emission_strength(sources)
            # Every mesh gets a Normal map: its high mesh's detail, or its materials' own dents,
            # scratches and grain.
            maps = ka.bake(m, out, m.name, res if m is meshes[0] else max(512, res // 2), high=highs.get(m.name),
                           cage=built.get("cage", 0.02), mask=m.name in masked, normal=True)
            for o in preview_only:
                o.hide_render = False
            ka.assign_only(m, ka.baked_material(f"M_{m.name}", maps, masked=m.name in masked, emission=glow,
                                                two_sided=m.name in two_sided))
            if glow > 0:
                report.setdefault("emission_strength", {})[m.name] = glow
        for h in highs.values():
            bpy.data.objects.remove(h)
        render(os.path.join(out, "preview_baked.png"))
        collision = [o for o in bpy.data.objects if o.name.startswith("UCX_")]
        # The stencils' overspray attribute is for the bake only; the game meshes do not carry it.
        for m in meshes:
            if "spray" in m.data.attributes:
                m.data.attributes.remove(m.data.attributes["spray"])
        report["exports"] = ka.export(meshes + collision, out, name)
    report["build_seconds"] = round(time.time() - started, 1)
    with open(os.path.join(out, "report.json"), "w") as f:
        json.dump(report, f, indent=2)
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    sys.exit(main(argv))
