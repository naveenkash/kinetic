# Billet crate

Heavy prop. Heavy tier, 1,800 kg (five Weave billets at 300 kg in a 300 kg steel cage, **PROPOSED** split). 240 × 120 × 120 cm (**PROPOSED**). Appears in the Yard (the Stacks, stacked three high between the cut hull sections; singles on the Floor beside the Shear Press pit) and on the Anchor Rim as staged Claim cargo. Billets are the processed Weave that every faction wants, so the crate is the Yard's reason to exist.

## Role in play

A billet crate is the Yard's Avalanche body. 03 calls the stacks slow to topple and devastating when they do, and the physics follows: a crate tips over a 1.0 s settle (**PROPOSED**) before it falls, so a player who looks up has time, and a player who does not is under 1,800 kg.

| Rig | Use |
|---|---|
| The Avalanche | Dog A on the bottom crate of a three-high stack, Haul at or above the 500 N stack break-free, then keep pulling: the stack sits on roller pallets (05 §6.1a, item 54), 353 N to start and 88 N rolling, so the 500 N governs and Rigger, Doubler, and Breaker drag the bottom crate out at 0.28, 0.40, and 0.51 m/s². Climber and Scrapper cannot. The two above come down into the canyon. |
| The Guillotine | A single crate Hung free from the Gantry rails on set lines, lifted into position by Reel (hold Haul within Hang, 1.0 m/s), then dropped by gravity when an owner recalls or cuts their set line; a line from crate to enemy only steers it (05 §10). Its 17.7 kN weight exceeds every single Hang load; it takes two Riggers' lines summed (24 kN) per 05 §10 and §6.2a. |
| The Anchor Drop | Enemy Hung to a crate on the lip of the Shear Press pit. Tip it in. |
| Overboard | A crate is the solid anchor behind you. A Jack against a player is a T push, no Kick (05 §3.2): the enemy skids at T over their mass. A light class Overboarding a Breaker moves themselves, not the Breaker: a Scrapper's 400 N on 220 kg is 1.8 m/s², under the Breaker's foot-hold, and that is intended (05 CHANGELOG item 49). |

- **Scrapper (400 N):** cannot start a stacked crate (500 N). Uses a single crate as cover and as an Overboard anchor. Dogs a spilled billet (300 kg, the bottom of Heavy, so no 30% bonus) and Hauls it at 1.3 m/s²: 7.3 m/s at the end of a 20 m line, 2,190 N·s into a shin, a Stall.
- **Rigger (600 N):** passes the 500 N stack break-free (the lurch) and drags the bottom crate out on its roller pallet at 0.28 m/s² net; it passes its 0.67 m/s Stall speed after 80 cm. Pendulums from a stacked crate at will: a Hang is not an active pull and never tests the 500 N break-free (05 §6.2). Hangs a crate across a canyon mouth as a barricade only while the crate's weight (17.7 kN) stays on the floor; hung free it needs two Riggers' Hang loads summed (05 §10, §6.2a).
- **Breaker (1,000 N):** the Avalanche: 0.56 m/s² frictionless, 0.51 m/s² net of rolling, out from under the stack in under two seconds. The Yard is HTR's map and this is their play.
- **Climber (400 N):** perches on the top crate, Slingshots off the cage and keeps nearly all of their 18 m/s. Cannot start a stacked crate.
- **Doubler (800 N, 1,200 N overspin):** starts a stack on a single line at 0.44 m/s²; a split line at 480 N does not, at 720 N overspin it does. A full overspin snap delivers 1,500 N·s: 0.83 m/s on the crate, over its 0.67 m/s Stall speed.

## Appearance

Silhouette: a rectangular steel cage of 50 mm angle and 8 mm flat bar, open on top with two removable top rails, forklift pockets in the base, four lifting lugs at the top corners. Inside, five billets: dark, dense, faintly iridescent bars 220 × 20 × 20 cm, strapped with two steel bands each.

| Part | Material | Base color | Rough / Metal |
|---|---|---|---|
| Cage angle and bar | `MI_Steel_Painted_Worn` | Guild grey `#5B6670` or HTR grey `#4A4D50` | 0.45–0.70 / 0 paint, 1 chips |
| Lifting lugs, fork pockets | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 / 1 |
| Billets | `MI_Weave` | `#1A2430` | 0.18–0.28 / 1, anisotropy 0.8, grazing sheen `#3A6E7F` |
| Banding | `MI_Steel_Galvanized` | `#9EA3A6` | 0.35–0.55 / 1 |
| Rust at welds | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 / 0.25 |

Wear story. The cage is Guild plant, welded in the Yard and repainted by hand; brush strokes read in the roughness. Lug holes are polished bright by shackles. Fork pockets are scraped to bare steel. The billets are never dirty: Weave does not hold grime, so the crate reads as a dull box holding something clean. Stencils: the Guild hitch-knot glyph and `LG-####` on one long side, a chalk tally of billet count on the other, the Office's `SITE PROPERTY` line in white. Under Yard floodlights the billets catch a single sharp highlight along the lattice direction. Albedo is kept dark so a singing line is the brightest thing in frame; the Weave sheen is a highlight, not an emissive.

No rated tape. The Yard's hull sections carry yellow-and-black tape; billet crates do not, so players read that any line moves a crate, slowly.

## Variants

| Variant | Mesh | Livery | Notes |
|---|---|---|---|
| Guild full | `SM_Heavy_BilletCrate_Guild` | grey, ochre glyph | Default. Five billets. |
| HTR full | `SM_Heavy_BilletCrate_HTR` | `MI_HTR_Grey_Matte`, orange band, `HTR-RC-####` | Yard sectors HTR has cut. |
| Half load | `SM_Heavy_BilletCrate_Half` | either | Two billets. **PROPOSED** 900 kg. Reads lighter: visibly empty top. |
| Empty cage | `SM_Heavy_BilletCrate_Empty` | either | 300 kg, Medium tier (**PROPOSED**). Anchor Rim only. |
| Broken | state mesh | either | Cage split, billets spilled. |

## Physics

| Property | Value |
|---|---|
| Tier / mass | Heavy, 1,800 kg |
| Collision | Box 240 × 120 × 120 cm. Billets as five boxes 220 × 20 × 20 cm once spilled. |
| Center of mass | 45 cm above the base, centered (billets sit low). |
| Friction | Roller pallets under every Avalanche stack (05 §6.1a, item 54): 0.02 to start, 0.005 rolling. Start force 0.02 × 17.7 kN = 353 N, under the 500 N stack break-free, so the 500 N governs: Rigger, Doubler, Breaker; not Climber or Scrapper. Rolling cost 88 N. A crate set directly on the Floor (0.55 on concrete) is fixed scenery that breaks; no line moves it (05 §6.1a). |
| Restitution | 0.05 |
| Break-free | Loose 0 N. In a stack 500 N: Rigger, Doubler, Breaker; not Climber or Scrapper at 400 N. Chained to a Gantry pallet 800 N: Doubler, Breaker. |
| Fracture | **PROPOSED** 5,000 N·s. Cage splits along the top rails; billets spill as Heavy props at 300 kg each. |
| Settle | **PROPOSED** 1.0 s angular damping ramp after break-free, matching the 1.0 s structural groan lead. |

Line acceleration (T / m): 400 N 0.22 m/s²; 600 N 0.33; 800 N 0.44; 1,000 N 0.56; 1,200 N 0.67. With v = sqrt(2 a d), a Breaker's 1,000 N gets a loose crate to 1.5 m/s after 2 m and 4.7 m/s at the end of a 20 m line. Net of the 88 N rolling cost ((T minus 88) / 1,800): Rigger 0.28 m/s², Doubler 0.40, Breaker 0.51, Doubler at overspin 0.62. Under a Breaker the crate reaches its 0.67 m/s Stall speed after 44 cm, under a Rigger after 80 cm, so Yard crates kill both ways: the dragged bottom crate at a walking pace and the two above it by toppling. A separated billet (300 kg): 400 N 1.3 m/s², 1,000 N 3.3 m/s².

Against a player (1,800 kg × relative speed): stagger 300 N·s at 0.17 m/s; flip 600 at 0.33 m/s; Stall 1,200 at 0.67 m/s. At 2 m/s: 3,600 N·s. From a 7 m drop (11.7 m/s): 21,060 N·s. At the Haul cap (25 m/s): 45,000 N·s. Off the Gantry rails at 40 m (28 m/s, under the 50 m/s hard velocity cap on everything, 05 §6.3): 50,400 N·s. A player's body counts against the fracture threshold: a Breaker flipped into a crate at 10 m/s is 2,200 N·s, a dent. A single spilled billet (300 kg) Stalls at 4.0 m/s; falling from the top of a stack (3.6 m, 8.4 m/s) it delivers 2,520 N·s. Crush: the crate's weight is 17.7 kN, over the 8 kN for 0.5 s threshold; a pin is a Stall. The 8 kN crush threshold is unreachable by any line; it comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1).

## Interactions

- **Haul to it:** the player is pulled to the cage. From below a stack, a Haul does not lift them (only a Breaker's 1,000 N lifts a 90 kg Climber outright); they Hang and climb by Reel at 1.0 m/s (05 §3.2). If the crate is falling, they are dragged with it.
- **Jack from it:** a launch wall. Jack splits V by inverse mass: a Breaker (V 10) sends a loose crate off at 1.1 m/s and takes 8.9 m/s; a Climber (V 18) sends it 0.8 m/s and takes 17.2. A stacked crate that does not break free gives the player the whole of V. Dog A crate, Dog B enemy, Jack: no Kick with a Dog in a Catch Plate (05 §3.2); the enemy skids at T over their mass and the crate takes T / 1,800 kg, or nothing under its break-free.
- **Hang from it:** a Pendulum or Gallows anchor from a top lug, for any class, stacked or loose: break-free is tested against active pull only (Haul, Jack, the Kick), so a Hang never unseats a stacked crate (05 §6.2). A Hang to a loose crate on the Floor is dead weight at 1,800 kg: the player hangs from it, it does not move. Hang an enemy to a crate on the pit lip and push the crate with another force: the Anchor Drop into the Shear Press pit.
- **Object ↔ object:** a crate onto a Medium prop flattens it. A crate onto a fuel drum triggers it (needs 10 m/s, so a drop over 5.1 m). Crate onto crate at over 2.8 m/s splits the lower one.
- **Object ↔ kinematic:** Haulers shove a crate along the trench as a wall and do not derail, and sever any line they pass through that is not attached to them (the snap, 150 N·s per 1,000 N recoil, both Dogs release; 05 §3.5). The Gantry magnet lifts HTR crates by the cage; the carried crate severs any Gallows line in its path; the drop is scripted and lands at 11 to 14 m/s.
- **Water:** no water in the Yard. Elsewhere it sinks.
- **Updraft and Pulse (Anchor Rim only):** force volumes act on free bodies only, which includes objects held by a player's line, authored on a 100 kg reference body and capped at 2 × a_ref per body (05 §6.6): a loose Rim crate, or one Hung free on summed set lines for a Guillotine, feels 100 / 1,800 = 5.6% of the authored acceleration in the Updraft and the Pulse (0.4 s outward, 0.6 s inhale toward the Stump, every 75 s from 0:55), so a line-held crate swings in a gust; a staged crate feels nothing. Spilled billets (300 kg) feel 33%; Light banding debris feels the full value, capped at 2 × a_ref.
- **Breathing:** not present in Ballast.
- **Floats / slides / rolls:** slides. Does not roll; the box ratio is too square to tumble without a drop.
- **Chain reactions:** a toppled stack crosses a trench ahead of a Hauler and makes a wall; the Hauler shoves the wall; players between are crushed.

## States

| State | Trigger | Visual | Physics |
|---|---|---|---|
| Intact | authored | Cage square, billets banded | As above |
| Dented | impact 2,500–5,000 N·s | Angle bowed on the struck side, one band snapped and curling | None |
| Broken | impact over 5,000 N·s | Top rails off, one long side folded open, billets slide out | Cage 300 kg Medium, billets 5 × 300 kg Heavy, 10 s Light debris: banding, chalk |
| Destroyed | in the Shear Press | Press ram crushes it | Removed on the ram stroke |

## VFX and SFX hooks

- Impact: `SFX_Impact_Cage_Steel`, `_Concrete`. Billet on steel: `SFX_Impact_Weave`, a low glassy clack, unique to Weave.
- Break-free: `SFX_Lurch_Steel_H` with `SFX_Groan_Stack` during the 1.0 s settle. `NS_Dust_Lurch` from the base.
- Fracture: `SFX_Cage_Split`, `NS_Impact_Steel_H`, `NS_Debris_Banding`.
- Weave: `NS_Weave_Sheen` camera-facing glint on billets at LOD0 only.
- Emissive: none.

## Technical

- LOD0 16,000 tris; LOD1 8,000; LOD2 3,000; LOD3 600. Nanite on for the intact static mesh, off for the geometry collection.
- Textures: 2 × 2K (cage, billets) plus `T_Trim_Steel_01` and the stencil atlas.
- Pivot: bottom center of the footprint. X+ along the long axis. Z+ up.
- Dog sockets: `SKT_Dog_Top_01`, `SKT_Dog_Top_02` (top rails, `Metal`), `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01`, `SKT_Dog_End_01`, `SKT_Dog_End_02` (`Metal`), `SKT_Dog_Hook_01` through `_04` (lugs, `Metal`), `SKT_Dog_Load_01` through `_03` (billets, `Metal`; a Dog here pulls a billet out once the cage is broken). Fragments: `SKT_Dog_F1_01` cage, `SKT_Dog_F2_01` through `F6_01` billets.
- Chaos: `GC_BilletCrate_Full`. L0 whole, damage threshold 5,000 (**PROPOSED** 1 unit = 1 N·s). L1: cage (1 cluster), billets (5 clusters), threshold 2,000. L2: banding and chalk debris, threshold 300, `RemoveOnMaxSleep` 10 s. Anchor field `FS_Anchor_BilletCrate` releases at 500 N of line force (stack break-free); Gantry-chained units release at 800 N. Damage propagation off so a toppling crate falls as one piece and splits on landing.
- Replication: RC-H, 15 Hz, wakes on line, blast, or contact. Fracture under RC-GC; billets stay RC-H.

## Reference search terms

steel stillage cage pallet, scrap metal stillage forklift pockets, carbon fiber billet bar stock, graphite block dark iridescent, steel banding strap snapped, shipyard steel yard stacked stillages, lifting lug shackle wear.
