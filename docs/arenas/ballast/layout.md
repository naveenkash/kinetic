# BALLAST: Layout

_Arena: Ballast. File purpose: tier-by-tier, zone-by-zone walk of the map with dimensions, routes per harness class, spawns, marker, Haul positions and the Well. Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Inventions are marked **PROPOSED**._

## Coordinate frame

- **Origin:** the Spine's centerline where it meets the Hold floor plane. Z = 0 is the Hold floor, the lowest playable floor.
- **X:** port negative, starboard positive. Hull interior spans X -18 to +18.
- **Y:** dorsal (sky side, the side HTR opened) negative, Belly (ground side, the nose side) positive. Hull interior spans local Y -15 to +15.
- **Z:** up.
- The hull axis leans 20 degrees off vertical toward +Y. All zone plans below are drawn in **hull-local section coordinates**. World Y of any feature = local Y + 0.364 x Z. Every original deck plate slopes down toward +Y at 20 degrees (14 to 26 degrees on the Breathing). Every prep-crew surface (catwalks, decks, ledges, stairs) is level at nominal pitch and tilts ± 6 degrees with the Breathing.
- Hull pitch is driven by docs/objects/kinematics/breathing-hull.md. Everything in the arena is a child of that root.

## Tier heights

| Z (m) | What |
|---|---|
| -10 | Top of the Well kill volume (docs/objects/environment/well-kill-volume.md) |
| 0 | Hold floor: the Wash (0.6 m water), the Sump (1.2 m water), the Spine Foot |
| 6 | Hold Ledge |
| 16 | Lower Bay decks, the Mouth Plate |
| 24 | Top of the third container in every stack |
| 26 | Upper Galleries, the Spine collar |
| 32 | Rack frame tops (fourth tier, empty) |
| 38 | Crown Net |
| 44 | Crown Deck, Spine head |
| 52 | Cut Gantry rails and beam |
| 56 | Cut face (sealed; vista above) |

## Slingshot reach by class (used for every route check below)

A Slingshot is a self-dog plus a Jack off a surface behind the direction of travel (05 section 10). The Kick (the 0.3 s launch phase) throws the player at the class launch speed V and the line then keeps pushing at the class tension T until the line reaches its class length (05 section 3.2). Launching straight up, the player decelerates at g minus T / m until they stop. Maximum vertical gain, line still pushing the whole way:

| Class | V (m/s) | T / m (m/s²) | Vertical reach (m) | Line length (m) |
|---|---|---|---|---|
| Climber | 18 | 4.4 | 30 | 50 |
| Doubler | 14 | 6.2 | 27 | 30 |
| Scrapper | 14 | 3.6 | 15.8 | 20 |
| Rigger | 12 | 4.6 | 13.8 | 40 |
| Breaker | 10 | 4.5 | 9.4 | 20 |

The Climber's Updraft kit is the 18 m/s launch speed itself (02), not a bonus on top of it. Every route below quotes the gain required against this table. Breakers always have a ladder, stair or ramp.

## Side elevation (looking from starboard; Y left to right, dorsal on the left)

```
Z 56  ===== cut face (sealed) =====               hull continues 338 m up the gorge wall
Z 52     [rail]======= CUT GANTRY beam =======[rail]
Z 44  Lip |net| CROWN DECK ...... Spine head ...... Belly wall   <- Spawn A, starboard Belly corner
Z 42  ---- top of Bay walls / top of Shear panel ----------
Z 32        rack frame tops (4th tier empty)
Z 26  UPPER GALLERY (dorsal) --- Spine collar ---           <- Spawn B, starboard gallery, dorsal end
Z 24        third container tops
Z 16  LOWER BAY DECK ===== MOUTH PLATE / Claim marker ===== gutter | Belly wall
Z 10        drive dog 1 ledge in the Spine
Z 6   HOLD LEDGE (dorsal)                                  <- bottom of Shear panel
Z 0   THE WASH 0.6 m ... Spine Foot ... THE SUMP 1.2 m -> torn nose rent
Z -10 ~~~~~~~~~~~~~~~~ THE WELL (whole gorge) ~~~~~~~~~~~~~~~~
      Hooks hover at Z -20, 20 m out from the rent and, after the Shear, 20 m off the port side.
      Not drawn: the 20 degree lean. World Y of every level shifts +0.364 m per metre of Z.
```

Scale note for every plan below: one character = 2 m in X, one row = 2 m in Y. Legend: `#` hull wall, `=` level prep-crew grating, `/` original sloped plating, `~` water, `[C]` container stack, `R` rack frame, `D` drive dog ledge, `F` floodlight stand, `T` water tank, `S` stairs, `L` ladder, `M` Claim marker, `A`/`B` spawns, `x` Well edge.

---

## TIER 1: THE HOLD (Z 0 to 6)

```
   PORT  -18 .. X .. +18  STBD        (local Y top = dorsal, bottom = Belly)
Y-15 ##################################
Y-13 #S== HOLD LEDGE (6 m) ==F==F==S#   drums chained at F, crane cradle removed
Y-11 ######====================######
Y- 9 #T  / /  /  / ######## /  /  / T#   ######## = Spine Foot, open on the Belly face
Y- 7 #T  THE WASH  ######## 0.6 m   T#
Y- 5 #  ~  ~   ~   ######## ~   ~   #
Y- 3 #  ~  scrap ~  ~  ~  ~  scrap ~ #
Y- 1 #  ~   ~   ~   ~   ~   ~   ~    #
Y+ 1 #  ~   crates  ~   ~  crates ~  #
Y+ 3 #  ~   ~   ~   ~   ~   ~   ~    #
Y+ 5 #  ~   ~   ~ THE SUMP 1.2 m ~   #
Y+ 7 #   ~   ~   ~   ~   ~   ~   ~   #
Y+ 9 ####   ~   ~   ~   ~   ~   ~  ###
Y+11    #xxxxx torn nose rent xxxxxx#     16 m wide, Z -2 to 4
Y+13     \   lip plating 25 deg   /
Y+15      \     THE WELL         /
```

### Zone 1: The Wash
- **Dimensions.** 36 m (X) by 20 m (Y, local -10 to +10), minus the 8 m square Spine Foot. Floor at Z 0 is crushed cargo compacted into a rough plain, under 0.6 m of standing seep water.
- **Floor material.** Steel plate and crushed container skin under water. Under 1 m deep, so the Stall multiplier is x1.0, not x3.0. Two scrap heaps (port and starboard, Y -3) are x2.0 surfaces, 6 m across.
- **Contents.** Four full water tanks bolted to the port and starboard walls (Y -9 to -5). Six steel crates, slid to the Belly side. Scrap, chain, planks, buckets. The black box (evidence object) half-sunk at X +6, Y +2.
- **Sightlines.** Long and low. You can see the whole Hold from any point; you cannot see out of it except up the Spine Foot and, after the Shear, out the port side. From the Bays you see the Wash only through the Belly gutter and the two slab windows per bay.
- **Routes out.** Stairs at both ends of the Hold Ledge (Zone 3). The Spine Foot ladder (Zone 4). Climber: self-dog, Dog B the Wash floor at your feet, Jack straight up the Belly gutter; 16 m gain against a 30 m reach, Perch on the deck edge to land. Doubler: same, 27 m reach. Scrapper (15.8 m) and Rigger (13.8 m): two hops, Wash to Hold Ledge (6 m), ledge to Lower deck (10 m). Breaker: stairs only.
- **Cover.** The tanks (2.4 m tall), the crates, the Spine Foot walls. Nothing on the Belly half.
- **Edges.** The torn nose rent along the Belly wall. The water force volume pushes toward it on every lean (events file).

### Zone 2: The Sump
- **Dimensions.** 16 m (X -8 to +8) by 6 m (Y +5 to +11), water 1.2 m deep. The only x3.0 surface on the map.
- **Floor.** Buckled plate, sloped 25 degrees toward the rent for its last 2 m (the lip plating).
- **Contents.** Floating buckets and a laundry-free drift of planks.
- **Why it matters.** A player who falls 36 m from the Upper Gallery lands at 26.6 m/s. On the Sump the Stall threshold is 36 m/s. It is the map's only safe long drop, and it sits 4 m from the Well edge, so every lean pushes you toward the rent while you wade at 3.2 m/s or less. The shove is authored on a 100 kg reference body (05 section 6.6), 4.8 m/s² at full lean: a Climber feels 5.3 m/s² and is off their feet, a Scrapper 4.4 (afloat above hull pitch 75.5), a Rigger or Doubler 3.7 (a skid), a Breaker 2.2.
- **Routes out.** Wade to the Wash (uphill against the lean), or Slingshot straight up the Belly gutter: self-dog, Dog B the Sump floor under the water, 16 m gain (Climber and Doubler only).
- **Edges.** The rent. Kill volume starts at Z -10 beyond it; the lip plating is 4 m of 25 degree slope first, so a player who slides off the Sump has 1.5 s before the Hooks.

### Zone 3: The Hold Ledge
- **Dimensions.** 36 m by 4 m (local Y -13 to -11), level grating at Z 6 along the dorsal wall. Stairs down to the Wash at both ends (X ± 16). A prep-crew stair tower (the Belly Stair, **PROPOSED**) rises from the starboard end of the ledge to the Starboard Lower Bay deck at Z 16: 10 m of rise, 20 m of run, two flights.
- **Contents.** Two floodlight stands, three chained fuel drums (generator fuel), a welding cart, tool bags. Condemnation notice: "Hold excluded from Cut Plan 7."
- **Sightlines.** Across the Wash to the rent. Up the Spine Foot opening. None to the Bays.
- **Routes.** Breaker honest route to the Bays: the Belly Stair. Everyone else: Slingshot (self-dog, Dog B the ledge grating, gain 10 m: every class except the Breaker).
- **Edges.** After the Shear, the port end of the ledge (X -18) is a Well edge: the wall panel from Z 6 up is gone.

### Zone 4: The Spine Foot
- **Dimensions.** 8 m by 8 m, Z 0 to 8, the bottom of the Spine shaft. The Belly face of the shaft is torn open onto the Wash from Z 0 to 8. The floor is the crushed-cargo plug.
- **Contents.** The dorsal ladder run starts here (docs/objects/structures/spine-ladder.md) and runs unbroken to Z 44. The Spine ratchet rail (arena-unique, see prop-manifest.md) starts here. One floodlight stand aimed up the shaft.
- **Sightlines.** 44 m straight up to the square of sky at the Spine head. Anyone on the dorsal ladder is visible from the Foot and the Mouth Plate.
- **Danger.** A drive dog Hauled free anywhere above comes to rest here. 40 t into an 8 m square room. Nothing in the Foot survives it.

---

## TIER 2: THE BAYS (Z 16 to 32)

Two cargo bays, port and starboard, each 14 m wide (X) and 30 m deep (Y), separated by the Spine. Each bay holds two magnetic racks bolted to the original sloped deck (the Slab). Each rack carries two stacks of three containers. The racks lean with the Slab, so every stack leans 20 degrees toward the Belly, held at its foot by the rack's end-stop. Prep crews built a level steel deck at Z 16 across the bay and a 2 m catwalk gallery at Z 26 along the dorsal wall.

### Lower Bay plan (Z 16)

```
   PORT  -18 .. X .. +18  STBD
Y-15 ##################################
Y-13 #S [CCCCCCC] ##D16##  [CCCCCCC] S#   dorsal racks: 2 stacks each, lean toward Belly
Y-11 #  [CCCCCCC] ######## [CCCCCCC]  #   D16 = starboard drive dog ledge at Z 18 in the shaft
Y- 9 #R         R ######## R         R#
Y- 7 #= = slab window = = ####  = = slab window = =#
Y- 5 #= = = = = = = = = ########= = = = = = = = = #
Y- 3 #===== LOWER DECK =MMMMMMMM= LOWER DECK ======#   MMMMMMMM = Mouth Plate, 8 x 8 m
Y- 1 #===== crates F ==MMMMMMMM== F crates =======#   cross-catwalk runs Y -4 to +4 through it
Y+ 1 # [CCCCCCC]  R ==MMMMMMMM== R  [CCCCCCC]    #   Belly racks
Y+ 3 # [CCCCCCC]  R ==========  R  [CCCCCCC]    #
Y+ 5 #R          R ============ R          R    #
Y+ 7 #= = = = = = = = = = = = = = = = = = = = = #
Y+ 9 #= = = = = = = = = = = = = = = = = = = = = #
Y+11 #g g g g g g g g BELLY GUTTER g g g g g g g #   3 m open slot, 16 m drop to the Wash
Y+13 ##################################
```

### Zone 5: Port Lower Bay
- **Dimensions.** 14 m (X -18 to -4) by 26 m (Y -13 to +13) of level grating at Z 16, minus two rack footprints (each 13 m by 6 m) and two slab windows (each 4 m by 6 m, at Y -7 and Y +8, where the grating is left out and the 20 degree Slab shows through, feeding the gutter). The Belly gutter is a 3 m open slot at Y +11 to +13.
- **Floor.** Steel grating, x1.0. Slab windows are original plate, x1.0, and slide loose Medium props 24 s of every 90.
- **Contents.** Two racks with twelve containers. Four loose steel crates (spilled contents). A cart-trolley. A scaffold-board stack. Hanging chains from the rack frame tops. Four demolition charges on the port wall ribs (the Shear cut, HTR's; see events file for what hitting one does). Two floodlight stands.
- **Sightlines.** Broken every 6 m by stacks. Clear lanes: the cross-catwalk (Y -4 to +4) sees port wall to starboard wall through the Mouth Plate. The gutter line (Y +10) sees the whole Belly side. Up to the galleries through the rack frames.
- **Routes.** To the Starboard Lower Bay: across the Mouth Plate (Zone 7). To the Upper Gallery: port stair at the dorsal corner (X -17, Y -13), 10 m rise, one switchback; or mantle up the stack faces (containers are 2.6 m tall, mantle heights 1.8 to 2.6 m: only a Climber mantles a container; everyone else uses the rack ladders). To the Hold: the Belly Stair on the starboard side only, or a 16 m drop into the gutter (Stall for everyone; the Wash is x1.0).
- **Cover.** Container stacks. The rack frame uprights (0.3 m steel, stops a Dog).
- **Edges.** The gutter. After the Shear: the outer 4 m strip of this deck (X -18 to -14) goes with the wall, and the new deck edge at X -14 is a Well edge, 100 m over the gorge.

### Zone 6: Starboard Lower Bay
Mirror of Zone 5 with three differences. The Belly Stair arrives here from the Hold Ledge (X +17, Y +9). There are no demolition charges on the starboard wall. The wall survives the Shear, so this is where fights go after 1:15.

### Zone 7: The Mouth Plate
- **Dimensions.** 8 m by 8 m grated platform at Z 16 spanning the Spine shaft, carrying the port-starboard cross-catwalk (4 m wide). Fractures at 2,500 N·s like any catwalk section (docs/objects/structures/catwalk-section.md). The ratchet rail passes through it on the dorsal face of the shaft.
- **Contents.** The Claim marker, authored at (0, 0, 16), dead center. The Haul spool's start sled, dorsal edge. Two hanging drive chains (arena-unique) from the Spine head, 18 m long, swinging to within 2 m of the plate.
- **Sightlines.** Down the shaft 16 m to the Foot. Up 28 m to the Spine head square. Both Lower Bays along the cross-catwalk. This is the most watched 64 m² on the map.
- **Danger.** Any drive dog released above Z 16 comes through this platform. The starboard dog at Z 18 is 2 m above it; a player on the Plate can Dog that piston's end from where they stand.
- **Routes.** Climber Ascension: self-dog, Dog B the Plate grating, Jack straight up the shaft: 28 m gain against a 30 m reach, landing arrested by a Perch on the Spine head kerb. On the lean the 6 degree gravity offset drifts the flight 3 m toward the Belly face; aim dorsal. A Doubler reaches 27 m and lands on the Z 42 dog ledge, 2 m short of the deck. Scrapper and Rigger reach the Spine collar at Z 26 (10 m gain). Breaker: ladder.

### Upper Bay plan (Z 26)

```
   PORT  -18 .. X .. +18  STBD
Y-15 ##################################
Y-13 #S========= UPPER GALLERY ========S#   2 m catwalk along the dorsal wall, both bays
Y-11 #=  [stack tops Z 24]  =D26=  [stack tops]  =#  Spine collar ring, port dog ledge at Z 26
Y- 9 #=                      =====                 =#
Y- 7 #R   open to Z 16       ==H==   open to Z 16 R#   H = hatches onto the collar
Y- 5 #                       =====                  #
Y- 3 #                       (shaft)                #
Y- 1 #                       (shaft)       Spawn B at the dorsal end, X +14 #
Y+ 1 #   [Belly stack tops Z 24]   [Belly stack tops]#
Y+ 5 #R                                            R#
Y+11 #        (gutter below, Z 16)                  #
Y+13 ##################################
```

### Zone 8: Port Upper Gallery
- **Dimensions.** 2 m wide catwalk at Z 26 along the port bay's dorsal wall, 14 m long (X -18 to -4), joined to the Spine collar (Zone 11) by a hatch and to the Lower Bay by the port stair. A 1 m hand cable, no rail.
- **Floor.** Grating, x1.0. Hung from the port wall on brackets.
- **Contents.** One floodlight stand, two stacked steel crates at the stair head, paint cans, a LayCast sponsor banner.
- **Sightlines.** Down onto the dorsal stack tops (2 m below) and across them to the Belly racks. Into the Spine through the collar. Across the shaft to the starboard gallery at the same height: a 20 m Clothesline lane between galleries.
- **Routes.** Onto the stack tops: a 2 m drop. To the Crown: the Spine collar then the dorsal ladder, 18 m of ladder (the Breaker's honest route). Climber and Doubler: self-dog, Dog B the gallery grating, Jack: 18 m gain to the Crown deck. Scrapper and Rigger: Jack to the Z 34 dog ledge (8 m) then to the Crown (10 m).
- **Edges.** Both long edges are a 10 m drop onto grating (Stall). After the Shear this gallery no longer exists; it falls with the wall.

### Zone 9: Starboard Upper Gallery
Mirror of Zone 8. **Spawn B** occupies its dorsal end (centre X +14, Y -12, Z 26, 10 m across along the gallery and the stair landing). The gallery survives the Shear.

### Zone 10: The Rack Tops
- **Dimensions.** Four stack tops per bay, each top 12.2 m by 2.4 m at Z 24, leaning 20 degrees toward the Belly. The rack frames rise another 8 m to Z 32 around them as open steel (the lifted-out fourth tier).
- **Floor.** Container roof, x1.0, sloped 20 degrees. A player standing still on a 26 degree roof at full lean slides (**PROPOSED**: player slide onset at 24 degrees, matching props). Climbers perch on the frame.
- **Contents.** Lifting chains left hooked to the frame tops, swinging. Clamp crates. One HTR cutting torch on the port dorsal rack top, inert.
- **Sightlines.** The best in the Bays. From a rack top you see both galleries, the Mouth Plate, and the Crown underside.
- **Routes.** Frame ladders down to the Lower deck. Frame tops at Z 32 are 12 m under the Crown deck: Climber, Doubler, Scrapper (15.8 m) and Rigger (13.8 m) can all Jack from a frame top through the open Crown underside to the deck. A Breaker (9.4 m) cannot; the Spine ladder remains the Breaker's honest route.
- **Edges.** Every stack top edge is an 8 m drop to grating (Stall at 12.5 m/s). This is where the Avalanche starts: the bottom container of any stack, Hauled above its break-free, takes the two above it down the Slab and into the gutter.

---

## TIER 3: THE CROWN (Z 38 to 52)

```
   PORT  -18 .. X .. +18  STBD
Y-19     n n n n n CROWN NET (Z 38, 8 m out) n n n n n
Y-15  xxxxxxxxxxxxxxxx CROWN LIP (2 m bulwark) xxxxxxxxxxxx   open to sky beyond
Y-13 |[col]====================================[col]|  gantry columns (rated), rails at Z 52 above
Y-11 |= = = = original drive-head plating 20 deg = = =|
Y- 9 |= cyl rack  manifold  = = = = = = = drums  = = =|
Y- 7 |= = = = = = = = = =######## = = = = = = = = = =|
Y- 5 |===== CROWN DECK ===######## grating ==========|  ######## = Spine head opening, 8 x 8 m
Y- 3 |= port cradle ======######## ====== stbd cradle |  Haul trucks, under the gantry beam ends
Y- 1 |====================########====================|
Y+ 1 |= crates = carts ===========  welding carts ====|
Y+ 5 |[col]==================================== [col]|
Y+ 9 |= scaffold-board stack = = = = = = = = = = = AAAA|  Spawn A: starboard Belly corner
Y+13 #########################################AAAA#
```

### Zone 12: The Crown Deck
- **Dimensions.** 36 m (X) by 30 m (Y) at Z 44. The Belly half (Y 0 to +13) is level prep-crew grating. The dorsal half (Y -13 to 0) is original drive-head plating at 20 degrees, sloping toward the deck centre (toward +Y, so loose props on it slide into the grating, not off the Lip). The Spine head is an 8 m square opening at the centre with a 1 m kerb. The dorsal edge (the Crown Lip, Y -15) has a 2 m bulwark of original plating; beyond it is sky and a 44 m drop to the gorge. The hull walls stand on port, starboard and Belly sides up to the cut face at Z 56; the roof is gone.
- **Floor.** Grating x1.0 and steel plate x1.0.
- **Contents.** A cylinder rack with six propane cylinders (HTR's cutting gas) and its gas manifold at the port dorsal corner. Two chained fuel drums starboard dorsal. Two welding carts, four steel crates, a cart-trolley, a hand truck, a scaffold-board stack. The two Haul cradles under the gantry beam ends at X ± 10, Y -3. Sponsor banners on the gantry columns. Condemnation notice on the Spine head kerb: "Cut Plan 7, Shear B, 16:40."
- **Sightlines.** Everything on the deck is visible from everywhere on it except behind the cylinder rack and the cradles. Down the Spine head: the Mouth Plate 28 m below. Over the Lip: the gorge, the Office gallery on the far wall, the Hooks.
- **Routes.** Down: the Spine dorsal ladder (Breaker's honest route, 18 m to the collar). Up to the gantry: a leg ladder on each column, 8 m. Climber: Jack from the deck to the beam. Over the Lip into the Crown Net: a 6 m drop onto netting (x2.0), ladders back up at X ± 12.
- **Cover.** The cylinder rack (which is also a bomb), the cradles, the Spine head kerb, the columns.
- **Edges.** The Spine head (28 m drop to the Plate, Stall). The Lip (net below, then air). After the Shear, the port deck edge above the sheared bay wall is a Well edge: the deck stays, the wall below it is gone.
- **Spawn A** centre: X +14, Y +10, Z 44, 10 m across. Measured along walkable routes (05 section 9.3): 32 m to the Lip around the Spine head kerb, 36 m along the Belly side to the post-Shear port edge, over 80 m to the nose rent by ladder and stair. Alternate spawn (contested within 15 m): the Starboard Upper Gallery, Belly end of the stair landing (X +17, Y -8, Z 26) **PROPOSED**. Alternate for Spawn B: Starboard Lower Bay, dorsal end (X +12, Y -11, Z 16) **PROPOSED**.

### Zone 13: The Crown Net
- **Dimensions.** 36 m by 8 m, Z 38, slung outside the Lip from the hull's dorsal frame to two outriggers. Arena-unique (prop-manifest.md). Soft surface x2.0.
- **Purpose.** HTR's debris net for cut material. For players it turns the Lip from an Overboard edge into a 6 m soft drop, which keeps the Crown's ring-out share honest. A Jack that clears the net's outer edge (8 m out) is still a Well kill.
- **Routes.** Two ladders back to the deck. Climber: Jack off the net frame.

### Zone 14: The Cut Gantry
- **Dimensions.** Two 1 m wide rails along the port and starboard walls at Z 52, 30 m long; a 36 m box beam between them at Y -6, carrying the torch carriage and the hook. Static for the match (arena-unique).
- **Contents.** The torch carriage (inert), two hanging lifting chains, a cable reel.
- **Sightlines.** The whole Crown deck, the Spine head square, the gorge.
- **Routes.** Column ladders (8 m; every class except the Breaker can also Jack up from the deck). Climbers perch on the beam (1.5 s) and re-dog. A Pendulum from the beam swings out over the Lip; on the recovery half of the Breathing the arc's rest point shifts 6 degrees toward the Lip, 2.1 m on a 20 m line.
- **Edges.** All of them. A fall from the beam to the deck is 8 m: Stall.

---

## THE SPINE (connector, Z 0 to 44)

### Zone 11: The Spine
- **Dimensions.** 8 m square shaft along the hull axis, leaning 20 degrees toward +Y. The dorsal face (the one that leans back, at 70 degrees) carries the full-length ladder run and the ratchet rail. The Belly face overhangs. The port and starboard faces carry the drive dog housings.
- **Drive dogs.** Six 40 t pistons (docs/objects/rated/drive-dog-piston.md), each 3 m in diameter, protruding 2 m into the shaft as a ledge. **PROPOSED** spacing 8 m: Z 10 port, 18 starboard, 26 port, 34 starboard, 42 port, 50 starboard. The Z 26 piston's end is the port platform of the Spine collar. The Z 42 piston is a ledge 2 m under the Crown deck. The Z 50 piston sits in the drive head above the deck, reachable from the gantry.
- **The Spine collar (Z 26).** A grated ring 1.5 m wide around the shaft, with hatches onto both Upper Galleries.
- **Openings.** Foot (Z 0 to 8, Belly face). Mouth Plate (Z 16, all faces, cross-catwalk). Collar hatches (Z 26, port and starboard). Head (Z 44, open).
- **Sightlines.** Straight up and down. Anyone on the ladder is a silhouette against the sky square or the Foot's floodlight.
- **Routes.** Breaker honest route for the whole map: Foot to Plate 16 m of ladder, Plate to collar 10 m, collar to head 18 m. Ladders run at 2.0 m/s, 1.6 m/s for a Breaker (v0.2.2): Foot to Crown in 22 s, 27.5 s for a Breaker. A player hanging in the shaft on a Hang line climbs or lowers by Reel at 1.0 m/s (05 section 3.2), which is the only way up the overhanging Belly face. Climber: one Jack from the Plate to the head (28 m of a 30 m reach). Doubler: Plate to the Z 42 ledge (26 m), then a 2 m mantle (Doubler mantle height 2.2 m). Scrapper and Rigger: Plate to collar (10 m), collar to the Z 34 ledge (8 m), ledge to head (10 m). Breaker: ladder, or hops of 8 m or less between dog ledges with the Breaker's 9.4 m reach.
- **The Draw lane.** A Hang line across the shaft between the Z 26 and Z 34 dog ledges (8 m across, 8 m up: 11.3 m line) sits exactly where a Climber Ascension passes.

---

## Spawns (05 section 9.3)

| Spawn | Centre (X, Y, Z) | Tier | Nearest Well edge, walkable route | Notes |
|---|---|---|---|---|
| A | +14, +10, 44 | Crown Deck, starboard Belly corner | 32 m (the Lip, around the Spine head kerb); 36 m (post-Shear port edge) | Drop animation: lowered by the gantry hook |
| B | +14, -12, 26 | Starboard Upper Gallery, dorsal end | Pre-Shear over 70 m (the nose rent, by stair and Belly Stair); post-Shear 26 m (the collar's port hatch stub: 10 m of gallery to the starboard hatch, 16 m around the collar ring) | Drop animation: sliding a line down from the Crown |
| Alt A | +17, -8, 26 | Starboard Upper Gallery, stair landing | 29 m (collar port hatch stub) | **PROPOSED** |
| Alt B | +12, -11, 16 | Starboard Lower Bay, dorsal end | 30 m (post-Shear port deck edge, across the Mouth Plate) | **PROPOSED** |

The two spawns are 28 m apart in 3D and 40 m apart by the shortest honest route (ladder plus gallery). They are on the same side of the hull because the Shear removes the port side; **PROPOSED** that crews swap spawns every Scrap round so the Crown advantage (drive dogs) and the Bay advantage (marker proximity) alternate.

## Claim marker (05 section 9.4)

Authored at (0, 0, 16): the centre of the Mouth Plate, directly over the shaft. If the Plate is destroyed and the marker falls, it enters the Well only if it leaves the hull; a marker that lands at the Spine Foot stays legal there. If it does enter the Well it resets to the authored position; if the Plate is gone at that moment, **PROPOSED** fallback position is the Starboard Lower Bay deck at (+8, -2, 16).

## Haul (05 section 9.1)

- **Spool start.** docs/objects/heavy/haul-spool.md, 2,000 kg, on the ratchet rail's sled at the Mouth Plate's dorsal edge (0, -4, 16).
- **Rail.** The Spine ratchet rail (arena-unique, prop-manifest.md) runs up the dorsal face from Z 0 to Z 44. One-way pawls every 0.5 m: the spool holds whatever height it has gained. Its hidden counterbalance matches the loaded sled exactly, so the only resistance is 200 N of pawl drag (a racked break-free) and the 4,000 kg of moving mass. Any class moves it. Acceleration = (sum of line tensions minus 200 N) / 4,000 kg, capped at 3 m/s: a solo Breaker 0.2 m/s² (28 m in 17 s), a solo Climber 0.05 m/s² (33 s), a whole crew pulling together up to 0.7 m/s². It cannot be pulled down.
- **Trucks.** **PROPOSED** both cradles are on the Crown Deck under the gantry beam ends: port cradle (-10, -3, 44) and starboard cradle (+10, -3, 44). Crews swap cradles between rounds. The 28 m of rail is shared; the last 6 m from the Spine head to a cradle is open deck, where the 2,000 kg spool rolls on grating at T/m minus rolling drag (a Breaker gives it about 0.4 m/s², 6 m in 5.5 s) and the tug of war decides it. If the spool is still on the rail at time-out, "furthest" credit goes to the crew whose line last moved it.
- The bible's trucks (a Crown hook and a Hold cradle) were replaced because the rail only ratchets upward; this is flagged for review.

## The Well (05 section 7.5)

- **Primary.** The torn nose rent at the Belly end of the Hold (X -8 to +8, Z -2 to 4), then 4 m of lip plating, then air. Kill volume: docs/objects/environment/well-kill-volume.md, top face at Z -10, extending across the full gorge floor plan (120 m by 200 m) and up the gorge outside the hull to Z 60, so anything that leaves the hull through any opening is caught.
- **After the Shear.** The hull's port face from Z 6 to 42 is open. The kill volume already covers it; no new volume is needed, only new edge markers on the Hold Ledge, both Lower Bays, the collar hatch and the Crown deck's port edge.
- **Hooks.** Two drones hover at Z -20, 20 m out from the rent. After the Shear, two more take station 20 m off the port side at Z 20. Rotors audible in the Hold at all times, in the Bays after the Shear.
