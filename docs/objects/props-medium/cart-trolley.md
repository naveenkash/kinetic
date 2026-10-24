# Cart trolley

Medium prop. Mass tier Medium, 120 kg. 150 × 80 × 100 cm (L × W × H to the push bar). Appears in the Yard (the Floor, beside scrap heaps and at the press apron), Hangtown (the Mid Hang, the market platform), and the Anchor (the Scaffold, on the wider contractor decks).

## Role in play

A flatbed on four casters. It is the only Medium that rolls freely on flat ground, which makes it a payload, a vehicle, and a projectile that keeps going after the line lets go. Because it rolls, it is also the Medium that gets closest to the speed caps on a line. The trade says anything with a cart in it is a Scrapper rig.

| Rig | Use |
|---|---|
| The Caboose | Haul an enemy to a cart you have already sent rolling toward a trench. |
| The Kiss | Load a propane cylinder on it. The cylinder arrives at the cart's speed. |
| Overboard | Dog the cart, Dog the enemy, Jack. No Kick fires against a Catch Plate, so it is a T push only: the cart rolls back at T / 120 kg (3.3 m/s² on a Scrapper line) while the enemy skids at T / their mass toward the edge. Park the cart against a wall so the push goes to them. |
| The Clothesline | A cart's push bar is a solid anchor at 1.0 m height. |

- **Scrapper:** the signature. Dogs a cart and a barrel, Hauls, re-dogs the cart to an enemy before it stops. 400 N on 120 kg is 3.3 m/s²; the cart reaches 11.5 m/s over the Scrapper's 20 m line, a Stall. No 30% bonus; the cart is over 100 kg.
- **Rigger:** Hangs a cart off a railing as a swinging barricade. 600 N over 40 m brings it in at 20 m/s.
- **Breaker:** ignores it until it is loaded. Then it is a 400 kg train.
- **Climber:** rides it. Perches on the deck and Jacks off the push bar.
- **Doubler:** one line on the cart, one on the enemy, Haul both. The enemy meets the cart in the middle.

## Appearance

Silhouette: low rectangular deck on four swivel casters, a tubular push bar at one end, a raised lip around three sides. Reads as a hospital trolley that has been through a war.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Deck plate | `MI_Steel_Painted_Worn` | `#5B6670` | 0.50–0.70 | 0 / 1 at chips |
| Frame and push bar | `MI_Steel_Galvanized` | `#9EA3A6` | 0.35–0.55 | 1 |
| Caster wheels | `MI_Rubber_Black` | `#1E1F20` | 0.70–0.85 | 0 |
| Caster forks | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 | 0.3 |
| Deck lip | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 | 1 |

Wear: push bar polished to a shine at both hand positions and bare at the grips. Deck plate scraped in parallel lines where loads were dragged on. Rust around every caster bolt. Rubber wheels flat-spotted on one side. Tape wrapped on the push bar: ochre and black on Guild carts, whatever was in the bin on Hangtown carts.

Text: Guild carts carry `LG-1180` stencil on the deck lip. Hangtown carts have a neighborhood stencil (a kettle) on the deck in `#B5302A`. Yard carts have `COLLAR FOUR PRESS WORKS` on the push bar (**PROPOSED** supplier).

Lighting: the galvanized frame picks up line glow strongly. The deck is a dark plane. Under Hangtown's Office lights the mustard tape is the brightest element.

## Variants

| Variant | Difference |
|---|---|
| Guild | Grey deck, ochre and black bar tape, `LG-` stencil. The Yard. |
| Hangtown | Deck repainted `#2E8B8B` teal, one caster replaced with a wooden wheel, rope lashings. |
| HTR | `#4A4D50` deck, orange push bar, all four casters new. The Anchor. |
| Loaded | Spawns with a propane cylinder strapped on (combined 165 kg) or a tool bag (140 kg). |
| Three-wheel | One caster missing. Drags a corner, veers left. Spawns in scrap heaps. |

## Physics

- Tier Medium, 120 kg unloaded.
- Collision: deck box 150 × 80 × 12 cm at 25 cm height; push bar as a capsule 80 cm long, radius 2 cm, at 100 cm; four wheel spheres radius 10 cm. Wheels are physics-constrained casters that swivel freely. (**PROPOSED**)
- Center of mass: 30 cm above the floor at deck center. Tips over sideways at 22° on a slope. (**PROPOSED**)
- Rolling resistance 0.04 on steel and concrete, 0.12 on planks, 0.30 on grating (wheels drop into the slots). Sliding friction 0.5 when a wheel is locked. Restitution 0.2. (**PROPOSED**) Rolling resistance costs 0.4 m/s² of the line's acceleration on steel; the arrival speeds below ignore it.
- Break-free (05 §6.2): loose on floor 0 N, anyone. Chained to a railing 800 N, Doubler and Breaker only; a Scrapper cannot steal a chained cart. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Does not fracture. Casters shear off at 1,500 N·s, one per hit, and become Light props. (**PROPOSED**)

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d)):

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 3.3 m/s² | 50 m | 18.2 m/s | 2,184 N·s |
| Scrapper | 400 N | 3.3 m/s² | 20 m | 11.5 m/s | 1,380 N·s |
| Rigger | 600 N | 5.0 m/s² | 40 m | 20.0 m/s | 2,400 N·s |
| Doubler | 800 N | 6.7 m/s² | 30 m | 20.0 m/s | 2,400 N·s |
| Doubler overspin | 1,200 N | 10.0 m/s² | 30 m | 24.5 m/s | 2,940 N·s |
| Breaker | 1,000 N | 8.3 m/s² | 20 m | 18.3 m/s | 2,196 N·s |

Only an overspun Doubler brings the cart close to the 25 m/s cap. Every full-line arrival is a Stall.

Impact on a player (impulse = 120 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 2.0 m/s (walking) | 240 N·s | Nothing |
| 2.5 m/s | 300 N·s | Stagger |
| 5.0 m/s | 600 N·s | Flip |
| 10.0 m/s | 1,200 N·s | Stall; a Scrapper needs 15 m of run-up, a Rigger 10 m, a Breaker 6 m |
| 11.7 m/s (7 m drop) | 1,404 N·s | Stall |
| 12.5 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall |
| 24.5 m/s (fastest Haul arrival, overspin) | 2,940 N·s | Stall |

A loaded cart adds the load's mass. Cart plus propane cylinder (165 kg) Stalls at 7.3 m/s, and the cylinder arms at 8 m/s on the same hit. A Scrapper's 400 N on 165 kg is 2.4 m/s², 9.8 m/s over 20 m: both happen.

Jack (05 §3.2 v0.2): the cart is Kicked to V × M / (M + m) within 0.3 s (the Kick), then keeps accelerating at T / 120 kg and keeps rolling after release. Climber (V 18): cart 7.7 m/s, Climber 10.3 back; then 3.3 m/s² over 50 m to 20 m/s. Scrapper (V 14): cart 6.7, Scrapper 7.3; then to 13.3 m/s over 20 m. Rigger (V 12): cart 6.2, Rigger 5.8. Doubler (V 14): cart 7.3, Doubler 6.7. Breaker (V 10): cart 6.5, Breaker 3.5; then 8.3 m/s² over 20 m to 19.4 m/s. A Jacked cart is a flip at the start and a Stall by the end of the line.

Crush: no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). The cart's weight is 1.2 kN. Only a kinematic crushes with it: a Hauler pinning a cart against a trench wall.

## Interactions

- **Haul to it:** a loose cart rolls to the player faster than the player moves on every class but Climber. 400 N on a 110 kg Scrapper is 3.6 m/s², under the 4 m/s² foot-hold, so the Scrapper skids while the cart rolls in at 3.3 m/s². Closing acceleration 6.9 m/s².
- **Jack from it:** the inverse-mass split above. A Climber off a cart takes 10.3 m/s of their 18 m/s V; a Breaker takes 3.5 of 10.
- **Hang from it:** a Hang to a chained cart does not start it; the chain carries the load up to the class Hang load. A Hang to a loose cart brings it along as 120 kg of dead weight on wheels: hung from a player on an overhead anchor it is a pendulum weight, and a Hang between cart and player with the cart on the ground makes a rigid tow bar, the player walks and the cart follows.
- **Object to object:** carts carry. Anything on the deck under 1 m/s relative is friction-held and travels with it.
- **Kinematic:** a Hauler strike at 12 m/s delivers the full relative impulse, 1,440 N·s to the cart, and throws it. The Lift counterweight sweeps carts off the Mid Hang walkways. A Hauler passing through a line that holds a cart on the Floor severs it (snap, both Dogs release); lines attached to the Hauler itself are not severed.
- **Water:** sinks. Rolls on the Hold floor with the flood.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A chained cart is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 120 kg cart feels 0.83× the authored acceleration (a_ref × min(100 / m, 2)) and, on casters, rolls on it: outward for 0.4 s, then toward the Stump on the 0.6 s inhale, every 75 s from 0:55. On the inner Scaffold edge the inhale takes it into the Well.
- **Breathing:** rolls downhill at 6°. Every 45 s it changes its mind. Carts in the Bays are never where they were.
- **Cut recoil:** 150 N·s per 1,000 N, 1.25 m/s on the cart at most. It rolls a meter and stops.
- Rolls freely, slides when tipped, does not float.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Four casters, straight bar. | Rolls. |
| Dented | Deck bowed, bar bent 15°. | No change. |
| Broken | One or more casters sheared, cart on its frame corner. | Drags, friction 0.5. Veers. |
| Destroyed | Not reachable. | |

## VFX and SFX hooks

- Rolling: `SFX_Cart_Roll_Steel`, `_Concrete`, `_Planks`, `_Grating` looped by surface and speed. A loose caster rattle layer on the broken state.
- Impact: `SFX_Cart_Hit_Steel`, `_Concrete`, `_Wood`; `NS_Impact_Steel_M` sparks over 600 N·s.
- Break-free (chained): chain snap, `SFX_BreakFree_Chain`. Under 800 N the chain hums and the cart rocks on its casters.
- Caster shear: `NS_Caster_Shear` sparks and the wheel prop spawn.
- No emissive.

## Technical

- LOD0 6,000 tris, LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set.
- Pivot: bottom center of the footprint at floor contact. X+ is the push-bar end.
- Dog sockets: `SKT_Dog_Top_01`, `_02` (deck, front and rear) `Metal`; `SKT_Dog_Handle_01` (push bar center) `Metal`; `SKT_Dog_Frame_01` to `_04` (deck lip corners) `Metal`; `SKT_Dog_SideN_01` (bar end) `Metal`; `SKT_Dog_SideS_01` (far end) `Metal`.
- Not a geometry collection. Static mesh with four physics-constraint casters.
- Anchor field `FS_Anchor_CartTrolley` releases chained instances at 800 N.
- Replication RC-M: server-authoritative, 20 Hz. Caster swivel is not replicated; clients derive it from velocity.

## Reference search terms

Industrial platform trolley swivel casters; hospital flatbed cart steel; foundry transfer cart; galvanized tube push bar wear; flat-spotted rubber caster; scrapyard hand cart teal paint.
