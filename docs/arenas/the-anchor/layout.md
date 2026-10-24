# THE ANCHOR: Layout

_Arena: The Anchor. File purpose: tier-by-tier, zone-by-zone walk with dimensions, routes per class, spawns, marker, and the Well._
_Sources: docs/bible/03-arenas.md (The Anchor, map rules 3 and 6), docs/bible/05-core-mechanics.md (2.2, 2.3, 3.2, 3.4, 7, 9.3, 9.4). Inventions are marked **PROPOSED**._

## Coordinate frame

- Origin: the Stump axis at Rim deck height. X east, Y north, Z up. Rim deck Z = 0 m. The lowest playable floor is the bowl floor at Z = -8 m.
- Azimuths: N = +Y, E = +X. Quadrants are centered on N, E, S, W and divided by ladder bays at azimuths 45, 135, 225, 315 degrees.
- Radii (r) are from the Stump axis.

Every number below is in meters unless stated.

## Radial section (the whole map in one line)

| r (m) | What is there |
|---|---|
| 0 to 12 | The Stump (solid Weave). Crown on top at Z 60. |
| 12 to 16 | The Well mouth. Open air from Z 60 down to the kill volume at Z -15. |
| 16 to 24 | The Scaffold. Five 8 m wide ring decks at Z 6, 14, 22, 30, 38. Footing stands on the bowl floor. |
| 14 to 20 | The Maintenance Ring at Z 42 (overlaps the Scaffold's inner half in plan). |
| 24 to 40 | The gap. Bowl floor at Z -8. Four plank gangways cross it at Z 0 to 6. |
| 40 to 60 | The Rim. Concrete deck at Z 0. Cranes at N and S, Office gallery at E, broadcast towers at NE and SW. |

## ASCII side elevation (N to S through the axis, not to scale)

```
Z 60  . . . . . . . . [=====CROWN=====] . . . . . . . . .     tip of crane boom
                       |   STUMP      |             /
Z 42  . . . . . . .[RING]           [RING]. . . . /
Z 38  . . . . . [L5]|              |[L5]        /
Z 30  . . . . . [L4]|              |[L4]      /
Z 22  . . . . . [L3]|              |[L3]    /        boom (lattice, 59 m)
Z 14  . . . . . [L2]|              |[L2]  /
Z 10  [CRANE DECK]==/ L1 ]         |[L1]/==[CRANE DECK]
Z  6     gangway  / [L1]           |   \  gangway
Z  0  ===RIM=====/      |          |    \======RIM===
Z -8      bowl floor    | Well mouth|        bowl floor
                        |  r12-16  |
Z -15 . . . . . . . . . | KILL     | . . . . . . . . . .
      r60    r40   r24 r16 r12  r12 r16  r24    r40    r60
S                                                        N
Legend: [ ] deck, === concrete, / lattice or ramp, . open air
Scale: 1 character is about 2 m horizontally; rows are the listed Z.
```

## ASCII plan, Rim tier (Z 0, with crane decks at Z 10)

```
                         N (+Y)
              . . . . . . . . . . . . . .
         .      [T]  BROADCAST NE             .
      .        ______                           .
    .         |CRANE |  <- Spawn A, Z 10          .
   .          |__N___|   boom runs to r13, Z56     .
  .     W-alt    ||gangway N                   E-alt .
  .     spawn  --++--------+------------        spawn.
  .      [X]   |  gap      |   gap      |       [X]   .   [G] Office gallery, E
  .   ---gang--|  bowl     | SCAFFOLD   |---gang W----.  (un-doggable, Z 0-12)
  .    way W   |  floor    | rings r16-24           [G]
  .            |  Z -8     |  Stump r12 |             .
  .            |           | Well r12-16|             .
   .           +-----------+------------+            .
    .                     ||gangway S              .
      .        ______     ||                     .
         .    |CRANE |  <- Spawn B, Z 10      .
            . |__S___|   [T] BROADCAST SW  .
              . . . . . . . . . . . . . .
                         S (-Y)
Legend: [X] alternate spawn (r50, Z0). [T] broadcast tower (r55, 30 m tall).
[G] Office gallery (r54-60, azimuth 75-105). ++ gangway head (r40).
Scale: the ring from the outer dots to the inner box is the Rim, 20 m wide.
```

## ASCII plan, one Scaffold deck (any of L1 to L5)

```
              315        N quadrant        45
                 \  ..................  /
                  \.   outer rail r24  ./
                [LB] ................ [LB]      LB = ladder bay (caged, 3 m wide)
                 .  .   deck 8 m     .  .
          W      .  .  . . . . . .  .  .     E
       quadrant  .  . .  Well    . .  .  quadrant
                 .  . .  mouth   . .  .
                 .  . .   r12-16 . .  .
                 .  . . [STUMP]  . .  .
                 .  . . r12      . .  .
                 .  .  . . . . . .  .  .
                [LB] ................ [LB]
                  /.   inner edge r16  .\
                 /  ..................  \
              225        S quadrant       135
Legend: . deck edge. Inner rail has 2 m gaps every 6 m (PROPOSED).
Scale: deck width 8 m. Circumference at r20 is 126 m, about 31 m per quadrant.
```

## Zones

Thirteen zones. Floor materials use the 05 section 7.3 surface table.

### Z1. North Crane Deck (Spawn A)

- Position: center (0, 50, 10). Octagon 10 m across. Floor: steel grating (hard, x1.0). 1.1 m rail. Crane cab (3 by 3 m) at the rear, r 54. Stair of two flights down to the Rim on the NE side.
- Contents: the slewing base of the north rim crane (arena-unique, see prop-manifest.md). The lattice boom leaves the deck at r 50, Z 10 and rises at 51 degrees to its tip at (0, 13, 56), over the Well mouth. A 4,000 kg counterweight slab hangs 2 m under the tip, rated, yellow-and-black taped. Generator, one racked fuel drum, tool bags.
- Sightlines: everything. The crane deck is the best camera on the map. The Crown lip is visible at 50 m. The far side of the Scaffold is hidden by the Stump.
- Edges: all eight sides. The 10 m drop to the Rim is under the Stall threshold (7.3 m) only if you land on the deck; it is not, so do not step off. Spawn protection: none beyond the 6 s Drop.
- Routes out: stair to the Rim (all classes). Dog the boom and ride it (Climber: Perch and hop along the lattice; Rigger: Hang-pendulum under it). Climber Slingshot: self-dog, Dog B the cab wall, Jack toward the Stump; L1 N's outer rail is 26 m out and 4 m down, inside the Climber's 33 m flat range. Scrapper and Doubler reach L1 the same way with a flat launch. Rigger and Breaker: stair, then gangway N.

### Z2. South Crane Deck (Spawn B)

Mirror of Z1 at (0, -50, 10). Stair on the SW side. The south crane's boom tip is at (0, -13, 56), directly over the Crown ladder's top.

### Z3. The Rim

- Annulus r 40 to 60, Z 0. Floor: concrete (hard). 1.1 m parapet on the inner edge at r 40 with four 2.5 m gaps for the gangways. Width 20 m.
- Contents by arc. N arc: crane N base, billet crate, two cradled fuel drums, floodlight stands. E arc: the Office gallery (docs/objects/environment/office-gallery.md) at r 54 to 60, azimuth 75 to 105, 12 m tall, un-doggable per 05 section 3.1; a floodlight mast at azimuth 90, r 58. S arc: crane S base, mirror of N. W arc: two shipping containers as contractor stores, rebar bundles, scrap heaps, a floodlight mast. Broadcast towers at r 55, azimuths 45 and 225, 30 m tall, each with three demolition charges on its legs (staged for the post-match strike, **PROPOSED**).
- Alternate spawns: E at (50, 0, 0) and W at (-50, 0, 0), each 10 m across.
- Sightlines: across the gap to the whole outer face of the Scaffold. The Crown from anywhere. Cover: containers (W), crane bases (N, S), gallery pillars (E), billet crates.
- Edges: the inner parapet. Behind it is the 8 m drop to the bowl floor (8 m onto concrete is a Stall: 12.5 m/s). The outer edge at r 60 is a 2 m wall, the end of the playable space.
- Routes: gangways to L1 (all classes). From the parapet a Climber's Slingshot reaches L3's outer rail with a steep launch (16 m out, 22 m up). A Scrapper or Doubler reaches L1 or L2 (6 to 14 m up). A Rigger or Breaker takes the gangway. No Haul lifts a player here: every class's pull tension is under every class's weight (05 section 3.4).

### Z4. The Gangways (N, E, S, W)

- Four plank bridges (docs/objects/structures/plank-bridge.md), each 17.1 m long, 2.0 m wide, from the Rim parapet gap (r 40, Z 0) to the L1 outer rail gap (r 24, Z 6). Slope 20.6 degrees. Handrail on the left side going in, 1.5 m. Floor: planks (x1.2).
- They are the Breaker's only honest crossing. They are also the Clothesline's favorite doorway. A gangway fractures at 600 N s (05 section 6.5, v0.2.2): a falling Heavy, a blast, or a Collapse section drops it into the gap. Lines do not.
- Edges: the open right side, 8 to 14 m above the bowl floor. A player flipped off a gangway lands on concrete at 12.5 to 16.6 m/s: Stall.

### Z5. The Bowl Floor

- r 16 to 40, Z -8. Concrete (hard). Between r 16 and 24 it is the Scaffold's footing: steel standards every 2 m, braces at 2 m height, dark. Between r 24 and 40 it is open and lit by two floodlight stands.
- Contents: scrap heaps (x2.0 surface under the gangway heads), one Stall Gel pad (4 by 4 m, x4.0) at (0, 32, -8) under the north boom, loose planks, a water barrel, chain.
- Exits: two ladders to the Rim at azimuths 45 and 225 (r 40, 8 m). Nothing else. A player who goes down here has left the fight for 20 s.
- Inner edge: the Well mouth at r 16, guarded by a 0.2 m toe board only. The Well kill volume is 7 m further down.

### Z6. Low Scaffold (L1 at Z 6, L2 at Z 14)

- Ring decks r 16 to 24, 8 m wide, four quadrants. Floor: planks over steel ledgers (x1.2). Outer rail 1.1 m continuous. Inner rail 1.1 m with 2 m gaps every 6 m (**PROPOSED**; the cladding crew removed sections to pass plates). Toe boards everywhere.
- Contents: scaffold board stacks (docs/objects/props-medium/scaffold-board-stack.md), clamp crates, tool bags, buckets, floodlight stands. L1 E: a 1,500 kg dust-suppression water tank (docs/objects/heavy/water-tank.md), fixed to the deck per 05 section 6.1a: it ruptures (1,500 N s) and releases its water as a force volume; it does not move. L2 N: a welding cart. L2 W: a two-cylinder propane rack. L1 W and L3 W: a demolition charge on the quadrant's maintenance clamp (**PROPOSED**, see events file).
- Sightlines: along your own quadrant, 31 m. The Stump blocks the far side. The deck above cuts upward view to a 4 m band at the inner and outer edges. Across the Well mouth you see the Stump face, 4 m away.
- Edges: the Well mouth (inner, 4 m to the face, lethal), the outer rail (8 m above the gangway heads at L1, 22 m above the bowl floor at L2).
- Routes: ladder bays (Z9) up and down. Gangways down (L1 only). Slingshot: a Climber or Doubler gains three levels in one move (30 and 27 m of rise against 24 m needed); a Scrapper gains two (16 m); a Rigger or Breaker gains one (14 and 9 m against 8 m). See the reach table below. Breaker: ladders, 8 m per level at 1.6 m/s, 5 s per level (4 s for everyone else at 2.0 m/s, 05 section 2.3).

### Z7. Mid Scaffold (L3 at Z 22)

- Same deck shape. The main fighting deck. Densest props: a four-cylinder propane rack at L3 N (racked, 200 N break-free), steel crates, a tool cart, two welding sets, paint cans, and the Ascendants' altitude marks on the Stump face behind the inner rail.
- Sightlines: as Z6, plus across the gap to the Rim parapet (16 m horizontal, 22 m vertical) and down to both crane decks.
- Routes: one Slingshot reaches the Ring (20 m) or the Crown lip only at a near-vertical angle (38 m up, 12 m in: 40 m), which fails without the Breath. From L3 a Climber reaches the Crown in two moves, or in one on the Breath (see events file).

### Z8. High Scaffold (L4 at Z 30, L5 at Z 38)

- L5 is the launch deck. The Ring passes 3.5 m above it (deck underside Z 41.5). L5 S holds the Stump footbridge: a 4 m plank bridge across the Well mouth to the Crown ladder's base at the Stump face. A four-cylinder propane rack stands at L5 S, 3 m from the footbridge. The torch gas manifold (docs/objects/volatiles/gas-manifold.md, Heavy, fixed) is bolted to the Stump face at the ladder base; its jet points along the footbridge.
- L4 E holds a billet crate (Heavy, 1,200 kg, on a wheeled cradle with rollers at friction 0.02, chocked with a chain at 800 N) and a second welding cart. Unchock it (Doubler or Breaker Haul) and a Rigger alone rolls it to the outer rail in 5 s (600 N against 240 N of rolling friction is 0.3 m/s2); the Pulse barely nudges it (1.5 m/s2 felt, 0.4 s).
- Sightlines: from L5's inner edge, up the bare Weave face 22 m to the Crown lip. From L5's outer rail, down to everything. The Ring blocks the sky for 20 s of every 120 s as it passes.
- Edges: as below, plus the Ring's drop ladder sweeping past at 1 m/s.
- Routes: Crown by Slingshot from L5's inner half (22 m up, 6 to 14 m in). A Climber (30 m of rise) and a Doubler (27 m) make it without the Breath and pass the lip at about 9 and 7 m/s. A Scrapper (16 m), Rigger (14 m), and Breaker (9 m) do not; they ride the full Breath (see events file), Reel up from a Hang on the lip (22 s from L5 for a 40 m or longer line, 17 s from the Ring for anyone), or take the ladder. Ring by its drop ladder (all classes, once per 120 s per azimuth) or by a Dog and a Hang to its rail (4 m up: anyone, then mantle). Crown ladder from the L5 S footbridge (all classes, 22 m, 11 s).

### Z9. The Ladder Bays

- Four caged ladders at azimuths 45, 135, 225, 315, r 20 to 23, Z 6 to 38, with a 3 by 3 m landing at every level. Floor: steel grating. Arena-unique (see prop-manifest.md).
- Climb speed 2.0 m/s, Breaker 1.6 m/s (05 section 2.3). Spool usable while climbing; Cutter one-handed. A player on a ladder is a player with a back plate showing. One level (8 m) is 4 s, 5 s for a Breaker.
- The NW bay (315) is destroyed by the Collapse. The SW bay (225) survives as the west cliff's only ladder.

### Z10. The Maintenance Ring

- docs/objects/kinematics/maintenance-ring.md. Z 42, r 14 to 20, 6 m wide steel grating, 1.2 m rails inside and out, two 3 m rail gaps (boarding points) at opposite azimuths. One revolution per 120 s, clockwise seen from above, 3 degrees per second, 0.89 m/s at mid-width. A drop ladder hangs from the outer edge at one boarding point down to Z 39.2.
- It is kinematic (05 section 6.7). Infinite mass. A line between the Ring and anything moves the anything. Hang to it and ride. Hang an enemy to it and they ride. It severs any line it passes through (05 section 3.5): any rod, set line, or live line crossing r 14 to 20 at Z 42 snaps when the Ring reaches that azimuth, once per 120 s. A line from L5's inner edge to the Crown lip crosses that volume. Lines attached to the Ring itself are safe.
- Sightlines: the entire Scaffold's inner edges, the Crown lip 18 m up, the whole Rim. The best overwatch for a Rigger.
- Edges: inner rail, 2 m from the Stump face, over the Well mouth. Outer rail, over L5 (4 m drop, fine) or, after the Collapse, over the west void (42 m, lethal).
- Props on the Ring are chained (800 N) so only a Doubler or a Breaker frees them.

### Z11. The Stump Face

- Cylinder r 12 from Z -8 to 60. Below Z 40 it is grey cladding with cut lines; above Z 40 it is bare Weave, dark and faintly iridescent. Dogs seat anywhere. Climbers Perch anywhere for 1.5 s.
- The Crown ladder (arena-unique) runs up the south face from Z 38 (footbridge) to Z 60 (gate in the lip), caged from Z 40 to 48 where the Ring passes 2 m outside it. The Breaker's honest route to the Crown, 22 m, 14 s at 1.6 m/s (11 s for everyone else), back plate to the world.
- The Reel route (05 section 3.2): any class whose line reaches the Crown lip can Dog it, Hang, step off, and hold Haul to Reel up at 1.0 m/s. From L5's inner edge the lip is 22.4 m away: Climber, Rigger, and Doubler, 22 s of climb. From the Ring's inner rail it is 17.1 m: every class, 17 s. The climber hangs over the Well mouth the whole way, chest to the lip, line exposed to any Cutter at the top and to the Ring's sweep (from L5 only). The ladder is faster and keeps your feet on something; the Reel keeps your chest plate toward the threat.
- Dust sheets off the face 5 s before the Pulse. Chalk prayer lines and painted altitudes mark it in every quadrant.

### Z12. The Crown

- Z 60. The sheared top is 24 m across. The central 20 m (r 0 to 10) is flat Weave, slightly glassy (hard, x1.0). The outer 2 m (r 10 to 12) is the shear lip: a broken 1 m high ring of torn Weave strands, climbable, uneven. No rail. The Crown gate (hinged, 1 m) is in the lip at due S.
- The Claim marker (docs/objects/heavy/claim-marker.md) sits at (0, 0, 60). The eye, r 0 to 3, is calm in the Pulse (**PROPOSED**). The Breath is calm inside r 8 (**PROPOSED**). Both calm zones sit inside the marker's 4 m radius.
- Contents: ribbon fragments (Light), two paint cans, two floodlight stands, the LayCast broadcast board on the north lip, the evidence cache under the north lip (docs/objects/environment/evidence-objects.md).
- Sightlines: everything outside the Scaffold. The decks below are hidden by the overhang of the Stump; you see only their inner edges from the lip.
- Edges: all of it. Over the lip is the Well mouth, then the Scaffold's inner edges 22 m and further down, then the kill volume. A player who goes over the lip at low speed may Dog the face, Hang, and Reel back up at 1 m/s: that is the only save.
- Routes: down to L5 by Pendulum from the crane boom tips (56 m) or by the ladder, or by Hanging to the lip and holding Jack to Reel out at 1 m/s. Down to the Ring by Dog and Reel (18 m). A Breaker leaves by the ladder, by a 17 m Reel down to the Ring, or not at all.

### Z13. The Well

- The annulus r 12 to 16 from the Crown lip down to the kill volume at Z -15. Open to the sky except where the Ring sweeps across it at Z 42 and the footbridge crosses it at Z 38.
- Kill volume: cylinder r 16, Z below -15 (docs/objects/environment/well-kill-volume.md). The bowl floor at r 16 to 40 is not part of it.
- No Hooks go down it (03). The ring-out tell is the Stump's hum dropping an octave, 0.5 s before the volume (canon, 05 v0.2.2), with the Stump's glow brightening around the faller. The Marshal calls it.
- The Updraft rises out of it from Z 0 upward; below Z 0 the shaft is still air (**PROPOSED**). A player who falls below the Rim datum inside the Well is not coming back.

## Spawns (05 section 9.3)

**Canon (05 section 9.3 v0.2.2: spawn distance is measured along walkable routes, and arenas where every tier overhangs the Well spawn on the Rim's crane decks).** The original conflict: section 9.3 required spawns on the high or mid tier, 10 m across, never within 25 m of a Well edge. Every Scaffold level is 0 to 8 m from the Well mouth (r 16). The Crown is 2 m from it. No high or mid floor on this map satisfies the 25 m rule. The spawns therefore use the crane decks, which are at mid-tier height (Z 10), 34 m from the Well edge in a straight line and 45 m along the walkable route (stair 10 m, Rim 10 m, gangway 17 m, L1 deck 8 m).

| Spawn | Center | Size | Distance to Well edge (straight / walkable) | Distance to other spawn |
|---|---|---|---|---|
| A (crew 1) | (0, 50, 10) North Crane Deck | 10 m | 34 m / 45 m | 100 m |
| B (crew 2) | (0, -50, 10) South Crane Deck | 10 m | 34 m / 45 m | 100 m |
| Alternate A' | (50, 0, 0) Rim E | 10 m | 34 m / 35 m | |
| Alternate B' | (-50, 0, 0) Rim W | 10 m | 34 m / 35 m | |

Reserve riggers (Claim) drop at their crew's spawn, or at the alternate if an enemy is within 15 m. After the Collapse the W alternate is under debris; it moves to (-50, -20, 0).

## Claim marker

Authored at (0, 0, 60), the Crown center. 4 m radius, Heavy, a 900 kg Weave billet on a pallet, on polished runners (friction 0.05 per 05 section 6.1a). Runner friction is about 450 N, so a Climber or Scrapper (400 N) cannot start it; a Rigger (600 N) moves it at 0.17 m/s2 and a Breaker (1,000 N) at 0.6 m/s2: 10 m to the lip takes 11 s for a Rigger and 6 s for a Breaker. The pallet sits entirely inside the Pulse's calm eye (r 0 to 3) at its authored position. Dragged outside the eye, the Pulse rolls it outward and the 1 m shear lip stops it (**PROPOSED**); the Breath does not lift Heavy objects. The marker cannot be pushed over the lip by any line, so the 05 section 9.4 Well reset (authored position, 3 s delay, **PROPOSED**) exists but is not reachable in practice.

## Haul

Not supported. No truck positions, no spool start.

## Slingshot reach per class

A Slingshot is self-dog A, Dog B a surface behind you, Jack. Per 05 section 3.2 the player leaves at the class launch speed V, then the line keeps pushing at T until it reaches class length. Rise straight up is V squared over twice (g minus T over m). Flat range is the bible's 33 m for a Climber at 45 degrees, scaled by V squared for the others.

| Class | V (m/s) | T over m (m/s2) | Rise, no Breath (m) | Scaffold levels per move | Flat range (m) | Crown from L5 (22 m up) |
|---|---|---|---|---|---|---|
| Climber | 18 | 4.4 | 30 | 3 | about 33 | Yes |
| Doubler | 14 | 6.2 | 27 | 3 | about 22 | Yes, barely |
| Scrapper | 14 | 3.6 | 16 | 2 | about 20 | No |
| Rigger | 12 | 4.6 | 14 | 1 | about 15 | No |
| Breaker | 10 | 4.5 | 9 | 1 | about 10 | No |

The Breath adds up to 14 m/s2 of lift inside r 18 and changes every row; see events-and-moving-parts.md.

## Route table per class

Honest routes use no lines. Slingshot routes name the Dog B anchor; the anchor must stay within class line length for the push to last.

| From | To | Breaker honest route | Climber Slingshot | Other classes |
|---|---|---|---|---|
| Crane deck (Z 10) | Rim (Z 0) | Stair, 6 s | Step off onto the crane base roof (Z 4), drop | Same |
| Rim (r 40) | L1 (r 24, Z 6) | Gangway, 17 m, 4 s | Self-dog, Dog B parapet behind, Jack flat (the Kick: 18 m/s): 16 m out, 6 m up | Scrapper, Doubler: same. Rigger: gangway |
| Rim | L3 (Z 22) | Gangway, then ladder bay, 2 levels, 14 s | Steep launch (about 70 degrees) from r 38: 14 m out, 22 m up, Perch the rail | Nobody else in one move |
| L1 | L5 (Z 38) | Ladder bay, 4 levels, 20 s at 1.6 m/s | Two moves (30 m rise each against 32 m) | Doubler two; Scrapper two to L3 then two more; Rigger one level per move; ladders 16 s for non-Breakers |
| L5 | Ring (Z 42) | Drop ladder as it passes, up to 120 s wait | Jump, Dog the Ring rail, Hang, Reel up, mantle | Anyone, same |
| L5 S | Crown (Z 60) | Footbridge 4 m, Crown ladder 22 m, 17 s | Self-dog, Dog B L5 deck, Jack up and inward: 22 m up, 6 m in, pass the lip at about 9 m/s, Perch | Doubler: pass the lip at 7 m/s and land on the Crown edge. Rigger, Doubler, Climber: Dog the lip from L5's inner edge (22.4 m), Hang, step off, Reel 22 s. Scrapper, Breaker: full Breath, ladder, or Reel from the Ring |
| Ring | Crown | Dog the lip from the inner rail (17.1 m), Hang, step off, Reel 17 s | Self-dog, Dog B the Ring deck, Jack: 18 m up | Everyone by Reel; Climber and Doubler by Slingshot |
| L4 | Crown | Ladder to L5, then as above | 30 m up is the Climber's whole rise: needs the Breath's build phase | Breath, or Reel from a 31 m Hang (Climber, Rigger) |
| Crown | L5 | Ladder, 14 s | Drop off the lip, Dog the face at Z 45, Hang, swing to L5's inner edge | Anyone with a Hang; or Hang to the lip and Reel out |
| Rim | Crown | 85 s: stair, gangway, ladders at 1.6 m/s, footbridge, Crown ladder | Not in one move: 60 m up | Fails every class |

Map rule 3 check: Rim to Scaffold (low to mid) is one Climber Slingshot and one Breaker gangway. Scaffold L5 to Crown (mid to high) is one Climber Slingshot and one Breaker ladder. Pass.

## Cover summary

| Zone | Cover |
|---|---|
| Rim | Containers W, crane bases N and S, gallery pillars E, billet crates |
| Scaffold decks | Board stacks, crates, the ladder bay cages, the Stump itself (walk around it) |
| Ring | Motor housing, hose reels |
| Crown | The marker. The lip, if you crouch behind it. Nothing else |
| Crane decks | The cab |
