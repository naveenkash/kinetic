# Stall Gel pad

Medium prop, prep-crew placed, rare. Mass tier Medium, 40 kg. 300 × 200 × 20 cm (a quilted gel mattress in a ripstop shell with a tubular frame edge and four corner grommets). Appears one or two per arena, at Office-chosen positions: the Yard (the Floor, below the control shack's ladder), Hangtown (the Upper Hang, under the Lift headframe), Ballast (the Bays, at the foot of the Spine ladders), the Anchor (the Rim, below the Scaffold's quadrant ladders). Never near a Well edge.

## Role in play

The only surface in the game that makes a fall safe. 05 §7.3 gives it a ×4.0 multiplier on the player's stagger, flip, and Stall thresholds against static: a player Stalls at 12 m/s on a hard floor and at 48 m/s on a pad. Terminal velocity is 50 m/s. In practice nothing in the playable height range Stalls on a pad. The pad does not help against objects hitting a player, blasts, crush, or kinematics. It is a 6 m² exception, and it can be moved by lines.

| Rig | Use |
|---|---|
| The Gallows | The counter. A pad Hauled under a hoisted teammate before the enemy releases them. |
| The Pendulum | Land a long swing on a pad. The Climber's safe apex. |
| The Anchor Drop | Prep crews never put a pad near an edge. Players Haul one there and jump after an enemy. |
| Overboard | A pad on the far side of a gap turns a Jack launch into a crossing. |

- **Scrapper:** at 40 kg the pad is under the 100 kg Medium limit, so it moves 30% faster on a Scrapper line: 400 N is 10 m/s², 20 m/s over 20 m, times 1.3 is the 25 m/s cap. Scrappers steal it and keep it.
- **Rigger:** Hangs a pad vertically across a doorway. It is a soft Clothesline that stops Dogs: a Dog landing on the pad seats in it, and the pad takes the Haul (a 40 kg sail).
- **Breaker:** uses it as a ladder-free descent. Breakers have 0.8 m jump height and no other way down fast.
- **Climber:** the pad is the Ascension's insurance. Falls from 30 m onto a pad (24 m/s) are a stagger, not a Stall.
- **Doubler:** snaps a line on purpose near a pad; the 10 m/s self-launch lands soft. A Snap with the far Dog in the pad is 1,500 N·s: the pad leaves at 37.5 m/s and Stalls whoever it hits. The safest object in the game, weaponized.

## Appearance

Silhouette: a thick, bright rectangular mattress with rounded quilting squares, a visible tubular frame at the edges, and a translucent gel core that glows faintly at the seams. Reads as the one clean, bright, soft thing on a demolition site.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Shell top | `MI_Canvas_Natural` tinted `#E6E4DD` with ripstop grid in the normal | | 0.80–0.90 | 0 |
| Gel seams and sight windows | `MI_StallGel` | `#3FE0D0` | 0.10–0.20 | 0 |
| Edge frame | `MI_Steel_Painted_Worn` in `#141414` | | 0.50 | 0 |
| Grommets | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |
| Underside | `MI_Rubber_Black` | `#1E1F20` | 0.80 | 0 |
| Branding | `MI_Decal_Stencil` | `#3FE0D0`, `#141414`, `#E6E4DD` | inherits | 0 |

Emissive: `MI_StallGel` at 2.0 on the seams and two 40 × 40 cm sight windows, subsurface 0.6. The pad is lit so that it never competes with a singing line: emissive is capped so a line at any force reads brighter at 5 m.

Wear: boot prints across the top. One corner scuffed to the gel. A Marshal's chalk circle around the pad's authored position. Shell is new; the Office replaces pads every match in fiction, so wear is 0.2.

Text: `NURRI STALL-GEL / OFFICIAL LANDING SYSTEM OF LAYCAST` along the long edge in cyan and black. `OFFICE OF THE LAY / PLACED BY PREP CREW / DO NOT REMOVE` in white on the short edge, which players ignore. A Marshal number `M-07` on each corner.

Lighting: the brightest matte surface in any arena. Under Hangtown's Office lights it is the only pure white on the map. In the Anchor's darkened sector after a tower collapse, the seams glow and mark the landing.

## Variants

| Variant | Difference |
|---|---|
| Standard | White shell, cyan seams, Nurri branding. |
| LayCast | Red `#C8392B` edge frame, LayCast logo in the center, cyan seams. The Anchor finals. |
| Chained | Four corner chains to floor anchors. Break-free 800 N: only a Doubler or Breaker can take it. Prep crews chain one of the two pads per arena. (**PROPOSED** placement rule) |
| Torn | Shell slit, gel exposed in a 60 cm patch. Multiplier still ×4.0 (cosmetic). |
| Folded | Spawns folded in half, 150 × 200 × 40 cm, 40 kg. Unfolds when a line Hauls a corner over 100 N. (**PROPOSED**) |

## Physics

- Tier Medium, 40 kg.
- Collision: one box 300 × 200 × 20 cm with a 4 cm rounded edge. The ×4.0 surface is the top face only; the underside and edges are hard (×1.0). (**PROPOSED**: edge faces are ×1.0)
- Center of mass: geometric center, 10 cm above the floor.
- Friction 0.9 top (grippy ripstop), 0.7 underside rubber on steel and concrete, 0.75 on wood. It does not slide easily; that is the point. Restitution 0.3 top face, 0.05 edges. (**PROPOSED**)
- Break-free (05 §6.2): loose 0 N, anyone; chained 800 N, Doubler and Breaker. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Does not fracture.

Surface rule (05 §7.3): the player's stagger, flip, and Stall thresholds against static are multiplied by 4.0 on the pad's top face: 20, 32, and 48 m/s. Worked falls, vertical landing on the pad:

| Fall height | Landing speed | On hard floor | On the pad |
|---|---|---|---|
| 7.3 m | 12 m/s | Stall | Nothing |
| 20 m | 19.8 m/s | Stall | Nothing |
| 30 m | 24.3 m/s | Stall | Stagger |
| 40 m (the Yard's Rails) | 28 m/s | Stall | Stagger |
| 60 m | 34.3 m/s | Stall | Flip |
| 130 m or more | 50 m/s terminal | Stall | Stall |

A Breaker (220 kg) landing at 28 m/s is still a stagger; the threshold is a speed against static, not an impulse, per 05 §7.2.

Line acceleration and arrival, other end static or braced: 400 N is 10 m/s², 600 N is 15, 800 N is 20, 1,000 N is 25, 1,200 N is 30 m/s². Every class reaches the 25 m/s Haul cap inside its line (a Scrapper exactly at 20 m with the bonus). Arrival 25 m/s, 1,000 N·s, a flip.

Impact on a player by the pad itself (impulse = 40 kg × relative speed): stagger at 7.5 m/s, flip at 15 m/s, 80 N·s at walking speed (nothing), 468 N·s from a 7 m drop (stagger), 1,000 N·s at the 25 m/s Haul cap (flip), 1,200 N·s at the 30 m/s Jack cap (Stall, exactly at threshold), 1,500 N·s on a Doubler Snap at 37.5 m/s (Stall). The pad's softness does not reduce the impulse it delivers; 05 applies surface multipliers to the player hitting static, not to objects hitting players.

Jack (05 §3.2 v0.2): the pad is Kicked to V × M / (M + m) within 0.3 s (the Kick), then T / 40 kg to the 30 m/s cap. Climber (V 18): pad 12.5 m/s, Climber 5.5 back; 10 m/s² takes the pad to the cap within 37 m. Scrapper (V 14): pad 10.3, Scrapper 3.7; 24 m/s at the end of 20 m. Breaker (V 10): pad 8.5, Breaker 1.5; 25 m/s² reaches the cap within 17 m. Doubler (V 14): pad 10.7, Doubler 3.3. Rigger (V 12): pad 9.2, Rigger 2.8. The Scrapper bonus applies to the post-Kick push, not to V (05 §6.1: 1.69× force, ×1.3 arrival speed).

Crush: 40 kg cannot crush, and no line reaches 8 kN (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). A player pinned between a pad and a wall by a kinematic is crushed through the pad; the pad gives no protection.

## Interactions

- **Haul to it:** the pad comes to the player, fast and flat; the player skids or, if a Climber, lifts (400 N on 90 kg is 4.4 m/s², over the 3 m/s² foot-hold). A Climber Hauling the pad brings it under themselves mid-fall if the line length allows.
- **Jack from it:** a weak launch. The split above leaves a Climber 5.5 m/s and sends the pad sliding away.
- **Hang from it:** a Hang to a chained pad holds on the chains up to the class Hang load without starting it. A Hang to a loose pad drags it as 40 kg of dead weight, which is how a Climber tows a landing. Hung from a corner grommet, it hangs as a vertical curtain. A player who Hangs from the pad while the pad lies on the floor has a 40 kg anchor at friction 0.7: a 130 kg Rigger pulling against it drags it.
- **Object to object:** objects landing on the pad take no benefit; they bounce at restitution 0.3. A propane cylinder dropped on a pad from 4 m (8.9 m/s) still triggers.
- **Kinematic:** a pad under a Hauler is shredded (destroyed state) and the Hauler continues.
- **Water:** floats, top up. In the Hold flood it drifts with the volume. A pad floating on the Hold's water combines with the water's ×3.0; use the higher, not the product (**PROPOSED**).
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A chained pad is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 40 kg pad feels 2× (cap) the authored acceleration (a_ref × min(100 / m, 2)): near the Stump the Updraft lifts it like a sail, and the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) skids it out, then back.
- **Breathing:** friction 0.7 holds it at 6°. Only a line moves it.
- **Cut recoil:** 150 N·s per 1,000 N. A Breaker-line cut kicks the pad at 3.75 m/s; it slides half a meter on its rubber.
- Slides reluctantly, floats, does not roll.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Flat, bright, seams lit. | ×4.0 top face. |
| Dented | Gel displaced to one end, a 10 cm sag. | No change. |
| Broken | Shell torn, gel sheet exposed, one grommet gone. | ×4.0 unchanged. One Hook socket removed. |
| Destroyed | Shredded by a kinematic or a blast over 2,500 N·s: strips and a gel pool 3 m across. | Pad gone. Gel pool is a decal with friction 0.2 for 6 s. No multiplier. |

## VFX and SFX hooks

- Landing: `SFX_GelPad_Land_Soft` scaled by speed, with a wet gel squelch layer over 20 m/s. `NS_GelPad_Ripple` on the surface, radius scaled by impulse.
- Impact by the pad: `SFX_GelPad_Slap_Steel`, `_Concrete`, `_Wood`, `SFX_GelPad_Splash`.
- Break-free (chained): `SFX_BreakFree_Chain` ×4. Under 800 N the chains hum and the pad lifts a corner.
- Destroyed: `NS_GelPad_Shred`, `NS_Gel_Pool_Decal`, `SFX_GelPad_Tear`.
- Emissive: seams and windows at 2.0, pulsing 0.1 Hz. Flashes to 4.0 for 0.3 s when a player lands on it, so the broadcast sees the save.

## Technical

- LOD0 6,000 tris (quilting modeled), LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set plus the emissive mask. The quilting is a normal map at LOD2 and below.
- Pivot: bottom center of the footprint. X+ is the long axis.
- Dog sockets: `SKT_Dog_Top_01`, `_02` (top face at thirds) `Fabric`; `SKT_Dog_Hook_01` to `_04` (corner grommets) `Metal`; `SKT_Dog_Frame_01`, `_02` (long edge frame centers) `Metal`; `SKT_Dog_Bot_01` (underside center) `Fabric`.
- Not a geometry collection. Static mesh with a physical material `PM_StallGel` on the top face carrying the ×4.0 surface tag, and `PM_Hard` on the edges and underside.
- Anchor field `FS_Anchor_StallGelPad` releases chained instances at 800 N.
- Replication RC-M. The surface multiplier is read server-side at the player's impact; clients predict the landing result from the replicated pad transform.

## Reference search terms

Stunt crash mat ripstop white; high fall air bag landing pad film set; gymnastics crash mat quilted; gel mattress translucent core; crash pad corner grommets chained; stunt mat under scaffold photo.
