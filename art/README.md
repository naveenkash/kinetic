# KINETIC generated assets

Every asset in KINETIC is generated from code, from its spec in [docs/objects](../docs/objects) (and later docs/characters and docs/arenas). Nothing is hand-modelled, bought or outsourced. Each asset is a Python script; running it rebuilds the asset exactly, so when a spec changes the script changes and the asset is rebuilt.

## How an asset is made

1. **Geometry.** Blender, scripted. The script builds the meshes from the spec's dimensions: metres, Z up, X+ toward the object's front, pivot at the bottom centre of the footprint (objects/README-props.md).
2. **Gameplay data.**
   - Dog sockets are empties named `SOCKET_<name>`, which Unreal imports as static-mesh sockets.
   - Collision is convex `UCX_<mesh>_<nn>` hulls, shaped as the spec's Physics section lists: boxes, chamfered boxes, cylinders and cones circumscribed on the round parts, and prisms standing in for its capsules (legs, tubes).
   - Moving parts, such as a container's doors, are separate meshes with their pivot on the hinge.
3. **Materials.** Procedural, from the wear story in the spec. Paint colour comes from the doc's hex. Chipped edges come from ambient occlusion, rust streaks from stretched noise concentrated where the doc says, plus a scraped bottom band, roof dirt and grime in the grooves. All of it is computed in object space, so it bakes the same every time.
4. **Bake.** Cycles bakes the material into textures: base colour, and ORM (R occlusion, G roughness, B metallic).
   - **High to low.** Detail too dense for the spec's triangle budget is modelled in full as a high mesh and baked onto a low one (`ka.high_low`). Examples are the links of a chain coil, the wraps on a cable reel and the bars of a weld mesh. The bake adds a tangent-space normal map in the OpenGL convention (Y up), which is what glTF reads. Unreal expects DirectX, so the Unreal import ([game/Tools/unreal/import_props.py](../game/Tools/unreal/import_props.py)) sets Flip Green Channel on them.
   - **Masked cards.** A masked mesh is a card, like a crate's weld mesh or the tube a chain coil's links are baked onto. Where the bake's rays miss the high mesh, its BaseColor alpha is 0, and the material clips at 0.5. Cards are two-sided (the GLB says so, and Unreal's masked master material is two-sided); every other material is one-sided.
   - **Emission.** A glowing material, like a floodlight lens, bakes an Emissive map, and its strength goes into `report.json`.
   - **Budgets.** Each asset states its LOD0 budget from the spec. The build warns when the game meshes go over it.
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
- the baked textures: BaseColor, ORM (and the AO, Roughness and Metallic maps it is packed from), plus Normal for a high-to-low bake and Emissive where something glows;
- for a part with its own pivot (a container door), its own `.fbx`;
- the `.fbx` and `.glb`;
- `report.json`, with triangle counts against the budget, the high mesh's count, size and sockets.

`--preview-only` skips the bake for quick looks, and `--res` overrides the body texture size the asset sets (2048 for volatiles, crates and the container, 1024 for light props, per their specs).

## What is here

| Path | What it is |
|---|---|
| `build.py` | Builds one asset: preview, UVs, bake, export, report |
| `lib/kinetic_art.py` | The shared toolkit. Shapes: boxes, cylinders, lathes, rings, tubes, corrugated sheet, stencils, bevels. Gameplay data: sockets and collision. Procedural materials: painted steel, wood, fabric, plastic, galvanized, emissive. Also baking (high to low, masked cards), export and the preview render |
| `props/shipping_container.py` | The 20 ft container, with variants `htr`, `meridian` and `guild` (docs/objects/heavy/shipping-container.md) |
| `props/fuel_drum.py` | The 200 L fuel drum (docs/objects/volatiles/fuel-drum.md) |
| `props/propane_cylinder.py` | The propane cylinder; its parts are shared with the rack (docs/objects/volatiles/propane-cylinder.md) |
| `props/cylinder_rack.py` | The six-slot cylinder rack, frame only (docs/objects/volatiles/cylinder-rack.md) |
| `props/steel_crate.py` | The steel crate, with variants `htr` and `guild` (docs/objects/props-medium/steel-crate.md) |
| `props/plank.py` | The scaffold plank (docs/objects/props-light/plank.md) |
| `props/clamp_crate.py` | The crate of scaffold couplers (docs/objects/props-light/clamp-crate.md) |
| `props/chain_coil.py` | The heaped coil of chain (docs/objects/props-light/chain-coil.md) |
| `props/cable_reel_small.py` | The small cable reel, pivot on its axle (docs/objects/props-light/cable-reel-small.md) |
| `props/folding_chair.py` | The folding chair (docs/objects/props-light/folding-chair.md) |
| `props/tool_bag.py` | The canvas tool bag, with variants `guild` and `htr` (docs/objects/props-light/tool-bag.md) |
| `props/paint_can.py` | The 4 L marking-paint can, variant by paint colour (docs/objects/props-light/paint-can.md) |
| `props/bucket.py` | The 15 L bucket, with variants `grey`, `blue` and `mustard` (docs/objects/props-light/bucket.md) |
| `props/floodlight_stand.py` | The tripod floodlight, with variants `office`, `htr` and `guild` (docs/objects/props-light/floodlight-stand.md) |

Requires Blender 5.2 (`brew install --cask blender`).
