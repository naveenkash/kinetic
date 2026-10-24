# Dogs

_Rigging grips. The tether nodes. "Dog it" means attach. Every line has two: Dog A and Dog B. A Doubler carries four._

## Physical description

A Dog is a projectile with gravity, 0.08 m collision radius (05 §3.1), fired from the Spool at class speed. In the hand it is a grip the size of a fist: a body, a set of jaws or a face that bites the surface, and a swivel at the tail where the line attaches. All four sidegrade types share the collision sphere; only the visible mesh and the seating logic differ.

| Type | Visible form | Dimensions (**PROPOSED**) | Seats in | Tradeoff |
|---|---|---|---|---|
| **Standard** | Cylinder with three spring jaws | 14 cm long, 7 cm diameter | Anything | Baseline |
| **Barb** | Cylinder with four hooked flukes that flare on impact | 14 cm long, 7 cm closed, 11 cm flared | Anything. Seats in Catch Plates from any angle, harder to shake | Lower max load on structure |
| **Mag** | Flat disc face, no jaws, a coil visible behind the face | 10 cm long, 9 cm diameter | Metal only. Bounces off concrete, wood, rock | Higher max load on metal |
| **Spike** | Tapered hardened point with two backward barbs | 16 cm long, 6 cm diameter | Concrete, wood, rock. Slides on metal | Higher max load on concrete, wood, rock |

### Faction finishes

Finish changes the mesh detail and impact sound within readability. It never changes the collision sphere or the type logic.

| Faction | Finish |
|---|---|
| Guild | Blunt, old-pattern. Brass body, steel jaws, hand-stamped. Dogs are often numbered 1 and 2 in paint. |
| HTR | Machined, anodized grey, orange swivel. Replaced every match. |
| Ascendancy | Needle-thin: the visible body is 3 cm in diameter and 18 cm long, polished. The collision sphere is unchanged; the halo and tracer carry the readability. |
| Hangers | Stubby and loud. Cast bodies, mismatched jaws, painted. Louder impact layer. |
| Fray | Barbed on every type, including Standard. Black, heat-scored. Illegal in three jurisdictions that no longer exist. |

## Flight

- Spawns at `S_Spool_LineExit`. Speed by class: Climber 140, Scrapper 160, Rigger 120, Doubler 120, Breaker 90 m/s. Gravity on.
- A dim team-tint tracer (0.1 s) and the slack line trailing behind.
- Max range is the class line length. Past it, or on a miss, the Dog falls as a harmless Light prop and auto-recalls in 1 s.
- Dogs do not seat in lines, Hooks, or the Marshal gallery. They bounce with a dull tick and fall.

## Seating animations

Each seat is a 0.08 to 0.12 s clip on the Dog mesh plus a surface VFX and a surface SFX. The line goes from slack to live the instant the seat completes.

| Surface | Standard and Barb | Mag | Spike | VFX | SFX |
|---|---|---|---|---|---|
| **Steel** | Jaws snap closed, 0.1 s, 2 mm bite | Face slaps flat, 0.08 s, 0.5 cm magnetic pull-in | Point skids 20 to 40 cm along the surface leaving a bright scratch, then falls and auto-recalls | Spark fan (Standard, Barb, Mag); scratch decal (Spike) | Clank; Mag a deep thunk; Spike a shriek then a drop |
| **Concrete** | Jaws bite with a crunch, 0.12 s, dust | Bounces off at 30% speed, falls, auto-recalls | Point drives in 3 cm, 0.1 s | Dust puff 30 cm; chip decal | Crunch; Mag a flat bonk; Spike a crack |
| **Wood** | Jaws bite, 0.1 s, splinters | Bounces, falls, auto-recalls | Point drives in 5 cm | Splinter burst, 6 Light splinters 1 s life | Thunk; Spike a hollow knock |
| **Rock** | Jaws bite a lip, 0.12 s | Bounces, falls, auto-recalls | Point drives in 2 cm | Chip spray | Crack |
| **Dynamic object** | As the object's surface material | As above | As above | | Object shudders if the line is under break-free (rack 200 N, stack 500 N, bolted 800 N, rated 1,000 N; 05 §6.2) |
| **Demolition charge** | Seats as on its casing (steel) without triggering it (05 v0.2 ruling) | As steel | As steel | Small spark | Clank; no hiss |
| **Catch Plate** | Jaws bite under the plate lip (Guild pattern) or snap flat (HTR chamfer), 0.08 s; Barb flukes flare and hook the rim | Snaps flat; all plates are metal | Slides off plates; Spike cannot seat in a player | Plate rim band flashes white for 0.2 s; 10 small sparks | **Seat clank**: a distinct, directional sound, mixed 6 dB louder on the victim's client (05 §11) |

Catch Plate seating notes:
- The Dog seats at the plate socket nearest the impact point (`S_Plate_Chest`, `S_Plate_Back`, `S_Plate_ShoulderL`, `S_Plate_ShoulderR`). The mesh snaps to the socket; a 6 cm offset is used for a second Dog on the same plate (**PROPOSED**).
- Standard and Mag Dogs seat in a plate only if the impact normal is within 60° of the plate normal (**PROPOSED**). Barb seats from any angle. Spike never seats in a player.
- A Dog in a plate can be unseated by a Cutter swing to that plate (05 §4.4): it pops out 20 cm, the line goes slack, and it recalls.
- Shaking: moving faster than 8 m/s relative to the puller gives a Standard Dog a small chance per second to unseat (**PROPOSED** tuning hook). Barb never shakes.

## Reseating

A Dog seated in an object that shatters reseats in the largest fragment (05 §3.5). Visually: the Dog pops free, the line goes slack for one frame, and the Dog snaps 10 to 50 cm to the nearest point on the largest fragment with a short seat clip.

## Visual states

| State | Mesh | Glow |
|---|---|---|
| Docked in cradle | Jaws closed, nose out | Off |
| In flight | Jaws open (Standard, Barb), face forward (Mag), point forward (Spike) | Team-tint tracer |
| Seated, slack | Jaws closed on surface | Swivel dim |
| Seated, under force | Jaws closed; Barb flukes visibly loaded | Swivel glows with the line core, same curve |
| Unseated, falling | Jaws open | Off |
| Recalling | Streaks back along the line path at 60 m/s | Short tracer |

## Technical

| Item | Spec |
|---|---|
| Actor | Pooled projectile, 8 per player (Doubler 16), server-authoritative hit. |
| Sockets on the Dog | `S_Dog_Line` at the swivel (line endpoint). `S_Dog_Nose` for the seat normal. |
| Mesh | LOD0 1.2k tris, LOD1 400. Ascendancy variant 900. |
| Textures | 512 per type per faction finish. |
| Material params | `TeamTint`, `Force_N` (swivel glow), `Wear`. |
| Surface response | Physical material on the hit surface selects seat clip, VFX, and SFX. Catch Plates use a dedicated physical material. |
| Seat determination | Nearest plate socket by distance from impact point; ties go to chest. |
| Audio | Seat sounds are class-independent. Faction finish selects a layer (Hangers louder, Ascendancy a higher ping). The plate seat clank is one shared sample set and is never customizable. |

## Reference search terms

grappling hook head, crane rigging shackle and swivel, lifting magnet, rock climbing cam and piton, rebar chisel point, harpoon head, carabiner gate snap.
