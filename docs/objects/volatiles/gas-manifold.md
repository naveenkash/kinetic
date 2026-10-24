# Compressed-gas manifold

Volatile. Fixed world geometry; it cannot be moved by any line (05 §6.4). 180 × 60 × 160 cm (**PROPOSED** dimensions): a steel frame bolted to the deck carrying four 50 L high-pressure cylinders and a header pipe with a single directional vent. Appears in Ballast, the Hold (the climber's pneumatic reserve, pre-Severance, still charged) and the Yard, the Floor (the Shear Press's nitrogen bank, at the trench heads west of the press pit).

## Role in play

The one volatile that pushes and does not kill. A Jack at 600 N or above applied to it, or any blast, opens the vent: a 6 m directional jet at 15 m/s for 3 s. It is a force volume. Force volumes never Stall directly; they move things into things that do. The manifold is a fan you aim by where you stand.

Why it is there: the press and the climber both run on compressed gas. The bank is plumbed in and cannot be moved by a prep crew in one shift. The Office tags it and marks the vent direction on the floor.

A Jack against fixed geometry gives the player the whole of V in the Kick (a Slingshot off the manifold), and the push at T that follows is what trips the vent. Thresholds are inclusive, at or above (05 §6.2):

| Class | Jack push T | Vents? |
|---|---|---|
| Climber | 400 N | No. Slingshots off it at V 18 and leaves it closed. |
| Scrapper | 400 N | No. V 14. |
| Rigger | 600 N | Yes. Exactly at the threshold, and thresholds are inclusive. V 12. |
| Doubler | 800 N | Yes. Split lines at 480 N do not; at 720 N overspin they do. |
| Doubler overspin | 1,200 N | Yes. |
| Breaker | 1,000 N | Yes. V 10, and the jet adds to the launch. |

| Harness | What they do with it |
|---|---|
| Scrapper | Cannot vent it. Uses the floor arrow to stand out of the jet, and Hauls enemies into it when someone else opens it. |
| Rigger | Vents it at exactly 600 N. Jacks off it at V 12 with the jet behind them, or Hangs a line across the jet's path first: a player carried into a Hang line at 15 m/s is flipped. |
| Breaker | Jacks off it: V 10 plus a 3 s jet behind them. Braced in the jet at 20 m/s² foot-hold, a Breaker holds everywhere but the mouth. |
| Climber | Cannot vent it. In someone else's jet a Climber (foot-hold 3 m/s²) is airborne anywhere in the cone; timed with a Slingshot it is a free 6 m. |
| Doubler | Vents it at 800 N, but not with two lines live (480 N each). Hangs an enemy in the jet's path as a set line, releases it so the remaining line returns to 800 N, then Jacks the manifold open with the full line. A Jack against the enemy's plate would be a T push only (no Kick against a Catch Plate), so the Hang is the better hold. |

## Appearance

Silhouette: four tall cylinders in a square steel cage with a thick horizontal header across the top and one bell-mouthed vent pipe out the front, 20 cm diameter, pointed 10° down. The vent makes it readable: it is a nozzle on legs.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Frame | `MI_Steel_Painted_Worn`, grey `#5B6670` | | 0.45 to 0.70 | 0 / 1 chipped |
| Cylinders | `MI_Steel_Painted_Worn`, green `#3E6B4A` (**PROPOSED** inert-gas color) | | 0.45 | 0 |
| Header, vent | `MI_Steel_Bare` | `#5C5F63` | 0.50 | 1 |
| Gauges | `MI_Glass_Lamp` over black dial | | 0.05 | 0 |
| Floor arrow | `MI_Decal_Stencil` white | `#E6E4DD` | | |
| Rust | `MI_Steel_Rusted` | `#7A3F1C` | 0.85 | 0.3 |

Wear: frame rusted at the floor bolts and at every weld. Cylinder paint worn to bare in a band where the retaining chains rub. Header polished bright at the valve handles. Vent mouth soot-free, bare metal, a faint frost ring in the Hold.

Markings: `OFFICE OF THE LAY / SITE PROPERTY / PRESSURE` white on the header, 5 cm letters, Marshal number. A white floor decal 6 m long in front of the vent: an arrow and the word `VENT` every 2 m. In Ballast the cylinders carry the `MERIDIAN` hexagon in `MI_Meridian_Cyan` `#2A9FB5` at 0.9 wear and a climber system code `TEN-PNEU-04`. In the Yard the frame carries a Guild Ledger plate `LG-0219` in ochre and **PROPOSED** `COLLAR FOUR PRESS WORKS` in white.

Under arena lighting it is a dark green and grey block. No emissive except gauge needles at 0.1 units. In Ballast's Hold it stands in 40 cm of seep water, the lower frame green-black. In the Yard it is slag-spattered on the press side.

## Variants

| Variant | Difference |
|---|---|
| Ballast (Meridian) | Standing in water. Rust 0.8. Cyan livery. |
| Yard (Guild) | Dry. Rust 0.4. Guild plate. Caged on three sides by rebar mesh. |
| Vented | Gauges at zero, frost gone, vent pipe discolored. Inert for the round. |
| Half bank (two cylinders) | **PROPOSED**: jet 1.5 s instead of 3 s, same speed and reach. Gauges read half. Yard sector variant for tuning. |

## Physics

Fixed world geometry. No mass, no tier, no break-free; it is static to every line in the game, including a Breaker's 1,000 N and a Doubler's 1,200 N overspin. Static collision: one box 180 × 60 × 160 cm for the frame, plus a cylinder radius 10 cm, length 60 cm for the vent pipe. Friction 0.6, restitution 0.05 (**PROPOSED**), which matter only for things that hit it.

Vent trigger: a Jack at 600 N or above applied to any part of the manifold (inclusive, 05 §6.2), or any blast whose radius reaches it. Not a Haul, not a Hang: the jet is the header's relief valve lifting when the frame is pushed, and the authored rule is Jack only. The Jack push force is the class T (Climber 400, Scrapper 400, Rigger 600, Doubler 800, Breaker 1,000, Doubler overspin 1,200 N; split Doubler lines 480 or 720 N). Hiss and venting 0.3 s before the jet reaches full speed. Jet: a cone 6 m long, 20 cm at the mouth widening to 3 m at 6 m, air at 15 m/s, for 3 s.

Force volume profile, **PROPOSED**: acceleration 20 m/s² at the mouth falling linearly to 5 m/s² at 6 m, directed along the vent axis. Against the class foot-holds (Climber 3, Scrapper 4, Rigger 5, Doubler 5, Breaker 5 m/s²) every unbraced player anywhere in the 6 m cone is airborne, the Breaker included (5 m/s² at the cone's edge). A braced Rigger (9 m/s²) holds beyond 4.4 m; a braced Breaker (20 m/s²) holds anywhere but the mouth itself. The jet is a force volume and follows the force volume rule (05 §6.6): it acts on players and free bodies only, scaled by mass as a_ref × min(100 / m, 2). A loose 6 kg chair feels 2 times the authored acceleration (the cap; 40 m/s² at the mouth) and reaches the jet speed (15 m/s) within about 3 m; a loose 45 kg cylinder also feels 2 times (40 m/s² at the mouth) and is over its 8 m/s trigger after 0.8 m; a loose 180 kg drum feels 0.56 times (11 m/s² at the mouth) and rolls 2 to 3 m; the 900 kg Claim marker feels 0.11 times and does not move. A cylinder held on a player's Hang line in the jet is a free body and swings downwind. A racked cylinder in the jet stays racked. Bolted and fixed objects are unaffected.

The jet has no impulse table because it Stalls nobody directly. What it does is deliver a player into a wall: a Climber carried 6 m by the jet arrives at the far wall at up to 15 m/s, which is over the 12 m/s static Stall threshold. Into the Yard's press pit, it is a ring-out. Into the Hold's nose on the Breathing lean, same.

The manifold is never a projectile. Crush: the 8 kN rule is unreachable by any line and comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1), so a player pinned against the manifold by a Hauled 250 kg crate is not crushed; a player pinned against it by a 2,000 kg container sliding on a Breathing lean, or by a kinematic, is.

Blast: none. A blast reaching the manifold vents it; it does not detonate (**PROPOSED**: the gas is inert nitrogen, so the Kiss on a manifold is a fan, not a bomb).

## Interactions

Haul to it: the most solid thing on the Floor. The player is pulled to it, a grapple, and the manifold takes nothing. Jack from it: a Slingshot off fixed geometry; the Kick gives the player all of V. If the line's push is 600 N or more the manifold vents in the player's face as they leave, adding the jet to their launch. A Breaker leaving at 10 m/s with 20 m/s² of jet behind them covers 6 m fast; a Rigger at 12 m/s does the same. Hang from it: a rigid Pendulum anchor; a Rigger's 12 kN Hang load cannot move it, and nothing can. A Hang never trips the vent.

Object to object: a cylinder Scattered into the manifold triggers the cylinder's blast, which vents the manifold. A crate Hauled into it stops dead. Loose Light props in front of it when it vents are a 15 m/s spray: a 6 kg chair at 15 m/s is 90 N·s, nothing; a 25 kg chain coil at 15 m/s is 375 N·s, a stagger. A loose cylinder in the jet arrives at the far wall over 8 m/s and blasts there.

Kinematics: the Breathing hull carries the Hold manifold with it; on the lean toward the nose the jet points 6° further down. In the Yard no Hauler path crosses the vent cone (level rule). A Hauler passing through a Hang line strung from the manifold severs it (05 §3.5); the Dog in the manifold is not harmed, only released.

Water: fixed, so no float. The Hold manifold vents across the water surface and pushes the water shove with it (**PROPOSED**: the jet volume adds its acceleration to the water volume's where they overlap). Floating props in the Hold are carried at jet speed. A blast that vents the Hold manifold from under 1 m of water is half impulse, but a half blast still vents it. Updraft: not present on the Anchor. Breathing tilt: fixed geometry rides the hull.

Chain reactions: the manifold is often the last link. Charge drops the wall, cylinders go, the blast vents the manifold, and the jet sweeps the survivors toward the Well.

## States

| State | Visual | Physics |
|---|---|---|
| Intact, charged | Gauges at full. Frost ring on the vent mouth in the Hold. | Vent trigger live. Static. |
| Dented | One cylinder dished, a gauge cracked. Any impact over 1,200 N·s. | Unchanged. |
| Armed | Relief valve lifts, hiss, 0.3 s. Frost cloud at the mouth. | Jet starts at 0.3 s. |
| Venting | White condensation jet 6 m, dust and debris streaming. 3 s. | Force volume active. |
| Vented | Gauges at zero. Frost melted. | Inert for the round. Still an anchor. |

No Broken or Destroyed state. It does not move and it does not fracture.

## VFX and SFX hooks

- Impact on it: deep hollow gong on the cylinders, clank on the frame, nothing moves.
- Armed: `NS_Vent_Manifold_Prime` (frost puff), `SFX_Vol_Hiss_Manifold`, a rising shriek that peaks at 0.3 s.
- Venting: `NS_Jet_Manifold` (6 m condensation cone, ground dust entrained, debris ribbons, 3 s), `SFX_Vol_Jet_Manifold`, a 3 s roar with a Doppler layer for players carried in it, then a 1 s dying whistle.
- Emissive: gauge needles only, 0.1 units.

## Technical

LOD0 16,000 tris, LOD1 8,000, LOD2 3,000, LOD3 600. Two 2K sets (frame and header; cylinders and gauges) plus `T_Trim_Steel_01`. Nanite on. Pivot at the bottom center of the frame footprint. X+ is the vent direction.

Dog sockets (Dogs seat in it; the sockets give Slingshot and Hang anchors and the snap target for the Jack trigger):

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Hook_01` | Lifting lug, top frame rear left | Metal |
| `SKT_Dog_Hook_02` | Lifting lug, top frame rear right | Metal |
| `SKT_Dog_Top_01` | Header pipe, center | Metal |
| `SKT_Dog_Valve_01` | Vent pipe, 20 cm behind the mouth | Metal |
| `SKT_Dog_SideN_01` | Front frame rail, 80 cm up | Metal |
| `SKT_Dog_SideS_01` | Rear frame rail, 80 cm up | Metal |
| `SKT_Dog_SideE_01` | Right cylinder band | Metal |
| `SKT_Dog_SideW_01` | Left cylinder band | Metal |

Not a geometry collection and not a simulated body: a static mesh actor with static collision and no anchor field. The jet is a force volume actor `FV_Jet_Manifold` attached to the vent socket: a cone trigger with the acceleration profile above, lifetime 3 s, server-spawned on the vent event. The vent event fires server-side when a Jack line seated in the manifold reports a push force of 600 N or more, or a blast field overlaps it. No body replication; the arm and vent events are reliable multicasts, and the force volume's lifetime is replicated by match-clock start time like a kinematic. Clients play the hiss from the arm event.

## Reference search terms

nitrogen cylinder manifold bank four pack; industrial gas manifold frame bolted floor; high pressure gas cylinder cradle pallet; hydraulic press nitrogen accumulator bank; compressed gas relief valve vent pipe; frost on gas vent cryogenic; shipyard gas bank cage.
