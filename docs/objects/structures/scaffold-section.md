# Scaffold Section

One-line: structure, destructible, Medium tier, 260 kg per bay, 2.0 m x 2.0 m plan x 8.0 m lift with intermediate ledgers at 2 m, the Anchor only: five ring decks of contractor scaffold around the Stump from r 16 to r 24 (8 m wide), decks L1 to L5 at Z 6, 14, 22, 30, 38 on the Rim datum (Rim deck Z 0, bowl floor and footing Z -8), per `arenas/the-anchor/layout.md`.

## Role in play

The Scaffold is the Anchor's main fighting volume. Every section is a floor, a wall, a ladder, and a potential fall. It is the only tier of the Anchor where sightlines break, so it is where Riggers set lines and where Scrappers chain props. A Breaker treats it as a thing to remove: pull a maintenance clamp (see `rated/maintenance-clamp.md`) and a whole ring goes. A Climber treats it as a launch grid: every standard is a Slingshot wall.

Named rigs: the Clothesline (ledger to ledger at 1.2 m), the Gallows (hang from the deck above and Reel), the Guillotine (a board stack hung over a plank run on a set line, dropped by recalling or cutting it), Overboard on the inner edge into the Well (a T push, never a Kick, since the Dog is in a plate), the Ascension launched from L5's inner half.

| Harness | Relationship |
|---|---|
| Climber | Perches on standards, Slingshots between levels in one Jack. Owns L4 and L5. |
| Scrapper | Boards and clamps become ammunition after any fracture. Recalls Dogs to re-dog between levels fast. |
| Rigger | Sets two lines across plank runs. The Scaffold's regular 2.5 m grid makes set-line placement predictable. |
| Doubler | Overspin snap on a section (fracture at 800 N s) is an easy drop. Two lines bridge two levels. |
| Breaker | Cannot Slingshot usefully (20 m line). Honest route is the ladders. Removes rings via clamps. |

## Appearance

Silhouette: tube-and-coupler steel scaffold, not system scaffold. Vertical standards every 2.0 m, horizontal ledgers every 2.0 m of height between the 8 m deck lifts, diagonal braces on every third bay. Decks are planks over steel ledgers (x1.2 surface), outer rail 1.1 m continuous, inner rail 1.1 m with 2 m gaps every 6 m, toe-boards everywhere; the inner edge is the Well mouth at r 16.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Standards, ledgers | galvanized steel tube, 48 mm | #8A8F93 | 0.55 | 0.9 |
| Couplers | drop-forged steel, oiled | #3C3F42 | 0.45 | 0.85 |
| Boards | hardwood, grey-weathered | #8C7355 | 0.85 | 0.0 |
| Toe-boards | steel, Office yellow | #F2C500 | 0.6 | 0.7 |
| Ties to Stump | Weave-faced steel plate | #1B1F26 | 0.3 | 0.6 |

Weathering: galvanizing dulled to matte by sixty years of salt air. Rust streaks only below couplers. Boards split at the ends, scorched where Cutters were tested against them. Stencils: contractor codes `HTR-SCF-04` on every fourth standard, Office inspection tags (white plastic, dated) zip-tied to ladders. Chalk tallies on the inner standards where Ascendants record perch altitude. No lights on the Scaffold itself; it is lit from the Rim's broadcast towers and from below by the Well's blue work-lights, so the inner edge is always backlit.

Read by tier: from the Rim, a dark lattice against the paler Stump. From inside the Scaffold, a grid of 2 m cells; the yellow toe-board line marks the Well edge at every level, 4 m from the Stump face. From the Crown, a ring of boards with the Maintenance Ring's rail circling outside it.

## Variants

1. **Standard bay.** As above. Decked. 260 kg.
2. **Ladder bay.** Four caged ladders at azimuths 45, 135, 225, 315, r 20 to 23, Z 6 to 38, with a 3 m x 3 m grated landing at every level (arena-unique). Climb 2.0 m/s, Breaker 1.6 (05 section 2.3). The honest route.
3. **Undecked bay.** Ledgers and braces only, no planks. Walkable only on tube. Used on L5's outer ring and in the west quadrant after the Collapse. 160 kg.
4. **Tie bay.** Carries a maintenance clamp tie to the Stump. Visually heavier; yellow-and-black tape on the tie plate (see `environment/structural-tape.md`).

## Physics

- Tier: Medium (260 kg per bay). Fragments per 05 section 6.5: five Light boards (18 kg each), Light clamps (1.2 kg, 10 s lifetime), and one Medium frame piece (a 2.5 m standard-and-ledger L, 40 kg; under 100 kg so it gets the Scrapper's 30% bonus).
- Anchoring: each bay is clustered to its neighbors and to its ring's tie bays. A ring is anchored by its maintenance clamps. Sections are static to lines: a Haul on a scaffold tube sings and nothing moves (static to static per 05 section 8).
- Fracture threshold: 800 N s per 05 section 6.5. Worked: a 180 kg fuel drum at 4.5 m/s (810 N s) breaks a bay. A 45 kg propane blast at 2 m delivers 750 N s, so a blast at 1.5 m or closer does it. A Breaker (220 kg) landing from a 3 m Slingshot at 7.7 m/s delivers 1,690 N s and goes through the deck.
- A player standing on a bay that fractures falls one level (8 m, 12.5 m/s) onto planks (x1.2, Stall at 14.4 m/s): a flip, not a Stall. Two levels is 16 m, 17.7 m/s: scrapped. From L1 to the bowl floor is 14 m onto concrete, 16.6 m/s: scrapped.
- Dogs seated in a shattered bay reseat in the largest fragment (05 section 3.5).

## Interactions

- **Haul** player to a standard: a grapple, the standard route for non-Climbers between levels. Haul a board (after fracture) to you: a 18 kg board arrives at up to 25 m/s, 450 N s, a stagger.
- **Jack** from a standard: a Slingshot. The Kick (05 section 3.2, the Jack's launch phase) gives the player the full class V against a static standard: 18 m/s for a Climber, 10 for a Breaker. Standards are the most common Kick surface in the game.
- **Hang** between two standards across a plank run: the Clothesline. Standards are 2 m apart so a line at 1.2 m blocks a bay exactly. The Gallows from the deck above (8 m up) hoists by Reel (05 section 3.2): hold Haul inside Hang and the rod shortens at 1 m/s under the class Hang load, 8 s to the ledger; only a Breaker's Haul lifts a Climber outright. A line across the Well mouth above Z 38 is severed by the Maintenance Ring's drop ladder on its next pass (05 section 3.5: kinematics sever any line they pass through).
- Chain: a fractured bay drops its boards onto the level below; each board is a 300 N s stagger if it lands on a head. A fuel drum rolling off a broken deck arms itself on the next level.
- Riding: the Scaffold does not move except during the Collapse, when the failing quadrant is parented to a scripted sag.

## States

| State | Visual |
|---|---|
| Intact | Boards flat, couplers tight. |
| Stressed | Only during the Collapse siren: the quadrant sags 0.4 m at the outer edge, boards tilt, dust falls from couplers. 5 s. |
| Failing | Couplers spark, standards bow, boards slide off the inner ledger. 1.0 s. |
| Failed | Bay gone. Standards above hang as bent stubs. Boards and clamps on the level below for 10 s, then gone. |

## VFX and SFX hooks

- `SFX_Scaffold_Creak` on any player landing above 5 m/s on a bay.
- `SFX_Scaffold_Fracture` plus `VFX_Scaffold_Splinter` (board splinters, rust dust) on fracture.
- `SFX_Scaffold_Collapse_Siren` and `VFX_Scaffold_Sag` for the event, 5 s lead per 05 section 11.
- Boards falling on steel: `SFX_Board_Clatter`, pooled.

## Technical

- LOD0 1,800 tris per bay, LOD1 900, LOD2 300, LOD3 impostor card. Instanced static mesh for intact rings, swapped to geometry collection on first damage within 10 m.
- One 2K trim sheet (`T_Scaffold_Trim`) for all steel, one 2K tiling for boards, one 1K decal atlas for tags and chalk.
- Pivot at the bay's inner-lower corner. Sockets: `Standard_NE/NW/SE/SW` (Dog seat hints), `Ledger_2/4/6` (intermediate ledgers), `Tie` on tie bays. Ladder bays use the arena-unique ladder actor.
- Chaos: geometry collection per ring. Cluster level 0 = ring (anchored by clamp anchor field), level 1 = bay, level 2 = boards and tube pieces. Damage threshold at level 1 from the 800 N s figure via the project impulse-to-strain scalar. Anchor field: one per ring, keyed to its maintenance clamps; releasing the field releases level 0.
- Collision: `Kinetic_WorldStatic` while intact, `Kinetic_Prop` for fragments. Dogs seat on all channels except `Kinetic_NoDog`.

## Reference search terms

tube and coupler scaffold, contractor scaffold ring around chimney, cooling tower scaffold, scaffold boards weathered, drop forged coupler, scaffold inspection tag.
