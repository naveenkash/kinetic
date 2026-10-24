# KINETIC: Chaos Destruction

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. Destructible definitions come from `docs/bible/05-core-mechanics.md` section 6 and the per-arena engine notes in `docs/bible/03-arenas.md`._

## 1. Engine facts this plan relies on (verified)

- Geometry Collections (GCs) are built in Fracture Mode with the Uniform, Cluster, Radial, Planar, Slice, Brick (experimental), Mesh and Custom tools. Each fracture pass creates a new level in the hierarchy; the root is Level 0 ([fracturing guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/fracturing-geometry-collections-user-guide)).
- With clustering on, higher levels (coarser clusters) break first ([clustering guide](https://dev.epicgames.com/documentation/unreal-engine/cluster-geometry-collections-user-guide-in-unreal-engine)).
- The component exposes `damage_model` (user-defined threshold per level, or material strength plus connectivity), a `damage_threshold` array per level, `use_size_specific_damage_threshold`, `damage_propagation_data` (`enabled`, `break_damage_propagation_factor`, `shock_damage_propagation_factor`), `enable_damage_from_collision`, `use_material_damage_modifiers`, `allow_removal_on_break`, `allow_removal_on_sleep`, `cluster_group_index`, `initialization_fields`, and replication fields `enable_replication`, `enable_abandon_after_level`, `replication_abandon_after_level`, `replication_max_position_and_velocity_correction_level` ([component API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionComponent), [damage model enum](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/DamageModelTypeEnum), [propagation struct](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionDamagePropagationData)).
- Fields: Anchor Fields set overlapped bones to Static; External Strain must exceed the Damage Threshold to fracture; Internal Strain lowers thresholds over time; Sleep/Disable and Kill fields remove bodies; Force Dynamic Switch wakes kinematic bodies without strain. Epic recommends avoiding strain falloff for performance ([Chaos Fields guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-fields-user-guide-in-unreal-engine)).
- Nanite can be enabled per GC in the Geometry Collection editor; opaque and masked materials only; DX12 SM6 required ([Nanite docs](https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)).
- 5.8 added per-transform linear and angular damping on GCs, `GetCurrentTransforms` Blueprint queries, and per-component Chaos Caching with runtime playback; Dataflow is production-ready ([5.8 release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)).
- Official documentation on damage propagation and field responses is thin. An Epic forum thread says so directly ([forum](https://forums.unrealengine.com/t/managing-geometry-collections-with-chaos-physics-field-responses-and-controlled-destruction/2557942)). Expect to calibrate by test.

## 2. Units: N·s fracture thresholds to engine damage thresholds

Chaos works in centimeters and kilograms. With `enable_damage_from_collision` on and the user-defined damage model, the strain a collision applies to a cluster scales with the collision impulse in engine units (kg·cm/s). The docs do not publish the exact formula. Working hypothesis, to be calibrated in week 4:

```
engine_threshold ≈ fracture_threshold_Ns × 100
```

| Destructible (6.5) | Threshold (N·s) | Starting engine value | Fragments |
|---|---|---|---|
| Market stall, furniture | 300 | 30,000 | Light planks |
| Scaffold section | 800 | 80,000 | Light boards, Light clamps, one Medium frame piece |
| Water tank | 1,500 | 150,000 | Medium shell pieces, releases force volume |
| Hanging house, small (1,200 kg) | 2,000 | 200,000 | Heavy wall pieces, Medium roof, Light contents |
| Hanging house, large (2,800 kg) | 3,500 | 350,000 | Heavy wall pieces, Medium roof, Light contents |
| Catwalk section | 2,500 | 250,000 | Medium grate pieces |
| Walkway on cable (Hangtown) | 2,500 | 250,000 | Medium deck sections, Light planks |
| Plank bridge (Yard) | 600 | 60,000 | Light planks |
| Shipping container | 6,000 | 600,000 | Does not fracture; dents, doors open, contents spill |

Epic's own fields example uses thresholds of 200,000 and 500,000 ([fields guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-fields-user-guide-in-unreal-engine)), which is the same order of magnitude, so the hypothesis is at least plausible. Player bodies count as impacting objects against these thresholds (6.5), so the Impact Monitor feeds player impulses into the same strain path. Calibration procedure: drop a 45 kg test sphere onto each GC from heights that give impulses at 0.8x, 1.0x and 1.2x the bible value; the 0.8x drop must not fracture, the 1.2x must. Record the multiplier per material in `DT_DestructibleTuning` and never hand-edit thresholds on placed instances.

Do not use the material-strength damage model at launch. It couples thresholds to connectivity in ways we cannot tune against a table.

## 3. Cluster strategy per object type

| Object | Levels | Level 1 (breaks first) | Level 2 | Level 3 | Notes |
|---|---|---|---|---|---|
| Market stall, furniture | 2 | 3 to 5 chunks (Planar) | 8 to 12 planks (Uniform) | none | All fragments Light, 10 s lifetime |
| Scaffold section | 3 | 1 cluster per bay (Custom) | frame piece (Medium) and board stack (Medium) (Slice) | single boards and clamps (Light) | Single boards are Light; board stacks are Medium (6.1) |
| Water tank | 2 | 2 halves (Planar) | 6 to 8 shell pieces (Uniform) | none | Break fires the water rush volume |
| Hanging house, small and large | 3 | 4 walls, roof, floor (Planar) | wall halves (Slice) | contents (separate Light props, not GC) | Walls Heavy, roof Medium. Two GC assets, thresholds 2,000 and 3,500 N·s |
| Catwalk section | 2 | 2 to 3 grate panels (Planar) | rails (Slice) | none | Panels Medium |
| Walkway on cable | 2 | 3 to 4 deck sections (Planar) | planks (Slice) | none | Sections Medium, planks Light |
| Plank bridge | 1 | 6 to 10 planks (Slice) | none | none | All Light |
| Shipping container | 1 | no fracture | | | Static mesh with a dent morph and a door hinge constraint; "spill" spawns pooled Light props |
| Billet stack (Yard) | 2 | each billet is one cluster | billet halves | | High thresholds, settle damping so it falls slowly enough to read (arena note) |
| Scrap heap (Yard) | none | | | | Pooled Light props, not a GC |
| Drive housing panel (Ballast) | 2 | panel (Planar) | shards | | Cosmetic plus Light props |

Damage propagation: `enabled = true`, `break_damage_propagation_factor = 0.5`, `shock_damage_propagation_factor = 0.1` as starting values. Shock propagation is what lets a 2,000 N·s hit on one house wall crack the far wall without breaking it outright.

Fragment tiering follows 6.5: fragments big enough to keep the parent tier keep it (set per level in the asset's size-specific data); the rest become Light props with a 10 s lifetime via `allow_removal_on_sleep` with a 10 s delay, and `allow_removal_on_break` off so pieces exist long enough to be hit.

## 4. Anchor fields for stacks and scaffolds

- **Container stacks (Ballast)**: each stack is one GC with one cluster per container, `cluster_group_index` per stack. An `FS_AnchorField_Generic` at the rack holds the bottom row Static. The line solver treats a racked container as invMass 0 until the vector sum of active pull (Haul, Jack push, Kick) meets the break-free value (05 section 6.2: rack 200 N, stack friction 500 N; Climber and Scrapper at 400 N cannot start a stack; thresholds inclusive). Hang lines never test break-free: a player can Pendulum from a stacked container with the anchor in place, up to the class Hang load. At break-free the Destruction Director removes the anchor for that cluster and applies a Force Dynamic Switch so it wakes. Released containers ride the racks' dead rollers (friction 0.02, 6.1a), which is what lets the Breathing slide them. On the Breathing lean, the arena timeline lowers the break-free value for stacks on the low side by 100 N, which is the "anchor field that releases on the lean" from the arena note, expressed as data rather than a physics guess.
- **Scaffold (Anchor arena)**: five rings, four quadrants, one GC per ring-quadrant. Anchor fields at the maintenance clamps hold Level 1 clusters Static. Pulling a clamp (Rated load, 1,000 N) removes that ring's anchor field; gravity does the rest. The Scaffold Collapse event removes a quadrant's anchors on all five rings in sequence from top to bottom at 0.4 s intervals, after the 5 s siren.
- **Hanging houses (Hangtown)**: each house is a GC kept kinematic (anchored) while its cable holds. The cable is a line-solver Hang constraint between the house root and the anchor point, with a rated 1,000 N break-free (6.2). Only three things drop a house: a Breaker's active pull, a Doubler at overspin, or any Cutter on the house's slack cable (0.6 s hold). Hang lines on a house never test break-free, and a line alone cannot swing a hanging house dangerously (a Breaker's 1,000 N deflects a 1,200 kg house to about 0.76 m/s); swings come from gravity, so the release path is: anchor removed, `Force Dynamic Switch`, and the house swings on its remaining constraint under its own weight. Hang loads add across lines (6.2a), so two Riggers can hold a 2,400 kg house that one cannot. Four dynamic houses maximum at once (arena note); the Destruction Director refuses a fifth release and queues it, and far houses stay kinematic on the sway animation.

## 5. Pre-fractured scripted events

The mid-match event fires at 1:15 remaining in a Scrap round, 4:00 remaining in Claim, 2:00 remaining in each Haul half and 3:00 remaining in Free Scrap (05 section 9.2, v0.2.2). All three use the same recipe: the collapsing section is a GC authored and fractured at build time, held by an anchor field, with any attached pieces (catwalks, walkways) parented to it. The event is a Field System actor on the arena timeline. Chaos Caching (per-component, runtime playback in 5.8) is used where the fall must be identical every match.

| Event | Trigger | Mechanism | Cosmetic prediction |
|---|---|---|---|
| **The Shear** (Ballast, mid-match event) | Timeline | 5 s siren. At 0 s the bay wall GC loses its anchor and a directional External Strain of 2x its threshold is applied along the outward normal. Its catwalks are children of the wall GC's root and fall with it. Chaos Cache playback for the wall; live simulation for players and props on it | Clients start the cache at the replicated event time |
| **Cable Day** (Hangtown, mid-match event) | Timeline | The market ring's main cable constraint breaks. The ring is a kinematic platform that rotates to 40° over 2.5 s along an authored curve (not simulated). Houses on the failed cable are released (up to the four-dynamic cap) along pre-authored swing paths implemented as kinematic splines for the first 3 s, then handed to simulation | Deterministic from the clock |
| **Scaffold Collapse** (Anchor, mid-match event) | Timeline | Section 4 sequence. Planks become Light props. Survivors of the GC remain as ledges because their anchors are kept | Server break events, local fragment sim |
| **Shift Change** (Yard) | Timeline | Not destruction: Hauler schedule doubles and the Gantry releases its load early | Clock |

The Gantry load and the Lift counterweight are kinematic rated attachments (6.7): kinematic on the Gantry's or Lift's path while attached, released by the script at the drop point, and converted to a simulated rated load early if a Breaker's line on them exceeds the 1,000 N break-free.

## 6. Fragment lifetime and pooling

- Light fragments: removal on sleep after 10 s; hard cap by Kill field after 20 s.
- Medium and Heavy fragments persist until round reset. They are gameplay objects.
- Spill contents (container doors, market goods) come from a per-arena prop pool: 150 Light props in the Yard scrap sectors, 60 elsewhere. Pools are pre-spawned at map load and parked below the Well volume with collision off.
- Round reset (05 section 9.2) resets every GC to its rest collection. Use the GC component's reset-to-rest path rather than respawning actors; respawning 40 GCs between rounds is a hitch.

## 7. Rated-load consequences as Field System triggers

Rated loads are simulated bodies that the solver treats as invMass 0 until the **magnitude of the vector sum of active pull** (Haul, Jack push, Kick) from every line on them reaches 1,000 N along one axis: a Breaker alone, a Doubler at overspin, or a crew whose lines add to it (6.2a, v0.2.2). Hang lines contribute nothing to this sum. When it is reached the Destruction Director:

1. Plays the 1.0 s minimum structural groan and dust tell (05 section 11).
2. Lets the load move (it is simulated). At 1,000 N on 3,700 to 6,000 kg that is 0.17 to 0.27 m/s², several seconds to visible motion. The slowness is the tell and is intended (v0.2.1).
3. Fires the authored consequence as a Field System actor: column drop (anchor removal plus External Strain on a ceiling GC), clamp pull (ring anchor removal), magnetic rack release (Ballast), anchor pin pull (Hangtown walkway GC release), counterweight drop (Anchor Rim).

Each consequence is a row in `DT_RatedLoads` naming the load actor, the field actor, the lead time and the reset behaviour. Consequences reset only between rounds.

## 8. Replication strategy

The engine exposes per-component replication of GC state with the ability to stop replicating below a cluster level ([component API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionComponent)). There is no official end-to-end guide for multiplayer destruction, and community reports show replication issues around Force Dynamic Switch and client-side collision ([forum sample](https://forums.unrealengine.com/t/ue5-chaos-replication/2740297)). Plan accordingly:

- `enable_replication = true`, `enable_abandon_after_level = true`, `replication_abandon_after_level = 1` on every GC. Level 1 clusters (the gameplay-relevant pieces: a container, a wall, a billet) replicate position and velocity; everything below is local.
- `replication_max_position_and_velocity_correction_level = 1`.
- The server decides every break. Break events are reliable multicasts carrying the cluster index, the strain vector and the server physics frame. Clients apply the break locally with an External Strain of exactly threshold plus epsilon at that cluster so their local pattern matches. Sub-fragment positions will differ slightly between clients; that is accepted for Level 2 and below because those fragments cannot Stall (they are Light).
- Medium fragments (Level 2 boards, roof pieces) that can Stall a player are an exception: their impulses are only evaluated on the server, so divergence affects visuals only.
- Clients may predict a cosmetic crack (shock propagation decal, dust) on local impact before the server break arrives. They never fracture locally first.
- Fragment velocity for the break multicast is quantized to 0.1 m/s; position to 1 cm.

## 9. Nanite and LOD policy

- Enable Nanite on every GC whose rest mesh exceeds 5,000 triangles and whose materials are opaque or masked. All structural GCs qualify. Internal faces get the same material slot as the exterior with a grime parameter.
- Virtual Shadow Maps expect Nanite geometry; non-Nanite meshes are much more expensive to shadow ([VSM docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine)). Fragments that are not Nanite (Light props with translucent tape decals, for instance) must have a 3-LOD chain.
- Masked materials only where needed (nets, mesh fencing). No translucency on GCs.
- Instance count is nowhere near the 16 million Nanite cap. The constraint is the collision proxy, not rendering: every GC leaf needs a convex or box proxy under 32 vertices; `collision_particles_fraction` at 0.5 for Level 2 and below.

## 10. Budgets

Server and client budgets, per arena. "Active" means simulating and awake.

| Arena | GC components placed | Active GCs (fracturing or awake) | Awake fragments | Dynamic houses | Pooled Light props | Notes from arena doc |
|---|---|---|---|---|---|---|
| Ballast | 48 (24 containers in 6 stacks, 8 catwalks, 4 tanks, 12 housing panels) | 12 | 300 | n/a | 60 | Per-stack strain, anchor releases on lean, Shear pre-fractured |
| The Yard | 36 (12 billet stacks, 8 plank bridges, 1 control shack, 15 hull sections) | 10 | 250 | n/a | 150 (scrap heaps, budgeted per sector) | Haulers kinematic; Gantry load is a kinematic rated attachment |
| Hangtown | 44 (12 houses, 16 stalls, 8 walkways, 8 roof tanks) | 8 | 250 | 4 | 60 | Distant houses kinematic sway |
| The Anchor | 40 (20 scaffold sections, 8 rim cranes, 4 towers, 8 misc) | 12 | 400 (collapse peak) | n/a | 60 | Collapse is top-down sequenced to spread cost |

Global caps: 12 active GCs, 400 awake fragments, 96 awake non-GC rigid bodies. The Destruction Director enforces caps by putting the oldest Light fragments to sleep and, beyond that, killing them. Server physics budget for destruction is 3 ms of the 16.7 ms frame (see `performance-budgets.md`).

## 11. Tools workflow for artists

1. Author the rest mesh as a static mesh with closed shells. Name: `SM_<Arena>_<Object>` (for example `SM_YRD_Billet_A`).
2. In the level, select the static mesh actor, open Fracture Mode, click New to create the GC. Save as `GC_<Arena>_<Object>` in `Content/Arenas/<Arena>/Destruction/`.
3. Level 1: Planar or Custom tool with hand-placed sites to make the gameplay pieces. Name clusters in the hierarchy panel (`Wall_N`, `Roof`, `Floor`). Gameplay code addresses clusters by name through a per-asset map, never by index.
4. Level 2: Uniform or Slice with a fixed Random Seed so every rebuild is identical. Grout 0 for structural, 1 cm for wood.
5. Clustering: Enable Clustering on. Auto Cluster only at Level 3.
6. Set Damage Threshold per level from `DT_DestructibleTuning` (the engine value, not N·s). Use Size Specific Damage Threshold off. Damage propagation on with the factors from section 3.
7. Collision: convex per leaf, max 32 verts. Check `Collision Particles Fraction` as above.
8. Removal: Remove On Sleep on for Light levels, 10 s; off for Medium and Heavy levels.
9. Anchor: place `FS_AnchorField_Generic` (or the project `FS_KineticAnchor`) and add it to Initialization Fields. Name `FS_<Arena>_<Object>_Anchor`.
10. Nanite: enable in the GC editor if the mesh qualifies.
11. Validate with the editor tool `Kinetic.ValidateGC` (in `project-structure.md`): checks thresholds against the table, leaf vertex counts, removal flags, names and the abandon level. Run `Kinetic.ValidatePlacements` on the level: any Heavy object whose placement friction is above 0.05 is flagged immovable and must be either re-staged on rollers (0.02) or skids (0.05) or tagged fixed scenery (6.1a). Water tanks are fixed by rule.
12. Test in PIE with `p.Chaos.DebugDraw.Enabled 1` and the Chaos Visual Debugger. Record a 45 kg drop test and attach the CVD capture to the asset review.
