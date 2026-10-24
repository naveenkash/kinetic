# Water barrel

Medium prop. Mass tier Medium full at 220 kg; Light empty at 25 kg. 58 cm diameter × 94 cm tall (195 L blue HDPE drum with a screw bung and a lifting rim). Appears in Hangtown (the Low Hang garden terraces, the Mid Hang beside every third house), the Yard (the Floor, at the cutting stations as quench water), and Ballast (the Hold, loose in the seep water).

## Role in play

An object that changes mass tier when it breaks. 05 §6.3 recomputes tier whenever mass changes: full, it is a Medium that Stalls a player at 5.5 m/s; split, it is a Light shell that cannot Stall anyone on a Haul and a wet patch. Prep crews stage them as quench water and fire cover, which is why they sit next to welding carts and fuel drums.

| Rig | Use |
|---|---|
| The Kiss | Haul a full barrel into a propane cylinder. A Scrapper's full line brings it in at 8.5 m/s, just over the cylinder's 8 m/s trigger. |
| The Guillotine | Hang a full barrel overhead (2.2 kN, within every class Hang load), Reel it up at 1.0 m/s, then recall or cut your own set line. From 4 m it lands at 8.9 m/s, 1,958 N·s, a Stall. The cheapest Medium Guillotine on Hangtown. |
| The Caboose | A barrel on a cart is a 340 kg train. |
| The Anchor Drop | A barrel at a terrace edge is a Heavy-feeling anchor for a Light enemy. |

- **Scrapper:** the first object a Scrapper reaches for on Hangtown. Full, 220 kg, no 30% bonus. Empty, 25 kg Light, the bonus applies and the shell arrives at the 25 m/s cap.
- **Rigger:** hangs one over a swing bridge landing and waits.
- **Breaker:** a barrel is a tool for triggering volatiles at range without walking to them. 1,000 N on 220 kg is 4.5 m/s², 13.4 m/s over 20 m.
- **Climber:** stands on it. Jacks off it: a Climber keeps 12.8 m/s of their 18 m/s V off a full barrel.
- **Doubler:** two barrels, one line each, Haul both into an enemy. 480 N per line moves each at 2.2 m/s²; slow, and the enemy sees both coming.

## Appearance

Silhouette: a ribbed plastic drum, two raised rolling hoops at the third points, a flat top with a bung and a vent cap, a molded lifting rim. Reads as a soft-edged cylinder.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Drum | `MI_HDPE_Blue` | `#1F4FA3` | 0.40–0.55 | 0 |
| Bung, vent cap | `MI_HDPE_Black` | `#232426` | 0.45–0.60 | 0 |
| Lifting rim | `MI_HDPE_Blue` at 0.3 roughness (polished by hands) | | | |
| Stencils | `MI_Decal_Stencil` | `#E6E4DD` | inherits | 0 |
| Lashings | `MI_Rope_Sisal` | `#B59D6E` | 0.90–1.00 | 0 |

Subsurface 0.15 on the drum so the water level reads as a darker band in backlight. Wear: rolling hoops scuffed white. Sun-chalked top. Mud ring at the base. Hangtown drums have a hand-painted number and a drip line of algae from the vent.

Text: `OFFICE OF THE LAY / QUENCH / NOT POTABLE` in white on Yard drums. Hangtown drums are numbered `H-14` in `#D9A441`. Ballast drums carry the Meridian cargo code `C5-H2O-07` at 0.9 wear (**PROPOSED** code format).

Lighting: blue plastic goes near-black in Ballast's Hold and reads only on the specular. On Hangtown terraces under Office lights it is the most saturated object on the tier. Line glow refracts through the rim.

## Variants

| Variant | Difference |
|---|---|
| Full | 220 kg. Default. Water level 90%. |
| Half | 120 kg. Water level 45%. Sloshes visibly on tilt. (**PROPOSED** mass) |
| Empty | 25 kg, Light tier. Bung off. Rings hollow. |
| Split | Empty shell with a 40 cm crack, 23 kg Light. Spawned by fracture. |
| Meridian | Grey `#8A8F94` HDPE, cyan hexagon, Ballast only. |
| Lashed | Rope-lashed to a terrace post. Break-free 800 N. |

## Physics

- Tier Medium full, Light empty. Tier is recomputed on every mass change (05 §6.3).
- Mass: 220 kg full, 120 kg half, 25 kg empty, 23 kg split.
- Collision: cylinder 58 cm diameter × 94 cm. Hoops add 1 cm radius; use a single cylinder.
- Center of mass: full, 42 cm above the base on the axis. Empty, 47 cm. Tips at 31° full. (**PROPOSED**)
- Friction 0.45 plastic on steel, 0.5 on concrete, 0.55 on wood. Restitution 0.25 full, 0.5 empty. Rolls on its side with rolling resistance 0.03. (**PROPOSED**)
- Break-free (05 §6.2): loose 0 N, anyone; in a cradle or on a cart 200 N, anyone; lashed 800 N, Doubler and Breaker only. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Fracture: splits at 1,000 N·s. Fragments: one Light shell at 23 kg and 195 L of water. (**PROPOSED**) Water is a 2 m radius decal and a 1.5 s, 1 m radius puddle that lowers friction to 0.2. No force volume; that belongs to the water tank. A flipped 220 kg Breaker landing on it at 5 m/s is 1,100 N·s and does not split it; at 8 m/s it does.

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d)), full barrel:

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 1.8 m/s² | 50 m | 13.4 m/s | 2,948 N·s |
| Scrapper | 400 N | 1.8 m/s² | 20 m | 8.5 m/s | 1,870 N·s |
| Rigger | 600 N | 2.7 m/s² | 40 m | 14.7 m/s | 3,234 N·s |
| Doubler | 800 N | 3.6 m/s² | 30 m | 14.7 m/s | 3,234 N·s |
| Doubler overspin | 1,200 N | 5.5 m/s² | 30 m | 18.2 m/s | 4,004 N·s |
| Breaker | 1,000 N | 4.5 m/s² | 20 m | 13.4 m/s | 2,948 N·s |

Empty barrel (25 kg): 400 N is 16 m/s²; every class reaches the 25 m/s cap inside its line and the Scrapper bonus cannot raise it. 25 kg × 25 m/s is 625 N·s, a flip.

Impact on a player, full barrel (impulse = 220 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 1.4 m/s | 300 N·s | Stagger |
| 2.0 m/s (walking) | 440 N·s | Stagger |
| 2.7 m/s | 600 N·s | Flip |
| 5.5 m/s | 1,200 N·s | Stall; a Scrapper needs 8.4 m of run-up, a Breaker 3.4 m |
| 6.8 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall |
| 11.7 m/s (7 m drop) | 2,574 N·s | Stall; the barrel splits on this hit |
| 18.2 m/s (fastest Haul arrival, overspin) | 4,004 N·s | Stall; splits |

Empty barrel: 300 N·s needs 12 m/s (stagger), 600 N·s needs 24 m/s (flip). A Haul cannot Stall with it. A Doubler Snap puts 1,500 N·s into the shell: it leaves at 60 m/s, capped at the 50 m/s terminal velocity, and carries 1,250 N·s, a Stall. A Jack to the 30 m/s cap is 750 N·s, a flip.

Jack (05 §3.2 v0.2), full barrel: is Kicked to V × M / (M + m) within 0.3 s (the Kick), then T / 220 kg. Climber (V 18): barrel 5.2 m/s, Climber 12.8 back. Scrapper (V 14): barrel 4.7, Scrapper 9.3. Rigger (V 12): barrel 4.5, Rigger 7.5. Doubler (V 14): barrel 5.2, Doubler 8.8. Breaker (V 10): barrel 5.0, Breaker 5.0. Empty barrel: Climber 14.1 m/s to the barrel and 3.9 back; Scrapper 11.4 and 2.6; the shell then accelerates at 16 m/s² to the 30 m/s cap within 20 m.

Crush: no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). A full barrel weighs 2.2 kN. Only a kinematic crushes with it.

## Interactions

- **Haul to it:** full, the barrel is heavier than every harness. 600 N on a 130 kg Rigger is 4.6 m/s², under the 5 m/s² foot-hold: the Rigger skids and the barrel tips, then slides in at 2.7 m/s². A Climber at 4.4 m/s² is airborne and does most of the closing.
- **Jack from it:** full, a good launch: the split above gives a Climber 12.8 m/s. Empty, the shell flies and the player keeps under 4 m/s.
- **Hang from it:** a Hang to a cradled or lashed barrel does not unseat it; the cradle carries the load up to the class Hang load. A Hang to a loose barrel brings it along as 220 kg (2.2 kN) of dead weight: a Climber (Hang load 4 kN) can hold one, and it pulls them to the floor.
- **Object to object:** a full barrel rolling into a stacked crate at 3 m/s does nothing. Into a market stall it breaks the stall (660 N·s).
- **Kinematic:** a Hauler strike splits it and throws the shell.
- **Water:** full, it floats awash (bulk density about 890 kg/m³). Empty, it floats high and bobs. In the Hold flood it is carried toward the nose at 0.45× the volume's authored acceleration (a_ref × min(100 / m, 2)); a racked barrel stays put until freed.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A cradled or lashed barrel is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose full barrel feels 0.45× the authored acceleration (a_ref × min(100 / m, 2)) and slides on the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55). A loose empty shell feels 2× (cap) and is lifted near the Stump, tumbled outward, then drawn back on the inhale.
- **Breathing:** upright barrels tip at 31°, so 6° only sloshes them. On their side they roll downhill every half cycle.
- **Cut recoil:** 150 N·s per 1,000 N. Under 1 m/s on a full barrel; 6 m/s on an empty shell, which rolls away when its line is cut.
- Rolls on its side, slides on its base, floats.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Blue, bung in. | 220 kg Medium. |
| Dented | Two creases, vent cap gone, drips. | No change. Water level falls 5% per 30 s. (**PROPOSED**) |
| Broken | Split shell, puddle, decal. | 23 kg Light. Friction 0.2 in the puddle for 1.5 s. |
| Destroyed | Shell despawns after 10 s if untouched. | |

## VFX and SFX hooks

- Impact: `SFX_Drum_Plastic_Full_Steel`, `_Concrete`, `_Wood` (a dull thud with slosh), `SFX_Drum_Plastic_Empty_*` (hollow boom). `SFX_Drum_Splash` into water.
- Roll: `SFX_Drum_Roll_Steel`, `_Concrete`, `_Planks` with a slosh layer scaled by fill.
- Break-free (lashed): `SFX_BreakFree_Rope`.
- Fracture: `SFX_Drum_Split`, `NS_Water_Burst_M` (195 L sheet and spray), `NS_Puddle_Decal`.
- No emissive. Subsurface fill band only.

## Technical

- LOD0 6,000 tris, LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set; the fill level is a material parameter, not a mesh.
- Pivot: bottom center of the base.
- Dog sockets: `SKT_Dog_Top_01` (top center beside the bung) `Metal` class for the HDPE (Mag Dog treats plastic as metal by project rule, **PROPOSED**); `SKT_Dog_Rim_01`, `_02` (lifting rim, opposite sides); `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01`, `SKT_Dog_SideE_01`, `SKT_Dog_SideW_01` (upper hoop); `SKT_Dog_Bot_01` (base, since it rolls). Fragment: `SKT_Dog_F01_01` on the split shell.
- Geometry collection `GC_WaterBarrel`: L0 whole (damage threshold 1,000), L1 one shell fragment. Mass swap from 220 to 23 at L0 break, with the water spawned as VFX only; tier swaps to Light on the same frame.
- Anchor fields: `FS_Anchor_WaterBarrel_Cradle` releases cradled instances at 200 N; `FS_Anchor_WaterBarrel_Lash` releases lashed instances at 800 N.
- Replication RC-M full and half. Empty and split instances drop to RC-L (Light predicted) with the tier swap.

## Reference search terms

Blue HDPE 200 litre drum; plastic drum rolling hoops scuffed; rain barrel hillside garden; quench water drum welding bay; split plastic drum crack; drum floating in flood water photo.
