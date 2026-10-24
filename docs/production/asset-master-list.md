# KINETIC — Master Asset and Document List

_Generated 2026-10-24 from docs/. Regenerate with python3 docs/production/generate-asset-list.py after any change. Every row is one file; every object, character, and arena has exactly one._

## Totals

| Section | Files | Words |
|---|---:|---:|
| Bible (canonical world, factions, arenas, progression, mechanics) | 7 | 19,720 |
| Harness classes | 5 | 7,624 |
| Faces (named characters) | 10 | 18,089 |
| Equipment | 6 | 8,678 |
| NPCs | 5 | 4,254 |
| Objects: Light props | 11 | 23,319 |
| Objects: Medium props | 10 | 21,277 |
| Objects: Volatiles | 5 | 12,864 |
| Objects: Heavy | 10 | 25,640 |
| Objects: Structures | 11 | 15,029 |
| Objects: Rated loads | 6 | 10,277 |
| Objects: Kinematics | 6 | 11,059 |
| Objects: Environment and world systems | 14 | 20,040 |
| Arena: Ballast | 6 | 19,705 |
| Arena: The Yard | 6 | 18,480 |
| Arena: Hangtown | 6 | 21,427 |
| Arena: The Anchor | 6 | 19,137 |
| Systems (UI, audio, VFX, animation, camera, modes, onboarding) | 10 | 30,455 |
| Technical architecture (UE5) | 12 | 38,659 |
| Production | 7 | 10,467 |
| **Total (with folder indexes)** | **165** | **377,669** |

## Bible (canonical world, factions, arenas, progression, mechanics)

_7 files, about 19,720 words._

| File | Title | Summary |
|---|---|---|
| [00-design-brief.md](bible/00-design-brief.md) | KINETIC — Design Brief | - Genre: Multiplayer physics-trap arena (FPS / third-person) |
| [01-world-and-lore.md](bible/01-world-and-lore.md) | KINETIC — World & Lore | Sixty-one years ago the only space elevator ever built snapped and fell across the equator. The people who take the wreck apart call themselves riggers. When tw… |
| [02-factions-and-archetypes.md](bible/02-factions-and-archetypes.md) | KINETIC — Factions & Archetypes | - A faction is who you fight for. A Harness is how you fight. Each faction fields one signature Harness and two Faces: named riggers who front the faction on th… |
| [03-arenas.md](bible/03-arenas.md) | KINETIC — Arenas | These come straight out of the lore and should be treated as hard constraints for level design. |
| [04-progression.md](bible/04-progression.md) | KINETIC — Progression | Everything a player earns is access, identity, or expression. Rated power never moves. A Greenhand's Spool pulls exactly as hard as a Wright's. The lore support… |
| [05-core-mechanics.md](bible/05-core-mechanics.md) | KINETIC — Core Mechanics Specification | 1. Everything is physics. No hitscan, no damage numbers, no health. If a thing hurts a player, it is because mass met velocity. |
| [CHANGELOG.md](bible/CHANGELOG.md) | Bible changelog | Every document written before this change must be updated to these values. |

## Harness classes

_5 files, about 7,624 words._

| File | Title | Summary |
|---|---|---|
| [breaker.md](characters/harnesses/breaker.md) | Breaker Harness | - Summary: Halvard-Teague Reclamation's harness. The heaviest body in sanctioned play. Removes the room instead of the player. |
| [climber.md](characters/harnesses/climber.md) | Climber Harness | - Summary: The Ascendancy's harness. The lightest body in the game and the longest line. Never where the floor is. |
| [doubler.md](characters/harnesses/doubler.md) | Doubler Harness | - Summary: The Fray's harness. Two active lines and a tension override that hurts the wearer on a timer everyone can hear. |
| [rigger.md](characters/harnesses/rigger.md) | Rigger Harness | - Summary: The Lay Guild's harness. The baseline everyone else is tuned against. Sets the room, then stands in the doorway. |
| [scrapper.md](characters/harnesses/scrapper.md) | Scrapper Harness | - Summary: The Hangtown Crews' harness, built from four other harnesses. Fastest Dogs, fastest Cutter, instant recall. Chains small traps in the open. |

## Faces (named characters)

_10 files, about 18,089 words._

| File | Title | Summary |
|---|---|---|
| [baskoro-hanif.md](characters/faces/baskoro-hanif.md) | Baskoro Hanif "Hitch" | - Faction: Lay Guild |
| [dela-santos.md](characters/faces/dela-santos.md) | Dela Santos "Kite" | - Faction: Hangtown Crews (Hangers) |
| [jesus-vilca.md](characters/faces/jesus-vilca.md) | Jesús Vilca "Fuse" | - Faction: Hangtown Crews (Hangers) |
| [mei-lin-haldane.md](characters/faces/mei-lin-haldane.md) | Mei-Lin Haldane "the Surveyor" | - Faction: Halvard-Teague Reclamation |
| [nnamdi-ferreira.md](characters/faces/nnamdi-ferreira.md) | Nnamdi Ferreira "Snap" | - Faction: The Fray |
| [odile-lacroix.md](characters/faces/odile-lacroix.md) | Odile Lacroix "the Auctioneer" | - Faction: The Fray |
| [paloma-reyes-okafor.md](characters/faces/paloma-reyes-okafor.md) | Paloma Reyes-Okafor "Swift" | - Faction: The Ascendancy |
| [rook-adeyemi.md](characters/faces/rook-adeyemi.md) | Rook Adeyemi "Wreckingball" | - Faction: Halvard-Teague Reclamation |
| [tendai-mutasa.md](characters/faces/tendai-mutasa.md) | Tendai Mutasa "the Verger" | - Faction: The Ascendancy |
| [teodora-quispe.md](characters/faces/teodora-quispe.md) | Teodora Quispe "Mother Line" | - Faction: Lay Guild |

## Equipment

_6 files, about 8,678 words._

| File | Title | Summary |
|---|---|---|
| [catch-plates.md](characters/equipment/catch-plates.md) | Catch Plates | Rigid plates mounted to the harness over the chest, between the shoulder blades, and on each shoulder. A Dog seats in whichever plate is nearest its impact poin… |
| [cutter.md](characters/equipment/cutter.md) | The Cutter | A fixed blade with a ceramic edge, a heating element inside the blade spine, a battery and switch in the handle, and a serial number engraved on the ricasso. Th… |
| [dogs.md](characters/equipment/dogs.md) | Dogs | A Dog is a projectile with gravity, 0.08 m collision radius (05 §3.1), fired from the Spool at class speed. In the hand it is a grip the size of a fist: a body,… |
| [harness-shared-components.md](characters/equipment/harness-shared-components.md) | Harness Shared Components | Fiction. When telemetry reads an impact that would have been lethal, the gel floods, the harness locks, and the rigger is scrapped. Stall-Gel is a Nurri product… |
| [line.md](characters/equipment/line.md) | The Line | A 6 mm filament (PROPOSED fiction diameter) rendered as a ribbon with a 1.2 cm bright core and a glow halo whose width scales with force. It runs from S_Spool_L… |
| [spool.md](characters/equipment/spool.md) | The Spool | A drum housing clamped to the right forearm. Inside, a drum holds the live filament. Outside, two Dogs sit in cradles on the housing's outer face, and a line ex… |

## NPCs

_5 files, about 4,254 words._

| File | Title | Summary |
|---|---|---|
| [bot-crew.md](characters/npcs/bot-crew.md) | Bot Crew (Tutorial) | HTR Probationers: Grade 1 on HTR's ladder (04). Bulk-leased Breakers with sequential Kessl serials, numbered not named, which is how HTR treats its reserves and… |
| [hook-drone.md](characters/npcs/hook-drone.md) | Hook Drone | Hooks are seen from above, briefly, and heard constantly. The silhouette is the hex hull and the dangling arm. |
| [marshal.md](characters/npcs/marshal.md) | Marshal | Three Marshals per match in the gallery; one is the calling Marshal and the voice. Office uniform, not harness: they are never on the floor. |
| [prep-crew.md](characters/npcs/prep-crew.md) | Prep Crew | During the Drop phase and between rounds they finish the arena the way it was found: |
| [reserve-rigger.md](characters/npcs/reserve-rigger.md) | Reserve Rigger | A reserve rigger is the player's character after a respawn in Claim (8 s timer, 3 reserves per crew per match), Haul (8 s, unlimited), or Free Scrap (10 s, unli… |

## Objects: Light props

_11 files, about 23,319 words._

| File | Title | Summary |
|---|---|---|
| [bucket.md](objects/props-light/bucket.md) | Bucket | Light prop. Light tier, 10 kg full of water, 2 kg empty. 30 cm top diameter, 26 cm base, 36 cm tall, 15 L HDPE bucket with a wire bail and plastic grip. Appears… |
| [cable-reel-small.md](objects/props-light/cable-reel-small.md) | Cable reel (small) | Light prop. Light tier, 28 kg. 60 cm flange diameter, 40 cm wide, 20 cm core, plywood flanges on a steel axle, half-wound with 12 mm steel wire rope. Appears in… |
| [chain-coil.md](objects/props-light/chain-coil.md) | Chain coil | Light prop. Light tier, 25 kg. Coiled 40 cm diameter × 18 cm tall; 6 m of 10 mm grade-80 lifting chain with a hook each end. Appears in the Yard (the Floor, bes… |
| [clamp-crate.md](objects/props-light/clamp-crate.md) | Clamp crate | Light prop. Light tier, 28 kg. 60 × 40 × 30 cm open-top steel mesh crate holding about 20 scaffold couplers. Appears in the Anchor (the Scaffold, two per level … |
| [floodlight-stand.md](objects/props-light/floodlight-stand.md) | Floodlight stand | Light prop. Light tier, 22 kg. 90 cm tripod footprint, mast 240 cm tall, twin 30 × 20 cm LED heads on a crossbar, cable to a floor socket. Appears in the Yard (… |
| [folding-chair.md](objects/props-light/folding-chair.md) | Folding chair | Light prop. Light tier, 6 kg. Folded 46 × 6 × 84 cm; open 46 × 50 × 80 cm, seat at 44 cm. Appears in Hangtown (the Mid Hang market platform, house porches, the … |
| [laundry-bundle.md](objects/props-light/laundry-bundle.md) | Laundry bundle | Light prop. Light tier, 8 kg. 50 × 40 × 30 cm knotted cloth bundle, or 180 × 120 cm hung on a line in the Hung variant. Appears in Hangtown only: the Low Hang (… |
| [paint-can.md](objects/props-light/paint-can.md) | Paint can | Light prop. Light tier, 5 kg full, 1.2 kg empty. 18 cm diameter × 20 cm tall, 4 L steel can with a wire bail. Appears in Hangtown (the Mid Hang, on porches and … |
| [plank.md](objects/props-light/plank.md) | Plank | Light prop. Light tier, 12 kg. 240 × 23 × 4 cm sawn softwood scaffold board. Appears in the Anchor (the Scaffold, as every walking surface, and as fragments aft… |
| [scrap-debris-kit.md](objects/props-light/scrap-debris-kit.md) | Scrap debris kit | Light prop kit. Light tier, 1 to 9 kg per piece, twelve pieces; a heap is 40 to 80 pieces in a 3 to 5 m mound 60 to 120 cm tall. Appears in the Yard (the Floor,… |
| [tool-bag.md](objects/props-light/tool-bag.md) | Tool bag | Light prop. Light tier, 20 kg. 55 × 30 × 32 cm open-top canvas rigger's bag with two steel-reinforced handles. Appears in the Yard (the Floor, at the base of ev… |

## Objects: Medium props

_10 files, about 21,277 words._

| File | Title | Summary |
|---|---|---|
| [brick-pallet.md](objects/props-medium/brick-pallet.md) | Brick pallet | Medium prop. Mass tier Medium, 290 kg. 120 × 80 × 70 cm (a 120 × 80 × 14 cm timber pallet carrying 120 clay bricks at 2.3 kg each in six courses, shrink-wrapped… |
| [cart-trolley.md](objects/props-medium/cart-trolley.md) | Cart trolley | Medium prop. Mass tier Medium, 120 kg. 150 × 80 × 100 cm (L × W × H to the push bar). Appears in the Yard (the Floor, beside scrap heaps and at the press apron)… |
| [hand-truck.md](objects/props-medium/hand-truck.md) | Hand truck | Medium prop. Mass tier Medium, 35 kg. 50 × 60 × 125 cm (W × D × H; a two-wheel steel dolly with a 35 × 20 cm toe plate, a curved back frame, and two pneumatic t… |
| [market-stall.md](objects/props-medium/market-stall.md) | Market stall | Medium prop. Mass tier Medium, 90 kg. 240 × 120 × 220 cm (L × W × H to the awning ridge). Appears in Hangtown (the Mid Hang, twelve on the market ring, four on … |
| [rebar-bundle.md](objects/props-medium/rebar-bundle.md) | Rebar bundle | Medium prop. Mass tier Medium, 280 kg. 600 × 18 × 12 cm (30 bars of 16 mm deformed rebar, 6 m long, tied in a rough hexagonal bundle with four wire ties). Appea… |
| [scaffold-board-stack.md](objects/props-medium/scaffold-board-stack.md) | Scaffold board stack | Medium prop. Mass tier Medium, 160 kg strapped. 390 × 45 × 23 cm (12 boards of 390 × 22.5 × 3.8 cm, two wide, six high, on two bearers, with two steel straps). … |
| [stall-gel-pad.md](objects/props-medium/stall-gel-pad.md) | Stall Gel pad | Medium prop, prep-crew placed, rare. Mass tier Medium, 40 kg. 300 × 200 × 20 cm (a quilted gel mattress in a ripstop shell with a tubular frame edge and four co… |
| [steel-crate.md](objects/props-medium/steel-crate.md) | Steel crate | Medium prop. Mass tier Medium, 250 kg. 120 × 100 × 90 cm (L × W × H). Appears in the Yard (the Stacks, stacked three high beside billet containers; the Floor, l… |
| [water-barrel.md](objects/props-medium/water-barrel.md) | Water barrel | Medium prop. Mass tier Medium full at 220 kg; Light empty at 25 kg. 58 cm diameter × 94 cm tall (195 L blue HDPE drum with a screw bung and a lifting rim). Appe… |
| [welding-cart.md](objects/props-medium/welding-cart.md) | Welding cart | Medium prop carrying two volatiles. Mass tier Medium, 140 kg total (50 kg cart, two 45 kg gas cylinders). 110 × 60 × 150 cm (L × W × H to the cylinder valves). … |

## Objects: Volatiles

_5 files, about 12,864 words._

| File | Title | Summary |
|---|---|---|
| [cylinder-rack.md](objects/volatiles/cylinder-rack.md) | Cylinder rack | Volatile. Medium tier empty at 95 kg; Heavy tier full at 365 kg (rack plus six 45 kg propane cylinders). Tier is recomputed whenever the mass changes (05 §6.3).… |
| [demolition-charge.md](objects/volatiles/demolition-charge.md) | Demolition charge | Volatile. Light tier, 12 kg. 40 × 25 × 18 cm, a shaped linear charge in a steel carrier bolted to structure. Placed by prep crews only; never loose at round sta… |
| [fuel-drum.md](objects/volatiles/fuel-drum.md) | Fuel drum | Volatile. Medium tier, 180 kg full. 58 cm diameter, 88 cm tall (a 200 L closed-head drum). Appears in Ballast, the Bays (generator fuel at the catwalk landings)… |
| [gas-manifold.md](objects/volatiles/gas-manifold.md) | Compressed-gas manifold | Volatile. Fixed world geometry; it cannot be moved by any line (05 §6.4). 180 × 60 × 160 cm (PROPOSED dimensions): a steel frame bolted to the deck carrying fou… |
| [propane-cylinder.md](objects/volatiles/propane-cylinder.md) | Propane cylinder | Volatile. Medium tier, 45 kg. 30 cm diameter, 120 cm tall including the valve guard. Appears in every arena: Ballast, the Bays (welding stations on the catwalks… |

## Objects: Heavy

_10 files, about 25,640 words._

| File | Title | Summary |
|---|---|---|
| [billet-crate.md](objects/heavy/billet-crate.md) | Billet crate | Heavy prop. Heavy tier, 1,800 kg (five Weave billets at 300 kg in a 300 kg steel cage, PROPOSED split). 240 × 120 × 120 cm (PROPOSED). Appears in the Yard (the … |
| [claim-marker.md](objects/heavy/claim-marker.md) | Claim marker | Heavy. Heavy tier, 900 kg: a Meridian Weave billet strapped to a steel pallet, with an Office beacon post. Pallet 120 × 100 × 15 cm, billet 100 × 60 × 40 cm, be… |
| [crane-counterweight.md](objects/heavy/crane-counterweight.md) | Crane counterweight | Rated load. Rated tier (over 3,000 kg), 6,000 kg (PROPOSED): five cast slabs of 1,200 kg on a steel tray. Stack 200 × 100 × 125 cm, tray 220 × 120 × 15 cm (PROP… |
| [hanging-house-large.md](objects/heavy/hanging-house-large.md) | Hanging house (large) | Heavy destructible. Heavy tier, 2,800 kg: any harness can deflect it a few degrees and pump it with the Sway, but no line swings it dangerously on its own (05 v… |
| [hanging-house-small.md](objects/heavy/hanging-house-small.md) | Hanging house (small) | Heavy destructible. Heavy tier, 1,200 kg. 300 × 250 × 260 cm (PROPOSED), one room, hung from a single cable through a roof beam at a rated 1,000 N attachment (0… |
| [haul-spool.md](objects/heavy/haul-spool.md) | Haul spool | Heavy. Heavy tier, 2,000 kg: a steel cable spool wound with recovered Meridian Weave ribbon. 160 cm flange diameter, 120 cm wide, 60 cm core, with 10 cm axle st… |
| [lift-counterweight.md](objects/heavy/lift-counterweight.md) | Lift counterweight | Rated load. Rated tier (over 3,000 kg), 4,500 kg (PROPOSED). Concrete block 260 × 120 × 60 cm in a steel guide frame 320 × 160 × 110 cm overall (PROPOSED); the … |
| [scrapped-body.md](objects/heavy/scrapped-body.md) | Scrapped body | Prop state of an eliminated player. Medium tier for every class; mass is the class mass: Climber 90 kg, Scrapper 110 kg, Rigger 130 kg, Doubler 130 kg, Breaker … |
| [shipping-container.md](objects/heavy/shipping-container.md) | Shipping container (20 ft) | Heavy prop. Heavy tier, 2,000 kg. 606 × 244 × 259 cm (ISO 20 ft). Appears in Ballast (the Bays, stacked on the dead magnetic racks either side of the Spine; loo… |
| [water-tank.md](objects/heavy/water-tank.md) | Water tank | Heavy. Heavy tier, 2,500 kg full (PROPOSED 240 kg steel shell plus 2,260 L of water). 130 cm diameter, 170 cm tall, a vertical cylindrical steel tank on a welde… |

## Objects: Structures

_11 files, about 15,029 words._

| File | Title | Summary |
|---|---|---|
| [billet-stack.md](objects/structures/billet-stack.md) | Billet Stack | structure, Heavy tier per unit, 2,800 kg per billet crate (2.4 m x 1.2 m x 1.2 m), stacked three or four high on pallets, the Yard Stacks tier: nine stacks alon… |
| [catwalk-section.md](objects/structures/catwalk-section.md) | Catwalk Section | structure, destructible, Heavy tier, 360 kg, 4.0 m long x 1.2 m wide x 1.1 m handrail height, Ballast: the port-starboard cross-catwalk (4 m wide) through the M… |
| [collar-rim-ground.md](objects/structures/collar-rim-ground.md) | Collar Rim Ground | structure, static world, infinite mass, two slabs each 60 m x 30 m (PROPOSED playable footprint) at the east and west ends of Hangtown: the sheared edges of Col… |
| [container-stack.md](objects/structures/container-stack.md) | Container Stack | structure, Heavy tier per unit, 2,000 kg per container (6.06 m x 2.44 m x 2.59 m, ISO 20 ft pattern), stacks of two to four, Ballast Bays on magnetic racks, por… |
| [hull-plate-section.md](objects/structures/hull-plate-section.md) | Hull Plate Section | structure, load-bearing and Rated tier (structural, 7,500 kg), 8.0 m x 4.0 m x 0.6 m including ribs, the Yard: stacked three high to form the Stacks' mesas and … |
| [market-platform.md](objects/structures/market-platform.md) | Market Platform | structure, cable-hung ring with destructible stalls, Heavy tier as an assembly (each of six deck segments 2,400 kg; stalls Medium), 18 m outer diameter, 4 m wid… |
| [plank-bridge.md](objects/structures/plank-bridge.md) | Plank Bridge | structure, destructible, Medium tier, 60 kg, 6.5 m span x 0.9 m wide, three 50 mm planks on two steel bearers, the Yard Floor: crossings over the three 6 m rail… |
| [scaffold-section.md](objects/structures/scaffold-section.md) | Scaffold Section | structure, destructible, Medium tier, 260 kg per bay, 2.0 m x 2.0 m plan x 8.0 m lift with intermediate ledgers at 2 m, the Anchor only: five ring decks of cont… |
| [spine-ladder.md](objects/structures/spine-ladder.md) | Spine Ladder | structure, static world with destructible cage panels, infinite mass (ladder) and Light (cage panels, 14 kg), rungs 0.3 m apart on a 0.5 m wide stile, in 6 m ru… |
| [swing-bridge.md](objects/structures/swing-bridge.md) | Swing Bridge | structure, destructible and cable-hung, Heavy tier, 420 kg, 12 m span x 1.2 m wide, deck 0.9 m below its two suspension cables, Hangtown Mid Hang: eleven bridge… |
| [walkway-on-cable.md](objects/structures/walkway-on-cable.md) | Walkway on Cable | structure, destructible in 2 m sections, Heavy tier per 10 m run (640 kg), sections 2.0 m x 1.0 m, hung 1.6 m below a main load cable on steel hangers, Hangtown… |

## Objects: Rated loads

_6 files, about 10,277 words._

| File | Title | Summary |
|---|---|---|
| [anchor-pin.md](objects/rated/anchor-pin.md) | Anchor Pin | rated load, structural, 4,000 kg, a 0.5 m diameter x 3.2 m steel pin through a cast anchor head, 2.0 m x 1.6 m x 1.4 m above the deck, Hangtown Collar rims: eig… |
| [drive-dog-piston.md](objects/rated/drive-dog-piston.md) | Drive Dog Piston | rated load, 40,000 kg, piston 6.0 m long x 1.6 m diameter seated in a 7.5 m housing, Ballast Spine: six housings on the shaft walls, with ledges at Z 10 (drive … |
| [magnetic-rack.md](objects/rated/magnetic-rack.md) | Magnetic Rack | rated load, structural, 5,500 kg frame (carrying up to three 2,000 kg containers), 6.4 m long x 2.6 m deep x 8.2 m tall, Ballast Bays: fourteen racks bolted to … |
| [maintenance-clamp.md](objects/rated/maintenance-clamp.md) | Maintenance Clamp | rated load, structural, 3,500 kg, a 2.4 m wide x 1.8 m tall x 1.2 m deep hydraulic clamp jaw seated on the Stump's Weave face, the Anchor: twenty clamps on the … |
| [press-counterweight.md](objects/rated/press-counterweight.md) | Press Counterweight | rated load, 9,000 kg steel block, 2.0 m x 1.2 m x 1.6 m, riding in a 14 m guide tower on the east side of the Shear Press pit, the Yard: one, live every round. |
| [structural-column.md](objects/rated/structural-column.md) | Structural Column | rated load, structural, 4,200 kg, 0.6 m x 0.6 m box section x 10.5 m tall, the Yard: the two columns under the control shack at (-62, +35) and (-62, +41) on the… |

## Objects: Kinematics

_6 files, about 11,059 words._

| File | Title | Summary |
|---|---|---|
| [breathing-hull.md](objects/kinematics/breathing-hull.md) | Breathing Hull | kinematic system, infinite mass, the whole 410 m freight climber _Tenacity_ rocking as one body, playable interior per arenas/ballast/layout.md (Hold floor Z 0,… |
| [gantry-crane.md](objects/kinematics/gantry-crane.md) | Gantry Crane | kinematic, infinite mass (crane), 60 m span bridge (Y -30 to +30) on two portal legs, rails 36 m above the Floor (bridge top walkable, 1.5 m wide; cab at the no… |
| [maintenance-ring.md](objects/kinematics/maintenance-ring.md) | Maintenance Ring | kinematic, infinite mass, a ring gantry at Z 42 on the Rim datum spanning r 14 to 20 (6 m wide steel grating, 1.2 m rails inside and out, two 3 m rail gaps as b… |
| [rail-hauler.md](objects/kinematics/rail-hauler.md) | Rail Hauler | kinematic, infinite mass, 14.0 m long x 3.2 m wide, deck at Z -0.4 (3.6 m above the trench's rail bed at Z -4), carrying a hull plate whose top rides at Z +3 ab… |
| [shear-press.md](objects/kinematics/shear-press.md) | Shear Press | kinematic, infinite mass, a 14 m x 14 m pit (X +38 to +52, Y -7 to +7) with sheer steel walls to Z -10 and a 10 m x 6 m x 2.5 m hydraulic ram cycling in it on a… |
| [the-lift.md](objects/kinematics/the-lift.md) | The Lift | kinematic, infinite mass (car, rope, headframe) with a rated attachment (the counterweight, 8,000 kg, 2.2 m x 1.5 m x 1.0 m concrete), car 3 m x 3 m cage at (30… |

## Objects: Environment and world systems

_14 files, about 20,040 words._

| File | Title | Summary |
|---|---|---|
| [broadcast-tower.md](objects/environment/broadcast-tower.md) | Broadcast Tower | environment, destructible, Heavy head (1,800 kg) on a static mast, 24 m lattice tower with a LayCast dish, a camera cluster, and a 6-lamp flood ring at 18 m, th… |
| [cable-anchor.md](objects/environment/cable-anchor.md) | Cable Anchor | environment, static world, infinite mass, the family of hardware where a cable meets a structure: hanger saddles (0.4 m), house hangers (0.8 m), headframe and g… |
| [chalk-and-graffiti.md](objects/environment/chalk-and-graffiti.md) | Chalk and Graffiti | environment, decal system, no mass, marks from 50 mm tallies to 6 m murals, all arenas: Hangtown densest (residents), Ballast and the Yard (Guild and HTR workin… |
| [condemnation-notice.md](objects/environment/condemnation-notice.md) | Condemnation Notice | environment, dressing, Light (0.05 kg paper on a 0.3 kg board where boarded), A4 (210 mm x 297 mm) Office notice or 400 mm x 600 mm HTR board, Hangtown (stapled… |
| [evidence-objects.md](objects/environment/evidence-objects.md) | Evidence Objects | environment, six Light props (2 to 14 kg), one per planned season (Claims 62 to 67), each a physical piece of the Question (01) recoverable by any player from a… |
| [floodlight-mast.md](objects/environment/floodlight-mast.md) | Floodlight Mast | environment, static world with a destructible head, Heavy head (640 kg) on an infinite-mass mast, 30 m lattice mast (Hangtown rims, 4 per rim), 24 m dock masts … |
| [force-volumes.md](objects/environment/force-volumes.md) | Force Volumes | environment, authored acceleration fields with no mass, five systems: the Updraft (the Anchor), the Pulse (the Anchor), water rush (Ballast tank ruptures), the … |
| [load-cable.md](objects/environment/load-cable.md) | Load Cable | environment, static world (infinite mass) except during authored failures, 90 mm galvanized wire rope, up to 400 m span, Hangtown: eight main load cables betwee… |
| [office-gallery.md](objects/environment/office-gallery.md) | Office Gallery | environment, static world, infinite mass, no Dog seating, a 12 m x 4 m x 3 m glazed Marshal box with an open camera deck on top, mounted high: Ballast on HTR's … |
| [slack-cable.md](objects/environment/slack-cable.md) | Slack Cable | environment, cuttable constraint, 24 mm wire rope or 30 mm polyester rope, 2 to 12 m lengths, orange tape at both ends, Hangtown only: every hanging house has t… |
| [sponsor-banners.md](objects/environment/sponsor-banners.md) | Sponsor Banners | environment, dressing with Light cloth and Medium frames, cloth 1 to 4 kg, frames 40 to 120 kg, formats 3 m x 1 m (rail banner), 6 m x 2 m (container face), 12 … |
| [structural-tape.md](objects/environment/structural-tape.md) | Structural Tape | environment, marking system, no mass, two tapes: Office rated-load tape (100 mm, yellow and black chevrons at 45 degrees) and Office slack-cable tape (50 mm, or… |
| [weave-ribbon-fragment.md](objects/environment/weave-ribbon-fragment.md) | Weave Ribbon Fragment | environment, Light prop, 0.4 to 8 kg, strips 0.3 m to 2.0 m long x 50 to 200 mm wide x 2 mm thick, all arenas: embedded in hull plates and the Collar rim shear … |
| [well-kill-volume.md](objects/environment/well-kill-volume.md) | Well Kill Volume | environment, kill volume, no mass, one per arena at the arena's floor (05 section 7.5): Ballast's gorge beyond the torn nose cone, the Yard's Shear Press pit fl… |

## Arena: Ballast

_6 files, about 19,705 words._

| File | Title | Summary |
|---|---|---|
| [events-and-moving-parts.md](arenas/ballast/events-and-moving-parts.md) | BALLAST: Events and Moving Parts | The hull (docs/objects/kinematics/breathing-hull.md) is a single kinematic root. Its pitch θ follows θ(t) = 70 + 6 sin(2π t / 90) degrees, t in seconds from the… |
| [layout.md](arenas/ballast/layout.md) | BALLAST: Layout | - Origin: the Spine's centerline where it meets the Hold floor plane. Z = 0 is the Hold floor, the lowest playable floor. |
| [lighting-and-mood.md](arenas/ballast/lighting-and-mood.md) | BALLAST: Lighting and Mood | - Key. The sun, entering over the Crown Lip and through the roofless dorsal half of the Crown. It lights the Crown deck's grating, the gantry beam's underside b… |
| [overview.md](arenas/ballast/overview.md) | BALLAST: Overview | The freight climber _Tenacity_ was 410 m long and 200 km up when the Strand parted. It came down nose-first into the Rucu Gorge, in the Cordillera foothills bel… |
| [prop-manifest.md](arenas/ballast/prop-manifest.md) | BALLAST: Prop Manifest | Placement states follow 05 section 6.2: loose 0 N, racked 200 N, stacked 500 N, bolted or chained 800 N, rated 1,000 N. "Bolted" objects are hull fittings a lin… |
| [signature-plays.md](arenas/ballast/signature-plays.md) | BALLAST: Signature Plays | - Where. Either Lower Bay. Puller on the cross-catwalk at the dorsal side of the Belly rack (Y -4). Target on the Belly strip of the Lower deck (Y +5 to +11) or… |

## Arena: The Yard

_6 files, about 18,480 words._

| File | Title | Summary |
|---|---|---|
| [events-and-moving-parts.md](arenas/the-yard/events-and-moving-parts.md) | THE YARD: Events and Moving Parts | Time notation: e is seconds elapsed since the Marshal calls "live." c is the Gantry's cycle time (0 to 120). Zone names are from layout.md. |
| [layout.md](arenas/the-yard/layout.md) | THE YARD: Layout | - Origin (0, 0, 0): center of the center trench's Floor-level span, 45 m west of the press pit center. X positive east (along the gantry rails). Y positive nort… |
| [lighting-and-mood.md](arenas/the-yard/lighting-and-mood.md) | THE YARD: Lighting and Mood | - Time: night shift, 02:40 local. No moon. |
| [overview.md](arenas/the-yard/overview.md) | THE YARD: Overview | The Collar Four Breaking Yard sits in the Basin, on the flat east of the Cordillera where the river slowed the wreck down. Hull sections from across the Lay com… |
| [prop-manifest.md](arenas/the-yard/prop-manifest.md) | THE YARD: Prop Manifest | Coordinates are (X, Y) at Floor datum unless a Z is given. Zone names match layout.md. Object paths point at the per-object docs; this file does not restate the… |
| [signature-plays.md](arenas/the-yard/signature-plays.md) | THE YARD: Signature Plays | Zone names and coordinates are from layout.md. Timetable values are from events-and-moving-parts.md: South trains nose-in at e 10 + 45k, Center at e 25 + 45k (f… |

## Arena: Hangtown

_6 files, about 21,427 words._

| File | Title | Summary |
|---|---|---|
| [events-and-moving-parts.md](arenas/hangtown/events-and-moving-parts.md) | HANGTOWN: Events and Moving Parts | The whole district moves with the wind. In engine this is a low-amplitude kinematic animation of the cable anchor points at the Hub and along each main cable, n… |
| [layout.md](arenas/hangtown/layout.md) | HANGTOWN: Layout | - Origin: the vertical axis of the Market Ring. X positive east (toward the East Rim), Y positive north (along the ravine), Z up. |
| [lighting-and-mood.md](arenas/hangtown/lighting-and-mood.md) | HANGTOWN: Lighting and Mood | - Time: blue hour, 25 minutes after sunset. The sun is below the western Collar half. The sky holds light; the ravine does not. |
| [overview.md](arenas/hangtown/overview.md) | HANGTOWN: Overview | Collar Three split when it landed. The two halves sit 400 m apart across a ravine in the Cordillera. During the Ribbon Wars people with nowhere else to go strun… |
| [prop-manifest.md](arenas/hangtown/prop-manifest.md) | HANGTOWN: Prop Manifest | Two hands placed everything here. The residents left their homes as they were when the Office evacuated them at 06:00 on match day: stalls, laundry, cooking gas… |
| [signature-plays.md](arenas/hangtown/signature-plays.md) | HANGTOWN: Signature Plays | Conventions. Class masses 90 / 110 / 130 / 130 / 220 kg (Climber / Scrapper / Rigger / Doubler / Breaker). Pull tension T 400 / 400 / 600 / 800 (1,200 overspin)… |

## Arena: The Anchor

_6 files, about 19,137 words._

| File | Title | Summary |
|---|---|---|
| [events-and-moving-parts.md](arenas/the-anchor/events-and-moving-parts.md) | THE ANCHOR: Events and Moving Parts | Heights use the Rim datum (Rim deck Z = 0). Times are seconds elapsed from the Marshal's "live" unless marked as remaining. |
| [layout.md](arenas/the-anchor/layout.md) | THE ANCHOR: Layout | - Origin: the Stump axis at Rim deck height. X east, Y north, Z up. Rim deck Z = 0 m. The lowest playable floor is the bowl floor at Z = -8 m. |
| [lighting-and-mood.md](arenas/the-anchor/lighting-and-mood.md) | THE ANCHOR: Lighting and Mood | - Time: 13:40 local, equatorial. Sun elevation 68 degrees, azimuth 255 (west-southwest). The sun sits over the west Rim, so after the Collapse the exposed west … |
| [overview.md](arenas/the-anchor/overview.md) | THE ANCHOR: Overview | One sentence: The Anchor is a map where the ground itself breathes and shoves on two timers, so the only safe place is the one everybody wants, twenty meters ac… |
| [prop-manifest.md](arenas/the-anchor/prop-manifest.md) | THE ANCHOR: Prop Manifest | Per 05 section 6.2: loose (0 N break-free, anyone), racked (200 N, anyone), stacked (500 N, not Climber or Scrapper), bolted or chained (800 N, Doubler or Break… |
| [signature-plays.md](arenas/the-anchor/signature-plays.md) | THE ANCHOR: Signature Plays | - The Kick (05 section 3.2): every Jack from a self-dog or between objects and geometry opens with the Kick, the 0.3 s capacitor launch to the class speed V of … |

## Systems (UI, audio, VFX, animation, camera, modes, onboarding)

_10 files, about 30,455 words._

| File | Title | Summary |
|---|---|---|
| [README.md](systems/README.md) | KINETIC: System Design Specifications | Other bible sources: 01 (LayCast as HUD, tone, the Office) in ui-hud and menus-and-frontend; 02 (Faces, kits, named rigs) in menus-and-frontend and onboarding-s… |
| [animation.md](systems/animation.md) | KINETIC: Animation Requirements | One shared humanoid skeleton for all five harnesses. Mass reads from silhouette and from motion, never from a different rig. A Breaker and a Climber share bones… |
| [audio.md](systems/audio.md) | KINETIC: Audio Design | Audio is a gameplay system before it is a mood. Bible 05 section 11 says every lethal thing has an audio tell with a lead time. The mix exists to make those tel… |
| [camera-and-feel.md](systems/camera-and-feel.md) | KINETIC: Camera, Feel, and Input | FOV is horizontal and does not change with aspect ratio below 16:9 (vertical is cropped). Sprint adds +4° over 0.2 s. Lines, Dogs, and the Cutter render with th… |
| [game-modes-and-match-flow.md](systems/game-modes-and-match-flow.md) | KINETIC: Game Modes and Match Flow | Common to all: two spawn zones per arena at opposite ends of the high or mid tier, 10 m across, never within 25 m of a Well edge (05 section 9.3). Reserve rigge… |
| [menus-and-frontend.md](systems/menus-and-frontend.md) | KINETIC: Menus and Frontend | Two voices own the frontend. |
| [onboarding-script.md](systems/onboarding-script.md) | KINETIC: Onboarding Script (The Apprenticeship) | - Under fifteen minutes total (04). Target 12:30. |
| [ui-design-system.md](systems/ui-design-system.md) | KINETIC: UI Design System | 1. The line is brighter than the UI. No UI element may out-glow a line. HUD luminance is capped at 70% of a line at 100 N, with 85% for alerts under 0.5 s (ui-h… |
| [ui-hud.md](systems/ui-hud.md) | KINETIC: In-Match HUD (the LayCast Package) | The HUD is LayCast's broadcast graphics package, seen from inside the harness. Every element must be justified as something the broadcaster would draw for four … |
| [vfx.md](systems/vfx.md) | KINETIC: VFX (Niagara Systems) | From 03: arenas are lit so that a singing line is always the brightest thing in frame. Nothing may out-glow a line. |

## Technical architecture (UE5)

_12 files, about 38,659 words._

| File | Title | Summary |
|---|---|---|
| [README.md](tech/README.md) | KINETIC Technical Documentation | 1. UE 5.8 shipped 2026-06-17 with hotfixes through at least 5.8.4. Epic's 5.8 notes mark Iris, MegaLights and Dataflow production-ready and the Physics Control … |
| [architecture-overview.md](tech/architecture-overview.md) | KINETIC Technical Architecture: Overview | Six players (eight in Free Scrap) on a dedicated server. Every elimination is a physics event: a Stall threshold crossed by relative speed, impulse, crush force… |
| [backend-architecture-diagram.md](tech/backend-architecture-diagram.md) | KINETIC: Backend Architecture Diagram | flowchart TB |
| [backend-architecture.md](tech/backend-architecture.md) | KINETIC: Backend Architecture | 1. The dedicated server is the only source of match truth. Clients never report results, Tonnage, Contract counts or Claim standing. The backend accepts those o… |
| [chaos-destruction.md](tech/chaos-destruction.md) | KINETIC: Chaos Destruction | - Geometry Collections (GCs) are built in Fracture Mode with the Uniform, Cluster, Radial, Planar, Slice, Brick (experimental), Mesh and Custom tools. Each frac… |
| [kinematics-and-force-volumes.md](tech/kinematics-and-force-volumes.md) | KINETIC: Kinematics, Force Volumes and Tells | Haulers, the Gantry, the Maintenance Ring, the Lift and its counterweight, the Breathing hull and the Shear Press ram are AKineticKinematicActor. Infinite mass,… |
| [networking.md](tech/networking.md) | KINETIC: Networking | Dedicated server, authoritative over all physics outcomes. Clients send inputs, predict their own character, Dog, line and flight body, and interpolate everythi… |
| [performance-budgets.md](tech/performance-budgets.md) | KINETIC: Performance Budgets | Physics runs at 60 Hz on every tier. Frame rate never changes physics. Competitive settings change only rendering. |
| [player-physics.md](tech/player-physics.md) | KINETIC: Player Physics | Only one of the first two is in the simulation at a time. The other is disabled (collision off, not ticking). The visual ragdoll is driven toward the flight bod… |
| [project-structure.md](tech/project-structure.md) | KINETIC: Project Structure | Build targets: KineticEditor, KineticGame, KineticServer, KineticClient. The server target compiles without Niagara and MetaSound runtime content. |
| [spool-and-line-implementation.md](tech/spool-and-line-implementation.md) | KINETIC: Spool and Line Implementation | The solver runs inside a Chaos::TSimCallbackObject registered with rewind support so it participates in networked physics resimulation. Epic's 5.8 overview stat… |
| [tech-stack-plan.md](tech/tech-stack-plan.md) | KINETIC: Tech Stack and Implementation Plan | 1. Team size decides the tooling. The roadmap assumes five core people in Phase 0 and about fifteen by Phase 1 ([team-and-roles.md](../production/team-and-roles… |

## Production

_7 files, about 10,467 words._

| File | Title | Summary |
|---|---|---|
| [art-pipeline.md](production/art-pipeline.md) | KINETIC — Art Pipeline and Quality Bar | Each object has one markdown file. Before modeling, the artist confirms from the spec: |
| [asset-master-list.md](production/asset-master-list.md) | KINETIC — Master Asset and Document List |  |
| [open-items.md](production/open-items.md) | KINETIC — Open Items After the v0.2.2 Reconciliation | Tension rescale to newtons; Jack as the Kick plus push; Kick interlock on Catch Plates; Reel within Hang; Hang never tests break-free; inclusive thresholds; rat… |
| [playtest-plan.md](production/playtest-plan.md) | KINETIC — Playtest Plan | - Phase 0: twice weekly from week 3. Internal, 6 players. |
| [risks.md](production/risks.md) | KINETIC — Risk Register |  |
| [roadmap-and-milestones.md](production/roadmap-and-milestones.md) | KINETIC — Roadmap and Milestones | Goal: prove the one thing nobody has shipped. A tether that is a spring, a negative spring, and a rigid rod between arbitrary bodies including a networked playe… |
| [team-and-roles.md](production/team-and-roles.md) | KINETIC — Team and Roles | - The bible is canonical. A document that disagrees with bible/05 is wrong until 05 is changed. |

## Folder indexes

| File | Title |
|---|---|
| [arenas/README.md](arenas/README.md) | KINETIC Arenas: Index, Comparison, and Level-Design Checklist |
| [characters/README.md](characters/README.md) | KINETIC Characters: Index and Shared Rules |
| [objects/README-props.md](objects/README-props.md) | KINETIC: Props, Volatiles, and Heavy Objects: Index and Shared Rules |
| [objects/README-structures.md](objects/README-structures.md) | Objects: structures, rated loads, kinematics, environment |
| [systems/README.md](systems/README.md) | KINETIC: System Design Specifications |
| [tech/README.md](tech/README.md) | KINETIC Technical Documentation |
