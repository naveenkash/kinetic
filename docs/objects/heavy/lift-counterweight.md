# Lift counterweight

Rated load. Rated tier (over 3,000 kg), 4,500 kg (**PROPOSED**). Concrete block 260 × 120 × 60 cm in a steel guide frame 320 × 160 × 110 cm overall (**PROPOSED**); the frame is what makes it read as car-sized. Appears only in Hangtown: it runs on a vertical guide through the Mid Hang, opposite to the Lift car, between the Low Hang and the Upper Hang headframe. It is the rated load that is already moving.

## Role in play

The counterweight is a kinematic while on its guide, carried as a rated attachment per 05 §6.7: it is kinematic while attached and becomes a simulated rated load when a Breaker's line reaches its 1,000 N break-free and holds it 0.6 s on the authored axis (the commit trigger, 05 §6.3), which releases it from the path on a scripted timeline regardless of friction or mass. Until then nothing moves it. Anything it hits takes the full relative impact, and anything caught between it and a walkway is crushed. Its danger is the shove and the pin, not the strike: at 3 m/s (**PROPOSED** Lift speed) a standing player struck square takes a 3 m/s relative hit, below the 4 m/s stagger threshold against a kinematic. The counterweight kills the player who runs into it, the player who is lined to it, and the player it pins.

| Rig | Use |
|---|---|
| The Caboose | Dog A enemy plate, Dog B counterweight, Haul or Hang. The enemy rides the Lift cycle into the headframe or the Low Hang deck. |
| The Anchor Drop | Enemy Hung to the counterweight as it starts its down run. |
| Overboard | Jack off it as it passes: a moving wall behind you. Against the enemy the Jack is a T push, no Kick (05 §3.2). A light class Overboarding a Breaker moves themselves, not the Breaker: a Scrapper's 400 N on 220 kg is 1.8 m/s², under the Breaker's foot-hold, and that is intended (05 CHANGELOG item 49). |
| Rated consequence | A Breaker's 1,000 N line, a Doubler at 1,200 N overspin, or any lines summing to 1,000 N on one axis (a Rigger's 600 plus a Scrapper's 400, 05 §3.3), held 0.6 s on the authored axis, pulls it off its guide (05 §6.3). |

- **Scrapper (400 N):** cannot move it. Dogs enemies to it for the Caboose. Jacks off it as it passes.
- **Rigger (600 N):** Hangs an enemy to it on a set line and walks away. Hangs a Clothesline across its guide so the counterweight severs it on schedule: kinematics sever any line they pass through (the snap, 150 N·s per 1,000 N recoil, both Dogs release; 05 §3.5).
- **Breaker (1,000 N):** the only base line that reaches the rated break-free. Their own line pulls back at 4.5 m/s², under their 5 m/s² foot-hold, so they do not need to brace: they stand for 0.6 s on the authored axis, the guide shoe tears, and the authored consequence fires (05 §6.3). Letting go before 0.6 s resets it. A Doubler's full overspin on them (5.5 m/s²) would take them off their feet unbraced; braced (20 m/s²) it does not. A Rigger and a Scrapper pulling on one axis reach the same 1,000 N.
- **Climber (400 N):** rides it. Jacking off it takes the whole 18 m/s, since a kinematic receives nothing. Perches on the frame for 1.5 s, re-dogs, rides again. The honest route up, taken dishonestly.
- **Doubler (800 N, 1,200 N overspin):** base 800 N does not reach it; 1,200 N overspin does, for 0.8 s before the snap at 1.8 s launches the Doubler off the guide at 10 m/s. The snap's 1,500 N·s into a released block is 0.33 m/s.

## Appearance

Silhouette: a dense grey slab in a rectangular steel frame with four guide shoes riding two vertical rails, a cable eye on top, and yellow-and-black tape on every frame member.

| Part | Material | Base color | Rough / Metal |
|---|---|---|---|
| Block | `MI_Concrete_Cast` | `#8E8B84` | 0.85–0.95 / 0 |
| Water staining, lower third | `MI_Concrete_Stained` | `#6F6C66`, efflorescence `#D8D4C8` | 0.88–0.98 / 0 |
| Frame, guide shoes | `MI_Steel_Painted_Worn` | Guild grey `#5B6670` | 0.45–0.70 / 0 paint, 1 chips |
| Rated tape | `MI_Tape_Rated` | `#F2C230` / `#141414`, 50 mm chevrons at 45° | 0.50–0.60 / 0 |
| Cable eye, rails | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 / 1 |
| Rail grease | `MI_Decal_Grime` | `#2B2622` | inherits |

Wear story. The block shows formwork seams every 60 cm and four tie holes per face. Its top is chalked with the Lift's rated mass by the Guild's Lift crew: `4500` in a hand, and a Ledger number `LG-####`. The frame's leading faces are polished where they pass the walkway edges. Guide shoes are bright steel with a grease fillet. The Office's white `RATED LOAD` stencil sits under the tape on the long sides. Cooking smoke has darkened the upper third; the lower third is water-stained from the drip nets. Under the Office's lights the tape is the brightest thing on it and still well under a singing line.

Nothing on it is orange. The slack-cable tape is orange and cuttable; the counterweight's cable is not cuttable, and its tape is yellow-and-black so no player confuses the two.

## Variants

| Variant | Notes |
|---|---|
| Guild-run | Default. Grey frame, chalk mass, `LG-####`. |
| HTR-run | After an HTR Claim win. Frame `MI_HTR_Grey_Matte` `#4A4D50`, orange band, `HTR-RC-####` plate. Tape unchanged. |
| Off-guide | State after the rated consequence: frame bent, one shoe torn free. |
| Top of travel | Parked at the headframe with the Lift car at the Low Hang. Authored start position per round. |

## Physics

| Property | Value |
|---|---|
| Tier / mass | Rated, 4,500 kg (**PROPOSED**) |
| Collision | Box 320 × 160 × 110 cm (frame). Block box 260 × 120 × 60 cm nested for Dog seating. |
| Center of mass | Center of the block. |
| Friction | 0.6 concrete on timber, 0.3 steel on steel when off-guide. |
| Restitution | 0.02 |
| Break-free | Rated, 1,000 N held 0.6 s on the authored axis: a commit trigger, not a physics slide (05 §6.3). Static to any line under 1,000 N (05 §3.3). Breaker; Doubler at overspin; or lines summing to 1,000 N on one axis (Rigger 600 + Scrapper 400). |
| Fracture | None. Non-fracturing. |
| On guide | Kinematic. Infinite mass. Scripted path, Lift speed 3 m/s (**PROPOSED**), run 40 m, dwell 10 s at each end, 46 s cycle (**PROPOSED**). |

Line acceleration once off the guide (T / m): 1,000 N 0.22 m/s²; 1,200 N 0.27 m/s². Lines under 1,000 N: zero, the line sings and the block does not notice. Over 2 m at 0.22 m/s² the block reaches 0.94 m/s, three times its free-body Stall speed; in practice gravity takes it the moment the shoe tears.

Against a player, on guide (player-hits-kinematic table: stagger 4 m/s, flip 6 m/s, Stall 9 m/s relative): a stationary player brushed at 3 m/s takes 3 m/s, no stagger. A Rigger running into it at 6.2 m/s meets it at 9.2 m/s: Stall. A Climber sprinting at 9 m/s into it: 12 m/s, Stall. A player lined to it and dragged (the Caboose) hits the headframe stop at 3 m/s relative to the stop plus whatever the line added.

Crush: a player pinned between the moving frame and a walkway rail is loaded by a kinematic, which applies whatever force the pin needs. The 8 kN for 0.5 s threshold is met in 0.5 s. This is the counterweight's kill. The 8 kN crush threshold is unreachable by any line; it comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1).

Against a player, off guide (4,500 kg × relative speed): stagger at 0.07 m/s, flip at 0.13 m/s, Stall at 0.27 m/s. At 2 m/s: 9,000 N·s. From a 7 m drop (11.7 m/s): 52,650 N·s. At the Haul cap: 112,500 N·s. Its own weight is 44 kN; anything under it is scrapped.

## Interactions

- **Haul to it:** the player is dragged along its path. Down run: toward the Low Hang deck. Up run: toward the headframe. The Caboose.
- **Jack from it:** a launch off a moving wall; the player takes the whole class V (Climber 18, Scrapper 14, Rigger 12, Doubler 14, Breaker 10 m/s) plus the counterweight's own 3 m/s if it is moving away. Overboard with the Well below. Dog A counterweight, Dog B enemy, Jack: no Kick (05 §3.2); the kinematic takes nothing and the enemy takes the whole T over their mass.
- **Hang from it:** the player rides it on a rigid tether. They can look, cut, and fire Dogs. They cannot move. A Hang never tests the rated 1,000 N break-free: break-free is active pull only (Haul, Jack, the Kick), so any class can Pendulum or Gallows from the frame while it runs its guide (05 §6.2); only a Breaker's Haul, a Doubler's overspin, or a crew's summed 1,000 N releases it. Reel works on it like on any Hang: holding Haul within Hang shortens the rod at 1.0 m/s up to the class Hang load (05 §3.2), so a player Hung to the frame climbs to it by Reel while it runs, and a Gallows anchored to the frame hoists the enemy by Reel while the kinematic carries both along its path.
- **Object ↔ object:** off guide it goes through anything Heavy or lighter. On guide, a house swung into it breaks (2,800 kg at 3 m/s relative is 8,400 N·s).
- **Object ↔ kinematic:** it is the kinematic. A Hang line between it and any object drags the object along the run and shears it at the walkway edges. Any line it passes through that is not attached to it is severed: the snap, 150 N·s per 1,000 N recoil to both ends, both Dogs release (05 §3.5). Lines attached to the frame ride with it.
- **Water, Updraft, Breathing:** not present. Force volumes would not touch it anyway: they act on players and loose objects only (05 §6.6).
- **Floats / slides / rolls:** neither. Off guide it falls.
- **Chain reactions (rated consequence, PROPOSED):** a Breaker's Haul at 1,000 N, a Doubler's overspin, or a crew's summed 1,000 N, held 0.6 s on the authored axis, tears a guide shoe (05 §6.3). The block leaves the guide (05 §6.7), becomes a simulated 4,500 kg body, and falls the Mid Hang shaft to the Low Hang nets and through them into the Well. The Lift car, now unbalanced, runs up to the headframe and strikes its stop; the car is a kinematic hazard at the top for the rest of the round. Everything on the block's fall line is removed. The consequence resets between rounds.

## States

| State | Trigger | Visual | Physics |
|---|---|---|---|
| Intact, on guide | authored | Clean frame, tape, grease on rails | RC-K kinematic, cycle running |
| Dented | not applicable | Concrete chips on the leading faces are authored decals, not a state | None |
| Broken (off guide) | line at 1,000 N or more | Guide shoe torn, frame bent, block tilted 15° | RC-R simulated 4,500 kg, falling |
| Destroyed | in the Well | Gone. Lift car parked at the headframe. | Removed; car hazard scripted |

## VFX and SFX hooks

- Running: `SFX_Lift_Rail_Loop`, a low rumble with grease squeal, audible 20 m. Approach klaxon not used; the Lift is on a readable cycle and the rail sound is the tell.
- Player struck: `SFX_Impact_Concrete_Body`.
- Rated groan: `SFX_Groan_Rated_Lift`, 1.0 s minimum lead before the shoe tears. `NS_Dust_Rated` from the guide rails, `NS_Sparks_Shoe` at the shoe.
- Consequence: `SFX_Lift_Freefall`, then `SFX_Impact_Concrete_Massive` at the nets, then `SFX_LiftCar_Stop` from the headframe.
- Emissive: none.

## Technical

- LOD0 24,000 tris; LOD1 12,000; LOD2 4,000; LOD3 800. Nanite on.
- Textures: 2 × 2K (block, frame) plus `T_Trim_Steel_01` at 4K and the stencil atlas.
- Pivot: the primary hang point, the cable eye on top of the frame. X+ along the long axis. Z+ up.
- Dog sockets, all on taped frame members, all `Metal`: `SKT_Dog_Top_01` (top rail beside the eye), `SKT_Dog_Hook_01` (cable eye), `SKT_Dog_Hook_02` (lower lifting lug), `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01` (long-side frame rails), `SKT_Dog_Frame_01` (leading vertical). None on the concrete; a Dog that lands there uses the concrete physical material, which matters for Mag and Spike Dogs.
- Chaos: not a geometry collection. `PHYS_LiftCounterweight` single box. Kinematic on a spline matched to the Lift car by the match clock. Anchor field `FS_Anchor_LiftCW` with rated threshold 1,000 N, releasing after 0.6 s of qualifying pull on the authored axis, not instantly (05 §6.3); release switches the body from kinematic to simulated per 05 §6.7 and fires the consequence RPC after the 1.0 s groan.
- Replication: RC-K on guide, transform by match clock, deterministic. RC-R once released: server-authoritative, 15 Hz, consequence as a server RPC.

## Reference search terms

traction elevator counterweight frame, mine hoist counterweight concrete, counterweight guide shoe rail grease, cast concrete block formwork tie holes, incline railway counterbalance, cable car counterweight pit, yellow black hazard chevron tape steel frame.
