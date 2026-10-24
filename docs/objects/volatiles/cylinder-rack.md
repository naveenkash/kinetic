# Cylinder rack

Volatile. Medium tier empty at 95 kg; Heavy tier full at 365 kg (rack plus six 45 kg propane cylinders). Tier is recomputed whenever the mass changes (05 §6.3). 120 × 80 × 150 cm, a welded steel frame holding six cylinders in two rows of three, each retained by a top chain. Appears in Ballast, the Crown (beside the HTR cutting gantry) and the Bays (one per catwalk landing); the Yard, the Stacks (cutting stations, two per sector); Hangtown, the Mid Hang (behind the market platform's kitchen row); the Anchor, the Scaffold (levels 2 and 4, one per quadrant).

## Role in play

Six bombs in a box. It is the Kiss's ammunition store and the Scatter's armory, and it is the single most dangerous object on a mid tier when it goes. It also tells players where cylinders are: every cutting station has one. Racked cylinders need 200 N to pull (05 §10), which every line has.

Why it is there: cutting crews draw cylinders from the rack and return empties to it. Prep crews leave the rack full because the cut resumes after the match. The Office tags the rack and every cylinder in it.

| Harness | What they do with it |
|---|---|
| Scrapper | Pulls single cylinders out at 200 N, fast, one after another, each at 8.9 m/s² plus the 30% bonus. Never stands within 6 m of a full rack. On an empty rack (95 kg, under 100 kg) the bonus applies too. |
| Rigger | Hangs a set line from an overhead anchor to the bolted rack. The rack does not move (800 N); the line is a tripwire with a rack at its foot. |
| Breaker | Hauls the bolted full rack free at 1,000 N against 800 N. 365 kg at 2.7 m/s²: 10.4 m/s at the 20 m line end. The first cylinder to touch anything over 8 m/s goes, then all six. |
| Climber | Jacks one cylinder out of the rack from above. The Kick, V 18 split against 45 kg, gives the cylinder 12 m/s at 0.3 s, over the trigger, back down onto the rack. |
| Doubler | Two lines on two cylinders in the same rack, Jack. Each line is 480 N, over the 200 N rack release. Both leave over the trigger in opposite directions. The rack stays unless the Doubler is at overspin. |

## Appearance

Silhouette: a square cage of 40 mm angle iron, open top, six pale verticals inside, chains across the top. Forklift pockets at the base. Reads as a rack of cylinders at 50 m because the cylinders are pale and the frame is dark.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Frame | `MI_Steel_Painted_Worn`, grey `#5B6670` | | 0.45 to 0.70 | 0 / 1 chipped |
| Chains | `MI_Chain_Oiled` | `#3C3E42` | 0.30 to 0.45 | 1 |
| Fork pockets, feet | `MI_Steel_Bare` | `#5C5F63` | 0.55 | 1 |
| Rust | `MI_Steel_Rusted` | `#7A3F1C` | 0.85 | 0.3 |
| Cylinders | per the propane cylinder spec | `#B8B4A8` | 0.40 to 0.60 | 0 |
| Placard | `MI_Decal_Stencil` | white | | |

Wear: frame paint gone on the top rail where cylinders are lifted over it, polished by hands. Rust at the feet and in the fork pockets. Chains bright where they run through the hooks, dark elsewhere. Cylinder bases scraped from the rack floor.

Markings: `OFFICE OF THE LAY / SITE PROPERTY` on a white placard wired to the front rail, with Marshal number and a `6 / 6` tally chalked by the prep crew. A faded yellow diamond. HTR racks carry `HTR-RC-####` in orange on the top rail; Guild racks a Ledger plate `LG-####` in ochre; Hangtown racks are painted two hanger colors and have a kitchen stencil. **PROPOSED** `RUCU GAS` on the cylinder bands.

Under arena lighting the rack is a dark frame around six pale bars. No emissive. On Ballast's Crown it is salt-white on the windward side. In the Yard it stands in cutting dust with slag on the near face. In Hangtown it is wired to the platform rail. On the Anchor it is dusted pale and the placard flaps.

## Variants

| Variant | Difference |
|---|---|
| Full, bolted (default) | 365 kg Heavy. Frame break-free 800 N (Doubler, Breaker). Each cylinder 200 N (anyone). |
| Full, loose | 365 kg Heavy. Frame 0 N. Slides on any line, slowly: 1.1 m/s² on 400 N. |
| Half (3 cylinders) | 230 kg Medium. Chalk tally `3 / 6`. Empty slots show rust rings. |
| Empty | 95 kg Medium, under 100 kg, so the Scrapper bonus applies. Chains hanging loose. No blast. Rings when hit. |
| Empties (6 empty cylinders) | 227 kg Medium. Cylinders have no handwheels. No blast; each vents a harmless 1 s hiss on impact over 8 m/s. Chalk `MT`. |
| Hanger | Painted, wired to a rail (800 N), one slot holds a kitchen cylinder in teal. |

## Physics

Rack alone: Medium, 95 kg. Full: Heavy, 365 kg. Collision: frame as four box rails (4 × 4 cm) and a base plate 120 × 80 × 5 cm; each cylinder its own capsule, radius 15 cm, length 105 cm, as a separate body constrained to the rack. Center of mass 65 cm up when full, 40 cm empty. Friction 0.55 on concrete, 0.4 on steel (**PROPOSED**). Restitution 0.1. The full rack tips past 28°.

Break-free: frame bolted 800 N (Doubler at 800 N, Breaker at 1,000 N, Doubler overspin at 1,200 N; not Climber, Scrapper, or Rigger; not split Doubler lines at 480 or 720 N) or loose 0 N by variant. Each cylinder in its slot, 200 N: every class pulls one, including a Doubler's split line at 480 N.

Haul on the loose or freed rack, other end static or braced:

| Class | T | Full 365 kg | Speed at line end | Impulse | Empty 95 kg | Speed at line end | Impulse |
|---|---|---|---|---|---|---|---|
| Climber | 400 N | 1.1 m/s² | 10.5 m/s at 50 m | 3,830 N·s, Stall | 4.2 m/s² | 20.5 m/s | 1,950 N·s, Stall |
| Scrapper | 400 N | 1.1 m/s² | 6.6 m/s at 20 m | 2,410 N·s, Stall | 4.2 m/s², 30% bonus | 13 × 1.3 = 16.9 m/s | 1,605 N·s, Stall |
| Rigger | 600 N | 1.6 m/s² | 11.3 m/s at 40 m | 4,120 N·s | 6.3 m/s² | 22.4 m/s | 2,130 N·s |
| Doubler | 800 N | 2.2 m/s² | 11.5 m/s at 30 m | 4,200 N·s | 8.4 m/s² | 22.4 m/s | 2,130 N·s |
| Breaker | 1,000 N | 2.7 m/s² | 10.4 m/s at 20 m | 3,800 N·s | 10.5 m/s² | 20.5 m/s | 1,950 N·s |

A full rack arriving at 10 m/s puts six cylinders over the 8 m/s trigger at the same instant.

Trigger: a cylinder triggers on impact over 8 m/s or another blast. Each cylinder: radius 4 m, 1,500 N·s at center, linear to 0 at 4 m, players within 2 m Stalled, 2 to 4 m flipped (authored overrides per 05 §6.4). Hiss and venting 0.3 s before each blast. Cylinders stand 40 cm apart in the rack. Under more than 1 m of water each blast is half impulse.

Chain reaction, **PROPOSED** timing: the first blast arms every neighbor within 4 m at once (all five). Each arms at 0.3 s and the detonations are staggered 0.1 s apart in order of distance, so the rack empties over 0.8 s. The first blast also launches neighbors: at 0.4 m the impulse is 1,350 N·s on 45 kg, 30 m/s, held by the 200 N chain for the first 0.05 s and then free. Armed cylinders therefore travel 1 to 3 m before they blow. The impulses are not summed at a point; each blast is its own 4 m field.

| Distance from rack center | What arrives | Player result |
|---|---|---|
| 0 to 2.6 m | inside the 2 m Stall rule of at least one cylinder | Stall |
| 2.6 to 4.6 m | inside the 2 to 4 m flip rule of at least one | flip |
| 4.6 to 7 m | a thrown armed cylinder may land and blow here | Stall or flip by its own rule |
| over 7 m | nothing (**PROPOSED** effective hazard radius 7 m) | |

The rack as a projectile: full, 365 kg: 300 N·s at 0.8 m/s, 600 at 1.6 m/s, 1,200 at 3.3 m/s. Empty, 95 kg: 300 at 3.2 m/s, 600 at 6.3, 1,200 at 12.6 m/s. A 7 m drop at 11.7 m/s: full 4,270 N·s Stall; empty 1,110 flip. Walked into at 2 m/s: full 730 flip; empty 190 nothing. Jack: a Breaker (220 kg) Jacking the full rack: the Kick gives the rack V 10 × 220 / 585 = 3.8 m/s and the Breaker 6.2 m/s; T then crawls the rack along at 2.7 m/s². Snap at full overspin on a loose full rack: 1,500 N·s is 4.1 m/s, Stall on whoever is in front, no cylinder over the trigger. Cut recoil: 150 N·s per 1,000 N; nothing on 365 kg. Crush: full rack 3.6 kN of weight, under 8 kN; a Breaker's 1,000 N Haul pressing it adds 1 kN, 4.6 kN, still under. The full rack crushes only with another load stacked on it.

## Interactions

Haul to it: a bolted rack is a grapple point for any line under 800 N. A Breaker or Doubler Hauls the rack to them and must move before it arrives, because the first cylinder to hit anything at 8 m/s blows them with it. Jack from it: a Slingshot anchor under 800 N; the Kick gives the player nearly all of V against a bolted rack. Hang from it: a solid Pendulum anchor at any class Hang load. Break-free is tested against active pull only (05 §6.2), so a player hanging from the bolted frame, or from a single racked cylinder, does not unseat it. Switching the same line to Haul afterward pulls the cylinder out at 200 N: a pendulum bomb.

Object to object: a crate Hauled into a full rack at over 8 m/s blows the rack. Two racks within 4 m of each other chain: level design keeps racks 8 m apart (**PROPOSED**). A cylinder Scattered out of a rack (cylinder to cylinder, Jack) Kicks on both ends, carries the rack's 200 N release lurch, and leaves at V/2 plus T/m. A Jack from a racked cylinder to an enemy's Catch Plate does not Kick (05 §3.2): it is a T push only, which still clears the 200 N release and sends the cylinder out at T over 45 kg.

Kinematics: the Gantry load dropped on a rack blows it. A Hauler through a loose rack sends six armed cylinders down the trench ahead of it. A kinematic passing through a line severs it (05 §3.5): a Hauler crossing a Hang line from the rack to the far side of a trench snaps it and both Dogs release.

Water: the rack sinks; full cylinders float out of an untethered slot. In the Hold the Breathing's water shove drags a loose rack 0.3 m per cycle. Blasts from a rack standing in water over 1 m deep are half impulse each; the Hold's seep water is 40 cm, so Hold racks blow at full. Force volumes act on players and free bodies only, scaled a_ref × min(100 / m, 2) (05 §6.6): racked cylinders and the bolted frame are unaffected by the Updraft or the Pulse until they are freed. A loose full rack (365 kg) feels 0.27 times the authored acceleration and does not lift; a loose empty rack (95 kg) feels 1.05 times; a cylinder pulled from the rack feels 2 times (the cap) and lifts in the Updraft, and a cylinder a player has pulled out and is holding on a Hang line swings as a free body. The Pulse (0.4 s out, 0.6 s in toward the Stump, every 75 s from 0:55) moves loose cylinders on the Scaffold and leaves the rack alone. Breathing tilt: a loose full rack on 6° steel does not slide (friction holds to 22°) or tip (28°). A loose empty rack is the same.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Six cylinders, chains taut, tally `6 / 6`. | Full. |
| Dented | Frame rail bent, one cylinder leaning. After any impact over 600 N·s. | Unchanged. |
| Drawn | Slots empty with rust rings; chains hanging. | Mass drops 45 kg per slot. Tier recomputes: Medium below 300 kg, which is one cylinder drawn (320 kg is still Heavy; 275 kg with two drawn is Medium). Under 100 kg with all six drawn, the Scrapper bonus applies. |
| Armed | One or more cylinders venting white, hiss. 0.3 s. | Committed. |
| Blown | Frame twisted, torn open on the blast side (Medium, 70 kg, non-fracturing). Six scorch decals. Foot rings scattered (Light, 10 s). | Each blast its own field. Frame survives as a loose Medium. |
| Destroyed | The twisted frame remains for the round. | 70 kg prop, 0 N. |

## VFX and SFX hooks

- Impact: frame clank with a chorus of cylinder rings. The ring count tells players how full it is.
- Break-free: cylinder from slot, chain rattle and a scrape, `NS_Lurch_Dust_Small`; rack from bolts, concrete shear, `NS_Lurch_Concrete_Medium`.
- Armed: `NS_Vent_Propane` per cylinder, up to six, `SFX_Vol_Hiss_Propane` layered with a 0.1 s offset so six hisses read as a swarm.
- Blast: `NS_Blast_Propane` per cylinder, staggered 0.1 s; `NS_Blast_Rack_Frame` once (frame sparks, chain fragments). Audio `SFX_Vol_Blast_Propane` six times with the Marshal gallery echo once.
- No emissive. The placard is retroreflective.

## Technical

LOD0 5,000 tris for the frame, plus six cylinder instances at their own LODs; LOD1 2,500, LOD2 1,000, LOD3 250. One 2K set for the frame plus the stencil decal atlas; cylinders use their own set. Pivot at the bottom center of the base plate.

Dog sockets on the frame:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Top_01` | Top rail, front center | Metal |
| `SKT_Dog_Top_02` | Top rail, rear center | Metal |
| `SKT_Dog_Hook_01` | Fork pocket, left | Metal |
| `SKT_Dog_Hook_02` | Fork pocket, right | Metal |
| `SKT_Dog_Frame_01` | Front upright, 100 cm | Metal |
| `SKT_Dog_Frame_02` | Rear upright, 100 cm | Metal |
| `SKT_Dog_Load_01` to `_06` | One per cylinder slot, at the cylinder's own Top socket | Metal |
| `SKT_Dog_Valve_01` | Chain hook, top rail (the rack has no valve; this keeps the volatile socket set complete) | Metal |

A Dog on a `Load` socket seats in that cylinder; a Haul on it pulls the cylinder out at 200 N, not the rack. Not a geometry collection; the frame swaps to its Blown mesh. Anchor fields: `FS_Anchor_CylRack_Frame` (800 N or none by variant) and `FS_Anchor_CylRack_Slot_01` to `_06` (200 N). Each cylinder spawns its own `FS_Blast_Propane` radial field (400 uu, 1,500 N·s, linear falloff, halved under 1 m of water) from a server event; the chain is driven by the server arming each neighbor on the first blast and scheduling detonations 0.1 s apart. The rack's tier and Scrapper-bonus flag are recomputed server-side whenever a slot empties. Replication RC-M for the rack and each cylinder at 20 Hz; the full rack stays RC-M despite its Heavy mass because its cylinders are Medium bodies (**PROPOSED**).

## Reference search terms

six cylinder gas rack pallet forklift pockets; LPG cylinder cage rack chained; welding gas bottle rack steel; cylinder storage cage shipyard; propane cylinder pallet rack rust; gas rack top rail chain hooks.
