# Press Counterweight

One-line: rated load, 9,000 kg steel block, 2.0 m x 1.2 m x 1.6 m, riding in a 14 m guide tower on the east side of the Shear Press pit, the Yard: one, live every round.

## Role in play

The counterweight is the press ram's return mass. It rides up as the ram comes down and down as it rises, on a visible cycle that is the Yard's heartbeat (see `kinematics/shear-press.md`). Pull it out of its guides and three things happen: the block falls 14 m into the tower base, the ram loses its return and slams down to stay, and the press's hydraulic header ruptures and vents across the pit's east lip for the rest of the round. The Well stops ticking and starts pushing.

| Harness | Relationship |
|---|---|
| Climber | The tower top is a mid-tier perch at 14 m with a view down the pit. The moving block is a free elevator for a Climber who Hangs to it. |
| Scrapper | Cannot move it. Rides it on a Hang line to reach the Rails side stair. |
| Rigger | Hangs a Clothesline across the tower's access ladder landing. |
| Doubler | Overspin from the pit's west lip, 18 m across: commits the block and the Doubler's recoil throws them away from the pit. Safe, for a Doubler. |
| Breaker | Owner. Brace on the tower's base slab, Haul the block's guide shoe outward when the block is at the top of its stroke. |

## Appearance

Silhouette: a dense rectangular block with two cast guide shoes on each side running in channel guides inside an open lattice tower, a 60 mm wire rope from its crown over the head sheave to the ram yoke. Yellow-and-black tape in a full band around the block's waist and down both tower legs on the pit side.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Block | cast steel, mill scale and rust | #4B5055 with #7A3B1E | 0.7 | 0.7 |
| Guide shoes | bronze, polished by travel | #9C7A3C | 0.35 | 0.9 |
| Tower lattice | angle steel, Guild red primer | #8C3B2E | 0.65 | 0.5 |
| Rope and sheave | greased wire rope, cast sheave | #3C3F42 / #6E7378 | 0.35 | 0.9 |
| Tape | Office | #F2C500 / #141414 | 0.7 | 0.0 |

Weathering: the block is the one object on the Yard that moves every twelve seconds and has done for forty years. Its guide faces are mirror-bright; its other faces are black with oil mist. The tower legs are worn silver where the shoes pass. Markings: `SHEAR PRESS 1  CWT 9,000 kg  LAY GUILD C4` in white on the block; the Guild's chalk tally of ram cycles this shift on the tower leg (a number in the thousands); HTR `CUT 04` orange on the live guide shoe's keeper. Lighting: the tower carries two red beacons at its top that pulse with the ram cycle; the block's tape is lit by the pit's own floodlights from below, so the tape is brightest when the block is low.

Read by tier: from the Rails, a red lattice beside the dark pit with a block sliding in it. From the Stacks, a landmark; the ram's thud and the beacons give the pit's position from anywhere. From the Floor, a 14 m tower with a taped block rising past you.

## Variants

1. **Live.** Keeper bolts on the pit-side guide shoes cut, tape, lamp. Every round.
2. **Stabilized (dressing only).** Would never ship; the press is the Well's heartbeat. Listed for parity: four bolts, no tape.
3. **Fallen (post-consequence).** Block in the tower base at a 20 degree lean, rope slack over the sheave, header venting.

## Physics

- Tier: Rated attachment per 05 section 6.7. Mass 9,000 kg. Break-free 1,000 N outward from the guides (across the tower, toward the Floor east of the pit). The block is kinematic on the ram cycle (see `kinematics/shear-press.md`) while attached and becomes a simulated rated load when any active pull on it (Haul, Jack push, or Kick; never a Hang) reaches 1,000 N for 0.6 s: a Breaker alone, a Doubler at overspin, or a crew summing to it on the outward axis. After the commit the authored timeline carries it regardless of friction or line. While riding it severs any line it passes through (05 section 3.5).
- Commit: 1,000 N sustained 0.6 s outward, while the block is in the upper half of its stroke (above 7 m). Below 7 m the pull sings and shudders and does not commit: the authored reason is that the lower guides are intact. The window is 5 s of every 12 s cycle.
- Fall: shoes leave the guides, the block drops 7 to 14 m inside the tower, 1.2 to 1.7 s, arriving at 12 to 16.5 m/s on the tower base slab. On a player in the tower base (there is an access door; people hide there): scrapped. Impact ring 3 m outside the tower legs, dust only.
- Ram: with no return mass the ram descends under its own weight and the residual hydraulic pressure, reaching the pit floor in 2.0 s and staying. Anything in the pit's kill volume is already scrapped; the ram landing is the sound.
- Header rupture: the hydraulic header on the tower's pit side splits. It becomes a force volume (see `environment/force-volumes.md`, gas jet class): a 6 m directional jet across the pit's east lip at 15 m/s for the rest of the round, not 3 s. It pushes toward the pit. A Climber (foot-hold 3) at the lip is airborne and in the pit in 0.8 s. A braced Breaker (20 m/s²) holds; an unbraced one (5) does not. The east lip, previously the Well's safest approach, is now its worst.
- Rope: the 60 mm rope goes slack and falls across the Floor east of the tower in a 14 m line. It is Heavy (420 kg) on a bare floor, fixed scenery per 05 section 6.1a, and a Hang anchor.

## Authored consequence

| Time | Event |
|---|---|
| 0.0 s | Commit. Lurch: the block's pit-side shoes jump the guide lip 30 mm, keeper bolts shear. Beacons go solid red. |
| 0.0 to 1.0 s | Groan, 1.0 s lead. The tower lattice rings, the rope judders on the sheave, the ram pauses at top of stroke (the press's control logic faults). Marshal: "Press counterweight's out of its guides. Mind the east lip." |
| 1.0 s | Release. Block falls. |
| 2.2 to 2.7 s | Base impact. Tower legs splay 0.2 m. Dust. |
| 2.7 to 4.7 s | Ram descends and seats on the pit floor with the loudest single sound on the map. Beacons out. |
| 4.7 s | Header splits. Jet volume on. `VFX_Hydraulic_Mist` across the east lip, continuous. Pit floodlights flicker to half. |
| After | The pit is silent for the first time. The east lip has a 6 m x 4 m jet pushing toward it at 15 m/s. The Gantry still drops its load in the pit on schedule; it now lands on the seated ram, bounces (a 7,500 kg body at 22 m/s against a kinematic) and can leave the pit at up to 8 m/s onto the Floor (**PROPOSED**: authored bounce direction west, scrap ring 6 m). |

What breaks: keeper bolts, guide lip, header, the ram's cycle, pit lighting. Reset between rounds.

## Interactions

- **Haul** the pit-side shoe outward from a brace at the tower base with the block high: commit. The Breaker is 3 m from a 9,000 kg block about to fall inside a lattice 1 m from them. The lattice holds; the design makes the Breaker's position safe by authoring and says so with the tape (tape on the legs, not the base slab).
- **Jack** off the moving block: a Slingshot that inherits 0 m/s (kinematic, infinite mass) but whose direction changes with the block's height. The 14 m tower is a vertical launch rail.
- **Hang** to the block and ride it: 14 m in 4 s on the ram's rise. Release at the top for the Rails stair. Stay too long and the block comes down with you on it at 3.5 m/s into the pit's lip zone (under the 4 m/s kinematic stagger; it is a safe ride, which is why it is a route).
- Chain: the jet volume catches fuel drums at the east lip and throws them into the pit; nothing comes back. A Claim marker Hauled into the jet goes into the Well and resets (05 section 9.4).

## States

| State | Visual |
|---|---|
| Intact | Block cycling, beacons pulsing, tape clean. |
| Stressed | Shoes chatter on the guides, beacons stutter, rope hums. |
| Failing | Groan, ram paused, block falling. |
| Failed | Block canted in the base, rope slack across the Floor, ram seated, mist across the lip. |

## VFX and SFX hooks

`SFX_CWT_Shoe_Chatter`, `SFX_CWT_Keeper_Shear`, `SFX_Tower_Ring`, `SFX_CWT_Base_Impact`, `SFX_Ram_Final_Seat`, `SFX_Header_Split`, `SFX_Hydraulic_Jet_Loop`; `VFX_Tower_Dust`, `VFX_Hydraulic_Mist`, `VFX_Beacon_State`; Marshal VO; force volume spawn; press cycle stop event.

## Technical

- LOD0 block 2,200 tris, tower 11,000; LOD1 half; LOD2 quarter; LOD3 box and lattice card.
- Textures: `T_Press_CWT` 2K, `T_Press_Tower` 4K, `T_Yard_Decal_Atlas` 2K.
- Pivot at the block's crown rope eye. Sockets: `Shoe_PitSide_Upper` (Haul target), `Shoe_0..3`, `Rope_Eye`, `Beacon_A/B`, `Header`, `Jet_Origin`, `BaseRest`.
- Implementation: the block is a kinematic on the press timeline (shared clock with the ram). The consequence hands it to a second timeline (fall to `BaseRest` with a 20 degree lean pose), stops the press clock, plays the ram's final descent, spawns the jet volume actor with an infinite duration, and swaps the rope spline to its slack pose. All server time-synced.
- Collision: `Kinetic_Kinematic` on the cycle, `Kinetic_RatedLoad` for the commit query only, `Kinetic_WorldStatic` fallen.

## Reference search terms

hydraulic shear press scrap yard, counterweight guide tower elevator, bronze guide shoe, wire rope sheave head frame, scrap baler ram pit.
