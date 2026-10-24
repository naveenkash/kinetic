# Hand truck

Medium prop. Mass tier Medium, 35 kg. 50 × 60 × 125 cm (W × D × H; a two-wheel steel dolly with a 35 × 20 cm toe plate, a curved back frame, and two pneumatic tires). Appears in the Yard (the Floor, at the control shack and the scrap heaps), Hangtown (the Mid Hang, at the market ring and the Lift landing), Ballast (the Bays, one per catwalk landing), and the Anchor (the Rim, in the broadcast tower laydown).

## Role in play

The lightest Medium. It cannot Stall anyone on a Haul or a Jack under either speed cap, which makes it a carrier, a trip, a lever, and an anchor rather than a projectile. Its job is to hold something heavier: a propane cylinder, a water barrel, a steel crate standing on end. A loaded hand truck is whatever is on it, plus 35 kg, on wheels. The one way it kills on its own is a Doubler's Snap.

| Rig | Use |
|---|---|
| The Caboose | A hand truck with a cylinder on it is the fastest way to bring a volatile to a Hauler trench. |
| The Clothesline | Lay it flat: a 12 cm bar across a walkway at wheel height. The back frame is an anchor at 1.25 m. |
| Overboard | A self-dog to enemy Jack is a T push only; the Kick never fires against a Catch Plate. A loaded hand truck wedged behind you against a railing is the backstop. |
| The Kiss | Two hand trucks, two cylinders, one Haul. |

- **Scrapper:** rolls it everywhere. At 35 kg it is under the 100 kg Medium limit, so it moves 30% faster on a Scrapper line; it reaches the 25 m/s cap over the Scrapper's 20 m line either way. A Scrapper tips a loaded hand truck back and Hauls it to an enemy's feet.
- **Rigger:** anchors a set line to the frame at 1.25 m. Hung from a railing it is a swinging gate.
- **Breaker:** ignores it unless it is carrying something. Steps over the Clothesline version.
- **Climber:** Jacks off it and keeps only 13 m/s of their 18 m/s V; the truck takes 5 m/s. Poor launch, good decoy.
- **Doubler:** a hand truck on each line, Dogged truck to wall, Jack. No Catch Plate is involved, so the Kick fires: each truck leaves at the full 14 m/s V and the 480 N split push (13.7 m/s²) takes it to the 30 m/s cap by 25 m of the 30 m line. Both flip anyone they hit (1,050 N·s). A Snap on one is 1,500 N·s: the truck leaves at 43 m/s and Stalls whoever it meets.

## Appearance

Silhouette: a tall narrow P shape: toe plate at the bottom, two fat tires behind it, a curved tubular frame rising to a loop handle. Reads as a stick figure from any distance.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Frame | `MI_Steel_Painted_Worn` | `#8E2A1F` red default; `#E8620C` HTR; `#C8912A` Guild | 0.45–0.70 | 0 / 1 at chips |
| Toe plate | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 | 1 |
| Tires | `MI_Rubber_Black` | `#1E1F20` | 0.70–0.85 | 0 |
| Hubs, axle | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 | 0.3 |
| Handle grips | `MI_Rubber_Black` worn to `#3A3B3D` | | 0.85 | 0 |
| Stencils | `MI_Decal_Stencil` | `#E6E4DD` | inherits | 0 |

Wear: the toe plate is scraped silver on its leading edge and bent down 1 cm at one corner. The frame paint is gone where loads rub, a bright vertical band on both uprights. One tire is soft. Hangtown hand trucks have rope wrapped on the handle; Guild ones have ochre and black tape.

Text: `LG-TOOL-0291` on the Guild frame. `HTR-HT-1156` on HTR. Hangtown trucks carry a kettle stencil and a house number in `#2E8B8B`. Ballast trucks carry a Meridian cargo stencil at 0.9 wear, grey frame.

Lighting: thin frame, so it reads by its tires and toe plate. Under line glow the painted frame catches a long highlight down each upright.

## Variants

| Variant | Difference |
|---|---|
| Red | Default. The Yard, Hangtown. |
| HTR orange | New tires, orange frame, tag. The Anchor, Ballast's Crown. |
| Guild ochre | Taped handle, bent toe plate. The Yard. |
| Loaded, cylinder | Propane cylinder strapped on, 80 kg total. Still under 100 kg: Scrapper bonus applies. Volatile travels with it. |
| Loaded, barrel | Full water barrel strapped on, 255 kg total. Tips over sideways at 14°. No bonus. |
| Flat tire | One tire flat; drags, veers right, rolling resistance 0.25 on that wheel. |

## Physics

- Tier Medium, 35 kg (05 §6.1 Medium band starts at 30 kg).
- Collision: frame box 50 × 8 × 125 cm; toe plate box 35 × 20 × 1 cm; two wheel cylinders 26 cm diameter × 7 cm. (**PROPOSED**)
- Center of mass: 45 cm above the floor, 5 cm behind the axle when standing. It balances on its wheels at 20° back-tilt. (**PROPOSED**)
- Friction 0.6 on the toe plate, rolling resistance 0.03 on tires, 0.15 on grating. Restitution 0.25 (it bounces on its tires). (**PROPOSED**)
- Break-free (05 §6.2): loose 0 N, anyone. Chained to a railing 800 N, Doubler and Breaker. A load strapped to it is in a cradle: 200 N, anyone, to pull the load off the truck. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Does not fracture. One wheel shears off at 1,200 N·s and becomes a Light prop. (**PROPOSED**)

Line acceleration and arrival, unloaded, other end static or braced: 400 N is 11.4 m/s², 600 N is 17.1, 800 N is 22.9, 1,000 N is 28.6, 1,200 N is 34.3 m/s². Every class reaches the 25 m/s Haul cap inside its line length (a Scrapper at 20 m without the bonus is 21.4 m/s, with it 25 m/s). Arrival is 25 m/s for everyone, 875 N·s, a flip.

Impact on a player, unloaded (impulse = 35 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 2.0 m/s (walking) | 70 N·s | Nothing |
| 8.6 m/s | 300 N·s | Stagger |
| 11.7 m/s (7 m drop) | 410 N·s | Stagger |
| 17.1 m/s | 600 N·s | Flip |
| 25 m/s (Haul cap, every class) | 875 N·s | Flip |
| 30 m/s (Jack cap) | 1,050 N·s | Flip |
| 34.3 m/s | 1,200 N·s | Stall; unreachable on Haul or Jack |
| 43 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall |

Loaded with a propane cylinder (80 kg): a Scrapper's 400 N is 5 m/s², 14.1 m/s over 20 m, times 1.3 is 18.4 m/s, 1,472 N·s: a Stall, and the cylinder armed at 8 m/s. Loaded with a full barrel (255 kg): Stall at 4.7 m/s; a Scrapper's 1.6 m/s² gets there after 7 m of line.

Jack (05 §3.2 v0.2): the truck is Kicked to V × M / (M + m) within 0.3 s (the Kick), then T / 35 kg to the 30 m/s cap. Scrapper (V 14): truck 10.6 m/s, Scrapper 3.4 back; then 11.4 m/s² over 20 m to 23.8 m/s (833 N·s, flip). Climber (V 18): truck 13.0, Climber 5.0; to the 30 m/s cap within 35 m of a 50 m line (1,050 N·s, flip). Rigger (V 12): truck 9.5, Rigger 2.5; cap within 24 m. Doubler (V 14): truck 11.0, Doubler 3.0. Breaker (V 10): truck 8.6, Breaker 1.4; 28.6 m/s² takes it to the cap within 14 m. The Scrapper bonus applies to the post-Kick push, not to V (05 §6.1: 1.69× force, ×1.3 arrival speed).

Crush: the truck's thin frame cannot deliver a crush; it folds. No line reaches 8 kN (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). A pinned player under a loaded truck takes the load's weight, not a crush, unless a kinematic is pushing.

## Interactions

- **Haul to it:** unloaded, the truck comes to the player at nearly the full line acceleration and the player barely moves (400 N on a 110 kg Scrapper is 3.6 m/s², under their 4 m/s² foot-hold: a skid). It arrives at the 25 m/s cap after 27 m of a Climber or Rigger line; on shorter lines, see the table. It arrives as a flip at best.
- **Jack from it:** weak launch. A Breaker keeps 1.4 m/s of V; a Climber 5.0.
- **Hang from it:** a Hang to a chained hand truck holds on the chain without starting it. A Hang to a loose one brings it along as 35 kg of dead weight, which a Climber barely notices. Hung overhead on a set line it is a 0.35 kN pendulum weight that staggers at the bottom of its swing only if it reaches 8.6 m/s, which needs a 3.8 m drop.
- **Object to object:** a hand truck carries anything placed on its toe plate under 1 m/s relative. The load is friction-held and travels with it. A hand truck into a market stall at over 8.6 m/s breaks the stall.
- **Kinematic:** a Hauler strike throws it 10 m. The Lift counterweight sweeps it off a landing.
- **Water:** sinks. The tires give it a slow sink; it lies on its back on the Hold floor.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A chained truck is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 35 kg truck feels 2× (cap) the authored acceleration (a_ref × min(100 / m, 2)): near the Stump the Updraft lifts it and tumbles it outward, and the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) rolls it out, then back toward the Well.
- **Breathing:** on its wheels it rolls downhill at 6°; on its toe plate it stays. Every hand truck in the Bays ends up at the low end of the deck by the second cycle.
- **Cut recoil:** 150 N·s per 1,000 N of force. A Breaker-line cut kicks the truck at 4.3 m/s; a Scrapper-line cut at 1.7 m/s. The truck twitches and rolls when its line is cut.
- Rolls when tipped back, stands on the toe plate otherwise, does not float.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Straight frame, two tires. | 35 kg, rolls. |
| Dented | Toe plate folded up 30°, one upright kinked. | Cannot carry a load; the cradle constraint is removed. |
| Broken | One wheel gone, frame on its side. | Drags at friction 0.6. Wheel is a Light prop. |
| Destroyed | Not reachable. | |

## VFX and SFX hooks

- Roll: `SFX_HandTruck_Roll_Steel`, `_Concrete`, `_Planks`, `_Grating` with a frame rattle and a soft-tire thrum.
- Impact: `SFX_HandTruck_Hit_Steel` (a tubular clang), `_Concrete`, `_Wood`. `SFX_HandTruck_Splash`.
- Break-free (chained): `SFX_BreakFree_Chain`. Load pulled off the cradle: `SFX_Strap_Snap`.
- Wheel shear: `NS_Impact_Steel_S`, `SFX_Wheel_Shear`.
- No emissive.

## Technical

- LOD0 6,000 tris (tire tread modeled), LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set.
- Pivot: bottom center of the footprint, at the toe plate and tire contact line. X+ is the toe plate direction.
- Dog sockets: `SKT_Dog_Top_01` (handle loop center) `Metal`; `SKT_Dog_Handle_01`, `_02` (handle grips) `Metal`; `SKT_Dog_Frame_01`, `_02` (uprights at 60 cm) `Metal`; `SKT_Dog_Frame_03` (axle center) `Metal`; `SKT_Dog_Load_01` (toe plate top face; a Dog here pulls the load off at 200 N) `Metal`; `SKT_Dog_Bot_01` (toe plate underside, for the tipped state) `Metal`.
- Not a geometry collection. Static mesh with a breakable wheel constraint and a load-cradle constraint (200 N) that accepts any Medium or volatile actor placed on the toe plate.
- Anchor field `FS_Anchor_HandTruck` releases chained instances at 800 N.
- Replication RC-M. The load is its own replicated actor; the cradle constraint is server-authoritative.

## Reference search terms

Two wheel hand truck steel pneumatic tires; red hand truck worn toe plate; dolly carrying gas cylinder strapped; sack truck bent toe plate scrapyard; hand truck loaded barrel; curved frame hand truck loop handle photo.
