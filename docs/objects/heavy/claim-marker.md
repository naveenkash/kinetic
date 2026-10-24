# Claim marker

Heavy. Heavy tier, 900 kg: a Meridian Weave billet strapped to a steel pallet, with an Office beacon post. Pallet 120 × 100 × 15 cm, billet 100 × 60 × 40 cm, beacon post to 180 cm. The 4 m radius capture ring is projected around it. One per arena in Claim mode: Ballast, the Bays (the central catwalk landing); the Yard, the Stacks (the mesa nearest the press); Hangtown, the Mid Hang (the market ring's center); the Anchor, the Crown.

## Role in play

The objective of Claim mode. A crew with at least one rigger inside the 4 m ring and no enemies inside earns 1 Claim point per second; first to 100 wins. It is physical and Heavy. It can be moved by lines, which is legal: per 05 §6.1a, "The Claim marker sits on polished runners (0.05): a Rigger or heavier moves it, slowly." A crew pulling together moves it faster. If it enters a Well it resets to its authored position. Everything else about it is a 900 kg block with a prize on top.

Why it is there: the Office places a sample billet of the contested Claim's Weave on the ground. The crew that holds the billet holds the Claim. This is the Accord in one object.

| Harness | What they do with it |
|---|---|
| Scrapper | 400 N is under the 441 N start force on the runners: the line sings and the marker stays. Adds as a vector to a crewmate's pull. Stands on it; Kisses cylinders around it. |
| Rigger | 600 N moves it: 0.18 m/s² net, 3.8 m/s at the end of a 40 m line. Hangs the ring: set lines across every approach, then stands inside. |
| Breaker | 1,000 N is 0.62 m/s² net: 5 m/s at the end of a 20 m line, 4,500 N·s to anyone it reaches. Onto a pinned enemy it is a crush. |
| Climber | Jacks off the beacon post to reach the Crown: the Kick, V 18 split against 900 kg, is 16.4 m/s for the Climber. Perches on the billet. |
| Doubler | Two lines, one on the marker and one on an enemy, Hang: the enemy is tethered to a 900 kg block on a 40° market platform. |

## Appearance

Silhouette: a low, dense, dark rectangular block on a pallet with a thin post rising from one corner and a flat beacon head. The billet is the darkest, cleanest, most expensive-looking thing in any arena. The ring is light on the floor, not geometry.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Billet | `MI_Weave` | `#1A2430`, iridescent `#3A6E7F` at grazing angles | 0.18 to 0.28 | 1, anisotropy 0.8 along the lattice |
| Pallet | `MI_Steel_Galvanized` | `#9EA3A6` | 0.35 to 0.55 | 1 |
| Runners (2, polished) | `MI_Steel_Bare` | `#5C5F63` | 0.25 | 1 |
| Straps (4) | `MI_Steel_Bare` ratchet straps, `#5C5F63` | | 0.5 | 1 |
| Beacon post | `MI_Office_White` | `#E6E4DD` | 0.55 | 0 |
| Beacon head | `MI_Glass_Lamp` | | 0.05 | 0 |
| Corner guards | `MI_Tape_Rated` is NOT used; the marker is Heavy, not Rated. Corners are `MI_HTR_Grey_Matte` `#4A4D50` | | 0.65 | 0 |

Wear: the billet does not wear. The pallet runners are mirror-bright from being dragged every match; the Office polishes and greases them to a 0.05 friction because the Accord says the marker must be movable. Boot prints on the billet's top face in `MI_Decal_Grime`. The post is scratched where Dogs seat.

Markings: `OFFICE OF THE LAY / CLAIM SAMPLE / DO NOT CUT` in black `#141414` on the post, 3 cm letters. A Claim number stencilled on the pallet end in white: `CLAIM 61-04` (the season and the arena). The billet carries a laser-etched Weave lot code `MW-C4-0917` on one face, 1 cm letters, read only up close. A LayCast sponsor plate on the pallet's camera side: `MERIDIAN MUTUAL / INSURING THE LAY SINCE THE LAY`, green on white. No faction marks; the Office owns it.

Beacon: the head emits 0.3 units in the Office's white when uncontested, shifting to the holding crew's broadcast color when a crew is scoring. Kept at 0.3 so a singing line is always brighter; the 4 m ring on the floor is a light function at 0.5 units with a 1 Hz breath. In a darkened sector (a Yard shack down, an Anchor tower down) the ring is visible and the billet is not.

Per arena: the Ballast marker stands on a catwalk landing; the Yard's is dusted with slag; Hangtown's sits on timber and leaves a dent; the Anchor's is salt-crusted and lives in the wind on the Crown.

## Variants

| Variant | Difference |
|---|---|
| Standard (default) | As above. |
| Held, crew-lit | Beacon and ring in the scoring crew's color. Same physics. |
| Contested | Beacon and ring flash white at 2 Hz. Same physics. |
| Reset | Appears at its authored position with a 1 s `NS_Marker_Reset` shimmer after entering a Well. |
| Finals (Anchor) | Billet is 20% larger in visual trim (same collision), the post carries the previous season's winning Face's name on a plate. |

## Physics

Heavy tier, 900 kg. Collision: pallet box 120 × 100 × 15 cm; billet box 100 × 60 × 40 cm on top; post as a capsule radius 3 cm, length 165 cm, no collision against players above the billet (so a Climber can perch). Center of mass 32 cm above the floor, centered on the billet. Friction 0.05 on the polished steel runners (05 §6.1a). On Hangtown's market ring the Office lays a steel plate under the marker so the runners still ride at 0.05 (**PROPOSED**). Restitution 0.02. Tips past 48°.

Break-free: loose on floor, 0 N. It is never racked or bolted.

Start force on the runners: 0.05 × 8.83 kN of weight = 441 N, tested against the vector sum of every line on the marker (05 §9.4). A single line under 441 N sings and the marker stays; two Scrappers pulling the same way (800 N) start it. Above it, net acceleration is (T − 441 N) / 900 kg, and the runners decelerate a coasting marker at 0.49 m/s². All under 1 g; it slides, it never flies.

| Puller | T | Moves? | Net acceleration | Speed at line end | Impulse into a player |
|---|---|---|---|---|---|
| Climber | 400 N | No | | | |
| Scrapper | 400 N | No | | | |
| Rigger | 600 N | Yes | 0.18 m/s² | 3.8 m/s at 40 m | 3,420 N·s, Stall |
| Doubler | 800 N | Yes | 0.40 m/s² | 4.9 m/s at 30 m | 4,410 N·s |
| Doubler overspin | 1,200 N | Yes, for 1.8 s | 0.84 m/s² | 1.5 m/s at the snap | 1,350 N·s |
| Breaker | 1,000 N | Yes | 0.62 m/s² | 5.0 m/s at 20 m | 4,500 N·s |
| Crew of three, 2,000 N (05 §6.2a) | | Yes | 1.73 m/s² | 5.9 m/s after 10 m | 5,310 N·s |

A Breaker alone takes 8 s to walk the marker 20 m; a crew of three moves it 10 m in 3.4 s. Two Scrappers together (800 N as a vector) move it like one Doubler. Once sliding at 5 m/s it coasts 25 m on the runners.

As a projectile: 900 kg. 300 N·s at 0.33 m/s, 600 at 0.67 m/s, 1,200 at 1.33 m/s. A sliding marker at 2 m/s is 1,800 N·s: Stall. At 5 m/s (the end of a Breaker's line) it is 4,500 N·s. A 7 m drop onto a player, 11.7 m/s, is 10,530 N·s. It does not need speed. A full-overspin snap delivers 1,500 N·s to it, 1.7 m/s, enough to Stall whoever is against it. Crush: 8.8 kN of weight, over the 8 kN crush threshold, which no line reaches and which comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1). A player pinned under the marker for 0.5 s is Stalled, with no line force required.

Not destructible. A demolition charge at 0 m (2,500 N·s) moves it 2.8 m/s and scratches the pallet.

## Interactions

Haul to it: the player is pulled to the marker, a grapple into the ring. On a Rigger's line or heavier the marker also slides toward the puller. Jack from it: a Slingshot off 900 kg; the Kick gives the player nearly all of V (a Scrapper 12.5 of 14 m/s) and the marker the rest, 1.5 m/s, then it slides to a stop in 2.3 m on the runners. Jack from the marker to an enemy's Catch Plate: no Kick (05 §3.2), a T push only; the enemy skids at T over their mass and the marker stays under its 441 N start force on any line but a Rigger's or heavier. Hang from it: a 900 kg Pendulum anchor that does not start moving under a Hang, because break-free is tested against active pull only (05 §6.2). A Hung enemy tethered to it is tethered to the ring; a hanging player climbs onto the billet by Reel. On a sloped surface (the market ring on Cable Day, at 40°) it becomes the Anchor Drop: tan 40° is 0.84, far over 0.05, so the marker slides toward the low edge at 5.8 m/s² and takes whoever is Hung to it.

Object to object: Heavy props Hauled into it stop; Medium props bounce. A crate stack Avalanched onto the marker buries it; the ring still counts, because the ring is at the marker's position and a buried marker is a design feature, not a bug.

Kinematics: Ballast's Breathing tilts the catwalk landing 6°; sin 6° is 1.02 m/s², over the runners' 0.49 m/s² of friction, so an unchecked marker would slide 0.53 m/s² toward the low side on every lean (**PROPOSED**: the Ballast landing has a 5 cm lip on both sides; the marker shudders against it and stays). A Hauler never runs through a marker position, but a Hauler passing through a line tied to the marker severs it (05 §3.5). The Anchor's Pulse is a force volume: 0.4 s of outward shove from the Stump, then 0.6 s of inhale toward it, every 75 s from 0:55. Force volumes act on free bodies scaled a_ref × min(100 / m, 2) (05 §6.6), so the 900 kg marker feels 0.11 times the authored acceleration and slides only if a_ref exceeds 4.4 m/s² (0.49 / 0.11). **PROPOSED**: it does, and because the inhale is longer than the shove the net drift is toward the Crown's center. The Pulse gathers the marker rather than throwing it; only a crew's line at the wrong moment sends it over the edge, which is the Crown fight.

Water: sinks. The Hold's seep water is 40 cm deep and the marker is never placed there. Updraft: 0.11 times the authored acceleration; it does not lift 900 kg. The marker's post whistles. Breathing tilt: covered above.

Well: entering any ring-out volume triggers an immediate reset to the authored position. The reset is instant on the server, 1 s shimmer on clients, and the ring resumes scoring after 2 s (**PROPOSED**).

## States

| State | Visual | Physics |
|---|---|---|
| Intact | As above. | Full. |
| Dented | Pallet corner folded, one strap snapped and trailing. After any impact over 2,000 N·s. | Unchanged. Cosmetic. |
| Buried | Under props. Beacon still visible through the ring light. | Unchanged. Props on it add their mass to the crush case. |
| Resetting | 1 s shimmer at the authored position. | Collision off for 1 s. |

No Broken or Destroyed state. The billet cannot be cut; the First Rule and the Office say so, and the beacon post says so in 3 cm letters.

## VFX and SFX hooks

- Impact on it: a dead, heavy knock with a faint high ring from the Weave. On timber, a crack of the deck. Sliding: a long greased-steel hiss with strap rattle, pitch by speed.
- Break-free: none (0 N). The first movement plays `SFX_Obj_Drag_Marker` and `NS_Drag_Dust_Medium`.
- Scoring: `SFX_Marker_Tick` once per second, audible within 10 m, faction-neutral. Contested: `SFX_Marker_Contest`, a 2 Hz double pulse.
- Reset: `NS_Marker_Reset` (a white ring collapsing inward, 1 s), `SFX_Marker_Reset`, the Marshal's "marker reset" bark.
- Emissive: beacon 0.3 units, ring light function 0.5 units at 1 Hz. Weave iridescence is specular, not emissive.

## Technical

LOD0 16,000 tris (the straps and ratchets cost; the billet is 200), LOD1 8,000, LOD2 3,000, LOD3 600. Two 2K sets (billet and post; pallet and straps) plus `T_Trim_Steel_01`. Nanite on. Pivot at the bottom center of the pallet footprint. X+ is the beacon post corner.

Dog sockets:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Hook_01` | Pallet fork pocket, X+ | Metal |
| `SKT_Dog_Hook_02` | Pallet fork pocket, X- | Metal |
| `SKT_Dog_Top_01` | Billet top face, center | Metal (Weave counts as metal for Mag Dogs, **PROPOSED**) |
| `SKT_Dog_SideN_01` | Billet face, X+ | Metal |
| `SKT_Dog_SideS_01` | Billet face, X- | Metal |
| `SKT_Dog_SideE_01` | Pallet rail, Y+ | Metal |
| `SKT_Dog_SideW_01` | Pallet rail, Y- | Metal |
| `SKT_Dog_Frame_01` | Beacon post, 150 cm | Metal |

Not a geometry collection. No anchor field. The 4 m ring is a sphere overlap on the marker's root, radius 400 uu, evaluated on the server at 10 Hz for Claim scoring. Multiple line forces on the body are summed by Chaos as ordinary applied forces, so the vector-add rule needs no special code; the 441 N start test reads that summed force (05 §9.4). Replication RC-H: server-authoritative, 15 Hz, asleep until a line or contact wakes it; the reset is a server teleport replicated as a wake event. The ring light and beacon color are replicated as a single byte of scoring state.

## Reference search terms

carbon fiber billet block dark anisotropic; steel pallet galvanized ratchet straps heavy load; skid pallet polished steel runners; survey beacon post white; salvage marker buoy post; graphite block polished; drag marks pallet concrete floor; racing series timing beacon.
