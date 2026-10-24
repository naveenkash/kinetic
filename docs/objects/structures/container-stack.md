# Container Stack

One-line: structure, Heavy tier per unit, 2,000 kg per container (6.06 m x 2.44 m x 2.59 m, ISO 20 ft pattern), stacks of two to four, Ballast Bays on magnetic racks, port and starboard, fourteen stacks per round.

## Role in play

The Avalanche lives here. A stack is three 2,000 kg bodies held by friction and a dead magnetic rack. Haul the bottom one above its 500 N break-free and the two above come down the way gravity and the Breathing want. On the lean toward your target the stack falls further. Containers do not fracture (05 section 6.5); they dent, and their doors blow open and spill Medium contents, which is how the Bays get their barrels.

| Harness | Relationship |
|---|---|
| Climber | Stack tops are the Bays' high ground. A Climber on a stack top is 7.8 m up and one Slingshot from a catwalk. |
| Scrapper | Cannot Avalanche (400 N against a 500 N threshold; 05 section 6.2). Loots spilled contents. |
| Rigger | A 600 N Haul breaks a stack free. Hangs a barrel from a rack beam on a set line as a Guillotine and drops it by recalling or cutting the set line (05 section 10). |
| Doubler | 800 N breaks it. Two lines to two stacks, Jack: the Scatter with two-ton shells. |
| Breaker | 1,000 N on 2,000 kg on the racks' dead roller pallets (0.02 to start, 0.005 rolling, 05 section 6.1a) nets 0.3 m/s² to start and 0.45 rolling on the level. Nobody Hauls one up the lean (g sin 6 is 1.02 m/s²); down the lean, the line is the trigger and gravity is the engine. The Avalanche is their rig. |

## Appearance

Silhouette: corrugated steel boxes, corner castings, twist-locks hanging unlocked. Stacked slightly offset because the rack lost power in L.0 and the Breathing has walked them 0.1 to 0.3 m out of line.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Shell (Consortium cargo) | corrugated steel, original paint | #2F4A6B (Consortium blue), #B23A2E (red), #8C8C7A (grey) | 0.6 | 0.7 |
| Shell (HTR staged) | repainted | #E8541E HTR orange, #4A4A4A panels | 0.5 | 0.7 |
| Corner castings | cast steel | #3C3F42 | 0.6 | 0.9 |
| Doors, hinges | steel, rust runs | #7A3B1E streaks | 0.75 | 0.6 |
| Floor | marine ply | #6B5436 | 0.9 | 0.0 |

Weathering: sixty years of seep water running down the corrugations has left white mineral streaks on the uphill face and rust on the downhill. Consortium markings survive under grime: `MERIDIAN CONSORTIUM  CLIMBER CARGO  TENACITY MANIFEST 7731`, hazard placards, a Collar Five destination stencil. HTR has spray-numbered every container in orange (`HTR 014`) for the cut sequence, and the Office has added a white reflective band at 1.0 m height on every container so the Avalanche reads on camera. Interior: a single caged work light per open container (3500 K), because prep crews need to inventory the contents.

Read by tier: from the Crown, grid rectangles with white bands. From the Bays, walls. From the Hold, undersides and the open end of the shaft where they can arrive from.

## Variants

1. **Sealed.** Doors locked. Dents only. Nothing spills. 2,000 kg.
2. **Loaded.** Doors unlatched. Contents: 4 fuel drums or 6 propane cylinders in cradles (200 N rack break-free) or 12 crates. Spills on a 6,000 N s dent or on any impact above 4 m/s that lands door-down. 2,400 kg.
3. **Cut.** Side panel removed by HTR cutters, interior visible, 1,400 kg. Lighter, so a Rigger's Haul moves it at 0.43 m/s² instead of 0.3.
4. **Open-top (Hold).** Crushed in the landing, filled with seep water. Static. A water surface with a x3.0 Stall multiplier where deeper than 1 m.

## Physics

- Tier: Heavy. Mass 2,000 kg. Per 05 section 3.4, 600 N on 2,000 kg is 0.3 m/s². Containers lurch and fall, they do not fly.
- Break-free: 500 N (in a stack, friction). Line force must exceed it; the Haul ramp reaches 500 N at 0.33 s on a Rigger line (600 N over 0.4 s), 0.20 s on a Breaker's.
- Breathing coupling: at a 4 degree lean the stack's anchor field releases for stacks flagged `Loose` (six of the fourteen). Containers ride the racks' dead roller pallets (friction 0.02 to start, 0.005 rolling, 05 section 6.1a), so a released container starts at g (sin angle minus 0.02 cos angle), 0.49 m/s² at 4 degrees, and rolls at g (sin angle minus 0.005 cos angle), 0.98 m/s² at the 6 degree peak. It crosses a 20 m bay in about 7 s and arrives at up to 5.8 m/s. Fast enough to kill, slow enough to read: the scrape loop is the tell. A container that leaves its rack onto the bare deck stops where it lands and is fixed scenery from then on (6.1a).
- Fracture: none. Dent at 6,000 N s. Doors blow at the dent threshold or on a door-down landing above 4 m/s.
- Falling container: from the top of a three-stack (5.2 m to the deck for the second unit) the second container hits at 10 m/s. On a player: 20,000 N s. On a catwalk: 2,500 N s needed, delivered many times over. On another stack: knocks the top unit loose if the hit is above the 500 N break-free equivalent, which it always is.
- Impact and crush: a sliding container is an object hitting a player. 05 section 7.1 gives the example: 2,000 kg at 0.6 m/s is 1,200 N s, a Stall. A rolling container at 5.8 m/s is 11,600 N s. A player pinned between it and a rack post is crushed (over 8 kN for 0.5 s) by stacked and moving weight, which is where crush comes from (CHANGELOG v0.2.1: never from lines). Design intent: the slide kills on contact; the fall is the spectacle and the stack-wide reset of the bay.
- Player on a sliding container: rides it (friction holds under 3 degrees of relative slope). Player on a falling container: airborne, lands with it. A 5.2 m drop is 10.1 m/s, under Stall, but the container's rebound counts as object-hits-player if they are between it and the deck.

## Interactions

- **Haul** bottom container (Avalanche): it lurches 0.2 s after break-free, then rolls at T/m net of the rollers (a Rigger's 600 N: 0.1 m/s² on the level, more on the lean). The two above lose their friction footing within 0.5 m of travel and tip toward the pulled side.
- **Jack** between two stacks (Doubler, Breaker): both bottom units move apart at 0.2 to 0.25 g; both stacks fall inward into the gap. The Scatter in two tons.
- **Hang** to a container: a Hang line never unseats a placed object (05 section 6.2; break-free is tested against active pull only), so any class Pendulums or Gallows-anchors from a stacked container up to the class Hang load. The container's own 19.6 kN of weight goes on the line only once it is falling, and then every single Hang load slips 0.5 m per 0.1 s with a loud creak: a 1.5 s delay, not a hold. The Guillotine is for Medium loads; containers are for the Avalanche.
- Chain: a falling container ruptures a water tank in the Hold (1,500 N s) if it goes down the shaft. See `rated/drive-dog-piston.md` for the authored version.

## States

| State | Visual |
|---|---|
| Intact | Stack square to the rack within 0.3 m. |
| Stressed | Line on it under 500 N: shudders, corner castings scrape, dust from the top units. |
| Failing | Break-free lurch, top units tilt 2 degrees, doors rattle. 0.2 s. |
| Failed | Units scattered on the deck, dented, doors open, contents out, work light swinging inside. |

## VFX and SFX hooks

- `SFX_Container_Scrape_Loop` on any slide, pitch by speed.
- `SFX_Container_Impact_Steel` (deck), `SFX_Container_Impact_Grate` (catwalk).
- `VFX_Container_Dent` (decal plus vertex displacement), `VFX_Door_Blow`, `VFX_Contents_Spill` (pooled props).
- Breathing: `SFX_Stack_Settle` at each lean peak.

## Technical

- LOD0 2,600 tris, LOD1 1,100, LOD2 300, LOD3 box. Dents via a 4-slot morph set, not geometry collection.
- Textures: `T_Container_Shell_A/B/C` 4K (three liveries), `T_Container_Interior` 2K, `T_Ballast_Decal_Atlas` 2K.
- Pivot at the bottom-center. Sockets: `Casting_0..7` (Dog seat hints), `Door_L`, `Door_R`, `Spill_0..5`, `WorkLight`.
- Physics: rigid bodies (not geometry collections) parented to `BP_BallastHull`. Stack friction is authored, not simulated: a per-stack anchor constraint holds each unit until break-free force or the lean flag, then releases. The Shear releases all port stacks at once.
- Collision: `Kinetic_Heavy`. Contents spawn on `Kinetic_Prop`. Budget: six dynamic containers simulating at once per Bay; others sleep until touched.

## Reference search terms

ISO 20 ft container corner casting, container stack collapse, cargo ship hold containers tilted, corrugated container rust streaks, twist lock unlocked.
