# Laundry bundle

Light prop. Light tier, 8 kg. 50 × 40 × 30 cm knotted cloth bundle, or 180 × 120 cm hung on a line in the `Hung` variant. Appears in Hangtown only: the Low Hang (laundry lines between shanties, drip nets full of it, terrace baskets), the Mid Hang (porches, the market platform), and in piles under the Upper Hang walkways. The map's soft surface.

## Role in play

Laundry is Hangtown's padding and Hangtown's curtain. A pile of it is a ×2.0 surface (05 §7.3, scrap heap, laundry, drip nets): stagger, flip, and Stall thresholds all double on it. A sheet of it on a line blocks sightlines and catches Dogs. A bundle on a line is a nothing: 8 kg, no stagger. The bundle's job is to be soft, to hide things, and to say that people live here.

- Named rigs: the Clothesline, literally: a Hang line at 1.2 m with a sheet over it is invisible until it is too late. The Pendulum, as the landing: a pile of laundry is where a Climber aims to land after a 7 m drop. Eviction cover: the Guillotine load above a doorway hidden by a hanging sheet.
- Scrapper: pulls a hanging sheet down with a Haul to open or close a sightline. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: Hangs a sheet in front of a set line. A Rigger's Clothesline behind laundry is the Mid Hang's signature trap.
- Breaker: walks through it. A Breaker does not get caught in a sheet; it tears.
- Climber: lands in piles on purpose. A fall of 14 m (16.6 m/s) onto laundry is under the 24 m/s doubled threshold. Climbers also Dog a hanging sheet to break a fall: the line holds, the sheet does not, and the 0.3 s it takes to tear is enough to cut speed.
- Doubler: no use. A bundle on a full Snapback arrives at the 50 m/s terminal velocity carrying 400 N·s, a stagger and a face full of cloth.

## Appearance

Silhouette: a bundle is a lumpy sphere with a knot on top. Hung, a rectangle with a sagged lower edge, moving in the Sway. Piles are low mounds 1 to 2 m across with sleeves and legs sticking out.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Cloth (cotton, poly) | `MI_Fabric_Laundry` | palette per bundle: `#E6E4DD` sheet white, `#2E8B8B` teal, `#B5302A` red, `#D9A441` mustard, `#6E4C9E` violet, `#4A4D50` work grey | 0.85 to 1.00 | 0 |
| Knot | `MI_Fabric_Laundry` | matches the outer cloth | 0.95 | 0 |
| Line it hangs from (sisal) | `MI_Rope_Sisal` | `#B59D6E` | 0.95 | 0 |
| Pegs (wood) | `MI_Timber_Rough` | `#7A5C3A` | 0.85 | 0 |

Wear story: sun-faded on the upward face, saturated in the folds. Rust streaks (`#7A3F1C`) where a sheet touched a cable. Starch and soot grime, tinted by the Hangtown grime parameter `#6E5A3C`. One in four bundles is work clothes: grey coveralls with a hanger stencil (a kettle, a kite, a hand) on the back, and ochre tape at the cuffs on ex-Guild coveralls.

No printed text. Chalk marks on the sheet occasionally: a child's drawing, a tally. Subsurface 0.3 so a backlit sheet glows warm against the ravine mist.

Under arena lighting: a hung white sheet is the brightest non-emissive surface in Hangtown. The Visibility Clause still holds: a singing line through a sheet must read brighter than the sheet. Tune the sheet albedo down to `#D6D2C8` if it competes.

## Variants

| Variant | Mass | Difference |
|---|---|---|
| `Bundle` | 8 kg | Knotted. Loose on a floor or basket. |
| `Hung_Sheet` | 2 kg | 180 × 120 cm cloth on two pegs on a sisal line. Blocks sight. Tears. |
| `Hung_Clothes` | 3 kg | Six garments on a line. Partial sight block. |
| `Pile` | 40 kg total, Medium surface | Five bundles merged as one static mesh with a ×2.0 surface volume on stagger, flip, and Stall. Does not move until struck by over 600 N·s (**PROPOSED**), then releases five Bundles. |
| `Coveralls_Hanger` | 3 kg | Single garment with neighborhood stencil. Reserve rigger livery reference. |

## Physics

- Tier: Light. Mass: 8 kg (Bundle).
- Collision: Bundle, a sphere radius 22 cm. Hung sheet, a cloth sim with a 180 × 120 × 2 cm box proxy for Dogs. Pile, a static box 200 × 150 × 50 cm with a surface volume.
- Center of mass: Bundle, geometric center.
- Friction 0.9, restitution 0.0 (**PROPOSED**). Laundry does not bounce or slide.
- Break-free: Bundle loose on floor 0 N. Hung sheet on pegs, 0 N; it is torn off by any line. Pile, see variant.
- Haul acceleration with the far end static: 50 m/s² at 400 N (Climber, Scrapper), 75 at 600 N (Rigger), 100 at 800 N (Doubler), 125 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 6.3 m, 4.2 m, 3.1 m, 2.5 m.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a bundle gives it 150 N·s, 19 m/s; it tumbles. On a hung sheet the recoil tears it off its pegs.
- Fracture: Bundle does not fracture. Hung sheet tears at 50 N·s (**PROPOSED**) into two cloth halves that fall as 1 kg Light props, 10 s lifetime.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 200 N·s | Nothing |
| Scrapper Haul | 25 m/s cap | 200 N·s | Nothing |
| Jack | 30 m/s cap | 240 N·s | Nothing |
| Fall from 7 m | 11.7 m/s | 94 N·s | Nothing |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 400 N·s | Stagger |

On a line a bundle cannot stagger anyone. What laundry does to a player is the surface rule (05 §7.3): a player landing on a Pile has every impact threshold doubled. Stagger at 10 m/s instead of 5, flip at 16 instead of 8, Stall at 24 instead of 12. A 24 m/s Stall is a fall of 29 m. The Pile is Hangtown's only forgiving floor and it should be rare: no more than six Piles on the map, none within 10 m of the Claim marker (**PROPOSED**).

## Interactions

- Haul to it: a bundle comes to you and stops. A hung sheet tears off its pegs and wraps your camera for 0.4 s (a cosmetic overlay, not a stagger).
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the bundle off at 16.5 m/s and takes 1.5 m/s. Then T pushes it at 50 to 125 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1). Nothing moves but the laundry. The Kick never fires with a Dog in a Catch Plate: a Jack from a bundle to an enemy is the T push alone, the bundle leaving from rest at 50 m/s² on a 400 N line and reaching the cap in 0.6 s over 9 m, while the enemy skids back at T over their mass.
- Hang from it: break-free is tested against active pull only (Haul, Jack, and the Kick) and does not apply to a Hang line, so what holds is the physical placement. A hung sheet's pegs and sisal line carry 300 N (**PROPOSED** physical behaviour), under every harness weight; the pegs let go and the player keeps falling with cloth on their camera. A Hang to a bundle brings it along as 8 kg of dead weight.
- Object to object: a bundle dropped on a market stall does nothing. A chain coil through a hung sheet tears it.
- Kinematic: the Lift counterweight tears through hung sheets in the Mid Hang on every cycle; prep crews re-hang them between rounds (reset).
- Water: a bundle floats for 5 s, then sinks. Hangtown has no standing water; this matters only if a bundle goes down the Well.
- Updraft and Pulse: not present in Hangtown. If a bundle is carried to the Anchor by a bug, force volumes act on loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 8 kg would be 12.5, so a bundle feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit, and it lifts and tumbles; a `Pile` held by its anchor field is unaffected until struck, and a bundle hung on a player's Hang line is a free body and swings in the gust.
- Sway: hung sheets move 20 to 40 cm with the Sway cycle. Bundles do not slide; friction 0.9 holds them on a 40° Cable Day tilt until struck.
- Chain reactions: none. Laundry ends chains; it is where things stop.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Knotted bundle, or taut sheet | 8 kg, or cloth sim |
| Dented | Bundle untied, sleeves out | No change |
| Broken | Sheet torn in two, pegs on the line | Two 1 kg Light halves, 10 s |
| Destroyed | Pile scattered into five bundles | Pile surface volume removed; five 8 kg props |

## VFX and SFX hooks

- Impact on steel: `SFX_Cloth_Thud_Steel` (soft, with a damp slap). Concrete: `SFX_Cloth_Thud`. Wood: `SFX_Cloth_Thud_Wood`. Water: `SFX_Cloth_Splash`.
- Hung sheet in Sway: `SFX_Sheet_Flap` loop, amplitude by wind.
- Tear: `SFX_Cloth_Rip_L`, `NS_Cloth_Fibers` (30 threads, 0.8 s).
- Player landing on a Pile: `SFX_Land_Laundry` and `NS_Dust_Starch` (warm dust puff, 1 s).
- Player Dogging a sheet: `SFX_Dog_Seat_Fabric` (a muffled seat clank; the Dog has passed through to the line behind).
- No emissive.

## Technical

- LOD0 2,000 tris (Bundle), LOD1 1,000, LOD2 400, LOD3 120. Hung sheet is a 24 × 16 cloth grid with LOD to a static plane at 15 m. One 1K set, palette by vertex color so one texture serves all colors.
- Pivot: Bundle, bottom center. Hung variants, the line's left peg (primary hang point). Pile, bottom center.
- Dog sockets: `SKT_Dog_Top_01` (knot, Fabric), `SKT_Dog_SideN_01` (bundle face, Fabric). Hung sheet: `SKT_Dog_Top_01` (hem center, Fabric), `SKT_Dog_Hook_01` (left peg, Wood), `SKT_Dog_Hook_02` (right peg, Wood). A Dog in Fabric holds the Standard and Barb Dogs; Mag and Spike Dogs slide off a sheet.
- Chaos: `GC_Laundry_Pile`. L0 static pile, L1 five Bundles. Damage threshold L0 = 600 (1 unit = 1 N·s, **PROPOSED** calibration). Anchor field `FS_Anchor_LaundryPile` holds the Pile static. Sheets are cloth, not geometry collections.
- Replication: RC-L. Cloth sim is client-local; only the tear event replicates.

## Reference search terms

Laundry line between shanty houses ravine; knotted cloth bundle on head carry; faded sheets on clothesline backlit; favela laundry lines cables; pile of laundry basket market; work coveralls hanging stencil back; drip net laundry hillside.
