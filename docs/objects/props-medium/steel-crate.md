# Steel crate

Medium prop. Mass tier Medium, 250 kg. 120 × 100 × 90 cm (L × W × H). Appears in the Yard (the Stacks, stacked three high beside billet containers; the Floor, loose by the rail trenches), Ballast (the Bays, on the dead magnetic racks), and the Anchor (the Rim, in contractor laydown rows).

## Role in play

The stackable Medium. It exists to be in a stack, and a stack exists to come down. It is the smallest object that Stalls a player at walking speed, and a line only has to start it: gravity and the floor do the rest.

| Rig | Use |
|---|---|
| The Avalanche | Haul the bottom crate of a three-high stack at or above the 500 N stack break-free. The two above it follow. |
| The Guillotine | Hang one crate overhead on a set line (2.5 kN, within every class Hang load), Reel it up at 1.0 m/s, then recall or cut your own set line. It drops by gravity: from 4 m it lands at 8.9 m/s, 2,200 N·s, a Stall. A second line to the enemy only steers it. |
| The Anchor Drop | Tether an enemy to a crate near an edge, then push the crate over. |
| Overboard | A self-dog to enemy Jack is a T push only; the Kick never fires against a Catch Plate. A stacked crate behind you (500 N break-free) is the backstop that keeps the push on them. |

- **Scrapper:** cannot Avalanche. 400 N is under the 500 N stack break-free; the line sings and the crate creaks. A loose crate moves at 1.6 m/s² and arrives at 8 m/s over the Scrapper's 20 m line, which is still a Stall. No 30% bonus; the crate is over 100 kg.
- **Rigger:** 600 N starts a stacked crate. Hangs a crate at 2 m over a doorway and leaves.
- **Breaker:** the Avalanche is theirs. 1,000 N on 250 kg is 4.0 m/s² once break-free clears; the crate lurches off the stack and falls.
- **Climber:** perches on top, Slingshots off the side. Never Hauls one toward themselves: 400 N on 90 kg is 4.4 m/s², above the Climber's 3 m/s² foot-hold, so the Climber leaves the ground and travels 73% of the distance.
- **Doubler:** two lines, two crates, one enemy between them. 480 N per split line does not start a stacked crate; 720 N at overspin does. Loose crates move on either.

## Appearance

Silhouette: a squat welded box with a hinged lid, two forklift channels in the base, and four lifting lugs at the top corners. Corners are rolled, not sharp. Lid has two hasps.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Body panels | `MI_Steel_Painted_Worn` | `#5B6670` default; `#4A4D50` HTR; `#C8912A` Guild | 0.45–0.70 | 0 on paint, 1 at chips |
| Lugs, hasps, fork channels | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 | 1 |
| Rust bloom at seams | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 | 0.3 |
| Stencils | `MI_Decal_Stencil` | `#E6E4DD` or `#E8620C` | inherits | 0 |

Wear story: hand-polish on the hasps and the lid's front edge. Fork channels scraped to bare steel in two horizontal bands. Rust streaks down from every lug. Paint chipped at all twelve edges. Stacked crates have a bright rectangle on the lid where the one above sat and rust where it did not.

Text: HTR variant reads `HTR-RC-4471` and `RECLAIMED / DO NOT RETURN` in orange on grey. Guild variant reads `LG-0932` with the hitch-knot glyph in ochre on black. Yard crates carry a chalk tally of weigh-ins.

Lighting: a large flat body that reads as a dark block under the Yard's floodlights. Lid highlights give the stack its stepped read. In Ballast the Hold grime tint `#3A4A3C` greens the lower third.

## Variants

| Variant | Difference |
|---|---|
| HTR grey | `#4A4D50` body, orange band at mid height, asset tag. Low wear 0.3. The Yard and the Anchor Rim. |
| Guild ochre | `#C8912A` brushed paint, high wear 0.7, chalk tallies. The Yard. |
| Meridian cargo | `MI_Meridian_Cyan` `#2A9FB5` at 0.85 wear, faded hexagon, cargo code `C5-BAY2-118`. Ballast only. |
| Dented | Two authored dents (lid, one side), lid hasps sprung. Spawns dented in the Hold. |
| Open | Lid up, contents visible (tool bags, chain). Used as laydown cover. |

## Physics

- Tier Medium, 250 kg.
- Collision: one box 120 × 100 × 90 cm. Lid is a second box 120 × 100 × 4 cm on a hinge constraint when the crate is in the open or broken state; otherwise merged.
- Center of mass: geometric center lowered 8 cm (contents settle). (**PROPOSED**)
- Friction 0.6 steel on steel, 0.5 on concrete, 0.45 on wood. Restitution 0.15. (**PROPOSED**)
- Break-free (05 §6.2): loose on floor 0 N, anyone; on a Ballast magnetic rack 200 N (cradle), anyone; in a stack 500 N, Rigger, Doubler, Breaker only; chained in the Anchor laydown 800 N, Doubler and Breaker only. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Does not fracture. Dents at 1,500 N·s, lid blows open at 3,000 N·s and spills contents (Light props, 10 s lifetime). (**PROPOSED**, modeled on the 05 §6.5 container rule) A flipped Breaker landing on it at 8 m/s is 1,760 N·s and dents it; player bodies count against these thresholds.

Line acceleration on a loose crate and arrival speed over a full line with the other end static or braced (v = sqrt(2 a d)):

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 1.6 m/s² | 50 m | 12.6 m/s | 3,160 N·s |
| Scrapper | 400 N | 1.6 m/s² | 20 m | 8.0 m/s | 2,000 N·s |
| Rigger | 600 N | 2.4 m/s² | 40 m | 13.9 m/s | 3,464 N·s |
| Doubler | 800 N | 3.2 m/s² | 30 m | 13.9 m/s | 3,464 N·s |
| Doubler overspin | 1,200 N | 4.8 m/s² | 30 m | 17.0 m/s | 4,243 N·s |
| Breaker | 1,000 N | 4.0 m/s² | 20 m | 12.6 m/s | 3,160 N·s |

Nothing reaches the 25 m/s Haul cap; a crate only passes 17 m/s by falling, by a kinematic, or by a Snap. The 0.4 s ramp adds about 0.2 s to each arrival.

Impact on a player (impulse = 250 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 1.2 m/s | 300 N·s | Stagger |
| 2.0 m/s (walking) | 500 N·s | Stagger |
| 2.4 m/s | 600 N·s | Flip |
| 4.8 m/s | 1,200 N·s | Stall; a Scrapper needs 7.2 m of run-up, a Breaker 2.9 m |
| 6.0 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall |
| 11.7 m/s (7 m drop) | 2,925 N·s | Stall; dents on landing |
| 13.9 m/s (fastest Haul arrival, Rigger or Doubler) | 3,464 N·s | Stall |

Jack (05 §3.2 v0.2): the crate is Kicked to V × M / (M + m) within 0.3 s, then accelerates at T / 250 kg. Breaker (V 10): crate 4.7 m/s, Breaker 5.3 m/s back. Rigger (V 12): crate 4.1, Rigger 7.9. Scrapper (V 14): crate 4.3, Scrapper 9.7. Doubler (V 14): crate 4.8, Doubler 9.2. Climber (V 18): crate 4.8, Climber 13.2. Every Jacked crate leaves at a Stall speed for anyone within arm's reach.

Crush: no line reaches the 8 kN crush rule (maximum pull is 1,200 N at overspin) (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). A crate's weight is 2.5 kN; three stacked are 7.4 kN, still under. A crate crushes only when a kinematic drives it: a Hauler pushing a crate against a trench wall pins and crushes in 0.5 s.

## Interactions

- **Haul to it:** the player is pulled to the crate. 400 N on a 90 kg Climber is 4.4 m/s², over the 3 m/s² foot-hold: airborne at once. 1,000 N on a 220 kg Breaker is 4.5 m/s², under the 5 m/s² foot-hold: the Breaker skids and the crate comes to them at 4.0 m/s².
- **Jack from it:** a launch with the inverse-mass split above. A Climber takes 13.2 m/s of the 18 m/s V off a loose crate, close to a wall launch; off a stacked crate under 500 N it is a wall launch and the crate takes nothing.
- **Hang from it:** a Hang to a stacked or racked crate does not unseat it; break-free is tested against active pull only, so a Pendulum or Gallows anchored to a crate in the Stacks holds up to the class Hang load. A Hang to a loose crate brings it along as 250 kg of dead weight: hung from a player on an overhead line it drags them to the floor; caught on a ledge it is a 250 kg anchor. A player Hung to a crate climbs it by Reel: hold Haul within Hang and the rod shortens at 1.0 m/s, a winch, not a pull.
- **Object to object:** crate on crate at over 4 m/s dents both. Crate into a propane cylinder at over 8 m/s triggers it; a Scrapper's line gets there over 20 m.
- **Kinematic:** a Hauler hitting a crate at 12 m/s delivers the full relative impact and throws it clear of the trench at roughly 12 m/s. The Gantry magnet lifts crates in the Yard. A Hauler passing through a line that holds a crate severs it (snap, both Dogs release); lines attached to the Hauler itself are not severed.
- **Water:** sinks. Resting on the Hold floor it is pushed by the flood volume at 0.4× its authored acceleration (a_ref × min(100 / m, 2)).
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A stacked, racked, or chained crate is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 250 kg crate feels 0.4× the authored acceleration (a_ref × min(100 / m, 2)): the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) shoves it a few centimeters out and back. The Updraft does not lift it.
- **Breathing:** at 6° the stack's friction holds. At the Shear the whole rack can drop.
- **Cut recoil:** 150 N·s per 1,000 N of line force, 0.6 m/s on a crate. Not visible.
- Slides, does not roll. Tumbles end over end only off an edge.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | As above. | Full mass, merged collision. |
| Dented | Material swap to the dented normal set, hasps sprung. | No change. |
| Broken | Lid open on its hinge, contents spilled. | Lid is a separate hinged body. Mass 230 kg. |
| Destroyed | Not reachable. Crates do not destroy. | |

## VFX and SFX hooks

- Impact: `SFX_Crate_Steel_L` on steel, `SFX_Crate_Concrete_L` on concrete, `SFX_Crate_Wood_L` on planks, `SFX_Crate_Splash` into water. Scale by impulse.
- Break-free: lurch scrape at 0.2 s, `NS_BreakFree_Dust_M` from the base, `SFX_BreakFree_Steel`. Under threshold, `SFX_Crate_Creak` loops while the line sings.
- Dent: `NS_Impact_Steel_M` sparks, `SFX_Dent_Hollow`.
- Lid blow: `NS_Crate_Spill` spawns the Light contents.
- No emissive.

## Technical

- LOD0 6,000 tris, LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set (BC, N, ORM) plus the stencil decal atlas.
- Pivot: bottom center of the footprint. X+ is the long axis.
- Dog sockets, all surface class `Metal`: `SKT_Dog_Top_01`, `SKT_Dog_Top_02` (lid, front and rear thirds); `SKT_Dog_Hook_01` to `_04` (lugs); `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01`, `SKT_Dog_SideE_01`, `SKT_Dog_SideW_01`; `SKT_Dog_Lid_01` (lid front edge).
- Not a geometry collection. Static mesh with a dent material swap and a hinge constraint on the lid.
- Anchor fields: `FS_Anchor_SteelCrate` releases stacked instances at 500 N (one field per stack, authored by the stack tool); `FS_Anchor_SteelCrate_Rack` releases Ballast rack instances at 200 N; `FS_Anchor_SteelCrate_Chain` releases chained instances at 800 N. Forces from several lines on one crate add as vectors before the comparison.
- Replication RC-M: server-authoritative, 20 Hz. The stack wakes as one event so all clients see the Avalanche start on the same frame.

## Reference search terms

Steel shipping crate hinged lid lifting lugs; military storage crate forklift pockets; site tool chest welded; stacked steel containers scrapyard; powder coat chipped edges macro; rust streak from bolt photo.
