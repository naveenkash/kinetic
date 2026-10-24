# Shipping container (20 ft)

Heavy prop. Heavy tier, 2,000 kg. 606 × 244 × 259 cm (ISO 20 ft). Appears in Ballast (the Bays, stacked on the dead magnetic racks either side of the Spine; loose units in the Hold) and the Yard (the Stacks, as billet and parts containers between the cut hull sections). Not in Hangtown. On the Anchor only as Rim storage behind the broadcast towers.

## Role in play

The container is the game's reference Heavy. 05 §3.4 tunes line feel against it: a Breaker's 1,000 N on 2,000 kg is 0.5 m/s², which is why containers lurch and fall rather than fly. A line starts the Avalanche; gravity finishes it. It is the Avalanche's body, the Guillotine's biggest load, and the thing that turns a lean in Ballast into a kill.

| Rig | Use |
|---|---|
| The Avalanche | Dog A on the bottom container of a stack, Dog B toward you or self-dog, Haul at or above the 500 N stack break-free. The stack comes down. Bracing is optional: a Rigger's own 600 N pulls back at 4.6 m/s² and a Breaker's 1,000 N at 4.5 m/s², both under their 5 m/s² foot-holds. |
| The Guillotine | A container Hung free from a Bay catwalk on set lines, lifted into position by Reel (hold Haul within Hang, 1.0 m/s), then dropped by gravity when the owner recalls or cuts their own set line; a second line from container to enemy only steers it (05 §10). Its 19.6 kN weight exceeds every single Hang load, so hanging one free takes two Riggers' lines summed (24 kN) per 05 §10 and §6.2a. |
| The Anchor Drop | Enemy Hung to a container on the Bay edge after the Shear. Tip the container. |
| The Caboose | Dog an enemy to a container being carried by the Yard Gantry, or Jack them off one: the Kick never fires with a Dog in a Catch Plate, so that Jack is a T push and the enemy skids at T over their mass (05 §3.2). |

- **Scrapper (400 N):** cannot start a stacked container (500 N break-free), and no Heavy object gets the Scrapper's 30% speed bonus. Can Hang to a loose one as an anchor for Overboard (a T push, no Kick, since the far Dog is in a Catch Plate); against a Breaker the Scrapper moves and the Breaker does not, 1.8 m/s² on 220 kg, which is intended (05 CHANGELOG item 49). Can hide behind it. Spills its contents for ammunition once the doors are blown.
- **Rigger (600 N):** starts a stack at 0.30 m/s², 100 N over the break-free. Hangs from a stacked container as a Pendulum anchor at any time: a Hang line is not an active pull, so it never tests the 500 N break-free (05 §6.2). Sets the Guillotine only with a second Rigger: a free-hung container (19.6 kN) exceeds one 12 kN Hang load and slips a single line at 0.5 m per 0.1 s; two Riggers' Hang loads sum to 24 kN (05 §10, §6.2a).
- **Breaker (1,000 N):** the Avalanche at 0.50 m/s². Hauls the bottom unit of a whole stack onto a lane; after the lurch, gravity does the rest.
- **Climber (400 N):** rides containers. Slingshots off their sides and takes almost all of their 18 m/s launch speed, since the container is 22 times their mass. Cannot start a stacked one, but Pendulums from one freely: a Hang does not test break-free. Uses any container's corner casting as a pivot.
- **Doubler (800 N, 1,200 N overspin):** starts a stack on a single line; a split line at 480 N does not, a split line at 720 N overspin does. A full overspin snap delivers 1,500 N·s to the container: 0.75 m/s, just over its 0.60 m/s Stall speed, so a container snapped into a shin still scraps, while the Doubler is launched back at 10 m/s and staggered.

## Appearance

Silhouette: the ISO box, corrugated sides, two doors on one end with four vertical lock bars, eight corner castings. Nothing added that breaks the read at fifty meters.

| Part | Material | Base color | Rough / Metal |
|---|---|---|---|
| Corrugated panels | `MI_Steel_Painted_Worn` | per variant | 0.45–0.70 / 0 paint, 1 chips |
| Corner castings, lock bars, hinges | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 / 1 |
| Door gaskets | `MI_Rubber_Black` | `#1E1F20` | 0.70–0.85 / 0 |
| Floor (visible when doors open) | `MI_Plywood_Weathered` | `#9C8564` | 0.80–0.92 / 0 |
| Rust bloom at panel seams and under castings | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 / 0.25 |

Wear story. Hand polish on the lock bar handles and the first 40 cm of the door edge. Rust runs from the top corner castings down the end panels in 10 to 30 cm streaks. The bottom 20 cm of every side is scraped to bare steel by forklift tines and rack runners. Stencils on both long sides and the door end: owner mark top left, a 40 cm asset code bottom right, and the Office's white `SITE PROPERTY` line under it on every container inside a claim. Under arena lighting the corrugation reads as alternating 10 cm bands; the paint albedo is kept below 0.5 luminance so a singing line is always brighter.

Per arena. Ballast containers wear the Meridian cyan at 0.8 wear and sixty-one years of seep water: green-black grime `#3A4A3C` on every lower face, salt white on the top. Yard containers are HTR orange at 0.3 wear with oil-black cutting dust on the roof. A Dog seat mark on any container is a 5 cm bright ring, authored decal, so players can see where lines have been.

## Variants

| Variant | Mesh | Livery | Where |
|---|---|---|---|
| Meridian cargo | `SM_Heavy_Container20_Meridian` | `MI_Meridian_Cyan` `#2A9FB5`, hexagon logo, cargo code `MC-7-####` | Ballast Bays and Hold |
| HTR reclamation | `SM_Heavy_Container20_HTR` | `MI_HTR_Orange_PowderCoat` `#E8620C`, `HTR-RC-####` | The Yard, Anchor Rim |
| Guild tagged | `SM_Heavy_Container20_Guild` | grey `#5B6670` body, ochre knot glyph and `LG-####` | The Yard (Guild plant) |
| 10 ft half | `SM_Heavy_Container10_*` | any of the above | Ballast Hold. 299 × 244 × 259 cm, **PROPOSED** 1,300 kg. |
| Doors blown | state mesh | any | After a 6,000 N·s impact. |

## Physics

| Property | Value |
|---|---|
| Tier / mass | Heavy, 2,000 kg (10 ft: **PROPOSED** 1,300 kg) |
| Collision | Box 606 × 244 × 259 cm. Doors: two boxes 122 × 259 × 5 cm on hinge constraints. |
| Center of mass | Geometric center. Loaded variant: lowered 20 cm (**PROPOSED**). |
| Friction | Rollers under every Avalanche stack (05 §6.1a, item 54): the Ballast racks' dead rollers and the Yard Stacks' and Anchor Rim's roller pallets, 0.02 to start, 0.005 rolling. 0.50 directly on concrete: fixed scenery, no line moves it (05 §6.1a). |
| Staging (05 §6.1a) | Ballast Bays and Hold: on the magnetic racks' dead rollers, 0.02; start force 0.02 × 19.6 kN = 390 N once any break-free is passed, so a loose Hold container moves for every class and a stacked one for Rigger, Doubler, Breaker. Yard Stacks and Anchor Rim: on roller pallets, the same 0.02 start (392 N, under the 500 N stack break-free, so the 500 N governs) and 0.005 rolling (98 N). Yard Floor singles set directly on concrete: fixed. They dent and their doors blow, but no line moves them. |
| Restitution | 0.05 |
| Break-free | Loose 0 N. In a stack 500 N: Rigger, Doubler, Breaker start it; Climber and Scrapper (400 N) do not; a Doubler's split line starts it only at 720 N overspin. Chained on a rack 800 N: Doubler, Breaker. |
| Fracture | 6,000 N·s. Does not fracture. Dents. Doors blow open and spill contents. |

Line acceleration (T / m): 400 N 0.20 m/s²; 600 N 0.30; 800 N 0.40; 1,000 N 0.50; 1,200 N 0.60. All under 0.1 g. Speed over distance is v = sqrt(2 a d): a Breaker's 1,000 N moves a loose container to 1.4 m/s after 2 m and 4.5 m/s at the end of a 20 m line. Those are frictionless figures. On rollers, once the 500 N stack break-free (or 0 for a loose unit) is passed, the 98 N rolling cost leaves (T minus 98) / 2,000: Rigger 0.25 m/s², Doubler 0.35, Breaker 0.45, Doubler at overspin 0.55; the same in Ballast and the Yard. Under a Breaker the container passes its 0.60 m/s Stall speed after 40 cm, under a Rigger after 72 cm. It never approaches the 25 m/s Haul cap on a flat floor; only a drop gets it there.

Against a player (impulse = 2,000 kg × relative speed): stagger 300 N·s at 0.15 m/s; flip 600 at 0.30 m/s; Stall 1,200 at 0.60 m/s. At 2 m/s: 4,000 N·s. From a 7 m drop (11.7 m/s): 23,400 N·s. At the Haul cap, 25 m/s: 50,000 N·s. Dropped from Ballast's Crown it is capped at the 50 m/s hard velocity limit on everything (05 §6.3): 100,000 N·s. A player's body counts as an impacting object: a Breaker flipped into a container at 10 m/s delivers 2,200 N·s, a dent and nothing more. Crush: resting on a player it applies 19.6 kN, so a pin against anything solid is a Stall at 0.5 s. The 8 kN crush threshold is unreachable by any line; it comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1). On the Breathing's rollers a loose container starts (tan 6° = 0.105 beats the 0.02 start) and then accelerates at g(sin 6° minus 0.005 cos 6°) = 0.98 m/s² down the lean, reaching Stall speed in 0.6 s.

## Interactions

- **Haul to it:** the player is pulled to the container. If it is on a stack above them, a Haul alone does not lift them: 1,000 N is under every harness's weight except a Climber's 883 N on a Breaker's line. They Hang and climb by Reel instead, holding Haul within Hang to shorten the rod at 1.0 m/s (05 §3.2). If the container is falling, they go with it.
- **Jack from it:** a wall for Slingshots and Overboard. Jack splits the class launch speed V by inverse mass within 0.3 s, then pushes at T. A Breaker (V 10 m/s) sends a loose container off at 1.0 m/s and takes 9.0 m/s back; a Climber (V 18) sends it 0.8 m/s and takes 17.2. The container then gains 0.2 to 0.6 m/s² until the line releases at length. A loose container Jacked by a Breaker is already past its 0.60 m/s Stall speed when it leaves. Jacking off a stacked container that does not break free gives the player the whole of V. Dog A container, Dog B enemy, Jack: no Kick, since a Dog is in a Catch Plate (05 §3.2); the enemy is pushed at T over their mass (a Breaker's 1,000 N on a 130 kg Rigger is 7.7 m/s², over the 5 m/s² foot-hold, off their feet) and the container takes 0.5 m/s² the other way, or nothing while it is under its break-free.
- **Hang from it:** a Pendulum or Gallows anchor from a top corner casting, for any class, whether the container is loose, stacked (500 N), or chained (800 N): break-free is tested against active pull only (Haul, Jack, the Kick), and a Hang line's static load is carried by the container's placement (05 §6.2). A Hang to a loose container on a floor brings it along as dead weight: at 2,000 kg the player hangs from it and it does not move. Hang an enemy to a container on an edge and tip it with another force: the Anchor Drop.
- **Object ↔ object:** a container onto a Medium prop fractures or flattens it. A container onto a volatile triggers it (impact well over 8 m/s after any drop over 3.3 m). Two containers meeting at over 3 m/s dent both.
- **Object ↔ kinematic:** a Hauler hits a container at full relative speed and shoves it along the trench as a wall. A Hauler passing through a line that holds a container severs it: the snap, 150 N·s per 1,000 N recoil to both ends, both Dogs release; lines attached to the Hauler itself are not severed (05 §3.5). The Gantry magnet carries HTR containers; the drop is scripted, and the carried container severs any Gallows line it passes through.
- **Water:** does not float in the Hold's seep water (under 1 m). A loose container feels the flood force volume at 100 / 2,000 = 5% of its authored acceleration (force volumes are authored on a 100 kg reference body and capped at 2 × a_ref per body, 05 §6.6); stacked or chained containers feel nothing until freed. Spilled Light contents feel the full authored acceleration, capped at 2 × a_ref.
- **Updraft and Pulse (Anchor Rim only):** force volumes act on players and free bodies only, including objects held by a player's line (05 §6.6). A loose Rim container, or one Hung free on summed set lines for a Guillotine, feels 5% of the authored acceleration in the Updraft and in the Pulse (0.4 s outward, then 0.6 s inhale toward the Stump, every 75 s from 0:55), so a line-held container swings in a gust; a staged one feels nothing. The per-body cap is 2 × a_ref, which no Heavy object approaches; spilled Light contents hit it.
- **Breathing:** the racks' dead rollers (0.02 start, 0.005 rolling) are why the lean slides them (05 §6.1a): loose containers accelerate at 0.98 m/s² down the 6° lean and stop at rack stops or the Bay edge. Stacked containers hold until the anchor field releases on the lean, then shudder (0.2 s lurch tell) and go.
- **Chain reactions:** the Avalanche takes catwalks (2,500 N·s each) and anything in the fall line.

## States

| State | Trigger | Visual | Physics |
|---|---|---|---|
| Intact | authored | Clean panels, doors shut | As above |
| Dented | one impact 3,000–6,000 N·s | Panel dent mesh swap on the struck face, paint chips | None |
| Broken (doors blown) | impact over 6,000 N·s | Doors swing open on hinges, lock bars bent, 8 to 14 Light debris pieces spill | Doors are hinged bodies for 10 s, then detach as Light props |
| Destroyed | not reachable | Containers never fracture. A container in the Well is removed. | Removed |

## VFX and SFX hooks

- Impact: `SFX_Impact_Container_Steel` (on steel), `_Concrete`, `_Wood`; pitch down with impulse. Over 6,000 N·s: `SFX_Container_DoorsBlow`.
- Break-free: `SFX_Lurch_Steel_H` plus `NS_Dust_Lurch` from the contact edge, 0.2 s before motion.
- Sliding on the lean: looped `SFX_Scrape_Steel_Loop`, volume by speed.
- Niagara: `NS_Impact_Steel_H` (sparks and dust), `NS_Debris_Spill` (contents), `NS_Dust_Avalanche` (per stack, triggered by the bottom unit's release).
- Emissive: none. Dog seat decal is a 5 cm ring.

## Technical

- LOD0 16,000 tris; LOD1 8,000; LOD2 3,000; LOD3 600. Nanite on. Doors are separate static meshes.
- Textures: 2 × 2K (body, doors and castings) plus `T_Trim_Steel_01` and the stencil atlas as decals.
- Pivot: bottom center of the footprint. X+ toward the door end. Z+ up.
- Dog sockets (all `Metal`): `SKT_Dog_Top_01`, `SKT_Dog_Top_02`, `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01`, `SKT_Dog_End_01` (blank end), `SKT_Dog_Door_01`, `SKT_Dog_Door_02`, `SKT_Dog_Hook_01` through `_04` (top corner castings). Door sockets apply force to the door body once blown.
- Chaos: not a geometry collection. `PHYS_Container20` single box. Dent states are three authored mesh swaps. Door hinges are physics constraints with a 6,000 N·s break impulse (**PROPOSED** mapping, 1 unit = 1 N·s). Anchor field `FS_Anchor_Container` releases at 500 N of line force (the stack break-free) or on the arena's lean release; chained units on the Ballast racks release at 800 N. Damage propagation off.
- Replication: RC-H, server-authoritative, 15 Hz, wakes on line force, blast, or contact; wake replicated as an event. Spilled debris goes RC-L.

## Reference search terms

20 ft ISO shipping container corrugated, container corner casting close-up, rusted shipping container door seals, container stack port friction scrape marks, shipping container door lock bar handle wear, intermodal container stencil codes, derelict container yard overgrown.
