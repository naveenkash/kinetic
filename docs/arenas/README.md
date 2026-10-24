# KINETIC Arenas: Index, Comparison, and Level-Design Checklist

_File purpose: the entry point for the four launch arenas. Index of the 24 arena documents, a side-by-side comparison, and the shared level-design checklist derived from the eight map rules in docs/bible/03-arenas.md. Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md (v0.2 with v0.2.1 rulings), docs/bible/CHANGELOG.md. Anything not in those files is marked **PROPOSED** in the arena documents themselves._

All numbers in the arena documents are written against 05-core-mechanics v0.2 with the v0.2.1 and v0.2.2 rulings (Reel, Kick interlock, kinematics sever lines, force volumes on players and loose objects only, Free Scrap event at 3:00 remaining, spawn distance along walkable routes): pull tension in newtons (Climber 400, Scrapper 400, Rigger 600, Doubler 800 or 1,200 at overspin, Breaker 1,000), Jack launch speed V of 18 / 14 / 12 / 14 / 10 m/s, foot-holds 3 / 4 / 5 / 5 / 5 m/s² (braced Rigger 9, braced Breaker 20), break-free 0 / 200 / 500 / 800 / 1,000 N, and the mid-match event at 1:15 remaining in Scrap, 4:00 remaining in Claim, 2:00 remaining in each Haul half.

## 1. Index

Each arena has six files. Every file is standalone and states its sources in its header.

| Arena | overview | layout | prop-manifest | events-and-moving-parts | lighting-and-mood | signature-plays |
|---|---|---|---|---|---|---|
| Ballast | [ballast/overview.md](ballast/overview.md) | [ballast/layout.md](ballast/layout.md) | [ballast/prop-manifest.md](ballast/prop-manifest.md) | [ballast/events-and-moving-parts.md](ballast/events-and-moving-parts.md) | [ballast/lighting-and-mood.md](ballast/lighting-and-mood.md) | [ballast/signature-plays.md](ballast/signature-plays.md) |
| The Yard | [the-yard/overview.md](the-yard/overview.md) | [the-yard/layout.md](the-yard/layout.md) | [the-yard/prop-manifest.md](the-yard/prop-manifest.md) | [the-yard/events-and-moving-parts.md](the-yard/events-and-moving-parts.md) | [the-yard/lighting-and-mood.md](the-yard/lighting-and-mood.md) | [the-yard/signature-plays.md](the-yard/signature-plays.md) |
| Hangtown | [hangtown/overview.md](hangtown/overview.md) | [hangtown/layout.md](hangtown/layout.md) | [hangtown/prop-manifest.md](hangtown/prop-manifest.md) | [hangtown/events-and-moving-parts.md](hangtown/events-and-moving-parts.md) | [hangtown/lighting-and-mood.md](hangtown/lighting-and-mood.md) | [hangtown/signature-plays.md](hangtown/signature-plays.md) |
| The Anchor | [the-anchor/overview.md](the-anchor/overview.md) | [the-anchor/layout.md](the-anchor/layout.md) | [the-anchor/prop-manifest.md](the-anchor/prop-manifest.md) | [the-anchor/events-and-moving-parts.md](the-anchor/events-and-moving-parts.md) | [the-anchor/lighting-and-mood.md](the-anchor/lighting-and-mood.md) | [the-anchor/signature-plays.md](the-anchor/signature-plays.md) |

What each file holds:

| File | Contents |
|---|---|
| overview.md | Story, location in the Lay, who contests the claim, feel, one-sentence thesis, modes, player count, footprint, tier heights, round-length expectations |
| layout.md | Tier-by-tier, zone-by-zone walk (8 to 15 named zones), dimensions, floor materials, sightlines, per-class routes with line-length and launch-speed checks, cover, edges, ASCII plans per tier, ASCII side elevation, both spawns, Claim marker position, Haul trucks and spool start, Well kill-volume extent |
| prop-manifest.md | Complete placed-object inventory by zone (object doc path, count, placement, state, orientation, prep-crew reason), totals by mass tier, dynamic-object budget check with a recommended cap, arena-unique objects with inline mini-specs |
| events-and-moving-parts.md | Second-by-second table of the big moving part, the mid-match event from the T-5 s siren through the zone-by-zone "after" state, secondary cycles, the Scrap sudden-death shrink event, the tells table with lead times, reset rules |
| lighting-and-mood.md | Time of day, weather, sky, key and fill, practicals per zone, fog and atmosphere, 6 to 8 hex swatches, map rule 8 compliance per zone, post-process intent, sound bed, lighting change after the event, three beauty shots |
| signature-plays.md | 8 to 12 arena-unique plays with zone positions, class, step-by-step inputs in 05 section 10 style, timing against the moving part, the number that makes each lethal, and the counter |

Object documents referenced by the manifests live under docs/objects/ (props-light, props-medium, volatiles, heavy, structures, rated, kinematics, environment). Objects that exist in only one arena are specified inline in that arena's prop-manifest.md under "Arena-unique objects."

## 2. Comparison

| | Ballast | The Yard | Hangtown | The Anchor |
|---|---|---|---|---|
| Setting | Freight climber Tenacity, nose-down at 70 degrees in the Rucu Gorge below Collar Five | Collar Four Breaking Yard, in the Basin, night shift | Vilca's Hang, strung across the ravine between the halves of Collar Three | The Meridian Anchor, the Stump and the socket bowl |
| Footprint (X by Y) | 36 m by 56 m (hull section 36 m by 30 m; playable = forward 72 m of the 410 m hull, canon per v0.2.2) | 160 m by 90 m | 130 m by 70 m (100 m ravine span plus 15 m of rim at each end, PROPOSED) | 120 m by 120 m disc (radius 60 m; the kilometer bowl is vista; layout.md is the geometry authority per v0.2.2) |
| Total playable height | 52 m | 44 m (trench floor -4 m to Gantry cab 40 m; Rails lowered from 40 m to 36 m per v0.2.2) | 38 m | 68 m (bowl floor -8 m to Crown 60 m on the Rim datum) |
| Tiers (m above lowest playable floor) | Hold 0 (ledge 6); Bays 16 lower / 26 upper / rack tops 24 to 32; Crown 44 (net 38, gantry rail 52); Spine 0 to 44 | Floor -4 trenches / 0 islands / 3.5 grate footbridges; Stacks 2.7 / 5.4 / 8, control shack to 14; Rails 24 mast platforms / 36 walkways and Gantry beam / 40 cab | Low Hang 0 to 6; Mid Hang 14 to 16 (roofs 19); Upper Hang 30; Collar rims 32; Lift headframe 38 | Bowl floor 0 (Rim datum -8); Rim 8 (0); crane decks 18 (10); Scaffold L1 to L5 14 to 46 (6 to 38); Ring 50 (42); Crown 68 (60) |
| Big moving part | The Breathing: hull pitch 70 ± 6 degrees, 90 s cycle; slide window t 10.5 to 34.5 | The Haulers: 3 trenches, 12 m/s, one train per trench every 45 s, staggered 15 s. Plus the Gantry on a 120 s cycle | The Sway: 12 s period, ±1.5 m at Mid Hang, east and west halves out of phase. Plus the Lift on a 48 s cycle | The Updraft (the Breath): 40 s cycle, full from 32 to 37 s. Plus the Maintenance Ring (120 s per revolution) and the Pulse (every 75 s from 0:55; 0.4 s outward shove then 0.6 s inhale toward the Stump; canon per v0.2.2) |
| Mid-match event | The Shear: port Bay wall and both port galleries shed into the gorge | Shift Change: Gantry drops where it is, Hauler interval halves for 60 s | Cable Day: NE main cable gives, east half of the Market Ring drops to 40 degrees, three houses swing free | Scaffold Collapse: west quadrant, five levels, falls outward toward the Rim |
| Well type | The torn nose cone at the Belly end of the Hold (kill volume from Z -10 across the gorge); after the Shear, the open port side | The Shear Press pit, 14 m by 14 m at (45, 0), kill volume below -6 m, ram on a 20 s cycle | The ravine. Kill volume at Z -12 under the whole span; mist below Z -4 | The annular shaft between the Stump and the Scaffold, radius 12 to 16 m, kill volume below Z -15. No Hooks; the ring-out tell is the Stump's hum dropping an octave (v0.2.2) |
| Best modes | Scrap, Claim. Haul supported vertically up the Spine on the counterbalanced ratchet rail (canon) | All three. Claim primary; the Apprenticeship's first Claim Match map | Scrap, Claim. Haul not supported | Claim (marker on the Crown), Scrap. Haul not supported |
| Claim marker | Mouth Plate over the Spine, (0, 0, 16) | Marker Flat, Floor, (0, +12, 0) | Market Ring center, (0, 0, 16) | Crown center, (0, 0, 60) |
| Spawns | A: Crown Deck starboard Belly corner (44 m). B: Starboard Upper Gallery dorsal end (26 m). Both starboard because the Shear removes port; crews swap each round (PROPOSED) | A: West Mesa (-68, -39, 8). B: East Mesa (+68, +39, 8) | A: West Collar rim (-58, 0, 32). B: East Collar rim (+58, 0, 32) | N and S crane decks (0, ±50, 10), canon per 05 section 9.3 v0.2.2 |
| Dominant class | Breaker; Climber second | Rigger; Breaker second | Scrapper; Climber second; Breaker weakest | Climber; Rigger second |
| Scrap sudden death (PROPOSED) | Full Shear: starboard wall goes too; decks fracture inward in strips over 60 s; only the Spine and Crown remain | Lockout: grates retract, Haulers run continuously, two kinematic blast curtains advance from X = ±80 at 1.0 m/s and hold at ±10 | Second Cable: SW main cable gives, west ring half drops, nets and terraces peel away over 20 s, Hub goes at +45 s | Held Breath: the Breath locks at full, the Pulse fires every 15 s, an Office-line cordon contracts from r 60 to r 24 at 1 m/s |
| Dynamic cap (simultaneous) | Heavy 12, Medium 40, Light 120 | Heavy 14, Medium 50, Light 150 | Heavy 10 (houses at most 4), Medium 36, Light 120 | Heavy 10, Medium 40, Light 100, plus a 150-fragment pool |
| Rated loads | 6 drive dogs, 4 magnetic racks, 4 structural columns | 4 uncut hull sections, press counterweight, 2 structural columns | 4 anchor pins, Lift counterweight, 14 house hitches | 20 maintenance clamps, 2 crane counterweight slabs |
| Light | Late afternoon sun down the open Crown cut; Hold lit by floodlights and water caustics | Night shift under floodlight masts, overcast with sodium glow | Blue hour, Office masts on the rims, warm practicals in evacuated houses | Hard equatorial noon, white concrete, deep shadow inside the Scaffold |
| Seasonal consequence (04) | HTR win: Crown and Spine only next season. Guild win: Breathing slows, Shear removed | Guild win keeps the schedule. HTR win doubles the Haulers permanently | Hanger win keeps it standing. HTR win removes a quarter of the map | Winner's colors on the Rim cranes and Face on the Crown board |

### Tier-reach summary (map rule 3)

Floor Slingshot vertical gain per class under 05 v0.2 (self-dog, Dog B at the feet, Jack): Climber 30 m, Doubler 27 m, Scrapper 15.8 m, Rigger 13.8 m, Breaker 9.4 m. A Climber at 18 m/s and 45 degrees travels about 33 m horizontally. No pull tension lifts a player (every T is below body weight), so there is no grapple-up anywhere; vertical movement is the Kick, the Pendulum, ladders, lifts, and kinematics.

| Arena | Largest tier gap | Climber in one Slingshot | Breaker honest route |
|---|---|---|---|
| Ballast | Mouth Plate (16) to Crown (44): 28 m | Yes, 2 m to spare, playtest it | Spine ladders at 1.6 m/s (ladders are 2.0 m/s, Breaker 1.6, per v0.2.2) |
| The Yard | Stacks (8) to Rails (36): 28 m | Yes, direct, 2 m to spare; the shack roof (14 m) gives 6 m more on the west | Control shack stair tower and mast ladders at 1.6 m/s |
| Hangtown | Mid (14 to 16) to Upper (30): 16 m | Yes | The Lift (48 s cycle) and the West Face Spine ladder |
| The Anchor | L5 (38) to Crown (60): 22 m, Rim (0) to L1 (6) to L5 is ladders | Yes from L5; from L4 on the Breath | Ladder bays at 45 / 135 / 225 / 315, south Crown ladder from 38 to 60 m (PROPOSED) |

## 3. Shared level-design checklist

Derived from the eight map rules in docs/bible/03-arenas.md. Each item has a pass criterion that can be checked in the editor or in a playtest. An arena ships when every row passes or carries a written exception approved by design.

### Rule 1. It is a demolition site, mid-demolition

| Check | Pass | Fail |
|---|---|---|
| 1a. Cut lines visible | At least one in-progress cut per tier: torch scoring, bright metal, slag, with the cutting crew's tools within 5 m | Any tier reads as a finished or abandoned space |
| 1b. Staged charges | Every demolition charge is on a structural element a cutting crew would drop, with tape and a notice | A charge sits on a floor, a prop, or anything a crew would not cut |
| 1c. Idle machines | Every crane, hauler, lift, and press is mid-task: a load on the hook, a train in the trench, a car between landings | A machine parked empty with no job |
| 1d. Paper trail | Condemnation notices, chalk, and at least one evidence object per arena, placed per 01-world-and-lore | None, or paper placed where no crew would post it |
| 1e. Seasonal state | The arena's 04-progression consequence is authored as a toggle (cut district, slowed Breathing, doubled Haulers, crane livery) | Consequence requires a new map |

### Rule 2. Every hazard has a job

| Check | Pass | Fail |
|---|---|---|
| 2a. Manifest reason column | Every row of prop-manifest.md has a prep-crew reason a demolition foreman would accept | Any "because it is fun" placement |
| 2b. Volatile logic | Propane and fuel sit within 10 m of a torch, a generator, or a vehicle that uses them; manifolds feed a visible line | Volatiles in the open with nothing to serve |
| 2c. Rack and stack logic | Racked objects are in racks a crew uses; stacks are staged for lift-out, with lift points up | Racks facing a wall; stacks nobody could pick |

### Rule 3. Three tiers of height, minimum

| Check | Pass | Fail |
|---|---|---|
| 3a. Three tiers | Low, mid, high, each with at least one zone a crew can hold for 20 s | Any tier is a transit space only |
| 3b. Climber reach | Every tier-to-tier gap is at or under 30 m of vertical gain from a surface a Climber can stand or Perch on, with a Dog B surface within 50 m and a landing that a Perch, Hang, or soft surface arrests | A gap over 30 m, or a landing at 12 m/s or more onto hard surface |
| 3c. Breaker honest route | Every tier is reachable by a Breaker without a line: ladder (2.0 m/s, Breaker 1.6 m/s), stair, ramp under 30 degrees, lift, or kinematic ride. Route time under 30 s between adjacent tiers. A hanging Breaker can also Reel up a Hang line at 1.0 m/s | A Breaker needs a Slingshot (9.4 m gain) or a Haul to climb |
| 3d. Other classes | Scrapper (15.8 m) and Rigger (13.8 m) can make every gap with one Slingshot plus a mantle, or a Pendulum from an anchor within class line length | A gap only a Climber or Doubler can make and only a Breaker can walk |
| 3e. Route check table | layout.md lists each adjacent-tier route per class with the number that proves it | Routes asserted without numbers |

### Rule 4. One big moving part on a readable cycle

| Check | Pass | Fail |
|---|---|---|
| 4a. One primary cycle | One kinematic or force volume dominates, with a fixed period that fits the round (at least one full cycle in a 150 s Scrap round) | Period longer than 150 s, or two parts competing for primary |
| 4b. One second of sound | Audio tell at least 1.0 s before the moving part can hurt anyone; 3 s klaxon for a Hauler, 2 s for a Gantry drop per 05 section 11 | Any phase change without audio lead |
| 4c. Deterministic phase | The cycle restarts on the Marshal's "live" so the event and the cycle line up the same way every round; phase at live is stated per mode | Phase drifts between rounds |
| 4d. Masters play with it | signature-plays.md has at least two plays that use the cycle as a tool, not an obstacle | The part is only ever something to avoid |
| 4e. Nothing moves it | Kinematics are infinite mass; the only exception is a rated attachment released by a Breaker's 1,000 N (05 section 6.7) | A line, blast, or prop alters the kinematic's path |
| 4f. It cuts lines | Every kinematic severs any line it passes through (snap, both Dogs release; lines attached to the kinematic itself survive). The events file names which set lines and bridges each kinematic sweeps | A line that survives a kinematic passing through it |

### Rule 5. One mid-match event that makes the map more dangerous, never less

| Check | Pass | Fail |
|---|---|---|
| 5a. Timing | Fires at 1:15 remaining in Scrap, 4:00 remaining in Claim, 2:00 remaining in each Haul half, 3:00 remaining in Free Scrap, with a 5 s siren and a visible sag or sparks | Any other time, or a shorter siren |
| 5b. One-way | After the event every affected zone has less floor, more edge, or faster hazards. No zone becomes safer | Debris creates new cover that is net safer than before |
| 5c. New edges are Well edges | Any floor removed by the event exposes a kill volume or a fall over 7.3 m onto hard surface | Removed floor exposes a soft landing |
| 5d. Early trigger | If a rated load can fire the event early, the scheduled event still fires on a fallback, so the map still gets more dangerous at the scheduled time | Early trigger removes the scheduled escalation |
| 5e. Spawns survive | Both spawns and both alternates remain valid after the event (no Well edge within 25 m, not under falling debris) | A spawn is under the event's debris path |

### Rule 6. A Well

| Check | Pass | Fail |
|---|---|---|
| 6a. A kill volume | At least one kill volume reachable from every tier by Overboard or a fall, with a distinct "down the Well" Scrap Card | Any tier from which no ring-out is possible |
| 6b. Hooks | Hook rotors spin up 0.5 s before the volume (05 section 7.5). Exception: the Anchor, where Hooks do not go down the Well and the 0.5 s tell is the Stump's hum dropping an octave (v0.2.2) | Silent ring-outs |
| 6c. Ring-out share | Playtest elimination mix shows ring-outs under 35 percent (05 section 12). Hangtown and the Anchor are the two arenas most likely to fail this; both have authored soft catches (nets, the Crown Net, the bowl floor) to hold the share | Over 35 percent without a plan to add floor |
| 6d. Marker reset | The Claim marker resets to its authored position if it enters the Well; the Haul spool cannot enter a Well on any supported route | A marker or spool can be lost for the round |

### Rule 7. Rated loads are marked

| Check | Pass | Fail |
|---|---|---|
| 7a. Tape | Every rated load (1,000 N break-free: a Breaker alone, a Doubler at overspin, or any crew whose lines sum to 1,000 N on one axis) carries yellow-and-black structural tape visible from 15 m on its doggable faces | Untaped rated load, or tape on a non-rated object |
| 7b. One consequence each | Each rated load has one authored consequence, a 1.0 s structural groan before it, and a debris path that is drawn in layout.md | Consequence undefined or silent |
| 7c. Readable tiers | Heavy objects (300 to 3,000 kg) are visibly massive; Medium and Light read as liftable. Placement state (racked, stacked, bolted, chained) is readable from the model, not only from the manifest | A 2,000 kg object that looks like a 200 kg object |
| 7d. Reset | Rated-load consequences reset between rounds only | A consequence resets mid-round |
| 7e. Heavy staging (05 section 6.1a) | Every Heavy object meant to move is staged on rollers (0.02), skids or greased runners (0.05), or hangs, and the manifest's placement note says which. Every other Heavy is marked fixed and breaks. Claim marker on polished runners, Haul spool on rollers, water tanks fixed, Ballast containers on the racks' dead rollers, and any stack meant to Avalanche on roller pallets (0.02 to start, 0.005 rolling: the bottom crate is dragged out and the rest fall) | A Heavy on a bare floor that a play or event expects to slide, or an Avalanche stack on skids |

### Rule 8. Lines are the light

| Check | Pass | Fail |
|---|---|---|
| 8a. Brightest thing in frame | In every zone, under every lighting state including post-event and darkened sectors, a line at full tension is the brightest emissive in frame. Lighting docs list a per-zone check | Any practical, sky, or VFX brighter than a singing line |
| 8b. No haze above the floor | Fog and mist stay below the lowest playable floor or at the horizon; no volumetrics inside playable volumes thicker than light haze | Mist in a fighting zone |
| 8c. No strobing | No flashing practicals; event and kinematic warning lamps go steady, not strobing | Any strobe |
| 8d. Team tints only | Line glow, Catch Plate bands, Dog trails, and HUD crew markers use only Amber #E69F00, Sky #56B4E9, Rose #CC79A7, Bone #E8E6E0 (05 section 11a), with the 1 m dark tick on enemy lines. Arena palettes avoid these hues for anything emissive | An arena practical that reads as a team tint |
| 8e. Dark sector test | Where an event or a broken tower kills the lights, line emissives are confirmed readable at full before any other tuning (03 Anchor engine note) | Dark sector tuned for mood before lines are checked |
| 8f. Set lines | A set line's dim emissive brightens within 5 m of a player and is visible against the arena's darkest floor | A set line that disappears against the floor |

### Cross-cutting checks from 05-core-mechanics

| Check | Pass | Fail |
|---|---|---|
| X1. Spawns (9.3) | Two zones, opposite ends of the high or mid tier, 10 m across, never within 25 m of a Well edge measured along a walkable route; alternate spawn used when an enemy is within 15 m. Arenas where every tier overhangs the Well (the Anchor) spawn on the Rim's crane decks | A spawn with a walkable route under 25 m to a Well edge |
| X2. Panic window (4.3) | Every lethal configuration gives 1.0 to 2.0 s from Dog seat to the point of no return, except where the attacker had to approach within 5 m. The Kick never fires with a Dog in a player's Catch Plate (3.2 interlock), so Overboard is a T push (600 N on 130 kg is a 4.6 m/s² skid, about 2 s over 9 m) and every Overboard play is written as a skid, not a launch | A configuration under 1.0 s with no approach tell, or an Overboard play that assumes a Kick against a player |
| X3. Dynamic budget | prop-manifest.md states the simultaneous Heavy and Medium cap and shows the worst authored case (the event plus the sudden-death shrink) under it | Cap undefined or exceeded by an authored moment |
| X4. Sudden death (9.1) | Scrap tie-break is an arena-specific shrink event, defined in events-and-moving-parts.md, that ends in a decision within 90 s | Undefined, or can end in a second tie |
| X5. Tells (11) | Every lethal system in the arena appears in the events file's tells table with audio, visual, and lead time at or above the minimums | A lethal system with no tell row |
| X6. Elimination mix (12) | Playtest target 40 percent object impact, 25 ring-out, 20 player-into-world, 10 blast, 5 crush | Any single category over 45 percent |
| X7. Force volumes (6.6) | Every force volume is authored as acceleration on a 100 kg reference body and the events file states the per-class value (a_ref × 100 / m: Climber ×1.11, Scrapper ×0.91, Rigger and Doubler ×0.77, Breaker ×0.45). Volumes act on players and loose objects only; racked, stacked, bolted, rated, and hanging objects are unaffected | A volume that moves a racked cylinder or a rated load, or a table that uses one number for every class |
| X8. Named rigs (10) | Gallows plays hoist by Reel (1.0 m/s within Hang, up to the class Hang load; only a Breaker's Haul lifts a Climber outright). Guillotine plays drop the load by recalling or cutting the owner's set line. Overboard plays are T pushes. Caboose plays note that the kinematic severs any other line it passes through | A play that Hauls a player upward, Hauls a Guillotine load off its rod, or Kicks against a Catch Plate |

## 4. Bible tensions and their rulings

These are the places where 03-arenas.md, 05-core-mechanics.md, and the arena geometry did not agree when the arena documents were first written. Most are now ruled in 05 v0.2.2 (docs/bible/CHANGELOG.md) and the arena documents carry the ruling without a PROPOSED tag. The three still open are marked.

| # | Tension | Status | Resolution in the arena documents |
|---|---|---|---|
| 1 | 03 Ballast's 300 m Spine inside a 410 m hull at 70 degrees is not a playable footprint | **Ruled, canon** | The forward 72 m is the arena; the rest is sealed vista. ballast/layout.md is the geometry authority |
| 2 | 03 says "drag the spool up the Spine," but no pull tension lifts 2,000 kg (19.6 kN) | **Ruled, canon** | The Spine ratchet rail in ballast/layout.md: a counterbalanced sled on a one-way rail, 200 N pawl drag, 4,000 kg moving mass, any class moves it, it holds between pulls, three lines add |
| 3 | Containers do not slide on any lean the hull reaches at steel friction | **Ruled (05 6.1a)** | Containers ride the magnetic racks' dead rollers (friction 0.02). The 250 N lean-window break-free stays PROPOSED |
| 4 | 03 Anchor Crown "three hundred meters up" vs map rule 3 | **Ruled, canon** | The Rim deck is 240 m up the Stump; the Crown is 60 m above the Rim. the-anchor/layout.md is the geometry authority (Rim 0, Scaffold 6 to 38, Ring 42, Crown 60) |
| 5 | 03 Anchor Pulse "every three minutes" never fires in a 2:30 Scrap round; its direction was both "toward the Rim" and "into the Well" | **Ruled, canon** | Every 75 s from 0:55. Profile: 0.4 s outward shove, then 0.6 s inhale toward the Stump |
| 6 | 05 9.3 spawns unsatisfiable on the Anchor and horizontally in Hangtown | **Ruled (05 9.3)** | Spawn distance is measured along walkable routes; the Anchor spawns on the Rim crane decks |
| 7 | 05 3.5 (Hang lines ignored by objects) vs 03 Yard (a Hauler cuts a trench bridge line) | **Ruled (05 3.5)** | Kinematics sever any line they pass through; lines attached to the kinematic itself survive |
| 8 | 05 section 10 Gallows "enemy hoists" and section 8 "player is hoisted" impossible under v0.2 tensions except Breaker on Climber | **Ruled (05 3.2 Reel)** | Within Hang, holding Haul shortens the rod at 1.0 m/s bearing the class Hang load. The Gallows hoists by Reel; hanging players climb by Reel. The Kiss row's "2 kN" is fixed to 200 N |
| 9 | Guillotine step 2: a 600 N Haul cannot pull a load off a 12 kN rod | **Ruled (05 3.6, 10)** | Recall or cut your own set line; gravity drops the load. Owners can recall set lines early |
| 10 | Kick-based Overboard reaches V in 0.3 s, leaving no panic window | **Ruled (05 3.2 interlock)** | The Kick never fires with a Dog in a player's Catch Plate. Overboard is a T push, a skid of a few m/s², which restores the window |
| 11 | 05 6.6 force volumes "everything inside" would lift racked cylinders and rated loads every Breath cycle | **Ruled (05 6.6)** | Volumes act on players and loose objects only, scaled as a_ref × 100 / m |
| 12 | 05 9.2 gave no Free Scrap event time | **Ruled (05 9.2)** | 3:00 remaining |
| 13 | Rated loads needed a Breaker only | **Ruled (05 6.2, 02)** | A Breaker alone, a Doubler at overspin, or any crew whose lines sum to 1,000 N on one axis |
| 14 | 03 rule 6 "sound of Hooks" vs no Hooks at the Anchor | **Ruled** | The Anchor's ring-out tell is the Stump's hum dropping an octave |
| 15 | Rule 3 at the Yard: Stacks (8 m) to Rails (40 m) was 32 m, over a Climber's 30 m single-Jack reach | **Ruled** | Rails lowered to 36 m (Gantry cab 40 m); 03 now says thirty-six meters. One Slingshot with 2 m to spare |
| 16 | 03 Hangtown "Pendulum a house into a crowd" vs lines that cannot swing Heavy hanging objects | **Ruled (05 6.3, v0.2.1)** | All swings come from gravity: a cut slack cable, Cable Day, or pumped Sway. The orange slack segment sits 3.5 m above the rated hanger eye (at least 1.5 m required), stated numerically in hangtown/layout.md and prop-manifest.md |
| 17 | crane-counterweight is filed under docs/objects/heavy by the object list but is rated per 03 and 05 6.1 | **Open** | The Anchor treats it as rated. The object doc should carry both or move |
| 18 | 05 6.1 defines Rated as over 3,000 kg or structural while 03 calls Floor hull sections rated | **Open, resolved locally** | The Yard splits 2,800 kg Heavy plates (stacks, Gantry) from 4,500 kg rated uncut sections (Floor) |
| 19 | Team Amber #E69F00 is within 16 degrees of hue of slack-cable orange and rated yellow tape | **Open, playtest** | Hangtown separates them by matte tape vs emissive line and the 1 m enemy tick |
