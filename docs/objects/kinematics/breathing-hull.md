# Breathing Hull

One-line: kinematic system, infinite mass, the whole 410 m freight climber _Tenacity_ rocking as one body, playable interior per `arenas/ballast/layout.md` (Hold floor Z 0, Hold Ledge Z 6, Lower Bay decks and Mouth Plate Z 16, Upper Galleries and Spine collar Z 26, Crown Net Z 38, Crown deck and Spine head Z 44; the Spine is the forward 72 m of the shaft), Ballast: the arena root. Pitch theta(t) = 70 + 6 sin(2 pi t / 90) degrees, about a transverse axis at the Hold floor where the nose rests in the gorge.

## Role in play

The Breathing is Ballast's one big moving part (03, rule 4) and everything else in the arena is its child. It is not a thing players hit; it is the thing that moves what players stand on. On the lean, loose containers slide, chains swing, Hold water shoves, a Hang line set at one end of the cycle is a different length at the other, and a Pendulum swung on the lean goes further than it should. New players fight the Breathing. Good players wait for it. The Shear (1:15 remaining) is the Breathing's event: the port bay wall sheds into the gorge and the hull's rocking does not stop.

| Harness | Relationship |
|---|---|
| Climber | Reads the lean as a launch angle. A Slingshot up the Spine on the back-lean gains 6 degrees of ramp. |
| Scrapper | Times barrels. A fuel drum released on a 6 degree deck rolls at 0.68 m/s² and arrives on schedule. |
| Rigger | Sets Hang lines at the cycle's extreme so they go taut or slack on purpose: the Draw, a line across the Spine that catches a swinging Climber. |
| Doubler | Overspin timers against a 90 s cycle: the Doubler's 1.8 s is a rounding error, and they play as if the deck were flat. |
| Breaker | Pulls racks and pistons on the lean toward the target so the fall goes further. |

## Appearance

Silhouette: the inside of a container-ship-sized climber standing nose-down at 70 degrees in a gorge, open to sky at the top where HTR has taken the Crown's plating. Three decks of Office grating laid level at mid-cycle across the hull's original 20 degree internal pitch, the Spine shaft running the long axis, the Bays either side, the Hold flooded at the bottom. The hull itself is dark Weave-faced composite over alloy ribs, with everything in it streaked by sixty years of seep water.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Hull skin (inside) | Weave-faced composite | #1B1F26 with sheen | 0.35 | 0.6 |
| Ribs and frames | alloy, Consortium grey | #7C8488 | 0.5 | 0.8 |
| Office decking | galvanized grating, new | #8A8F93 | 0.55 | 0.9 |
| Seep streaks | mineral scale | #C9C3B6 | 0.8 | 0.0 |
| Hold water | dark, oil film | #0F1A1C, specular | 0.05 | 0.0 |
| Sky opening (Crown) | | | | |

Weathering: the hull is the most weathered object in the game. Markings: `TENACITY  MERIDIAN CONSORTIUM  FREIGHT CLIMBER 7` in 4 m letters on the Spine's main bulkhead, deck numbers, HTR survey lines in orange spray along every cut, Guild chalk along the Shear's cut line reading `NOT YET`. Office decking carries yellow edge stripes. Lighting: daylight through the Crown, Office LED strips on every catwalk, work lamps on rated loads, the Hold lit only by what falls through the grating and by lines. Hold water reflects every singing line, which is why the Hold reads as the darkest and the most readable tier at once.

Read by tier: from the Crown, the whole interior falling away below you and the sky swinging 12 degrees across the opening every 90 s. From the Bays, walls that lean. From the Hold, water moving toward and away from the nose, and the nose opening to the gorge on the forward lean.

## Variants

1. **Standing season (default).** Full cycle, Shear at 1:15.
2. **Stabilized (Guild Claim win).** Amplitude 3 degrees, period 120 s, no Shear (04). The port racks are dead; the port catwalks have Guild bracing.
3. **Cut (HTR Claim win).** Crown and Spine only, open to sky (04). The Bays and Hold are gone; the Spine opens directly onto the gorge below the Crown deck. Amplitude 6 degrees. The Well is 50 m down the Spine.

## Physics

- Kinematic root. Rotation about the transverse axis through the Hold floor: theta(t) = 70 + 6 sin(2 pi t / 90) degrees, leaning toward the nose first. Max angular speed 0.42 degrees per second (0.0073 rad/s). The Crown deck (Z 44) moves 4.6 m laterally each half-cycle at a peak speed of 0.32 m/s; the Upper Galleries (Z 26) 2.7 m; the Lower Bay decks (Z 16) 1.7 m. Angular acceleration is negligible (peak 0.0005 rad/s²; at the Crown 0.03 m/s²). Nothing is thrown by the rocking itself. Everything is moved by gravity on tilted floors.
- Floors at the lean: prep-crew grating is level at theta 70 and tilts 0 plus or minus 6 degrees (g sin 6 = 1.02 m/s² along the deck); the original deck plates (the Slabs under the racks) slope 20 degrees toward the Belly and swing 14 to 26. Against the foot-holds in 05 section 5.3 (3 to 5 m/s²) no player loses footing from the lean. Players walk on a 6 degree slope; movement speeds are unchanged.
- Loose objects: a rolling drum on a 6 degree deck accelerates at 0.68 m/s² (2/3 g sin 6); 20 m in 7.7 s, arriving at 5.2 m/s (936 N s on a player, a flip; under the drum's 10 m/s trigger, no blast). Sliding props with a 0.08 friction coefficient (**PROPOSED**, oily steel) accelerate at 0.24 m/s² above a 4.6 degree slope and stop below it: they move for about 25 s of every 45 s half-cycle. Container stacks flagged `Loose` release their anchor at 4 degrees (see `structures/container-stack.md`). Hanging chains (Light, 2 m lengths on hooks) swing 6 degrees with a 2.8 s period excited by the lean; a chain hit is 300 N s at most, a stagger.
- Hang lines: a line from a hull-fixed point to another hull-fixed point does not change length (both move together). A line from the hull to a free body on the deck (a container, a player) changes by the body's slide. A line from the hull to a swinging chain or a sliding container goes taut on the lean away and slack on the lean toward. A Pendulum from a Spine anchor: the player's start point moves 2 to 5 m during the swing; releasing at the apex on the forward lean adds the hull's 0.15 to 0.39 m/s to the player's velocity in world space, which is the "further than physics said" of 03, modest and real.
- Hold water: the Wash, 0.6 m deep standing seep (1.2 m in the Sump by the nose rent). It is a tilt-driven force volume (see `environment/force-volumes.md`, Hold flood): force along the deck toward the low end, scaled by sin(angle). At full lean, 600 N on a reference body: a Climber (6.7 m/s²) is swept off their feet; a Breaker (2.7 m/s²) wades. The nose opens onto the gorge; on the forward lean the Hold floor is a 6 degree ramp into the Well 18 m from the sump.
- The Shear: at 1:15 remaining (4:00 in Claim, 2:00 per Haul half), 5 s siren, then the port bay wall panel (30 m along the hull, Z 6 to 42, pre-fractured, with the port decks' outer 4 m strips and the port Upper Gallery's outer edge parented) rotates outward to 90 degrees and falls free over 2.5 s on an authored path. Everything parented to it goes with it. Lines are severed as the panel passes through them (05 section 3.5: a kinematic severs any line it passes through; the panel and the galleries riding it are kinematic during the fall, so a Hang from the port gallery to the Spine, or a Gallows hung off the port wall, snaps and drops whoever was on it). The port Lower Bay is open to the gorge along its length; the new deck edge at X -14 is a Well edge. The Breathing continues.

## Cycle and tells

| Time | Theta | Event | Tell |
|---|---|---|---|
| 0 s | 70 (neutral) | Moving toward the nose lean. | Groan begins: a long structural note, low, rising as the angle grows. |
| 22.5 s | 76 (nose down) | Peak forward lean. Hold water at the nose, loose containers sliding toward the nose, chains swung forward. | Groan peaks and holds; a single deep crack at the peak (`SFX_Hull_Peak_Crack`). Sky at the Crown has swung 6 degrees. |
| 45 s | 70 | Neutral. Objects slow. | Groan fades to the hull's idle tick. |
| 67.5 s | 64 (nose up) | Peak back lean. Water to the Spine wall, containers toward the Spine, the Hold floor sloping away from the Well. | Groan and crack again, higher pitch (authored: the back lean loads a different frame). |
| 90 s | 70 | Loop. | |

Audio: `SFX_Hull_Groan_Loop` driven by angle magnitude (pitch and level), `SFX_Hull_Peak_Crack` at each extreme, `SFX_Chain_Swing_Loop` per chain cluster, `SFX_Hold_Water_Shift` layered by tilt rate, `SFX_Container_Scrape_Loop` from every sliding container. Visual: the sky through the Crown, dust falling from frames at each peak, water surface tilt, swinging chains and lamps, LED strips on hanger rods swaying. Lead: the lean is continuous and slow; the tells for its consequences (a container slide, a water shove) are the break-free lurch (0.2 s) and the force volume particles (1 s) per 05 section 11. The Shear carries the 5 s siren.

## Interactions

- **Haul** on the lean: a container pulled downhill arrives sooner and keeps going after the line releases.
- **Jack** up the Spine on the back lean: the shaft tilts 6 degrees toward the Crown deck's far side; a Climber's 18 m/s launch lands 2 m further along it.
- **Hang** across the Spine at the forward lean's peak: 45 s later, the line is a Clothesline 2 m from where it was in world space while staying put in hull space. Players track hull space; it is the swinging Climber on a world-anchored path (there are none; all anchors are hull) who is caught. In practice the Draw catches a Pendulum released at the wrong phase.
- Haul mode: the 2,000 kg spool climbs the Spine on a counterbalanced ratchet rail (arena-unique; specified in `arenas/ballast`). The rail is a child of the hull root, so the spool's climb tilts with the shaft: 6 degrees of lean adds or removes 1.02 m/s² along the rail, and the ratchet holds what the lean would take back.
- Chain: the Breathing is the chain. Every object doc for Ballast lists its lean behavior.

## States

Kinematic system: `Forward lean`, `Neutral`, `Back lean`, plus `Sheared` after the event. The cycle restarts at theta 70 on the Marshal's live call every round. The Stabilized variant halves the amplitude; the Cut variant removes the lower decks.

## VFX and SFX hooks

`SFX_Hull_Groan_Loop`, `SFX_Hull_Peak_Crack`, `SFX_Hull_Idle_Tick`, `SFX_Shear_Siren`, `SFX_Shear_Release`; `VFX_Frame_Dust_Fall`, `VFX_Hold_Water_Surface` (tilt-driven plane with foam at the low edge), `VFX_Chain_Swing`, `VFX_Shear_Sparks_Along_Cut` (siren phase), `VFX_Shear_Wall_Fall`.

## Technical

- Hull as a single kinematic root actor (`BP_BallastHull`) with an animated tilt (03 engine notes); all contents are attached children simulating in its frame. Rotation set per physics step from server time; no replication of the angle, every client evaluates the same function.
- Contents simulate in hull space: Chaos bodies attached to the root receive the frame's motion as kinematic target transforms. With angular acceleration this small, fictitious forces are ignored; gravity in hull space is re-expressed per step as the world gravity vector rotated by minus the hull angle, which is what makes decks slope.
- The Shear: a geometry collection of the port wall with level 0 anchored by a field actor; the event clears the field and applies an authored initial velocity so the fall path matches across clients. Catwalks and racks on the port side are children of the wall cluster. The gorge below is `Kinetic_KillVolume`.
- LOD: hull interior 180,000 tris at LOD0 in view-dependent sections, HLOD per deck. Textures: `T_Hull_Interior_Trim` 4K, `T_Hull_Ribs` 4K, `T_Seep_Decal_Atlas` 2K, `T_Hold_Water` (material only).
- Collision: hull `Kinetic_WorldStatic` (moving with the root), water `Kinetic_ForceVolume` overlap, nose opening and gorge `Kinetic_KillVolume`.

## Reference search terms

ship wedged in gorge, Costa Concordia interior tilted, cargo ship hold listing, freight elevator shaft interior vertical, space elevator climber concept interior, seep water mineral streaks steel.
