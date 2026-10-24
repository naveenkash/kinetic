# Doubler Harness

- **Summary:** The Fray's harness. Two active lines and a tension override that hurts the wearer on a timer everyone can hear.
- **Faction:** The Fray
- **Mass:** 130 kg (Standard weight class)
- **Role:** Burst. Pull 800 N, two lines at 480 N each, overspin to 1,200 N (720 N each when split), Kick 14 m/s, the Snapback. Committed, telegraphed, loud.

## Silhouette

One heavy arm. The Doubler is Rigger mass and near-Rigger proportion, 0.62 m at the shoulders, but the right forearm carries a twin-drum Spool that runs from wrist to elbow and ends in a flared snap guard. The right arm is visibly thicker than the left from any distance. The torso is lean, black, and wrapped in neon tape that reads as bright stripes at 50 m.

- **Spool:** the full right forearm. Two drums stacked along the arm, upper and lower, each with its own line exit. A flared guard at the elbow protects the face from the snap. From 50 m the right arm reads as a tube with a bell at the elbow.
- **From behind:** a Guild-pattern back plate with the serial filed off, a shortened spine frame, and the right arm's bulk visible past the hip. Neon tape crosses the back in an X.
- **From above:** asymmetric: one thick arm. Shoulder plates are standard shelves.
- **Stance:** right arm held slightly out from the body, as if it is warm. It is.

## Construction

| Component | Form | Material | PBR |
|---|---|---|---|
| Chest plate | 30 x 26 cm, Guild pattern, serial field ground out, painted black | Steel, black paint | Base `#0E0E10`, roughness 0.5, metallic 0.9 under paint; grind marks `#8C8F92` at the serial field |
| Back plate | 32 x 30 cm, same treatment, red-match count painted on it in neon | Steel | Count numerals `#39FF14` |
| Shoulder plates | 16 x 14 cm, standard shelves, black | Steel | As chest |
| Spine frame | Guild frame cut down to 0.45 m, re-welded | Aluminum, black | Weld beads bright |
| Spool mount | Full forearm gauntlet, two drum housings, heat-scored around both line exits | Steel, bare | Heat score gradient: straw `#C9A24A` to blue `#3B4C7A` to brown `#4A2E1E` across 6 cm; roughness 0.3 to 0.6 |
| Snap guard | Flared steel bell at the elbow, 14 cm diameter (**PROPOSED**) | Steel | Scorched inside |
| Gel reservoirs | Two 0.8 L cylinders, labels removed, replaced with bookie's chalk marks | Steel | Chalk white |
| Telemetry pack | Office-issue, intact and sealed. The Fray tampers with everything else | ABS | `#2B2E31` |
| Lease plate | Filed. A bare rectangle where it was. The Office fines this every match and the Fray pays | Steel | Bright file marks |
| Straps | Black webbing with neon tape wound over every strap | Webbing, tape | Tape `#39FF14`, `#FF2BD6`, `#FFE600`, roughness 0.4 |
| Boots | Mid height, black, steel toe, neon laces | Leather | `#0E0E10` |

Heat scoring is the Doubler's signature wear. It grows with overspin use (runtime `Wear` on the gauntlet ramps 0.05 per overspin, cosmetic, resets per match, **PROPOSED**).

## Catch Plates

Guild-pattern plates with the serials gone. Chest 30 x 26 cm, back 32 x 30 cm, shoulders 16 x 14 cm. A Dog seats under the lip as on a Rigger. The team-tint rim band is a strip of retrofit LED tape rather than a machined band. The line exits from the plate socket. On a Doubler the exit point can carry two incoming enemy lines at once; the plate socket supports two seated Dogs side by side at a 6 cm offset (**PROPOSED** for all classes, used most on Doublers and Breakers).

## Color and livery rules

| Zone | Default | Customizable |
|---|---|---|
| Plates | Black | Black to dark grey only; the neon has to read |
| Tape | Neon green, magenta, yellow | Any two neons from the Fray set; pattern free |
| Red-match count | Back plate numerals | Generated from a Fray Contract track (**PROPOSED**) |
| Gauntlet | Bare heat-scored steel | Locked. The heat score is a tell. |
| Team-tint zones | Plate rim strips, both Spool rim rings, shoulder lamp | Locked |

Locked: gauntlet length and drum count, snap guard, plate dimensions. The Doubler's neon must never read as a team tint: Fray neon yellow `#FFE600` sits near Amber `#E69F00` in hue, so keep the Fray tape at full saturation and matte (roughness 0.4, no emissive) so it separates from the emissive Amber rim bands, and never allow the Fray yellow within 3 cm of a plate rim.

## Animation set

| Clip | Notes |
|---|---|
| Locomotion | Run 6.2 m/s, sprint 7.5 m/s, crouch 2.8 m/s. Right arm carried out. Swagger permitted. |
| Jump, mantle | Jump 1.1 m. Mantle to 2.2 m, favoring the left arm. |
| Slide | 1.2 s. |
| Dog fire A/B, C/D | Two fire sets, one per drum. Upper drum fires A/B with a wrist snap; lower drum fires C/D with a forearm drop. Dog speed 120 m/s. |
| Self-dog | Forearm to chest plate; either drum. |
| Mode switch | 0.25 s per line. The Doubler has two mode gestures at once: right hand for the upper line, right elbow angle for the lower (**PROPOSED** read). |
| Kick (Jack engage) | 0.3 s per drum. Each drum has its own capacitor: a Jack on both lines fires two Kicks, and the gauntlet flares white twice in quick succession. Forearm recoils 8 cm. Launch 14 m/s. The Kick never fires with either Dog in another player's Catch Plate (the Accord safety interlock, 05 §3.2); a Jack on a player is the 800 N motor push, no flare. Sanctioned Doubler gauntlets carry the Office interlock seal beside the inspection seal, intact; red-match Spools have the interlock filed off, and the Fray's Faces will tell you so. |
| Overspin | Hold pose: forearm shakes, 0 to 1.0 s ramp from 800 toward 1,200 N. At 1.0 s the gauntlet's heat score glows and the arm locks straight. At 1.8 s: snap launch, the player is thrown backward at 10 m/s, 0.8 s stagger; the far end takes 1,500 N·s x (F / 1,200 N). Needs launch-backward airborne pose and the 0.8 s stagger recovery. |
| Release, recall | Recall 0.6 s per drum. Releasing one line while the other is live triggers a 0.3 s tension-restore arm tremor. |
| Dragged skid | Four directions. Foot-hold 5 m/s², as the Rigger. |
| Airborne, hoisted, flipped | Shared set, plus the snap-launch pose. |
| Reel (in Hang) | Left hand reels the upper drum; the lower drum is reeled by rolling the forearm (**PROPOSED** read), so a Double Gallows reels both rods at once. 1 m/s each, clicks per 0.25 m, out of phase. Hanging variant: reel-climb with the right arm held out from the body. |
| Stagger | 0.6 s standard, 0.8 s overspin. |
| Cutter | Draw 0.20 s, swing 0.15 s, recover 0.30 s. Draw from the left thigh, handle up. |
| Cut self free | Chest, shoulders (60° to 90°), back (150°+). |
| Stall lock | 0.2 s. |
| Drop | Class default: sliding a line, letting go early, landing in a roll, 4.5 s. |
| Rig Calls | All stop, slack, take up, dog in. Bookie's signals in the Face slots. |
| Idle | Three: blow on the gauntlet, flick the snap guard, count on fingers. |

## First-person arms

The right forearm is the biggest first-person element in the game: two drums, four Dogs, two line exits, the heat score gradient, and the snap guard's bell at the frame edge. Both lines leave from the same arm at 8 cm vertical offset, which is how the player tells them apart. During overspin the gauntlet's heat score brightens and the whole arm shakes. The left hand and Cutter are standard.

## Audio identity

- **Footsteps:** Standard tier. Same as Rigger with a neon-lace squeak.
- **Harness creak:** metal tick from the hot gauntlet, like an engine cooling.
- **Spool:** two drums at slightly different pitch, spin pitch 0.95x and 1.05x baseline, producing a beat. Overspin raises both; the octave jump at 1.0 s is the loudest Spool sound in the game.
- **Recall:** 0.6 s wind, doubled.

## Technical

| Item | Spec |
|---|---|
| Poly budget | LOD0 85k tris (body 35k, harness 50k, of which gauntlet 14k) (**PROPOSED**). LOD1 42k, LOD2 17k, LOD3 5k, LOD4 impostor 1.5k. |
| Texture sets | Body 4K, harness hard 4K, gauntlet 4K (it is on screen constantly), harness soft 2K, head 4K per Face. Mid-spec: 2K. |
| Skeleton | `SK_Rigger`. `S_Spool` on `lowerarm_r` at 10% from elbow, housing extends to the wrist. Two line exits: `S_Spool_LineExit` (upper) and `S_Spool_LineExit_2` (lower, this class only). `S_Spool_DogC_Rest`, `S_Spool_DogD_Rest` on the lower drum. `S_Cutter_Sheath` on `thigh_l`. |
| Cloth | Tape tails. |
| Physics asset | `PA_Rigger_Doubler`, 130 kg. Right arm 2 kg heavier than left for ragdoll asymmetry. |
| Chaos | None on player. |

## Reference search terms

illegal street racing engine bay heat discoloration, blued steel temper colors, underground fight circuit wraps, neon gaffer tape on black, bookmaker tic-tac hand signals, welded cut-down roll cage, scorched exhaust header.
