# HANGTOWN: Layout

_Arena: Hangtown (Vilca's Hang). File purpose: tier-by-tier, zone-by-zone walk with dimensions, routes per harness class, sightlines, cover, edges, spawns, Claim marker, and Well extent._
_Sources: docs/bible/03-arenas.md (Hangtown, map rules), docs/bible/05-core-mechanics.md (2.2 movement, 3.4 class tuning, 7.3 surfaces, 9.3 spawns, 9.4 marker). Everything not in those files is **PROPOSED**._

## Coordinate frame

- Origin: the vertical axis of the Market Ring. X positive east (toward the East Rim), Y positive north (along the ravine), Z up.
- Z = 0 is the lowest playable floor: the Drip Nets. All heights below are meters above the nets.
- Playable box: X -65 to +65, Y -35 to +35, Z -12 (kill) to +38.
- Solid ground exists only at |X| ≥ 50 (the two Collar rims). Everything between hangs on cable.

## Height summary

| Z (m) | What is there |
|---|---|
| 38 | Lift headframe top platform, counterweight jib |
| 32 | West Rim deck (Spawn A), East Rim deck (Spawn B) |
| 30 | The Hub, the four High Lines (walkways on the main cables), Lift upper landing, headframe ramp foot |
| 19 | House roof ridges (small 18.5, large 19) |
| 16 | Market Ring deck, Claim marker |
| 15 | Lift Gate landing |
| 14 | House floors, West Row and East Row |
| 4 to 6 | Garden Terraces, Low Shanties, counterweight bottom of travel (2) |
| 2 | Lift bottom landing, Spine ladder foot landing, West Face plank bridge |
| 0 | Drip Nets (sag to -1 at center) |
| -4 | Top of the ravine mist |
| -12 | Well kill volume |
| -60 | Ravine floor (never reached) |

## Cable structure (read this first)

Four **main load cables** (docs/objects/environment/load-cable.md) run from four **anchor pins** (docs/objects/rated/anchor-pin.md, rated) in the rim faces to **the Hub**, a steel splice collar at (0, 0, 30). Pins: NW (-50, +12, 31), SW (-50, -12, 31), NE (+50, +12, 31), SE (+50, -12, 31). Each cable is 51.5 m long. A **High Line** walkway (docs/objects/structures/walkway-on-cable.md) hangs 0.4 m under each main cable.

The **Market Ring** (docs/objects/structures/market-platform.md) hangs 14 m below the Hub on four hanger cables (friction 0, a pendulum, per 05 section 6.1a).

**Hitch geometry (single cable per house).** Each house hangs on one hitch cable. At the house end is the **rated hanger eye** (1,000 N, yellow-and-black band): small houses ridge Z 18.5, eye Z 19.0; large houses ridge Z 19.0, eye Z 19.5; Low Shanties ridge Z 8.5, eye Z 9.0. The orange **slack-cable segment** (docs/objects/environment/slack-cable.md), 1.5 m long, sits 3.5 m above the eye: small houses Z 22.5 to 24.0, large Z 23.0 to 24.5, shanties Z 12.5 to 14.0. A player standing on the ridge reaches about 3.6 m above it (raised hand plus 1.5 m Cutter), 0.4 m short of the segment, so the eye and the segment are never confused at a glance and cutting means leaving the roof: a Climber Perches the hitch (1.5 s covers the 0.6 s hold), or any class Floor Slingshots off the roof and fires Hang to the main cable at the apex, hanging beside the segment. The Office prep crew pre-rigged a second, slack **catch line** from each non-condemned house's eye to its main cable, 12 m longer than the hitch. Cut the slack segment and the house falls until the catch goes taut, one tier down, or into the Well if the house is condemned and has no catch (**PROPOSED** catch model; it realizes 03's "drops a tier, or into the Well"). Only a Breaker's line, a Doubler at overspin, or a cut slack cable drops a house; a Hang line to a house holds without unseating it (05 section 6.2).

Four **Low Hang cross cables** anchored in the rim faces at Z 16 (Y = ±15 and Y = ±22) carry the Drip Nets (on 16 m edge ropes), the Garden Terraces (on 10 to 12 m hitches), and the two Low Shanties.

## ASCII plans

Scale: one character = 5 m in X and 5 m in Y. Columns run X -65 (left) to +65 (right). Rows run Y +35 (top) to -35 (bottom). Diagonal cables are stepped.

### Upper tier (Z 30 to 38)

```
         X -65 -50       -25         0         +25       +50 +65
 Y +35   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y +30   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y +25   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y +20   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y +15   RRp = = = = . . . . . . . . . . . . = = = = pRR     p = anchor pin, = = High Line
 Y +10   RRR . . . . = = = = . . . . = = = = . . . . RRR
 Y  +5   RRR . . . . . . . . = = . . = = . . . . . . RRR
 Y   0   RAR . . . . . . . . . . H . c . . L - - - . RBR     H = Hub, c = counterweight jib
 Y  -5   RRR . . . . . . . . = = . . = = . . . . . . RRR     L = headframe, - - - = ramp
 Y -10   RRR . . . . = = = = . . . . = = = = . . . . RRR     A = Spawn A, B = Spawn B
 Y -15   RRp = = = = . . . . . . . . . . . . = = = = pRR
 Y -20   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y -25   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y -30   RRR . . . . . . . . . . . . . . . . . . . . RRR
 Y -35   RRR . . . . . . . . . . . . . . . . . . . . RRR
```

### Mid tier (Z 14 to 19)

```
         X -65 -50       -25         0         +25       +50 +65
 Y +15   . . . . . . . . . . . . . . . . . . . . . . . . . .
 Y +10   . . . . W3~ ~W2 ~ . O O . . . ~E2~ ~ E3 . . . . . .     W/E = houses, ~ = swing bridge
 Y  +5   . . . . . . . . ~W1~ O O O O ~E1 . . . . . . . . . .     O = Market Ring deck
 Y   0   . . . . . . . . . | O O M O c | L L . . . . . . . .     M = Claim marker, c = counterweight
 Y  -5   . . . . . . . . ~W4~ O O O O ~E4 . . . . . . . . . .     L L = Lift car and Gate landing
 Y -10   . . . . W6~ ~W5 ~ . O O . . . ~E5~ ~ E6 . . . . . .     | = cross plank
 Y -15   . . . . . . . . . . . . . . . . . . . . . . . . . .
```

### Low tier (Z 0 to 6)

```
         X -65 -50       -25         0         +25       +50 +65
 Y +25   . . . . . . . . . . . . . . . . . . . . . . . . . .
 Y +20   . . . . . . . T T T L1T T T T T T T T . . . . . . .     T = Garden Terrace (Z 4 to 6)
 Y +15   . . . . . . . # # # # # # # # # # # # . . . . . . .     # = Drip Nets (Z 0)
 Y +10   . . . . . . . # # # # # # # # # # # # . . . . . . .     L1 L2 = Low Shanties
 Y  +5   . . . . . . . # # # # # # # # # # # # . . . . . . .     l = Spine ladder foot
 Y   0   . . . . . . . # # # # # # # # c # # K . . . . . . .     - - - = West Face plank bridge
 Y  -5   . . . . . . . # # # # # # # # # # # # . . . . . . .     c = counterweight sole plate
 Y -10   . . . . . . . # # # # # # # # # # # # . . . . . . .     K = Lift bottom landing (Z 2)
 Y -15   . . . . . . . # # # # # # # # # # # # . . . . . . .
 Y -20   . . . . . . . T T T T T T T T L2T T T . . . . . . .
 Y -25   . . . l - - - . . . . . . . . . . . . . . . . . . .     ladder foot and bridge at Y -24
```

### Side elevation (looking north, X left to right, Z up)

```
 Z +40                                       T               T = headframe top (38)
 Z +35                                       |
 Z +30   RRR = = = = = = = = = H = = = = = L = = RRR          rims 32, cables 30
 Z +25   RRR |   |   |         /    :      |     RRR          | = slack cable hitches
 Z +20   RRR ^   ^   ^         /    :      |     RRR          ^ = roofs 19, / = Hub ladder
 Z +15   RRR W3  W2  W1  O O O M O  c  E1 E2 E3  RRR          O = ring 16, c = counterweight
 Z +10   RRR                     |  :      |     RRR          : = counterweight guide
 Z  +5   RRR     T T T T T T T T T T T T T T     RRR          T = terraces 4 to 6
 Z   0   RRl - - # # # # # # # # # # # # # # K   RRR          # = nets 0, K = Lift landing 2
 Z  -5   RRR ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ RRR          ~ = mist (top at -4)
 Z -10   RRR ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ RRR
 Z -12   RRR x x x x x x x x x x x x x x x x x x RRR          x = Well kill volume
```

## Zone walk

Routes use these checks (05 sections 3.2 and 3.4, v0.2.2). A Haul cannot lift a player: every class's pull tension (400 to 1,000 N) is under its own weight (883 to 2,158 N), except a Breaker's 1,000 N on a 90 kg Climber. Going up means a Jack launch, a Pendulum, a Reel, or an honest route. **Reel:** in Hang, holding Haul shortens the rod at 1.0 m/s and holding Jack lengthens it, bearing the class Hang load. Any hanging player climbs at 1 m/s: a Breaker on the ring deck Hangs to a main cable 14 m up (inside its 20 m line) and Reels to the High Line in 14 s, exposed the whole way; a player who lands on a set line (solid to players) fires their own Hang and Reels. Ladders are climbed at 2.0 m/s, 1.6 m/s for a Breaker (05 section 2.3). **Floor Slingshot:** self-dog A, Dog B the floor at your feet, Jack. A vertical launch at the class speed V, pushed at T until the line reaches class length. Vertical gain: Climber 30 m, Doubler 27 m, Scrapper 15.8 m, Rigger 13.8 m (plus a 2.2 m mantle), Breaker 9.4 m. **Rim Pendulum:** self-dog A, Dog B a main cable 25 m out, Hang, step off the lip. A 25 m rod swings you to the Hub in 5 s, arriving at 0 m/s (Climber, Rigger, Doubler; the 20 m lines of the Scrapper and Breaker do not reach the pivot). Reel in during the last 2 s of the swing and you arrive 2 m above the rail instead of level with it; a swinger pushed off their arc stays on the rod and Reels up to the cable rather than releasing. A Climber's ground Slingshot at 45° travels 33 m (the bible's figure); every other class lands inside 20 m. Breaker lines are 20 m; any Breaker route using a line names an anchor within 20 m. Honest routes are ladders, ramps, the Lift, and walkways.

### Upper tier

**Z1. West Rim (Spawn A).** X -65 to -50, Y -35 to +35, Z 32. Concrete Collar rim (docs/objects/structures/collar-rim-ground.md), 15 m deep, scarred by sixty years of cable anchors. A 1.2 m parapet runs the whole lip. The two High Line gates (Y ±12) are reached only by **gate runs** (**PROPOSED**): 12 m railed walks along the inside of the parapet, entered from the rim deck at Y ±24 and ending at the gate. The Spine ladder head is at (-50, -24, 32), at the SW run's entrance. Spawn A center (-58, 0, 32), 10 m across. Floodlight mast at (-60, +20, 32). Office prep tent, two anchor pins in the face below the gates. Sightlines: everything. The Hub is 58 m away, the Market Ring 14 m below the eye line. Cover: the parapet (crouch), the mast base, a stack of scaffold boards. Edges: the lip, 32 m above the nets at X -30 or the Well at X -50 (nothing below the lip but mist). Routes out: Breaker walks a gate run and a High Line (62 m, 12 s at run speed) or climbs down the Spine ladder (30 m at 1.6 m/s, 19 s; 15 s for everyone else). Climber, Rigger, and Doubler take the Rim Pendulum: Dog B the NW or SW main cable 25 m out, Hang, step off at the gate, arrive at the Hub rail 5 s later with no speed to lose (rod tension at the bottom is three times body weight: 2.6 kN for a Climber, 3.8 kN for a Rigger or Doubler, all under their Hang loads). A Climber can instead Slingshot off the parapet at 45° (33 m), air-dog the Hub at the apex, and Hang to swing in. Scrapper walks; 20 m reaches nothing useful from the lip.

**Z2. East Rim (Spawn B).** Mirror of Z1 at X +50 to +65. Spawn B center (+58, 0, 32). Gates at Y ±12 and the headframe ramp gate at Y 0. Floodlight mast at (+60, -20, 32). Routes out: Breaker takes the headframe ramp (17 m, down 2 m, railed both sides) to the Lift upper landing and rides the Lift down (18 s) or walks a gate run and the NE or SE High Line. The Lift car is 20 m from the lip: a Breaker Dog reaches it.

**Z3. The High Lines.** Four walkways, NW, NE, SW, SE, each 50 m long, 1.2 m wide, timber slats, two hand cables at 1.0 m, deck at Z 30 falling to Z 29.6 at the Hub. Sway amplitude ±0.6 m. Sightlines: down onto every roof and the ring; across to the opposite line at 24 m. Cover: none. Edges: both sides, the whole length. Below the inner 20 m of each line are the Drip Nets (30 m fall, 24.3 m/s, Stall on nets by 0.3 m/s). Below the outer 30 m is the Well. Routes: walking. Crossing from NW to SW mid-span is a 24 m gap: Climber only, by a 45° Slingshot (Dog B the deck 1.4 m behind you, Jack at 18 m/s, 33 m range, landing on the far walkway at 5.9 m/s vertical: a stagger). Every other class's launch falls short of 20 m. A Scrapper's 20 m line does not even reach across.

**Z4. The Hub.** Steel splice collar, 6 m diameter plate deck at Z 30, 1.1 m rail, the four main cables clamped at its ears. The Hub Ladder (docs/objects/structures/spine-ladder.md, dressed in timber rungs) runs from the north rail (0, 3, 30) down to the ring deck at (0, 8, 16), 14.9 m long. Sightlines: straight down onto the Claim marker 14 m below. Cover: the collar's central drum (2 m diameter, 1.5 m tall). Edges: all around. Routes down: Breaker climbs the ladder (9 s at 1.6 m/s, 7.5 s for everyone else, exposed, back plate to the ring). Everyone else Pendulums from the Hub's rail, drops onto the Stall Gel pad at the ladder foot (×4.0, the one safe drop), or drops 14 m to bare deck (16.6 m/s onto wood: Stall threshold 14.4 m/s on wood, so a straight drop Stalls; Hang a 10 m line first and drop the last 4 m).

**Z5. The Headframe and Lift Run.** Vertical zone at X +22 to +32, Y -5 to +5, Z 2 to 38. The Lift (docs/objects/kinematics/the-lift.md): 3 m by 3 m cage car, west gate, running Z 2 to Z 30 at (30, 0). Upper landing 6 m by 6 m at Z 30 with the winch house on its south side (diesel winch, one fuel drum). Headframe tower to Z 38 with the top platform (3 m by 3 m, sheave, floodlight). Headframe ramp 17 m from the rim lip at (50, 0, 32) to (33, 0, 30). Lift Gate landing at (27, 0, 15), 3 m by 3 m, between E2 and E5. Bottom landing 6 m by 4 m at (25.5, 0, 2). The counterweight (docs/objects/heavy/lift-counterweight.md, rated) runs on guide cables at (14, 0) from Z 30 down to Z 2, opposite the car, through the 3.4 m **Counterweight Gap** between the ring's east edge (X +10) and the East Row. Sightlines: from the top platform, the whole map. Cover: the winch house, the car cage. Edges: the Lift Gate landing and the bottom landing; the ramp is railed both sides. The car and the counterweight sever any line across their slots as they pass (05 section 3.5): a Clothesline across the Lift Gate or a Hang across the Counterweight Gap lives until the next pass; lines seated in the car or the counterweight themselves are not severed. Routes: the Lift is the Breaker's honest route Low to Upper. Stepping off at the Lift Gate is a 1.9 s window as the car passes Z 14 to 17 at 1.6 m/s. Lift bypass for any class: Hang to the headframe jib or a main cable and Reel up at 1 m/s.

### Mid tier

**Z6. The Market Ring.** 20 m diameter timber deck at Z 16, built in two halves bolted along the X = 0 seam, hung from four hanger cables to four corner posts at radius 7 m. 1.1 m perimeter rail with gates at the four swing bridges and the Hub Ladder. A 4 m diameter central hatch is closed by a bolted steel cover plate (**PROPOSED**, break-free 800 N: Breaker, or a Doubler's single line) on the west half's frame; the Claim marker sits on it at (0, 0, 16) on polished runners (friction 0.05, 05 section 6.1a: a Rigger or heavier moves it, slowly). Eight market stalls in the 2 m perimeter band. Cooking row on the east half's north arc: a gas manifold and four racked propane cylinders. Sway ±1.5 m. Sightlines: open across the deck; broken outward by houses at 6 to 7 m; the Hub 14 m overhead; the Counterweight Gap east. Cover: stalls (fracture 300 N s, they are not cover for long), the corner posts, the marker itself. Edges: the full perimeter; 16 m to the nets (17.7 m/s, flip and stagger, no Stall on nets) and the Counterweight Gap. Routes: four swing bridges to W1, W4, E1, E4 (6 to 7 m, 2 m fall to the porches), the Hub Ladder, or a Floor Slingshot to the Hub 14 m up (Climber Perches the rail; Doubler and Scrapper mantle; a Rigger arrives at rail height with nothing to spare and mantles). Breaker to Upper from here: Hub Ladder only, or walk to the Lift Gate (E1 bridge, plank to E2, plank to the Gate: 22 m).

**Z7. The West Row.** Six houses hung from the NW and SW main cables, floors at Z 14. W1 large (-18, +4.3), W2 small (-29, +7), W3 small (-40, +9.6, condemned, no catch). W4 small (-18, -4.3), W5 large (-29, -7), W6 small (-40, -9.6, condemned). Small houses 4 by 4 m, large 6 by 5 m, interior one room with a door each side and a window per wall, flat porches 1 m deep. Swing bridges (docs/objects/structures/swing-bridge.md): W1 to W2 6 m, W2 to W3 7 m, W4 to W5 6 m, W5 to W6 6 m, W2 to W5 9.5 m (the long cross bridge). Plank (docs/objects/structures/plank-bridge.md) W1 to W4 4 m. W1 is the Vilca house: the season's evidence object hangs on its east wall. Sway ±1.5 m, east and west halves 3 s out of phase. Sightlines: dense; a sprinter breaks line of sight every 6 m; windows give 1 m wide firing slots. Cover: house walls (fracture 2,000 N s small, 3,500 N s large). Edges: every porch, every bridge; below W1, W2, W4, W5 are nets (14 m fall, 16.6 m/s, flip); below W3 and W6 is the Well. Routes: bridges (any class). Low to here: ladder from the north terrace (Z 6) to W2's porch (8 m, residents' own, 5 s for a Breaker), the Breaker's honest route; a Floor Slingshot off the nets to a porch 14 m up (every class but the Breaker, whose 9.4 m falls short); a Pendulum from a main cable. Up from here: Hang to the main cable 11 m above a roof and Reel to the High Line in 11 s (every class; the Breaker's 20 m line reaches from any roof).

**Z8. The East Row.** Mirror with the Lift threaded through it. E1 small (+18, +4.3), E2 large (+25, +6.0), E3 small (+40, +9.6, condemned). E4 small (+18, -4.3), E5 large (+25, -6.0), E6 small (+40, -9.6, condemned). Bridges: E1 to E2 2 m plank, E2 to E3 10 m swing bridge (passes 4 m north of the Lift car), E4 to E5 plank, E5 to E6 swing bridge, E1 to E4 4 m plank, E2 and E5 to the Lift Gate 2 m planks. Residents' ladder from the south terrace to E5's porch. Everything else as Z7. The Lift car passing at 1.6 m/s is the row's clock.

### Low tier

**Z9. The Drip Nets.** Cargo net 60 m by 30 m (X ±30, Y ±15) at Z 0, sagging 1 m at center, hung between the Y ±15 cross cables. Surface multiplier ×2.0 (05 section 7.3): Stall at 24 m/s vertical, a fall of about 29 m. Movement at crouch speed is unaffected; run speed −20% (**PROPOSED** net drag). Laundry lines at 2.2 m crisscross it every 6 m (Light props, cosmetic cover). The counterweight sole plate at (14, 0, 0), a 2 m by 2 m steel plate with a timber buffer at Z 1: the counterweight bottoms here every 48 s. Sightlines: up at everything; everyone above sees you. Cover: laundry, the shanties' undersides. Edges: the net's perimeter beyond Y ±15 (the terraces catch at ±15 to ±22; beyond ±22 and beyond X ±30 is the Well). Routes up: terrace rope ladders at X ±20 (4 m), then house ladders (Breaker honest route: 12 m of ladder at 1.6 m/s plus the walk, about 12 s); a Floor Slingshot off the net itself (Climber reaches the Hub at 30 m; Doubler 27 m; Scrapper 15.8 m and Rigger 13.8 m plus mantle reach the porches and the ring deck; Breaker cannot). The Lift bottom landing by rope ladder.

**Z10. The Garden Terraces.** Two planter walks, north (Y +15 to +22) and south (Y -22 to -15), X ±30, decks at Z 4 to 6 in 10 m steps, 2 m walk plus 1 m planter boxes. Timber (×1.2). Water barrels, tool bags, buckets. Sightlines: along the walk 60 m; up into the rows. Cover: planters (0.8 m, crouch). Edges: outer side is the Well. Routes: rope ladders to the nets, residents' ladders to W2 and E5.

**Z11. The Low Shanties.** L1 small house on the north terrace at (-12, +19, floor 4), L2 on the south at (+12, -19, floor 4), hung from the Y ±22 cross cables, each with its own slack cable (cutting it drops the shanty into the Well, taking its terrace section). The two dropped-house catches from W2 and E5 land beside them at floor Z 2. Cover: the only enclosed rooms on the Low tier.

**Z12. The West Face.** The west rim's ravine wall, X -50, Z -60 to 32, concrete and rock. The Spine ladder (docs/objects/structures/spine-ladder.md) bolted to it from the foot landing (-49, -24, 2) to the head at (-50, -24, 32), 30 m. The West Face plank bridge, 18.4 m, from the foot landing to the south terrace's west end at (-30, -20, 4). Sightlines: a ladder climber is visible from the whole West Row. Edges: the plank bridge over the Well, both sides. Routes: the Breaker's honest route Low to Upper on the west: 18 m of plank at 5 m/s, then 30 m of ladder at 1.6 m/s, about 23 s (19 s for everyone else). The East Face (X +50) has no ladder; the Lift is the east route.

**Z13. The Lift bottom landing.** 6 m by 4 m deck at (25.5, 0, 2), hand-truck, the Lift's bottom sheave, a condemnation notice bundle. Rope ladder to the nets. 6 s dwell every 48 s.

## Spawns (05 section 9.3)

| Spawn | Center | Extent | Tier | Notes |
|---|---|---|---|---|
| A | (-58, 0, 32) | X -63 to -53, Y -5 to +5 | Collar rim at Upper height | Solid concrete. Alternate: (-58, +22, 32) on the same rim. |
| B | (+58, 0, 32) | X +53 to +63, Y -5 to +5 | Collar rim at Upper height | Alternate: (+58, -22, 32). |

Well-edge check (05 section 9.3 v0.2.2: 25 m measured along a walkable route). Walkable means on foot without mantling, jumping, or a line; parapets and rails bound the route (**PROPOSED** reading). The parapet is continuous, so the first Well edges reachable on foot from Spawn A are the High Line gates. Route: spawn center (-58, 0) to a gate run entrance at (-50, ±24), 25.3 m on the deck, then 12 m along the railed run to the gate: 37 m. The Spine ladder head at the SW run entrance: 25.3 m. Spawn B: 37 m to either gate; the headframe ramp is railed both sides and the Lift upper landing and car are caged, so the first unrailed edge on that route is the Lift Gate landing, 17 m of ramp plus 15 m of descent away. All routes exceed 25 m. The kill volume ends at X ±50 and does not extend under the rims.

## Claim marker (05 section 9.4)

Authored position (0, 0, 16) on the hatch cover plate, west half of the Market Ring, on polished runners (friction 0.05): about 450 N to start a 900 kg marker, so a Rigger (600 N) or heavier moves it and a Climber or Scrapper (400 N) cannot alone. 4 m radius capture. After Cable Day the west half tilts 2° east, under the runners' 2.9° friction angle, so the marker holds unless lined; if it is Hauled across the seam onto the 40° east half it slides at about 5.9 m/s², leaves the ring over the SE low edge at about 9 m/s, lands on the nets at (7, -7, 0), and stays live there, because the nets are playable floor. It resets per 05 section 9.4 only if it leaves the net footprint into the Well. If Hauled off the west half it falls 16 m to the nets and stays live. A Breaker (or a Doubler on a single 800 N line) who pulls the hatch cover drops the marker through the Eye to the nets.

## Haul

Not supported. No continuous floor connects two truck positions; the spool would spend the match in the ravine.

## Well kill volume (05 section 7.5)

Box: X -50 to +50, Y -45 to +45, Z -12 and below. Hooks spin up at Z -8 (0.5 s before the volume at terminal fall speeds). Mist top at Z -4 so the nets are always clear of fog (map rule 8). Out-of-claim boundary: Marshal warning beyond Y ±40, Stall beyond Y ±45 at any height (**PROPOSED**).
