# Drive Dog Piston

One-line: rated load, 40,000 kg, piston 6.0 m long x 1.6 m diameter seated in a 7.5 m housing, Ballast Spine: six housings on the shaft walls, with ledges at Z 10 (drive dog 1, starboard) and Z 26 (port, at the Spine collar) per `arenas/ballast/layout.md`, and four dead housings at Z 34 and Z 42 (**PROPOSED** dressing positions; the Z 42 dog is the one the Full Shear sudden-death event releases on its own). Two live per round: Z 10 and Z 26.

## Role in play

The drive dog is Ballast's biggest lever and the single highest-Tonnage object in the game (04: dropping one down the Spine is worth more than a scrap). A Breaker who commits one sends forty tons down a 6 m shaft through every catwalk that bridges it, into the Hold, where it ruptures a water tank and floods the nose. It is the Avalanche's older brother and the Anchor Drop's big cousin: Hang an enemy to the piston before it goes and they go with it.

| Harness | Relationship |
|---|---|
| Climber | The Spine is their highway; a live piston is the thing that closes it. They can see the taped housing from anywhere in the shaft. |
| Scrapper | Cannot move it. Can cut the Breaker's line during the 0.6 s commit window (see README). |
| Rigger | Hangs a Clothesline across the ladder beside a live housing so the Breaker climbing to it walks into a wall. |
| Doubler | Overspin reaches 1,200 N at 1.0 s and snaps at 1.8 s: 1.3 s above 1,000 N, enough to commit a piston on a single overspun Haul, at the cost of a stagger. The Fray's version of the HTR play. |
| Breaker | The owner. Reach the housing by ladder, Haul the piston face from the rest platform beside it, brace. |

## Appearance

Silhouette: a horizontal cylinder in a cradle housing on the shaft wall, its domed face toward the shaft, four claw-like dog pads folded along its sides. The housing has a quarter-panel cut away by HTR so the piston's barrel is visible. Yellow-and-black tape in a continuous band around the housing mouth.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Piston barrel | nitrided steel, original | #3C3F42 | 0.35 | 1.0 |
| Face dome | Weave-coupled contact plate | #1B1F26 with sheen | 0.3 | 0.7 |
| Dog pads | hardened steel, bright wear faces | #A8ADB1 | 0.25 | 1.0 |
| Housing | cast alloy, Consortium grey, seep-streaked | #7C8488 / #C9C3B6 | 0.6 | 0.7 |
| Hydraulic lines | braided steel, oil-dark | #2A2C2E | 0.5 | 0.6 |
| Tape | Office yellow and black | #F2C500 / #141414 | 0.7 | 0.0 |

Weathering: the barrel still shows machining marks; the housing is scaled white where seep water runs. The hydraulic lines have bled a sixty-year oil stain down the shaft wall in a black fan. Markings: `DRIVE DOG 3-P  TENACITY  MAX LOAD 1,200 t` cast into the housing; HTR's `CUT 02  SEQ A` in orange on the live housings; Guild chalk reading `SEATED - FRICTION ONLY - DO NOT LOAD` on the cradle, which is the lore reason it moves at 1,000 N: the seating bolts were cut in L.40 and never replaced. Lighting: an Office work lamp clipped to each live housing (3500 K, aimed at the tape) so the tape reads from the opposite wall. Dead housings are unlit.

Read by tier: from the Crown, two lit tape bands in the shaft below. From the Bays, a lit barrel face at the shaft lip. From the Hold, a dark cylinder overhead with a lamp beside it.

## Variants

1. **Live, upper (Z 26, the collar).** Falls 26 m of height, 27.7 m along the 70 degree shaft, 2.4 s, arriving at 22 m/s. Takes the Mouth Plate at Z 16 (and the Claim marker on it, which goes down the shaft and resets per 05 section 9.4) and lands on the Spine Foot's plug.
2. **Live, lower (Z 10).** Falls 10.6 m along the shaft, 1.5 s, 14 m/s. Takes nothing on the way, slides out of the Foot's torn Belly face into the Wash and ruptures the two tanks there. Lower Tonnage, faster consequence.
3. **Dead.** Untaped, welded, lamp off. Static. Dogs seat; nothing happens.
4. **Stabilized (Guild season win).** Live housings re-bolted: still taped (structural) but break-free raised to a value no line reaches. Dressing says why: new bright bolts and a Guild seal.

## Physics

- Tier: Rated. Mass 40,000 kg. Break-free 1,000 N: a Breaker alone, a Doubler at overspin, or any lines summing to 1,000 N along the face's axis (a crew; 05 section 6.2). The 1,000 N is a 0.6 s commit trigger; after it the authored timeline carries the piston regardless of friction or line. A Breaker's 1,000 N on 40,000 kg would be 0.025 m/s²; the line does not move the piston, it unseats it. Everything after is gravity.
- Commit: per the README's authored-consequence spec, line force at or above 1,000 N sustained for 0.6 s (**PROPOSED**). Cutting the line before 0.6 s cancels with no effect beyond the lurch tell.
- Fall: the shaft is pitched at 70 degrees plus or minus 6 (the Breathing). Along-shaft acceleration g sin 70 = 9.2 m/s² (8.8 at 64 degrees, 9.5 at 76). From Z 26: 27.7 m of travel, 2.4 s, arriving at 22 m/s; it passes Z 16 (the Mouth Plate) 1.3 s after leaving the housing. From Z 10: 10.6 m, 1.5 s, 14 m/s.
- Impacts, per 05 section 7: on a player at any speed above 0.03 m/s the impulse exceeds 1,200 N s; scrapped. On the Mouth Plate (2,500 N s, catwalk class): delivered at the first contact; the plate fractures ahead of the piston on the authored timeline. On the Spine Foot plug: the piston stops in the crushed cargo in 1.2 m. The lower dog slides out of the Foot into the Wash and ruptures the two tanks within 4 m of its stop (1,500 N s each). Any line the falling piston passes through is severed (05 section 3.5; the piston is kinematic for the fall): a Hang across the shaft at Z 16 to 26 snaps and drops its Pendulum.
- Crush: anything between the piston and the shaft wall or a ladder: over 8 kN for 0.5 s, scrapped.
- Players tethered: a Hang line from the piston to a player (Anchor Drop) drags them down the shaft at the piston's speed; the line's Hang load is irrelevant because the player is the light end. They have 0.6 s commit plus 1.2 s groan plus the first 0.5 s of fall before they pass 12 m/s relative to the shaft wall: 2.3 s panic window. Within design target.

## Authored consequence

Trigger: commit per README. Then:

| Time | Event |
|---|---|
| 0.0 s | Commit. Lurch: piston shifts 40 mm, tape band splits at the mouth. `SFX_Piston_Lurch`. |
| 0.0 to 1.2 s | Groan. 1.2 s lead (05 section 11 minimum 1.0 s). Housing bolts shear in sequence, four clanks at 0.25 s spacing. Dust and scale fall from the housing. The work lamp swings. Marshal: "Drive dog loose in the Spine." |
| 1.2 s | Release. Piston slides out of the cradle on its own hydraulic film, nose down the shaft. The Mouth Plate's LED strips go red. |
| 1.2 to 3.6 s (upper) or 1.2 to 2.7 s (lower) | Fall. Ladder cage hoops shear 0.1 s ahead of the face. The Mouth Plate fractures when the face is 0.3 m from it (upper dog only; authored, not simulated, so every client agrees). Lines in the path snap. Sparks from the barrel against the far wall on the lean. |
| 3.6 s (upper) | Impact on the Spine Foot plug. Crater decal 4 m. Spine-wide `SFX_Piston_Impact` and a 0.6 s camera shake within 30 m. The piston stays in the Foot as a 40,000 kg static obstacle; the Foot is closed to the Wash on the Belly side. |
| 2.7 s (lower) | The piston slides out of the Foot's torn Belly face into the Wash, stops 6 m in, and ruptures both Hold tanks (see `environment/force-volumes.md`, water rush toward the rent, 4 s); the Wash surges 0.4 m deeper for 20 s then drains. The piston lies across the Wash as a 1.6 m high step between the Foot and the Sump. |

What breaks: the Mouth Plate (upper dog), cage panels on the ladders beside the path, two Hold tanks (lower dog), any line crossed, and any player or container in the way (containers struck are launched at up to 12 m/s and dent).

Reset: full, between rounds, per 05 section 9.2 step 5.

## Interactions

- **Haul** the piston face (Breaker or Doubler overspin): commit as above. Unbraced, the Breaker takes 4.5 m/s² toward the static piston (under their 5 m/s² foot-hold: a skid of about 1 m over the commit). Braced (20 m/s²), nothing.
- **Jack** off the piston face: a Slingshot off the most solid thing in the shaft, for every class.
- **Hang** from the piston face: a Hang line never unseats a placed object (05 section 6.2; break-free counts active pull only), so any class uses a live piston as a Pendulum pivot across the shaft up to the class Hang load. Hang an enemy to the piston before committing it: the Anchor Drop. Hang yourself to it by mistake: the same.
- Chain: the water rush pushes Hold players toward the nose on the forward lean; the piston's landing puts its face across the sump and leaves a 1.6 m high step that the rush carries bodies over.

## States

| State | Visual |
|---|---|
| Intact | Seated, taped, lamp on the tape. |
| Stressed | Line on it under 1,000 N or under 0.6 s: housing shudders, tape flutters, cradle scrapes 10 mm and returns. |
| Failing | The 1.2 s groan sequence. |
| Failed | Housing empty, cradle scored, piston in the Hold. |

## VFX and SFX hooks

`SFX_Piston_Lurch`, `SFX_Piston_Bolt_Shear` x4, `SFX_Piston_Slide_Loop`, `SFX_Piston_Impact`; `VFX_Housing_Dust`, `VFX_Barrel_Sparks`, `VFX_Hold_Crater`, `VFX_Tank_Rupture` x2; Marshal VO line; shaft bridge strip color driver.

## Technical

- LOD0 piston 5,200 tris, housing 7,800; LOD1 half; LOD2 quarter; LOD3 cylinder and box.
- Textures: `T_DriveDog_Piston` 4K, `T_DriveDog_Housing` 4K, `T_Ballast_Decal_Atlas` 2K.
- Pivot at the housing mouth center. Sockets: `Face` (Dog seat and Haul target), `Pad_0..3`, `Lamp`, `Tape_Band`, `Path_Start`, `Path_End`, `Sever_Volume` (the swept 1.6 m cylinder along the fall path, tested against line segments each step).
- Implementation: piston as a rigid body kept kinematic in the cradle; the consequence timeline drives it kinematically along the shaft spline (deterministic, server time-synced) rather than simulating a 40 t body against hull geometry. Contact with players and props is resolved as a kinematic hit (infinite mass per 05 section 6.7 for the duration of the fall). Catwalk fractures are fired by timeline events. Parented to `BP_BallastHull` so the fall follows the lean.
- Collision: `Kinetic_RatedLoad` seated, `Kinetic_Kinematic` during fall, `Kinetic_WorldStatic` when landed.

## Reference search terms

hydraulic ram cylinder large industrial, pipeline pig launcher, ship stern tube bearing housing, tunnel boring machine thrust cylinder, oil-stained engine room bulkhead.
