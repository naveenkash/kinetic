# KINETIC generated assets

Every asset in KINETIC is generated from code, from its spec in [docs/objects](../docs/objects) (and later docs/characters and docs/arenas). Nothing is hand-modelled, bought or outsourced. Each asset is a Python script; running it rebuilds the asset exactly, so when a spec changes the script changes and the asset is rebuilt.

## How an asset is made

1. **Geometry.** Blender, scripted. The script builds the meshes from the spec's dimensions: metres, Z up, X+ toward the object's front, pivot at the bottom centre of the footprint (objects/README-props.md).
2. **Gameplay data.**
   - Dog sockets are empties named `SOCKET_<name>`, which Unreal imports as static-mesh sockets.
   - Collision is `UCX_<mesh>_<nn>` boxes.
   - Moving parts, such as a container's doors, are separate meshes with their pivot on the hinge.
3. **Materials.** Procedural, from the wear story in the spec. Paint colour comes from the doc's hex. Chipped edges come from ambient occlusion, rust streaks from stretched noise concentrated where the doc says, plus a scraped bottom band, roof dirt and grime in the grooves. All of it is computed in object space, so it bakes the same every time.
4. **Bake.** Cycles bakes the material into textures: base colour, and ORM (R occlusion, G roughness, B metallic).
5. **Export.**
   - FBX for Unreal: the meshes, sockets and collision.
   - GLB for the browser client: the render meshes, with the baked textures embedded.
6. **Preview.** Two renders, one of the procedural material and one of the baked textures (what Unreal gets), under the arena's sodium light.

In Unreal, materials, Chaos destruction, Niagara effects and MetaSounds are set up by Unreal editor scripts. That part starts once the engine is installed.

## Run it

```bash
/Applications/Blender.app/Contents/MacOS/Blender -b --factory-startup -P art/build.py -- shipping_container --variant htr
```

The output goes to `art/out/<asset>/<mesh>/`, which is not committed: run the script to rebuild it. The output is:
- `preview.png` and `preview_baked.png`;
- the baked textures;
- the `.fbx` and `.glb`;
- `report.json`, with triangle counts, size and sockets.

`--preview-only` skips the bake for quick looks, and `--res` sets the body texture size (default 2048).

## What is here

| Path | What it is |
|---|---|
| `build.py` | Builds one asset: preview, UVs, bake, export, report |
| `lib/kinetic_art.py` | The shared toolkit: boxes, cylinders, corrugated sheet, bevels, sockets, collision, the procedural painted-steel material, baking, export and the preview render |
| `props/shipping_container.py` | The 20 ft container, with variants `htr`, `meridian` and `guild` (docs/objects/heavy/shipping-container.md) |

Requires Blender 5.2 (`brew install --cask blender`).
