# Propane cylinder

Volatile. Medium tier, 45 kg. 30 cm diameter, 120 cm tall including the valve guard. Appears in every arena: Ballast, the Bays (welding stations on the catwalks) and the Crown (HTR cutting gantry); the Yard, the Floor and the Stacks (cutting stations between hull sections); Hangtown, the Mid Hang (kitchens, market platform); the Anchor, the Scaffold (contractor cutting stations on levels 2 and 4).

## Role in play

The game's standard bomb. It is the reason the Kiss and the Scatter exist. It kills by blast, not by its own impulse: 05 §3.4 states the canonical case, "A Scrapper's 400 N on a 45 kg propane cylinder is 8.9 m/s²: it reaches 19 m/s over a 20 m line, about 850 N·s, a flip but not a Stall." It triggers at 8 m/s, and the blast Stalls anyone within 2 m. The cylinder is a delivery system for its own blast.

Why it is there: a cutting crew runs oxy-propane torches. Every cutting station has a cylinder, and the prep crew leaves them where the cutters left them. The Office stencils each one as site property before the match.

| Harness | What they do with it |
|---|---|
| Scrapper | Hauls it at 8.9 m/s² with the 30% bonus (Medium under 100 kg): 24.7 m/s at 20 m, 1,110 N·s, a flip, and a blast on arrival. Kisses two from a rack at 200 N each. |
| Rigger | The Guillotine: Hangs one from an overhead anchor (45 kg is 441 N, within every class Hang load), Reels it up by holding Haul within Hang at 1.0 m/s, sets the line, and cuts or recalls it when the enemy passes. The cylinder falls by gravity, passes 8 m/s after 3.3 m, and blasts on the floor at their feet. A second line to the enemy only steers it. 600 N pulls it from a rack or a stack. |
| Breaker | 1,000 N is 22.2 m/s²; the cylinder hits the 25 m/s cap after 14 m. Uses it as a fuse: Hauls it into a crate stack it cannot otherwise reach. |
| Climber | Scatter: Jacks two apart from above. The Kick gives each cylinder 9 m/s and T keeps pushing; both are over the 8 m/s trigger before they land. |
| Doubler | Two lines, two cylinders, one target, at 480 N each. Or overspins a Hang on one and snaps it at 1.8 s: 1,500 N·s on 45 kg is 33 m/s into the crowd. |

## Appearance

Silhouette: a tall thin can with a collar. Foot ring at the base, a welded three-prong valve guard at the top, a brass valve with a red handwheel inside the guard. The silhouette must read as "bomb" at 30 m, which is why it is taller and narrower than a real 47 L cylinder.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Body | `MI_Cylinder_Propane_Grey` | `#B8B4A8` | 0.40 to 0.60 | 0 (1 where scraped) |
| Foot ring, valve guard | `MI_Steel_Bare` | `#5C5F63` | 0.50 to 0.65 | 1 |
| Valve body | `MI_Steel_Bare` tinted brass `#A8894A` | | 0.35 | 1 |
| Handwheel | `MI_Steel_Painted_Worn`, red `#B5302A` | | 0.55 | 0 |
| Shoulder band | `MI_Decal_Stencil` | per supplier | | |

Wear: the body is scraped to bare steel in a band 5 cm above the foot ring where it gets rolled, and polished where hands lift it by the guard. Rust starts at the foot ring weld. Dents are on the shoulder, never the base.

Markings: `OFFICE OF THE LAY / SITE PROPERTY` in white `#E6E4DD` on the shoulder, 4 cm letters, with a Marshal number `M-0417`. Below it a supplier band in faded red: **PROPOSED** `RUCU GAS`, with a tare weight and test date in stamped text. A yellow diamond hazard decal at 0.7 wear.

Under arena lighting it reads as a matte pale vertical. No emissive on the cylinder itself; the singing line must stay the brightest thing in frame. In Ballast's Hold it is green-black at the base from seep water. In the Yard it carries slag spatter on one side. In Hangtown half of them are painted in the hanger palette, teal `#2E8B8B` and mustard `#D9A441`. On the Anchor they are pale with concrete dust.

## Variants

| Variant | Difference |
|---|---|
| Full (default) | 45 kg. Everything above. |
| Empty | 22 kg, Light tier (tier recomputes on mass change, 05 §6.3). Valve open, handwheel missing. Does not blast; vents a 1 s harmless hiss on impact over 8 m/s. Floats high in water. Scrapper Light bonus applies. |
| HTR | `MI_HTR_Grey_Matte` body with an orange shoulder band `#E8620C`. Asset tag `HTR-RC-2291`. Low wear. |
| Hanger-painted | Two hanger colors, a kitchen stencil (a kettle). Dents at 0.8 wear. |
| Dented | Shoulder crushed 4 cm, valve guard bent. Cosmetic until Blown. |

## Physics

Medium tier, 45 kg. Collision: one capsule, radius 15 cm, length 105 cm, plus a 15 cm cylinder for the guard. Center of mass 52 cm above the base when full, 60 cm when empty. Friction 0.40 steel on steel, 0.55 on concrete, 0.35 on wet steel (**PROPOSED**). Restitution 0.15. Tips over past 14°.

Break-free: loose on floor 0 N; in a rack or cradle 200 N (anyone); chained to a cart 800 N (Doubler at 800 N, Breaker at 1,000 N, not Climber, Scrapper, or Rigger).

Haul acceleration by class, other end static or braced, with the arrival speed at the class line length and the impulse into a player:

| Class | T | a | Line | Arrival speed | Impulse | Result |
|---|---|---|---|---|---|---|
| Climber | 400 N | 8.9 m/s² | 50 m | 25 m/s cap, reached at 35 m | 1,125 N·s | flip, and blast |
| Scrapper | 400 N | 8.9 m/s² | 20 m | 19 m/s; 24.7 m/s with the 30% bonus | 850 N·s; 1,110 N·s | flip, and blast |
| Rigger | 600 N | 13.3 m/s² | 40 m | 25 m/s cap, reached at 23 m | 1,125 N·s | flip, and blast |
| Doubler | 800 N | 17.8 m/s² | 30 m | 25 m/s cap, reached at 18 m | 1,125 N·s | flip, and blast |
| Doubler overspin | 1,200 N | 26.7 m/s² | 30 m | 25 m/s cap, reached at 12 m | 1,125 N·s | flip, and blast |
| Breaker | 1,000 N | 22.2 m/s² | 20 m | 21 m/s after 10 m; 25 m/s cap at 14 m | 945 to 1,125 N·s | flip, and blast |

The cylinder's own impulse never reaches a Stall on a Haul. It reaches the 8 m/s trigger after 3.6 m on a Scrapper line and 1.4 m on a Breaker line, so every Hauled cylinder arrives armed.

Jack: the Kick drives the ends apart to the class V, split by inverse mass, then T pushes. A Scrapper (110 kg) Jacking a cylinder: the Kick gives the cylinder 9.9 m/s and the Scrapper 4.1 m/s at 0.3 s; the cylinder then accelerates at 8.9 m/s² and reaches 21 m/s at the 20 m line end. A Breaker's Kick gives it 8.3 m/s, then 22.2 m/s²: the 30 m/s cap at 19 m. A Climber's Kick gives it 12 m/s, then 8.9 m/s²: the cap at 45 m. At the 30 m/s cap the cylinder is 1,350 N·s, a Stall by impulse, and it blasts. Two cylinders Jacked apart (the Scatter) each take V/2 and then T/m each way: 9 m/s each on a Climber's line at 0.3 s, both over the trigger.

Snap: a Doubler's overspin snap delivers 1,500 N·s × (F / 1,200 N) to the far end. At full overspin a cylinder leaves at 33 m/s (under the 50 m/s terminal velocity), 1,500 N·s into whatever it meets, Stall, plus the blast. Cut recoil: 150 N·s per 1,000 N to both ends. A Breaker-line cut gives the cylinder 3.3 m/s, under the trigger. Cutting a line on a cylinder never sets it off.

Trigger: impact over 8 m/s with anything, or another blast. Hiss and venting 0.3 s before the blast. Blast radius 4 m, impulse 1,500 N·s at center falling linearly to 0 at 4 m. Per 05 §6.4 the player results are authored overrides of the falloff so that a blast is always readable: within 2 m Stalled, 2 to 4 m flipped.

| Distance | Impulse | Object result | Player result (05 §6.4) |
|---|---|---|---|
| 0 m | 1,500 N·s | 45 kg neighbor leaves at 33 m/s | Stall |
| 1 m | 1,125 N·s | neighbor at 25 m/s | Stall |
| 2 m | 750 N·s | neighbor at 17 m/s | Stall |
| 3 m | 375 N·s | neighbor at 8 m/s, at the trigger | flip |
| 4 m | 0 | | nothing |

Under more than 1 m of water a blast delivers half impulse and no fire patch (05 §6.4).

Other speeds: walked into at 2 m/s, 90 N·s, nothing. A 7 m drop, 11.7 m/s, 527 N·s, stagger, but it triggers. Thresholds: 300 N·s at 6.7 m/s, 600 at 13.3 m/s, 1,200 at 26.7 m/s. Crush: 441 N of weight; a cylinder never crushes.

## Interactions

Haul to it: the player is pulled to the cylinder at T over the player's mass, and the cylinder comes to them at T over 45 kg; closing acceleration is the sum. A Scrapper (3.6 m/s², under their 4 m/s² foot-hold) skids while the cylinder does the moving. Jack from it (self-dog to cylinder): the Kick gives the cylinder most of V and the player a little recoil; against a racked cylinder (200 N) the Kick releases the rack first. Jack from the cylinder to an enemy's Catch Plate: the Kick never fires when a Dog is in a Catch Plate (05 §3.2), so it is a T push only. The cylinder accelerates at T over 45 kg from rest (8.9 m/s² on a Scrapper line, 22.2 on a Breaker's) and the enemy skids at T over their mass; the cylinder still passes the 8 m/s trigger within 3.6 m and blasts on whatever it meets, but it leaves slowly enough to be seen. Hang from it: break-free is tested against active pull only (05 §6.2), so a racked cylinder holds a hanging player up to the class Hang load and does not leave the rack; a player can Pendulum from a cutting station, and climb to it by Reel. A loose cylinder comes along as 45 kg of dead weight, or is Reeled up to an anchor as a Guillotine weight and dropped by cut or recall.

Object to object: two cylinders Hauled together (the Kiss) close at up to 25 m/s and both trigger on contact; from a rack each needs 200 N, which any line has. The Scatter (cylinder to cylinder, Jack) still Kicks on both ends; the Kick interlock applies to Catch Plates only. One blast triggers every cylinder within 4 m, and launches cylinders it does not trigger. Cylinder into a fuel drum at over 10 m/s triggers the drum.

Kinematics deliver full relative impact. A Hauler at any speed over 8 m/s detonates a cylinder in its path. The Gantry load dropped onto one detonates it. A kinematic passing through a line severs it (05 §3.5): a Hauler crossing a Hang line that holds a cylinder over the trench snaps the line, both Dogs release, and the cylinder drops where it hung. Lines seated in the kinematic itself are not severed.

Water: a full cylinder floats with 15 cm of valve guard showing; empty it floats on its side. In the Hold on the Breathing tilt, floating cylinders drift with the water shove toward the nose. The Hold's seep water is 40 cm deep; a blast there is a full blast. A blast in a pool over 1 m deep is half impulse. On the Breathing, an upright cylinder on steel does not slide at 6° (friction holds to 22°) and does not tip (**PROPOSED**: the 90 s cycle is slow enough not to rock it over the 14° limit). On its side it rolls at 0.7 m/s² and reaches 8 m/s after 45 m, which is longer than any Bay. Force volumes act on players and free bodies only, scaled by mass as a_ref × min(100 / m, 2) (05 §6.6): a loose 45 kg cylinder feels 2 times the authored acceleration (the cap), so in the Anchor's Updraft a loose cylinder at the Stump lifts and tumbles, and airborne cylinders are the Updraft's own Scatter. A racked cylinder is unaffected until it is pulled. A cylinder hung on a player's Hang line is a free body and swings in a gust: a Guillotine cylinder sways in the Updraft, and its drop point moves with it. Cable-hung map objects do not count as free. The Pulse (0.4 s outward shove, then 0.6 s inhale toward the Stump, every 75 s from 0:55) throws a loose cylinder on the Scaffold outward and then draws it back harder; a cylinder that reaches the Scaffold's inner edge on the inhale goes down the Well.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | As above. | Full behavior. |
| Dented | Shoulder crushed, guard bent, handwheel at an angle. | Unchanged. Reached after any impact between 4 and 8 m/s. |
| Armed | Valve plume, frost ring on the shoulder, hiss. 0.3 s. | Already committed. Cutting a line on it changes nothing. |
| Blown | Replaced by a 20 cm torn foot ring (Light, 3 kg, 10 s lifetime) and a scorch decal 2 m across. | Blast field applied once, server-side. |
| Destroyed | Nothing remains after 10 s. | |

There is no Broken state. A cylinder that would break has already blown.

## VFX and SFX hooks

- Impact: hollow steel ring on steel, dull thud on concrete, wet slap on wood, splash in water. Pitch rises with how empty it is.
- Break-free from a rack: a scrape and a clank of the retaining chain. Niagara `NS_Lurch_Dust_Small`.
- Armed: `NS_Vent_Propane` (white cold plume from the valve, 0.3 s), audio `SFX_Vol_Hiss_Propane`, directional, audible at 20 m.
- Blast: `NS_Blast_Propane` (orange-white flash 0.1 s, pressure ring 4 m, lofted debris, 1.5 s smoke). Audio `SFX_Vol_Blast_Propane` with a low-frequency sweetener; the Marshal gallery echo plays 0.2 s later. Underwater: `NS_Blast_Propane_Sub` (a dome of foam, no flash).
- No emissive. The hazard decal is retroreflective (roughness 0.2) so it catches a nearby singing line.

## Technical

LOD0 5,000 tris, LOD1 2,500, LOD2 1,000, LOD3 250. One 2K texture set (BC, N, ORM) plus the shared stencil decal atlas. Pivot at the bottom center of the foot ring.

Dog sockets:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Top_01` | Top, center of the valve guard | Metal |
| `SKT_Dog_Valve_01` | Valve, on the brass body | Metal |
| `SKT_Dog_SideN_01` | Body, 60 cm up, X+ | Metal |
| `SKT_Dog_SideS_01` | Body, 60 cm up, X- | Metal |
| `SKT_Dog_Bot_01` | Center of the foot ring | Metal |

Not a geometry collection; it swaps mesh on Blown. Blast is a Chaos `Radial Vector` field actor `FS_Blast_Propane`: radius 400 uu, magnitude 1,500 N·s at center, linear falloff, one-shot; magnitude halved and the fire flag cleared when the origin is under more than 1 m of water. Spawned by a server event that also applies the 2 m Stall and 4 m flip to players by overlap, so the player rule does not depend on the physics impulse. Placed cylinders in racks sit in anchor field `FS_Anchor_PropaneRack` with a 200 N release; chained cylinders in `FS_Anchor_Chained` at 800 N. Tier is recomputed from current mass when the empty variant is spawned. Replication RC-M: server-authoritative Chaos body at 20 Hz; clients predict the hiss locally when the arm event arrives and play the blast VFX on the server's blast event.

## Reference search terms

47 kg LPG cylinder steel; propane cylinder valve guard three prong; oxy-propane cutting torch cart; dented gas cylinder scrap yard; Pakistan LPG cylinder painted; shipyard cutting station gas bottles; cylinder foot ring rust.
