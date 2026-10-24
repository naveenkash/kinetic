# KINETIC: VFX (Niagara Systems)

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 05 sections 3.5, 3.6, 3.7, 6.2, 6.4, 6.5, 6.6, 7.4, 7.5, 11 and arena rule 8 from 03 ("Lines are the light"). Numbers in 05 are canonical. Anything marked **PROPOSED** is new here._

## 1. The readability rule

From 03: **arenas are lit so that a singing line is always the brightest thing in frame. Nothing may out-glow a line.**

Enforced as numbers:

- Reference: a line at class T has emissive luminance **L_line = 1.0** (normalized; in-engine, the line material's emissive at T is the scene's white point for VFX).
- A line at 100 N is 0.30 L_line. A set line is 0.15 L_line, rising to 0.30 within 5 m of a player. A slack line is 0.08 L_line.
- No particle system, blast included, may exceed **0.85 L_line** for longer than 0.1 s, or **0.6 L_line** sustained. Blasts peak at 0.85 for one frame, then decay.
- Scene lighting: no emissive surface, floodlight, or sky exceeds 0.5 L_line within the playable volume. Floodlights are bright on the ground, not in frame.
- Heat shimmer, smoke, dust, mist: maximum 40% opacity over any line. A line must remain readable through any VFX at 20 m.
- Team tint on VFX uses the canonical crew tints from 05 section 11a (summarized in ui-hud.md section 2): Amber `#E69F00`, Sky `#56B4E9`, Rose `#CC79A7`, Bone `#E8E6E0`. Tinted VFX are limited to line-owned effects (line, Dog trail, seat spark, snap). Physics VFX (dust, debris, blasts, water) are never tinted.

Validation: a debug view "Line Luminance Check" renders everything above 0.6 L_line in red. Any red that is not a line is a bug. The Anchor's tower-collapse darkened sector is the acceptance test (03 engine notes).

## 2. Systems list

Budget column is the per-instance particle count at LOD 0 and the max simultaneous instances. LOD distances: LOD 0 under 15 m, LOD 1 to 40 m, LOD 2 to 100 m, culled beyond unless the system is a tell (marked T), in which case LOD 2 persists to the arena bound. GPU sims unless noted.

### 2.1 Lines

| System | Trigger | Parameters | Visual | Lifetime | Budget |
|---|---|---|---|---|---|
| **NS_Line_Core** (T) | Line exists | Force_N, HangLoadRatio, Mode, Team, Length, Overspin_t, IsSet, IsSlack | Not a particle system: a ribbon mesh with an emissive material. Core width 1.5 cm. Emissive = f(Force) linear from 0.08 at slack to 1.0 at T, to 1.3 at 1,200 N. Hang lines map load / Hang load onto the same curve so a fully loaded Hang glows like a full Haul (**PROPOSED**, matches audio.md section 2.1). Hue is team tint; at high force the core whitens toward Bone (hot filament). Mode pattern (PROPOSED): Haul, 20 cm bright chevrons travel from the Dogs to the midpoint at 4 m/s; Jack, chevrons travel midpoint to Dogs; Hang, no travel, bright 10 cm knots at each Dog. Enemy lines carry a dark 4 cm tick every 1 m | Line life | 1 per line, max 20 lines (12 in Scrap) |
| **NS_Line_Heat** | Force over 50% T | Force_N, Length | Heat shimmer refraction along the ribbon, width scaling 5 to 20 cm with force. Rising wisps, 10 per meter per second at T, Bone at 20% opacity, 0.4 s life | While over 50% T | 200 particles, max 12 |
| **NS_Line_Slack** | A seated, no B, or Haul past 25 m/s | Team | Ribbon only, 0.08 L_line, no heat, no pattern, visible sag catenary | 4 s max | 0 particles |
| **NS_Line_Set** (T) | Hang released; the owner may recall it early (05 section 3.6: a recalled set line reels in over the class recall time with no fade, knots collapsing toward the owner; that is the Guillotine drop) | Team, Life_s, PlayerWithin5m, Recalled | Ribbon at 0.15 L_line, slow 0.5 Hz breathe. Within 5 m of any player: 0.30 L_line, breathe stops. Expiry: ribbon goes slack (catenary sag), fades to 0 over 0.5 s with 30 drifting embers along its length | Class life, 8 or 12 s | 30 particles at expiry, max 8 |
| **NS_Line_Slip** (T) | Hang over Hang load | Overload_ratio | Ribbon jitter: 1 cm vertex noise at 10 Hz, plus 5 sparks per 0.1 s at the Dog with the higher load, Bone | 0.3 s per 0.5 m slipped | 20 particles, max 4 |
| **NS_Line_Overspin** (T) | Overspin_t over 1.0 s | Overspin_t, Team | Ribbon emissive flickers to Bone white at 8 Hz, 50% duty, on top of the force glow (05 section 11: "line flickers white"). Heat wisps double. From 1.6 s, 10 sparks per frame shed along the length | 0.8 s | 100 particles, max 2 |
| **NS_Snap_Flash** (T) | Line cut, snapped, or severed by a kinematic passing through it (05 section 3.5; adds a shear spark burst from the kinematic's edge) | Tension_N, CutPoint, Team, IsOverspinSnap | A one-frame flash at the cut point, 0.85 L_line, radius 0.3 m at 100 N to 1.5 m at 1,200 N. The two ribbon halves recoil toward their Dogs over 0.15 s with a whip curve. Ember burst along both halves: count = Tension_N / 5. The halves' recoil is visual only: a normal cut delivers 150 N·s per 1,000 N to each end (05 section 4.2), under stagger, so nothing is launched. Only an overspin snap launches the Doubler and carries 1,500 N·s × (F / 1,200 N) to the far end; that snap adds a 2 m Bone shock ring. Embers are team tint for 0.2 s then Bone, 0.8 s life, gravity 0.5 | 1.2 s | 240 particles max, max 4 |
| **NS_Kick** (T) | Jack engaged with no Dog in a player's Catch Plate (05 section 3.2: capacitor discharge, once per Jack; the interlock suppresses it on any player-target Jack, which shows only NS_Line_Core's 0.3 s ramp) | Class_V, Team, IsSelfDog | Spool housing flare: a one-frame team-tinted flash at the Spool, 0.6 L_line, then a 0.3 s discharge glow on the housing vents fading to 0; 20 sparks from the vents; a single bright ring pulse travels the line from the Spool end to the far Dog at 60 m/s. Flare size scales with class V (Climber largest). Self-dog Kick adds a 0.5 m dust ring at the feet. The flare never exceeds 0.6 L_line, so a Kick cannot out-glow the line it starts | 0.4 s | 30, max 6 |
| **NS_Reel** | Reel held in Hang (05 section 3.2) | Direction, HangLoadRatio, Team | Spool collar ratchet glint, 8 per meter; the rod's end knots slide along the ribbon as it shortens or lengthens; at 100% Hang load NS_Line_Slip takes over. No flare, no sparks: a winch is quiet | While held | 10, max 6 |
| **NS_Snap_Recoil** | Doubler snap | Direction | A short Bone shock ring at the Doubler's Spool, 0.3 m, 0.1 s, plus 3 s of heat-score smoke from the Spool housing | 3 s | 40 particles, max 2 |

### 2.2 Dogs

| System | Trigger | Parameters | Visual | Lifetime | Budget |
|---|---|---|---|---|---|
| **NS_Dog_Fire** | Dog launched | Class, Team | Spool muzzle puff, 0.1 s, Bone, with a team-tinted flash at 0.4 L_line one frame | 0.3 s | 15, max 10 |
| **NS_Dog_Trail** | Dog in flight | Speed, Team | Thin ribbon trail 1 cm, team tint at 0.3 L_line, fades over 0.3 s behind the Dog. The slack line follows it as a dim catenary | Flight | 1 ribbon, max 10 |
| **NS_Dog_Seat** (T) | Dog seats | Surface, MassTier, IsPlate, Team | Seat spark per surface, see below. Catch Plate seat: a team-tinted ring 0.4 m expanding from the plate over 0.2 s at 0.6 L_line, visible from every angle including behind the victim, so a Spotter sees the plate light up | 0.5 s | 40, max 10 |
| **NS_Dog_Miss** | Dog exceeds range or hits nothing | | Dog bounces as a prop, a 3-particle dust puff, then a 1 s rewind with the line ribbon reeling in | 1 s | 10, max 6 |
| **NS_Dog_Unseat** | Cutter melee hits a plate | Team | The Dog pops out with 10 Bone sparks and a 0.2 s plate flash | 0.4 s | 15, max 4 |

Seat spark by surface: steel, white-orange sparks (20), 0.4 s; concrete, grey chips (12) and dust; wood, splinters (8) tinted by the wood; rock, sharp grey chips (10); cable or rope, 4 fibre strands and a twang vibration on the cable mesh; dynamic prop, surface spark plus a 1-frame object outline flash in team tint; rated load, surface spark plus a Caution tape flash on the yellow-and-black markings for 0.3 s.

### 2.3 Players

| System | Trigger | Parameters | Visual | Lifetime | Budget |
|---|---|---|---|---|---|
| **NS_Drag_Skid** | Grounded player under line force | Surface, Speed | Boot-contact dust or sparks (steel) streaming opposite to the pull, 15 per second per foot, Bone or surface color | While dragging | 60, max 6 |
| **NS_Stagger** | Stagger (0.3 to 1.0 s) | Duration | First-person: a two-frame Bone edge flash and a vignette. Third-person and others: three short Bone "shake lines" off the harness, 0.2 s. No stars, no cartoon | Duration | 10, max 8 |
| **NS_Flip** | Flip | | Trail of 6 Bone motion streaks from the harness plates during the ragdoll arc; readable arc for spectators | Until landing | 20, max 4 |
| **NS_Hoisted** | Player hoisted | Team of the line | The line's Dog-end knot at the plate brightens to 1.0 L_line regardless of force; harness strap tension lines (mesh) draw taut | While hoisted | 0 particles |
| **NS_Brace** | Brace engaged (0.3 s) | Class | Foot-plant dust, two puffs; Breaker adds a harness lock glint. Braced state: a faint Bone ground ring 0.6 m under the player at 0.1 L_line so enemies can read "braced" (PROPOSED) | While braced | 10, max 6 |
| **NS_Perch** | Climber perch (1.5 s max) | | Hand and foot contact dust, 4 particles, plus a thin 1.5 s drain ring at the contact point, Bone, 0.15 L_line | 1.5 s | 8, max 3 |
| **NS_Stall_Gel** (T) | Stall threshold crossed | Pose | 0.2 s gel flood: Gel `#8FE3CF` fluid bursts from six harness ports, 120 particles, settling into a translucent shell mesh over the ragdoll (material swap). Harness lock: a one-frame Bone flash on every plate. The shell persists 4 s on the body prop, then lifts with the Hook | 6 s | 120, max 6 |
| **NS_Hook_Retrieve** | 4 s after Stall | | Hook descends, rotor wash dust 2 m radius, clamp spark, 2 s lift with gel drips (20) | 3 s | 60, max 4 |
| **NS_Ringout_Mist** (T) | Player passes lowest playable floor | Arena | A 0.5 s wake of the Well's mist (Hangtown, Ballast gorge) or spray (Yard press pit) drawn upward along the fall line, Bone 30%, and a Hook rotor-wash disc visible below. Anchor Well: the Stump's hum drops an octave (05 v0.2.2), rendered as a 1 m Bone ring that contracts toward the Stump over 0.5 s, no mist, no Hooks | 1.5 s | 80, max 4 |
| **NS_Drop** | Match entry (6 s) | Drop type | Hook lowering (rotor dust), line slide (NS_Dog_Trail reuse), Hauler step-off (rail sparks), cargo drop (Breaker: dust ring 2 m on landing) | 6 s | 60, max 6 |

### 2.4 Objects and structure

| System | Trigger | Parameters | Visual | Lifetime | Budget |
|---|---|---|---|---|---|
| **NS_BreakFree** (T) | Active pull (Haul, Jack push, or the Kick) exceeds break-free (05 section 6.2: rack 200 N, stack 500 N, bolted 800 N, rated 1,000 N). Hang lines never unseat a placed object (05 v0.2.1) | Placement, MassTier | 0.2 s lead: object shudders (vertex offset 2 cm at 15 Hz), scrape dust at contact edges. Then a visible lurch with a dust ring sized to tier (Medium 0.5 m, Heavy 1.5 m) | 0.6 s | 40, max 8 |
| **NS_Rated_Groan** (T) | Rated load starts moving | Material | 1.0 s minimum lead before the authored consequence: dust streams from the element's seat (concrete grey, steel rust-red), 30 per second, falling 2 m; authored debris (bolts, grit) trickles. At consequence: the material's fracture set below | 1.0 s + | 60, max 4 |
| **NS_Fracture_Wood** | Market stall, furniture, planks fracture (300 N·s) | Impulse | Splinter burst 20, dust puff, 1 s. Player bodies count as impactors (05 section 6.5): a flipped Rigger through a stall triggers this | 1.5 s | 30, max 10 |
| **NS_Fracture_Scaffold** | 800 N·s | Impulse | Clamp sparks 15, board dust, falling grit for 2 s. Fragments: Light boards, Light clamps, one Medium frame piece | 2 s | 50, max 6 |
| **NS_Fracture_House** | 2,000 N·s (small, 1,200 kg) or 3,500 N·s (large, 2,800 kg). Hangers are rated 1,000 N; the impulse comes from gravity (a cut slack cable, the Cable Day settle, Sway pumping), never from a line swinging a house on its own (05 v0.2.1) | Impulse, Size | Plaster dust cloud 3 m, 200 particles, 40% opacity cap, roof tile scatter (instanced meshes), contents puff. Dust must clear below 40% opacity within 1.5 s so lines inside read | 4 s | 260, max 2 (03 budget: four dynamic houses; two fracturing at once) |
| **NS_Fracture_Catwalk** | 2,500 N·s | Impulse | Grate shear sparks 40, rust dust, bolt scatter | 2 s | 70, max 3 |
| **NS_Fracture_Tank** | 1,500 N·s | Impulse | Shell spray plus water rush (2.5) | 2 s + | 60, max 2 |
| **NS_Container_Dent** | 6,000 N·s | Impulse, HitPoint | Dent decal, paint flake sparks 20, doors blow open with a 0.5 m dust puff and contents spill (pooled props) | 1 s | 30, max 4 |
| **NS_Structure_Dust** | Any scripted collapse, groan phase | Lead_s | Continuous dust fall from the failing element for Lead_s; sized so a player under it sees it fall past their camera | Lead_s | 40 per second, max 6 |
| **NS_Heavy_Roll** | Heavy object on rollers or skids moving under a line (05 section 6.1a) | Friction, Speed | Grit kicked from the roller or skid contact line, 10 per second per meter of width, plus a roller squeal decal shimmer. Heavy objects on a bare floor are fixed scenery and get no motion VFX; they fracture or dent instead | While moving | 40, max 4 |
| **NS_Debris_Light** | Fragments too small to keep tier | | Light props with a 10 s lifetime (05 section 6.5), fading in the last 1 s | 10 s | Pooled, 200 arena-wide |

### 2.5 Volatiles and force volumes

Radii from 05 section 6.4. Blast visuals must show the radius so players learn it.

| System | Trigger | Parameters | Visual | Lifetime | Budget |
|---|---|---|---|---|---|
| **NS_Volatile_Armed** (T) | Impact over threshold, 0.3 s before blast | Type | Hiss venting: a thin high-pressure jet from the valve, 40 particles, with a one-frame Caution flash on the container. This is the only warning. A Dog seating in a demolition charge does not arm it (05 section 6.4): NS_Dog_Seat plays a plain seat and nothing else, so a venting charge always means an impact or a blast is coming | 0.3 s | 40, max 6 |
| **NS_Blast_Propane** | Propane cylinder, 4 m radius | | Fireball 2 m, 0.3 s, peaking 0.85 L_line one frame then under 0.6 by 0.1 s. A shock ring expanding to exactly 4 m over 0.2 s (the Stall radius at 2 m is a brighter inner ring). Smoke 3 s, 40% opacity cap. 60 shrapnel sparks | 3 s | 200, max 3 |
| **NS_Blast_Fuel** | Fuel drum, 3 m radius | | Dirtier fireball, black smoke 5 s, shock ring to 3 m, then NS_Fire_Patch. Under more than 1 m of water (05 section 6.4): half impulse, no fire patch, a muted bubble dome replaces the fireball | 5 s | 220, max 3 |
| **NS_Fire_Patch** (T) | Fuel blast | | 3 m patch for 6 s. Low flames 0.5 m, 80 particles, 0.5 L_line max, heat shimmer, an outward push visible as embers streaming at 4 m/s. Dies over the last 1 s | 6 s | 120, max 3 |
| **NS_Blast_Charge** | Demolition charge, 5 m radius | | Hard, fast, grey: concrete dust dominates, flash 0.85 L_line one frame, shock ring to 5 m, structural fracture set follows | 3 s | 240, max 2 |
| **NS_Gas_Jet** (T) | Manifold (fixed world geometry, no line moves it) vents on a Jack over 600 N or a blast: 6 m jet, 15 m/s, 3 s | Direction | Dense white jet cone 6 m, 150 particles per second, frost crystals on the manifold, 40% opacity cap at the jet axis so a line through it reads | 3 s | 300, max 2 |
| **NS_Water_Rush** (T) | Tank rupture or Hold tilt | Direction, Volume | Surface foam sheet (mesh) plus 200 spray particles at the front, debris carried; 1 s lead as a swell before the push | Per arena | 250, max 2 |
| **NS_Updraft** (T) | Anchor gust, 40 s cycle | Strength | 1 s lead: dust and grit rising from the Well, 100 particles per second, Bone, accelerating upward; laundry-like scraps of Weave fibre spiral. Strongest at the Stump | Gust length | 300, max 1 |
| **NS_Pulse** (T) | Anchor Pulse, 3 min | | Marshal count drives a Stump glow at 0.5 L_line swelling for 3 s; the Pulse itself is a radial ground shock ring from the Stump to the Rim over 1 s, dust lifted in a wave, loose grit sliding outward | 4 s | 400, max 1 |
| **NS_Breathing** | Ballast lean, 90 s | Tilt | Water surface tilt in the Hold (mesh), chain swing, dust sifting from the Bays' racks on the lean, 20 per second | Continuous | 60, max 1 |

### 2.6 Kinematics

| System | Trigger | Visual | Budget |
|---|---|---|---|
| **NS_Hauler_Sparks** (T) | Hauler on rails, lights at 3 s lead | Wheel sparks on curves 30 per second, warning lights (mesh emissive, Caution, 0.4 L_line, never above), rail dust. Shift Change: sparks double | 60, max 4 |
| **NS_Gantry** (T) | Gantry traverse and drop, 2 s lead | Magnet hum glow (0.3 L_line, Bone-blue), cable sway, at release a 1-frame magnet flash and the load's own dust on impact | 40, max 1 |
| **NS_Lift** | Hangtown Lift | Pulley grease glint, counterweight dust on its guides | 20, max 1 |
| **NS_Ring** | Anchor Maintenance Ring | Gear sparks at the drive, 10 per second | 20, max 1 |

### 2.7 Mid-match events

Each has a 5 s telegraph per 05 section 9.2 and 03: authored sag or sparks. Ballast Shear: cutting-torch sparks along the bay wall seam for 5 s, then the pre-fractured section goes with NS_Rated_Groan and a 300-particle debris fall into the gorge. Yard Shift Change: every Hauler lamp strobes for 5 s. Hangtown Cable Day: the main load cable sheds strands (NS_Line_Slip scaled up) for 5 s, then NS_Fracture_House and NS_Structure_Dust along the market ring. Anchor Scaffold Collapse: a visible sag (mesh) with NS_Fracture_Scaffold chained down five levels.

## 3. Budget summary

| Context | Particle ceiling (LOD 0 equivalent) | Hard caps |
|---|---|---|
| Scrap 3v3, steady state | 2,500 | 12 lines, 6 seat effects, 4 drag skids |
| Peak (event plus two blasts) | 6,000 for 2 s | 2 blasts, 1 event, 2 fractures of Heavy tier |
| Free Scrap 8 players | 3,500 | 20 lines |
| Broadcast and spectator | plus 20% for line tags and outlines | |

Niagara systems marked (T) are tells. They never cull on distance inside the arena bound, never drop below LOD 2, and are excluded from the dynamic budget scaler. Everything else scales first.

## 4. Team-tint rules, summarized

- Tinted: line ribbon, Dog trail, Dog fire flash, Catch Plate seat ring, snap embers (first 0.2 s), hoisted knot, Claim marker ring.
- Never tinted: dust, debris, fire, water, gas, blasts, structural effects, Stall Gel (always `#8FE3CF`), Hooks, kinematics (always Caution or Bone).
- Enemy lines: dark 1 m tick pattern on the ribbon. Own lines: unbroken. Third crew: double tick.
- The Mono accessibility palette replaces all tints with Bone and relies on the tick pattern. Every tinted system must therefore also read in Bone.
