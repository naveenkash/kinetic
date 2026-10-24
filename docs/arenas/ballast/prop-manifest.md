# BALLAST: Prop Manifest

_Arena: Ballast. File purpose: complete inventory of every placed object by zone, placement state per 05 section 6.2, tier totals, dynamic budget, and inline specs for arena-unique objects. Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Inventions are marked **PROPOSED**. Zone names and coordinates match layout.md (hull-local frame, origin at the Spine centreline on the Hold floor)._

Placement states follow 05 section 6.2: loose 0 N, racked 200 N, stacked 500 N, bolted or chained 800 N, rated 1,000 N. "Bolted" objects are hull fittings a line cannot move below 800 N; only a Doubler or a Breaker shifts them. "Rated" needs a Breaker or a Doubler on overspin.

## Inventory

### Tier 1: The Hold

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| The Wash | docs/objects/heavy/water-tank.md | 4 | Port and starboard walls, Y -9 to -5, two per side, 2.4 m tall, full; fixed, breaks (05 section 6.1a): it ruptures, it does not move | fixed | Long axis along Y | Climber's potable water. Cut Plan 7 never needed the Hold drained, so nobody did |
| The Wash | docs/objects/props-medium/steel-crate.md | 6 | Belly half, Y +1, three per side, half-submerged | loose | Random yaw | Spilled from crushed containers on landing. Prep crew left them as "site condition" |
| The Wash | docs/objects/props-light/scrap-debris-kit.md | 6 clusters | Two x2.0 heaps at (-10, -3) and (+10, -3), 6 m across; four thin scatters elsewhere | loose | n/a | Crushed cargo. The heaps are where HTR dumps cut offal through the slab windows |
| The Wash | docs/objects/props-light/chain-coil.md | 4 | Floating in the Wash near the heaps | loose | n/a | Lifting chain lost down the gutter |
| The Wash | docs/objects/props-light/plank.md | 8 | Floating, drift toward the Sump on each lean | loose | n/a | Cribbing offcuts from the Bay decks |
| The Wash | docs/objects/props-light/bucket.md | 6 | Floating | loose | n/a | Bailing buckets. Nobody bails |
| The Wash | docs/objects/environment/evidence-objects.md | 1 | The _Tenacity_ flight recorder, half-sunk at (+6, +2, 0), orange housing | bolted | n/a | Season evidence object for the Question (01 lore: "a black box in the Hold of Ballast") |
| The Wash | docs/objects/environment/force-volumes.md | 1 | Wash shove volume, whole Wash and Sump, direction +Y, 0 to 4.8 m/s² on a 100 kg reference body tied to hull pitch; players and loose props only | n/a | +Y | The Breathing moving the water |
| The Wash | docs/objects/environment/force-volumes.md | 4 | Tank rush volumes, one per tank, 10 m cone toward +Y, 12 m/s² on a 100 kg reference body for 3 s on rupture (a 1,200 N force on any body); players and loose props only | n/a | +Y | Authored flood |
| The Sump | docs/objects/props-light/bucket.md | 2 | Floating | loose | n/a | As above |
| The Sump | docs/objects/environment/well-kill-volume.md | 1 | Top face Z -10, full gorge plan 120 m by 200 m, rises to Z 60 outside the hull skin | n/a | n/a | The Well |
| The Sump | docs/objects/environment/structural-tape.md | 1 run | Yellow-black on the rent lip plating | n/a | n/a | Marshal-required edge marking |
| Hold Ledge | docs/objects/props-light/floodlight-stand.md | 2 | X -10 and +10 on the ledge, aimed across the Wash at the rent | loose | Facing +Y | Hold lighting. Powered from the drums' generator |
| Hold Ledge | docs/objects/volatiles/fuel-drum.md | 3 | X +14, chained to the ledge rail in a row | chained | Upright | Generator fuel for the Hold floods |
| Hold Ledge | docs/objects/props-medium/welding-cart.md | 1 | X -14 | loose | Facing the stair | Left by the crew that cut the Belly Stair footings |
| Hold Ledge | docs/objects/volatiles/propane-cylinder.md | 2 | Racked on the welding cart's cylinder cradle | racked | Upright | Cutting gas for the stair footings. The only volatiles in the Hold; the tanks are 12 m away |
| Hold Ledge | docs/objects/props-medium/hand-truck.md | 1 | Beside the drums | loose | Leaning on the wall | Drum handling |
| Hold Ledge | docs/objects/props-light/tool-bag.md | 2 | Stair heads | loose | n/a | Crew kit |
| Hold Ledge | docs/objects/structures/scaffold-section.md | 6 | The Belly Stair tower from (+17, -11, 6) to (+17, +9, 16), two flights | bolted | Along Y | Prep-crew access from the Hold to the Bays |
| Hold Ledge | docs/objects/environment/condemnation-notice.md | 1 | On the dorsal wall at X 0 | n/a | n/a | "Hold excluded from Cut Plan 7. No entry below Z 6 during cutting" |
| Hold Ledge | docs/objects/environment/chalk-and-graffiti.md | 3 | Guild tally marks and "SHE TOLD YOU" on the dorsal wall | n/a | n/a | Guild crews leaving their opinion of the harmonics reports |
| Spine Foot | docs/objects/structures/spine-ladder.md | 1 run | Dorsal face, continuous Z 0 to 44, 22 sections of 2 m | bolted | Vertical in hull frame | Original climber maintenance ladder |
| Spine Foot | docs/objects/props-light/floodlight-stand.md | 1 | On the plug, aimed up the shaft | loose | Up | Shaft lighting |
| Spine Foot | docs/objects/props-light/scrap-debris-kit.md | 2 clusters | The plug surface | loose | n/a | Crushed cargo |
| Spine Foot | docs/objects/props-light/cable-reel-small.md | 1 | Against the port face | loose | n/a | Floodlight cabling |
| Spine Foot | Spine ratchet rail (arena-unique) | 1 | Dorsal face beside the ladder, Z 0 to 44 | bolted | Along the shaft | HTR's lift for cut sections. The Haul mode's rail |

### Tier 2: The Bays

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Port Lower Bay | docs/objects/rated/magnetic-rack.md | 2 | Dorsal rack centred (-11, -11, Slab), Belly rack centred (-11, +3, Slab); each 13 m by 6 m, 16 m tall frame | rated | Long axis along X, leaning 20 degrees toward +Y | Original cargo racks. Magnets dead since L.0. A rated pull (a Breaker alone, a Doubler at overspin, or a crew whose lines sum to 1,000 N on one axis: two Riggers plus anyone, or a Rigger and a Doubler) tears the frame's bolts and tips it toward the Belly (scripted); its six containers leave their rollers onto the Belly strip and into the gutter. The frame stays as a leaning wreck |
| Port Lower Bay | docs/objects/heavy/shipping-container.md | 12 | Two stacks of three per rack, 12.2 by 2.4 by 2.6 m, long axis along X | stacked (500 N; 250 N at hull pitch 74 degrees or more, **PROPOSED**), on the rack's dead rollers (friction 0.02, 05 section 6.1a), held by the rack end-stop | Leaning 20 degrees with the rack | Cargo HTR has not yet lifted. The fourth tier is gone; that is why the frame tops are empty |
| Port Lower Bay | docs/objects/structures/container-stack.md | 4 | Governs each stack's strain flag and release order | n/a | n/a | Avalanche authoring |
| Port Lower Bay | docs/objects/props-medium/steel-crate.md | 4 | On the deck at the rack ends, Y -5 and Y +6 | loose | Random yaw | Contents spilled when HTR opened doors to check manifests |
| Port Lower Bay | docs/objects/props-medium/cart-trolley.md | 1 | Cross-catwalk, port end (-6, -2, 16) | loose | Along X | Moves cut offal to the slab windows |
| Port Lower Bay | docs/objects/props-medium/scaffold-board-stack.md | 1 | Dorsal corner by the stair | stacked | Along X | Deck cribbing stock |
| Port Lower Bay | docs/objects/props-light/clamp-crate.md | 2 | Beside the board stack | loose | n/a | Cribbing clamps |
| Port Lower Bay | docs/objects/props-light/chain-coil.md | 3 | Hung from the rack frame tops at Z 32, 6 m long, swinging | chained at the top end | Hanging | Lifting slings left on the frames |
| Port Lower Bay | docs/objects/props-light/floodlight-stand.md | 2 | (-8, -8, 16) and (-8, +8, 16) | loose | Facing the Spine | Bay work light |
| Port Lower Bay | docs/objects/volatiles/demolition-charge.md | 4 | On the port wall ribs at Z 20, Y -12, -4, +4, +12 | bolted | Facing inboard | The Shear cut. HTR's charges on the rib roots. Impact over 6 m/s to a charge blows a 6 m by 6 m wall window (events file) |
| Port Lower Bay | docs/objects/environment/structural-tape.md | 6 runs | Rack uprights, gutter edge, slab window edges | n/a | n/a | Rated-load and edge marking (map rule 7) |
| Port Lower Bay | docs/objects/environment/condemnation-notice.md | 1 | Port wall, Y 0, Z 18 | n/a | n/a | "Shear B. Stand clear of port wall after siren" |
| Port Lower Bay | docs/objects/environment/chalk-and-graffiti.md | 4 | HTR cut-line paint on the wall; Hanger stencil on a container | n/a | n/a | Cutting marks and trespass |
| Starboard Lower Bay | docs/objects/rated/magnetic-rack.md | 2 | Mirror of port | rated | As port | As port |
| Starboard Lower Bay | docs/objects/heavy/shipping-container.md | 12 | Mirror of port; on the rack's dead rollers (friction 0.02, 05 section 6.1a) | stacked | As port | As port |
| Starboard Lower Bay | docs/objects/structures/container-stack.md | 4 | Mirror | n/a | n/a | As port |
| Starboard Lower Bay | docs/objects/props-medium/steel-crate.md | 4 | Mirror | loose | Random yaw | As port |
| Starboard Lower Bay | docs/objects/props-medium/cart-trolley.md | 1 | Cross-catwalk starboard end | loose | Along X | As port |
| Starboard Lower Bay | docs/objects/props-medium/scaffold-board-stack.md | 1 | Dorsal corner | stacked | Along X | As port |
| Starboard Lower Bay | docs/objects/props-light/clamp-crate.md | 2 | Beside the board stack | loose | n/a | As port |
| Starboard Lower Bay | docs/objects/props-light/chain-coil.md | 3 | Frame tops, swinging | chained | Hanging | As port |
| Starboard Lower Bay | docs/objects/props-light/floodlight-stand.md | 2 | Mirror | loose | Facing the Spine | As port |
| Starboard Lower Bay | docs/objects/props-light/paint-can.md | 4 | At the Belly Stair head | loose | Upright | Cut-line marking paint |
| Starboard Lower Bay | docs/objects/environment/structural-tape.md | 6 runs | As port | n/a | n/a | As port |
| Mouth Plate | docs/objects/structures/catwalk-section.md | 1 | The Plate itself, 8 m by 8 m at Z 16, plus the 4 m cross-catwalk through it (2 further sections, one per bay) | bolted | Along X | Prep-crew crossing of the shaft. Fractures at 2,500 N·s |
| Mouth Plate | docs/objects/heavy/claim-marker.md | 1 | Authored (0, 0, 16); on polished runners (friction 0.05, 05 section 6.1a): a Rigger or heavier moves it slowly | loose | Billet along X | The Claim |
| Mouth Plate | docs/objects/heavy/haul-spool.md | 1 | On the rail sled at (0, -4, 16), Haul mode only; on rollers (friction 0.02, 05 section 6.1a): 400 N starts it, three lines add | racked (rail sled, 200 N pawl drag) | Axis along X | The payload |
| Mouth Plate | Spine drive chain (arena-unique) | 2 | Hung from the Spine head kerb at (-3, -3, 44) and (+3, +3, 44), 18 m long, lower ends at Z 26, swinging on the lean | chained at the top | Hanging | Original drive-head hoist chains |
| Mouth Plate | docs/objects/environment/structural-tape.md | 4 runs | Plate edges over the shaft | n/a | n/a | Edge marking |
| Port Upper Gallery | docs/objects/structures/catwalk-section.md | 7 | 2 m wide, Z 26, X -18 to -4 along the dorsal wall, plus the stair landing | bolted (to the port wall; parented to the Shear panel) | Along X | Prep-crew access to the rack tops and collar |
| Port Upper Gallery | docs/objects/props-light/floodlight-stand.md | 1 | Stair head | loose | Facing the stacks | Gallery light |
| Port Upper Gallery | docs/objects/props-medium/steel-crate.md | 2 | Stacked at the stair head | stacked | Along X | Cutter consumables |
| Port Upper Gallery | docs/objects/props-light/paint-can.md | 2 | On the crates | loose | Upright | Marking paint |
| Port Upper Gallery | docs/objects/props-light/tool-bag.md | 1 | Collar hatch | loose | n/a | Crew kit |
| Port Upper Gallery | docs/objects/environment/sponsor-banners.md | 1 | Dorsal wall, 6 m by 2 m, "Kessl Harness Leasing" | n/a | Facing +Y | LayCast dressing |
| Starboard Upper Gallery | docs/objects/structures/catwalk-section.md | 7 | Mirror; survives the Shear | bolted | Along X | As port |
| Starboard Upper Gallery | docs/objects/props-light/floodlight-stand.md | 1 | Stair head | loose | Facing the stacks | As port |
| Starboard Upper Gallery | docs/objects/props-medium/steel-crate.md | 2 | Stacked at the stair head | stacked | Along X | As port |
| Starboard Upper Gallery | docs/objects/props-light/cable-reel-small.md | 1 | Collar hatch | loose | n/a | Floodlight cabling |
| Starboard Upper Gallery | docs/objects/props-light/tool-bag.md | 1 | Spawn B landing | loose | n/a | Crew kit |
| Starboard Upper Gallery | docs/objects/environment/sponsor-banners.md | 1 | Dorsal wall, "Stall-Gel by Nurri" | n/a | Facing +Y | LayCast dressing |
| Rack Tops | docs/objects/props-light/chain-coil.md | 4 | Hooked to the frame tops at Z 32, two per bay, 8 m long, ends at Z 24 | chained | Hanging | Lifting slings for the removed fourth tier |
| Rack Tops | docs/objects/props-light/clamp-crate.md | 2 | On the port dorsal stack top | loose | n/a | Rack dog shims |
| Rack Tops | docs/objects/props-medium/welding-cart.md | 1 | Port dorsal stack top, chained to the frame | chained | Along X | The torch that started the lift-out cuts |
| Rack Tops | docs/objects/environment/structural-tape.md | 8 runs | Every frame upright at Z 24 to 32 | n/a | n/a | Rated marking |
| The Spine | docs/objects/rated/drive-dog-piston.md | 6 | Z 10 port, 18 starboard, 26 port, 34 starboard, 42 port, 50 starboard (**PROPOSED** 8 m spacing); 3 m diameter, 2 m protrusion | rated | Axis along X, in wall housings | The climber's drive dogs, still seated. 1,000 N on the piston's axis releases one down the shaft: a Breaker alone, a Doubler at overspin, or a crew's lines summing to it (two Riggers plus anyone; a Rigger and a Doubler) |
| The Spine | docs/objects/structures/catwalk-section.md | 4 | The Spine collar ring at Z 26, 1.5 m wide | bolted | Ring | Prep-crew landing between ladder runs |
| The Spine | docs/objects/structures/spine-ladder.md | 1 run | Starboard face, Z 16 to 44 (second run, lets two climb at once) | bolted | Vertical | Original |
| The Spine | docs/objects/props-light/floodlight-stand.md | 2 | On the collar, aimed up and down | loose | Up and down | Shaft lighting |
| The Spine | docs/objects/environment/structural-tape.md | 6 runs | Every piston face | n/a | n/a | Rated marking |
| The Spine | docs/objects/environment/chalk-and-graffiti.md | 2 | Ascendancy altitude marks at Z 40 on the dorsal face | n/a | n/a | Climbers paint their highest perch |

### Tier 3: The Crown

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Crown Deck | docs/objects/volatiles/cylinder-rack.md | 1 | Port dorsal corner (-14, -9, 44) | bolted | Along Y | Cutting gas store for the gantry torch |
| Crown Deck | docs/objects/volatiles/propane-cylinder.md | 6 | In the rack | racked | Upright | Oxy-propane for the cutters |
| Crown Deck | docs/objects/volatiles/gas-manifold.md | 1 | Beside the rack (-11, -9, 44), feeds the gantry torch line | fixed (Heavy; no line moves it, per the v0.2 rulings) | Jet faces +X | Torch supply. A Jack over 600 N (Rigger, Doubler, Breaker) vents a 6 m jet across the deck; the manifold itself never moves |
| Crown Deck | docs/objects/volatiles/fuel-drum.md | 2 | Starboard dorsal (+13, -9, 44), chained | chained | Upright | Gantry generator fuel |
| Crown Deck | docs/objects/props-medium/welding-cart.md | 2 | Belly side, (+6, +2) and (-6, +2) | loose | Along X | Cutter carts between shifts |
| Crown Deck | docs/objects/props-medium/steel-crate.md | 4 | Two at each cradle | loose | Along X | Rigging stock for the gantry hook |
| Crown Deck | docs/objects/props-medium/cart-trolley.md | 1 | (-4, +4, 44) | loose | Along Y | Moves gas and crates |
| Crown Deck | docs/objects/props-medium/hand-truck.md | 1 | By the drums | loose | Leaning | Drum handling |
| Crown Deck | docs/objects/props-medium/scaffold-board-stack.md | 1 | (-10, +9, 44) | stacked | Along X | Net repair boards |
| Crown Deck | docs/objects/structures/scaffold-section.md | 2 | Leg ladders' guard frames on the port Belly column | bolted | Vertical | Column access |
| Crown Deck | docs/objects/props-light/tool-bag.md | 3 | Cradles and the Spine head kerb | loose | n/a | Crew kit |
| Crown Deck | docs/objects/props-light/paint-can.md | 6 | Along the dorsal plating cut line | loose | Upright | Marking the next cut |
| Crown Deck | docs/objects/props-light/clamp-crate.md | 2 | At the cradles | loose | n/a | Hook rigging clamps |
| Crown Deck | docs/objects/rated/structural-column.md | 4 | Gantry legs at (±16, -13) and (±16, +5), Z 44 to 52 | rated | Vertical | Gantry support. A rated pull on a column (Breaker, Doubler overspin, or lines summing to 1,000 N) drops the 36 m beam across the deck (authored consequence: beam falls onto the deck along Y -6, crush lane 4 m wide, 1.0 s groan first) |
| Crown Deck | Crown Net (arena-unique) | 1 | Outside the Lip, Z 38, 36 m by 8 m | bolted | Along X | Debris net for cut material |
| Crown Deck | docs/objects/environment/floodlight-mast.md | 2 | Port and starboard Belly corners, 10 m tall, off (daylight) | bolted | Vertical | Night-shift lighting |
| Crown Deck | docs/objects/environment/broadcast-tower.md | 1 | On the hull's dorsal rim above the cut face, outside play | n/a | n/a | LayCast relay |
| Crown Deck | docs/objects/environment/office-gallery.md | 1 | Hung from the western gorge wall, 30 m beyond the Lip at Z 50, not dog-able | n/a | Facing the Crown | Marshals |
| Crown Deck | docs/objects/environment/sponsor-banners.md | 4 | One per gantry column | n/a | Facing inboard | "Meridian Mutual", "Kessl", "Nurri", "LayCast" |
| Crown Deck | docs/objects/environment/condemnation-notice.md | 1 | Spine head kerb | n/a | n/a | "Cut Plan 7. Shear B 16:40. HTR Reclamation" |
| Crown Deck | docs/objects/environment/structural-tape.md | 8 runs | Columns, Spine head kerb, Lip bulwark, post-Shear port edge (pre-placed, lit at the Shear) | n/a | n/a | Rated and edge marking |
| Crown Deck | docs/objects/environment/weave-ribbon-fragment.md | 1 | A 3 m strip of Weave clamped to the drive head at Z 48, visible from the deck | bolted | n/a | What the whole claim is for |
| Cut Gantry | Crown cutting gantry (arena-unique) | 1 | Rails at Z 52 along the port and starboard walls, beam at Y -6 | static | Beam along X | HTR's section lifter and torch carriage, idle for the match |
| Cut Gantry | docs/objects/props-light/chain-coil.md | 2 | Hanging from the hook block, 4 m | chained | Hanging | Hook slings |
| Cut Gantry | docs/objects/props-light/cable-reel-small.md | 1 | On the starboard rail | loose | n/a | Torch cable |
| Cut Gantry | docs/objects/environment/load-cable.md | 2 | Rail hand cables, 1 m high | bolted | Along Y | Fall protection, not dog-able as a line (static world surface) |

## Totals by tier

| Tier | Count | Notes |
|---|---|---|
| Light | 14 chain-coil, 8 plank, 8 bucket, 10 tool-bag/clamp-crate, 12 paint-can, 10 floodlight-stand, 3 cable-reel, 10 scrap clusters: **75 placed** | Scrap clusters spawn pooled debris up to the 120 budget |
| Medium | 22 steel-crate, 3 cart-trolley, 4 welding-cart, 3 scaffold-board-stack, 2 hand-truck, 2 drive chains (arena-unique, 120 kg): **36 placed** | Plus fragments from fractured catwalks and tanks |
| Heavy | 24 shipping-container (rack rollers), 4 water-tank (fixed, breaks), 1 claim-marker (runners), 1 haul-spool (rail sled, counts as rollers; Haul only), 1 gas-manifold (fixed): **31 placed** | Gantry beam becomes a 32nd if dropped, then fixed. Every Heavy that moves is staged per 05 section 6.1a |
| Rated | 6 drive-dog-piston, 4 magnetic-rack, 4 structural-column: **14** | Each one-use per round |
| Volatile | 8 propane-cylinder, 5 fuel-drum, 4 demolition-charge, 1 gas-manifold: **18** | Charges are the only volatiles in the Bays; two cylinders in the Hold, six on the Crown |
| Kinematic | 1 breathing-hull root | Everything else is its child |

## Dynamic budget check

Simultaneously simulating bodies, worst case in one round:

| Tier | Worst case | Recommended cap | Enforcement |
|---|---|---|---|
| Heavy | Rack pull (6 containers) + Avalanche in the other bay (3) + 2 ruptured tanks (shell pieces are Medium) + marker + spool = 11 | **12** | Containers sleep in racks (stacked state) and wake only on break-free or Shear shake. If a 13th Heavy wakes, the oldest resting container that is not touching a player freezes kinematic |
| Medium | 36 placed + up to 12 fragments (tank shells, catwalk grate pieces) = 48 | **40** | Fragments beyond the cap spawn as Light with a 10 s lifetime (05 section 6.5). Crates sleep after 3 s at rest |
| Light | 75 placed + pooled scrap + 10 s fragments | **120 pooled** | Oldest fragment despawns first |
| Hang constraints on objects | 6 crews x 2 set lines max = 12 lines + 2 Doubler second lines | 16 | Matches class limits in 05 section 3.4 |

The Shear is the spike: it wakes the port Belly rack's two bottom containers (strain flag), the port galleries (kinematic on an authored path, not simulated), and whatever players and props were on them. Budget the Shear second at 10 Heavy and 30 Medium and it fits.

## Arena-unique objects

### Spine ratchet rail
- **What.** HTR's section lift: a steel rail with a sled, bolted to the dorsal face of the Spine beside the ladder, Z 0 to 44. One-way pawls every 0.5 m. A hidden counterbalance (2,400 kg block running inside the hull's dorsal plating, never visible, never dog-able) matches the loaded sled exactly, so a line only fights the pawls and the mass.
- **Dimensions.** Rail 0.6 m wide, 44 m long. Sled 2.4 m by 1.6 m by 0.8 m with a tow eye at each end.
- **Tier and mass.** Rail: static world. Sled: Heavy when loaded with the 2,000 kg spool, 400 kg unloaded. The hidden counterbalance is 2,400 kg, matching the loaded sled, so the loaded system has no net weight and 4,000 kg of moving mass (sled, spool and counterbalance; the sled alone is not counterbalanced and a Dog on an empty sled does nothing useful).
- **Physics.** Break-free 200 N upward (pawl drag; "racked" in 6.2 terms). Acceleration = (sum of line forces along the rail minus 200 N) / 4,000 kg. Speed cap 3 m/s. Cannot descend: pawls hold. Lines sum: a solo Breaker gives 0.2 m/s², a solo Climber 0.05 m/s², Breaker plus two Riggers 0.5 m/s². Dogs seat on the tow eye or the spool; a Dog on the spool pulls at the sled's centre of mass.
- **Appearance.** Galvanised rail, HTR orange sled, yellow-black tape on the pawl teeth. The pawls click once per 0.5 m; the click is the Haul mode's progress sound.
- **Tell.** A line pulling the sled sings; the sled's pawl click is audible across the Bays.

### Spine drive chain
- **What.** Two original hoist chains hanging from the Spine head kerb into the shaft.
- **Dimensions.** 18 m long, 24 mm link, hanging from Z 44 to Z 26 at rest.
- **Tier and mass.** Medium, 120 kg each. Chained at the top (800 N to tear the top anchor free; only a Doubler or Breaker can, and the chain then falls down the shaft as a loose Medium prop).
- **Physics.** A simulated chain of 9 rigid 2 m links. Swings with the Breathing: at 6 degrees of gravity offset the lower end travels 1.9 m each way on the 90 s cycle. A Dog seats on any link. A Breaker's Haul (1,000 N, 8.3 m/s² on 120 kg) swings the lower end 8 m across the shaft and arrives at 11.5 m/s: 1,380 N·s on impact, a Stall. A Rigger's 600 N arrives at 8.9 m/s: 1,070 N·s, a flip. A Hang from a link makes it a pendulum pivot for a player.
- **Appearance.** Black oxide steel, Guild ochre tape on the lowest link (someone's set-line marker).
- **Tell.** Continuous chain clink, louder on the lean; a Hauled chain sings through the line and rattles.

### Crown Net
- **What.** HTR's debris net outside the Crown Lip.
- **Dimensions.** 36 m by 8 m, 0.2 m mesh, hung 6 m below the deck (Z 38) from the hull's dorsal frame to two steel outriggers at X ±16. Two ladders back to the deck at X ±12.
- **Tier and mass.** Static world surface, soft (x2.0 Stall multiplier per 05 section 7.3, same class as drip nets).
- **Physics.** Catches players and props. A player landing from the deck arrives at 10.8 m/s against a 24 m/s threshold: safe. A player who goes over the Lip faster than about 7 m/s horizontal clears the 8 m net before the 1.1 s fall ends and is a Well kill; slower, they land in the net. A Climber Slingshotting themselves over at the Kick's 18 m/s clears it easily. A player pushed over by an enemy's Jack gets no Kick (05 section 3.2 interlock), only the T push: a Breaker's 1,000 N needs about 7 m of skid to bring a Rigger to 7 m/s, so the net catches short pushes and not long ones. Props that land in it stay (the net is a resting surface). Not cuttable, not a line.
- **Appearance.** Orange polymer mesh, sagging 1 m at the centre, holding two cut plate offcuts and a paint can.
- **Tell.** Net creak on landing.

### Crown cutting gantry
- **What.** HTR's rail gantry across the drive head. Static for the match (idle), so it is world geometry, not a kinematic.
- **Dimensions.** Two rails 30 m long at Z 52 along the port and starboard walls (1 m walkway each with a hand cable). One 36 m box beam, 1.2 m deep, at local Y -6, carrying a torch carriage (2 m by 1.5 m) and a hook block hanging 3 m below the beam. Four structural-column legs (rated, see inventory).
- **Tier and mass.** Static. If a column is pulled, the beam becomes a simulated Heavy at 3,000 kg (top of the Heavy band) falling 8 m onto the deck, and comes to rest as world geometry again after 2 s: fixed, breaks, on bare grating per 05 section 6.1a.
- **Physics.** Perchable, dog-able, walkable. The hook block (Medium, 200 kg) swings on 3 m of chain.
- **Appearance.** HTR orange, grey machined carriage, the torch head capped, sponsor banners on the legs.
- **Tell.** Beam drop: 1.0 s structural groan and sparks from the pulled column's foot, then the fall.
