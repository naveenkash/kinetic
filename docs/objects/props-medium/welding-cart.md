# Welding cart

Medium prop carrying two volatiles. Mass tier Medium, 140 kg total (50 kg cart, two 45 kg gas cylinders). 110 × 60 × 150 cm (L × W × H to the cylinder valves). Appears in the Yard (the Floor, at every cutting station along the trench edges; the Stacks, parked on the hull-section mesas), Ballast (the Crown, at HTR's idle cutting gantry; the Spine, on the ladder landings; the Hold, one overturned in the seep water), and the Anchor (the Scaffold, levels two and four).

## Role in play

A volatile on wheels. The prep crew's story is that a cutting crew was working here; the design's story is that a Scrapper can roll two propane-class cylinders into a room without ever touching them. Each cylinder uses the 05 §6.4 propane numbers: impact over 8 m/s triggers it, blast radius 4 m, impulse 1,500 N·s at center falling linearly to 0 at 4 m, Stall within 2 m and flip at 2 to 4 m as authored results that override the falloff. The second cylinder triggers on the first blast.

| Rig | Use |
|---|---|
| The Kiss | The cart is the Kiss delivered. Haul it into anything at over 8 m/s. |
| The Scatter | Dog the cart, Dog the enemy, Jack. No Kick fires against a Catch Plate, so it is a T push only: the cart accelerates from rest at T / 140 kg (2.9 m/s² on a Scrapper line, 11 m to arm; 7.1 m/s² on a Breaker line, 4.5 m). Dog the cart to a wall instead and the Kick fires: 14 m/s at once on a Scrapper line, armed from the first meter. |
| The Caboose | A Hauler through a welding cart is a Hauler through a blast. |
| The Guillotine | Hang the cart overhead (1.4 kN, within every class Hang load), then recall or cut your own set line. It falls, hits at 8 m/s from 3.3 m, and detonates on the floor. |

- **Scrapper:** rolls it. 400 N on 140 kg is 2.9 m/s²; the cart passes 8 m/s after 11 m of roll and arrives at 10.7 m/s at the end of a 20 m line. No 30% Scrapper bonus on the cart (over 100 kg). A Scrapper can also Dog one cylinder (200 N cradle break-free, anyone) and pull it out as a 45 kg volatile that does get the bonus: 8.9 m/s², 18.9 m/s over 20 m, times 1.3 is 24.6 m/s, 1,107 N·s plus a blast.
- **Rigger:** parks it in a doorway on a set line as an area denial. Nobody sprints through a room with a hissing cart.
- **Breaker:** pulls the cylinders out and leaves the cart. Or pushes the whole cart into a container stack: 1,000 N is 7.1 m/s², 8 m/s after 4.5 m.
- **Climber:** Jacks off the cart from above and keeps 11 m/s of their 18 m/s V. Never stands near it after.
- **Doubler:** a line on each cylinder, Jack. 480 N per split line clears the 200 N cradle break-free; both cylinders fly apart. The Scatter in one input.

## Appearance

Silhouette: a two-wheeled upright cart with a tall back frame, two cylinders strapped side by side with a chain, a hose coil on a hook, a torch on a bracket, a regulator pair at the valves. The tall twin-cylinder shape reads from 50 m.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Cart frame | `MI_Steel_Painted_Worn` | `#5B6670` Guild grey or `#E8620C` HTR | 0.45–0.70 | 0 / 1 at chips |
| Wheels | `MI_Rubber_Black` with `MI_Steel_Bare` hubs | `#1E1F20` | 0.70–0.85 | 0 |
| Cylinders | `MI_Cylinder_Propane_Grey` | `#B8B4A8` | 0.40–0.60 | 0 |
| Cylinder collars, valves | `MI_Steel_Bare` with brass tint `#8F7A3E` on the valve | | 0.40–0.55 | 1 |
| Hose | `MI_Rubber_Black`, one red `#8E2A1F`, one green `#3E7A4E` | | 0.75 | 0 |
| Chain | `MI_Chain_Oiled` | `#3C3E42` | 0.30–0.45 | 1 |
| Stencils | `MI_Decal_Stencil` | `#E6E4DD`, `#141414` | inherits | 0 |

Wear: cylinder paint scraped to bare steel on the foot ring and dented at shoulder height from being dropped. Spatter burns on the cart frame. Hose grimed black at the torch end. The push handles polished bright. A cut-off disc wired to the frame as a spare.

Text: cylinders stencilled `RUCU GAS / PROPANE / 45 KG / RETURN EMPTY` (**PROPOSED** supplier) with an Office tag `OFFICE OF THE LAY / SITE PROPERTY / M-22`. HTR carts carry `HTR-WC-0088` on the frame. A hand-lettered tag on the hose: `LEAKS. BLEED FIRST.` in Guild hand.

Lighting: the grey cylinders are the brightest matte surface on the Yard floor. Valves catch line glow. In Ballast's Crown, salt white on the cylinder shoulders.

## Variants

| Variant | Difference |
|---|---|
| Guild | Grey frame, ochre and black tape on the handles. The Yard. |
| HTR | Orange frame, new cylinders, asset tag. Ballast's Crown and the Anchor. |
| One-cylinder | Second cradle empty, 95 kg. Under 100 kg, so the Scrapper bonus applies. Spawns after a prep-crew "removal" in the Spine. |
| Spent | Both cylinders present but valves painted red and tagged `EMPTY`. Cylinders are inert, 20 kg each, Light. Cart total 90 kg. A deliberate bluff the prep crew places once per match. (**PROPOSED**) |
| Burning | Post-blast remnant: the frame, blackened, no cylinders. 50 kg. |

## Physics

- Tier Medium, 140 kg total; cart 50 kg; cylinders 45 kg each. Tier recomputes as cylinders leave (05 §6.3).
- Collision: frame box 110 × 60 × 150 cm, two wheel cylinders 40 cm diameter × 8 cm; cylinder bodies as two capsules 120 cm tall, radius 15 cm, as separate bodies constrained to the cart. (**PROPOSED**)
- Center of mass: 65 cm above the floor, 10 cm forward of the axle toward the cylinders. The cart balances on its wheels at 12° back-tilt; it rests on a front foot otherwise. (**PROPOSED**)
- Friction 0.6 on the foot, rolling resistance 0.05 on wheels. Restitution 0.15. (**PROPOSED**)
- Break-free (05 §6.2): cart loose on floor 0 N, anyone; cart chained to a station 800 N, Doubler and Breaker; each cylinder in the cradle 200 N, anyone. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Volatile (05 §6.4, per cylinder): trigger impact over 8 m/s to the cylinder, or another blast. Blast radius 4 m, impulse 1,500 N·s at center to 0 at 4 m. Players within 2 m Stalled, 2 to 4 m flipped. Hiss tell 0.3 s before the blast. The second cylinder is 30 cm from the first and triggers on it; two overlapping blasts of the same radius, 0.3 s apart. Under more than 1 m of water (Ballast's Hold) each blast delivers half impulse, 750 N·s at center, and there is no fire patch.
- Does not fracture. The frame is a non-fracturing Medium after the blast.

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d)), whole cart:

| Class | Tension | Acceleration | Line | Arrival | Distance to 8 m/s |
|---|---|---|---|---|---|
| Climber | 400 N | 2.9 m/s² | 50 m | 16.9 m/s | 11 m |
| Scrapper | 400 N | 2.9 m/s² | 20 m | 10.7 m/s | 11 m |
| Rigger | 600 N | 4.3 m/s² | 40 m | 18.5 m/s | 7.5 m |
| Doubler | 800 N | 5.7 m/s² | 30 m | 18.5 m/s | 5.6 m |
| Doubler overspin | 1,200 N | 8.6 m/s² | 30 m | 22.7 m/s | 3.7 m |
| Breaker | 1,000 N | 7.1 m/s² | 20 m | 16.9 m/s | 4.5 m |

Impact on a player, whole cart intact (impulse = 140 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 2.0 m/s (walking) | 280 N·s | Nothing |
| 2.1 m/s | 300 N·s | Stagger |
| 4.3 m/s | 600 N·s | Flip |
| 8.0 m/s | 1,120 N·s | Flip, and the cylinders arm |
| 8.6 m/s | 1,200 N·s | Stall by impact, then a blast on top |
| 10.7 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall; arms |
| 11.7 m/s (7 m drop) | 1,638 N·s | Stall; blast |
| 22.7 m/s (fastest Haul arrival, overspin) | 3,178 N·s | Stall; blast |

A player hit by the cart at over 8 m/s takes the impact impulse and is then at 0 m from a blast. There is no survivable welding-cart hit above 8 m/s.

Jack (05 §3.2 v0.2): the cart is Kicked to V × M / (M + m) within 0.3 s (the Kick), then T / 140 kg. Climber (V 18): cart 7.0 m/s, Climber 11.0 back. Scrapper (V 14): cart 6.2, Scrapper 7.8; the cart then passes 8 m/s within 4 m. Rigger (V 12): cart 5.8, Rigger 6.2. Doubler (V 14): cart 6.7, Doubler 7.3. Breaker (V 10): cart 6.1, Breaker 3.9; 8 m/s within 2 m. A Jacked cart is armed before it reaches anything.

Crush: no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). The cart folds under a kinematic rather than crushing; a crush that deforms the cylinder bodies counts as an impact at the closing speed and triggers over 8 m/s.

## Interactions

- **Haul to it:** both ends move. A Scrapper (110 kg, 3.6 m/s², under the 4 m/s² foot-hold, skidding) and the cart (2.9 m/s²) close at 6.5 m/s². From 10 m the closing speed at contact is 11.4 m/s: the cart detonates on the Scrapper. From 4 m it is 7.2 m/s: no blast, and a 1,000 N·s flip. The window is the Haul ramp plus about 1.5 s.
- **Jack from it:** launch, and the cart rolls away armed. If it hits a wall, the blast reaches back 4 m.
- **Hang from it:** a Hang to a chained cart or to a cradled cylinder holds without starting either; only Haul, Jack, and the Kick test break-free. A Hang to a loose cart drags it, cylinders and all, as 140 kg (1.4 kN) of dead weight: two volatiles swinging at the player's hip.
- **Object to object:** a cart rolling into a fuel drum at over 10 m/s triggers the drum and both cylinders. A cart in a propane cylinder rack sets off the rack.
- **Kinematic:** a Hauler through a cart is a 12 m/s impact: blast in the trench, 4 m radius, timed to the klaxon.
- **Water:** sinks upright. In the Hold under more than 1 m of water the blasts are half impulse and no fire.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A chained cart is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust); cradled cylinders go with the cart. A loose 140 kg cart feels 0.71× the authored acceleration (a_ref × min(100 / m, 2)) and rolls on the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55): out, then back. On level two the outward push takes it over the edge and it detonates on the Rim.
- **Breathing:** rolls downhill at 6° on its wheels if tipped back; on its foot it stays. At the Shear it goes into the gorge.
- **Cut recoil:** 150 N·s per 1,000 N, about 1 m/s on the cart. On a loose cylinder it is 3.3 m/s, under the trigger.
- Rolls when tipped, stands when upright, does not float.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Two cylinders, chain, hoses. | 140 kg, three bodies constrained. |
| Dented | Cylinder shoulder dented, hose torn, hissing faintly. | No change. The hiss is cosmetic; arming only happens on the 8 m/s tell. |
| Broken | Chain gone, one or both cylinders out of the cradle. | Cart 50 kg plus loose 45 kg volatiles; all three under 100 kg, Scrapper bonus applies to each. |
| Destroyed | Blackened frame, no cylinders, scorch decal 4 m. | Frame 50 kg Medium, non-fracturing. |

## VFX and SFX hooks

- Roll: `SFX_WeldCart_Roll_Steel`, `_Concrete`, `_Planks`, with a cylinder-chain rattle layer.
- Impact: `SFX_Cylinder_Clang` (the loud hollow ring that makes everyone turn), `SFX_Cart_Hit_*` by surface.
- Arm: `SFX_Volatile_Hiss` 0.3 s, `NS_Volatile_Vent` white jet from the valve.
- Blast: `NS_Blast_Propane_4m` (fireball, pressure ring, 30 Light sparks), `SFX_Blast_Propane`, camera shake 4 m. Second blast 0.3 s later. `NS_Scorch_Decal_4m`. Underwater: `NS_Blast_Propane_Sub` (bubble dome, no fire), `SFX_Blast_Sub`.
- Break-free (cylinder from cradle): `SFX_BreakFree_Chain` and the cradle strap snap.
- No emissive except the torch pilot: a 0.5 emissive blue point on the torch tip, Yard only.

## Technical

- LOD0 6,000 tris for the cart and 2,500 per cylinder (the cylinders are instances of the propane cylinder asset), LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set for the cart; cylinders use their own.
- Pivot: bottom center of the footprint at the wheel contact line. X+ is the push-handle side.
- Dog sockets on the cart: `SKT_Dog_Top_01` (frame top bar) `Metal`; `SKT_Dog_Handle_01`, `_02` (push handles) `Metal`; `SKT_Dog_Frame_01`, `_02` (axle ends) `Metal`; `SKT_Dog_Load_01`, `_02` (one per cylinder body, mid height; a Dog here pulls the cylinder against the 200 N cradle break-free) `Metal`; `SKT_Dog_Valve_01`, `_02` (cylinder valves; louder hiss) `Metal`.
- Not a geometry collection. Static mesh cart with two constrained cylinder actors (`BP_PropaneCylinder` instances) and a chain constraint that breaks at 200 N.
- Anchor fields: `FS_Anchor_WeldingCart` releases chained instances at 800 N; `FS_Anchor_WeldingCart_Cradle` releases each cylinder at 200 N.
- Replication RC-M for the cart and each cylinder. The blast is a server event; clients predict the fireball on the hiss tell and receive the impulse as a wake.

## Reference search terms

Oxy acetylene welding cart two cylinders; cutting torch cart chain strapped; shipyard gas cart hoses coiled; welding cart dented cylinders scrapyard; cylinder trolley two wheel upright; cutting station demolition site photo.
