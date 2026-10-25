# THE YARD: Layout

_Arena: The Yard (Collar Four Breaking Yard). File purpose: tier-by-tier, zone-by-zone geometry, routes, spawns, marker, Haul positions, and the Well._
_Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Anything not in those files is marked **PROPOSED**._

## Coordinate frame

- Origin (0, 0, 0): center of the center trench's Floor-level span, 45 m west of the press pit center. X positive east (along the gantry rails). Y positive north. Z up.
- Footprint: X -80 to +80 (160 m), Y -45 to +45 (90 m).
- Datum: the Floor islands are Z = 0. Trench floors are Z = -4. The lowest playable floor is the trench floor.
- Unreal units: multiply by 100.

## Tier summary

| Tier | Zones | Z (m) |
|---|---|---|
| Low: the Floor | The Trenches (3), South Dock, Haul Lane, Marker Flat, North Dock, The Press | -4 to 0, grate crossings at 3.5 |
| Mid: the Stacks | West Mesa (Spawn A), Pick Stack, The Canyons, East Pick Stack, East Mesa (Spawn B), Shack Perch | 2.7, 5.4, 8, 10.5 to 14 |
| High: the Rails | North Rail Walk, South Rail Walk, Mast Platforms, The Bridge (Gantry) | 24, 36, 40 |

13 zones. Each trench counts as one zone with three lanes.

## Floor plan (top-down)

Scale: 1 character = 2 m in X, 1 row = 3 m in Y. North is up. Mesa footprints are drawn on the Floor plan so the islands read.

```
      X  -80                              0                                  +80
        +--------------------------------------------------------------------------------+
  Y +45  |....................................................................BBBBBBBBBBBB|
         |.........C....L...............~...................~...............L.BBBBBBBBBBBB|
  Y +39  |....................................................................BBBBBBSBBBBB|
         |.........Cl...................l...................l.................BBBBBBBBBBBB|
  Y +33  |===============H=========p=========H=========p=========H===================H====|
         |===============H=========p=========H=========p=========H===================H====|
  Y +27  |..........l...........######..l...######..........l....rrrrrr.........l.........|
         |....................~.######......######..........g....rrrrrr...................|
  Y +21  |..rrrrrr....kkkkkk....######......######............................eeeeee......|
         |..rrrrrr....kkkkkk....######.................~.######...............eeeeee......|
  Y +15  |............kkkkkk.............................######...............eeeeee......|
         |............kkkkkk......................M......######....c..........eeeeee......|
  Y  +9  |...............................................######......PPPKPPP..............|
         |...................................~........~..............PPPPPPP..............|
  Y  +3  |===============H=========p=========H=========p=========H===PPPPPPP=========H====|
         |===============H=========p=========H=========p=========H===PPPPPPP=========H====|
  Y  -3  |..rrrrrr..................................######...........PPPPPPP..............|
         |..rrrrrr............~.....b...............######.......b....~...................|
  Y  -9  |................................................................................|
         |..T..................................w..o..w.................................T..|
  Y -15  |..................######......######...............######.......................|
         |..........b....b..######....c.######...............######........b....brrrrrr...|
  Y -21  |..................######......######...............######..............rrrrrr...|
         |..........l.......######......######..............l######.............l.........|
  Y -27  |===============H=========p=========H=========p=========H===================H====|
         |===============H=========p=========H=========p=========H===================H====|
  Y -33  |AAAAAAAAAAAA..................l...................l...................l.........|
         |AAAAAAAAAAAA....................................................................|
  Y -39  |AAAAAASAAAAA..L..........~..............~..............~..........L.............|
         |AAAAAAAAAAAA..........................................................f.........|
        +--------------------------------------------------------------------------------+
Legend: . Floor (0 m)   = trench (-4 m)   P press pit (the Well)   K press counterweight (rated)
        H grate footbridge (3.5 m)   p plank bridge (0 m)   l portal leg with cage ladder
        L floodlight mast   C control shack column (rated)   r rated hull section on the Floor
        # Canyon mesa (8 m)   k Pick Stack   e East Pick Stack   A West Mesa   B East Mesa   S spawn center
        M Claim marker   o Haul spool start   T Haul truck   b billet stack   ~ scrap heap
        c propane cylinder rack   f fuel drums   g gas manifold   w welding cart
```

## Stacks plan (top-down)

```
      X  -80                              0                                  +80
        +--------------------------------------------------------------------------------+
  Y +45  |       CCCC                                                         ############|
         |       CCCC                                                         ############|
  Y +39  |       CCCC                                                         R#####S#####|
         |          l                   l                   l                 ##l#########|
  Y +33  |================================================================================|
         |================================================================================|
  Y +27  |          l         11######22l   ######22        l                   l         |
         |                    11######22    ######22                                      |
  Y +21  |            ######2211R#####22    R#####22                          ######      |
         |            ######2211######22               22######11             ######      |
  Y +15  |            ######R2                         22######11             ######R     |
         |            ######22                         22R#####11             ######      |
  Y  +9  |                                             22######11    PPPKPPP              |
         |                                                           PPPPPPP              |
  Y  +3  |===========================================================PPPPPPP==============|
         |===========================================================PPPPPPP==============|
  Y  -3  |                                          ######           PPPPPPP              |
         |                                          R#####                                |
  Y  -9  |                                                                                |
         |                                                                                |
  Y -15  |                  ######22    ######11             ######22                     |
         |                  ######22    ######11             ######22                     |
  Y -21  |                  R#####22    R#####11             R#####22                     |
         |          l       ######22    l#####11            l######22           l         |
  Y -27  |================================================================================|
         |================================================================================|
  Y -33  |##########l#                  l                   l                   l         |
         |############                                                                    |
  Y -39  |######S#####R                                                                   |
         |############                                                                    |
        +--------------------------------------------------------------------------------+
Legend: # mesa top (8 m)   1 single ledge (2.7 m)   2 double ledge (5.4 m)   R welded rung ladder on a mesa end
        C control shack (floor 10.5 m, roof 14 m)   l portal leg cage ladder   S spawn center   K counterweight
```

## Rails plan (top-down)

```
      X  -80                              0                                  +80
        +--------------------------------------------------------------------------------+
  Y +45  |       CCCC                                                                     |
         |       CCCC  LL                                                  LL             |
  Y +39  |       CCCC                                                                     |
         |                                                             cc                 |
  Y +33  |================================================================================|
         |          l                   l                   l          GG       l         |
  Y +27  |                                                             GG                 |
                                 (bridge beam spans Y -30 to +30)      GG
  Y  +3  |                                                           PPPPPPP              |
         |                                                           PPPPPPP              |
  Y  -3  |                                                           PPPPPPP              |
         |                                                             GG                 |
  Y -27  |=============================================================GG=================|
         |          l                   l                   l                   l         |
  Y -33  |                                                                                |
         |             LL                                                  LL             |
  Y -39  |             LL                                                  LL             |
        +--------------------------------------------------------------------------------+
Legend: = rail walkway (36 m)   G Gantry bridge beam (top 36 m), drawn at X = +45, its position at "live" in Scrap
        c Gantry cab (40 m)   L mast platform (24 m)   l portal leg head   C shack roof (14 m)
```

## Side elevation (looking north, X across, Z up)

```
 Z
 40 |                                                             [cab]
 36 |--rail walk----------------------------------------------------GG---------------rail walk--
 30 |          [load, 30 m, during traverse ->]                      |
 24 |   L                                                            |                    L
 14 | CCC shack                                                      |
  8 | AAAA   kkk   ###     ###   ###     ###                         |     eee   BBBB
  5 |              ##2     ##2   ##2     1##                         |     ee
  3 |     H              H             H           H  (grate footbridges at 3.5 m)
  0 |________________________________________________________|  pit  |________________________
 -4 |  trench floor, Hauler rail bed (Hauler deck at -0.4, load top at +3)   |       |
-10 |                                                                        |  ram  |
    +-------------------------------------------------------------------------------------------
     X -80                             0                            +38    +52             +80
```

## Zones

Dimensions are given as X extent by Y extent. "Routes" lists the honest Breaker route and the Climber Slingshot route; the other classes fall between them (Rigger 40 m line, Doubler 30 m, Scrapper 20 m). Ladder climb is 2.0 m/s, Breaker 1.6 m/s (05 section 2.3); the Spool works while climbing.

### Low tier: the Floor

**1. The Trenches (South, Center, North).** Three cuts, each 160 m by 6 m, 4 m deep, centered on Y = -30, 0, +30 (**PROPOSED** positions). Floor: oiled slag over steel rail bed. Contents: twin rails, trench-lip amber lamps every 10 m, steel rungs welded to the north wall every 20 m (**PROPOSED**; the only way out on foot, since a 4 m wall exceeds every mantle height). Haulers run here (docs/objects/kinematics/rail-hauler.md): the deck rides at -0.4 m, the hull plate load tops out at +3 m. South and North Haulers run west to east; Center Haulers run east to west and cross the pit on two rail stringers at -4 m. Sightlines: the full 160 m along the trench, nothing across it. Cover: none. Edges: both lips are edges onto the kill lane; the center trench's lips at X +38 to +52 are the pit. Routes across: grate footbridges (3.5 m, fixed, X = -50, -10, +30, +70), plank bridges (0 m, docs/objects/structures/plank-bridge.md, X = -30 and +10; fracture 600 N·s per 05 section 6.5, so the first Hauler load through each trench sweeps them away, and a Breaker who jumps onto one breaks it), a Hang line footbridge (any class), a Slingshot (any class, 6 m gap), a Hauler ride. A Breaker's honest crossing is a grate.

**2. South Dock.** X -56 to +80 by Y -45 to -33 (12 m deep strip; West Mesa occupies the west end). Floor: concrete apron, drainage grooves. Contents: two floodlight masts (X = -52 and +52, Y = -40, docs/objects/environment/floodlight-mast.md), three scrap heaps, the Hauler fuel point (six fuel drums, X +60, Y -42), chain coils, cable reels. Sightlines: east-west along the strip, broken by the mast bases; north across the south trench into the Haul Lane. Cover: mast bases, heaps. Edges: the south trench lip along the whole north side.

**3. Haul Lane.** X -80 to +80 by Y -27 to -3 (the south-center island). Floor: steel deck plate. The Haul route is the clear 6 m corridor Y -15 to -9. Contents: Truck A at (-75, -12), Truck B at (+75, -12), the spool start at (0, -12), six billet stacks (docs/objects/structures/billet-stack.md) staged for shipping at X -60, -50, +50, +60 (Y -20) and X -28, +30 (Y -6), two welding carts at (-6, -14) and (+6, -14), one propane cylinder rack at (-24, -20), rated hull sections R3 (X -76 to -64, Y -9 to -3) and R4 (X +62 to +74, Y -25 to -19). Four Canyon mesas stand on this island (SC1 to SC4). Sightlines: the corridor is open end to end, 160 m, which is the point of the Haul route; the mesas break the north and south sides every 10 to 14 m. Cover: mesa faces, billet stacks, R3 and R4. Edges: the south trench lip (Y -27) and the center trench lip (Y -3).

**4. Marker Flat.** X -80 to +80 by Y +3 to +27 (the north-center island). Floor: steel deck plate with slag drifts. Contents: the Claim marker at (0, +12), four scrap heaps around it at 8 to 12 m, three Canyon mesas (NC1, NC2, NC3), the Pick Stack and East Pick Stack, rated hull sections R1 (X -76 to -64, Y +13 to +19) and R2 (X +30 to +42, Y +19 to +25), a cylinder rack at (+34, +10), the gas manifold at (+20, +22), cart trolleys and brick pallets. Sightlines: the marker is visible from the Rails and from NC2's top; at Floor level it is visible from 20 m in any direction and from nowhere further. Cover: heaps (x2.0 soft surface, 05 section 7.3), mesa faces, R2. Edges: the center trench lip (Y +3) and the north trench lip (Y +27). The pit's north lip is 7 m from this island's south edge at X +38 to +52.

**5. North Dock.** X -80 to +56 by Y +33 to +45 (East Mesa occupies the east end). Floor: concrete apron. Contents: the control shack's two structural columns at (-62, +35) and (-62, +41) (docs/objects/rated/structural-column.md), two floodlight masts (X = -52 and +52, Y = +40), two scrap heaps, cable reels, a hand truck. Sightlines: east-west along the strip. Cover: columns, mast bases. Edges: the north trench lip along the south side.

**6. The Press (the Well).** Pit 14 m by 14 m, X +38 to +52, Y -7 to +7, open at Floor level, walls sheer steel to -10 m. The ram rises from -10 m to -6.5 m on a 20 s cycle regardless of occupancy. Kill volume: the pit below -6 m (docs/objects/environment/well-kill-volume.md). Hooks spin up 0.5 s before entry from a hatch in the pit's north wall (**PROPOSED**). The press frame is a steel portal over the west half of the pit, 12 m tall; the press counterweight (docs/objects/rated/press-counterweight.md) hangs from the frame's north side at (45, +9), 6 m above the Floor, as a rated load. A 1.1 m pipe rail runs the pit's north and south lips (**PROPOSED**; it stops a walking player and not a dragged one, since a drag goes airborne above the class foot-hold). The east lip is open: that is the press approach. Sightlines: from the Rails, the pit is a lit square. From the Floor, the frame hides the far side. Edges: all four lips.

### Mid tier: the Stacks

Hull sections (docs/objects/structures/hull-plate-section.md) are 12 m long, 6 m wide, 2.7 m deep, 2,800 kg, Heavy. Stacked three high they form 8 m mesas. The mesas are tack-welded by the Yard crew and are static world geometry (**PROPOSED**), so that the mid tier cannot be deleted; only the Pick Stack tops are live.

**7. West Mesa (Spawn A).** X -80 to -56 by Y -45 to -33, top at 8 m, on the South Dock. Spawn A is the 10 m circle centered at (-68, -39, 8). Distance to the nearest pit edge (38, -7): 110 m straight, about 140 m along the shortest walkable route (rungs, dock, grate 1 stairs, Haul Lane). Contents: four folding chairs and a tool bag (the shift's smoke corner), two cable reels. Sightlines: the whole Haul Lane west half and the south rail walk. Cover: a 1.2 m parapet of cut plate on the north edge (**PROPOSED**). Routes down: welded rungs on the east face (-56, -39), a drop to the dock (8 m, over the 7.3 m Stall fall: do not jump; drop to the 5.4 m ledge first). Routes up to the Rails: Breaker takes the dock to the portal leg at (-60, -30), cage ladder 36 m at 1.6 m/s, 22.5 s. Climber: self-dog, Dog B the trench floor at (-62, -30, -4), Jack straight up. An 18 m/s Kick plus the line's 400 N push (4.4 m/s² against gravity, 05 section 3.2) gives about 30 m of gain from the 8 m mesa: the 36 m walkway with 2 m to spare. One Slingshot. Perch the leg head if the apex is short and Reel up a Hang.

**8. Pick Stack.** X -56 to -44 by Y +8 to +20, three sections, top at 8 m, west end of Marker Flat. The Gantry's pick point. Its top section is live (Heavy, in-stack, break-free 500 N; it falls off the stack when pulled, it does not slide) and leaves on the magnet 40 s after "live" in Scrap, after which the stack is 5.4 m tall. A 2.7 m ledge on its east face (X -44 to -40). Rungs on the east face. Sightlines: the marker at 50 m, the west half of the Rails. Cover: the live top section, until it goes.

**9. The Canyons.** Seven mesas, NC1 (X -36 to -24, Y +14 to +26), NC2 (X -12 to 0, Y +16 to +26), NC3 (X +14 to +26, Y +6 to +18), SC1 (X -44 to -32, Y -27 to -15), SC2 (X -20 to -8, Y -27 to -15), SC3 (X +4 to +16, Y -9 to -3, one section high, 2.7 m), SC4 (X +22 to +34, Y -27 to -15). Tops at 8 m with 2.7 m and 5.4 m ledges stepping down on the ends shown as 1 and 2 on the Stacks plan. Gaps between mesas and between mesas and trenches are 6 to 14 m. Floor between: steel deck. Sightlines break every 10 m at Floor level; from a mesa top you see the neighbouring two tops and the Rails, and the Floor only at the gap you are standing over. Cover: everywhere at Floor level, nowhere on top. Edges: every mesa edge is a 2.7 to 8 m drop; the 8 m drop is a Stall onto steel (12 m/s), survivable onto a scrap heap (x2.0). Routes up: rungs on each mesa's marked end; a Breaker steps 2.7, 5.4, 8 on the stepped ends with a rung at each (mantle 1.8 m is short of 2.7 m). Climber: any Slingshot off the Floor. Scrapper (20 m line): self-dog, Dog B the Floor behind you, Jack at 14 m/s: up to 16 m of vertical gain, so 8 m is one hop with margin. Breaker by line: a 10 m/s launch gives about 9 m, enough for a mesa top only from directly below it; the rungs are quicker.

**10. East Pick Stack.** X +56 to +68 by Y +8 to +20, two sections, top at 5.4 m, east end of Marker Flat. The Gantry's pick point after Shift Change. Rungs on the east face. Sightlines: the press approach, the pit, Spawn B above and behind at 30 m.

**11. East Mesa (Spawn B).** X +56 to +80 by Y +33 to +45, top at 8 m, on the North Dock. Spawn B is the 10 m circle centered at (+68, +39, 8). Distance from the circle's center to the nearest pit edge (52, +7): 35.8 m straight; about 71 m along the shortest walkable route (west-face rungs 12 m, North Dock to grate 4 at X +70 14 m, across the grate 18 m, Marker Flat to the pit's north lip 27 m). Passes 05 section 9.3's walkable-route test with a 46 m margin. Contents: paint cans, a laundry bundle on a line (the shift's washing), a tool bag. Sightlines: the east half of Marker Flat, the pit from above, the north rail walk. Routes: rungs on the west face; cage ladder on the leg at (+60, +30), 17.5 s for a Breaker from the mesa top via the 8 m landing step; Climber Slingshot off the north trench floor as at Spawn A.

**12. Shack Perch.** The control shack (docs/objects/environment/office-gallery.md variant, **PROPOSED**), 8 m by 10 m, floor 10.5 m, roof 14 m, bolted to the west portal leg at (-60, +30) and standing on two rated structural columns at (-62, +35) and (-62, +41). Contents: the Marshals' gallery (non-doggable), the maintenance log evidence object (docs/objects/environment/evidence-objects.md), four floodlights on the roof edge lighting the west third of the Rails and Marker Flat. Sightlines: everything west of X = 0. Cover: the shack walls. Edges: the roof's south edge over the north trench (18 m drop to the rail bed). Routes: the leg's cage ladder has a landing at 10.5 m. A Climber on the roof reaches the Rails in one Slingshot: self-dog, Dog B the dock below, Jack, 26 m of gain against a 30 m reach. A Breaker who pulls a column drops the shack (events-and-moving-parts.md).

### High tier: the Rails

**13. North Rail Walk and South Rail Walk.** Two walkways, 160 m by 1.2 m, at 36 m, outboard of the gantry rails at Y = +31.5 and Y = -31.5, with a 1.0 m handrail on the outboard side only (**PROPOSED**). Floor: open grate. The rail head is inboard at 36 m; the Gantry's end trucks ride on it and pass a player on the walkway without contact. A player standing on the rail head is shoved off at the Gantry's speed (1.2 to 4.8 m/s, below the 4 m/s kinematic stagger except at double speed). Portal legs at X = -60, -20, +20, +60 each carry a cage ladder from the Floor with landings at 10.5 m (west leg north side only), 24 m, and 36 m. Sightlines: everything. Cover: the leg heads (2 m by 2 m boxes) and nothing else. Edges: 36 m on both sides. A fall from here onto any Floor surface is a Stall, onto a scrap heap it is still a Stall (26.6 m/s against a x2.0 threshold of 24 m/s).

**14. Mast Platforms.** Four, 4 m by 4 m, at 24 m, on the floodlight masts at (±52, ±40). Each carries three floodlights and a ladder from the dock. Sightlines: half the Yard each. Cover: the lamp housings. The platforms sit outside the Gantry's span (Y ±30), so the load never passes over them; the walkway is 16 m up and 10 m inboard. Routes: a Climber reaches the walkway from here in one Slingshot (16 m of gain). A Rigger (about 14 m of gain per Jack) cannot, and takes the cage ladder on the nearest leg for the last 16 m.

**15. The Bridge (the Gantry).** docs/objects/kinematics/gantry-crane.md. A 60 m bridge beam spanning Y -30 to +30 at 36 m (top surface walkable, 1.5 m wide), a trolley that runs along it, a hoist and electromagnet on a 6 m line during traverse, and a cab at the north end at 40 m. It moves along X on the 120 s cycle in events-and-moving-parts.md. Standing on the beam is free transport. The hoist cable is doggable; a player hanging from it climbs by Reel at 1.0 m/s (05 section 3.2). The load on the magnet is a rated attachment (05 section 6.7): kinematic until a Breaker's line exceeds its 1,000 N break-free, after which it falls from wherever the Gantry is. Sightlines: everything, moving. Edges: both sides, 36 m.

Two further zones noted for the Floor count: the **Press Approach** (the east lip, X +52 to +80 at Y -7 to +7, where the Center Hauler exits the pit stringers) is part of zone 6, and the **Alternate spawn** for a third crew or a contested drop is the top of NC3 (+20, +12, 8), 20 m from the pit edge, used only when both authored spawns are contested (**PROPOSED**; it breaks the 25 m rule and is the fallback of last resort).

## Spawns (05 section 9.3)

| Spawn | Center | Tier | Nearest Well edge | Alternate |
|---|---|---|---|---|
| A | (-68, -39, 8) on West Mesa | Mid | 110 m straight, about 140 m walkable | NC1 top (-30, +20, 8), 70 m from the pit |
| B | (+68, +39, 8) on East Mesa | Mid | 35.8 m straight, about 71 m walkable | NC3 top (+20, +12, 8) |

The spawns are at opposite ends of the mid tier, 155 m apart on the diagonal. A reserve drops at the alternate when an enemy is within 15 m of the crew's spawn.

## Claim marker (05 section 9.4)

Authored position (0, +12, 0) on Marker Flat, 4 m radius, a Weave billet on a pallet set on polished runners along X (friction 0.05, 05 section 6.1a) (docs/objects/heavy/claim-marker.md). A Rigger's line moves it at about 0.18 m/s², a Breaker's at about 0.62 m/s²; a 400 N line does not start it. It is 15 m from the center trench lip, 15 m from the north trench lip, 38 m from the pit, and 15 m from the Shift Change drop point (-14, +7) in Scrap. If moved into a trench it stays there and is contestable at -4 m; if it enters the pit it resets.

## Haul

| Item | Position |
|---|---|
| Spool start | (0, -12, 0), docs/objects/heavy/haul-spool.md, 2,000 kg in a roller cradle (friction 0.02, about 400 N to start) |
| Truck A (west crew) | (-75, -12, 0), bed facing east, ramp down |
| Truck B (east crew) | (+75, -12, 0), bed facing west, ramp down |
| Route | 75 m each way along the Haul Lane corridor Y -15 to -9. No trench crossing. The south trench lip is 12 m from the corridor edge; the center trench lip is 6 m. A spool that enters a trench is a Hauler's problem and is swept to the trench end plenum, then resets to start (**PROPOSED**). |

## The Well

Kill volume: X +38 to +52, Y -7 to +7, Z below -6 m. The ram at top of stroke reaches -6.5 m. The pit's full depth is -10 m. Hooks hatch in the north wall at -5 m. Scrap Card: "down the Well," Yard variant "pressed" (**PROPOSED**).

## Route check against 05 sections 3.2 and 3.4

Vertical reach per Jack is derived from 05 section 3.2: the line launches you at V, then keeps pushing at T until L reaches the class line length. Straight up with Dog B at your feet, reach is about V² / (2 (g - T/m)) while the push lasts and the line is long enough.

| Class | V (m/s) | T/m (m/s²) | Line (m) | Vertical gain per Jack |
|---|---|---|---|---|
| Climber | 18 | 4.4 | 50 | about 30 m |
| Doubler | 14 | 6.2 | 30 | about 27 m (line-limited) |
| Scrapper | 14 | 3.6 | 20 | about 16 m |
| Rigger | 12 | 4.6 | 40 | about 14 m |
| Breaker | 10 | 4.5 | 20 | about 9 m |

Ladder times use 05 section 2.3: 2.0 m/s, Breaker 1.6 m/s.

| Route | Gain | Breaker (honest) | Climber | Rigger | Scrapper / Doubler |
|---|---|---|---|---|---|
| Trench to Floor | 4 m | Rungs every 20 m, 2.5 s | One Jack off the far wall | One Jack | One Jack |
| Floor to mesa top | 8 m | Rungs, 5 s | One Jack | One Jack | One Jack |
| Mesa to shack roof | 6 m | Ladder landing at 10.5 m, 2 m mantle | One Jack | One Jack | One Jack |
| Mesa to mast platform | 16 m | Dock ladder, 10 s | One Jack | Jack to the shack roof, then one Jack (10 m) | Scrapper two Jacks via the shack roof; Doubler one |
| Mesa to Rails (any mesa) | 28 m | Cage ladder, 17.5 s | One Jack (2 m margin) | Ladder from the 24 m landing, 6 s | Doubler: one Jack from the shack roof (22 m), or ladder from 24 m; Scrapper: ladder from 24 m |
| Shack roof to Rails | 22 m | Cage ladder, 14 s | One Jack (8 m margin) | Ladder from 24 m | Doubler one Jack (5 m margin) |
| Floor to Rails | 36 m | Cage ladder, 22.5 s | Two Jacks (Floor to mesa, mesa to Rails) | Ladder, 18 s | Ladder, 18 s |

Map rule 3 is met: the Breaker has an honest route to every tier, and the Climber has a one-Slingshot route from each tier to the next (Floor to Stacks anywhere; Stacks to Rails from any mesa top). The Rails sit at 36 m (03-arenas.md agrees) so that this holds with 05's launch speeds.

## Grey-box findings (PROPOSED)

Building the grey-box level (`game/Tools/unreal/build_yard_greybox.py`) from this layout turned up places where its numbers collide. The level resolves them as below. Each resolution is **PROPOSED** until this document adopts it.

- **Grate footbridges.** The script places each grate at the layout's X, or the nearest X along its trench, 2 m at a time, where both landings and both stairs are clear of the mesas, rated sections, placeholders and the Haul corridor (Y -15 to -9). Most of the layout's X values put a landing inside a mesa that stands on the lip: SC2 at the south trench's X -10, the East Mesa along the whole north lip from X +56 to +80. The grates stand at:

| Trench | Layout X | Grey-box X |
|---|---|---|
| South (Y -30) | -50, -10, +30, +70 | -50, -2, +40, +76 |
| Center (Y 0) | -50, -10, +30, +70 | -50, -6, +34, +70 |
| North (Y +30) | -50, -10, +30, +70 | -50, -14, +28, +54 |

  A stair runs straight away from the trench where it can, and turns along the lip where it cannot. Spawn B's walkable route (zone 11) now crosses the north trench at X +54, not +70.
- **Ladder feet.** Each ladder keeps the floor at its foot clear, 1.5 m out and 2 m across, and the stairs are routed round it. The north trench's grate at X +54 had put its stair 0.8 m from the East Mesa's rung face, too close for a rigger to reach the rungs.
- **Hatches.** The mast platforms (24 m) and the portal landings (24 m, and 10.5 m on the shack's leg) stand over their own ladders, so a climber would hit their heads on them. Each has a hatch over the ladder: 1 m by 1.3 m in a mast platform, 0.9 m by 0.8 m in a landing.
- **Portal legs.** The layout puts the legs at (X, ±30), on the trench line ("the portal leg at (-60, -30)", "the trench floor at (-62, -30, -4)"). The legs on the lips would stand inside the West and East Mesas, NC1's ledge, SC2 and the control shack. The grey-box therefore puts each leg against a trench wall, from the trench floor up to a cross-head under the rail walk: Y ±27 to ±28.3 and ±31.7 to ±33. That leaves 28.4 to 31.6 clear for the 3.2 m Hauler. The cage ladder climbs the inner leg's east face from the lip, so it is still taken from the Floor. The leg heads stand beside the walk, over the dock side.
- **Control shack.** At Y +33 to +43, north of the leg it is bolted to. Its columns at Y +35 and +41 stand inside it.
- **Scrap heaps.** Three of the manifest's heap positions are inside a mesa or a ledge. The grey-box piles each heap against the mesa at the nearest clear spot: (+10, +18) becomes (+8, +18), (-8, +20) becomes (-14, +20), and (-40, +22) becomes (-42, +22).
- **Haul trucks.** The cab stands against the end wall, the bed inboard of it, then the ramp: the bed spans X -77.5 to -65.5 (Truck A) and +65.5 to +77.5 (Truck B). At the layout's (±75, -12), a 12 m truck would reach past the wall.
