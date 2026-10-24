# KINETIC — Art Pipeline and Quality Bar

_How an object goes from its spec document to a shipped asset. Every object in docs/objects, every character in docs/characters, and every arena in docs/arenas follows this._

## 1. Read the spec

Each object has one markdown file. Before modeling, the artist confirms from the spec:

- Dimensions and pivot.
- Mass tier and whether it is destructible, volatile, rated, or kinematic.
- Dog seat sockets (where a Dog may attach; named per objects/README-props.md).
- States (intact, damaged, destroyed) and variants.
- The arena(s) it appears in and the lighting it must read under.

If the spec is missing any of those, the spec is fixed first.

## 2. Blockout

- Build at exact spec dimensions in meters. Unreal units are centimeters.
- Pivot at the spec location (usually base center for props, hinge for doors and hatches, attachment point for hanging objects).
- Export to engine immediately as a greybox with correct collision and mass. The level designer places greyboxes; nothing waits on art.
- Verify in engine: mass tier behaves per bible/05 section 6 (a Scrapper's 4 kN Line should move a 180 kg drum at about 2.3 g; a 2,000 kg container should barely creep).

## 3. High-poly and bake

- Hero props (anything a player will stand next to for a full round: containers, haulers, the Spool, harnesses, hanging houses) get a sculpted high-poly and baked normals.
- Everything else uses mid-poly with weighted normals and a trim-sheet workflow from the shared material library (objects/README-props.md).

## 4. Materials

- Shared material library first. Use the listed base colors and roughness ranges. A new master material needs the technical artist's approval.
- Weathering tells the object's story per its spec: where hands touch, where rust runs, where a prep crew stenciled it. No uniform grunge.
- Team tint, faction livery, and emissive parameters are material parameters, never baked.
- Readability test: place the asset in the arena's darkest zone and its brightest zone. A Line at 4 kN must out-glow the asset in both (bible/03 map rule 8).

## 5. Destruction

For anything with a fracture threshold in its spec:

- Fracture in the Fracture Editor following tech/chaos-destruction.md: cluster levels, damage thresholds mapped from the spec's N·s number, anchor fields for stacked or hung states.
- Fragments must match the spec's fragment list and tiers. A hanging house breaks into Heavy walls, Medium roof, Light contents, not into uniform chunks.
- Verify the tell: break-free lurch and the 0.2 s shudder before release (bible/05 section 11).

## 6. LODs and budgets

- Triangle budgets per LOD are in each spec's Technical section. Nanite is used for static environment geometry; see tech/chaos-destruction.md for its policy on geometry collections.
- Every asset has LOD0–LOD3 or Nanite enabled, never neither.
- Texture resolution per the spec. Default: 2K for props, 4K for hero props and harnesses, 1K for Light props.

## 7. Sockets and gameplay hooks

- Dog seat sockets named per convention. Test: fire a Dog at every face of the object; it seats where the spec says and nowhere it shouldn't.
- Catch Plate sockets on harnesses: chest, back, shoulder_l, shoulder_r. Line exit points tested with the cut-self-free turn animations.
- Volatiles: trigger volume, blast origin socket, venting socket for the 0.3 s hiss tell.

## 8. Audio and VFX hooks

- Physical material assigned (steel, concrete, wood, canvas, water) so impact sounds and sparks resolve by surface.
- Emissive and audio parameters wired to the force value for anything that sings (Lines, slipping Hang lines, stressed cables).

## 9. Review

A prop ships when:

1. It matches its spec's dimensions, states, and variants.
2. It behaves per bible/05 in a test arena (mass, break-free, fracture, volatile).
3. It passes the readability test against a Line in its arena.
4. Its tells play with the lead times in bible/05 section 11.
5. It is within its LOD and texture budget.
6. The level designer has placed it with a job written in the prop manifest.

## 10. Characters

- Shared humanoid skeleton with class-specific sockets (characters/README.md).
- Five harness classes are five bodies; Faces are head, hands, costume, and material variants on their class body. A Face never changes the class silhouette.
- Every animation in systems/animation.md exists on the shared skeleton before any Face is finished.
- First-person arms are a separate mesh per class with the Spool and Cutter at spec positions.

## 11. Arenas

- Greybox from layout.md with the ASCII plans as the source. All zones at spec dimensions.
- Two weeks of daily playtests on greybox before any art.
- Art in this order: structures, kinematics, hero props, ground and walls, dressing, lighting, audio, VFX.
- Lighting per lighting-and-mood.md. Readability test in every zone with a Line at 4 kN and at 10 kN.
- Every play in signature-plays.md recorded on video by QA before the arena is called done.
