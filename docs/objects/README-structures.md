# Objects: structures, rated loads, kinematics, environment

Index and shared specification for the object documents in `structures/`, `rated/`, `kinematics/`, and `environment/`. Launch format is 3v3 (3v3v3 is post-launch; the Rose team tint is reserved for it). Heavy objects move only when staged on rollers (friction 0.02 to start, 0.005 rolling) or skids (0.05) per 05 section 6.1a; on a bare floor they are fixed scenery, and the object docs say which staging each has (Avalanche stacks are on roller pallets). **Arena geometry authority is `docs/arenas/*/layout.md`** (Anchor Rim datum Z 0: Scaffold decks L1 to L5 at Z 6 to 38, Ring Z 42, Crown Z 60; Ballast Hold floor Z 0 and the forward 72 m of the Spine, Crown deck Z 44; the Yard Rails at 36 m). Where an object doc's cycle numbers differ from `docs/arenas/*/events-and-moving-parts.md`, the arena file wins and the object doc is to be corrected. Every file is standalone. Every number not marked **PROPOSED** comes from `docs/bible/05-core-mechanics.md` (v0.2, tension in newtons) or `03-arenas.md`. Where this index and an object file disagree, the object file is newer; fix the index.

## Index

### structures/ (destructible or load-bearing built elements)

| File | One line |
|---|---|
| `structures/scaffold-section.md` | Medium 2.5 m scaffold bay, 800 N s fracture, the Anchor's five rings; anchored by the maintenance clamps. |
| `structures/catwalk-section.md` | Heavy 4 m grating crossover, 2,500 N s, Ballast's Bays and the Spine's shaft bridges; the piston's fall line. |
| `structures/swing-bridge.md` | Heavy 12 m cable-hung deck with orange slack hitches, Hangtown Mid Hang; the Cut-Loose pendulum. |
| `structures/market-platform.md` | Hangtown's 18 m hanging ring, Claim marker site, drops to 40 degrees on Cable Day. |
| `structures/plank-bridge.md` | Medium 4.5 m three-plank trench crossing in the Yard, 600 N s **PROPOSED**; the 12 kg plank factory. |
| `structures/container-stack.md` | Heavy 2,000 kg containers on dead magnetic racks, 500 N stack break-free, Ballast; the Avalanche. |
| `structures/billet-stack.md` | Heavy 2,800 kg Weave billet crates three and four high, the Yard; slow topple, pin kills. |
| `structures/hull-plate-section.md` | Rated 7,500 kg cut hull slab: Yard Stacks terrain, propped Floor sections, the Gantry's load. |
| `structures/walkway-on-cable.md` | Heavy grating runs hung under Hangtown's main cables; fold down their length when a cable goes. |
| `structures/collar-rim-ground.md` | Hangtown's two solid rims: spawns, prep stores, anchor pin seats, Lift headframe. |
| `structures/spine-ladder.md` | Ballast's four caged shaft ladders, the Breaker's honest route; climb speed **PROPOSED**. |

### rated/ (loads only a Breaker, or a Doubler at overspin, can move; each with an authored consequence)

| File | One line |
|---|---|
| `rated/drive-dog-piston.md` | 40,000 kg piston in the Spine; falls the shaft, takes the bridges, ruptures two Hold tanks. |
| `rated/magnetic-rack.md` | 5,500 kg rack frame with its stack; tips across a Ballast bay with a catwalk end on it. |
| `rated/structural-column.md` | 4,200 kg column; drops the Yard control shack and its mast into the Stacks, or a Ballast catwalk pair into the Hold. |
| `rated/anchor-pin.md` | 4,000 kg pin in a Collar rim head; drops a main cable, its walkway, and its houses' footing. |
| `rated/maintenance-clamp.md` | 3,500 kg Stump clamp; drops a whole Scaffold ring down the outside of the Stump. |
| `rated/press-counterweight.md` | 9,000 kg kinematic rider; falls its tower, seats the ram, vents a hydraulic jet across the pit lip. |

### kinematics/ (infinite-mass scripted movers)

| File | One line |
|---|---|
| `kinematics/rail-hauler.md` | 12 m/s **PROPOSED** rail truck on a 40 s headway in three Yard trenches; klaxon 3 s; the Caboose. |
| `kinematics/gantry-crane.md` | 60 m gantry on 40 m rails, 150 s cycle, a 7,500 kg rated attachment released on a 2 s strobe; the Magnet Drop. |
| `kinematics/maintenance-ring.md` | 20 m radius ring at 1.05 m/s, 120 s per revolution around the Stump; a sweeper, not a hammer. |
| `kinematics/the-lift.md` | Hangtown's 3 m/s rope elevator with an 8,000 kg rated-attachment counterweight through the Mid Hang. |
| `kinematics/shear-press.md` | The Yard's Well: a 12 s ram cycle audible everywhere, with the kill volume on the pit floor. |
| `kinematics/breathing-hull.md` | Ballast's root: 6 degrees of pitch on a 90 s period, floors that slope, water that shoves, the Shear. |

### environment/ (dressing and world systems)

| File | One line |
|---|---|
| `environment/load-cable.md` | 90 mm uncuttable main cables; fail only by Cable Day or an anchor pin. |
| `environment/slack-cable.md` | Orange-taped secondaries; any Cutter, 0.6 s hold; house hitches at 1,000 N, bridge hitches at 800 N. |
| `environment/cable-anchor.md` | The three anchor silhouettes (load saddle, slack hitch, rated pin) and the Dog seats players aim at. |
| `environment/condemnation-notice.md` | Office, HTR, and Guild paper: the text layer and the stakes on Hangtown's doors. |
| `environment/chalk-and-graffiti.md` | Five hands of marks; Guild chalk is data-driven and truthful. |
| `environment/floodlight-mast.md` | The Office's light; shack masts fall with the column and darken a Rails sector. |
| `environment/broadcast-tower.md` | LayCast's Rim towers; a tipped or struck tower darkens an Anchor sector. |
| `environment/office-gallery.md` | The Marshal box: `NoDog`, unreachable, the Hooks' dock, the voice. |
| `environment/sponsor-banners.md` | Eight fictional sponsors with logo specs; cloth that shows force volumes. |
| `environment/weave-ribbon-fragment.md` | The valuable thing as a harmless Light prop; the unzip edge. |
| `environment/structural-tape.md` | Yellow-and-black rated tape and orange slack tape as a generated, validated system. |
| `environment/evidence-objects.md` | Six seasonal pieces of the Question, one per arena location, with every faction's reading. |
| `environment/force-volumes.md` | Updraft, Pulse, water rush, Hold flood, gas jet: profiles over time and radius. |
| `environment/well-kill-volume.md` | The four Wells: geometry, tells, Hook rules, payout heights. |

## Shared spec: authored consequence scripting

Applies to every rated load (05 section 6.3) and to the non-rated objects whose movement has a scripted result (a live broadcast tower guy at 800 N, a house hitch at 1,000 N, a propped Floor hull section). The load is simulated or kinematic; the consequence is scripted. One state machine, one set of timings, one set of hooks.

### States

| State | Entry condition | Exit |
|---|---|---|
| `Seated` | Round reset | A qualifying line force begins |
| `Stressed` | Any line force on the load's Haul target below threshold, or above threshold for under the commit time | Force drops to zero (back to `Seated`), or commit |
| `Committed` | See commit rule | Automatic after the groan |
| `Consequence` | Groan complete | Timeline complete |
| `Spent` | Timeline complete | Round reset |

### Commit rule

- Threshold: the object's break-free (rated 1,000 N; bolted 800 N; house hitch 1,000 N). Thresholds are inclusive (05 section 6.2): a Breaker's 1,000 N starts a rated load.
- Active pull only: break-free and commit are tested against Haul force, the Jack's push, and the Kick (05 section 3.2, the Jack's capacitor-discharge launch phase). A Hang line never commits or unseats anything; it holds, up to the class Hang load, so any class Pendulums or Gallows-anchors from stacks, columns, clamps, and pins without consequence.
- After the groan a rated load that is line-driven moves at 0.17 to 0.27 m/s² (1,000 N on 3,700 to 6,000 kg). That slowness is intended and is itself a tell (05 section 6.2). Object docs whose consequences are gravity-driven after release say so.
- Direction: the component of the summed line force (05 section 6.2a: several lines add as vectors) along the object's authored `FallAxis`, within a 45 degree cone. Force outside the cone counts as stress, never as commit. The structural tape's chevrons point along `FallAxis` (see `environment/structural-tape.md`).
- Duration: the in-cone component must stay at or above threshold for **0.6 s continuous** (**PROPOSED**). Strain accumulates at 1 x while above threshold and decays at 2 x when below, so a flicker across the threshold does not commit and a cut line before 0.6 s cancels with no effect beyond the tells. A Doubler's overspin (800 N ramping to 1,200 N over 1.0 s, snap at 1.8 s) crosses 1,000 N at 0.5 s and holds 1.3 s: it commits a rated load on a single overspun Haul, which 05 section 3.3 permits.
- Who (canon, 05 section 6.2 and CHANGELOG v0.2.2): for rated, a Breaker alone, a Doubler at overspin, or any two or more lines whose active pull sums to 1,000 N along the same axis (a crew: two Riggers, a Rigger and a Scrapper, three Climbers); for bolted, a Doubler's single line, a Breaker, or lines summing to 800 N. The 1,000 N is a **commit trigger**, not a sustained drive: after the 0.6 s commit the authored timeline carries the whole load regardless of friction or whether the lines stay on. Rated attachments (the Gantry load, the Lift counterweight) release on any active pull reaching 1,000 N the same way.
- On commit the line is released from the load's constraint logic: the consequence does not depend on the line staying attached. The Breaker's Dog stays seated as a Dog for the rest of its life; the load moves under script.

### Tell sequence (05 section 11)

| Phase | Duration | Audio | Visual | Notes |
|---|---|---|---|---|
| Stressed | while below commit | object creak, line singing | object shudders, tape flutters, dust | Repeats freely. |
| Break-free lurch | 0.2 s | lurch, scrape, `SFX_Tape_Tear` | object shifts 20 to 60 mm, tape splits at the target | Fires once, at commit. |
| Groan | **1.0 s minimum**, per object up to 1.5 s (the piston uses 1.2 s) | structural groan, object-specific sequence (bolt shears, relief valve, cable sing) | dust, authored debris (Light props, 10 s), lamp swing, strip flicker along the affected structure | The lead time for everyone else. Marshal VO line fires at groan start. |
| Consequence | per object, 1.4 to 7.6 s | object-specific | object-specific | Deterministic timeline. |
| Spent | to round end | ambient (mist, dripping, swinging) | the arena's new shape | Static collision, new Dog seats. |

The groan is never shorter than 1.0 s (05 section 11, rated load moving). Object docs state their own groan length; none is under 1.0 s.

### Consequence timeline rules

- Server-authoritative, server time-synced: the server sets `ConsequenceStartTime`; clients evaluate the same timeline from that stamp. No per-frame transform replication of consequence bodies.
- Kinematic over simulated wherever the result must match across clients: tipping racks, folding columns, falling pistons, tipping towers, and sliding shack ruins are kinematic timelines with a mesh swap at the end. Simulation is used only for the loose debris the timeline hands off (containers at detach, boards from a fractured bay, spilled props), always from authored initial velocities and under a per-object budget.
- Structures in the path (catwalks, cage panels, terraces) are fractured by timeline events at the authored contact time, not by simulated contact.
- Players and props in the path receive the consequence body as a kinematic (05 section 6.7: anything a kinematic hits takes the full relative impact) for the duration of the move, and the moving body severs any line it passes through that is not attached to it (05 section 3.5, canon): the snap plays, both Dogs release. Every kinematic doc names what it cuts (the Hauler: trench lines below Z 3; the Gantry: lines across its bridge, trolley, falls, and load; the Ring: lines across the Well mouth from Z 38.5 to 44; the Lift car and counterweight: lines across their runs; the press ram: lines below Z -6.5; the Breathing's Shear panel and the port gallery strips riding it: every line they fall through). Hooks are exempt (**PROPOSED**). The 05 section 7.1 thresholds apply: player hits kinematic, Stall at 9 m/s; object hits player, Stall at 1,200 N s; crush, 8 kN for 0.5 s.
- Lighting and audio sector changes (`Lighting.Sector.<name>`, Marshal VO, global SFX) fire from timeline events through the arena's managers, replicated as gameplay tags.
- Tonnage credit: mass of the load times the distance its center moves during the consequence, credited to the committing player (several lines: split by force share); kills during the consequence credit the committing player as a Spotter-class assist (04).

### Reel, Kick, and the Guillotine (05 sections 3.2 and 10)

- **Reel:** inside Hang, holding Haul shortens the rod at 1.0 m/s and holding Jack lengthens it, bearing up to the class Hang load. The Gallows hoists by Reel (slow, strong, silent except the ratchet); only a Breaker's Haul lifts a Climber outright. Hanging players climb by Reel. Object docs say "Reel" wherever a player climbs a line or a load is lifted into place.
- **Kick interlock:** the Kick (the Jack's capacitor-discharge launch to V) never fires when either Dog is in a player's Catch Plate; a Jack against a player is the T push only. Overboard is therefore a skid of a few m/s² that stays inside the panic window. The Scrapper's 30% bonus applies to Haul acceleration and the Jack push, not to the Kick's V.
- **Guillotine step 2:** recall or cut your own set line; gravity drops the load. Owners can recall set lines early. The docs no longer describe a Haul from the load to the enemy.

### Live-load selection

Each round the arena picks which rated loads are live (per the object docs: Ballast two pistons of eight and four racks of fourteen; the Yard one shack column, four propped Floor sections, the counterweight; Hangtown one pin per rim; the Anchor two clamps on two rings, never S5 and never the Collapse quadrant, and two live tower guys opposite the Collapse quadrant). Live loads carry tape, a work lamp, and HTR's `CUT` stencil; dead loads carry none. Selection is seeded per round and mirrored across the two spawn sides so neither crew starts nearer a live load.

### Reset (05 section 9.2 step 5)

- At round end every object returns to its authored transform and state: `Seated`, tape fresh, lamps on, sectors lit, fractured structures restored, debris pooled, force volumes spawned by consequences destroyed, the press cycle restarted on the round clock, kinematics re-phased to round Live.
- Rated-load consequences reset only between rounds (05 section 6.3). Nothing restores a consequence mid-round.
- Mid-match events (the Shear, Shift Change, Cable Day, the Scaffold Collapse) also reset between rounds and re-fire at 1:15 remaining in Scrap, 4:00 remaining in Claim, 2:00 remaining in each Haul half (05 section 9.2).
- Evidence objects (environment) reset position between rounds; collection state persists per player per season.
- Seasonal variants (04) are not resets: a Claim result changes the authored state for the next season and the object docs list those variants.

### Hooks every consequence exposes

`OnStressed(force, inCone)`, `OnCommit()`, `OnGroanStart()`, `OnRelease()`, `OnTimelineEvent(name, time)`, `OnSpent()`, plus `GetFallAxis()`, `GetHaulTargetSocket()`, `GetGroanDuration()`, `GetLive()`. The tape generator, the Guild chalk generator, the Marshal VO table, and the Tonnage ledger all read these.

## Contradictions and open items found in the bible

Logged here so the object docs can be read against them. Items ruled by CHANGELOG v0.2.2 are marked **RESOLVED** with the ruling; the docs have been updated to it.

1. **RESOLVED. Who moves rated loads.** A Breaker alone, a Doubler at overspin, or any lines summing to 1,000 N on one axis (a crew). 02 updated. Every rated doc says so.
2. **RESOLVED. The Pulse.** A 0.4 s outward shove, then a 0.6 s inhale toward the Stump, every 75 s from 0:55. `force-volumes.md` carries the profile; the falloff numbers remain **PROPOSED**. Note: `arenas/the-anchor/events-and-moving-parts.md` still shows a 1.5 s shove and 0.8 s inhale; the CHANGELOG value is used here.
3. **RESOLVED. Scale.** Geometry authority is `docs/arenas/*/layout.md`. The Anchor's Rim deck is 240 m up the Stump (Rim datum Z 0); Scaffold decks at Z 6, 14, 22, 30, 38; Ring Z 42; Crown Z 60. Ballast's Spine is the forward 72 m of the shaft; Hold floor Z 0, Crown deck Z 44. `scaffold-section.md`, `maintenance-clamp.md`, `maintenance-ring.md`, `spine-ladder.md`, `breathing-hull.md`, `drive-dog-piston.md`, and `catwalk-section.md` are aligned.
4. **RESOLVED. Pulse period.** 75 s from 0:55: two per Scrap round, six per Claim match.
5. **RESOLVED. The Anchor's ring-out tell** is the Stump's hum dropping an octave. `well-kill-volume.md` updated.
6. **Kiss break-free.** 05 section 10 still reads "racked cylinders need 2 kN"; 05 section 6.2 now says 200 N. The docs use 200 N.
7. **RESOLVED. Kinematics sever lines.** A kinematic severs any line it passes through (snap, both Dogs release); lines attached to the kinematic are not severed. The "Hauler hits the player" reading is removed from `plank-bridge.md` and `rail-hauler.md`; every kinematic doc names what it cuts.
8. **RESOLVED.** Walkway on cable 2,500 N s and plank bridge 600 N s are in 05 section 6.5.
9. **RESOLVED. Force volumes** are authored as acceleration on a 100 kg reference body; a body feels a_ref x 100 / m, capped at 2 x a_ref; players and free bodies only (loose objects and loads on a player's line; not cable-hung map objects, placed objects, lines, or Dogs).
10. **Counterweight tier overlap.** 05 section 6.1 lists "counterweights" under Heavy and "crane counterweights" under Rated; 03 lists the press and Lift counterweights as rated. The docs treat both as rated attachments per 05 section 6.7 (v0.2 resolved the kinematic-versus-rated question; the tier wording in 6.1 still overlaps).
11. **RESOLVED. Gantry at Shift Change** drops whatever the magnet holds; an empty magnet drops nothing and the next pick comes early. `gantry-crane.md` now follows the arena's 120 s cycle and mode phasing.
12. **RESOLVED. Ladders** are 2.0 m/s (Breaker 1.6) in 05 section 2.3.
13. **Sudden-death naming.** 05 section 9.1 v0.1 called the Scrap tie-break a "Pulse"; v0.2 renamed it to an arena-specific shrink event. No object doc uses the old name; the Anchor's Pulse is the only Pulse.
14. **HTR orange and slack-cable orange.** Not a contradiction in the bible, a readability risk: HTR livery (#E8541E) and the cuttable tape (#FF8A00) are both orange. `structural-tape.md` fixes the hue gap, the thread, and the never-on-the-same-object rule.
15. **Teodora and the First Rule.** 01 says she wrote it down; CHANGELOG v0.2.1 says she enforces it and is 72. No object doc depends on this; `chalk-and-graffiti.md` references her only in the apprenticeship.
16. **New, from the v0.2.2 pass.** The Yard Rails are moving to 36 m; `gantry-crane.md`, `floodlight-mast.md`, `structural-column.md`, and `office-gallery.md` use 36 m (cab 40 m), while `arenas/the-yard/layout.md` still reads 40 m in places. The Yard layout proposes the control shack as the Office gallery variant; `office-gallery.md` keeps a separate gallery and flags the choice. `arenas/the-anchor/events-and-moving-parts.md` Pulse timings (1.5 s and 0.8 s) differ from the CHANGELOG (0.4 s and 0.6 s). Hooks are proposed exempt from line severing.
