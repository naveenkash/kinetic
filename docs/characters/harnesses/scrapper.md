# Scrapper Harness

- **Summary:** The Hangtown Crews' harness, built from four other harnesses. Fastest Dogs, fastest Cutter, instant recall. Chains small traps in the open.
- **Faction:** Hangtown Crews (Hangers)
- **Mass:** 110 kg (Standard weight class)
- **Role:** Improvisation. Pull 400 N, Kick 14 m/s. Light props and Medium props under 100 kg travel 30% faster on a Scrapper's line (Haul acceleration and the post-Kick Jack push, not the Kick's launch speed). A failed line costs nothing. Loses every straight pull.

## Silhouette

Lopsided. The Scrapper reads as a compact, hunched figure with all its clutter low and to the left: pouches, a coil of chain, a hand tool, hung from the waist belt. Shoulders 0.54 m across, and uneven: the left shoulder plate is larger than the right because they came from different harnesses. The back plate sits low, between the shoulder blades and the belt.

- **Spool:** mid-forearm on the right arm, with visible modifications: an external capacitor can, an exposed drum edge, a second line-exit guard. From 50 m the right forearm reads as a cuff with something bolted to it.
- **From behind:** the low back plate and the belt clutter. The silhouette widens at the hips, not the shoulders. No other class carries weight at the waist.
- **From above:** a kidney shape, offset left.
- **Stance:** forward hunch, hands near the belt. Walks fast and quiet. Idle fidgets.

## Construction

The four plate sources are canon for this harness (**PROPOSED** names): Kessl lease plates (Guild pattern, steel with a lip), HTR cast-offs (anodized, logo ground off), Ascendancy mirror (scratched matte and overpainted), and Basin cast (local foundry, rough sand-cast aluminum with porosity). Every Scrapper mesh uses at least three of the four.

| Component | Form | Material | PBR |
|---|---|---|---|
| Chest plate | 28 x 24 cm, typically Kessl pattern, overpainted | Steel | Base any salvage color, roughness 0.6 to 0.8, metallic 0.9 under paint; paint chips to steel `#B8B4AC` |
| Back plate | 30 x 28 cm, typically HTR cast-off with logo ground off, mounted low | Anodized aluminum, ground | Grind marks `#8C8F92`, roughness 0.5 |
| Shoulder plates | Left 15 x 13 cm, right 13 x 11 cm; different sources | Mixed | Mixed |
| Spine frame | Half a Guild frame, one rail, lashed to a Basin-cast crossbar with wire | Aluminum, wire | Ochre chips over alloy |
| Spool mount | Guild cradle with the straps replaced by a ratchet tie-down | Brass, nylon | Tie-down `#E8C23A` |
| Gel reservoirs | One 1.2 L cylinder (**PROPOSED**) on the single rail, plus a Nurri retail refill bottle zip-tied beside it | Steel, PET | Gel `#E8D9B0` visible in the bottle |
| Telemetry pack | Office-issue, 14 x 10 x 4 cm, strapped to the belt because the back plate is in the wrong place | ABS, seal | `#2B2E31`, seal white |
| Lease plate | Present and legal. Kessl stamped tag riveted to whichever plate the Office inspector could reach | Brass | Serial often re-stamped |
| Straps | Mixed: canvas, seatbelt webbing, rope. Belt is a cargo strap with pouches | Mixed | Any; grime high |
| Boots | Mismatched. Left a work boot, right a hi-top with a tire-tread sole (**PROPOSED**) | Leather, rubber | `#4A3A2C`, `#1A1A1A` |

Wear is total. Every surface has history. Use three wear layers: factory, paint, grime.

## Catch Plates

Mismatched but legal. Chest 28 x 24 cm, back 30 x 28 cm, shoulders 15 x 13 and 13 x 11 cm. The Office inspector verifies each one meets the dog-able minimum and paints a small white inspection dot in a corner (**PROPOSED**). A Dog seats differently on each plate type: a lip bite on the Kessl, a flat snap on the HTR, a scrape-and-bite on the Basin cast. The line exits from the plate socket. Because the back plate is low, the Scrapper's back-plate line leaves from the lumbar region, and the cut-self-free turn for the back is a look over the hip rather than over the shoulder.

## Color and livery rules

| Zone | Default | Customizable |
|---|---|---|
| Plates | Four salvage finishes, overpainted | Paint anything, within the plate value rule (25% to 70% luminance) |
| Stencil | Neighborhood stencil in teal `#3DA38C` on back plate | Player's crew stencil; one of 12 shipped Hangtown district stencils (**PROPOSED**) |
| Straps, pouches | Mixed | Yes |
| Tape | Whatever was in the bin: default sun yellow `#E8C23A` and rust `#7A3B22` | Pattern |
| Team-tint zones | Plate rim bands (retrofit light strips, visibly glued on), Spool rim ring, shoulder lamp | Locked |

Locked: plate dimensions, belt clutter volume, the left-heavy silhouette. The Scrapper is the most customizable harness and the one most likely to break readability; QA the 50 m test against every paint preset.

## Animation set

| Clip | Notes |
|---|---|
| Locomotion | Run 7.0 m/s, sprint 8.5 m/s, crouch 3.2 m/s. Hunched, quick, head on a swivel. |
| Jump, mantle | Jump 1.3 m. Mantle to 2.4 m, scrambling, one knee. |
| Slide | 1.2 s. A baseball slide on the hip pouches. |
| Dog fire A/B | Snap of the forearm from the hip. Housing recoil 2 cm. Dog speed 160 m/s. Fire rate is the fastest; keep the fire clip under 0.2 s. |
| Self-dog | Thumb to chest plate. |
| Mode switch | 0.15 s. Haul: a tug gesture. Jack: a shove. Hang: a flat hand. Fastest switch in the game; the clips are almost twitches. |
| Kick (Jack engage) | 0.3 s. The external capacitor can flares white and visibly jolts on its zip ties; forearm snaps back 8 cm. Launch 14 m/s. The 30% Scrapper prop bonus applies to the Jack push after the Kick, not to the Kick itself. |
| Release, recall | Recall instant. The Dogs vanish and reappear in the cradles with a flash; no wind animation. The drum spins back on a 0.2 s cosmetic lag that never gates input. |
| Dragged skid | Four directions. Foot-hold 4 m/s²: a Rigger's 600 N (5.5 m/s²) lifts a Scrapper; another Scrapper's 400 N (3.6 m/s²) only skids them. Boots scrabble. |
| Airborne, hoisted, flipped | Shared pose set. Hoisted Scrapper kicks. |
| Reel (in Hang) | Two hands on the brass drum, a quick palm-over-palm. 1 m/s, click per 0.25 m. Hanging variant: knees up, reeling, looking around. |
| Stagger | 0.6 s. |
| Cutter | Draw 0.15 s (class bonus −0.05 s), swing 0.15 s, recover 0.30 s. Draw from the chest strap, handle down, a single pull. |
| Cut self free | Chest, shoulders (60° to 90°), back (over the hip, 150°+). |
| Stall lock | 0.2 s. |
| Drop | Class default: stepping off a Hauler or Lift car at the spawn, 5.0 s. |
| Rig Calls | All stop, slack, take up, dog in. Three Face slots. |
| Idle | Three: re-zip-tie the Spool, check a pouch, tap the plate rim light to make it work. |

## First-person arms

The right forearm shows the modded Spool: brass drum, a capacitor can with a hand-painted warning, the yellow ratchet strap. The two stubby Dogs sit in open cradles. The left hand is near the chest strap; the Cutter handle is visible at the left edge of the frame before draw, which is the Scrapper's quickest-draw tell. Looking down shows the belt pouches.

## Audio identity

- **Footsteps:** Standard tier, uneven, and lighter in character than the Rigger's because of the soles. The two boots sound different: a leather heel and a rubber slap. Belt clutter rattles on every landing.
- **Harness creak:** zip-tie squeak, chain tinkle.
- **Spool:** a clatter over the whine, spin pitch 1.3x baseline. Mode switch is a plastic click.
- **Recall:** a snap, no wind.

## Technical

| Item | Spec |
|---|---|
| Poly budget | LOD0 80k tris (body 35k, harness 45k) (**PROPOSED**). LOD1 40k, LOD2 16k, LOD3 5k, LOD4 impostor 1.5k. |
| Texture sets | Body 4K, harness hard 4K, harness soft 4K, belt clutter 2K, head 4K per Face. Mid-spec: 2K. |
| Skeleton | `SK_Rigger`. `S_Spool` on `lowerarm_r` at 50%. `S_Cutter_Sheath` on `spine_03`, offset left 12 cm, beside the chest plate, never over it. `S_Plate_Back` lowered to `spine_02` on this class only; the socket name is unchanged. `S_Gel_L` on the single rail; `S_Gel_R` is the refill bottle. |
| Cloth | Pouch flaps, strap tails, chain as a short physics chain (6 links). |
| Physics asset | `PA_Rigger_Scrapper`, 110 kg. Belt clutter adds no mass. |
| Chaos | None on player. |

## Reference search terms

scrapyard worker gear, favela electrician tool belt, bike messenger bag clutter, mismatched motocross armor, sand-cast aluminum porosity, zip-tied field repair, shipbreaking yard workers Chittagong.
