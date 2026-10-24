# THE YARD: Prop Manifest

_Arena: The Yard (Collar Four Breaking Yard). File purpose: every placed object by zone, its placement state, orientation, and the reason a prep crew put it there; tier totals; dynamic budget; arena-unique objects._
_Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Placement states follow 05 section 6.2. Anything not in those files is marked **PROPOSED**._

Coordinates are (X, Y) at Floor datum unless a Z is given. Zone names match layout.md. Object paths point at the per-object docs; this file does not restate their dimensions. Masses quoted in the budget section are the tier midpoints used for planning and defer to the object docs.

## Inventory by zone

### 1. The Trenches

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Trenches | docs/objects/kinematics/rail-hauler.md | 3 | One per trench, enters from X -80 (South, North) or X +80 (Center) on schedule | kinematic | Along X | The Yard's freight. Does not stop for the match. |
| Trenches | docs/objects/structures/hull-plate-section.md (Hauler load) | 3 | Chained to each Hauler deck, top at +3 m | chained to kinematic (not line-movable) | Flat, long axis along X | Inbound raw plate (outer trenches), outbound pressed stock (center). |
| Trenches | docs/objects/structures/plank-bridge.md | 6 | X -30 and +10 on each trench, Floor lip to Floor lip | loose (laid, 0 N); fracture 600 N·s (05 section 6.5) | Across Y | Shift crew's shortcut. The first Hauler load through each trench sweeps it away, and a Breaker who jumps onto one (220 kg at 2.7 m/s or more) breaks it: walk. |
| Trenches | Grate footbridge (arena-unique, below) | 12 | X -50, -10, +30, +70 on each trench, deck at 3.5 m with stairs both ends | bolted (static) | Across Y | Sanctioned crossings, high enough to clear a loaded Hauler. |
| Trenches | docs/objects/environment/chalk-and-graffiti.md | 3 | Trench end walls: the Guild's timetable chalked at each entry | static | Facing the trench | Workers read the schedule here. So do players. |

### 2. South Dock

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| South Dock | docs/objects/environment/floodlight-mast.md | 2 | (-52, -40) and (+52, -40), platform at 24 m, ladder on the north face | bolted (static) | Lamps aimed north and inboard | Night-shift lighting. |
| South Dock | docs/objects/volatiles/fuel-drum.md | 6 | Fuel point at (+60, -42), two rows of three on a drip tray | racked (200 N) | Upright | Diesel for the cutting crew's generators at the far lot. |
| South Dock | docs/objects/props-light/scrap-debris-kit.md | 3 | Heaps at (-30, -40), (0, -40), (+30, -40), 4 m across, 1.2 m high | loose (pooled) | Random | Offcuts swept from the press approach. x2.0 Stall surface. |
| South Dock | docs/objects/props-light/chain-coil.md | 4 | Against the mast bases | loose | Coiled | Hauler tie-down chain. |
| South Dock | docs/objects/props-light/cable-reel-small.md | 3 | Along the south wall at X -20, 0, +20 | loose | On rim | Floodlight feed. |
| South Dock | docs/objects/props-medium/hand-truck.md | 1 | (+40, -42) | loose | Tilted back | Moving the drums. |
| South Dock | docs/objects/environment/sponsor-banners.md | 4 | On the south perimeter wall, 2 m above the Floor | static | Facing north | LayCast dressing (Kessl Harness Leasing, Nurri Stall-Gel). |

### 3. Haul Lane

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Haul Lane | Haul truck (arena-unique, below) | 2 | (-75, -12) bed facing east; (+75, -12) bed facing west | static | Along X | Haul mode delivery points. Dressed as Guild and HTR billet trucks. |
| Haul Lane | docs/objects/heavy/haul-spool.md | 1 | (0, -12), Haul mode only, in a roller cradle (friction 0.02, 05 section 6.1a): about 400 N to start, so any class alone moves it and three lines add as vectors | on rollers (0 N break-free) | Axis along Y so it rolls along X | The disputed Weave spool, staged the way riggers stage a load. |
| Haul Lane | docs/objects/structures/billet-stack.md (3 x docs/objects/heavy/billet-crate.md) | 6 stacks, 18 crates | (-60, -20), (-50, -20), (+50, -20), (+60, -20), (-28, -6), (+30, -6); three crates high on a roller pallet (friction 0.02 to start, 0.005 rolling, 05 section 6.1a). An Avalanche drags the bottom crate out on its rollers and the two above fall. A crate that lands off the pallet is fixed, breaks. | stacked (500 N) on rollers | Square to X | Pressed billets staged for the trucks. Slow topple, 1.5 s lean before fall (**PROPOSED** settle time). |
| Haul Lane | docs/objects/props-medium/welding-cart.md | 2 | (-6, -14) and (+6, -14), either side of the spool start | loose | Handles toward the spool | The shift was tack-welding the spool's cradle when the match was called. |
| Haul Lane | docs/objects/volatiles/cylinder-rack.md with 4 x docs/objects/volatiles/propane-cylinder.md | 1 rack | (-24, -20), against SC2's north face | racked (200 N) | Valves facing the mesa | Torch gas for the welding carts. |
| Haul Lane | docs/objects/structures/hull-plate-section.md, rated uncut variant (R3, R4) | 2 | R3 at X -76 to -64, Y -9 to -3; R4 at X +62 to +74, Y -25 to -19, on greased skids (friction 0.05) angled toward the nearest trench lip | rated (1,000 N: a Breaker, a Doubler at overspin, or crew lines summing to 1,000 N on one axis), yellow-and-black tape | Flat, long axis along X | Next shift's cut, waiting for a Hauler slot. |
| Haul Lane | docs/objects/props-medium/steel-crate.md | 4 | Two at each truck, beside the ramps | loose | Square | Truck crews' kit. |
| Haul Lane | docs/objects/props-light/tool-bag.md | 3 | On the welding carts and at Truck A | loose | Any | Shift kit. |
| Haul Lane | docs/objects/props-light/clamp-crate.md | 2 | At the foot of SC1 and SC4 | loose | Square | Scaffold clamps for the mesa rung ladders. |
| Haul Lane | docs/objects/environment/structural-tape.md | 2 runs | Around R3 and R4 | static | | Rule 7 marking. |

### 4. Marker Flat

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Marker Flat | docs/objects/heavy/claim-marker.md | 1 | (0, +12), Claim mode only, pallet on polished runners (friction 0.05, 05 section 6.1a). Planning mass 900 kg per 6.1a: a Rigger's 600 N moves it at about 0.18 m/s², a Breaker's 1,000 N at about 0.62 m/s², a 400 N line does not start it. | on runners (0 N break-free) | Pallet square to X, runners along X | The claim. |
| Marker Flat | docs/objects/props-light/scrap-debris-kit.md | 5 | Heaps at (-10, +6), (+8, +6), (+10, +18), (-8, +20), (-40, +22) | loose (pooled) | Random | Press offcuts dumped clear of the lanes. Cover and soft landing at the marker. |
| Marker Flat | docs/objects/props-medium/cart-trolley.md | 3 | (-20, +8), (+6, +24), (+40, +12) | loose | Handles toward the marker | Moving billets to the marker pallet. |
| Marker Flat | docs/objects/props-medium/brick-pallet.md | 2 | (-18, +22) and (+28, +24) | loose | Square | Fire-brick for the press lining. |
| Marker Flat | docs/objects/volatiles/cylinder-rack.md with 4 x docs/objects/volatiles/propane-cylinder.md | 1 rack | (+34, +10), 6 m from the pit's north lip | racked (200 N) | Valves facing the pit | Torch gas for the press-side cutters. |
| Marker Flat | docs/objects/volatiles/gas-manifold.md | 1 | (+20, +22), against NC3's north face | fixed world geometry (no line moves it) | Outlets facing west along Y +22 | Feeds the cutting station. Vents a 6 m jet west when Jacked over 600 N: Doubler or Breaker, or a Doubler at overspin. |
| Marker Flat | docs/objects/structures/hull-plate-section.md, rated uncut variant (R1, R2) | 2 | R1 at X -76 to -64, Y +13 to +19; R2 at X +30 to +42, Y +19 to +25, on greased skids (friction 0.05) angled toward the nearest trench lip | rated (1,000 N: a Breaker, a Doubler at overspin, or crew lines summing to 1,000 N on one axis), taped | Flat, along X | Staged for the Gantry. R2 is the next pick after the Pick Stack. |
| Marker Flat | docs/objects/props-medium/scaffold-board-stack.md | 1 | (-30, +8) | stacked (500 N) | Along X | Boards for the plank bridges. |
| Marker Flat | docs/objects/props-light/plank.md | 6 | Scattered within 10 m of the marker | loose | Random | Fell off the board stack. Scrapper ammunition. |
| Marker Flat | docs/objects/props-light/bucket.md | 4 | Around the cart trolleys | loose | Upright | Flux and water. |
| Marker Flat | docs/objects/environment/condemnation-notice.md | 1 | On NC2's west face at 1.6 m | static | Facing west | HTR's license challenge, stapled over Guild chalk. |
| Marker Flat | docs/objects/environment/force-volumes.md (Shift Change debris shove) | 1 | Spawned at the Gantry drop point (-15, +9) on landing: 3 m radius, radial, 0.5 s, a_ref 12 m/s² on a 100 kg body (05 section 6.6: a body feels a_ref × 100 / m; players and loose objects only, racked cylinders and stacked crates unaffected) | volume | Radial | The plate's impact throws the heap and anyone standing too close. A stagger, not a Stall. |

### 5. North Dock

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| North Dock | docs/objects/rated/structural-column.md | 2 | (-62, +35) and (-62, +41), 0.6 m box columns, 10.5 m tall, under the control shack | rated (1,000 N: a Breaker, a Doubler at overspin, or crew lines summing to 1,000 N on one axis), taped | Vertical | Hold the shack up. Pull either and the shack comes down. |
| North Dock | docs/objects/environment/floodlight-mast.md | 2 | (-52, +40) and (+52, +40), platform at 24 m, ladder on the south face | bolted (static) | Lamps aimed south and inboard | Lighting. |
| North Dock | docs/objects/props-light/scrap-debris-kit.md | 2 | (-20, +40) and (+20, +40) | loose (pooled) | Random | Offcuts. |
| North Dock | docs/objects/props-light/cable-reel-small.md | 2 | At the shack columns | loose | On rim | Shack power and comms. |
| North Dock | docs/objects/props-medium/hand-truck.md | 1 | (0, +42) | loose | Upright | Dock kit. |
| North Dock | docs/objects/props-light/folding-chair.md | 2 | Under the shack, between the columns | loose | Facing south | The shift's lookout seat. |
| North Dock | docs/objects/environment/sponsor-banners.md | 4 | North perimeter wall | static | Facing south | Dressing. |

### 6. The Press

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Press | docs/objects/kinematics/shear-press.md | 1 | Pit at (45, 0), frame over the west half, ram -10 to -6.5 m, 20 s cycle | kinematic | Frame legs at Y ±7 | The Yard's reason to exist. |
| Press | docs/objects/rated/press-counterweight.md | 1 | Hanging from the frame's north side at (45, +9), underside at 6 m, on an 8 m cable | rated (1,000 N: a Breaker, a Doubler at overspin, or crew lines summing to 1,000 N on one axis), taped | Cable vertical | Ram return weight. Pulled by a Breaker, it swings north over Marker Flat then lands at (45, +14) as a 2 m by 2 m by 3 m block (authored consequence). |
| Press | docs/objects/environment/well-kill-volume.md | 1 | X +38 to +52, Y -7 to +7, Z below -6 | volume | | The Well. |
| Press | Pit lip rail (arena-unique, below) | 2 | North and south lips, 1.1 m pipe rail | bolted (static) | Along X | Keeps walking workers out. Does nothing for dragged ones. |
| Press | docs/objects/props-light/floodlight-stand.md | 2 | East lip corners | loose | Aimed into the pit | Press-side work lights. Knock one in and the pit goes dark on that side. |
| Press | docs/objects/environment/structural-tape.md | 1 run | Around the counterweight and the frame legs | static | | Rule 7. |

### 7 to 12. The Stacks

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| West Mesa | docs/objects/structures/hull-plate-section.md | 6 | Two end to end, three high, tack-welded | static (mesa): fixed, breaks | Along X | Cut stock awaiting the Gantry. Also Spawn A. |
| West Mesa | docs/objects/props-light/folding-chair.md | 4 | Around a cable reel at (-68, -36, 8) | loose | Any | Smoke corner. |
| West Mesa | docs/objects/props-light/tool-bag.md | 1 | At the rung head | loose | | Shift kit. |
| West Mesa | docs/objects/props-light/cable-reel-small.md | 1 | (-68, -36, 8) | loose | Flat (table) | Smoke corner. |
| Pick Stack | docs/objects/structures/hull-plate-section.md | 3 | One wide, three high, stacked on friction; the top section is live and falls off the stack when pulled, it does not slide | top: stacked (500 N); lower two: static | Along X | The Gantry's next lift. |
| East Pick Stack | docs/objects/structures/hull-plate-section.md | 2 | Two high, stacked on friction; the top section is live after Shift Change and falls when pulled, it does not slide | top: stacked (500 N); bottom: static | Along X | The Gantry's reversed pick. |
| Canyons | docs/objects/structures/hull-plate-section.md | 19 | NC1 (3), NC2 (3), NC3 (3), SC1 (3), SC2 (3), SC3 (1), SC4 (3), with stepped ends | static (mesa): fixed, breaks | Along X | Cut stock. The mid tier. |
| Canyons | docs/objects/props-light/scrap-debris-kit.md | 2 | On the 5.4 m ledges of NC1 and SC4 | loose (pooled) | | Offcuts left by the cutters. Soft landing for a mesa drop. |
| Canyons | docs/objects/props-medium/steel-crate.md | 3 | On NC2's top, 1 m from the south edge | loose | Square | Cutter kit left up top. The Guillotine load for a Rigger on NC2: Hang it from the leg head, release, and recall the set line when someone walks under (05 section 10). |
| Canyons | docs/objects/props-light/paint-can.md | 6 | NC3 top and SC2 top | loose | Upright | Marking paint. |
| Canyons | docs/objects/environment/chalk-and-graffiti.md | 7 | Each mesa's Floor-level face: Guild cut marks, section numbers, one "NOT TODAY" over an HTR tag | static | | The site is being read by its workers. |
| East Mesa | docs/objects/structures/hull-plate-section.md | 6 | Two end to end, three high | static (mesa): fixed, breaks | Along X | Cut stock. Spawn B. |
| East Mesa | docs/objects/props-light/laundry-bundle.md | 3 | On a line between two rebar posts at (+66, +36, 8) | loose | Hanging | The night shift's washing. |
| East Mesa | docs/objects/props-light/paint-can.md | 3 | At the rung head | loose | Upright | Marking paint. |
| Shack Perch | Control shack (docs/objects/environment/office-gallery.md variant, **PROPOSED**) | 1 | Floor 10.5 m, roof 14 m, 8 m by 10 m, bolted to the west leg and the two columns | bolted (drops only via columns) | Windows south | Marshals' gallery and the Yard office. |
| Shack Perch | docs/objects/environment/evidence-objects.md (maintenance log) | 1 | On the office desk | loose (Light) | | This season's evidence. |
| Shack Perch | Roof floodlights (part of the shack, docs/objects/environment/floodlight-mast.md head units) | 4 | Roof edge, aimed south-east | bolted | | Light the west third of the Rails and Marker Flat. Go dark with the shack. |
| Shack Perch | docs/objects/volatiles/demolition-charge.md | 2 | On the north portal leg at (+20, +30), 3 m above the Floor, on the cage ladder's frame | bolted (800 N) | | HTR survey crew has tagged the leg for decommissioning. Dogs seat in a charge without triggering it (05 section 6.4); it needs an impact over 6 m/s or a blast. Detonation sheds the leg's ladder cage from 0 to 12 m (Medium fragments); the leg stands. |

### 13 to 15. The Rails

| Zone | Object (doc path) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Rail Walks | docs/objects/structures/catwalk-section.md | 2 x 160 m | Outboard walkways at Y ±31.5, 36 m | bolted (static, non-fracturing here, **PROPOSED**) | Along X | Crane inspection access. |
| Rail Walks | docs/objects/structures/spine-ladder.md (cage ladder variant) | 8 | One per portal leg, Floor to 36 m, landings at 24 and 36 (and 10.5 on the shack leg); 2.0 m/s climb, Breaker 1.6 | bolted (static) | Vertical | The honest route. |
| Rail Walks | docs/objects/props-light/tool-bag.md | 2 | At the leg heads (-20, +30, 36) and (+20, -30, 36) | loose | | Inspection kit. |
| Rail Walks | docs/objects/props-light/clamp-crate.md | 1 | (+60, +31.5, 36) | loose | | Rail clamps. |
| Mast Platforms | docs/objects/props-light/floodlight-stand.md | 4 | One per platform, spare lamp | loose | Any | Spare lighting. |
| The Bridge | docs/objects/kinematics/gantry-crane.md | 1 | Bridge beam top at 36 m, cab at 40 m | kinematic | Across Y | The Yard's crane. |
| The Bridge | docs/objects/structures/hull-plate-section.md (Gantry load) | 1 | Hanging on the magnet from pick to drop | rated attachment (05 section 6.7): kinematic on the magnet, doggable; a Breaker Haul over 1,000 N strips it and it falls where it is | Flat | The pick. Drops into the press, or wherever Shift Change finds it. |
| The Bridge | docs/objects/environment/broadcast-tower.md | 1 | LayCast camera mast on the cab roof, 4 m | bolted | | The feed. |
| Arena-wide | docs/objects/environment/floodlight-mast.md head units | 12 | Three per mast platform | bolted | Inboard | Lighting. |
| Arena-wide | docs/objects/environment/structural-tape.md | 1 run | Every rated load and the trench lips at the grate stairs | static | | Rule 7. |

## Totals by tier

| Tier | Objects placed | Of which line-movable | Notes |
|---|---|---|---|
| Light | 12 heaps (pooled, about 25 pieces each when live), 4 chain coils, 6 cable reels, 10 folding chairs, 7 tool bags, 9 paint cans, 3 clamp crates, 4 buckets, 6 planks, 3 laundry bundles, 6 floodlight stands, 1 evidence object, 2 demolition charges (volatile) | all | About 380 Light bodies if every heap were live at once. They are not (budget below). |
| Medium | 18 billet-crate... see Heavy. 2 welding carts, 3 cart trolleys, 2 hand trucks, 7 steel crates, 2 brick pallets, 1 scaffold board stack, 6 plank bridges, 8 propane cylinders (volatile), 6 fuel drums (volatile) | all | 37 Medium bodies. |
| Heavy | 18 billet crates (stacked on roller pallets), 1 Claim marker (runners), 1 Haul spool (rollers), 1 Pick Stack top and 1 East Pick Stack top (stacked, fall only), 1 Gantry load (hanging) | all, per 05 section 6.1a staging | 23 Heavy bodies. Every Heavy not on rollers, skids, runners, or hanging is fixed, breaks: the mesas, the dropped Shift Change plate, and any crate that comes to rest off its roller pallet. |
| Rated | 4 uncut hull sections (R1 to R4), 1 press counterweight, 2 structural columns | Breaker line or Doubler overspin only | 7 rated loads, 7 authored consequences. |
| Volatile | 8 propane cylinders, 6 fuel drums, 2 demolition charges, 1 gas manifold | | 17, all with a job. |
| Static dressing | 37 mesa hull sections, 3 Haulers, 1 Gantry, 1 press, 4 masts, 8 cage ladders, 12 grate footbridges, 2 trucks, 2 catwalks, banners, notices, chalk | not simulated | |

## Dynamic budget check

Simultaneously simulating bodies in the worst authored case (every billet stack toppled, both Pick Stack tops live, the Gantry load falling, the marker and spool moving):

| Tier | Worst case live | Recommended per-arena cap | How the cap holds |
|---|---|---|---|
| Heavy | 24 (18 crates + marker + spool + 2 pick tops + Gantry load + 1 counterweight in its authored swing) | **14** | A billet crate that comes to rest off its roller pallet is fixed scenery (05 section 6.1a) and stops simulating; a toppled stack costs 3 Heavy for about 6 s. In practice no more than three stacks are down at once. |
| Medium | 37 placed + Medium fragments from plank bridges and the ladder cage | **50** | Fragments live 10 s. Fuel drums and cylinders that have detonated are removed. |
| Light | 380 potential | **150**, pooled | Scrap heaps simulate only in the sector (one of six, 2 per island half) nearest a player or a live line; distant heaps are static meshes. Heap pieces that leave their heap live 10 s. |
| Rated | 7 | all 7 | Simulated only during their authored consequence, then static. |
| Kinematic | 5 (3 Haulers, Gantry, press ram) | 5 | Scripted. |

If the Heavy cap is hit, the oldest resting Heavy goes kinematic first. Never freeze the marker, the spool, or the Gantry load.

## Arena-unique objects

Objects not in the shared object list. Full mini-specs.

### Grate footbridge

| | |
|---|---|
| Dimensions | 8 m span by 2.4 m wide, deck at 3.5 m, open steel grate, 1.1 m handrails both sides; a 5 m stair flight at 35 degrees at each end, landing 1.5 m by 2.4 m |
| Tier / mass | Static world geometry (not simulated). Treated as structure, no tape. |
| Appearance | Galvanized grate, Guild ochre handrail, trench number stenciled on the landing, amber lamp on each stair head |
| Physics | Solid to players and objects. Clearance under the deck 3.5 m; a loaded Hauler (top at +3 m) passes beneath with 0.5 m to spare. The stairs are where Clotheslines go (1.2 m across the landing). A player flipped off the deck falls 7.5 m to the trench floor: 12.1 m/s, a Stall. |
| Count | 12 (4 per trench at X = -50, -10, +30, +70) |

### Haul truck

| | |
|---|---|
| Dimensions | Flatbed 12 m by 3 m, bed at 1.2 m, cab at the outboard end, steel ramp 4 m long at 17 degrees on the inboard end, delivery volume 4 m by 3 m on the bed |
| Tier / mass | Static. Not doggable on the cab (Marshal-flagged); the bed and ramp are doggable static surfaces. |
| Appearance | Truck A in Guild ochre and black with a Ledger number on the door; Truck B in HTR orange and grey with a fleet number. Both with LayCast delivery-sensor rigs on the bed corners. |
| Physics | The spool scores when its center is inside the bed volume for 1.0 s (**PROPOSED**). The ramp is a 17 degree incline the 2,000 kg spool rolls back down unless a line holds it: two Hang lines from the spool to the bed's headboard are the standard finish (19.6 kN of load against 12 kN per Rigger line; Hang loads add per 05 section 6.2a). One line slips. |
| Count | 2 |

### Pit lip rail

| | |
|---|---|
| Dimensions | 14 m long, 1.1 m high, 60 mm pipe on posts every 2 m, north and south lips of the pit |
| Tier / mass | Static. Doggable. |
| Appearance | Yellow-and-black banded pipe (it marks the Well, not a rated load; the band pattern is diagonal to distinguish it from rule 7 tape, **PROPOSED**) |
| Physics | Solid to players walking or running (a sprinting player is staggered against it at 5 m/s, flipped over it at 8 m/s). A dragged player who has gone airborne clears it. Objects clear it or pile against it. A Hang line to the rail is the legal "Hang to the floor" counter to Overboard at the pit. |
| Count | 2 |

### Lockout curtain

Used only in the Scrap sudden-death event (events-and-moving-parts.md).

| | |
|---|---|
| Dimensions | Two curtains, each 90 m wide (Y -45 to +45) by 36 m tall (0 to 36 m), hung from trolleys on the gantry rails at X = -80 and X = +80 in the parked state |
| Tier / mass | Kinematic (05 section 6.7). Infinite mass, scripted path. |
| Appearance | Chain-mail blast curtain over heavy canvas, Guild ochre top band, "LOCKOUT: CLEAR THE YARD" stenciled at 20 m, lit from behind by its own trolley lamps so it reads as a wall of dull light |
| Physics | Advances inboard at 1.0 m/s. Anything it touches is pushed at 1.0 m/s (no stagger, under 4 m/s). A player pinned between the curtain and anything solid is crushed per 05 section 7.1 (force over 8 kN for 0.5 s). Objects are pushed along and into the trenches. Lines through the curtain are severed (05 section 3.5: a kinematic severs any line it passes through); lines attached to the curtain itself are not. |
| Count | 2 |

### Hauler load coupling note

The hull plate on a Hauler deck is chained to the deck (05 section 6.2: 800 N). 05 section 6.7 names the Gantry's load as a rated attachment a Breaker can strip; this arena does not extend that to the Hauler loads. They are tagged as part of the kinematic for the match, so that a train cannot be stripped mid-crossing: a bare Hauler sweeps no plank bridges and no Hang lines, and the map would get safer (**PROPOSED**). Flag for the rail-hauler.md author.
