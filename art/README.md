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
   - GLB: the render meshes with the baked textures embedded, a preview any glTF viewer opens.
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

## Arenas

The Yard is drawn by a visual kit over the grey-box level, and the grey boxes stay underneath for collision and the navmesh. It is built in three steps:

1. **Surfaces.** `art/surfaces/build_surfaces.py` bakes the arena's tiling surfaces to `art/out/surfaces/<name>/`:
   - the surfaces are Concrete, Slag, HullSteel, Rust, Corrugated, Grating, Hazard, Ochre, HTROrange and DarkSteel;
   - each has BaseColor, ORM and Normal maps, and its tile size in metres;
   - every noise is sampled on a 4D torus, so the maps tile with no seam.
2. **The kit.** `art/kit/yard_kit.py` reads `art/kit/yard_layout.json` and builds 16 meshes, `art/out/kit/SM_Yard_<Group>.fbx`:
   - `make -C game yard-greybox` writes the layout file: every box with its role and label, and every ladder.
   - The meshes cover the slabs and their lip angles, the trench walls and rail beds, and the hull-section Stacks.
   - They also cover the corrugated perimeter walls, the portal legs, crossheads and rail walks, and the lattice masts.
   - The rest are the press and the Well, ladders and cages, the grate bridges, the trucks and other placeholders as objects, the scrap heaps, and clutter.
   - `art/kit/kitlib.py` holds the shapes. Faces are UV'd in metres, so a surface repeats at its true size.
   - `art/kit/covered.json` lists the boxes the kit draws over. The press ram and the plank bridges move or break, so they are not covered.
3. **Unreal.** `make -C game import-kit` imports the surfaces into `M_KineticSurface` instances and the kit as Nanite meshes. Then `make -C game yard-greybox` places the kit and stops drawing the covered boxes.

```bash
blender -b --factory-startup -P art/surfaces/build_surfaces.py -- --res 1024
```

```bash
blender -b --factory-startup -P art/kit/yard_kit.py
```

The level's night lighting is set in `build_yard_greybox.py`:
- the sodium cloud deck is a dim emissive sky dome, which a real-time sky light captures as the fill;
- the mast, shack, press and pit-stand floods are spotlights in candela;
- a cage lamp hangs over each spawn;
- exposure is manual, tuned on `make -C game shots`;
- Lumen lights it, with virtual shadow maps.

## Characters

`art/characters/harness.py` builds the five harness classes (docs/characters/harnesses) on one shared skeleton with Unreal Mannequin bone names:

```bash
blender -b --factory-startup -P art/characters/harness.py -- [--class Rigger] [--res 2048]
```

Each is a skin-modifier body weighted to the skeleton (with hands and a face), the class's harness from `harness_parts.py` (Catch Plates at their class sizes with lips, chamfers or mirror faces, bolts and stamped numbers, body-fitted straps and buckles, the spine frame and gel reservoirs, the Spool with its mode ring, Dogs and line exit, the Cutter, boots, headgear, the class's extras), and six takes (Idle, Run, Climb, Fall, Brace, Stagger). The livery is baked to 2K textures; the team-tint parts keep a slot of their own. `make -C game import-characters` imports them. These are the stand-ins until MetaHuman supplies the faces and bodies (PROPOSED: the harness stays code-built and is fitted onto them).

