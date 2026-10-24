# Rebar bundle

Medium prop. Mass tier Medium, 280 kg. 600 × 18 × 12 cm (30 bars of 16 mm deformed rebar, 6 m long, tied in a rough hexagonal bundle with four wire ties). Appears in the Anchor (the Rim, in the contractor laydown beside the crane pads; the Scaffold, lying along the outer deck edge at level two), and the Yard (the Floor, stacked three deep against the press pit's north wall).

## Role in play

The heaviest Medium and the longest. It is a 6 m lever that nearly reaches the Heavy tier. It rolls on a slope, sweeps on a Haul, and bridges gaps that a line cannot. It does not pierce anything; nothing in the game does. It reads like it should, and that is enough to make players step back.

| Rig | Use |
|---|---|
| The Guillotine | Hang it horizontal at 2.5 m across a Scaffold corridor (2.7 kN, within every class Hang load), then recall or cut your own set line. It drops by gravity at 7.0 m/s, 1,960 N·s, a Stall. |
| The Clothesline | On the ground it is a 12 cm trip bar across a whole corridor. |
| Overboard | Self-dog to the bundle and Jack: no Catch Plate is involved, so the Kick fires and the bundle leaves at 4.4 m/s along the Scaffold deck edge, sweeping three players off. |
| The Anchor Drop | Near the Rim's edge, it is the heaviest unrated object a Scrapper line can start. |

- **Scrapper:** 400 N on 280 kg is 1.4 m/s². Slow, but it moves: 7.6 m/s at the end of a 20 m line, a Stall. Scrappers drag it sideways to block a ladder. No 30% bonus; it is far over 100 kg. A Scrapper cannot start a stacked bundle (400 N under 500 N).
- **Rigger:** Hangs it across the Scaffold as a barrier that stands 12 cm off the deck. Sliders pass under; nobody else does.
- **Breaker:** Hauls it end-on down a corridor like a ram. 1,000 N is 3.6 m/s²; 12 m/s at the end of a 20 m line.
- **Climber:** a 6 m balance beam. Hangs a line from its center and pendulums under it.
- **Doubler:** two lines on the two ends, Jack. The bundle spins flat and sweeps a 6 m circle.

## Appearance

Silhouette: a long, slightly sagging bundle, hexagonal in section, with rusty wire ties every 1.5 m and ragged, unaligned ends. The 30 bar ends read as a dense cluster of dots.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Bars | `MI_Steel_Rusted` | `#7A3F1C` to `#A8562A` | 0.75–0.95 | 0.25–0.40 |
| Bar ends (cut) | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 | 1 |
| Wire ties | `MI_Steel_Rusted` | `#A8562A` | 0.85–0.95 | 0.3 |
| Tag | `MI_Decal_Stencil` on a tin tag | `#E6E4DD` | inherits | 0 |
| Rated-look tape | none. Rebar is not a rated load. Do not tape it. | | | |

Wear: rebar is sold rusted and stays that way. Bright steel only at the saw-cut ends and where the bundle has been dragged, a polished strip along the underside. Mud and concrete dust in the deformations. Where hands lift it (both ends, 50 cm in) the rust is rubbed to a dark brown.

Text: a tin tag wired to one end: `HTR-RB-16-600 / 30 PC / 284 KG`. On the Yard's stacks, chalk: `PRESS WALL / DO NOT CUT`.

Lighting: a dark rust bar that disappears against the Anchor's grey concrete until it moves. Under line glow the bar ends flare as thirty points. The Yard's cutting dust makes it blacker.

## Variants

| Variant | Difference |
|---|---|
| Standard | 30 bars, 280 kg, four ties. |
| Short | 300 cm, 15 bars, 140 kg. Fits the Scaffold corridors. |
| Bent | Two bars kinked upward at one end. Catches on railings. |
| Loose-tied | Spawns with two ties already cut. Fractures at half threshold. |
| Green | New epoxy-coated bars, `#3E7A4E`, roughness 0.5. The Anchor only, HTR delivery. (**PROPOSED** color) |

## Physics

- Tier Medium, 280 kg.
- Collision: one capsule 600 cm long, radius 9 cm. The bundle is modeled as rigid; sag is baked into the mesh. (**PROPOSED**)
- Center of mass: geometric center. Very low; it never tips, it rolls.
- Friction 0.5 on steel, 0.55 on concrete, 0.5 on wood. Rolling resistance when on its side 0.08 because the hex section is lumpy; it rolls in 60° lurches rather than smoothly. Restitution 0.1. (**PROPOSED**)
- Break-free (05 §6.2): loose 0 N, anyone; stacked three deep 500 N, Rigger, Doubler, Breaker; chained to the Rim rail 800 N, Doubler and Breaker. A Doubler's 480 N split line does not start a stacked bundle; 720 N at overspin does. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Fracture: ties break at 2,000 N·s. Fragments: 6 Medium sub-bundles of 5 bars at 47 kg each, 600 cm long, radius 4 cm. Sub-bundles do not fracture further. (**PROPOSED**) A flipped Breaker landing on it at 8 m/s (1,760 N·s) does not break the ties; a 7 m drop does.

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d)):

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 1.4 m/s² | 50 m | 12.0 m/s | 3,360 N·s |
| Scrapper | 400 N | 1.4 m/s² | 20 m | 7.6 m/s | 2,128 N·s |
| Rigger | 600 N | 2.1 m/s² | 40 m | 13.1 m/s | 3,668 N·s |
| Doubler | 800 N | 2.9 m/s² | 30 m | 13.1 m/s | 3,668 N·s |
| Doubler overspin | 1,200 N | 4.3 m/s² | 30 m | 16.0 m/s | 4,480 N·s |
| Breaker | 1,000 N | 3.6 m/s² | 20 m | 12.0 m/s | 3,360 N·s |

Impact on a player (impulse = 280 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 1.1 m/s | 300 N·s | Stagger |
| 2.0 m/s (walking) | 560 N·s | Stagger |
| 2.1 m/s | 600 N·s | Flip |
| 4.3 m/s | 1,200 N·s | Stall; a Scrapper needs 6.5 m of run-up, a Breaker 2.6 m |
| 5.4 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall |
| 11.7 m/s (7 m drop) | 3,276 N·s | Stall; ties break on landing |
| 16.0 m/s (fastest Haul arrival, overspin) | 4,480 N·s | Stall; ties break |

A sub-bundle (47 kg) is under 100 kg, so it gets the Scrapper bonus: 400 N is 8.5 m/s², 18.4 m/s over 20 m, times 1.3 is 24 m/s, 1,128 N·s, a flip. A Rigger's line brings one to the 25 m/s cap over 40 m: 1,175 N·s, still a flip. Sub-bundles Stall only on a Jack that reaches the 30 m/s cap (1,410 N·s) or on a Snap: 1,500 N·s puts a sub-bundle at 32 m/s and it carries the full 1,500 N·s into a player.

Swept impact: a bundle rotating about one end at 2 rad/s has a tip speed of 12 m/s. Impulse on a player at the tip is computed from the bundle's effective mass at that point, not the full 280 kg; use one third of the mass for a tip hit (93 kg × 12 = 1,116 N·s, a flip). (**PROPOSED** rule for long bodies)

Jack (05 §3.2 v0.2): the bundle is Kicked to V × M / (M + m) within 0.3 s (the Kick), then T / 280 kg. Climber (V 18): bundle 4.4 m/s, Climber 13.6 back. Scrapper (V 14): bundle 3.9, Scrapper 10.1. Rigger (V 12): bundle 3.8, Rigger 8.2. Doubler (V 14): bundle 4.4, Doubler 9.6. Breaker (V 10): bundle 4.4, Breaker 5.6. Every Jack starts the bundle at or above its 4.3 m/s Stall speed, except the Rigger's and Scrapper's, which reach it within 2 m.

Crush: no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). The bundle weighs 2.7 kN; three stacked are 8.2 kN, which crushes a player pinned under the stack's bottom bundle for 0.5 s. A Hauler driving it against a wall crushes.

## Interactions

- **Haul to it:** the bundle is heavier than every harness. 1,000 N on a 220 kg Breaker is 4.5 m/s², under the 5 m/s² foot-hold: the Breaker skids 6 m while the bundle comes 4 m. Every other class is airborne and does more of the closing.
- **Jack from it:** strong launch. A Climber keeps 13.6 m/s of V; a Breaker keeps 5.6 of 10.
- **Hang from it:** a Hang to a stacked or chained bundle holds without starting it, up to the class Hang load; a Climber can Pendulum from the Yard's press-wall stack. A Hang to a loose bundle brings it along as 280 kg of dead weight. Hung horizontally from two overhead anchors (two set lines, Rigger), it is a 2.7 kN barrier. A single Climber or Scrapper line (Hang load 4 kN) holds it static but slips once it swings; Rigger, Doubler, and Breaker lines hold it swinging. Two lines' Hang loads add (05 §6.2a). Hung from a player, it drags them flat.
- **Object to object:** it rolls into crates and stops them sliding. It breaks market stalls and scaffold board straps on contact over 3 m/s.
- **Kinematic:** the Maintenance Ring sweeps it off the Scaffold top. A Hauler strike snaps the ties.
- **Water:** sinks immediately.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A stacked or chained bundle is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 280 kg bundle feels 0.36× the authored acceleration (a_ref × min(100 / m, 2)): the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) starts it rolling outward, then back; on the inner Scaffold edge the outward push takes it into the Well.
- **Breathing:** not present in Ballast.
- **Cut recoil:** 150 N·s per 1,000 N, 0.5 m/s. Not visible.
- Rolls on its side in lurches, slides end-on, never floats.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Tight bundle, four ties. | One body, 280 kg. |
| Dented | One tie gone, end bars splayed 20 cm. | No change. |
| Broken | Ties gone, six sub-bundles fanned. | Six Medium bodies, 47 kg each, each under the 100 kg Scrapper bonus limit. |
| Destroyed | Not reachable. Sub-bundles persist. | |

## VFX and SFX hooks

- Impact: `SFX_Rebar_Hit_Steel` (a ringing clang with a long decay), `_Concrete`, `_Wood`. The thirty bars rattle against each other in a `SFX_Rebar_Rattle` layer on every motion.
- Roll: `SFX_Rebar_Roll_Lurch` per 60° step.
- Break-free: `SFX_BreakFree_Steel_Heavy`, `NS_BreakFree_Dust_M`, rust flakes. Under threshold the bundle shudders and rings.
- Fracture: `SFX_Wire_Tie_Snap` ×4, `NS_Rebar_Fan` with rust dust.
- No emissive.

## Technical

- LOD0 6,000 tris (bars as 6-sided prisms, ties modeled), LOD1 3,000, LOD2 1,200 (a single hex prism with a bar-end texture), LOD3 300. One 2K set, with the deformation pattern in the normal.
- Pivot: bottom center of the resting footprint. X+ is the long axis.
- Dog sockets: `SKT_Dog_End_01` (X+), `SKT_Dog_End_02` (X−) `Metal`; `SKT_Dog_Top_01`, `_02`, `_03` (at 150, 300, 450 cm) `Metal`; `SKT_Dog_Hook_01`, `_02` (wire tie loops at 150 and 450 cm) `Metal`; `SKT_Dog_Bot_01` (center underside) `Metal`. Fragments: `SKT_Dog_F01_01` to `SKT_Dog_F06_01` at each sub-bundle center.
- Geometry collection `GC_RebarBundle`: L0 whole (damage threshold 2,000), L1 six sub-bundles (threshold 100,000). No L2.
- Anchor fields: `FS_Anchor_RebarBundle` releases stacked instances at 500 N; `FS_Anchor_RebarBundle_Chain` releases chained instances at 800 N.
- Replication RC-M; sub-bundles stay RC-M after fracture because they inherit Medium tier.

## Reference search terms

Rebar bundle 16 mm tied wire; rebar laydown construction site; rusty rebar bundle ends close up; rebar bundle tag tin; epoxy coated rebar green; rebar bundle crane lift photo.
