# Climber Harness

- **Summary:** The Ascendancy's harness. The lightest body in the game and the longest line. Never where the floor is.
- **Faction:** The Ascendancy
- **Mass:** 90 kg (Light weight class)
- **Role:** Vertical mobility. Slingshot, Pendulum, perch, re-dog mid-air. Pull 400 N, Kick 18 m/s (the highest Jack launch; about 33 m per launch at 45°). Any Haul on a Climber is a slingshot, for them or against them.

## Silhouette

A vertical line. The Climber is tall, narrow, and symmetrical, 0.48 m at the shoulders with plates on. There is no bulk anywhere. The harness is a thin frame of straps and four small mirror plates; the back plate is a tall narrow fin that extends the spine visually. The head is uncovered or wrapped, never helmeted.

- **Spool:** high on the right wrist, like a wide bracelet. The housing is the smallest in the league, 9 cm long. From 50 m the right arm reads as bare with a bright band at the wrist.
- **From behind:** the fin. A 24 cm wide, 30 cm tall mirror plate centered on the spine with nothing beside it. The gel reservoirs are flat packs under the plate and do not show.
- **From above:** a thin cross. Shoulder plates are small ovals that sit on the deltoid, not on a shelf.
- **Stance:** weight forward on the balls of the feet. Idle sways. The Climber never stands square.

## Construction

| Component | Form | Material | PBR |
|---|---|---|---|
| Chest plate | 26 x 22 cm, slightly convex, polished, altitude of highest perch hand-painted in white numerals | Stainless steel, mirror polish | Base `#D8DADC`, roughness 0.08, metallic 1.0; wear map adds fine directional scratches, never dulls below roughness 0.2 |
| Back plate | 24 x 30 cm fin, convex, polished | Stainless, mirror | As chest |
| Shoulder plates | 14 x 12 cm ovals, polished | Stainless, mirror | As chest |
| Spine frame | Two flat titanium ribbons 1.5 cm wide under the straps, pelvis to C7 | Titanium | `#9BA0A4`, roughness 0.4, metallic 1.0; hidden under wraps except at the hips |
| Spool mount | Wrist cuff, hinged, one silver latch | Titanium, silver tape | Tape `#C9CDD2`, roughness 0.5 |
| Gel reservoirs | Two 0.6 L flat packs (**PROPOSED**) under the back fin, 2 cm thick | Polymer | Not visible; sight-glass at hip strap, gel `#E8D9B0` |
| Telemetry pack | 10 x 8 x 3 cm under the fin, Office seal | ABS | `#2B2E31` |
| Lease plate | 5 x 2.5 cm riveted tag at lower-left chest plate edge | Stainless | As plates; many Ascendants' tags read SPONSORED with a sect mark |
| Straps | 3 cm white wrap, layered over 2 cm webbing, hands and forearms wrapped | Cotton wrap | `#F4F1EA`, roughness 0.95; grime at palms and knees |
| Boots | Low, soft-soled climbing shoe with a hard toe rand | Rubber, canvas | `#E9E4D8` upper, `#1A1A1A` rand |

Wear is directional. Mirror plates scratch along the swing arc. Wraps grey at contact points. Nothing is painted except the altitude numerals and the sect mark.

## Catch Plates

Small convex mirrors. Chest 26 x 22 cm, back 24 x 30 cm, shoulders 14 x 12 cm. All four are at the minimum dimension that seats a 16 cm Dog cleanly. A Dog seats with a bright ping and a flash reflected in the mirror; the plate's team-tint rim band is the only colored thing on the harness. The line exits from the plate socket and immediately goes taut, because 90 kg moves on any line. Visually the Climber snaps toward the puller the instant the Dog seats; the plate should look like it was yanked.

## Color and livery rules

| Zone | Default | Customizable |
|---|---|---|
| Plates | Mirror | Locked. No paint on the mirror field except the altitude numerals and one 6 cm sect mark. |
| Wraps | White `#F4F1EA` | Off-white to pale sky `#A9C7E8` only |
| Tape | Silver | Pattern within Rope Mark rules; silver base locked |
| Team-tint zones | Plate rim bands, Spool rim ring, shoulder lamp | Locked |
| Altitude numerals | Highest perch, generated from the Ledger | Font locked |

The mirror is the Ascendancy's identity and its readability tell: it reflects arena light and reads as "light harness" at any distance. Reflections must not out-shine a singing line (03, arena rule 8); clamp specular to 70% of the line core's emissive in the material.

## Animation set

| Clip | Notes |
|---|---|
| Locomotion | Run 7.5 m/s, sprint 9.0 m/s, crouch 3.5 m/s. Forward lean, light footfall, arms high. |
| Jump, mantle | Jump 1.4 m. Mantle to 2.6 m in a single pull, no knee. |
| Slide | 1.2 s. A low feet-first slide on the hip wrap. |
| Perch | Enter, hold up to 1.5 s, drop. Four surface variants: wall, ceiling, pipe, edge. Can fire Dogs while held; needs a one-handed perch pose per variant. |
| Dog fire A/B | Wrist flick. Housing recoil 1 cm. Dog speed 140 m/s. Mid-air variants for every airborne pose; the Climber keeps full momentum on an airborne fire where other classes lose 15% horizontal speed (05 v0.2). |
| Self-dog | Wrist to chest plate, a tap. |
| Mode switch | 0.20 s. Haul: fingers close. Jack: fingers flare. Hang: open palm facing down. Small, fast. |
| Kick (Jack engage) | 0.3 s. The Spool dumps its capacitor into the line: wrist cuff flares white, the arm is thrown back 10 cm, and the Climber leaves the surface at 18 m/s. The smallest housing, the biggest launch. |
| Release, recall | Recall 0.4 s, a quick wind. |
| Dragged skid | Four directions, short: foot-hold is 3 m/s² and any class's line (a Scrapper's 400 N is 4.4 m/s² on 90 kg) lifts the Climber off their feet. The skid lasts the 0.4 s ramp and no longer. |
| Airborne, hoisted, flipped | The Climber's primary state. Airborne poses must be the most refined in the game: tucked, extended, swinging, reaching. Hoisted: body straight, one hand on the line. |
| Reel (in Hang) | Left hand crosses to the wrist cuff and turns the drum; the rod shortens (hold Haul) or lengthens (hold Jack) at 1 m/s. A ratchet click every 0.25 m. Hanging variant: the Climber climbs their own Hang line hand over hand in time with the drum, the fastest-looking Reel in the game at the same 1 m/s. |
| Stagger | 0.6 s. |
| Cutter | Draw 0.20 s, swing 0.15 s, recover 0.30 s. Draw from the left forearm sheath, blade toward the elbow. |
| Cut self free | Chest, shoulders (60° to 90°), back (150°+). Airborne variants of all three. |
| Stall lock | 0.2 s. Airborne variant is the common one. |
| Drop | Class default: sliding a line from above the spawn, 4.5 s. |
| Rig Calls | All stop, slack, take up, dog in. Prayer gestures in the Face slots. |
| Idle | Three: re-tie a wrap, look up and measure, balance on one foot. |

## First-person arms

The right wrist cuff sits small in the lower right of the frame; most of the forearm is white wrap. Two needle Dogs lie along the cuff. The left forearm shows the sheathed Cutter's handle when the left hand is raised. The chest plate is visible as a bright curve at the bottom of the frame when looking down, and its mirror reflects the sky, which is a quiet way to tell the player they are high up.

## Audio identity

- **Footsteps:** Light tier. Soft sole on steel is a pad. On wood a tap. Almost silent on concrete.
- **Harness creak:** cotton wrap rustle on every reach.
- **Spool:** high whine, spin pitch 1.2x baseline. Mode switch is a soft click.
- **Recall:** 0.4 s zip.

## Technical

| Item | Spec |
|---|---|
| Poly budget | LOD0 70k tris (body 38k, harness 32k) (**PROPOSED**). LOD1 35k, LOD2 14k, LOD3 4k, LOD4 impostor 1.5k. |
| Texture sets | Body 4K, harness hard 2K, harness soft 4K (wraps are most of the surface), head 4K per Face. Mid-spec: 2K and 1K. |
| Skeleton | `SK_Rigger`. `S_Spool` on `lowerarm_r` at 92% toward the wrist. `S_Cutter_Sheath` on `lowerarm_l`, inner face. `S_Gel_L`/`S_Gel_R` flat on `spine_02` under the fin. |
| Cloth | Wrap tails at wrists and hips as cloth. Keep under 400 verts. |
| Physics asset | `PA_Rigger_Climber`, 90 kg. Mass evenly distributed; this body gets thrown and it has to look light in flight. |
| Chaos | None on player. |

## Reference search terms

speed climber chalk and tape, free solo climbing shoes, sailing bowman harness, gymnast wrist wraps, polished stainless mirror scratches, pilgrim white cloth wrap, aerial silks performer.
