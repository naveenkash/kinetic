# Water tank

Heavy. Heavy tier, 2,500 kg full (**PROPOSED** 240 kg steel shell plus 2,260 L of water). 130 cm diameter, 170 cm tall, a vertical cylindrical steel tank on a welded cradle. Appears in Ballast, the Hold (four intact pre-Severance potable tanks, full, in cradles against the nose bulkhead) and Hangtown, the Mid Hang (a smaller rooftop variant on the larger houses, 90 × 110 cm, 780 kg full).

## Role in play

A wall of mass that becomes a flood. Per 05 §6.1a, "Water tanks are fixed: they rupture (6.5, 6.6); they do not move." It stands on a bare deck, not on rollers or skids, so no line and no crew of lines moves it. It does not need to move to kill: when it fractures at 1,500 N·s it releases a force volume that carries players and props toward the nose of the Hold. In Ballast it is the Scrapper's map event: rupture one on the lean toward the nose and ride the rush out while the enemy is carried into the Well.

Why it is there: the climber Tenacity carried potable water for its crew. The tanks survived the fall because the Hold crushed around them. HTR has not cut them because they are full and nobody wants 2 tonnes of water loose in the Hold during a shift.

| Harness | What they do with it |
|---|---|
| Scrapper | Ruptures it: Hauls a 180 kg drum into it (2.2 m/s², 8.3 m/s after 16 m, 1,500 N·s) or Kisses a cylinder against it. Then rides the flood. A line on the tank itself sings and does nothing. |
| Rigger | Hangs a line from the tank's top lug across the Hold at chest height. The tank is the most solid anchor in the tier. |
| Breaker | Cannot move it; nothing can (05 §6.1a). Hauls a 250 kg crate into it at 4 m/s² instead; 6 m/s after 4.5 m is 1,500 N·s. |
| Climber | Jacks off it: the Kick, V 18 split against 2,500 kg, gives the Climber 17.4 m/s. Perches on it. Stays above the water line when it goes. |
| Doubler | Overspins a Haul on a crate next to the tank and snaps at 1.8 s: 1,500 N·s into the crate, which carries it into the tank. |

## Appearance

Silhouette: a vertical drum with a domed top, a manway hatch on the dome, a sight glass down one side, a drain valve at the base, and four cradle feet. Taller than a player. Reads as a tank because of the dome and the sight glass.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Shell | `MI_Steel_Painted_Worn`, Meridian white `#D8D9D4` | | 0.45 to 0.70 | 0 / 1 chipped |
| Cradle, manway ring | `MI_Steel_Bare` | `#5C5F63` | 0.55 | 1 |
| Rust | `MI_Steel_Rusted` | `#7A3F1C` to `#A8562A` | 0.80 to 0.95 | 0.3 |
| Sight glass | `MI_Glass_Lamp` | `#E8EEF2` | 0.08 | 0 |
| Drain valve | `MI_Steel_Bare` brass tint `#A8894A` | | 0.35 | 1 |
| Hangtown shell | `MI_HDPE_Blue` or `MI_Hanger_Paint` | `#1F4FA3` | 0.45 | 0 |

Wear: the Hold tanks are sixty-one years old. Paint survives only on the dome. The shell is streaked with rust from every weld, green-black with algae up to the 40 cm seep line, salt-white above it. Hand polish on the manway dogs where cutters have opened it to check the water. The sight glass shows the level: full, with a brown tide mark.

Markings: `MERIDIAN` hexagon in `MI_Meridian_Cyan` `#2A9FB5` at 0.9 wear on the shell, with the climber system code `TEN-POT-02` and `POTABLE / 2260 L` in faded black. Over it, fresh: `OFFICE OF THE LAY / SITE PROPERTY` in white `#E6E4DD`, 5 cm letters, and a Marshal number. HTR has chalked `DO NOT CUT` on the cradle. In Hangtown the rooftop tank is a blue poly or a painted steel drum with a hanger stencil (a kettle) and a sisal lashing; `MERIDIAN` never appears on it.

Under arena lighting the Hold tanks are the palest large shapes in the darkest tier. No emissive. The sight glass catches a nearby singing line.

## Variants

| Variant | Difference |
|---|---|
| Ballast full (default) | 2,500 kg. Fractures. Flood volume 2,260 L. |
| Ballast empty | 240 kg, Medium (tier recomputes on mass change, 05 §6.3). Rust-through holes. Fractures at 1,500 N·s into shell pieces with no flood. Booms when hit. A Breaker slides it at 4.2 m/s². |
| Hangtown rooftop full | 780 kg, Heavy. 90 × 110 cm. Fixed to the roof per 05 §6.1a; it moves only when the house moves, and rides the house. Fractures at 1,500 N·s. Flood volume 700 L, 3 m, 1 s (**PROPOSED** scaled). |
| Hangtown rooftop empty | 80 kg, Medium, under 100 kg: Scrapper bonus applies. |
| Dented | Shell dished 10 cm on one side. Cosmetic. |

## Physics

Heavy tier, 2,500 kg full. Collision: one cylinder, radius 65 cm, height 170 cm, plus a 20 cm cylinder for the dome. Cradle as a 140 × 140 × 15 cm box. Center of mass 80 cm up when full, 85 cm empty. Surface friction 0.40 wet steel, restitution 0.05 (**PROPOSED**), which matter only for things that hit it.

Fixed scenery per 05 §6.1a: the full tank stands directly on the Hold deck in its welded cradle, not on rollers or skids, so no line moves it, no crew of lines adding as vectors moves it, and no blast moves it. Break-free does not apply; the cradle never releases. A line seated in the tank sings and the tank does nothing. The only thing that changes the tank is damage: it ruptures.

Fracture threshold 1,500 N·s (05 §6.5). Fragments: four Medium shell pieces, 60 kg each, plus the dome (Medium, 40 kg) and the cradle, which stays. Rupture releases a force volume. Player bodies count as impacting objects against the threshold (05 §6.5): a 220 kg Breaker at 6.8 m/s breaks the tank. A Breaker's sprint is 6.0 m/s (1,320 N·s, not enough); a Breaker Hauled into the tank by another Breaker's line (4.5 m/s² on 220 kg) passes 6.8 m/s after 5 m, breaks the tank, and hits static at under 12 m/s, so they survive the hit and ride the flood.

Things that reach 1,500 N·s against the tank: a 250 kg crate at 6 m/s (4.5 m on a Breaker line, 11 m on a Scrapper's); a 180 kg drum at 8.3 m/s (16 m on a Scrapper line, no trigger); a 45 kg cylinder's blast at 0 m (exactly 1,500 N·s); a demolition charge within 2 m; a crate arriving at the 25 m/s Haul cap from any class, 6,250 N·s. A Doubler's full-overspin snap delivers 1,500 N·s to the far end: a snap with the far Dog in the tank ruptures it directly.

Flood volume, **PROPOSED**: on rupture a force volume 6 m long and 3 m wide spawns downslope from the tank, acceleration 8 m/s² at the tank falling to 2 m/s² at 6 m, lifetime 2 s, direction set by the Breathing's current tilt. 8 m/s² exceeds every unbraced foot-hold (Climber 3, Scrapper 4, Rigger 5, Doubler 5, Breaker 5), so everyone inside is airborne and carried; a braced Rigger (9 m/s²) and a braced Breaker (20 m/s²) hold at the tank and are released as the volume weakens past them. The flood is a force volume and acts on players and free bodies only, scaled a_ref × min(100 / m, 2) (05 §6.6): a loose 45 kg cylinder feels 16 m/s² at the tank (the 2× cap) and reaches its 8 m/s trigger after about 2 m if the volume has not weakened past it; a loose 180 kg drum feels 4.4 m/s² and rolls; a loose 250 kg crate feels 3.2 m/s² and slides. A cylinder a player is holding on a Hang line swings with the rush. Racked cylinders, the fixed manifold, and the other tanks are unaffected. Water in the Hold rises 15 cm for the rest of the round over an 8 × 8 m area, to 55 cm: still under the 1 m depth at which blasts halve. Force volumes never Stall directly. The flood kills by delivering players to the nose at up to 6 m/s, where the torn cone is the Well.

The full tank is never a projectile; it is fixed. Its fragments are: a 60 kg shell piece reaches 300 N·s at 5 m/s, 600 at 10 m/s, 1,200 at 20 m/s; a Scrapper moves one at 6.7 m/s² with the under-100 kg bonus, 15 m/s at the end of a 20 m line, 900 N·s, a flip. The 40 kg dome at the 25 m/s Haul cap is 1,000 N·s, a flip. Crush: 24.5 kN of weight, over the 8 kN threshold that no line reaches and that comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1), but a fixed tank never falls on anyone; the case does not arise.

## Interactions

Haul to it: a grapple; the player comes to the tank. The tank does not come to the player. Jack from it: a Slingshot anchor; the Kick gives the player almost all of V (a Breaker 9.2 of 10 m/s) and the tank nothing it can use. Hang from it: the best Pendulum anchor in the Hold; the Rigger's 12 kN Hang load cannot move it, and nothing can. A player hanging from the top lug climbs to the dome by Reel (holding Haul within Hang shortens the rod at 1.0 m/s) and perches above the water line. A Jack from the tank to an enemy's Catch Plate does not Kick (05 §3.2); it is a T push on the enemy only.

Object to object: anything Heavy Hauled into the tank stops. A crate at 6 m/s ruptures it. The Kiss of a cylinder onto a tank: the cylinder's blast at contact is exactly 1,500 N·s, a rupture plus a 4 m blast.

Kinematics: the Breathing hull carries the tank in its cradle; the relative impact is zero, and the tank never leaves the cradle. There is no Gantry, Hauler, or Lift in the Hold. In Hangtown the rooftop tank rides its house: when the house swings or drops, the tank goes with it as part of the house's mass, and a house-to-house collision over 1,500 N·s at the roof ruptures the tank onto whatever is below.

Water: the Hold's seep water is 40 cm deep; the tank sits in it. Full it does not float. Empty, 240 kg in a 2.3 m³ shell, it floats and the water shove on the Breathing carries it toward the nose at the water's speed. Updraft: not present. Breathing tilt: above.

Chain reactions: four tanks against the nose bulkhead, 2 m apart. One flood does not rupture the next (the volume applies acceleration, not impulse, to Heavy bodies). A charge on the bulkhead ruptures all four at once: a 10,000 L rush.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | As above, full sight glass. | Fixed (05 §6.1a). Static collision. |
| Dented | 10 cm dish, sight glass cracked and weeping. After any impact 800 to 1,500 N·s. | Unchanged. |
| Broken (ruptured) | Shell split along a weld, four Medium pieces and the dome on the deck, water sheet 0.5 s. | Flood volume 2 s. Pieces are 60 kg Medium props, 300 N·s at 5 m/s; a Scrapper moves them at 6.7 m/s² with the under-100 kg bonus. |
| Destroyed | Pieces persist for the round. Cradle stays. Deck water 15 cm higher. | Nothing more. |

## VFX and SFX hooks

- Impact: a deep, dull, full-tank thud with no ring; the empty variant booms. Steel on steel scrape when the empty variant slides.
- A line pulling on the fixed tank: the line sings against it with no lurch and no shudder; a faint cradle tick, `SFX_Obj_Fixed_Tick`, tells the puller it is going nowhere.
- Fracture: `NS_Rupture_Tank` (a 2 m water sheet, weld sparks, spray for 2 s), `SFX_Obj_Rupture_Tank`, a weld-tear crack then a 3 s rush.
- Flood: `NS_Flood_Rush_Hold` (surface foam front, debris, mist), `SFX_FV_Flood`, a rising rush with a 1 s lead (the force volume tell in 05 §11).
- No emissive.

## Technical

LOD0 16,000 tris, LOD1 8,000, LOD2 3,000, LOD3 600. Two 2K sets (shell; cradle, manway, valves) plus `T_Trim_Steel_01`. Nanite on for the intact static mesh; off for the geometry collection. Pivot at the bottom center of the cradle footprint.

Dog sockets:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Hook_01` | Lifting lug, dome, X+ | Metal |
| `SKT_Dog_Hook_02` | Lifting lug, dome, X- | Metal |
| `SKT_Dog_Top_01` | Manway hatch center | Metal |
| `SKT_Dog_SideN_01` | Shell, 120 cm up, X+ | Metal |
| `SKT_Dog_SideS_01` | Shell, 120 cm up, X- | Metal |
| `SKT_Dog_SideE_01` | Shell, 120 cm up, Y+ | Metal |
| `SKT_Dog_SideW_01` | Shell, 120 cm up, Y- | Metal |
| `SKT_Dog_Frame_01` | Cradle rail, front | Metal |
| `SKT_Dog_F1_01` to `SKT_Dog_F5_01` | One per fragment | Metal |

Chaos geometry collection `GC_WaterTank_Hold`: L0 whole tank, L1 five pieces (four shell quarters, dome), L2 none. Damage thresholds in the project's 1 unit = 1 N·s calibration: L0 1,500; L1 99,999 (shell pieces do not break further). Player capsule impacts are included in the damage evaluation at player mass times relative speed. Anchor field `FS_Anchor_TankCradle` is permanent with no release: the L0 cluster is anchored static and leaves the anchor only by damage, never by line force. Rupture fires a server event that spawns `FV_Flood_Hold` oriented by the hull's tilt sign. Tier is recomputed server-side for the empty variant, which is a loose Medium body. Replication: no body replication while intact (static); the rupture is an RC-GC cluster break replicated with `ReplicationAbandonAfterLevel` 2, and the Medium shell pieces replicate as RC-M afterward. The Hangtown rooftop variant is a child cluster of its house's geometry collection, so it inherits the house's RC-H transform until it ruptures.

## Reference search terms

vertical steel potable water tank domed top manway; ship fresh water tank sight glass; rusted white steel tank algae waterline; industrial water tank cradle welded feet; rooftop water tank shanty corrugated; blue poly water tank rooftop Manila; tank weld seam rupture.
