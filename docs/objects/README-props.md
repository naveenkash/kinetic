# KINETIC: Props, Volatiles, and Heavy Objects: Index and Shared Rules

_Object spec index v0.1. Every number here defers to `docs/bible/05-core-mechanics.md`. Where a value is not in 05 it is marked **PROPOSED**._

## How to read an object doc

Each file in `props-light/`, `props-medium/`, `volatiles/`, and `heavy/` is standalone. It carries its own mass, collision, break-free, fracture, impact numbers, sockets, LOD budgets, and replication class. This README holds only what is shared: the index, the Dog seating rule, the material library, naming and pivot conventions, tier budgets, and the impact quick-reference every doc computes against.

## Index

### Light props (under 30 kg): `props-light/`

| File | Object | Mass | One line |
|---|---|---|---|
| folding-chair.md | Folding chair | 6 kg | Scrap-heap and market filler. Never hurts anyone alone. Breaks at 300 N·s. |
| chain-coil.md | Chain coil | 25 kg | Heaviest Light prop. Flips a player off a Haul. The Scrapper's favorite. |
| plank.md | Plank | 12 kg | Scaffold and bridge board. Stagger at Haul cap. Wood surface for Spike Dogs. |
| tool-bag.md | Tool bag | 20 kg | Soft, dense, staggers. Guild and HTR variants. |
| paint-can.md | Paint can | 5 kg | Smallest prop. Leaves a paint decal on fracture. Pure clutter and tell. |
| laundry-bundle.md | Laundry bundle | 8 kg | Hangtown soft prop. Tangles Dogs. ×2.0 Stall surface when piled. |
| bucket.md | Bucket | 10 kg (full) | Spills water on tip. Rolls. Empty variant 2 kg. |
| clamp-crate.md | Clamp crate | 28 kg | Scaffold clamps in a steel crate. Fragments of the Anchor's Scaffold. |
| floodlight-stand.md | Floodlight stand | 22 kg | Yard and Hangtown light. Emissive. Falls over and strobes out. |
| cable-reel-small.md | Cable reel (small) | 28 kg | Rolls on its flanges. Flip at Haul cap. |
| scrap-debris-kit.md | Scrap debris kit | 1–9 kg each | Twelve junk pieces that make a scrap heap (×2.0 surface). |

### Medium props (30–300 kg): `props-medium/`

| File | Object | Mass | One line |
|---|---|---|---|
| steel-crate.md | Steel crate | 250 kg | The stackable Medium. Avalanche fodder. Stall at 5 m/s. |
| cart-trolley.md | Cart trolley | 120 kg | Rolls. Scrapper's Caboose on wheels. |
| scaffold-board-stack.md | Scaffold board stack | 160 kg | Strapped planks. Fractures into 12 Light planks. |
| market-stall.md | Market stall | 90 kg | Hangtown canvas and timber. Fractures at 300 N·s. |
| water-barrel.md | Water barrel | 220 kg full | Blue HDPE drum. Floats empty. Stall at 6 m/s full. |
| rebar-bundle.md | Rebar bundle | 280 kg | Long, rolls on a tilt, skewers nothing but reads like it does. |
| brick-pallet.md | Brick pallet | 290 kg | Half pallet of block. Fractures into Light bricks. |
| welding-cart.md | Welding cart | 140 kg | Cart with two gas cylinders. A volatile on wheels. |
| hand-truck.md | Hand truck | 35 kg | Two-wheel dolly. Lightest Medium. Rolls and tips. |
| stall-gel-pad.md | Stall Gel pad | 40 kg | Prep-crew landing pad. ×4.0 Stall threshold. Rare. |

### Volatiles: `volatiles/`

| File | Object | Mass | One line |
|---|---|---|---|
| propane-cylinder.md | Propane cylinder | 45 kg | 8 m/s trigger. 4 m radius, 1,500 N·s. The Kiss. |
| fuel-drum.md | Fuel drum | 180 kg | 10 m/s trigger. 3 m radius, 1,200 N·s, 6 s fire patch. |
| demolition-charge.md | Demolition charge | 12 kg | 6 m/s trigger. 5 m radius, 2,500 N·s. Breaks what it is bolted to. |
| gas-manifold.md | Compressed-gas manifold | fixed world geometry | No line moves it. A Jack over 600 N applied to it vents a 6 m jet at 15 m/s for 3 s. |
| cylinder-rack.md | Cylinder rack | 95 kg empty, 365 kg full | Six propane cylinders at 200 N break-free each. Chain reaction in a frame. Tier recomputes as cylinders leave. |

### Heavy and Rated: `heavy/`

| File | Object | Mass | One line |
|---|---|---|---|
| shipping-container.md | Shipping container (20 ft) | 2,000 kg | Does not fracture. Dents. Doors blow open at 6,000 N·s. Rides dead rollers in Ballast. The Avalanche. |
| billet-crate.md | Billet crate | 1,800 kg | Weave billets in a steel cage. The Yard's stacks. |
| hanging-house-small.md | Hanging house (small) | 1,200 kg | One-room shanty on cable. Fractures at 2,000 N·s. |
| hanging-house-large.md | Hanging house (large) | 2,800 kg | Two-storey hanging house. Hangtown's wrecking ball. |
| lift-counterweight.md | Lift counterweight | 4,500 kg, Rated | Car-sized concrete block on the Hangtown Lift. Already moving. |
| water-tank.md | Water tank | 2,500 kg full | Fixed (05 §6.1a). Fractures at 1,500 N·s and releases a flood volume. |
| claim-marker.md | Claim marker | 900 kg | Weave billet on a pallet, on polished runners (0.05). 4 m radius. A Rigger or heavier moves it. Resets from the Well. |
| haul-spool.md | Haul spool | 2,000 kg | The Haul mode payload, on rollers (0.02). Any class starts it; a crew moves it. |
| crane-counterweight.md | Crane counterweight | 6,000 kg, Rated | Rim crane ballast. The Anchor Drop. |
| scrapped-body.md | Scrapped body | 90 to 220 kg by class, Medium | An eliminated player for 4 s. Keeps its class mass. Non-fracturing prop. In poor taste. |

Note on folders: the folder is the art bucket, not the mass tier. `lift-counterweight` and `crane-counterweight` are Rated tier per 05 §6.1 and 03. `scrapped-body` is Medium tier per 05 §7.4. Each doc states its own tier.

## The Dog seating rule

05 §3.1: a Dog seats in any static world surface, any dynamic object, and any Catch Plate. Sockets never restrict where a Dog can land. Sockets exist for four reasons:

1. **Snap.** A Dog that lands within **15 cm** of a socket snaps to it. This gives a clean visual seat and a deterministic constraint attach point across server and clients.
2. **Surface class.** Each socket carries one of `Metal`, `Concrete`, `Wood`, `Rock`, `Fabric`. This drives the Mag and Spike Dog load rules (04, Dogs) and the seat clank sound. A Dog that lands off-socket uses the physical material under the hit instead.
3. **Reseat.** 05 §3.5: when an object shatters, the Dog reseats in the largest fragment. It reseats to that fragment's `_01` socket.
4. **Bots and the Apprenticeship.** Bot aim and tutorial prompts target sockets.

### Naming

`SKT_Dog_<Zone>_<NN>` on the static mesh or on the geometry collection root. `<NN>` is two digits from 01. Zones:

| Zone | Use |
|---|---|
| `Top` | Upper face. Guillotine and Gallows anchors. |
| `Bot` | Underside. Only on objects that can roll or tumble. |
| `SideN`, `SideS`, `SideE`, `SideW` | Vertical faces, object-local. X+ is N. |
| `End` | Ends of long objects (planks, rebar, reels). `End_01` is X+. |
| `Handle` | Carry handles, hand grips. |
| `Rim` | Lips of open containers. |
| `Lid`, `Door` | Hinged parts. A Dog here applies force to the hinge body. |
| `Hook` | Lifting lugs, eyes, shackles. Preferred Hang anchor. |
| `Frame` | Structural members of carts, racks, stands. |
| `Load` | The payload of a carrier (billets on a crate, cylinders in a rack). A Dog here can pull the load off the carrier. |
| `Valve` | Volatiles only. A Dog here makes the hiss tell louder. |

Fragments: `SKT_Dog_F<fragment>_<NN>`, fragment index matching the geometry collection's Level 1 cluster order. Every fragment that inherits Medium or Heavy tier needs at least one.

### Minimum socket counts

| Tier | Minimum | Must include |
|---|---|---|
| Light | 2 | one `Top` or `Handle` |
| Medium | 4 | one `Top`, one `Hook` if it has lugs |
| Volatile | 4 | one `Valve`, one `Top` |
| Heavy | 6 | two `Hook`, one `Top`, one per long side |
| Rated | 4 | all on the yellow-and-black taped surface |

### Socket orientation

Socket X axis is the outward surface normal. Socket Z is object-up where possible. The Dog mesh is oriented by the socket. No socket sits within 5 cm of a mesh edge, so the Dog does not clip.

### Catch Plate parity

Players carry exactly four Catch Plates (05 §5.5). Objects do not. Do not name object sockets `Chest`, `Back`, `Shoulder`.

## Shared material library

Base colors are sRGB hex for the albedo mid-tone. Roughness is the range across the wear mask, clean to worn. Metallic is binary unless stated. All materials are instances of three master materials: `M_Master_Hard` (steel, concrete, plastic), `M_Master_Soft` (canvas, fabric, rope), `M_Master_Emissive`. Every hard material has a `Wear` parameter 0–1 that drives edge chipping, grime in cavities, and the rust mask. Every material has a `Grime` parameter per arena (see below).

| Material instance | Base color | Roughness | Metallic | Notes |
|---|---|---|---|---|
| `MI_Steel_Painted_Worn` | paint color param, default `#5B6670` | 0.45–0.70 | 0 on paint, 1 where chipped | The workhorse. Chip mask reveals `MI_Steel_Bare` then `MI_Steel_Rusted`. |
| `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 | 1 | Mill scale, cut edges, fresh torch cuts (blued `#3B4A5E` within 2 cm of a cut). |
| `MI_Steel_Galvanized` | `#9EA3A6` | 0.35–0.55 | 1 | Spangle pattern normal. Scaffold, clamps, grating, fencing. |
| `MI_Steel_Rusted` | `#7A3F1C` to `#A8562A` | 0.75–0.95 | 0.25–0.40 | Flaking at 0.9 wear. Never on HTR-owned objects below 0.4 wear. |
| `MI_Chain_Oiled` | `#3C3E42` | 0.30–0.45 | 1 | Dark oil film. Rust only in the links that do not move. |
| `MI_HTR_Orange_PowderCoat` | `#E8620C` | 0.30–0.45 | 0 | Halvard-Teague Reclamation. Replaced every match in fiction: low wear, sharp chips. |
| `MI_HTR_Grey_Matte` | `#4A4D50` | 0.60–0.70 | 0 | HTR secondary. Machined plates, Spool housings, crate bodies. |
| `MI_Guild_Ochre` | `#C8912A` | 0.55–0.75 | 0 | Lay Guild. Brushed on, not sprayed. Visible brush direction in the roughness. |
| `MI_Guild_Black` | `#1C1B19` | 0.60–0.80 | 0 | Guild secondary. Tape and stencil. |
| `MI_Tape_Rated` | `#F2C230` / `#141414` | 0.50–0.60 | 0 | Yellow-and-black structural tape (03 rule 7). 50 mm chevrons, 45°. Only on Rated loads. |
| `MI_Tape_Slack` | `#FF7A00` | 0.40–0.50 | 0 | Hangtown slack-cable tape (03). Emissive 0.15 so it reads in the dark. Never on anything but cuttable cable. |
| `MI_Office_White` | `#E6E4DD` | 0.55–0.70 | 0 | Office of the Lay stencils and Marshal property marks. |
| `MI_Meridian_Cyan` | `#2A9FB5` | 0.60–0.80 | 0 | Pre-Severance Meridian Consortium livery. Always at 0.8+ wear: sixty-one years old. |
| `MI_Ascendancy_White` | `#ECEAE4` | 0.25–0.40 | 0 | Ascendancy wraps and perch marks. |
| `MI_Ascendancy_Silver` | `#C9CCD1` | 0.15–0.25 | 1 | Ascendancy tape. Mirror finish. |
| `MI_Fray_Neon_Magenta` | `#FF2BD6` | 0.35–0.50 | 0 | Fray tape. Emissive 0.3. |
| `MI_Fray_Neon_Green` | `#39FF14` | 0.35–0.50 | 0 | Fray tape. Emissive 0.3. |
| `MI_Hanger_Paint` | palette: `#2E8B8B` teal, `#B5302A` red, `#D9A441` mustard, `#6E4C9E` violet | 0.50–0.75 | 0 | Hangtown hand paint. Each house and stall picks two. Drips allowed. |
| `MI_Plywood_Weathered` | `#9C8564` | 0.80–0.92 | 0 | Grey-silvered face, delaminating edges. |
| `MI_Timber_Rough` | `#7A5C3A` | 0.78–0.90 | 0 | Sawn softwood. Scaffold boards, stall frames, bridges. |
| `MI_Timber_Dark` | `#4A3522` | 0.70–0.85 | 0 | Oil-stained or old. Hangtown floors. |
| `MI_Canvas_Natural` | `#B9AE93` | 0.90–1.00 | 0 | Guild canvas, stall awnings. Subsurface 0.2. |
| `MI_Tarp_Blue` | `#2B5F9E` | 0.55–0.70 | 0 | Poly tarp. Sheen in the normal. Hangtown and Yard. |
| `MI_Tarp_Orange` | `#D9641E` | 0.55–0.70 | 0 | HTR site tarp. Distinct from powder coat by roughness and weave. |
| `MI_Fabric_Laundry` | palette per bundle | 0.85–1.00 | 0 | Cotton and poly. Subsurface 0.3. |
| `MI_Rope_Sisal` | `#B59D6E` | 0.90–1.00 | 0 | Hangtown lashings. |
| `MI_Concrete_Cast` | `#8E8B84` | 0.85–0.95 | 0 | Counterweights, blocks. Formwork seams and tie holes. |
| `MI_Concrete_Stained` | `#6F6C66` | 0.88–0.98 | 0 | Water staining, efflorescence `#D8D4C8`. |
| `MI_Rubber_Black` | `#1E1F20` | 0.70–0.85 | 0 | Tires, bumpers, hose. |
| `MI_HDPE_Blue` | `#1F4FA3` | 0.40–0.55 | 0 | Water barrels, buckets. Subsurface 0.15 so liquid level reads. |
| `MI_HDPE_Black` | `#232426` | 0.45–0.60 | 0 | Fuel-rated plastic. |
| `MI_Cylinder_Propane_Grey` | `#B8B4A8` | 0.40–0.60 | 0 | Propane cylinder paint. Dented, scraped to bare steel on the foot ring. |
| `MI_Drum_Fuel_Red` | `#8E2A1F` | 0.45–0.65 | 0 | Fuel drum. Oxidized to `#B04A3A` at high wear. |
| `MI_Weave` | `#1A2430` | 0.18–0.28 | 1 | Meridian Weave. Anisotropic (0.8) along the lattice. Faint iridescence `#3A6E7F` at grazing angles. The thing everyone wants. |
| `MI_StallGel` | `#3FE0D0` | 0.10–0.20 | 0 | Emissive 2.0. Subsurface 0.6. Only on Stall sequence VFX and the gel pad. |
| `MI_Glass_Lamp` | `#E8EEF2` | 0.05–0.10 | 0 | Floodlight lenses. Emissive driven by the light actor. |
| `MI_Decal_Stencil` | `#E6E4DD`, `#141414`, `#E8620C`, `#C8912A` | inherits | 0 | Deferred decal atlas `T_Decal_Stencils_01`. All text and logos. |
| `MI_Decal_Grime` | `#2B2622` | inherits | 0 | Drips, oil, boot scuffs, hand polish. |

### Per-arena grime parameter

Set on the material instance by the arena's grime volume at load. It changes tint and dust, never the base material.

| Arena | Grime tint | Character |
|---|---|---|
| Ballast | `#3A4A3C` green-black | Seep water, algae in the Hold, salt white on the Crown. |
| The Yard | `#2A2622` oil-black | Cutting dust, slag spatter, rail grease. |
| Hangtown | `#6E5A3C` warm dust | Cooking smoke, chalk, laundry starch. |
| The Anchor | `#B8B2A4` pale | Concrete dust, salt, bird lime on the Rim. |

### Who stencils what

| Mark | Material | Where it appears |
|---|---|---|
| **HTR** block letters, asset tag `HTR-RC-####`, orange band | `MI_Decal_Stencil` orange on grey | Anything HTR owns: crates, containers in the Yard, charges, gantry loads. |
| **Lay Guild** hitch-knot glyph, Ledger number `LG-####` | ochre on black | Guild tool bags, scaffold, the Yard's own plant, Haul spool. |
| **OFFICE OF THE LAY / SITE PROPERTY**, Marshal number | white | Every staged volatile. Gel pads. Claim marker. Rated tape. |
| **MERIDIAN** hexagon, pre-Severance cargo codes | cyan at high wear | Original Strand cargo in Ballast: containers, drums, tanks. |
| Hangtown neighborhood stencils (a hand, a kite, a kettle) | hanger palette | Houses, stalls, laundry poles, Scrapper reserve riggers. |
| **Nurri Stall-Gel**, **Kessl Harness Leasing**, **Meridian Mutual**, **LayCast** | sponsor colors | Broadcast surfaces: gel pads, floodlights, container sides facing cameras. |
| **PROPOSED** suppliers: **Rucu Gas** (propane), **Basin Petrol Co-op** (fuel), **Collar Four Press Works** (Yard plant) | white or faded | Volatiles and Yard equipment. Mark as PROPOSED until approved. |

## Naming, pivot, and axes

- Static mesh `SM_<Category>_<Object>_<Variant>`. Geometry collection `GC_<Object>_<Variant>`. Physics asset `PHYS_<Object>`.
- Pivot at the **bottom center of the resting footprint** for anything that stands on a floor. Pivot at the **primary hang point** for anything whose authored state is hung (hanging houses, lift counterweight). Pivot at the **axle center** for anything whose authored state is rolling (reels, spools).
- X+ is the object's long axis or front. Z+ is up. Unreal units: 1 uu = 1 cm.
- Center of mass is authored in the physics asset, never left to the convex hull.

## Tier budgets

| Tier | LOD0 | LOD1 | LOD2 | LOD3 | Texture sets | LOD switch (screen size) |
|---|---|---|---|---|---|---|
| Light | 2,000 | 1,000 | 400 | 120 | 1 × 1K (BC, N, ORM) | 0.5 / 0.2 / 0.08 |
| Medium | 6,000 | 3,000 | 1,200 | 300 | 1 × 2K | 0.6 / 0.25 / 0.1 |
| Volatile | 5,000 | 2,500 | 1,000 | 250 | 1 × 2K + decal | 0.6 / 0.25 / 0.1 |
| Heavy | 16,000 | 8,000 | 3,000 | 600 | 2 × 2K + trim | 0.8 / 0.35 / 0.12 |
| Rated / landmark | 24,000 | 12,000 | 4,000 | 800 | 2 × 2K + 4K trim | 0.9 / 0.4 / 0.15 |

Shared trim sheets: `T_Trim_Steel_01` (4K: channel, angle, rivet row, weld bead, cut edge), `T_Trim_Timber_01` (2K), `T_Decal_Stencils_01` (2K atlas, all text). ORM packs Occlusion, Roughness, Metallic. Nanite is on for Heavy and Rated static meshes; off for geometry collections and anything under Medium.

## Replication classes

| Class | Name | Used by | Behavior |
|---|---|---|---|
| **RC-L** | Light predicted | Light props | Client predicts locally when within 10 m of the local player or on the local player's line. Server authoritative; correction at 10 Hz, snap if error over 50 cm. Sleeps to static after 3 s at rest. |
| **RC-M** | Medium server-sim | Medium props, volatiles | Server-authoritative Chaos body. Replicated at 20 Hz with client extrapolation. Clients run a non-authoritative local sim for contact response only. |
| **RC-H** | Heavy server-sim | Heavy | Server-authoritative. 15 Hz. Sleeps until a line force, blast, or contact wakes it. Wake is replicated as an event so all clients wake the body the same frame. |
| **RC-R** | Rated | Rated loads | As RC-H, plus the authored consequence fires as a server RPC on the 1.0 s groan tell (05 §11). |
| **RC-K** | Kinematic | Haulers, Gantry, Lift, Ring, Breathing hull | Transform replicated by match clock. Deterministic. Nothing moves them (05 §6.7). |
| **RC-GC** | Geometry collection | All destructibles | Server-authoritative fracture. Cluster breaks replicated; `ReplicationAbandonAfterLevel` = 2 so Light fragments go local after the second break. Fragments that inherit Medium or Heavy tier stay replicated as RC-M or RC-H. |

Blast events (volatiles) are server events. Clients play the hiss tell locally at the arm moment and predict the blast VFX; the impulse is applied server-side and replicated as a wake.

## Chaos conventions

- Destructibles are Geometry Collections with cluster levels L0 (whole), L1 (the fragment list in 05 §6.5), L2 (Light debris, 10 s lifetime).
- **Damage threshold** values are written in the project's impulse-calibrated units: **1 engine damage unit = 1 N·s** (**PROPOSED**; calibrated in `L_Calibration` against the 05 §6.5 table). L0 threshold is the 05 fracture threshold. L1 thresholds are per doc.
- Placed objects live in an **anchor field** (`FS_Anchor_<Object>`) until a line force over their break-free threshold (05 §6.2) releases them. The release is the lurch tell (0.2 s, 05 §11).
- Fragments under Light tier use `RemoveOnMaxSleep` at 10 s.
- Damage propagation is off for Heavy objects that must fall as one piece (containers, billet crates), so they take dents (material swap) rather than cluster breaks.

## Impact quick-reference (05 §7)

All docs compute "what it does to a player" from these.

| Event | Stagger | Flip | Stall |
|---|---|---|---|
| Object hits player | 300 N·s | 600 N·s | **1,200 N·s** |
| Player hits static | 5 m/s | 8 m/s | **12 m/s** |
| Player hits kinematic | 4 m/s | 6 m/s | **9 m/s** |
| Crush (pinned) | | | **8 kN for 0.5 s** |

Surface multipliers (05 §7.3) apply to stagger, flip, and Stall thresholds alike: wood ×1.2, scrap heap, laundry, or drip nets ×2.0, water over 1 m ×3.0, Stall Gel pad ×4.0.

Impulse delivered by an object = mass × relative speed. A Medium object is dangerous at walking speed. A Light object is a tell, a tangle, or a trip.

### Line numbers the docs compute against (05 §3.4, v0.2 scale)

Pull tension is in newtons. Hang loads are in kilonewtons. The two differ by about ten times on purpose: the filament is strong, the Spool motor is the limit.

| | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|
| Pull tension T | 400 N | 400 N | 600 N | 800 N (1,200 N overspin) | 1,000 N |
| Jack launch speed V | 18 m/s | 14 m/s | 12 m/s | 14 m/s | 10 m/s |
| Hang load | 4 kN | 4 kN | 12 kN | 8 kN | 10 kN |
| Line length | 50 m | 20 m | 40 m | 30 m | 20 m |
| Foot-hold (braced) | 3 m/s² | 4 m/s² | 5 (9) m/s² | 5 m/s² | 5 (20) m/s² |

Doubler two-line split: 480 N per line, 720 N at overspin.

- **Haul:** force T along the line, ramping over 0.4 s, closing speed capped at 25 m/s. On an object of mass m with the far end static, a = T / m and the speed after distance d is √(2ad). A Scrapper's 400 N on a 45 kg propane cylinder is 8.9 m/s², 19 m/s over a 20 m line, about 850 N·s: a flip, not a Stall. A Breaker's 1,000 N on a 2,000 kg container is 0.5 m/s²: it lurches and falls rather than flies.
- **Jack:** the **Kick** (capacitor discharge) drives the two ends apart to relative speed V within 0.3 s, split by inverse mass; the motor then pushes at T until line length. Separation capped at 30 m/s. Against static or Rated geometry the player gets all of V.
- **Hang:** rigid rod. Hang loads from several lines on one object add (05 §6.2a, §10): two Riggers hang 2,400 kg. Pull forces from several lines add as vectors. **Break-free is tested against active pull only (Haul, Jack, and the Kick).** A Hang line holds without tugging; its static load is carried by the object's placement, so a player can Pendulum or Gallows-anchor from a racked, stacked, bolted, or rated object without unseating it, up to the class Hang load. A Hang to a loose object (break-free 0) brings it along as dead weight.
- **Reel (within Hang, 05 v0.2.2):** holding Haul shortens the rod at 1.0 m/s and holding Jack lengthens it, bearing up to the class Hang load. A winch, not a pull. Loads are lifted into Guillotine position by Reel; the Gallows hoists by Reel; hanging players climb by Reel.
- **Kick interlock (05 v0.2.2):** the Kick never fires when either Dog is in another player's Catch Plate. A Jack against a player is the T push only. A self-dog-to-object Jack Kicks; an object-to-enemy Jack does not.
- **Guillotine step 2 (05 §10 v0.2.2):** recall or cut your own set line; gravity drops the load. A second line only steers it. The load must be within the class Hang load to hang.
- **Kinematics sever any line they pass through** (snap, both Dogs release). Lines attached to the kinematic itself are not severed.
- **Force volumes (05 §6.6 v0.2.2):** act on players and free bodies only: loose objects (break-free 0) and objects held on a player's line (a Hang-held load swings in a gust). Racked, stacked, bolted, rated, and cable-hung map objects are unaffected until freed. Authored as acceleration on a 100 kg reference body; a body of mass m feels a_ref × 100 / m, capped at 2 × a_ref.
- **Rated loads (05 v0.2.2):** a Breaker alone, a Doubler at overspin, or any lines summing to 1,000 N on one axis (a crew).
- **Snap:** a normal cut recoils 150 N·s per 1,000 N into both ends, below stagger. A Doubler overspin snap delivers 1,500 N·s × (F / 1,200 N) to the far end.
- **Scrapper bonus:** Light props, and Medium props under 100 kg, move 30% faster on a Scrapper's line (05 §6.1): 30% on travel and arrival speed, implemented as 1.69× force on the Haul and the post-Kick Jack push, never on the Kick's V. The caps are not raised. A scrapped Climber (90 kg) qualifies.
- **Crush:** 8 kN for 0.5 s is unreachable by any line (05 §7.1). It comes from kinematics and from stacked or falling weight over about 800 kg.
- **Tier** is recomputed whenever an object's mass changes. 50 m/s is a hard velocity cap on everything, falling or launched.

### Break-free (05 §6.2, v0.2 scale)

| Placement | Break-free | Who can start it |
|---|---|---|
| Loose on floor | 0 N | Anyone |
| In a rack or cradle | 200 N | Anyone |
| In a stack (friction) | 500 N | Rigger, Doubler, Breaker. Not Climber or Scrapper (400 N). |
| Bolted or chained (removable) | 800 N | Doubler, Breaker |
| Rated / structural | 1,000 N | Breaker; Doubler at overspin; any lines summing to 1,000 N on one axis |

The docs read the threshold as inclusive, per the "Who can start it" column: a Doubler's 800 N starts a bolted object and a Breaker's 1,000 N starts a rated load. Anchor fields (`FS_Anchor_<Object>`) release at these values.

### Heavy staging (05 §6.1a)

At these tensions a Heavy object resting on a floor does not slide. Anything Heavy that is meant to move sits on rollers, skids, greased runners, or hangs. Prep crews stage loads the way riggers do.

| Heavy placement | Effective friction | Starts moving at |
|---|---|---|
| On rollers or a wheeled cradle | 0.02 | about 0.2 N per kg: a Rigger moves 3,000 kg, a Climber 2,000 kg |
| On skids or greased runners | 0.05 | about 0.5 N per kg: a Breaker moves 2,000 kg, a Rigger 1,200 kg |
| Hanging | 0 (pendulum) | any line deflects it slightly; dangerous swings come from gravity |
| Directly on the floor | 0.3 or more | no line. Fixed scenery that breaks, not a load that moves. |

Authored: the Claim marker on polished runners (0.05, start threshold tested against the vector sum of lines, 05 §9.4); the Haul spool on rollers (0.02 to start, 0.005 rolling; 392 N to start, any class alone, a crew's three lines add as vectors); water tanks fixed, they rupture; Ballast containers ride the magnetic racks' dead rollers, which is why the Breathing slides them. Any stack meant to Avalanche sits on rollers: the Yard's billet and container stacks are on roller pallets (0.02 to start, 0.005 rolling), so the Avalanche drags the bottom crate out. Rated loads: the 1,000 N break-free is a commit trigger, not a physics slide; once qualifying pull holds the load for 0.6 s on the authored axis, the scripted timeline carries the whole load regardless of friction or mass (05 §6.3). Ballast's Haul route is a counterbalanced ratchet rail up the Spine: 200 N pawl drag, 4,000 kg moving mass, any class moves it, lines never lift the spool. Every Heavy doc names its staging per arena.

## Contradiction log

Logged for the bible owners while the object docs were written against 05 v0.1. Status after 05 v0.2 (see `docs/bible/CHANGELOG.md`): **RESOLVED** items are now canon and the object docs follow the new text; **OPEN** items still carry a PROPOSED reading in the docs.

| # | Item | Status | Resolution in v0.2 |
|---|---|---|---|
| 1 | 02 vs 05, Scrapper small-prop bonus (02 named barrels and cylinders; 05 gave the 30% to Light only) | **RESOLVED** | 05 §6.1: Light props and Medium props under 100 kg. A propane cylinder yes, a fuel drum no. |
| 2 | 05 §4.3 vs §5.3, Rigger "off their feet" at 7.7 m/s² against an 8 m/s² foot-hold | **RESOLVED** | Foot-hold is now 3/4/5/5/5 m/s² (Breaker lowered from 7 to 5 in v0.2.1). The Rigger leaves the floor at 0.26 s in the reference case. |
| 3 | 05 §5.4 brace numbers did not make a braced Breaker static | **RESOLVED** | Braced foot-hold 9 m/s² (Rigger), 20 m/s² (Breaker). Tensions rescaled. A braced Breaker is static to every line; a braced Rigger holds a Breaker's line but not a Doubler's full overspin. |
| 4 | 05 §6.4 propane 2 m Stall was an override of the 1,500 N·s falloff | **RESOLVED** | Stated in 05 as an authored result that overrides the falloff. |
| 5 | 03 vs 05, Lift counterweight tier | **RESOLVED** | 05 §6.7 rated attachment: kinematic on its guide until a Breaker's line exceeds 1,000 N, then a simulated rated load. |
| 6 | Scrapped body filed under `heavy/` but Medium per 05 §7.4 | **OPEN (folder only)** | Not a bible issue. The doc states Medium. Move the file if the folder should match tier. |
| 7 | 01 glossary vs 05 §3.1 set-line wording | **CLOSED** | Same meaning, no numeric conflict. |
| 8 | Demolition charge triggered by Dogs at 90 to 160 m/s | **RESOLVED** | 05 §6.4: Dogs seat in a charge without triggering it. Trigger is object or player impact over 6 m/s, or a blast. |
| 9 | "Heavy T/m under 1 g" failed for a 900 kg marker on a 10 kN line | **RESOLVED** | Tensions rescaled. 1,000 N on 900 kg is 1.1 m/s². |
| 10 | No single Hang load holds the 19.6 kN Haul spool | **RESOLVED** | 05 §6.2a: Hang loads add across lines; pull forces add as vectors. Haul is a crew mode. |
| 11 | "Heavy, fixed" manifold had no break-free row | **RESOLVED** | 05 §6.4: fixed world geometry; no line moves it. Vents on a Jack over 600 N or a blast. |
| 12 | Propane 2 to 4 m flip band was also an override | **RESOLVED** | Stated in 05 as an authored result. |
| 13 | Lines on a scrapped body when the Hook lifts it at 4 s | **RESOLVED** | Lines release at the Stall (05 §7.4 step 4); the Hook lifts a bare body. Bodies in the Anchor's Well are not retrieved. |
| 14 | Tier of objects whose mass changes mid-round | **RESOLVED** | 05 §6.3: tier is recomputed whenever mass changes. |
| 15 | Player bodies against fracture thresholds | **RESOLVED** | 05 §6.5: a player's body counts as an impacting object (130 kg Rigger at 5 m/s, 650 N·s, breaks a stall). |
| 16 | Scaffold "Medium boards" vs 12 kg Light planks | **RESOLVED** | 05 §6.5: Light boards, Light clamps, one Medium frame piece. Single boards Light; stacks Medium. |
| 17 | 05 §7.3 multipliers covered Stall only | **RESOLVED** | Multipliers apply to stagger, flip, and Stall. |
| 18 | 03 Ballast containers sliding on a 6° lean against steel friction | **RESOLVED** | 05 §6.1a: containers in Ballast ride the magnetic racks' dead rollers (0.02), which is why the Breathing slides them. |
| 19 | One hanging-house fracture threshold for both sizes | **RESOLVED** | 05 §6.5: small 2,000 N·s, large 3,500 N·s. Wall-piece masses in the docs stay PROPOSED. |
| 20 | Break-free "above" vs "under" at exactly the threshold | **RESOLVED** | 05 §6.2 "Who can start it" column: Doubler 800 N starts a bolted object, Breaker 1,000 N starts a rated load. Docs read the threshold as inclusive. The §6.2 prose still says "above it"; a one-word fix would close it fully. |
| 21 | Kinematic Lift counterweight vs rated load a Breaker can move | **RESOLVED** | 05 §6.7 rated attachment rule. |
| 22 | No terminal velocity for objects | **RESOLVED** | 05 §6.3: 50 m/s for everything. Chaos stability at 2,000 kg × 50 m/s still needs an engine check. |
| 23 | Blasts under water | **RESOLVED** | 05 §6.4: under more than 1 m of water, half impulse and no fire patch. |
| 24 | 03 scrap heaps as ammunition vs ×2.0 safe surface | **RESOLVED** | 05 §9.2: prop pools do not replenish within a round. The 40% surface-removal threshold in `scrap-debris-kit.md` stays PROPOSED. |

### New notes against 05 v0.2

25. **05 §6.4 manifold "over 600 N" vs a Rigger's 600 N.** **RESOLVED (v0.2.1, inclusive thresholds): a Rigger vents it.** Original note: Strictly read, a Rigger's base Jack does not vent the manifold and a Doubler's split line (480 N) does not either. `gas-manifold.md` uses the strict reading, marked PROPOSED. If a Rigger should vent it, the bible should say "600 N or more".
26. **Hung houses cannot be swung lethally by a line at v0.2 tensions.** A hung house is a pendulum: steady deflection is atan(T / mg). A Breaker's 1,000 N holds a 1,200 kg small house 4.9° off plumb; released, it swings back at 0.76 m/s, 912 N·s, a flip, under the 2,000 N·s fracture. **RESOLVED (v0.2.1):** "Lines cannot swing Heavy hanging objects dangerously on their own; house and platform swings come from gravity (cut cable, Cable Day settle, Sway pumping)." The house docs describe exactly those routes; the routes are canon, the specific drop heights and sweep speeds stay PROPOSED.
27. **05 §6.7 "exceeds 1,000 N" vs §6.2 inclusive.** **RESOLVED (v0.2.1):** all thresholds are inclusive (at or above). A Breaker's 1,000 N starts a rated load; a Rigger's 600 N Jack vents the manifold. Item 25 is closed by the same ruling.
28. **Rated loads move slowly after the groan.** At 0.17 to 0.27 m/s² a rated load needs several seconds of Haul after the 1.0 s groan before visible motion (crane top slab about 3.4 s to the tray edge; container about 2.2 s to tip, both PROPOSED). **RESOLVED: intended per the bible owners.**
29. **Guillotine with a Heavy load needs Hang-summing.** A single Rigger's 12 kN Hang holds at most 1,200 kg. **RESOLVED: Guillotine load limits and Hang-summing are now stated in 05 §10.**
30. **Eviction as a Doubler play.** **RESOLVED (v0.2.1):** "Hanging houses attach at a rated 1,000 N; only a Breaker, a Doubler at overspin, or a cut slack cable drops one." The Doubler's base 800 N does not. Both house docs changed from 800 N bolted to 1,000 N rated.
31. **Break-free vs a hanging player's weight.** Break-free (200 to 1,000 N) is below every harness weight (Climber 883 N, Breaker 2,158 N), so a literal reading let a hanging player unseat racked, stacked, bolted, or rated objects. **RESOLVED (05 §6.2): break-free is tested against active pull only (Haul, Jack, and the Kick). A Hang holds without tugging; its static load is carried by the placement, so a player can Pendulum from any placed object up to the class Hang load. A Hang to a loose object brings it along as dead weight.** All Hang rows in the object docs follow this.
32. **05 §3.4 propane example omitted the Scrapper's own bonus.** **RESOLVED:** 05's examples are corrected.
33. **Terminal velocity scope.** **RESOLVED: 50 m/s is a hard velocity cap on everything**, including snap-launched objects.
34. **v0.2.1 scrapped body mass.** "A scrapped body keeps its class mass (90 to 220 kg), tier Medium." `scrapped-body.md` is being updated from the flat 180 kg of v0.1 §7.4; the index row above should read 90 to 220 kg by class.
35. **Gallows from a placed anchor after the Hang ruling.** A Hang never tests break-free, so any class can set the Gallows' Hang phase on a stacked container or a rated counterweight. The rig then switches to Haul (05 §10), which is active pull and does test the anchor's break-free with the same T it applies to the enemy. Climber and Scrapper (400 N) stay under every placed threshold, so the anchor holds and the enemy hoists. A Breaker Gallows off a stacked container (1,000 N against 500 N) starts the container too: it lurches at 0.5 m/s² while the enemy accelerates at T / their mass. **CLOSED: consistent as written; no bible change needed.**
36. **Breaker brace added nothing at v0.2.** **RESOLVED:** Breaker unbraced foot-hold is now 5 m/s² (braced 20 unchanged), so a Doubler's overspin (5.5 m/s²) takes an unbraced Breaker off its feet and Brace matters for Breakers. Docs use 3/4/5/5/5.
37. **Crush is unreachable by lines.** **RESOLVED:** 05 §7.1 now says so; crush comes from kinematics and stacked or falling weight over about 800 kg.
38. **05 §7.1 impulse example exceeded the Haul cap.** **RESOLVED:** 05's examples are corrected.
39. **Scrapper bonus and the Kick.** **RESOLVED (05 §6.1):** the bonus applies to Haul acceleration and the post-Kick Jack push, not to the Kick's V, as the docs implemented.
40. **03 Ballast vertical Haul route.** **RESOLVED:** canon is a counterbalanced ratchet rail up the Spine (200 N pawl drag, 4,000 kg moving mass, any class moves it, lines never lift the spool). The 2.5° ramp and the interim Bays-deck reading are both withdrawn from `haul-spool.md`. On the Yard Floor the spool sits on rollers (0.02) per 05 §6.1a.
41. **Claim marker movable by lines.** **RESOLVED (05 §6.1a):** the marker sits on polished runners at 0.05; a Rigger or heavier moves it, slowly. `claim-marker.md` uses 0.05, not the 0.10 it proposed.
42. **Water tank not movable.** **RESOLVED (05 §6.1a):** water tanks are fixed; they rupture, they do not move. Heavy objects on a bare floor are fixed scenery that breaks.
43. **Scrapper bonus on a scrapped Climber.** **RESOLVED (05 §7.4):** yes, it applies. "Riggers find this funny."
44. **Brief arithmetic, corrected in the docs.** A Breaker's 1,000 N on a 45 kg cylinder (22.2 m/s²) reaches 21 m/s after 10 m and the 25 m/s cap at 14 m, not "21 m/s over 20 m". `propane-cylinder.md` carries the correct figures. 05 v0.2 does not contain the wrong figure.
45. **05 §6.1a roller coefficient.** **RESOLVED:** rollers are 0.02 to start and 0.005 rolling, as `haul-spool.md` implemented.
46. **Marker start force vs vector sum.** **RESOLVED (05 §9.4):** the start threshold is tested against the vector sum of lines.
47. **Ballast Haul progress is un-losable.** **RESOLVED:** intended; no tiebreak change.
48. **Scrapper bonus: acceleration or speed?** **RESOLVED:** 30% on travel and arrival speed, implemented as 1.69× force on qualifying props, which is how the docs apply it.
49. **Overboard against a Breaker by a light class.** **RESOLVED:** nearly inert is intended.
50. **Mass-scaled force volumes and Light props.** **RESOLVED:** force-volume acceleration is capped at 2 × a_ref per body. Docs cap every factor at 2×.
51. **03 vs 05 Pulse period.** **CLOSED:** 03 already says 75 s from 0:55.
52. **Scrapped body plates and the Kick interlock.** **RESOLVED:** yes, a scrapped body's plates trigger the interlock.
53. **Rated counterweight vs the roller rule.** **RESOLVED (05 §6.3):** the rated 1,000 N break-free is a commit trigger; after 0.6 s of qualifying pull on the authored axis the scripted timeline carries the whole load regardless of friction or mass. `crane-counterweight.md` moves the whole stack; the top-three-slabs reading is withdrawn.
54. **Skid-staged Heavy is tipped, not dragged.** **RESOLVED (05 §6.1a):** any stack meant to Avalanche sits on rollers; the Yard's billet stacks are on roller pallets (0.02 start, 0.005 rolling) and the Avalanche drags the bottom crate out. `billet-crate.md` and `shipping-container.md` updated.
55. **2× cap flattens Light vs small Medium in force volumes.** Under a_ref × min(100 / m, 2) every body under 50 kg feels the same 2 × a_ref, so the manifold jet, the Updraft, and the flood no longer sort a 6 kg chair from a 45 kg cylinder by speed. A 3× cap (ceiling at 33 kg) would restore the sort without changing any Medium figure. Observation, not a contradiction; docs follow the 2× cap.
