# Rigger Harness

- **Summary:** The Lay Guild's harness. The baseline everyone else is tuned against. Sets the room, then stands in the doorway.
- **Faction:** Lay Guild
- **Mass:** 130 kg (Standard weight class)
- **Role:** Area control. Two set lines, the stiffest Hang in the game (12 kN), longest reach outside the Climber. Pull 600 N, Kick 12 m/s. Nothing fast.

## Silhouette

Square. The Rigger reads as a rectangle with a person inside it. Shoulders are flat and level, 0.62 m across with plates on. The torso is boxed by an external spine frame that runs from the pelvis to the base of the neck and carries two gel reservoirs at kidney height, one each side. The bulk sits in the upper back and shoulders, never below the ribs.

- **Spool:** mid-forearm on the right arm, housing axis parallel to the ulna. Brass drum face points outward. From 50 m the right forearm reads as a thick cuff, not a bracelet and not an elbow block.
- **From behind:** the spine frame is the tell. Two vertical rails, two cylinders, one wide back plate between the shoulder blades. No other class shows hard vertical lines down the back.
- **From above:** a near-square footprint. Shoulder plates are horizontal shelves; the head sits in a square frame.
- **Stance:** feet at shoulder width, weight even. Rigger locomotion never leans. The Brace pose drops the hips and widens the stance to 0.8 m.

## Construction

| Component | Form | Material | PBR |
|---|---|---|---|
| Chest plate | 30 x 26 cm, flat with a 3 cm lip top and bottom, hand-stamped Ledger numbers lower right | Mild steel, brass rim | Base `#6E6A60` to `#8C8578`, roughness 0.55 to 0.7, metallic 0.9, wear map exposes bright steel `#B8B4AC` on edges |
| Back plate | 32 x 30 cm, bolted to the spine frame at four points | Mild steel | Same as chest; heavier scratch wear |
| Shoulder plates | 16 x 14 cm, flat shelves with a rolled outer edge | Mild steel, brass bolt heads | Same as chest |
| Spine frame | Two 2.5 cm square-section rails, 0.6 m long, cross-braced at three points | Aluminum, hand-painted ochre | Base `#C8862B`, roughness 0.6, metallic 0.3, chips to bare alloy `#A8AAAD` |
| Spool mount | Forearm cradle with two canvas straps and a brass buckle | Brass, canvas | Brass `#A67C3B`, roughness 0.35, metallic 1.0 |
| Gel reservoirs | Two 0.8 L cylinders, 7 cm diameter, 22 cm long (**PROPOSED**) with a sight-glass window | Steel, glass, Nurri decal | Steel as plates; gel visible `#E8D9B0` at 60% opacity |
| Telemetry pack | 14 x 10 x 4 cm box under the back plate, Office seal | ABS, lead seal | `#2B2E31`, roughness 0.8, metallic 0 |
| Lease plate | 6 x 3 cm stamped tag, lower-left chest plate | Brass | As Spool mount; most Guild riggers' tags are stamped CLEAR |
| Straps | 5 cm canvas webbing, chest cross, waist belt, leg loops | Canvas | `#B9A98A`, roughness 0.9, metallic 0; sweat darkening at shoulders |
| Boots | Ankle height, leather, hobnail sole, steel toe cap visible through wear | Leather, steel | `#4A3A2C`, roughness 0.7 |

Everything looks repaired because it has been. Mismatched bolts, a stitched strap, one re-brazed rail joint. Wear maps favor edges and contact points: strap rub on plates, hand polish on the Spool drum.

## Catch Plates

Flat steel rectangles with a 3 cm lip. Chest 30 x 26 cm, back 32 x 30 cm, shoulders 16 x 14 cm. A Dog seats centered on the plate socket; the jaws bite under the lip and the plate's team-tint rim band flashes once at seat. The line exits perpendicular to the plate face from the socket, then bends toward the puller. The Rigger's lip is deep enough that a seated Dog visibly hangs from it, which is the point: the plate looks like it was made to be grabbed.

## Color and livery rules

| Zone | Default | Customizable |
|---|---|---|
| Plates | Steel with brass rim | Paint on the field only, rim locked |
| Spine frame | Ochre `#C8862B` | Yes, any faction or crew color |
| Straps | Canvas `#B9A98A` | Tone only |
| Tape | Ochre and black bands | Pattern, within Rope Mark rules |
| Team-tint zones | Plate rim bands, Spool rim ring, shoulder lamp at `S_Lamp_Shoulder` | Locked |
| Stamped numbers | Ledger number on chest plate | Generated from the player's Ledger |

Locked for readability: plate size and rim band, spine frame geometry, shoulder shelf profile.

## Animation set

| Clip | Notes |
|---|---|
| Locomotion | Run 6.2 m/s, sprint 7.5 m/s, crouch 2.8 m/s. Upright, no lean. |
| Jump, mantle | Jump 1.1 m. Mantle to 2.2 m, a two-beat push. |
| Slide | 1.2 s. Rigger slide is a drop to one hip, canvas skid. |
| Brace | Enter 0.3 s: crouch, feet to 0.8 m, right hand to Spool, left to knee. Hold. Release. Foot-hold rises from 5 to 9 m/s²: holds a Breaker's 1,000 N (7.7 m/s²), not a Doubler's full overspin (9.2 m/s²). |
| Dog fire A/B | Forearm presented, wrist locked. Housing recoil 2 cm. |
| Self-dog | Right forearm taps chest plate. |
| Mode switch | 0.25 s. Haul: palm closes. Jack: palm opens outward. Hang: fist, thumb up. These are the Guild hand signals. |
| Kick (Jack engage) | 0.3 s. Housing flares white through the drum slot, forearm recoils 6 cm, the Rigger plants the rear foot. Launch 12 m/s. |
| Release, recall | Recall 0.6 s, visible drum spin-back. |
| Dragged skid | Four directions. Rigger digs heels; dust at the hobnails. Foot-hold 5 m/s²: a Scrapper's 400 N (3.1 m/s²) only skids a Rigger; a Breaker's 1,000 N lifts them. |
| Airborne, hoisted, flipped | Shared pose set. Hoisted: arms free, legs hang straight. |
| Reel (in Hang) | Two-hand reel on the Spool drum, left hand turning, right forearm braced against the chest plate: the winch pose. 1 m/s, a ratchet click every 0.25 m. This is how the Rigger hoists a Gallows: slow, strong, silent except for the clicks. Hanging variant: the Rigger climbs their own line by Reel, legs straight. |
| Stagger | 0.6 s. Against another Rigger's Hang line the victim staggers 1.0 s. |
| Cutter | Draw 0.17 s (class bonus −0.03 s), swing 0.15 s, recover 0.30 s. Draw from the left thigh. |
| Cut self free | Chest, shoulder L/R (60° to 90° turn), back (150°+ turn). |
| Stall lock | 0.2 s to locked pose. Standing variant locks mid-brace. |
| Drop | Class default: lowered by a Hook, 5.5 s. |
| Rig Calls | All stop, slack, take up, dog in. Three Face slots. |
| Idle | Three: check Spool strap, count on fingers, look up at the structure. |

## First-person arms

Right forearm fills the lower right of the frame: brass drum face, strap buckle, the ochre-and-black tape band at the line exit. Dog A and B sit in cradles on the housing's outer face; the player sees them leave. The left hand holds the Cutter low until drawn; the blade's heat ramp is visible in the lower left. Chest plate lip is visible at the bottom edge of the frame when looking down, so a seated chest Dog is on screen without a HUD element.

## Audio identity

- **Footsteps:** Standard tier. Hobnail on steel is a sharp double tick. On wood a dull knock. On concrete a scrape.
- **Harness creak:** canvas and leather, low frequency, on every mantle and brace.
- **Spool:** brass ratchet. Spin pitch baseline 1.0x. Mode switch is a mechanical click.
- **Recall:** 0.6 s drum wind with a stop clunk.

## Technical

| Item | Spec |
|---|---|
| Poly budget | LOD0 85k tris (body 35k, harness 50k) (**PROPOSED**). LOD1 42k, LOD2 17k, LOD3 5k, LOD4 impostor 1.5k. |
| Texture sets | Body 4K, harness hard 4K, harness soft 4K, head 4K per Face. Mid-spec drops all to 2K. |
| Skeleton | `SK_Rigger`. `S_Spool` on `lowerarm_r` at 55% from elbow to wrist. `S_Cutter_Sheath` on `thigh_l`, outer face. `S_Gel_L`/`S_Gel_R` on `spine_02` at the frame rails. |
| Cloth | Strap tails and the chest-cross canvas as cloth, colliding with plate shapes. |
| Physics asset | `PA_Rigger_Rigger`, 130 kg. Plates welded. Brace state raises foot-hold from 5 to 9 m/s² (05 §5.4). |
| Chaos | No destructibles on the player. Gel reservoir sight-glass drains via `GelFlood`. |

## Reference search terms

ironworker tool belt and harness, high-steel connector gear, longshore rigging vest, 1940s lineman climbing belt, brass hand-stamped tool tags, waxed canvas work jacket wear, demolition crew hard hat scuffs.
