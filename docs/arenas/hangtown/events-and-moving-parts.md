# HANGTOWN: Events and Moving Parts

_Arena: Hangtown (Vilca's Hang). File purpose: the Sway and the Lift cycles second by second, Cable Day in full, secondary cycles, the Scrap sudden-death shrink event, every tell with lead time, and reset rules._
_Sources: docs/bible/03-arenas.md (Hangtown: the Sway, the Lift, Cable Day, map rules 4 and 5), docs/bible/05-core-mechanics.md (6.7 kinematics, 9.1 and 9.2 match rules, 11 tells). Zones and coordinates per docs/arenas/hangtown/layout.md. Everything not in the bible is **PROPOSED**._

## 1. The Sway (the big moving part)

The whole district moves with the wind. In engine this is a low-amplitude kinematic animation of the cable anchor points at the Hub and along each main cable, not a wind force on every body (03 engine note). Houses, bridges, walkways, the ring, the terraces, and the nets follow their anchors. Players standing on them ride along; no force is applied to players.

| Parameter | Value |
|---|---|
| Period | 12 s |
| Axis | Y (north-south, along the ravine) |
| Amplitude at Upper Hang (High Lines, Hub) | ±0.6 m |
| Amplitude at Mid Hang (houses, ring) | ±1.5 m (ring ±0.8 m, since the Hub averages four cables) |
| Amplitude at Low Hang (nets, terraces, shanties) | ±2.2 m |
| Phase | NW cable 0 s, NE cable 3 s, SW cable 6 s, SE cable 9 s (**PROPOSED** per-cable phasing; it preserves 03's "east half 3 s out of phase with the west half" and adds north-south breathing) |
| Peak velocity | 0.79 m/s at Mid, 1.15 m/s at Low |
| Tell | 1 s timber-and-cable creak before every reversal, localized to the cable that is reversing |

### Second-by-second (one 12 s cycle, Mid Hang, Y offset in meters)

| t (s) | NW cable (W1, W2, W3) | NE (E1, E2, E3) | SW (W4, W5, W6) | SE (E4, E5, E6) | W1 to W4 gap change | Creak |
|---|---|---|---|---|---|---|
| 0 | +1.5 (north apex) | 0, moving north | -1.5 (south apex) | 0, moving south | gap widest, +3.0 m | |
| 1 | +1.3 | +0.75 | -1.3 | -0.75 | +2.6 | |
| 2 | +0.75 | +1.3 | -0.75 | -1.3 | +1.5 | NE and SE creak |
| 3 | 0 | +1.5 (apex) | 0 | -1.5 (apex) | 0 | |
| 4 | -0.75 | +1.3 | +0.75 | -1.3 | -1.5 | |
| 5 | -1.3 | +0.75 | +1.3 | -0.75 | -2.6 | NW and SW creak |
| 6 | -1.5 (south apex) | 0 | +1.5 (north apex) | 0 | -3.0 (W1 and W4 closest) | |
| 7 | -1.3 | -0.75 | +1.3 | +0.75 | -2.6 | |
| 8 | -0.75 | -1.3 | +0.75 | +1.3 | -1.5 | NE and SE creak |
| 9 | 0 | -1.5 | 0 | +1.5 | 0 | |
| 10 | +0.75 | -1.3 | -0.75 | +1.3 | +1.5 | |
| 11 | +1.3 | -0.75 | -1.3 | +0.75 | +2.6 | NW and SW creak |
| 12 | +1.5 | 0 | -1.5 | 0 | +3.0 | |

What it does to lines. A Hang line between W1 and W4 is set at some length L₀. Six seconds later the houses are 3 m closer or farther. Farther: the line is over its Hang load and slips at 0.5 m per 0.1 s with a creak (05 section 3.2), or on a Rigger's 12 kN line it holds and both houses are pulled 1.5 m off their sway. Closer: the rod goes slack and anything hanging from it drops 3 m. A Pendulum from a house roof gains or loses 1.5 m of swing depending on the phase. A Slingshot to a house porch aimed at t = 0 lands 3 m off at t = 6. New players fight the Sway. Good players read the creak.

## 2. The Lift (secondary kinematic, the honest route)

docs/objects/kinematics/the-lift.md. Car 3 m by 3 m at (30, 0), running Z 2 (bottom landing) to Z 30 (upper landing). Counterweight (docs/objects/heavy/lift-counterweight.md, rated) runs the opposite direction on guides at (14, 0), through the 3.4 m Counterweight Gap east of the Market Ring. Cycle 48 s. The car starts at the bottom and departs 3 s after the Marshal calls "live".

| t (s) | Car | Counterweight | Notes |
|---|---|---|---|
| -3 | bottom dwell, Z 2 | top, Z 30 | Bell, two strikes. Departure tell (kinematic approach, 3 s). |
| 0 | departs up, 1.6 m/s | departs down | Diesel winch note rises. |
| 7 | Z 13 | Z 19 | Guide cable hum begins (2 s to Mid Hang passage). |
| 8 to 10 | Z 14 to 17 | Z 17 to 14 | **Lift Gate window**, 1.9 s. Step on or off. Counterweight passes the ring's east edge: Gap plays live. |
| 16 | Z 27 | Z 5 | Hum under the nets. Crush zone arms. |
| 17.5 | Z 30, arrives | Z 2, bottoms on the buffer | Crush: anyone at (14, 0) on the nets. |
| 18 to 24 | upper dwell, 6 s | sits on the buffer | Safe to board. Bell at t = 21. |
| 24 | departs down | departs up | |
| 32 to 34 | Z 17 to 14 | Z 14 to 17 | Second Lift Gate window. Counterweight passes the ring's east edge going up. |
| 41.5 | Z 2, arrives | Z 30, stops 1 m under the jib guard | Crush against the guard for anyone riding the counterweight's top. |
| 42 to 48 | bottom dwell | top | Bell at t = 45. Cycle repeats. |

Kinematic rules (05 section 6.7): nothing moves the Lift or the counterweight. A player hit by the car or the counterweight takes the full relative speed against the 9 m/s kinematic Stall threshold; at 1.6 m/s this is a shove, so the Lift kills only by crush or by what it drags you into. A line between the car and anything moves the anything (the Caboose, vertical). The car and the counterweight sever any line they pass through (05 section 3.5): a Hang across the Lift Gate or the Counterweight Gap snaps on the next pass, both Dogs release; lines seated in the car or the counterweight themselves are not severed.

## 3. Cable Day (the mid-match event)

Fires at 1:15 remaining in every Scrap round and once in Claim at 4:00 remaining (4:00 elapsed), per 05 section 9.2 v0.2. T0 is the moment the NE anchor pin shears.

### Sequence

| Time | What happens | Where it is felt |
|---|---|---|
| T-5.0 | Siren. Marshal: "Cable Day." The NE High Line sags 2 m at its rim end. Sparks at the NE pin socket. | Everywhere. Players on the NE line feel the deck drop under them. |
| T-3.0 | Strand pings: individual wires in the NE cable let go, a tick every 0.3 s, accelerating. | East Rim, East Row. |
| T-1.0 | Rated-load groan from the pin socket (05 section 11, 1.0 s minimum). | East Rim. |
| T0 | The NE anchor pin shears. The cable's rim end whips free. The bang is heard across the arena. | |
| T+0.0 to 1.5 | The NE High Line's outer 25 m (X +25 to +50) sheds in five 5 m sections into the Well. Players on it fall with it. | Z3, Z2 (the NE gate now opens onto air). |
| T+0.5 | The Hub drops 1 m and shifts 2 m south-west, tilting 5° toward the NE. The three remaining High Lines dip 1 m at their inner ends. | Z4, Z3. |
| T+1.0 | The NE hanger shackle shears under the lurch. The east half of the Market Ring begins to pivot about the seam. | Z6. |
| T+1.0 to 4.0 | The east half rotates to 40°. Stalls, chairs, buckets, and the four cooking cylinders (their rack spills at 25°) slide toward the SE low edge (Z 9.6 at the rail). The manifold hose tears: a 3 s jet east along the deck (a_ref 18 m/s² on a 100 kg body, 05 section 6.6: Climber 20, Breaker 8.2; it moves players and loose objects only). Players on the east half slide at about 4 m/s² and hit the SE rail at up to 9 m/s (flipped over it onto the nets). | Z6. |
| T+1.5 to 3.0 | The inner 25 m of the NE High Line swings down from the Hub ear and hangs vertically: **the Dangle**, a 25 m climbable walkway hanging from (3, 3, 30) to Z 5, swinging ±4 m. | Z4, Z9. |
| T+2.0 | E1, E2, E3 are now hung from the dangling cable. They swing free on pre-authored arcs (below). The Hub pendant light swings on its chain, period 3 s. The east half's string lights short out. | Z8, Z6, Z9. |
| T+4.0 | The east half bounces once and settles at 40°, hanging by the SE hanger. The west half has dropped 0.4 m and tilts 2° east (**PROPOSED** value: under the 2.9° friction angle of the marker's 0.05 runners, so the marker does not creep). The Claim marker, on the hatch cover on the west half, holds and is 1 m from a 40° slope; Hauled across the seam it slides off the SE low edge onto the nets and stays live there. | Z6. |
| T+5 onward | The arena is in its Cable Day state until round end. | |

### Freed house arcs (pre-authored, per 03 engine note)

| House | Period | Sweep | Height | First pass | Hits |
|---|---|---|---|---|---|
| E1 (small, 1,200 kg) | 9 s | NE quadrant of the ring's edge, X +8 to +22, Y +2 to +10 | Z 15 to 18 | T+5 | Anyone on the ring's NE arc or the E1 bridge stub. 1,200 kg at 6 to 8 m/s. |
| E2 (large, 2,800 kg) | 10 s | X +18 to +30, Y +4 to +8; clears the Lift car by 1 m | Z 14 to 19 | T+6 | Anyone on the Lift Gate landing's north edge, or hanging off the car's north face. |
| E3 (small) | 11 s | low and wide, X +22 to +44, 2 m east of the Lift bottom landing | Z 3 to 9 | T+7 | Anyone upright on the east third of the nets or the north terrace's east end. Crouching under it works at Z 3 only at the arc's ends. |

Each house fractures (small 2,000 N s, large 3,500 N s) if it hits another house or the Lift structure; fragments fall per 05 section 6.5. E1 and E2 are authored not to meet. A player who Hauls one into the other gets the thesis statement.

### Zone-by-zone after Cable Day

| Zone | State |
|---|---|
| Z1 West Rim | Unchanged. |
| Z2 East Rim | The NE gate opens onto a 92 m drop. Pin socket smoking, arc flash every 4 s from the severed floodlight feeder (**PROPOSED**). Spawn B still solid; its alternate at (+58, -22, 32) is preferred by the reserve logic because the NE gate edge is now 14 m from the main spawn. |
| Z3 High Lines | Three remain. Inner ends 1 m lower. The SE line is the only cable route from the East Rim to the Hub. |
| Z4 Hub | Tilted 5°, 1 m lower, the Dangle hanging from its NE ear. Hub Ladder intact. Pendant swinging. |
| Z5 Lift Run | Running. E2's arc passes 1 m from the car's north face. The Lift Gate landing survives; its E2 plank is gone. The bottom landing is 2 m from E3's arc. |
| Z6 Market Ring | West half near level (2° east), marker in place on its runners. East half at 40°, string lights out, stalls piled at the SE rail, cylinders loose and rolling, manifold spent. Standing on the east half requires a Hang line or a brace. |
| Z7 West Row | Unchanged except a 0.4 m step at the W1 and W4 bridges. |
| Z8 East Row | E1, E2, E3 swinging. E4, E5, E6 intact. The E1-E2, E1-E4 planks and the E2-E3 swing bridge are gone. E4 and E5 are now the only houses with routes to the Lift Gate. |
| Z9 Drip Nets | E3 sweeps the east third every 11 s. The Dangle's lower end swings over the net center-north. |
| Z10, Z11 | North terrace's east end under E3's arc. South side unchanged. |
| Z12, Z13 | Unchanged. |

### If a Breaker pulled the NE pin early

Pulling the NE anchor pin fires the Cable Day sequence from T0 without the siren; the pin's 1.0 s groan is the tell. At the scheduled time (1:15 remaining in Scrap, 4:00 in Claim) the event then fires on the SE cable instead (**PROPOSED** fallback, so rule 5 holds): the SE line sheds, E4, E5, E6 swing free, and the east half, now held by nothing, goes down the Well with everything on it.

## 4. Secondary cycles and triggers

| Cycle or trigger | Period or condition | Effect |
|---|---|---|
| Counterweight crush zone | t = 17.5 and 41.5 of each Lift cycle | Crush at (14, 0, 0) and under the jib guard. |
| Lift Gate window | t = 8 to 10 and 32 to 34 | Board or leave the car at Z 14 to 17. |
| Eviction (house drop) | any Cutter, 0.6 s hold on a house slack cable; or lines summing to 1,000 N on one axis on the house (houses attach at a rated 1,000 N: a Breaker alone, a Doubler at overspin, or a crew such as two Riggers plus anyone, per 05 v0.2.2) | Non-condemned house falls 12 m to its catch at floor Z 2, arrives at about 15 m/s, swings there (period 10.6 s). Condemned houses (W3, W6, E3, E6, L1, L2) fall into the Well. |
| Demolition charges | object or player impact over 6 m/s to the charge at W3's or E3's rated hanger eye (Z 19.0) | Blows the eye: the house goes down the Well. 5 m blast radius centered 0.5 m above the roof ridge. |
| Cut-Loose (bridge) | 0.6 s hold on either end's slack cable | Bridge swings down from its far end; riders are flung at the far end's arc speed. Over the nets they survive flipped; outside the net footprint (W2-W3, W5-W6, E2-E3, E5-E6) they go down the Well. |
| Roof tank rupture | 1,500 N s to a roof tank | Local flood on that roof, pushes outward at 4 m/s for 2 s. |
| Counterweight release | lines summing to 1,000 N on one axis on the counterweight (05 section 6.7, rated attachment): a Breaker, a Doubler at overspin, or a crew | The counterweight leaves its guides and falls as a simulated rated load. From Mid Hang height it reaches the sole plate in 1.7 s and crushes anything in its footprint; the Lift car, unbalanced, runs down to the bottom landing under its brake at 4 m/s and stays there for the round (**PROPOSED** consequence). Riders take 4 m/s against a kinematic: a stagger. Resets between rounds. |
| Anchor pin pull (NW, SW, SE) | Haul at 1,000 N on one axis: a Breaker, a Doubler at full overspin, or a crew whose lines sum to it (two Riggers plus anyone), 1.0 s groan | That High Line sheds its outer 25 m and dangles its inner 25 m; the cable's two non-condemned houses drop to their catches, the condemned one goes down the Well; the Hub drops 1 m and tilts 5° that way; that ring half tilts 10°. |

## 5. Sudden death: Second Cable (**PROPOSED**)

Scrap only, on a time-out tie with equal riggers (05 section 9.1). The arena shrinks to what hangs from two cables.

| Time | Event |
|---|---|
| 0 | Marshal: "Second Cable." 5 s siren. The SW pin socket sparks; the SW High Line sags 2 m. |
| T0 | SW pin shears. SW High Line sheds its outer 25 m, dangles its inner 25 m. W4 and W5 drop to their catches; W6 goes down the Well. |
| T+1 to 4 | The west half of the Market Ring drops to 40° toward the south-west. The deck is now a saddle. The hatch cover shears its bolts and falls through the Eye with the Claim marker (irrelevant in Scrap). |
| T+5 to 25 | The falling SW cable's rim end has cut the Low Hang cross cables at the west face. The nets and terraces peel away west to east, 3 m/s, X -30 at T+5 to X +30 at T+25, taking L1, L2, the West Face plank bridge, and the dropped houses' catches with them. Hooks spin up continuously below. |
| T+25 | Remaining floor: both rims, the NW and SE High Lines, the Hub, the two 40° ring halves, the Lift and its landings, W1, W2, E4, E5, and six houses swinging on the Dangles. Everything below Z 2 is the Well. |
| T+45 | If still tied: the NW pin shears, the Hub and ring go down the Well over 5 s. If both crews are scrapped together the round is replayed. |

## 6. Tells (05 section 11, Hangtown-specific)

| System | Audio | Visual | Lead |
|---|---|---|---|
| Sway reversal | timber-and-cable creak, localized | the reversing cable's houses pause | 1.0 s |
| Lift departure | bell, two strikes; winch note | car lamp turns green | 3.0 s |
| Counterweight passing Mid or bottoming | guide cable hum | shadow under the Hub pendant | 2.0 s |
| Slack cable under the Cutter | saw-whine rising in pitch | sparks at the tape, cable jitters | 0.6 s (the hold itself) |
| House drop | hitch twang, then the catch snap | house lurches 0.3 m before it goes | 0.2 s plus the 1.6 s fall |
| House swing under a line | line sings, hitch groans | house leads its sway | continuous |
| Demolition charge armed | hiss | venting at the collar | 0.3 s |
| Anchor pin pull | structural groan, strand pings | dust from the socket, the High Line sags | 1.0 s minimum |
| Counterweight release | structural groan, guide cables twang | counterweight shudders, dust from the guides | 1.0 s minimum |
| Cable Day | siren | NE line sags 2 m, sparks at the pin | 5.0 s |
| Manifold vent | gas roar | white plume along the deck | 1.0 s (force volume) |
| Freed house approaching | wind through its windows, hitch creak | house silhouette crosses a light | continuous; period posted above |
| Second Cable | siren | SW line sags | 5.0 s |
| Ring-out | Hooks spin up at Z -8 | rotor light in the mist | 0.5 s |

## 7. Reset rules

Per 05 section 9.2 step 5, between Scrap rounds: every object returns to its authored placement and state. Hangtown additions (**PROPOSED** where not in the bible):

- All fourteen houses re-hung at their authored positions with slack cables and catch lines restored. Dropped houses and fragments cleared. The two demolition charges re-staged.
- The counterweight re-seated on its guides if released. Cable Day state cleared: NE pin restored, NE High Line and hangers restored, east half level, Hub level, Dangle removed, string lights on, manifold recharged, cylinders back in the rack. Any early pin pulls reversed.
- Bridges re-hung. Hatch cover re-bolted. Claim marker (Claim mode only) at (0, 0, 16).
- The Lift reset to bottom dwell; the car departs 3 s after "live" so every round's Lift Gate windows fall at the same clock times (8, 32, 56 s and so on).
- The Sway phase reset to t = 0 at "live".
- Set lines cleared, as everywhere.
- Claim mode is one 8:00 round: no resets; Cable Day fires once at 4:00 remaining (05 section 9.2); reserve riggers drop per 05 section 9.3.
- Second Cable state never persists: it only occurs in a round's final seconds.
