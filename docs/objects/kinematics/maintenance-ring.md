# Maintenance Ring

One-line: kinematic, infinite mass, a ring gantry at Z 42 on the Rim datum spanning r 14 to 20 (6 m wide steel grating, 1.2 m rails inside and out, two 3 m rail gaps as boarding points at opposite azimuths, a chain drop ladder on the inner rail reaching down to Z 38.5), 3.5 m above the L5 deck (Z 38), 2 m outside the Stump face (r 12), the Anchor: one revolution every 120 s, clockwise seen from above, per `arenas/the-anchor/layout.md`.

## Role in play

The Ring is the Anchor's sweeper. At 0.89 m/s it is too slow to kill by impact (the kinematic stagger threshold is 4 m/s). What it sweeps is the Well mouth and the Stump face between Z 38.5 and 48: every Pendulum, every Hang across the mouth, every Climber perched on the bare Weave above L5, and every line in its path, because a kinematic severs any line it passes through (05 section 3.5, canon). Its drop ladder passes a given azimuth once per revolution at 0.73 m/s and takes whatever is hanging there with it, over the Well. Ride it or be swept by it (03). It is also the only moving platform that passes every quadrant of the Stump above L5, so it is the Climber's taxi and the Breaker's one way around the Stump without the Crown ladder.

| Harness | Relationship |
|---|---|
| Climber | Rides the inner rail as a moving Slingshot origin that circles the Stump 18 m below the Crown lip: the Ascension launch point when the Breath is full. |
| Scrapper | Lines props to the Ring and lets it drag them around the Well mouth as a slow flail. |
| Rigger | A set line across the Well mouth above L5 lasts until the Ring's next pass severs it (snap, both Dogs release). A Clothesline between the Ring's own rails rides with it: lines attached to the kinematic itself are not severed by it. The Ring's deck is the Anchor's best overwatch. |
| Doubler | Commits a maintenance clamp from the Ring as it passes (see `rated/maintenance-clamp.md`). |
| Breaker | Rides it braced. The only way a Breaker crosses the Stump's far side in under a minute. |

## Appearance

Silhouette: a lattice ring girder with a 6 m grating deck, four bogies on a circular rail carried by the Scaffold's outer standards, 1.2 m rails inside and out, two 3 m boarding gaps, a chain drop ladder hanging from the inner rail 3.5 m down to L5 deck height over the Well mouth, a tool rack, a hose reel, and HTR's inspection camera. Orange, like everything HTR brought.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Ring girder | lattice steel, HTR orange | #E8541E | 0.55 | 0.6 |
| Deck | grating | #5E6366 | 0.6 | 0.9 |
| Drop ladder | chain and steel rungs, orange with a yellow-and-black bottom rung | #E8541E, #F2C500 / #141414 | 0.55 | 0.6 |
| Bogies | cast steel, grey | #6E7378 | 0.5 | 0.8 |
| Rail | machined steel, bright | #B3B6B8 | 0.3 | 1.0 |
| Camera and lamps | black housings, 4000 K lamps | #1A1A1C | 0.4 | 0.3 |

Weathering: salt fog on everything, orange chalked pale on top faces. The drop ladder's bottom rung is polished by Climbers' hands. Markings: `HTR  STUMP RING  INSPECTION UNIT 1  KEEP CLEAR OF ARM` in white on the inner girder face; Ascendancy chalk on the deck grating, a spiral of altitudes; a Guild tag on the hose reel reading `NOT OURS`. Lighting: two 4000 K lamps under the inner rail aimed down the Stump face ahead of the drop ladder (the Ring lights its own approach; this is the visual tell), a red beacon, deck strips. Its shadow sweeps each deck for 10 s per pass.

Read by tier: from the Rim, an orange ring turning slowly above the Scaffold's top, its lamps sweeping. From the Scaffold, a moving ceiling 3.5 m over L5 and a chain ladder coming round over the Well mouth. From the Crown, the ring 18 m below, turning.

## Variants

1. **Standard.** Drop ladder down to Z 38.5 over the Well mouth.
2. **Over the west void (post-Collapse).** The Scaffold Collapse removes the west quadrant (azimuth 225 to 315); for 30 s of every revolution the Ring's outer rail is over a 42 m drop to the Rim bowl. The Ring itself is unchanged.
3. **Season colors (cosmetic).** Beacon color matches the Claim-holding faction.

## Physics

- Kinematic, infinite mass. Angular speed 3 degrees per second. Deck speed 0.89 m/s at mid-width (r 17), 0.73 at the inner rail (r 14), 1.05 at the outer rail (r 20).
- Impact: the drop ladder at 0.73 m/s against a player is under every threshold (stagger at 4 m/s). It does not hurt. It moves them: a Climber perched on the Stump face at Z 40 to 48, or a player hanging on a Pendulum through the Well mouth, is pushed sideways at 0.73 m/s over the mouth and must re-dog or fall. A player on L5 is under the Ring with 1.6 m of headroom and is not touched. This is the sweep.
- Crush: a player between the Ring's inner rail and the Crown ladder cage (Z 40 to 48, 2 m clearance, but the cage bars are 0.3 m from the rail's bogies at the south azimuth): over 8 kN for 0.5 s, scrapped. A player on the drop ladder when it passes the L5 S footbridge (Z 38, r 12 to 16): pinned against the footbridge rail, scrapped. The tape on the bottom rung says so.
- Riders: the deck is a moving platform; players inherit 0.89 m/s tangential velocity. Boarding is a 3.5 m climb from L5 by the drop ladder (2.0 m/s, Breaker 1.6) or a Jack; the boarding gaps are the only places without a rail. Dropping off the outer rail onto L5 is a 4 m fall, fine; off the inner rail is the Well.
- Lines: a Hang from the Ring to a player on L4 is a rigid rod that drags them around the Stump at 0.9 m/s and over the L4 inner edge in under 10 s. Slow, visible, and a line they can cut at leisure: the Ring is the Anchor's "tell everyone" kill. Lines not attached to the Ring that cross its path (any line across the Well mouth between Z 38.5 and 44, any line from the Stump face to a deck through that band) are severed on contact: the snap plays, both Dogs release, and whoever was hanging on it falls.
- Updraft: the Ring is unaffected (kinematic). Pulse: unaffected. Players on the Ring are affected by both.

## Cycle and tells

120 s revolution, clockwise viewed from the Crown, starting at the north quadrant at round Live.

| Time | Position | Tell |
|---|---|---|
| 0 s | North (0 degrees). | Lamps on the deck ahead. |
| 30 s | East (90 degrees). | Rail click at each bogie joint, every 7.5 s. |
| 60 s | South. | |
| 90 s | West. | |
| 120 s | North. | |

Audio: `SFX_Ring_Rail_Loop` (a motor drone, spatialized to the Ring's nearest point), a bell at each ladder bay (azimuths 45, 135, 225, 315 at 15, 45, 75, 105 s), `SFX_Ring_Ladder_Chain` (chain rattle, 3 s lead) as the drop ladder approaches. Visual: the lamps light the Stump face 6 m ahead of the ladder (8 s of warning at 0.73 m/s), the red beacon, the shadow sweeping each deck for 10 s. Lead: 8 s by lamp, 3 s by chain; the Ring is the slowest tell in the game and the one most ignored. The L5 inner toe-board carries a kinematic path stripe (see `environment/structural-tape.md`).

The Ring never stops. During the Scaffold Collapse it continues, and crosses the west void from 75 to 105 s of a Scrap round.

## Interactions

- **Haul** self to the Ring from L4 or L5: a grapple onto a moving platform (hoisted, then Reel the last meters); you arrive at the deck's speed.
- **Jack** off the inner rail toward the Stump: the Slingshot into the Updraft, 18 m below the Crown lip. The Kick's V is unaffected by the Scrapper bonus and by the Ring's motion; the Ring's 0.73 m/s tangential velocity is added to it. At the Climber's 18 m/s launch speed with the Updraft at peak, this is the Ascension.
- **Hang** enemy to the Ring: the slow drag. Hang self to the Ring's inner rail and step off: a Pendulum around the Well mouth at 0.73 m/s, reeled up or down at 1 m/s by holding Haul or Jack (the Reel, 05 section 3.2). The Pulse ends it.
- Chain: props on the Ring are chained at 800 N (Doubler or Breaker frees them). A prop Hung under the inner rail sweeps the Well mouth as a 0.73 m/s flail; a Gallows line from the Crown down the Stump face is severed on the Ring's next pass, and the hoisted player drops.

## States

Kinematic: no failure. `Running`. The Ring's rail rides the Scaffold's outer standards above L5; L5's clamps are never live (authoring rule; see `rated/maintenance-clamp.md`), and the west-quadrant Collapse leaves the rail standing on the other three quadrants' standards (the rail is a complete ring girder and spans the void).

## VFX and SFX hooks

`SFX_Ring_Rail_Loop`, `SFX_Ring_Bell`, `SFX_Ring_Ladder_Chain`, `SFX_Line_Snap` (severed lines); `VFX_Lamp_Cones`, `VFX_Ring_Beacon`, `VFX_Ring_Shadow_Sweep`, `VFX_Rail_Dust_Line`.

## Technical

- LOD0 42,000 tris, LOD1 16,000, LOD2 5,000, LOD3 torus and arm box.
- Textures: `T_Ring_Girder` 4K, `T_Ring_Arm` 2K, `T_Anchor_Decal_Atlas` 2K.
- Pivot at the Stump axis at Z 42 (the Ring rotates about its pivot; no spline needed). Sockets: `Ladder_Top`, `Ladder_Bottom` (Z 38.5), `Gap_A/B` (boarding), `Deck_0..31` (Dog seat hints every 3.3 m), `Lamp_A/B`, `Beacon`, `Bogie_0..3`.
- Implementation: a single kinematic rotation about Z at 3 degrees per second from server time, zero replication traffic. Moving platform base for riders; kinematic target transforms per physics step for contact velocities. Line severing: a swept volume (the Ring's annulus Z 41 to 44 plus the ladder's r 13.5 to 14.5, Z 38.5 to 42) tests every live and set line segment each step; a line not attached to the Ring that intersects it snaps with the 05 section 4.2 recoil.
- Collision: `Kinetic_Kinematic`.

## Reference search terms

rotating ring gantry inspection platform, chimney inspection ring rail, tank roof rotating crane ring, circular rail bogie, wind turbine tower climbing ring.
