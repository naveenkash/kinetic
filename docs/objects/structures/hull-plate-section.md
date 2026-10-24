# Hull Plate Section

One-line: structure, load-bearing and Rated tier (structural, 7,500 kg), 8.0 m x 4.0 m x 0.6 m including ribs, the Yard: stacked three high to form the Stacks' mesas and canyons, lying loose on the Floor as rated loads, and carried by the Gantry as its load. Also appears cut and seated as the Shear bay wall in Ballast (see `kinematics/breathing-hull.md`).

## Role in play

Hull plate sections are the Yard's terrain. Stacked, they are the walls and roofs of the mid tier. Loose on the Floor, they are the rated loads a Breaker drags into a trench to make a wall ahead of a Hauler (03: it derails nothing, it blocks). Hanging from the Gantry, one is the Magnet Drop. A section is the heaviest thing that routinely moves in the Yard, and the one object every harness has to plan around.

| Harness | Relationship |
|---|---|
| Climber | Mesa tops are the mid tier's perches. The Gantry load is a 7,500 kg ride with a 50 m line to anywhere. |
| Scrapper | Static terrain. The gaps between stacked plates are 0.4 m: a slide passes, a Breaker does not. |
| Rigger | Hang anchors. A line to a plate is a line to the world. |
| Doubler | Overspin reaches 1,200 N: 1.3 s above the 1,000 N break-free, enough to commit a Floor section before the snap. |
| Breaker | Only a Breaker moves one on purpose: 1,000 N on 7,500 kg is 0.13 m/s², nothing. The line does not move the section; it knocks out the timber wedge the Guild propped it on, and the section tips into the trench beside it. |

## Appearance

Silhouette: a slab of curved climber hull, cut square by plasma, with internal ribs on the concave face and the original exterior on the convex. Cut edges glow-scored. Three magnet pads welded on the convex face for the Gantry.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Exterior skin | Weave-faced composite, matte | #1B1F26, sheen in raking light | 0.35 | 0.6 |
| Ribs | aluminium-steel alloy, original grey | #7C8488 | 0.5 | 0.8 |
| Cut edges | heat-blued steel, slag | #4A3A5C to #B36A2E | 0.6 | 0.7 |
| Magnet pads | mild steel, 600 mm square, bright | #A8ADB1 | 0.3 | 1.0 |
| Rated tape | yellow and black | #F2C500 / #141414 | 0.7 | 0.0 |

Weathering: each section carries the name of the climber it came from in Consortium lettering, half cut away: `PERSEV`, `ANCITY`, `NSTANCY`. Rain streaks. Bird lime on top plates. Guild cut-sequence chalk on the ribs (`3/7 PRESS WED`). Yellow-and-black structural tape around every Floor section (rated) but not around stacked sections (they are terrain, and a stack does not move; see `environment/structural-tape.md` for the rule). HTR has sprayed `CLAIM 61` in orange on the three largest.

Lighting: none on the sections. Floodlights from the masts rake them; the Weave skin specular is the Yard's visual signature.

Read by tier: from the Rails, a grid of dark rectangles with bright magnet pads. From the Stacks, walls 4 m high with rib shadows. From the Floor, slabs on the concrete with tape around them.

## Variants

1. **Stacked (terrain).** Three high, 12.6 m mesa with 0.4 m gaps. Static. No tape.
2. **Floor section (rated load).** Lying convex-down on the concrete, one edge propped on a timber wedge at a trench lip, taped. Break-free 1,000 N. The Breaker's wall.
3. **Gantry load.** The Pick Stack's top section, 2,800 kg (Heavy, 500 N in-stack break-free until the magnet takes it), hanging from the magnet convex-up. Simulated body on a constraint with a scripted release (03 engine notes); a rated attachment while hanging (05 section 6.7).
4. **Ballast bay wall (Shear).** Still part of _Tenacity_, pre-fractured, released by the event. Not a Yard object but the same mesh family.

## Physics

- Tier: Rated. Mass 7,500 kg (**PROPOSED**, consistent with the Rated floor of 3,000 kg and the Gantry's rated hoist).
- Floor section break-free: 1,000 N. Only a Breaker or a Doubler at overspin. A slab on bare concrete is fixed scenery (05 section 6.1a); the wedge is its staging. No line slides 7,500 kg on concrete (friction at a 0.3 coefficient is 22 kN); the section is authored propped on a timber wedge with its center of mass 0.4 m inside the trench lip, and the commit (1,000 N sustained 0.6 s, README) knocks the wedge out. **Authored consequence (the Plow):** 1.0 s groan while the wedge splinters, then the section slides down its greased skids to the lip (0.2 to 0.27 m/s², 4 s, visible) and drops into the 6 m trench, standing across it as a 2.7 m wall. It blocks players until the next train, which carries it ahead of itself at 12 m/s as a 12 m by 2.7 m wall whose ends overhang both lips by 3 m, to the end plenum, where it is dumped and removed. Anyone in the trench or within 3 m of either lip in its path is hit by a kinematic at 12 m/s: scrapped. Four Floor sections per round are staged this way, one per trench crossing the Guild has flagged `UNSTABLE`.
- Gantry load falling: the Gantry's load is the lighter 2,800 kg Pick Stack plate (see `kinematics/gantry-crane.md`), released at 9.3 m over the pit (17.6 m/s) or at traverse height in Shift Change (about 21 m, 20.3 m/s). On a player: scrapped at any contact. On a billet stack: tips it. On the press pit: the Well swallows it. On the Floor: impact decal, a 0.5 s debris shove, then fixed scenery.
- Stacked sections do not move. There is no stack break-free; they are world geometry. The Breaker's "tip a stack" in 03 refers to billet stacks.
- Crush: a player in the trench under the dropping section is pinned between 7,500 kg and the trench floor. Over 8 kN for 0.5 s: scrapped. A player on the lip is swept into the trench by the sliding edge (kinematic push at about 1 m/s, under the stagger threshold) and then has the Hauler schedule to think about. The 1,000 N is a 0.6 s commit trigger (a Breaker, a Doubler at overspin, or a crew summing to it on the slide axis); after it the authored timeline carries the section regardless of friction.

## Interactions

- **Haul** a Floor section (Breaker): lurch, groan, creep. A player standing on it rides it into the trench.
- **Jack** off a stacked section: the most common Slingshot surface in the Yard.
- **Hang** from the Gantry load to yourself: a 7,500 kg pendulum bob that moves at 2 m/s. Ride it across the Yard. When the magnet releases, you go with it unless you cut.
- Chain: a dropped Gantry load onto a fuel drum cradle: the drums are launched at over 10 m/s and detonate. A section into a trench ahead of a Hauler: the Hauler passes under; anyone on the section is a Caboose without a line.

## States

| State | Visual |
|---|---|
| Intact | Flat, taped, magnet pads clean. |
| Stressed | Under a line below 1,000 N: the tape flutters, the wedge creaks, a long low groan. |
| Failing | Lurch 0.2 s, groan 1.0 s minimum, section slides on screaming steel, sparks from the trailing edge. |
| Failed | Section seated in the trench at an angle, trench lights red on both sides, tape torn. |

## VFX and SFX hooks

- `SFX_HullSection_Groan` (sustained, pitch by force), `SFX_HullSection_Slide_Sparks`, `SFX_HullSection_Impact_Trench`.
- `VFX_Weave_Sheen_Pulse` on impact. `VFX_Steel_Sparks_Trail` while sliding.
- Gantry: `SFX_Magnet_Release_Clack`, `VFX_Magnet_Pad_Flash` 2 s before drop (05 section 11, Gantry drop lead).

## Technical

- LOD0 6,500 tris, LOD1 2,800, LOD2 900, LOD3 slab. Ribs collapse to normal map at LOD2.
- Textures: `T_HullPlate_Exterior` 4K (Weave clearcoat), `T_HullPlate_Ribs` 2K, `T_Yard_Decal_Atlas` 2K.
- Pivot at the convex face center. Sockets: `Magnet_0..2`, `Rib_Dog_0..7` (seat hints), `Tape_Loop`.
- Physics: stacked variant is static mesh with complex collision. Floor variant is a rigid body with `Kinetic_RatedLoad` channel, kept kinematic until the authored-consequence state machine (README) hands it to simulation. Gantry variant is a rigid body on a 6-DOF constraint with linear limits locked, released by the Gantry timeline.
- Collision: `Kinetic_WorldStatic` (stacked), `Kinetic_RatedLoad` (Floor), `Kinetic_Heavy` while hanging.

## Reference search terms

ship breaking yard hull plate cut, Alang hull section crane, plasma cut steel slab edge, scrap yard magnet crane plate, composite hull rib structure.
