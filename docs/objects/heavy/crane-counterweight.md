# Crane counterweight

Rated load. Rated tier (over 3,000 kg), 6,000 kg (**PROPOSED**): five cast slabs of 1,200 kg on a steel tray. Stack 200 × 100 × 125 cm, tray 220 × 120 × 15 cm (**PROPOSED**). Appears on the Anchor (the Rim, in the ballast trays of the Rim cranes, with the Well two jib-lengths inward) and in the Yard (the Shear Press counterweight at the pit's edge, and the Gantry's tail ballast forty meters up on the Rails).

## Role in play

The crane counterweight is the heaviest thing a player can put a line on. Only a Breaker's 1,000 N line, a Doubler at 1,200 N overspin, or lines summing to 1,000 N on one axis (a Rigger's 600 plus a Scrapper's 400) move it (05 §3.3: rated loads are static to any pull under 1,000 N), and when it moves the crane goes with it. The Anchor Drop with a crane counterweight is a named signature play on the Anchor, and it is a crew play: someone Hangs the enemy to the slabs, and a Breaker pulls the slabs off the tray.

| Rig | Use |
|---|---|
| The Anchor Drop | Dog A enemy, Dog B counterweight, Hang. A Breaker Hauls the counterweight off its tray and over the Rim edge. The enemy follows into the Well unless they cut. |
| Rated consequence | The jib drops. On the Anchor, into the Scaffold or over the Rim. In the Yard, the press ram or the Gantry tail. |
| Overboard | A static anchor behind you that nothing lighter than a Breaker can shift. The Jack against the enemy is a T push, no Kick (05 §3.2). A light class Overboarding a Breaker moves themselves, not the Breaker: a Scrapper's 400 N on 220 kg is 1.8 m/s², under the Breaker's foot-hold, and that is intended (05 CHANGELOG item 49). |
| The Gallows | Hang an enemy from a slab loop or the jib above, then hoist by Reel (hold Haul within Hang, 1.0 m/s; 05 §3.2); the slabs are the most solid thing on the Rim and a Hang never tests their rated break-free (05 §6.2). |

- **Scrapper (400 N):** cannot move it alone; with a Rigger pulling on the same axis the crew reaches 1,000 N and it goes. Uses it as the one anchor the Pulse will not slide. Dogs enemies to it.
- **Rigger (600 N):** Hangs an enemy to it on a set line, Hangs themselves to it against the Pulse, and lets the Breaker finish. Anchors Clotheslines to it.
- **Breaker (1,000 N):** the mover. Their 1,000 N held 0.6 s on the authored axis is a commit trigger, not a physics pull (05 §6.3): after the 1.0 s groan and the 0.6 s hold the scripted timeline carries the whole 6,000 kg stack off the tray regardless of friction or mass. The authored slide takes about 5 s to the tray edge (**PROPOSED**); that slow slide is intended, it is the window in which the Breaker can be interrupted, and letting go before 0.6 s resets it. No brace needed against their own line (4.5 m/s², under their 5 m/s² foot-hold); against a Doubler's overspin (5.5 m/s²) they brace (20 m/s²).
- **Climber (400 N):** a perch and a Slingshot wall. Jacking off a rated load gives the player the whole of V: 18 m/s. In the Updraft that carries a Climber toward the Crown.
- **Doubler (800 N, 1,200 N overspin):** base 800 N does not reach it. Overspin at 1,200 N moves it from 1.0 s to the snap at 1.8 s, about 0.06 m of slide; then the Doubler is launched at 10 m/s away from the Rim edge and the slabs keep whatever motion they had plus the snap's 1,500 N·s, 0.25 m/s on the stack.

## Appearance

Silhouette: a short stack of thick concrete slabs with cast-in lifting loops on top, sitting in a shallow steel tray at the crane's tail, taped yellow-and-black on every tray edge.

| Part | Material | Base color | Rough / Metal |
|---|---|---|---|
| Slabs | `MI_Concrete_Cast` | `#8E8B84` | 0.85–0.95 / 0 |
| Slab staining and bird lime (Anchor) | `MI_Concrete_Stained` | `#6F6C66`, `#D8D4C8` | 0.88–0.98 / 0 |
| Tray and retainers | `MI_Steel_Painted_Worn` | claim holder color | 0.45–0.70 / 0 paint, 1 chips |
| Rated tape | `MI_Tape_Rated` | `#F2C230` / `#141414` | 0.50–0.60 / 0 |
| Lifting loops | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 / 1 |
| Rust under the tray lip | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 / 0.25 |

Wear story. Each slab carries a cast mass mark, `1200`, and a pour date. The top slab is chalked with the crane's total and a Ledger number. Lifting loops are polished where shackles ride. The tray's retainers are scraped where slabs were dragged in. Under the tape, the Office's white `RATED LOAD` stencil. On the Anchor the slabs wear pale concrete dust `#B8B2A4`, salt, and bird lime streaks on the upwind face. In the Yard they are oil-black on top from cutting dust. The claim holder's colors go on the tray: 04 gives the Anchor's Rim cranes to the Claim winner, so the tray is `MI_HTR_Orange_PowderCoat` `#E8620C` or `MI_Guild_Ochre` `#C8912A` or whichever faction holds it. Under the Rim's open sky the tape is the brightest thing on the stack; it is matte and never reads as a line.

## Variants

| Variant | Where | Notes |
|---|---|---|
| Rim crane, Guild | Anchor | Ochre tray, hitch glyph. Default when no Claim is held. |
| Rim crane, HTR | Anchor | Orange tray, `HTR-RC-####`. |
| Rim crane, other holder | Anchor | Ascendancy white tray with silver tape; Hanger painted tray; Fray black tray, serials filed. |
| Press counterweight | Yard | Seven slabs, 8,400 kg (**PROPOSED**), tray bolted to the press frame. Consequence: the ram stalls mid-stroke and the pit stays open. |
| Gantry tail ballast | Yard, the Rails | Five slabs on the Gantry's tail. Kinematic while the Gantry moves. Consequence: the Gantry drops its load early wherever it is. |
| Slid | State: top slabs off the tray, jib dropped. |

## Physics

| Property | Value |
|---|---|
| Tier / mass | Rated, 6,000 kg (**PROPOSED**); five slabs at 1,200 kg |
| Collision | Five boxes 200 × 100 × 25 cm stacked; tray box 220 × 120 × 15 cm. |
| Center of mass | Center of the stack, 77 cm above the tray base. |
| Friction | Tray: 0.3 steel on steel (**PROPOSED**), irrelevant to the release, which is a scripted timeline after the commit trigger (05 §6.3). Slab on slab 0.7. Slabs on concrete after a fall: 0.6, fixed scenery that breaks nothing further. |
| Restitution | 0.02 |
| Break-free | Rated, 1,000 N held 0.6 s on the authored axis: a commit trigger (05 §6.3). Static to any pull under 1,000 N. Breaker; Doubler at overspin; or lines summing to 1,000 N on one axis (Rigger 600 + Scrapper 400). |
| Fracture | None. Slabs are non-fracturing. |

Line acceleration (T / m): 1,000 N 0.17 m/s²; 1,200 N 0.20 m/s². Under 1,000 N: zero. Those are reference figures the stack never uses: once 1,000 N is held 0.6 s on the authored axis the release is a scripted timeline (05 §6.3), and the whole stack leaves the tray at the authored speed, building to about 0.4 m/s at the tray edge over about 5 s (**PROPOSED**), twice its 0.20 m/s Stall speed. A single slab, once separated, is 1,200 kg and Heavy, so any line moves it: 400 N 0.33 m/s², 600 N 0.50, 800 N 0.67, 1,000 N 0.83, 1,200 N 1.00; a Scrapper gets a loose slab to 3.6 m/s at the end of a 20 m line. The Yard's press counterweight (8,400 kg) moves at 0.12 m/s² under 1,000 N.

Against a player (6,000 kg × relative speed): stagger 300 N·s at 0.05 m/s; flip 600 at 0.10 m/s; Stall 1,200 at 0.20 m/s. At 2 m/s: 12,000 N·s. From a 7 m drop (11.7 m/s): 70,200 N·s. Off the Gantry tail at 40 m (28 m/s, under the 50 m/s hard velocity cap on everything, 05 §6.3): 168,000 N·s. A single slab (1,200 kg): Stall at 1.0 m/s; at 2 m/s 2,400 N·s; from 7 m 14,040 N·s. Crush: the stack weighs 59 kN, a slab 11.8 kN; either pins a player past 8 kN at 0.5 s. The 8 kN crush threshold is unreachable by any line; it comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1). On the Gantry tail the stack is a kinematic: stagger 4, flip 6, Stall 9 m/s relative, and the Gantry's slow cycle means the kill is the pin against a rail post, not the strike.

## Interactions

- **Haul to it:** the player is pulled to the most solid thing on the map. The slabs do not notice. If a Breaker is already sliding them, the player is dragged with them.
- **Jack from it:** a launch off an immovable wall; a rated load receives nothing and the player takes the whole class V (Climber 18, Scrapper 14, Rigger 12, Doubler 14, Breaker 10 m/s). In the Updraft, a Slingshot off the stack carries toward the Crown. Dog A slab, Dog B enemy, Jack: no Kick with a Dog in a Catch Plate (05 §3.2); the rated stack takes nothing and the enemy takes the whole T over their mass.
- **Hang from it:** the player is anchored, any class: break-free is tested against active pull only (Haul, Jack, the Kick), so a Hang to the rated stack carries the player's static load through the tray without moving a slab (05 §6.2). The Pulse's radial shove does not move a player Hung to the counterweight; the line holds under the Hang load. An enemy Hung to it is set up for the Anchor Drop; the slabs then go over by a Breaker's Haul, not by the Hang.
- **Object ↔ object:** slabs sliding off the tray onto a Rim crane's own cab crush it. Slabs into the Scaffold take a quadrant's worth of planks (800 N·s per section).
- **Object ↔ kinematic:** the Maintenance Ring passing a slab that has fallen onto the Scaffold shoves it inward toward the Well, and severs any line it passes through that is not attached to it (the snap, both Dogs release; 05 §3.5). On the Gantry tail the carried stack does the same to any Gallows line in its path.
- **Water:** sinks. Not present on the Anchor or in the Yard.
- **Updraft:** none on the stack: force volumes act on players and free bodies only, and a rated load in its tray is not free (05 §6.6). A loose slab (1,200 kg) feels 100 / 1,200 = 8% of the authored acceleration, authored on a 100 kg reference body and capped at 2 × a_ref per body.
- **Pulse:** 0.4 s outward then 0.6 s inhale toward the Stump, every 75 s from 0:55. A loose slab on the Rim feels 8% of the authored acceleration each way; the stacked rated load feels nothing (05 §6.6).
- **Breathing:** not present.
- **Floats / slides / rolls:** slides on the tray once a line reaches 1,000 N. Slabs do not roll; they slide and topple over an edge.
- **Chain reactions (rated consequence, PROPOSED):** a Breaker's Haul, a Doubler's overspin, or a crew's lines summed on one axis reach 1,000 N and hold it 0.6 s (the commit trigger, 05 §6.3). After a 1.0 s groan the whole stack slides off the tray on its authored timeline in the Haul direction, about 5 s to the edge. The crane, now tail-light, tips: its jib drops along an authored path into the Scaffold quadrant below it (Anchor) or across the pit lip (Yard). Scaffold sections under the jib fracture at 800 N·s. The slabs continue over the Rim edge into the Well if the Haul direction was inward, or across the Rim if outward. Resets between rounds.

## States

| State | Trigger | Visual | Physics |
|---|---|---|---|
| Intact | authored | Five slabs square in the tray, tape clean | RC-R, anchor field held |
| Dented | not applicable | Chipped slab corners are authored, not a state | None |
| Broken (slid) | 1,000 N held 0.6 s | All five slabs off the tray, jib down, tray retainers bent | Five free slabs at 1,200 kg each, RC-H |
| Destroyed | slabs in the Well | Gone | Removed |

## VFX and SFX hooks

- Impact: `SFX_Impact_Concrete_Massive` on any surface; `SFX_Impact_Concrete_Slab` for single slabs.
- Rated groan: `SFX_Groan_Rated_Crane`, 1.0 s minimum before the first slab moves, audible across the Rim. `NS_Dust_Rated` from the tray edge, `NS_Grit_Slide` under the slabs.
- Jib drop: `SFX_Crane_Jib_Fail`, cable whip layer, `NS_Dust_Collapse` on landing. 5 s siren is not used; this is a player-made consequence and the groan is the tell.
- Line under force on the stack: the line sings at full brightness; the stack adds `SFX_Creak_Rated` below break-free so a Scrapper's wasted line is audibly wasted.
- Emissive: none.

## Technical

- LOD0 24,000 tris; LOD1 12,000; LOD2 4,000; LOD3 800. Nanite on.
- Textures: 2 × 2K (slabs, tray) plus `T_Trim_Steel_01` at 4K and the stencil atlas.
- Pivot: bottom center of the tray footprint. X+ toward the crane's jib. Z+ up.
- Dog sockets, all on taped tray edges and loops: `SKT_Dog_Top_01` (top slab loop, `Metal`), `SKT_Dog_Hook_01`, `SKT_Dog_Hook_02` (lifting loops, `Metal`), `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01` (tray long edges, `Metal`), `SKT_Dog_End_01` (tray tail edge, `Metal`). A Dog on the bare concrete uses the concrete physical material. Per slab once separated: `SKT_Dog_F1_01` through `F5_01` at the loops.
- Chaos: not a geometry collection. `PHYS_CraneCW` five stacked boxes on the tray with anchor field `FS_Anchor_CraneCW`, which releases after 0.6 s of qualifying pull (1,000 N on the authored axis), not instantly. Release after the 1.0 s groan runs the scripted slide, converts each slab to a simulated RC-H body as it leaves the tray, and fires the jib consequence RPC. Damage propagation off. The Gantry tail variant is kinematic on the Gantry's spline until released.
- Replication: RC-R, server-authoritative, 15 Hz, consequence as a server RPC on the groan tell. Separated slabs RC-H.

## Reference search terms

tower crane counterweight slabs, crawler crane counterweight tray, cast concrete counterweight lifting loops, mobile crane ballast blocks stacked, gantry crane tail ballast, concrete slab pour date stamp, crane counterweight chevron hazard tape.
