# KINETIC — Arenas

## Rules every arena follows

These come straight out of the lore and should be treated as hard constraints for level design.

1. **It is a demolition site, mid-demolition.** Someone is taking this structure apart. The arena shows where they stopped for the match: cut lines, staged charges, idle cranes, condemnation notices.
2. **Every hazard has a job.** No barrel without a reason a cutting crew put it there. No crane without a load it was moving.
3. **Three tiers of height, minimum.** Low, mid, high. Each tier should be reachable by a Climber in one Slingshot and by a Breaker in one honest route.
4. **One big moving part on a readable cycle.** Telegraphed by sound a full second before it matters. Veterans play around it; masters play with it.
5. **One mid-match event** that makes the map _more_ dangerous, never less. Demolition only goes one way.
6. **A Well.** Somewhere to be thrown that ends the conversation. The sound of Hooks below it.
7. **Rated loads are marked.** Yellow-and-black structural tape on anything only a Breaker can move. Players must be able to read what will and won't come down.
8. **Lines are the light.** Arenas are lit so that a singing line is always the brightest thing in frame. No haze, no strobing, nothing that competes with the Visibility Clause.

Target format for all four: crews of three, 3v3, with Scrap (elimination) and Claim (control) as the primary modes. 3v3v3 is a post-launch variant; the Rose team tint in 05 section 11a is reserved for it. Haul (physics payload) is noted where a map supports a route.

---

## BALLAST

_Freight climber "Tenacity," wedged nose-down in a gorge in the Cordillera foothills below Collar Five._

**Story.** The 410-meter freight climber _Tenacity_ was two hundred kilometers up when the Strand parted. It came down nose-first into the Rucu Gorge, stayed mostly intact, and has been too dangerous to cut for sixty years: it is held in place by its own crushed cargo and nothing else. HTR holds the demolition claim. The Guild contests it on safety grounds. The matches decide who gets to kill it, and the hull rocks the whole time, because HTR's cutters have already started.

**Layout.** You fight _inside the hull_, which stands at roughly seventy degrees. Three decks, read top to bottom as the climber lies:

- **The Crown** (high). The climber's drive head, open to sky where HTR has taken the plating. Bright, windy, exposed. HTR's cutting gantry sits here, idle for the match. The honest route up for Breakers is the maintenance ladders in the spine; everyone else Slingshots.
- **The Bays** (mid). Two cargo bays either side of the central spine shaft. Container stacks sit on magnetic racks that lost power in L.0 and have been held by friction and habit ever since. Catwalks cross between the bays at two heights. This is where most fights happen.
- **The Hold** (low). The nose. Crushed cargo, standing seep-water, the darkest tier. The torn nose cone opens onto the gorge: the Well. Water tanks here are intact and full.
- **The Spine.** The forward 72 m of the climber's central shaft (the rest is crushed or flooded and sealed off), connecting all three decks, lined with ladders and the climber's drive dogs: forty-ton pistons, rated loads, still seated in their housings.

**The moving part: the Breathing.** The hull rocks six degrees either way on a ninety-second cycle, groaning. On the lean, loose containers slide, hanging chains swing, water in the Hold shifts and shoves whatever's standing in it. A Hang line set at one end of the cycle goes slack or snaps taut at the other. A Pendulum swung on the lean goes further than physics said it should. New players fight the Breathing. Good players wait for it.

**Mid-match event: the Shear.** At the midpoint, HTR's offscreen cutters finish a cut. Sirens, then an entire bay wall sheds into the gorge, taking its catwalks with it. The Bays open to sky on one side and to the Well on the other. Every fight in the Bays is now a fight next to an edge.

**What breaks.** Container stacks (the Avalanche, on a lean, is the map's signature). Catwalk sections. Drive housing panels in the Spine. The Hold's water tanks: rupture one and the flood rush pushes players and props toward the nose.

**Rated loads.** The drive dogs in the Spine. A Breaker who Hauls one down the shaft has sent forty tons through three decks of whatever was in the way. Also the magnetic racks themselves.

**Signature plays.**
- Avalanche on the lean: pull the bottom container just as the hull tips toward your target.
- The Draw: Hang a line across the Spine shaft and wait for the Breathing to swing a Climber into it.
- Hold flood: rupture a tank as a Scrapper and ride the rush out while the enemy gets carried into the nose.

**The Well.** The torn nose cone. On the lean toward it, the Hold floor becomes a ramp.

**Modes.** Scrap, Claim (marker in the Bays). Haul runs vertically up the Spine on HTR's counterbalanced ratchet rail: the spool rides a sled whose hidden counterweight matches it exactly, one-way pawls every 0.5 m hold any height gained, and the only resistance is 200 N of pawl drag plus 4,000 kg of moving mass. Any class can move it; a crew pulling together moves it fast. Lines never lift the spool; the rail does.

**Engine notes.** Hull is a single kinematic root with an animated tilt; all contents are children simulating in its frame. Containers as geometry collections with per-stack strain thresholds and an anchor field that releases on the lean. Water as a simple force volume with a tilt-driven direction. The Shear is a pre-fractured section released by a field on a timer, with its catwalks parented to it.

---

## THE YARD

_The Collar Four Breaking Yard, in the Basin. Where the Lay's big pieces come to die._

**Story.** Hull sections from across the Lay arrive here on rail haulers, get cut, pressed, and shipped as Weave billets. The Guild runs the Yard under Accord license. HTR wants the license. The Office makes them fight for it a shift at a time, and the Yard keeps running during the match because the Yard does not stop for anything.

**Layout.** Open, horizontal, cut into three heights:

- **The Rails** (high). The gantry crane's rails, thirty-six meters up, running the length of the Yard. The crane cab, the control shack, the floodlight masts. The only high ground, and the most exposed to Climbers, who own it unless someone makes them leave.
- **The Stacks** (mid). Cut hull sections stacked three high, forming mesas and canyons. Billet containers. Sightlines break every ten meters. The main fighting tier.
- **The Floor** (low). Rail trenches cut the floor into islands connected by grates and plank bridges. Scrap heaps between them, full of loose, light, throwable junk. The Shear Press pit sits center-east: the Well, with a hydraulic ram at the bottom that cycles whether or not anyone is in it.

**The moving parts.**
- **The Haulers.** Automated rail trucks carrying hull sections run through the trenches on a fixed schedule: klaxon, then lights, then train. You do not move a Hauler. A Hauler moves you. Dog an enemy to one and let the timetable do the rest. Climbers ride them for free repositioning. Breakers can stand on one and arrive like a delivery.
- **The Gantry.** A sixty-meter crane with an electromagnet traverses the Yard on a slower cycle: picks a hull section from the Stacks, carries it the length of the map, drops it in the press. Players can ride the load, Dog the load, swing from the load, or stand under the load exactly once.

**Mid-match event: Shift Change.** For sixty seconds the Hauler schedule doubles and the Gantry drops its load early, wherever it happens to be. Every trench becomes a lane you don't cross.

**What breaks.** Billet stacks: slow to topple, devastating when they do. The control shack (and its floodlights with it, darkening a sector of the Rails). Scrap heaps, which come apart into a Scrapper's favorite ammunition. Plank bridges over the trenches.

**Rated loads.** Hull sections on the Floor. The press counterweight. A Breaker can tip a stack or pull a hull section into a trench ahead of a Hauler, which derails nothing (Haulers are unstoppable) but does make a wall.

**Signature plays.**
- The Caboose, in its purest form.
- The Magnet Drop: time the Gantry's release over a Claim marker.
- Trench bridges: Hang a line across a trench as a footbridge, then watch a Hauler sever it (kinematics cut lines, 05 section 3.5) and drop whoever was mid-crossing.

**The Well.** The Shear Press pit. The ram's cycle is audible everywhere.

**Modes.** All three. Haul runs the spool along the Floor between trenches; the Haulers are the payload's problem too.

**Engine notes.** Haulers are kinematic bodies on splines, not simulated vehicles: determinism matters more than suspension. The Gantry load is a simulated body on a constraint with a scripted release. Billet stacks as geometry collections with high damage thresholds and a settle time so they fall slowly enough to read. Scrap heaps as pooled small-prop clusters, budgeted per sector.

---

## HANGTOWN

_Vilca's Hang: a neighborhood strung on cable across the ravine between the two halves of Collar Three._

**Story.** Collar Three split on landing, leaving a four-hundred-meter ravine between its halves. During the Ribbon Wars, people with nowhere else to go strung cables across it and built on them. Three thousand people live in houses that hang. HTR holds a demolition claim on both halves. The Guild and the Hangers contest it. On match days the Hang is evacuated and the Office puts up the lights, and the residents watch their homes get used as weapons on LayCast like everyone else.

**Layout.** Three tiers by cable height:

- **The Upper Hang** (high). The main load cables, anchored into the Collar halves. Walkways run along them. The Lift's headframe. The only solid ground on the map is the two Collar rims at either end, and they are far apart.
- **The Mid Hang** (mid). The houses. The market platform, a hanging ring at the center. Swing bridges. Dense, dark, full of corners. A hanging house is a wrecking ball with a kitchen in it.
- **The Low Hang** (low). Drip nets, garden terraces, the lowest shanties, laundry. Below them, mist, and the ravine floor a long way down: the Well.

**The moving parts.**
- **The Sway.** The whole district moves with the wind in slow, overlapping arcs. Houses drift toward and away from each other on a cycle you can learn. A Hang line between two houses is a different length every three seconds.
- **The Lift.** A counterweighted rope elevator running Low to Upper. Its counterweight, a car-sized block of concrete, travels the opposite direction through the Mid Hang. The Lift is the honest route up. The counterweight is a rated load that is already moving.

**Special affordance: slack cables.** Marked with orange tape and visible fraying. Any Cutter can cut them. Cut the slack cable on a house and it drops a tier, or into the Well. Cut a swing bridge and whoever is on it swings. Everywhere else in the game the Cutter is a safety tool. In Hangtown it is a demolition tool, and it should feel like a transgression the first time. (Flagged as an open design question; see _Progression → Open questions_.)

**Mid-match event: Cable Day.** A main load cable gives. Half the market platform drops to forty degrees and hangs by its remaining line. Everything on it slides toward the low edge, and the houses that hung from the failed cable are now swinging free in the Mid Hang.

**What breaks.** Houses, which shatter when they hit each other. Walkways. Water tanks on roofs. Market stalls. Two hanging houses swung into each other is the map's thesis statement.

**Rated loads.** The Lift counterweight. The anchor pins in the Collar rims (a Breaker who pulls one drops a whole cable's worth of walkway). Every house hangs from a rated 1,000 N hanger: only a Breaker's line, a Doubler at overspin, or a cut slack cable drops one.

**Signature plays.**
- Eviction: drop a house on someone, by cutting its slack cable or Hauling it down.
- The Swing: cut a house's slack cable so it drops a tier and sweeps the walkway below, or pump the Sway with a timed Hang line. Lines alone cannot swing a house dangerously; gravity does the work (05 section 6.1a).
- Cut-Loose: wait until an enemy crosses a swing bridge, then cut the near end.
- Overboard, everywhere, all the time. There is almost no floor.

**The Well.** The ravine. The map's defining feature is that the Well is directly below nearly every square meter of it.

**Modes.** Scrap, Claim (marker on the market ring). Haul is not recommended; the spool would spend the match in the ravine.

**Narrative weight.** This is the map where the stakes are visible. Chalk drawings on the walls. Condemnation notices stapled to doors. A Hanger win in the seasonal Claim keeps Hangtown as it is. An HTR win means next season's Hangtown has a cut district: a quarter of the map gone, the cables hanging empty. Players should feel it.

**Engine notes.** A constraint tree of cables with houses as geometry collections held in an anchored state until their slack cable is cut or a rated 1,000 N pull releases the hanger; houses fracture only on impact (2,000 N·s small, 3,500 N·s large). Sway is a low-amplitude kinematic animation of the anchor points, not a simulated wind on every body. Strict budget on simultaneously dynamic houses (suggest four), with distant houses swapping to kinematic sway. Cable Day is a scripted constraint break on the market ring with pre-authored swing paths for the freed houses.

---

## THE ANCHOR

_The Meridian Anchor: the artificial island where the Strand began, and the three hundred meters of it still standing._

**Story.** A kilometer-wide bowl of concrete and Weave: the socket that held the Strand to the planet. At its center, the Stump: three hundred meters of Strand still rooted, still under residual tension, humming at a frequency riggers claim they can feel in their teeth. Beneath it, the Well: the shaft where the cable's root goes down into the seabed. This is the championship arena. The Accord says whoever holds the Anchor Claim holds the right to re-raise the Strand from it. Nobody has held it two seasons running.

**Layout.** Concentric:

- **The Rim** (low, outer). The socket's edge. Rim cranes, the Office's gallery, LayCast's broadcast towers. Wide, open, the only place on the map where you can see everything.
- **The Scaffold** (mid). Contractor scaffolding wrapped around the Stump in rings, five levels deep, connected by ladders and planks. The main fighting volume. The scaffolding is modular, and it looks it.
- **The Crown** (high). The Stump's sheared top. A platform barely twenty meters across, sixty meters above the Rim deck (which itself sits 240 m up the Stump's root), in the wind. The highest ground in the game. Standing on it means something to every faction, for different reasons.
- **The Well** (center, down). The shaft under the Stump. Final.

**The moving parts.**
- **The Updraft.** Gusts rise from the Well on a forty-second cycle, strongest near the Stump. Light harnesses get lifted. Hang loads swing. Dogs fired across the gust drift. Climbers call it the Breath. Everyone else calls it the Well's bad breath. Timed right, it carries a Slingshot to the Crown.
- **The Maintenance Ring.** A rotating ring gantry around the Stump at Scaffold-top height, one revolution every two minutes. Ride it or be swept by it.
- **The Pulse.** Every 75 seconds, first at 0:55 into the round, the Stump's residual tension releases: a radial shove outward from the Stump for 0.4 s that slides loose objects and unbraced players toward the Rim, then a 0.6 s inhale back toward the Stump that takes anyone left on the inner edge of the Scaffold into the Well. Marshals count it down. Everyone braces. Someone always doesn't.

**Mid-match event: Scaffold Collapse.** One quadrant of the Scaffold fails, telegraphed by sirens and a visible sag. Five levels of planks and whoever was on them go down the outside of the Stump toward the Rim. The quadrant is now a cliff face with the odd surviving ledge.

**What breaks.** Scaffolding, by section. Rim cranes. Broadcast towers, which kill the lights in a sector and make lines the only illumination there, which is the point.

**Rated loads.** Crane counterweights on the Rim. The maintenance clamps on the Stump itself, which a Breaker can pull free to drop a whole ring of Scaffold.

**Signature plays.**
- The Ascension: Slingshot off the Stump into the Updraft at the top of its cycle to reach the Crown. The Ascendancy's signature, and the single most-replayed clip type on LayCast.
- Overboard on the Pulse: hold position on the inner Scaffold edge, line an enemy to yourself, and let the Pulse decide.
- The Anchor Drop into the Well, with a crane counterweight.

**The Well.** The literal one. The sound here is different: Hooks don't go down it. The ring-out tell is the Stump's hum dropping an octave as you fall past the Scaffold base. (Lore: the Office retrieves from the Anchor's Well by other means and does not discuss them. The fatality rate for this map is published separately.)

**Modes.** Claim, with the marker on the Crown. Scrap. This is the finals map; it should be the hardest to read and the most rewarding to master.

**Engine notes.** Scaffold as modular geometry collections with a structural anchor field keyed to the clamps. Updraft and Pulse are radial force fields on timers with a falloff from the Stump. The Ring is kinematic. Tower collapses toggle light sectors; confirm line emissives read at full in the darkened sector before anything else is tuned.

---

## Backlog

Three more concepts, one line each, for after the first four ship.

- **The Smelt.** The Weave re-spinning foundry at Collar Six. Crucibles on overhead rails, slag channels on the floor, conveyors that don't stop. A map where the floor itself is the hazard and everything that moves is on fire.
- **The Drowned Collar.** Collar One, half-sunk off the Anchor. Tides. Rust. Structures that are solid at low water and ring-outs at high.
- **The Climb.** The interior of a standing Collar segment with no floor at all. Ascendancy home ground. Every fight is vertical, every ring-out is the same direction.
