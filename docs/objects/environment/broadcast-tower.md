# Broadcast Tower

One-line: environment, destructible, Heavy head (1,800 kg) on a static mast, 24 m lattice tower with a LayCast dish, a camera cluster, and a 6-lamp flood ring at 18 m, the Anchor Rim: six towers at 60 degree spacing, each lighting a 60 degree sector of the Scaffold; also two non-destructible towers on the Yard's control shack side and the Hangtown east rim.

## Role in play

On the Anchor the towers are the lights (03: towers "kill the lights in a sector and make lines the only illumination there, which is the point"). Each tower's flood ring lights one sector of the Scaffold and the Stump face behind it. A tower comes down when its head takes 3,000 N s (**PROPOSED**; it is a tower-class destructible, between catwalk and large house) from a Pulse-thrown Heavy object, a crane counterweight slab, or a Scaffold ring sheet, or when a Breaker Hauls its guy anchor (800 N bolted class) and the tower tips. Its sector goes dark. In the dark sector the Visibility Clause carries the whole fight.

| Harness | Relationship |
|---|---|
| Climber | The dish platform at 18 m is the Rim's one perch above the Scaffold's L5. |
| Scrapper | Darkens a sector cheaply: a Pulse-thrown fuel drum into the head is 180 kg at 18 m/s, 3,240 N s. |
| Rigger | Works best in the dark: set lines brighten within 5 m of a player (05 section 11), so a dark sector is a Rigger's sector. |
| Doubler | Camera lines. The Fray plays to the LayCast camera clusters and the pirate feed's glyphs are on the tower legs. |
| Breaker | Tips a tower by its guy anchor. Slow, loud, certain. |

## Appearance

Silhouette: a four-leg lattice tower with a 3 m LayCast dish at the top angled at the sky (the broadcast uplink), a camera cluster on a boom at 20 m, a ring of six floods at 18 m aimed inward at the Scaffold, a cable bundle down one leg to an equipment container at the base, three guys to anchors on the Rim deck. LayCast teal on the dish and the container; Office grey lattice.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Lattice | galvanized angle | #6E7378 | 0.55 | 0.8 |
| Dish | composite, LayCast teal with white logo | #00A9A5 / #F5F5F0 | 0.5 | 0.1 |
| Camera cluster | black housings, red tally lights | #1A1A1C, #FF2A1A emissive | 0.4 | 0.5 |
| Flood ring | as floodlight mast heads | #1A1A1C, 4000 K | 0.4 | 0.6 |
| Equipment container | LayCast teal, sponsor banners | #00A9A5 | 0.6 | 0.6 |
| Guys and anchors | 20 mm wire, bolted anchors, yellow-and-black tape on the live anchor | #6E7378, #F2C500 / #141414 | 0.4 | 0.9 |

Weathering: salt fog on everything, the dish's logo chalked by sun on its upper half. The camera cluster's housings are new; LayCast replaces them every season. Markings: `LAYCAST  ANCHOR  TOWER 4  UPLINK` on the container; sponsor banners on the container's faces (see `sponsor-banners.md`); a red-match tally sticker from the Fray on the leg the pirate camera uses; the season winner's colors on a pennant at the dish (04: the Anchor's prize is colors on the Rim cranes and the board; the pennant is a **PROPOSED** addition). Lighting: the flood ring (6 lamps, 90,000 lm) aimed inward; camera tally lights red when a cluster is live on the feed (cosmetic, driven by the replay director); the equipment container's two end screens show the crew scoreboard in the canonical team tints (05 section 11a: Amber #E69F00 and Sky #56B4E9, Rose #CC79A7 reserved for the post-launch 3v3v3 crew, Bone #E8E6E0 for neutral), clamped below line brightness; dish obstruction light at the top, red.

Read by tier: from the Crown, six teal dishes in a ring far below. From the Scaffold, a rake of light from your sector's tower and a dark lattice behind it. From the Rim, a 24 m tower at your shoulder with a container you can hide behind.

## Variants

1. **Rim tower, live (the Anchor).** One guy anchor live (800 N), head destructible. Two per round of six (**PROPOSED**), chosen opposite the Collapse quadrant.
2. **Rim tower, fixed (the Anchor).** All guys bolted (no break-free), head still destructible by impact.
3. **Uplink tower (the Yard, Hangtown).** No flood ring (the masts light those maps). Indestructible. Dish and cameras only.
4. **Fallen.** Tower across the Rim bowl as a 24 m lattice, dish shattered, container intact.

## Physics

- Mast: static world. Head (dish, cameras, flood ring): a 1,800 kg Heavy cluster on the mast top. Head fracture 3,000 N s (**PROPOSED**): the ring and dish shear off and fall 18 m to the Rim bowl (18.8 m/s, scrapped under it, 6 m debris ring), the sector's lamps die as they fall.
- Guy tip (live variant): the live guy anchor's break-free is 800 N (a Doubler's single line, a Breaker, or lines summing to 800 N on the axis). Commit 800 N sustained 0.6 s (the authored-consequence spec applies although the anchor is not Rated tier; see README, non-rated consequences). 1.0 s groan (the tower sways 0.6 m at the top, guys sing), then the tower tips away from the pulled guy on a 3.0 s timeline and lands across the bowl toward the Stump. The head arrives at 12 m/s. Anyone under the 24 m fall line is scrapped. The sector goes dark at the 1.0 s mark (lamps cut by the cable bundle tearing).
- Pulse: the tower is static and unaffected. Loose objects on the Rim are thrown toward the Rim by the Pulse (see `force-volumes.md`); towers are placed 8 m inside the Rim edge so the Pulse throws things past their bases, not into their heads. The head is reached by objects thrown off the Scaffold's outer edge at L4 and L5 heights, which is the authored way a Pulse kills a tower.
- Dark sector: `Lighting.Sector.Anchor_<n> = Off`. Confirm line emissives read at full in the darkened sector before anything else is tuned (03 engine notes). The Well's blue work-lights and the Stump's hum-driven iridescence stay on; they are not in any sector.

## Interactions

- **Haul** the live guy anchor across its axis (Breaker, Doubler): the tip. The Breaker stands 6 m from a tower that falls away from them.
- **Jack** off a tower leg: a Slingshot across the Rim.
- **Hang** from the camera boom to an enemy on the Rim, then Reel: a 20 m Gallows at 1 m/s with the broadcast camera watching from 2 m away. LayCast replays this one a lot. A tipping tower is kinematic for its fall and severs every line it passes through (05 section 3.5).
- Chain: a tower across the bowl is a 24 m lattice bridge from the Rim toward the Scaffold base, 1.2 m wide on its top chord: a new route for Breakers and a new Clothesline site for everyone else.

## States

| State | Visual |
|---|---|
| Intact | Lit, tallies blinking, dish steady. |
| Stressed | Live guy under line: tower top sways, guys sing, dish wobbles, floods flicker. |
| Failing | Groan, cable bundle tears (sparks down the leg), sector dark, tower tipping. |
| Failed | Across the bowl. Container at the base still lit (its own power), banners intact: the one lit object in a dark sector, 2 m tall, and not brighter than a line. |

## VFX and SFX hooks

`SFX_Tower_Guy_Sing`, `SFX_Tower_Lattice_Groan`, `SFX_Cable_Bundle_Tear`, `SFX_Tower_Fall`, `SFX_Dish_Shatter`; `VFX_Bundle_Sparks_Down_Leg`, `VFX_Flood_Die_Sequence`, `VFX_Dish_Fragments`; `Lighting.Sector` toggle; Marshal: "Tower four's down. Sector's dark."

## Technical

- LOD0 16,000 tris, LOD1 6,000, LOD2 2,000, LOD3 lattice card and dish disc.
- Textures: `T_Office_Lattice_Trim` 2K shared, `T_LayCast_Dish` 2K, `T_Camera_Cluster` 1K, `T_LayCast_Container` 2K, `T_Sponsor_Atlas` 4K.
- Pivot at the base center. Sockets: `Head`, `Dish`, `Camera_Boom` (replay camera mount), `Flood_0..5`, `Guy_0..2`, `Guy_Live_Anchor`, `Container`, `Pennant`.
- Implementation: tip is a timeline (deterministic) about the base hinge; the head fracture is a geometry collection with level 0 head cluster and level 1 dish, ring, and cameras. Sector tag on the lighting manager. The camera boom socket is a real replay camera the director can cut to.
- Collision: `Kinetic_WorldStatic` mast; `Kinetic_Heavy` head fragments; fallen tower `Kinetic_WorldStatic`.

## Reference search terms

outside broadcast uplink tower, lattice tower satellite dish, stadium camera gantry tally light, guyed lattice mast tipping, broadcast compound container teal livery.
