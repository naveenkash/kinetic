# Breaker Harness

- **Summary:** Halvard-Teague Reclamation's harness. The heaviest body in sanctioned play. Removes the room instead of the player.
- **Faction:** Halvard-Teague Reclamation (HTR)
- **Mass:** 220 kg (Heavy weight class)
- **Role:** Structure. The only harness that moves rated loads (1,000 N pull, the 1,000 N rated break-free). Highest tension. Kick 10 m/s. Braced, it is a wall. Airborne, it is cargo.

## Silhouette

A trapezoid on legs. Shoulders 0.82 m across, hips narrower, head low between two shoulder plates shaped like dozer blades. The bulk is everywhere above the waist and it is symmetrical, which no other class is. Boots are tall and square.

- **Spool:** low on the right forearm, housing butted against the elbow, the largest housing in the league. From 50 m the right arm reads as a club.
- **From behind:** a single wide back plate, 40 x 36 cm, with the HTR logo field. Shoulder blades flare out past the torso line. No visible spine frame; the frame is inside the shell.
- **From above:** a wide slab with the head as a small circle at center. The two shoulder plates read as one continuous shelf.
- **Stance:** wide. Feet at 0.7 m walking, 1.0 m braced. Walks with a forward lean of 5°, as if into wind.

## Construction

| Component | Form | Material | PBR |
|---|---|---|---|
| Chest plate | 36 x 30 cm, machined, 5 mm chamfered edge, HTR logo etched upper left, match number etched lower right | 7075 aluminum, anodized | Base `#5A5E62`, roughness 0.45, metallic 0.85; near-zero wear (replaced every match) |
| Back plate | 40 x 36 cm, same finish, logo field 20 cm wide in orange | Anodized aluminum | Orange field `#FF5A1F`, roughness 0.5 |
| Shoulder plates | 22 x 18 cm, raked outward 15°, blade profile | Anodized aluminum | As chest |
| Spine frame | Internal. Visible only as a ribbed shell between chest and back plates | Composite shell | `#2B2E31`, roughness 0.7, metallic 0.2 |
| Spool mount | Hard-shell forearm gauntlet from elbow to mid-forearm, two cam-lock latches | Aluminum, polymer liner | Grey as plates, orange latches |
| Gel reservoirs | Two 1.2 L flat tanks (**PROPOSED**) inside the shell, level indicators on the back plate flanks | Polymer | Indicator LEDs team tint |
| Telemetry pack | Integrated into back plate, Office seal visible as a 3 cm white disc | Sealed | `#F2F2F0` |
| Lease plate | 8 x 4 cm riveted tag, lower-left chest. HTR crews lease from Kessl in bulk; serials are sequential | Stainless | `#B8B4AC`, roughness 0.3 |
| Straps | 7 cm woven synthetic, orange, with grey quick-release buckles | Synthetic webbing | `#FF5A1F`, roughness 0.85 |
| Boots | Knee height, hard-shell shin, steel heel plate, 4 cm sole | Composite, steel | `#2B2E31`, roughness 0.6; heel plate bright metal |

Nothing is repaired. Everything is replaced. Wear maps are near flat except the boots and the Spool gauntlet, which take arena grime over a match (runtime `Wear` ramps from 0 to 0.3 over a round, **PROPOSED**).

## Catch Plates

Large machined rectangles with a chamfered edge instead of a lip. Chest 36 x 30 cm, back 40 x 36 cm, shoulders 22 x 18 cm. A Dog seats flat against the anodized face with a hard clank and a 0.5 cm magnetic snap-down. The seated Dog looks small against the plate, which reads correctly: a Breaker is the easiest body to Dog and the hardest to move. The line exits from the plate socket and sags visibly before it takes load, because the Breaker rarely goes anywhere.

## Color and livery rules

| Zone | Default | Customizable |
|---|---|---|
| Plates | Matte grey `#5A5E62` | Field color only, chamfer locked |
| Logo fields | HTR orange `#FF5A1F` with white logo | Sponsor swap allowed; field stays orange-family for readability |
| Straps | Orange | Yes |
| Tape | Grey and orange, machine-wrapped | Pattern |
| Team-tint zones | Plate rim bands, Spool rim ring, gel level LEDs, shoulder lamp | Locked |
| Match number | Etched, regenerated per match | Generated |

Locked: plate dimensions and chamfer, shoulder rake, gauntlet volume, boot height.

## Animation set

| Clip | Notes |
|---|---|
| Locomotion | Run 5.0 m/s, sprint 6.0 m/s, crouch 2.2 m/s. Heavy heel strike, arms low. |
| Jump, mantle | Jump 0.8 m. Mantle to 1.8 m, a three-beat haul with a knee. |
| Slide | 1.2 s. A controlled drop to both knees on the shin shells. |
| Brace | Enter 0.3 s: wide stance, both fists down, head low. Foot-hold rises from 5 to 20 m/s²: static to every line in the game, including a Doubler's full overspin. Unbraced, that overspin lifts a Breaker, which is why Breakers brace. Hold pose must read as immovable at 50 m. |
| Dog fire A/B | Whole arm presented from the shoulder. Housing recoil 4 cm. Dog speed 90 m/s. |
| Self-dog | Forearm to chest plate; the gauntlet clanks. |
| Mode switch | 0.35 s. Haul: fist pulled to hip. Jack: forearm pushed out. Hang: forearm raised level. Big, slow, legible. |
| Kick (Jack engage) | 0.3 s. The shell window flares white, the gauntlet recoils 4 cm, and the Breaker rocks back a half step. Launch 10 m/s. The slowest Kick and the only one that reads as effort. |
| Release, recall | Recall 0.9 s, a long drum wind. |
| Dragged skid | Four directions, two-stage: heels, then whole-body skid. Foot-hold 5 m/s²: another Breaker's 1,000 N (4.5 m/s²) skids an unbraced Breaker; a Doubler's full overspin (5.5 m/s²) is the one line that lifts one. |
| Airborne, hoisted, flipped | Hoisted Breaker hangs with arms wide; it should look wrong. |
| Reel (in Hang) | Two hands on the gauntlet drum, a slow heavy crank from the shoulder. 1 m/s, a click every 0.25 m, each click a detent clunk. Hanging variant: the Breaker reel-climbs with visible effort; it should look like the only slow thing the class does on purpose. |
| Stagger | 0.6 s. |
| Cutter | Draw 0.30 s (class penalty +0.10 s), swing 0.15 s, recover 0.30 s. Cross-draw from the right hip to the left hand: the slow draw is visible. |
| Cut self free | Chest, shoulders (60° to 90°), back (150°+). The back turn is a full torso rotation at reduced speed. |
| Stall lock | 0.2 s. Standing variant locks braced. |
| Drop | Class default: dropped like cargo from a Hook on a four-point sling, 5.0 s. |
| Rig Calls | All stop, slack, take up, dog in. Three Face slots. |
| Idle | Three: check gauntlet latches, roll shoulders, look at the ceiling. |

## First-person arms

The right gauntlet fills the lower right third of the screen. The drum face is large, grey, with an orange rim ring in team tint. Dogs are visible as two squat cylinders on the housing. The left hand is empty until the Cutter is drawn; the cross-draw brings the blade across the bottom of the frame. The chamfered top edge of the chest plate is visible when looking down 30°.

## Audio identity

- **Footsteps:** Heavy tier. Steel heel plate on steel is a bell note. On concrete a thud with grit. Every step has a low-frequency layer the other classes lack.
- **Harness creak:** none. Hydraulic hiss on brace enter, latch click on mantle.
- **Spool:** low growl, spin pitch 0.7x baseline. Mode switch is a heavy detent clunk.
- **Recall:** 0.9 s wind, ending in a servo stop.

## Technical

| Item | Spec |
|---|---|
| Poly budget | LOD0 95k tris (body 35k, harness 60k) (**PROPOSED**). LOD1 48k, LOD2 19k, LOD3 6k, LOD4 impostor 1.5k. |
| Texture sets | Body 4K, harness hard 4K x 2 (shell, plates), harness soft 2K, head 4K per Face. Mid-spec: 2K. |
| Skeleton | `SK_Rigger`. `S_Spool` on `lowerarm_r` at 15% from elbow. `S_Cutter_Sheath` on `thigh_r`, front face, handle forward. `S_Gel_L`/`S_Gel_R` inside the shell on `spine_02`. |
| Cloth | None. Strap tails are rigid. |
| Physics asset | `PA_Rigger_Breaker`, 220 kg. Mass biased 60% to torso. Braced state is treated as static below 20 m/s² (05 §5.4); only an object impact above the stagger threshold or a kinematic breaks it. |
| Chaos | None on player. Rated-load interaction is on the load, not the harness. |

## Reference search terms

industrial exoskeleton lifting rig, mining rescue breathing apparatus harness, bomb disposal suit shoulder profile, forklift operator hi-vis, machined anodized aluminum plate, demolition excavator livery, racing pit crew fire suit.
