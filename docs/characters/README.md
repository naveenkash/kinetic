# KINETIC Characters: Index and Shared Rules

_Art and design specifications for every body, harness, tool, and NPC in KINETIC. Every number that touches gameplay comes from `docs/bible/05-core-mechanics.md`. Anything invented here is marked **PROPOSED**. Each document stands alone; this file holds what they share._

## Index

### Harness classes (`harnesses/`)

| File | One line |
|---|---|
| [climber.md](harnesses/climber.md) | Ascendancy. 90 kg. Mirror plates, white wraps, bracelet Spool. Longest line, lightest body, never on the floor. |
| [scrapper.md](harnesses/scrapper.md) | Hangtown Crews. 110 kg. Mismatched salvage plates, modded Spool, instant recall. Fastest Dogs and Cutter. |
| [rigger.md](harnesses/rigger.md) | Lay Guild. 130 kg. Canvas and brass, square shoulders, spine frame. Two set lines, stiffest Hang. |
| [doubler.md](harnesses/doubler.md) | The Fray. 130 kg. Twin-drum Spool, neon on black, filed serials. Two active lines, overspin. |
| [breaker.md](harnesses/breaker.md) | Halvard-Teague Reclamation. 220 kg. Orange and grey slab, low Spool, blade shoulders. Moves rated loads. |

### Faces (`faces/`)

| File | One line |
|---|---|
| [teodora-quispe.md](faces/teodora-quispe.md) | "Mother Line." Guild Wright, 72, Rigger. Runs the apprenticeship. Knows who signed the harmonics logs. |
| [baskoro-hanif.md](faces/baskoro-hanif.md) | "Hitch." Guild Lead, 29, Rigger. Counts his lines on the feed. Sister fights for the Fray. |
| [rook-adeyemi.md](faces/rook-adeyemi.md) | "Wreckingball." HTR star, 41, Breaker. Ex-Guild. Walks at the thing holding the floor up. |
| [mei-lin-haldane.md](faces/mei-lin-haldane.md) | "The Surveyor." HTR, 24, Breaker. Calls the failure before it happens. Her models match the Guild's. |
| [paloma-reyes-okafor.md](faces/paloma-reyes-okafor.md) | "Swift." Ascendant, 31, Climber. Full Anchor match without touching the ground. |
| [tendai-mutasa.md](faces/tendai-mutasa.md) | "The Verger." Ascendancy strategist, 52, Climber. Jacks between two enemies. Keeps the evidence high. |
| [jesus-vilca.md](faces/jesus-vilca.md) | "Fuse." Hangtown lead, 36, Scrapper. Times barrels. Furious at condemnation notices. |
| [dela-santos.md](faces/dela-santos.md) | "Kite." Layborn prodigy, 17, Scrapper. Pirate streamer. Fastest Cutter measured. |
| [nnamdi-ferreira.md](faces/nnamdi-ferreira.md) | "Snap." Fray red-match veteran, 33, Doubler. Missing two fingers. Overspins everything. |
| [odile-lacroix.md](faces/odile-lacroix.md) | "The Auctioneer." Runs the Fray's book, 45, Doubler. Takes bets mid-fight. Bought evidence from everyone. |

### Equipment (`equipment/`)

| File | One line |
|---|---|
| [spool.md](equipment/spool.md) | The forearm device. Housing, drum, Dog cradles, line exit, class variants, spin states. |
| [line.md](equipment/line.md) | The live filament. Emissive and pitch curves against newtons, slack and set states, snap VFX, dark line. |
| [dogs.md](equipment/dogs.md) | Standard, Barb, Mag, Spike. Seating animation into steel, concrete, wood, rock, and Catch Plates. |
| [cutter.md](equipment/cutter.md) | Standard, Hot, Long. Ceramic blade, heat ramp, draw and swing, the snap. |
| [catch-plates.md](equipment/catch-plates.md) | The four dog-able plates. Dimensions per class, seat sockets, rim lighting, line exit. |
| [harness-shared-components.md](equipment/harness-shared-components.md) | Stall Gel system, telemetry pack, lease plate and serial, tape wraps, tallies. |

### NPCs (`npcs/`)

| File | One line |
|---|---|
| [marshal.md](npcs/marshal.md) | Office referees in the overhead gallery. The match's voice. Bored, fair. |
| [hook-drone.md](npcs/hook-drone.md) | Recovery drones below the lowest floor. Rotors are the ring-out tell. |
| [prep-crew.md](npcs/prep-crew.md) | Office staging crews. Place charges, tape rated loads, paint tallies. |
| [reserve-rigger.md](npcs/reserve-rigger.md) | The crew reserve that drops when you respawn. How the variant differs per faction. |
| [bot-crew.md](npcs/bot-crew.md) | The tutorial HTR Probationer crew in the Yard. |

---

## Silhouette readability rules

The silhouette tells the enemy how a line will behave with you. These rules bind every harness, every Face, every livery, and every reserve variant.

1. **Mass reads at 50 m from any angle.** Five masses, five silhouettes, ordered by visual width at the shoulders: Climber 0.48 m, Scrapper 0.54 m, Rigger 0.62 m, Doubler 0.62 m, Breaker 0.82 m (**PROPOSED** target widths, harness included). Rigger and Doubler share mass and width; they are told apart by the Doubler's twin-drum right forearm and the Rigger's spine frame.
2. **Bulk location is class-locked.** Climber: no bulk. Scrapper: low, asymmetric, left hip. Rigger: shoulders and spine. Doubler: right forearm. Breaker: shoulders and chest, everywhere.
3. **The Spool is always on the right forearm.** Its position on the forearm is class-locked: Climber at the wrist, Scrapper and Rigger mid-forearm, Doubler the full forearm, Breaker low against the elbow. The Cutter is always in the left hand.
4. **Catch Plates are never hidden.** No livery, patch, cloth, or prop may cover more than 15% of any plate. Plate rims carry a 1 cm light band in team tint at all times. Plate base value stays between 25% and 70% luminance so a Dog and a line exit read against it.
5. **Team tint owns the light.** Every emissive on a player is team tint: line, Spool rim ring, plate rim bands, shoulder lamp, Dog trails, HUD crew markers. Set lines and neutral elements render Bone. Enemy lines additionally carry a 1 m dark tick pattern so team is never carried by color alone (05 §11a). Faction livery never uses a team-tint hue above 40% saturation. Line glow color is team-locked by the Visibility Clause and is not customizable.
6. **Livery changes surface, never shape.** Customizable: paint, tape pattern, decals, patches, head, under-layer cloth within 2 cm of the body, Dog finish, Cutter handle. Locked: plate dimensions, Spool housing volume, spine frame, shoulder profile, boot height, anything that alters the width or height of the silhouette by more than 2 cm.
7. **Three-tier cap test.** From above (a Climber's view), each class must be distinguishable by shoulder footprint alone. From behind, by back plate shape and Spool arm. Verify both at 50 m against every arena's mid tier before a livery ships.
8. **Faces wear the class.** A Face is a head, a voice, a tape pattern, personal wear, and a Drop. A Face never changes a plate dimension.
9. **Reserve variants obey everything above.** A reserve livery is a second surface treatment on the same mesh.

### Team tints (canonical, 05 §11a)

| Slot | Name | Hex | Used in |
|---|---|---|---|
| Team A | Amber | `#E69F00` | 3v3 side A |
| Team B | Sky | `#56B4E9` | 3v3 side B |
| Third crew | Rose | `#CC79A7` | 3v3v3 side C |
| Neutral | Bone | `#E8E6E0` | Set lines, neutral markers, Drop-phase rim bands |

Chosen from a colorblind-safe palette. Enemy lines carry a 1 m dark tick pattern in addition to their tint. Line glow, Catch Plate rim bands, HUD crew markers, and Dog trails use these and nothing else. Line core renders at the tint's white-shifted value under force (see `equipment/line.md`).

Validation items: Amber against HTR orange `#FF5A1F` and Guild ochre `#C8862B` relies on the rim band's emissive and the core's white shift, not on hue; confirm at 50 m on the Yard and Ballast. Free Scrap (8 solo) has no canonical palette: **PROPOSED** the local player renders Amber and every other player Sky with the enemy tick.

### Faction palettes

| Faction | Primary | Secondary | Accent | Soft goods |
|---|---|---|---|---|
| Lay Guild | Ochre `#C8862B` | Black `#141211` | Brass `#A67C3B` | Canvas `#B9A98A` |
| HTR | Orange `#FF5A1F` | Matte grey `#5A5E62` | Dark grey `#2B2E31` | Logo white `#F2F2F0` |
| Ascendancy | Wrap white `#F4F1EA` | Mirror steel (metallic 1.0, roughness 0.08) | Silver tape `#C9CDD2` | Pale sky `#A9C7E8` (**PROPOSED**) |
| Hangtown Crews | Any salvage base | Stencil teal `#3DA38C` (**PROPOSED**) | Sun yellow `#E8C23A` (**PROPOSED**) | Rust `#7A3B22` |
| The Fray | Black `#0E0E10` | Neon green `#39FF14` | Neon magenta `#FF2BD6`, neon yellow `#FFE600` | Heat-scored steel (blue-brown gradient) |

Hex values for the Guild, HTR, Ascendancy, and Fray primaries are derived from the colors named in the faction bible and are **PROPOSED** as exact values.

---

## Shared skeleton and rig spec

### Skeleton

- **Asset:** `SK_Rigger`. One skeleton for all five classes and all ten Faces. UE5 Mannequin (Manny/Quinn) bone hierarchy and naming, so all retargeting and marketplace mocap works unchanged.
- **Scale:** capsule 1.9 m tall, 0.4 m radius, eye height 1.7 m, as per 05 §2.1. Class and Face height differences are mesh and retarget-pose offsets inside the same capsule; the capsule never changes.
- **Class proportion:** handled by a per-class retarget pose and per-class harness mesh, not by bone scale. Bone lengths are identical across classes so animations share without foot sliding.
- **Faces:** head mesh swap on `head`, body-mesh material swap, hands shared per class. A Face's hands may carry unique material (scars, missing fingers via a mesh variant on `hand_r` for Nnamdi only).

### Sockets

Every class defines every socket below with the same name. Transforms differ per class; the class docs give the offsets.

| Socket | Parent bone | Purpose |
|---|---|---|
| `S_Spool` | `lowerarm_r` | Spool housing root |
| `S_Spool_LineExit` | `lowerarm_r` | Line origin and Dog muzzle. Line meshes spawn here. |
| `S_Spool_DogA_Rest`, `S_Spool_DogB_Rest` | `lowerarm_r` | Docked Dog positions on the housing |
| `S_Spool_DogC_Rest`, `S_Spool_DogD_Rest` | `lowerarm_r` | Doubler only, second drum |
| `S_Spool_LineExit_2` | `lowerarm_r` | Doubler only, lower drum line origin |
| `S_Cutter_Hand` | `hand_l` | Cutter grip when drawn |
| `S_Cutter_Sheath` | class-specific (see class docs) | Cutter holstered |
| `S_Plate_Chest` | `spine_03` | Chest Catch Plate center. Dog seat point. Line exit point. Self-dog target. |
| `S_Plate_Back` | `spine_03` | Back Catch Plate center |
| `S_Plate_ShoulderL` | `clavicle_l` | Left shoulder Catch Plate center |
| `S_Plate_ShoulderR` | `clavicle_r` | Right shoulder Catch Plate center |
| `S_Telemetry` | `spine_03` | Telemetry pack, under the back plate |
| `S_LeasePlate` | `spine_02` | Lease plate, lower-left chest |
| `S_Gel_L`, `S_Gel_R` | `spine_02` | Stall Gel reservoirs on the spine frame |
| `S_Tally_01` to `S_Tally_08` | plate meshes | Tally decal anchors |
| `S_Lamp_Shoulder` | `clavicle_l` | Team-tint shoulder lamp |
| `S_Hook_Lift` | `spine_03` | Where a Hook grabs a scrapped body |
| `S_Drop_Attach` | `spine_03` | Drop entry attach point (Hook lowering, line slide) |
| `S_Camera_FP` | `head` | First-person camera, 1.7 m |

### Catch Plate sockets and the line exit

A Dog seats at the plate socket nearest its impact point. The line leaves the body from that socket. The panic-window turn angle is a property of the socket: chest 0°, shoulders 60° to 90°, back 150° or more (05 §5.5). Plate meshes are rigid, welded to the body in the physics asset, and never deform.

### Physics assets

- `PA_Rigger_<Class>` per class. Total mass set to the class value (90, 110, 130, 130, 220 kg). Mass distribution: 55% torso and harness, 20% legs, 15% arms, 10% head (**PROPOSED**).
- **Airborne state** (05 §5.1): ragdoll-with-pose. Pelvis, spine, and legs simulate. Upper body (spine_03 up, arms, head) runs animation with physics blend weight 0.35 so the player can still aim and cut.
- **Flipped state:** full simulate, blend 1.0, until landing.
- **Stall lock:** all constraints set to locked, angular drive strength maxed, over 0.2 s. Body becomes a kinematic-ish prop at its class mass (90 to 220 kg), tier Medium, for 4 s (05 §7.4, v0.2.1).
- **Harness plates:** separate convex shapes welded to their parent bone. Needed so Dog seating and Cutter melee can hit-test the plate, not the capsule.
- **Chaos:** no destructible parts on players. Tape and straps are cloth with collision against plate shapes only.

### Material parameter names

Shared across all harness, Spool, line, Dog, and Cutter materials so one Material Parameter Collection drives them.

| Parameter | Type | Range | Meaning |
|---|---|---|---|
| `TeamTint` | LinearColor | | Team slot color |
| `FactionPrimary`, `FactionSecondary`, `FactionAccent` | LinearColor | | Livery |
| `Wear` | float | 0 to 1 | Wear map strength |
| `Force_N` | float | 0 to 1,200 | Current line pull force in newtons. Drives emissive and audio pitch. Class T: 400 / 400 / 600 / 800 / 1,000; overspin ceiling 1,200. |
| `ForceNorm` | float | 0 to 1 | `Force_N / 1200` for Haul and Jack. For Hang, `0.83 x load / class Hang load` (**PROPOSED**, see `equipment/line.md`). |
| `Overspin` | float | 0 to 1 | 0 before 1.0 s, 1 at the octave tell |
| `KickFlare` | float | 0 to 1 | Spool capacitor discharge on Jack engage; 1 at 0 s, 0 at 0.3 s |
| `SetLine` | bool | | Set-line dim state |
| `Proximity` | float | 0 to 1 | Set-line brighten within 5 m of a player |
| `DarkLine` | bool | | Cutscene-only illegal variant. Never true in a sanctioned match. |
| `GelFlood` | float | 0 to 1 | Stall Gel bloom |
| `TallyMask` | texture | | Tally decals |
| `LeaseSerial` | int | | Serial atlas index |

### Shared animation list

Every class ships the full list below. Durations that mechanics fix are given here; class-specific numbers are in the class docs.

| Group | Clips | Fixed durations |
|---|---|---|
| Locomotion | idle, walk 8-way, run 8-way, sprint fwd, start and stop, turn in place 90/180 | speeds per class (05 §2.2) |
| Crouch | enter, idle, move 8-way, exit | |
| Slide | enter, loop, exit | 1.2 s total |
| Jump | launch, apex, fall loop, land soft, land hard | land hard = stagger landing |
| Mantle | low, mid, high | height per class |
| Perch | enter, hold, drop (Climber only) | hold up to 1.5 s |
| Brace | enter, hold, release (Rigger, Breaker) | enter 0.3 s |
| Spool | fire A, fire B, self-dog, mode Haul, mode Jack, Kick (Jack engage), mode Hang, Reel in/out (in Hang), release, recall | mode switch per class; Kick 0.3 s; Reel 1 m/s with a ratchet click per 0.25 m; recall per class |
| Dragged | skid fwd/back/left/right, skid to airborne | |
| Airborne | pose set: pulled fwd, pulled back, pulled up, falling | |
| Hoisted | hang loop, hang look, hang fire, hang cut, reel climb | reel climb 1 m/s |
| Flipped | ragdoll blend in, land, stagger out | stagger 0.6 s |
| Stagger | light 0.3 s, standard 0.6 s, Rigger-line 1.0 s, overspin 0.8 s | Stagger locks movement and Spool only; look and Cutter stay live (05 v0.2), so every stagger clip is upper-body additive. |
| Cutter | draw, warm hold, swing, recover, sheath | draw per class, swing 0.15 s, recover 0.30 s |
| Cut self free | chest (0° turn), shoulder L/R (60° to 90° turn), back (150°+ turn) | |
| Stall | lock pose set (standing, airborne, prone), gel bloom | lock 0.2 s |
| Drop | one per Face, one per class default | under 6 s |
| Rig Calls | all stop, slack, take up, dog in, three Face slots | 1.5 to 3 s each |
| Idle variants | three per class, one per Face | 4 to 8 s |

---

## Contradictions found in the bible

Logged here so the team can resolve them. No character document resolves them silently.

1. **Resolved by 05 v0.2 (bible CHANGELOG, 2026-10-24).** The original 05 gave pull tensions in kN that were ten times too high for its own foot-hold and panic-window numbers. v0.2 rescales pull tension to newtons (400 / 400 / 600 / 800 (1,200 overspin) / 1,000 N), adds Jack launch speeds (18 / 14 / 12 / 14 / 10 m/s), lowers foot-holds to 3 / 4 / 5 / 5 / 5 m/s² (the Breaker was 7 in v0.2 and is 5 in the follow-up rulings, so a Doubler's overspin lifts an unbraced Breaker) and braced foot-holds to 9 (Rigger) and 20 (Breaker). Hang loads stay in kN. Every document in this folder uses the v0.2 values.
2. **Resolved by v0.2.1.** Teodora Quispe is 72 (born L.-11), worked the Ribbon Wars at fifteen, and did not write the First Rule; she enforces it. Her Face doc is updated.
3. **Resolved by v0.2.1.** Paloma Reyes-Okafor was not born on Collar Seven; her grandmother survived its fall and Paloma speaks of it as inherited memory. Her Face doc is updated.
4. **Resolved by v0.2.1.** A scrapped body keeps its class mass (90 to 220 kg), tier Medium.
5. **Resolved by v0.2.1.** Six Collars came down on land (Two through Seven), One into the sea, five lost upward.
6. **Resolved by v0.2.1.** Weight classes: Light = Climber; Standard = Scrapper, Rigger, Doubler; Heavy = Breaker. Used here for audio and footstep tiers.
7. **Resolved by 05 v0.2.** 02 now states the Climber's Jack launch speed of 18 m/s against 12 to 14 m/s for the others, which is the forty percent.
8. **Ruled by 05 v0.2:** bodies in the Anchor's Well are not retrieved by Hooks. The Hook doc keeps the spin-up tell as an unseen audio source at the Anchor and shows no retrieval.
9. **Ruled by 05 v0.2:** mid-match event at 1:15 remaining in Scrap, 4:00 in Claim, 2:00 per Haul half.
10. **Dog size vs "needle-thin."** 05 fixes the Dog projectile at 0.08 m collision radius. 02 describes Ascendancy Dogs as needle-thin. The Dog doc keeps the collision sphere and thins only the visible mesh.
