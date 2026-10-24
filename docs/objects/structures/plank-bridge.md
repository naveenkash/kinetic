# Plank Bridge

One-line: structure, destructible, Medium tier, 60 kg, 6.5 m span x 0.9 m wide, three 50 mm planks on two steel bearers, the Yard Floor: crossings over the three 6 m rail trenches (4 m deep), raised on 3.5 m bearer towers so the deck sits at Z 3.5, 0.5 m above a loaded Hauler's plate top (Z +3), per `arenas/the-yard/layout.md`.

## Role in play

Plank bridges are the Floor's only crossings that do not involve a grate or a Hang line. They are the Yard's cheapest trap. A Hauler does not touch them (its plate top passes 0.5 m below the deck), but a player dragged along the trench by a Hauler hits a bearer tower at 12 m/s and is scrapped against static world, and a Hang footbridge strung beside one is severed by the next train (05 section 3.5). A plank bridge is also the easiest way to make a Scrapper's favorite object: a 12 kg plank.

| Harness | Relationship |
|---|---|
| Climber | Ignores it. Slingshots the trench. |
| Scrapper | Breaks it on purpose for the planks. Lines a plank to an enemy's back plate as a drag anchor. |
| Rigger | A Clothesline at the entry, and a second across the trench as a backup footbridge. |
| Doubler | A snap on an enemy mid-plank throws them into the trench 3 s before the next Hauler. |
| Breaker | 220 kg on a 60 kg bridge: bearers bend 40 mm, planks creak, holds. A Breaker's or Doubler's Haul can pull the whole bridge off its seats (800 N break-free). |

## Appearance

Silhouette: three rough planks laid side by side across two rusted channel bearers carried on short lattice towers at each trench lip, with a four-step stair up each end. No handrail. A yellow-and-black Office stencil on the deck: `TRENCH 2  CROSSING 07`.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Planks | softwood, oil-stained | #9A7B4F | 0.85 | 0.0 |
| Bearers | steel channel, rusted | #7A3B1E | 0.75 | 0.6 |
| Seat notches | cast concrete trench lip | #9A9590 | 0.8 | 0.0 |
| Stencil | Office yellow | #F2C500 | 0.7 | 0.0 |

Weathering: planks are grey where they are dry and black where hydraulic oil from the Haulers has misted them. Boot prints in the oil. The bearers are the same channel as the Yard's rail ties: the Guild cut them from stock. The center plank on one in four bridges is a different wood (replacement after a Hauler strike last shift). A chalked Hauler timetable on the concrete beside the busiest crossing, in Guild hand.

Lighting: none on the bridge. Each trench has red Hauler approach lights every 20 m on both lips, which flash 3 s before a Hauler (05 section 11). The lights put a red wash on the bridge underside.

Read by tier: from the Rails (36 m up), thin pale lines across dark trenches, the red lights beside them. From the Stacks, a plank-width gap in the canyon. From the Floor, three boards and a drop.

## Variants

1. **Standard.** 4.5 m, three planks, 60 kg.
2. **Wide.** 4.5 m, five planks, 1.5 m, 95 kg. Spool route for Haul mode. Fracture 900 N s (**PROPOSED**).
3. **Half.** One plank missing. Sprinting across requires a 0.3 m lateral correction. Dressing for the condemned east Floor.
4. **Grate.** Steel grate panel in the same seats, 180 kg, fracture 2,500 N s (catwalk class). Six of these on the Haul spool route so the payload can cross.

## Physics

- Tier: Medium (60 kg). Fracture threshold 600 N s (canon, 05 section 6.5). Fragments: three Light planks (12 kg each, 10 s lifetime unless picked up by a line, which refreshes).
- Anchoring: bearers sit in notches. Break-free 800 N (bolted or chained class, since the bearers are pinned). Only a Doubler's or a Breaker's Haul lifts a whole bridge.
- Worked impacts: a 180 kg fuel drum Hauled onto it at 3.4 m/s is 612 N s and breaks it. A Breaker landing from 1.5 m (5.4 m/s) delivers 1,190 N s and goes through. A Rigger landing from the same height (700 N s) also goes through; a Climber (90 kg, 490 N s) does not. A propane blast at 3 m delivers 375 N s, not enough.
- A player on a breaking bridge falls 7.5 m to the trench floor (12.1 m/s on oiled slag over steel: scrapped), or 3.5 m to the Floor lip (8.3 m/s, a flip) if they were over the stair. The trench is 4 m deep with rungs every 20 m; the mantle height for a Breaker is 1.8 m, so a Breaker climbs out by rung (2.0 m/s class rate, Breaker 1.6) or not at all. The next Hauler arrives on the schedule (see `kinematics/rail-hauler.md`).
- A 12 kg plank Hauled by anyone reaches the 25 m/s cap inside 10 m (33 m/s² on a Scrapper's 400 N) and delivers 300 N s: a stagger, nothing more (05 section 7.1 notes a plank needs 100 m/s to Stall). Its value is as a Hang anchor, a Jack recoil mass, and as a thing to hang off a Climber's back plate.

## Interactions

- **Haul** the bridge (Breaker): it lifts out and comes to you as a 60 kg prop. The trench is now 4.5 m wider for everyone.
- **Jack** from a bearer: a flat Slingshot along the trench, which is a bad idea 3 s before a Hauler.
- **Hang** plank to plank across a trench below Z 3: a rope footbridge for players, until the next Hauler severs it (05 section 3.5, canon: a kinematic severs any line it passes through; the snap plays and whoever was mid-crossing falls into the trench ahead of the train). Above Z 3.5, between two bridge decks, a Hang footbridge survives the trains.
- Chain: a broken bridge drops planks into the trench. A Hauler at 12 m/s hits a 12 kg plank: the plank leaves at up to 24 m/s, 290 N s on anyone it finds. Staggers crowds at trench edges. A Guillotine armed from a bridge deck (a drum Hung over the trench on a set line) drops when the owner recalls or cuts the set line (05 section 10); gravity does the rest.

## States

| State | Visual |
|---|---|
| Intact | Planks flat, stencil legible. |
| Stressed | Under a Breaker: bearers flex, planks lift at the free ends, creak. |
| Failing | Center plank splits lengthwise, bearers twist out of their notches. 0.3 s. |
| Failed | Empty notches, planks in the trench. |

## VFX and SFX hooks

- `SFX_Plank_Creak` scaled by load, `SFX_Plank_Split`, `VFX_Plank_Splinter`.
- `SFX_Bearer_Scrape` on Breaker break-free.
- Uses the trench's `VFX_Hauler_ApproachLight` for the red wash; no own lights.

## Technical

- LOD0 600 tris, LOD1 240, LOD2 60. Instanced across the Floor until damaged.
- Textures: `T_Yard_Timber` 1K, `T_Yard_Steel_Trim` 2K shared, `T_Yard_Decal_Atlas` 2K.
- Pivot at the west bearer tower top. Sockets: `Plank_0..2` (prop spawn), `Bearer_W`, `Bearer_E`, `Stair_W`, `Stair_E`, `Stencil`.
- Chaos: geometry collection, level 0 bridge (anchored by a per-bridge anchor field so it is static to lines under 800 N), level 1 planks and bearers. Damage threshold from 600 N s. On a Breaker break-free the anchor field is cleared and the whole level 0 body is handed to the line as a Medium prop.
- Collision: `Kinetic_WorldStatic` intact, `Kinetic_Prop` planks. Planks carry `Tag.ScrapperBoost` for the 30% Light speed bonus.

## Reference search terms

scaffold plank trench crossing, railway yard pit crossing planks, steel channel bearer, oil stained lumber, demolition site walkway boards.
