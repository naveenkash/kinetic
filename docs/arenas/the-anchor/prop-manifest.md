# THE ANCHOR: Prop Manifest

_Arena: The Anchor. File purpose: complete inventory of placed objects by zone, placement states, prep-crew reasons, tier totals, dynamic budget, and arena-unique mini-specs._
_Sources: docs/bible/03-arenas.md (map rules 1, 2, 7), docs/bible/05-core-mechanics.md (6.1 to 6.5, 7.3). Inventions are marked **PROPOSED**._

## Placement states

Per 05 section 6.2: loose (0 N break-free, anyone), racked (200 N, anyone), stacked (500 N, not Climber or Scrapper), bolted or chained (800 N, Doubler or Breaker), rated (1,000 N, Breaker or Doubler overspin only, yellow-and-black tape). "Static" means map geometry, not an object.

Heavy staging per 05 section 6.1a: a Heavy object moves only on rollers or a wheeled cradle (friction 0.02, about 0.2 N per kg), on skids or greased runners (0.05, about 0.5 N per kg), or hanging. A Heavy on a bare floor is fixed scenery that breaks and is marked "fixed, breaks" below. Every Heavy row states which.

Force volumes per 05 section 6.6 (v0.2.2): the Breath and the Pulse act on players and loose objects only, felt at the reference acceleration times 100 over the body's mass. Racked, stacked, chained, bolted, rated, and hanging objects are unaffected until something frees them. A Heavy on rollers, skids, or runners is loose, but at 900 to 1,200 kg it feels a tenth of the reference and only creeps; a Heavy on a bare floor is fixed scenery. A loose 45 kg cylinder feels 2.2 times the reference and a 25 kg clamp crate 4 times: the Pulse throws them at the outer rail at 14 to 50 m/s.

## The prep crew's story

A cladding contractor is stripping the Stump from the top down. Torch gas comes up the south face through a manifold. Plates come off, get billeted on L4, and the rim cranes lift billets down and ballast slabs up. The Office rents the site for the match at the shift horn. Everything below is where that crew left it.

## Inventory

Zones follow layout.md. Positions are (X, Y, Z) or r and azimuth.

### Z1 and Z2. Crane decks (per deck, two decks)

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Crane deck | Rim crane (arena-unique) | 1 | Slewing base centered on the deck | static | Boom toward the Stump, luffed to 51 degrees | Hoisting ballast slabs to the Crown |
| Crane deck | docs/objects/heavy/crane-counterweight.md | 1 | Hanging: 2 m under the boom tip on the hoist line at r 13, Z 54 to 55, over the Well mouth | rated (hoist brake, authored release) | Long axis tangential | The load the crane was moving when the horn went |
| Crane deck | docs/objects/volatiles/fuel-drum.md | 1 | In a cradle beside the generator, rear of deck | racked | Upright | Generator fuel |
| Crane deck | docs/objects/props-light/tool-bag.md | 2 | Against the cab wall | loose | | Crane rigger's kit |
| Crane deck | docs/objects/props-light/cable-reel-small.md | 1 | Beside the cab door | loose | On its side | Hoist signal cable |
| Crane deck | docs/objects/props-light/chain-coil.md | 1 | At the boom root | loose | | Slab slings |
| Crane deck | docs/objects/props-medium/steel-crate.md | 1 | Deck front, inner side | loose | Long axis radial | Sling and shackle store |
| Crane deck | docs/objects/props-light/floodlight-stand.md | 1 | Deck front corner | loose | Aimed at the boom tip | Night hoisting |
| Crane deck | docs/objects/environment/sponsor-banners.md | 2 | On the cab roof and rail | static | Facing the gallery | LayCast dressing |
| Crane deck | docs/objects/environment/structural-tape.md | 1 set | Boom root, counterweight slings | static | | Rated-load marking (map rule 7) |

### Z3. The Rim

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Rim N, S | docs/objects/heavy/billet-crate.md | 2 | r 46, azimuth 0 and 180, beside each crane base, on a wheeled cradle (rollers, 0.02): 240 N to start, any class | loose, on rollers | Long axis tangential | Billets waiting for the ferry; outside the Pulse's reach (r 44), so only a line moves them |
| Rim N, S | docs/objects/volatiles/fuel-drum.md | 4 | Two per crane base in a cradle, r 48 | racked | Upright | Crane generator reserve |
| Rim W | docs/objects/heavy/shipping-container.md | 2 | r 50, azimuths 250 and 290, doors facing the Stump, on the bare concrete | fixed, breaks (doors blow open and spill contents per 05 section 6.5) | Long axis tangential | Contractor stores and site office |
| Rim W | docs/objects/props-medium/rebar-bundle.md | 2 | Against the parapet, r 41, azimuth 270 | stacked | Tangential | Parapet repair |
| Rim W | docs/objects/props-light/scrap-debris-kit.md | 3 | Between the containers | loose | | Stripped cladding offcuts |
| Rim E | docs/objects/environment/office-gallery.md | 1 | r 54 to 60, azimuth 75 to 105, 12 m tall | static, un-doggable | Facing the Stump | Marshals' gallery |
| Rim E | docs/objects/props-light/folding-chair.md | 4 | Under the gallery front, r 53 | loose | | Marshals' spares |
| Rim E, W | docs/objects/environment/floodlight-mast.md | 2 | r 58, azimuths 90 and 270 | static | Aimed at the Scaffold | Night shift lighting |
| Rim NE, SW | docs/objects/environment/broadcast-tower.md | 2 | r 55, azimuths 45 and 225, 30 m tall, three legs | static, breakable | Lights aimed at the Scaffold and Crown | LayCast cameras and floods |
| Rim NE, SW | docs/objects/volatiles/demolition-charge.md | 6 | One per tower leg at 1.2 m height | bolted | Facing outward | Post-match tower strike (**PROPOSED**) |
| Rim, all | docs/objects/props-light/floodlight-stand.md | 6 | One per 60 degrees at r 44 | loose | Aimed at the gap | Gangway lighting |
| Rim, all | docs/objects/props-medium/cart-trolley.md | 2 | r 45, azimuths 120 and 300 | loose | Tangential | Moving crates to the cranes |
| Rim, all | docs/objects/props-medium/hand-truck.md | 2 | Beside the containers and the N billet crate | loose | Leaning | Drum handling |
| Rim, all | docs/objects/props-medium/steel-crate.md | 4 | Two beside each crane base | stacked (pairs) | Tangential | Shackles, slings |
| Rim, all | docs/objects/props-medium/water-barrel.md | 2 | r 44, azimuths 60 and 240 | loose | Upright | Crew water |
| Rim, all | docs/objects/props-light/tool-bag.md | 4 | On crates | loose | | |
| Rim, all | docs/objects/props-light/cable-reel-small.md | 4 | At the gangway heads | loose | | Deck power to the Scaffold |
| Rim, all | docs/objects/environment/sponsor-banners.md | 12 | On the parapet, one per 30 degrees | static | Facing the gallery cameras | LayCast dressing |
| Rim, all | docs/objects/environment/chalk-and-graffiti.md | 4 sets | Parapet and container walls | static | | Crew tallies, Ascendant prayers |
| Rim, all | docs/objects/environment/condemnation-notice.md (Office posting variant) | 4 | Gangway heads | static | | "Anchor Claim: contested. Office of the Lay." |

### Z4. Gangways and footbridge

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Gangways | docs/objects/structures/plank-bridge.md | 4 | r 40 to 24, Z 0 to 6, azimuths 0, 90, 180, 270. Fracture 600 N s (05 section 6.5) | static, breakable | Radial | Crew access to the Scaffold |
| Gangways | docs/objects/props-light/bucket.md | 4 | One hung on each handrail at mid-span | loose | | Bolt buckets |
| L5 S footbridge | docs/objects/structures/plank-bridge.md (4 m variant) | 1 | r 16 to 12, Z 38, azimuth 180 | static | Radial | Access to the Crown ladder |

### Z5. The Bowl Floor

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Bowl floor | docs/objects/props-light/scrap-debris-kit.md | 4 | Under each gangway head, r 36 | loose | | Dropped offcuts |
| Bowl floor | docs/objects/props-medium/stall-gel-pad.md | 1 | (0, 32, -8), 4 by 4 m | bolted | | Office safety pad under the north boom |
| Bowl floor | docs/objects/props-light/plank.md | 8 | Scattered r 26 to 38 | loose | | Blown off the decks |
| Bowl floor | docs/objects/props-light/chain-coil.md | 3 | Among the footing standards | loose | | |
| Bowl floor | docs/objects/props-medium/water-barrel.md | 1 | r 30, azimuth 135 | loose | Upright | |
| Bowl floor | docs/objects/props-light/floodlight-stand.md | 2 | r 30, azimuths 45 and 225, beside the exit ladders | loose | Aimed inward | Footing inspection |
| Bowl floor | docs/objects/props-light/cable-reel-small.md | 2 | At the ladder feet | loose | | |

### Z6 to Z8. Scaffold decks (L1 to L5)

The baseline per quadrant per level, then exceptions. Twenty quadrant-levels in total.

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Every quadrant-level | docs/objects/structures/scaffold-section.md | 4 | The deck itself, four sections per quadrant per level, 80 total | bolted (clamps at the Stump face are rated) | | The Scaffold |
| Every quadrant-level | docs/objects/props-medium/scaffold-board-stack.md | 1 | Against the outer rail, mid-quadrant. Authored under 300 kg (Medium) so it stays a line-movable prop | stacked | Tangential | Spare boards |
| Every quadrant-level | docs/objects/props-light/clamp-crate.md | 1 | At the ladder-bay landing | loose | | Clamps for the next ring |
| Every quadrant-level | docs/objects/props-light/tool-bag.md | 1 | On the board stack | loose | | |
| Every quadrant-level | docs/objects/props-light/bucket.md | 1 | At the inner rail gap | loose | | Bolt bucket |
| Every quadrant-level | docs/objects/props-light/plank.md | 2 | Leaning on the inner rail | loose | Vertical | Plate skids |
| Every quadrant-level | docs/objects/props-light/floodlight-stand.md | 1 | Outer rail, quadrant center | loose | Aimed at the Stump face | Cutting light |
| Every level, N and S | docs/objects/props-light/paint-can.md | 1 each (10) | On the inner rail | loose | | Tape-line paint and Ascendant altitude marks |
| L1 E | docs/objects/heavy/water-tank.md | 1 | Deck center, r 20, azimuth 90, on the bare deck | fixed, breaks (ruptures at 1,500 N s, releases a water rush off the deck edge; does not move) | | Dust suppression for the cutting |
| L2 N | docs/objects/props-medium/welding-cart.md | 1 | r 20, azimuth 0 | loose | Radial | Plate cutting |
| L2 W | docs/objects/volatiles/cylinder-rack.md with 2 docs/objects/volatiles/propane-cylinder.md | 1 rack, 2 cyl | r 22, azimuth 270 | rack bolted, cylinders racked | Rack tangential | Torch gas, west face |
| L1 W, L3 W | docs/objects/volatiles/demolition-charge.md | 2 | On the W quadrant's maintenance clamp at the Stump face | bolted | | The west quadrant is scheduled to come down (**PROPOSED**) |
| L3 N | docs/objects/volatiles/cylinder-rack.md with 4 propane cylinders | 1 rack, 4 cyl | r 18, azimuth 0, inner side | rack bolted, cylinders racked | Tangential | Torch gas, north face |
| L3, all | docs/objects/props-medium/steel-crate.md | 4 | One per quadrant at the outer rail | loose | Tangential | Cut plate crates |
| L3 E | docs/objects/props-medium/cart-trolley.md | 1 | r 20, azimuth 90 | loose | Tangential | Tool cart |
| L3 S | docs/objects/props-medium/hand-truck.md | 1 | At the SE ladder landing | loose | Leaning | |
| L3 S | docs/objects/props-medium/welding-cart.md | 1 | r 20, azimuth 180 | loose | Radial | |
| L4 E | docs/objects/heavy/billet-crate.md | 1 | r 20, azimuth 90, on a wheeled cradle (rollers, 0.02), chocked by a chain | chained (800 N to unchock), then loose on rollers | Tangential | Next crane lift down. Unchocked, a Rigger's 600 N rolls it to the outer rail in 5 s; the Pulse (1.5 m/s2 felt on 1,200 kg) only nudges it. Rails yield to a Heavy above 5 m/s (**PROPOSED**) |
| L4 E | docs/objects/props-medium/welding-cart.md | 1 | r 22, azimuth 100 | loose | | |
| L5 S | docs/objects/volatiles/cylinder-rack.md with 4 propane cylinders | 1 rack, 4 cyl | r 20, azimuth 170, 3 m from the footbridge | rack bolted, cylinders racked | Tangential | Torch gas for the upper face |
| L5 S, Stump face | docs/objects/volatiles/gas-manifold.md | 1 | Bolted to the face at r 12, Z 39, at the Crown ladder base | fixed world geometry (05 section 6.4) | Jet along the footbridge, outward | Torch gas distribution |
| L5 N | docs/objects/props-medium/water-barrel.md | 1 | r 22, azimuth 0 | loose | Upright | |
| L5, all | docs/objects/props-light/cable-reel-small.md | 2 | At two ladder landings | loose | | Ring power tails |
| Stump face, all levels | docs/objects/rated/maintenance-clamp.md | 20 | One per quadrant per level at r 12, deck height. Moves for a Breaker alone, a Doubler at overspin, or a crew summing 1,000 N on one axis (a Rigger and a Scrapper) | rated | Radial | Holds the Scaffold to the Stump |
| Stump face | docs/objects/environment/chalk-and-graffiti.md | 4 sets | Inner-rail height, every quadrant | static | | Painted altitudes, prayers |
| Stump face | docs/objects/environment/structural-tape.md | 20 sets | Every clamp | static | | Map rule 7 |
| Ladder bays | Scaffold ladder bay (arena-unique) | 4 | Azimuths 45, 135, 225, 315 | static | | Crew access |
| Stump face S | Crown ladder (arena-unique) | 1 | r 12, azimuth 180, Z 38 to 60 | static | | Crew access to the Crown |

### Z10. The Maintenance Ring

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Ring | docs/objects/kinematics/maintenance-ring.md | 1 | Z 42, r 14 to 20 | kinematic | Clockwise from above | Clamp inspection gantry |
| Ring | docs/objects/props-light/cable-reel-small.md | 2 | Beside the motor housing | chained | | Clamp torque leads |
| Ring | docs/objects/props-light/tool-bag.md | 2 | At the boarding points | chained | | |
| Ring | docs/objects/props-light/chain-coil.md | 2 | Inner rail | chained | | Clamp slings |
| Ring | docs/objects/props-light/clamp-crate.md | 1 | Beside the motor housing | chained | | |

### Z12. The Crown

| Zone | Object (doc) | Count | Placement note | State | Orientation | Prep-crew reason |
|---|---|---|---|---|---|---|
| Crown | docs/objects/heavy/claim-marker.md | 1 | (0, 0, 60), 900 kg on polished runners (0.05, about 450 N to start: Rigger or heavier). Pallet inside the Pulse's eye (r 0 to 3). Dragged outside the eye it feels at most 1.8 m/s2 of Pulse against 0.5 of friction and creeps a few centimeters; the Breath's 1.6 m/s2 felt never lifts it. It does not leave the Crown | loose, on runners | | The Claim |
| Crown | docs/objects/environment/weave-ribbon-fragment.md | 6 | Scattered r 4 to 9 | loose | | Shear debris from the Severance |
| Crown | docs/objects/props-light/paint-can.md | 2 | At the north lip | loose | | Altitude marks |
| Crown | docs/objects/props-light/floodlight-stand.md | 2 | r 8, azimuths 60 and 300 | loose | Aimed at the marker | Survey lighting |
| Crown | docs/objects/environment/evidence-objects.md | 1 | Sealed survey canister bolted under the north lip at r 11, Z 59.4 (**PROPOSED**: the Verger's cache) | bolted | | Narrative evidence, one per Claim |
| Crown | LayCast broadcast board (arena-unique) | 1 | North lip, r 11, 3 by 2 m | static | Facing S | The holder's Face |
| Crown | Crown gate (arena-unique) | 1 | South lip at the ladder top | static, hinged | | Crew access |
| Crown | docs/objects/environment/force-volumes.md | 2 | Breath column and Pulse field, see events file | n/a | | |

## Totals by tier

| Tier | Placed count | Notes |
|---|---|---|
| Light (under 30 kg) | 171 | tool bags 34, buckets 24, planks 48, chain coils 10, paint cans 12, clamp crates 21, floodlight stands 31, cable reels 14, scrap kits 7, ribbon fragments 6, folding chairs 4 |
| Medium (30 to 300 kg) | 56 | board stacks 20, steel crates 11, carts 3, hand trucks 3, welding carts 3, water barrels 4, rebar bundles 2, gel pad 1, fuel drums 6 (counted again under Volatile), propane cylinders 10 (counted again under Volatile) |
| Heavy (300 to 3,000 kg) | 7 | shipping containers 2 (fixed, breaks), billet crates 3 (rollers), water tank 1 (fixed, breaks), Claim marker 1 (runners). Hanging and rated: 2 crane slabs, counted under Rated. The gas manifold is fixed world geometry per 05 section 6.4 and is not an object |
| Rated | 22 | maintenance clamps 20, crane counterweight slabs 2 |
| Volatile | 25 | propane cylinders 10, fuel drums 6, demolition charges 8, gas manifold 1 |
| Static structures | 14 | gangways 4, footbridge 1, ladder bays 4, Crown ladder 1, cranes 2, towers 2 (breakable) |
| Kinematic | 1 | the Maintenance Ring |

Tier note: the task lists docs/objects/heavy/crane-counterweight.md under heavy. 03-arenas.md names crane counterweights as rated loads. The Anchor treats them as rated (4,000 kg, yellow-and-black tape, moved only by a Breaker's 1,000 N line or a Doubler at overspin, with an authored consequence). The object doc should carry both tiers or move to rated.

## Dynamic object budget check

Simultaneously simulating bodies, not placed bodies. Objects sleep when at rest and wake on contact, line force, or force volume.

| Class | Placed | Worst case awake | Recommended cap | How the cap holds |
|---|---|---|---|---|
| Heavy | 7 (plus 2 rated slabs when released) | 4 (the three crates on rollers and the marker on runners; containers and the tank are fixed and never simulate as bodies) | **10** | Collapse debris is Light boards and clamps plus one Medium frame piece per section (bible changelog ruling). |
| Medium | 56 | The full Breath wakes every loose Medium inside r 24: 36 on the decks | **40** | Decks carry 36 loose Medium by placement. Racked cylinders, stacked boards, and chained objects are unaffected by force volumes (05 section 6.6). |
| Light | 171 | The full Breath wakes every loose Light inside r 24: about 100 | **100** | Lifted Light props that pass Z 70 despawn (**PROPOSED**). Props carried over the Rim settle and sleep. |
| Fragments | 0 at start | Collapse: 20 sections into about 120 Light boards and clamps plus 20 Medium frame pieces | **150**, 10 s lifetime per 6.5 (**PROPOSED** separate pool) | Only the Collapse and clamp pulls make fragments. |
| Scaffold geometry collections | 80 sections | 20 (one quadrant) during the Collapse | 20 | The Collapse is the only multi-section event; clamp pulls free 4 at a time. |

Spike events: the Breath peak (5 s every 40 s) and the Collapse (10 s once per round). The two never overlap in Scrap (Collapse at 75 s elapsed falls at 75 mod 40 = 35, the Breath is dying). In Claim the Collapse fires at 4:00 remaining per 05 section 9.2, 240 s elapsed, 240 mod 40 = 0, calm. In Held Breath the Light cap is enforced by despawn at Z 70.

## Arena-unique objects

### Rim crane (2)

- Dimensions: slewing base 6 m diameter, 10 m tall to the deck. Lattice boom 59 m long, 1.5 m square section, from (r 50, Z 10) to (r 13, Z 56), 51 degrees. Hoist line 2 m to the slab.
- Tier: static structure. The boom is doggable along its whole length and is a Perch surface. Not walkable for non-Climbers above the first 6 m.
- Appearance: grey lattice, the Claim holder's colors on the base (default Office grey), yellow-and-black tape at the boom root and hoist.
- Physics: static. The slab release is authored (see events file). The boom does not move during a match.

### Crane counterweight slab (2, uses docs/objects/heavy/crane-counterweight.md, tier override)

- Dimensions 2.4 by 1.0 by 1.0 m. Mass 4,000 kg. Rated. Hangs at r 13, Z 54 to 55 under each boom tip, 0.6 m from the Stump face and 1.0 m from the Ring's inner rail.
- Physics: rated and hanging, so force volumes do not touch it (05 section 6.6). A Breaker's Haul, a Doubler overspin, or any lines summing to 1,000 N on one axis (a Rigger and a Scrapper together) fires the authored release: 1.0 s brake squeal, then the hoist pays out and the slab drops down the Well mouth at free fall. It passes L5 inner edge at 1.9 s, the Rim datum at 3.3 s, the kill volume at 3.8 s. Anything Hung to it goes with it.

### Scaffold ladder bay (4)

- Dimensions: 3 m wide, r 20 to 23, Z 6 to 38, caged ladder with a 3 by 3 m grating landing at every level. Steel.
- Tier: static. Doggable. Climb speed 2.0 m/s, Breaker 1.6 m/s; Spool usable while climbing, Cutter one-handed (05 section 2.3).
- Appearance: galvanized, cage hoops every 1 m, quadrant number stenciled at every landing.
- Physics: the NW bay (azimuth 315) is part of the W quadrant's collapse and is destroyed by it. The other three are indestructible.

### Crown ladder (1)

- Dimensions: 22 m, Z 38 to 60, bolted to the south Stump face at r 12.0 to 12.5. Caged from Z 40 to 48. Hinged 1 m gate at the top in the shear lip.
- Tier: static. Doggable. The Breaker's honest route to the Crown, 14 s at 1.6 m/s. A climber on it shows a back plate to the whole south half of the map, and the south boom tip hangs 6 m above its upper rungs: a Gallows anchor by Reel.
- Appearance: aluminum, scuffed, prayer chalk on every tenth rung.

### Stump footbridge (1, docs/objects/structures/plank-bridge.md 4 m variant)

- 4.0 m long, 1.0 m wide, planks (x1.2), one handrail. Spans the Well mouth from L5 S inner edge to the Crown ladder base. Static, indestructible. The gas manifold's jet runs along it.

### LayCast broadcast board (1)

- 3 by 2 m LED board on a 2 m post at the Crown's north lip. Static, un-doggable (counts as Office property). Shows the current Claim holder's Face. Emissive is capped below line emissive (map rule 8).

### Crown gate (1)

- 1.0 m wide, 1.0 m high hinged steel gate in the shear lip at due S. Static. Opens inward. It stops a Breaker from being Jacked straight off the ladder top only if closed; it is open at round start.
