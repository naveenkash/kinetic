# BALLAST: Events and Moving Parts

_Arena: Ballast. File purpose: the Breathing cycle second by second, the Shear in full, secondary cycles, the Scrap sudden-death event, every tell with its lead time, and reset rules. Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Inventions are marked **PROPOSED**._

## 1. The big moving part: the Breathing

The hull (docs/objects/kinematics/breathing-hull.md) is a single kinematic root. Its pitch θ follows θ(t) = 70 + 6 sin(2π t / 90) degrees, t in seconds from the Marshal's "live" call. Everything in the arena is a child of that root and simulates in its frame, so the Breathing reads as a slow change in the direction of gravity: up to 6 degrees toward the Belly (+Y) at full lean, 6 degrees toward the dorsal wall at full recovery.

Derived values:
- Original deck plates: 20 + (θ - 70) degrees, so 14 to 26 degrees.
- Prep-crew surfaces: 0 ± 6 degrees.
- Slide onset for loose Medium props on steel plate: **PROPOSED** 24 degrees (friction 0.45), so deck plates slide props while θ ≥ 74, which is t 10.5 to 34.5 s each cycle.
- Container stacks: break-free 500 N normally (Rigger, Doubler, Breaker), **PROPOSED** 250 N while θ ≥ 74, which lets a Climber or Scrapper (400 N) start an Avalanche for 24 s of every 90. Nothing releases on its own; stacks creep 0.3 m per cycle against their rack end-stops and shudder. The containers ride the magnetic racks' dead rollers (05 section 6.1a, friction 0.02), so the end-stop is the only thing holding a stack on a 14 to 26 degree rack; once a bottom container passes the end-stop's break-free the stack rolls under its own weight at 3.3 to 4.3 m/s² toward the Belly, leaves the rack at about 6.8 m/s, skids across the level Belly strip on bare grating (friction 0.3, 2.9 m/s² of braking, 8 m to stop) and drops into the gutter before it stops.
- Wash shove volume: authored on a 100 kg reference body (05 section 6.6) as 0.8 x (θ - 70) m/s² toward +Y when θ > 70, so 0 to 4.8 m/s² reference; a body of mass m feels 0.8 x (θ - 70) x 100 / m. At full lean: Climber 5.3 m/s², Scrapper 4.4, Rigger and Doubler 3.7, Breaker 2.2. A Climber wading at hull pitch above 73.8 degrees and a Scrapper above 75.5 are off their feet and swept toward the rent; everyone else skids and keeps footing. It acts on players and loose props only: crates (100 kg) feel the reference value, a 12 kg plank feels 40 m/s² and skips across the surface, and the fixed tanks and racked cylinders ignore it. When θ < 70 the volume reverses at a quarter strength toward the dorsal wall.
- Hanging objects (drive chains, slings, Hang loads, Pendulum players) shift their rest point by 6 degrees of their hang length: 1.9 m on an 18 m chain, 4.2 m on a 40 m Rigger Pendulum.

### Cycle table (one 90 s cycle; repeats)

| t (s) | θ (deg) | Deck plate | Prep surface | State | Audio |
|---|---|---|---|---|---|
| 0 | 70.0 | 20 | 0 | Neutral, leaning toward the nose | Low hull hum |
| 5 | 72.1 | 22 | 2 | Chains begin to drift +Y | Chain clink rises |
| 10 | 73.9 | 24 | 4 | Wash shove 3.1 m/s² (reference); Climbers in the Wash lose footing at 74 | Water moving |
| **10.5** | 74.0 | 24 | 4 | **Slide window opens.** Loose Medium on plate starts moving. Stacks drop to 250 N break-free | Scrape of crates on the Slab, rack dogs tick |
| 15 | 75.2 | 25 | 5 | Crates reach the slab windows and the gutter | First crate splash in the Wash |
| 20 | 75.9 | 26 | 6 | Near full lean. Wash shove 4.7 m/s² (reference); Climbers and Scrappers in the Wash are swept | Stacks creak continuously |
| **21.5** | 76.0 | 26 | 6 | **Reversal groan** (1.0 s lead) | 40 Hz moan rising to a steel shriek |
| **22.5** | 76.0 | 26 | 6 | Full lean. Hull reverses | Shriek cuts to silence for 0.5 s |
| 25 | 75.9 | 26 | 6 | Still sliding | Resumes |
| 30 | 74.8 | 25 | 5 | Sliding slows | |
| **34.5** | 74.0 | 24 | 4 | **Slide window closes.** Stacks back to 500 N | Last scrape |
| 40 | 72.1 | 22 | 2 | Chains swinging back | |
| 45 | 70.0 | 20 | 0 | Neutral, recovering toward dorsal | Hum |
| 50 | 67.9 | 18 | -2 | Wash trickles toward the dorsal wall at 1.2 m/s² max | Water retreats from the rent |
| 60 | 64.8 | 15 | -5 | Stacks settle hard against end-stops | A single rack thud per bay |
| **66.5** | 64.0 | 14 | -6 | **Reversal groan** (1.0 s lead) | Lower pitched than the lean groan |
| **67.5** | 64.0 | 14 | -6 | Full recovery. Hull reverses | |
| 75 | 65.2 | 15 | -5 | Dust falls from the Crown plating | |
| 80 | 67.9 | 18 | -2 | | |
| 90 | 70.0 | 20 | 0 | Cycle restarts | Hum |

Scrap rounds are 150 s: a round sees one full cycle plus two thirds of a second. The Shear fires at t = 75 (1:15 remaining), when θ is 64.8 degrees and the stacks are at their most stable. The first lean after the Shear opens at t = 100.5 with 49.5 s on the clock, and the port racks' west ends now overhang the gorge. In Claim the Shear fires at t = 240 (4:00 remaining), which is the same phase, θ 64.8. In each Haul half it fires at t = 120 (2:00 remaining), θ 74.8, inside a slide window: the port wall goes while the crates are already moving.

### Lines and the moving hull

The Breathing hull is the frame everything is parented to, so it is never a kinematic that lines meet: a line strung between two hull points (a Draw line across the Spine, a Clothesline between galleries) moves with the hull and is never severed by it. The things that become kinematics during a round do sever lines they pass through (05 section 3.5, both Dogs release with the snap): the Shear panel and the port galleries from T-0, the Full Shear strips as they go, and a falling drive dog on its authored path down the shaft, which cuts every line across the Spine below it before it reaches the Plate. Lines attached to those kinematics themselves are not severed; that is what the Shear Caboose relies on. The drive chains and the dropped gantry beam are simulated bodies, not kinematics: they load lines, they do not sever them.

## 2. The mid-match event: the Shear

Fires at 1:15 remaining in a Scrap round, 4:00 remaining in Claim and 2:00 remaining in each Haul half (05 section 9.2), after a 5 s siren. Offscreen, HTR's cutters at the top of the port wall finish cut Shear B. The port Bay wall panel (30 m along Y, 36 m tall from Z 6 to Z 42) and everything bracketed to it shed into the gorge.

### Sequence

| Time | Event | Zones affected |
|---|---|---|
| T-5.0 | Siren. Marshal: "Shear." Sparks run down the port cut line from Z 42 to Z 6 over 5 s (authored sag: the panel bows outward 0.5 m) | Everywhere hears it; port side sees it |
| T-5.0 to T-0 | Port Upper Gallery floodlight flickers. Port deck's outer 4 m strip shudders. Structural tape on the future edges begins to glow (edge-lit emissive, **PROPOSED**) | Port Lower Bay, Port Upper Gallery, Hold Ledge port end, Crown deck port edge |
| T-0 | Cut completes. Panel releases, rotating outward about its bottom edge at Z 6. Port Upper Gallery (7 catwalk sections) and the port deck's outer 4 m strip are parented to it and go with it. Anything standing on them is airborne over the gorge. From T-0 the panel is a kinematic: any line it passes through on its way out (a Clothesline across the port bay, a Hang to the port wall from the deck) is severed; lines attached to the panel or the galleries ride with them | Port Upper Gallery ceases to exist |
| T+0 to T+2.5 | Panel rotates to 90 degrees and falls free. Four demolition charges on the ribs go with it (they do not detonate; they are HTR's, cut-wired) | |
| T+1.0 | Shear shake: a 0.5 s, 0.2 m lateral jolt in the hull frame. Every container stack with a strain flag (struck by any object or line this round) releases its bottom container. Unflagged stacks shudder and creep 0.5 m. The port Belly rack's two stacks are flagged by default: they lose their bottom containers toward the gutter, their west ends (X -17.5 to -14) passing over the open gorge on the way | Both bays, port Belly rack first |
| T+2.5 | Daylight floods the Bays from port. Dust. Post-process shift (lighting file) | Both Bays, Hold Ledge, Crown |
| T+4.0 | Two Hooks rise into view off the port side at Z 20, 20 m out. Rotors audible in the Bays | |
| T+6.0 | New edge markers on: Hold Ledge port end, Port Lower Bay deck at X -14, Spine collar port hatch (now opens onto air; the hatch grating remains as a 1.5 m stub), Crown deck port edge above the gap | |

### After the Shear, zone by zone

| Zone | Before | After |
|---|---|---|
| The Wash | Enclosed, dark | Side-lit from Z 6 up on port. No new edge: the port wall below Z 6 stands |
| Hold Ledge | Enclosed | Port end (X -18) is a Well edge. A Jack along the ledge toward port is Overboard |
| Port Lower Bay | 14 m wide, walled | 10 m wide (X -14 to -4), open to the gorge. The port Belly rack's stacks are two containers tall. Containers still roll toward the gutter in Y, not toward the port edge; the Shear changes the edges, not the slides. Every Jack toward port on this deck is now Overboard |
| Starboard Lower Bay | Mirror of port | Unchanged; now the safe bay. Expect crews to cross the Mouth Plate toward it |
| Mouth Plate | Watched from both bays | Lit from port. A line Hauling someone off the Plate toward port now has 14 m of deck then air |
| Port Upper Gallery | 14 m catwalk, Z 26 | Gone. The port stair's top flight ends in air (its last 2 m fall with the gallery) |
| Starboard Upper Gallery | Spawn B | Unchanged. Clothesline lane across the shaft now ends in air on the port side |
| Rack Tops | 8 m above grating | The port stacks' west ends overhang the gorge by 3.5 m; a stack top is now a perch beside a 100 m drop |
| The Spine | Enclosed shaft | Port collar hatch opens onto air. A Draw line across the shaft at Z 26 now has a Well on one end |
| Crown Deck | Lip and Spine head | Port deck edge is a Well edge. Spawn A at 36 m from it stays legal |
| Crown Net | Debris net | Unchanged |
| Cut Gantry | Idle | Unchanged. The port rail now overhangs the gap |

### Demolition charges before the Shear

The four charges on the port wall ribs are HTR's and are cut-wired to the Shear, but they are still charges: impact over 6 m/s to a charge (a Hauled crate, a thrown cylinder, a player) detonates it per 05 section 6.4. Blast radius 5 m, impulse 2,500 N·s. **PROPOSED** structural consequence: it blows a 6 m by 6 m window out of the port wall around that rib, not the whole panel. The window is an early, small Well edge at Z 17 to 23. The Shear still fires on schedule and takes the rest.

## 3. Secondary cycles

| Cycle | Period | What happens | Gameplay |
|---|---|---|---|
| Hooks in the gorge | Continuous, 20 s patrol loop | Two drones drift along the rent, rotors audible across the Hold | Ring-out tell lives here. They spin up 0.5 s before any kill-volume entry |
| HTR cutters above the cut face | Sparks every 30 s for 3 s, offscreen above Z 56 | Light spill down the Spine head and the Crown dorsal plating | Flavour until T-5, when they become the Shear's sag |
| Hold generator | 12 s loop | The Hold floodlights dip for 0.3 s | Reminds the Hold that its light comes from three drums on the ledge |
| Drive chains | 90 s, locked to the Breathing | Lower ends travel 1.9 m each way | Swinging Medium weights over the Mouth Plate |
| Rack creep | 90 s | 0.3 m per cycle; a stack that has crept 1.5 m (five cycles) sets its strain flag | In Claim, by minute eight every stack is flagged |
| Gantry hook sway | 90 s | The 200 kg hook block swings 0.3 m | A Dog on it makes a pendulum anchor that moves |

## 4. Sudden death: Full Shear (**PROPOSED**)

Scrap tie-break per 05 section 9.1 (time-out with equal riggers). The Marshal calls "Full shear." The arena shrinks from the outside in until one crew is gone.

| Time | Event | Playable area remaining |
|---|---|---|
| 0 | 5 s siren. Sparks run down the starboard cut line | All |
| 5 | Starboard Bay wall panel, Starboard Upper Gallery, starboard deck's outer 4 m strip shed. Spawn B's gallery is gone | Both Lower Bay decks (10 m wide each), Mouth Plate, Spine, Hold, Crown |
| 9 to 25 | Both Lower Bay decks fracture outward from the hull walls inward in 2 m strips, one strip every 4 s (4 strips per side), each strip with a 1.0 s groan first | By t 25: only the Mouth Plate and the cross-catwalk's 4 m stubs |
| 25 | Hold Ledge drops into the Wash (both stairs with it). Water tanks rupture in sequence, port then starboard, 2 s apart: the Wash becomes a 12 m/s² (100 kg reference) rush toward the rent for 8 s, which lifts every class including an unbraced Breaker (5.5 m/s²) | Hold is a flood |
| 33 | Crown Net cut away. Crown deck's outer 6 m strips (port and starboard) fracture | Crown inner 24 m by 30 m |
| 40 | Breathing amplitude doubles: θ = 70 ± 12 degrees, 90 s cycle unchanged (**PROPOSED**). Prep surfaces now tilt 12 degrees; crates slide on grating at the extremes; the Mouth Plate becomes a ramp toward the Belly every 45 s | |
| 60 | Mouth Plate fractures. Marker (if Claim rules applied) is irrelevant in Scrap | Spine ladders, dog ledges, collar ring, Crown inner deck, gantry |
| 90 | The drive dog at Z 42 releases on its own (authored), falls the shaft | Spine below Z 16 is a crush zone for 3 s |
| First Stall | Round ends | |

Design intent: the shrink forces play into the Spine and the Crown, where Breakers own the dogs and Climbers own the air, within 60 s.

## 5. Tells (05 section 11, arena specifics)

| System | Audio | Visual | Lead |
|---|---|---|---|
| Breathing reversal | 40 Hz hull moan rising to a shriek | Dust sifts from every overhead; chains hang at their extreme | 1.0 s |
| Slide window opening | Scrape of crates on plate, rack dogs ticking | Crates visibly creep; prep-crew tape on slab windows brightens (**PROPOSED** edge-lit tape) | 0.5 s before first prop moves |
| Container break-free | Lurch and scrape | Container shudders 0.2 m | 0.2 s |
| Rack pull (rated) | Structural groan, bolt shear pops | Dust from the rack feet, Slab plating buckles | 1.0 s before the rack moves |
| Drive dog release (rated) | Deep hydraulic groan, then a steel scream down the shaft | Dust ring around the housing, the piston's tape flashes | 1.0 s before the piston leaves its housing; 1.3 s more before it passes Z 16 from Z 26 |
| Gantry column pull (rated) | Groan, column foot sparks | Beam sags 0.3 m at that end | 1.0 s |
| Tank rupture | Hollow boom | Shell splits, water sheets out | Rush volume begins 1.0 s after the boom (force volume tell) |
| Wash shove | Water moving, slap against the tanks | Surface ripples toward +Y | 1.0 s (volume ramps with θ, so the ramp is the tell) |
| Demolition charge armed | Hiss | Venting at the charge | 0.3 s |
| Propane, drum, manifold | Per 05 section 6.4 | Per 05 | 0.3 s |
| The Shear | Siren, 5 s | Sparks down the port cut line, panel bows 0.5 m outward, tape glows | 5 s |
| Shear shake stack releases | Rack dogs snap | Flagged stacks shudder hard | 1.0 s after T-0, 0.2 s lurch before release |
| Ring-out | Hooks spin up | | 0.5 s |
| Full Shear strips | Groan per strip | Strip sags 0.2 m | 1.0 s per strip |

## 6. Reset rules

Between rounds (05 section 9.2 step 5):
- Every object returns to its authored position and placement state, including all 24 containers, both racks' bolts, all six drive dogs, the four columns, the Mouth Plate, both galleries, the port wall panel, the four charges, both tanks' shells and water, the Crown Net and the gantry beam.
- The Breathing restarts at t = 0 (θ 70, leaning toward the nose) on the "live" call, so the first slide window of every round opens at 10.5 s.
- Strain flags clear. Rack creep resets to 0.
- Set lines clear. Dogs recall.
- Wash water returns to 0.6 m, Sump to 1.2 m, shove volume to 0.
- Hooks return to their rent station; the port-side pair despawns.
- In Haul, the spool returns to the Mouth Plate sled and crews swap cradles. In Claim (single round) nothing resets except the marker on Well entry.
- Full Shear is Scrap-only and never persists.
