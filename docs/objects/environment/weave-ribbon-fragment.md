# Weave Ribbon Fragment

One-line: environment, Light prop, 0.4 to 8 kg, strips 0.3 m to 2.0 m long x 50 to 200 mm wide x 2 mm thick, all arenas: embedded in hull plates and the Collar rim shear faces as dressing, loose in the Yard's scrap heaps and broken-strap billet crates, on the Anchor's Stump face where the clamps have bruised it, and one 2 m fragment as a Claim 67 evidence object.

## Role in play

Weave is the thing everyone wants (01), and a fragment is the smallest piece of the whole story a player can pick up. As a prop it is Light, fast on a Scrapper's line, and useless as a weapon (an 8 kg strip at the 25 m/s cap is 200 N s, under a stagger). Its job is to be seen: an oil-slick sheen in every arena that marks where the valuable material is, and in the Yard where the billets are. It also teaches the unzip: a fragment's torn edge is the visual that the lore hangs on (hit with a point impact, Weave unzips like a stocking).

| Harness | Relationship |
|---|---|
| Climber | The Ascendancy treats fragments as relics. Ascendant riggers' plates carry a strip of it as a cosmetic (04 livery, Contract reward **PROPOSED**). |
| Scrapper | Hangers sell it. A Scrapper's line moves a fragment at the 30% bonus; it is the fastest Light prop in the game and the least useful. |
| Rigger | The Guild logs every fragment's Lot. Guild riggers do not touch it in a match; it is bad form. |
| Doubler | The Fray smuggles it. A Fray bark fires when a Doubler's line touches one. |
| Breaker | HTR counts it as inventory. HTR's reserve riggers collect fragments in the Yard between rounds (dressing animation). |

## Appearance

Silhouette: a flat ribbon with a hard straight edge where it was cut and a feathered, curling edge where it unzipped. The lattice is visible as a fine diagonal weave in raking light. It does not drape; it holds a gentle curve like spring steel and chatters when it moves.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Face | carbon-boron lattice ribbon | #1B1F26 base with a thin-film iridescence (blue to bronze by angle) | 0.3 | 0.6 |
| Cut edge | plasma-cut, slag beads | #B36A2E | 0.6 | 0.7 |
| Unzipped edge | fine filaments, pale | #C9CED3 | 0.5 | 0.4 |
| Office impound tag (evidence) | yellow card, wire | #F2C500 | 0.85 | 0.0 |

Weathering: Weave does not rust and does not weather; what is on a fragment is what has been on it: Yard dust, Hold mineral scale, Hangtown soot. The sheen is the tell and is constant. Markings: Guild Lot numbers in white paint pen on larger fragments (`LOT 61-0218-F`), Office impound tags on the evidence fragment, HTR orange dots on fragments counted. The Ascendancy scratches a short prayer into fragments on the Anchor, and the Office fines them for it.

Lighting: the thin-film material is the brightest non-emissive surface in the game under raking flood light. Rule: fragment sheen never exceeds a line's emissive at any angle. The material has a sheen clamp.

Read by tier: from far, glints. From mid, dark strips with a color shift as you move. From close, the weave and the unzipped filaments.

## Variants

1. **Scrap strip.** 0.3 to 0.6 m, 0.4 to 1 kg. Scrap heaps, Yard Floor. Pooled, 40 per arena.
2. **Billet spill.** 1.0 to 1.5 m, 3 to 5 kg. Broken-strap billet crates. Straight cut both ends.
3. **Embedded.** Static, in hull plates and shear faces: the root strands pulled out of concrete on the Collar rims, the bruise under the Anchor clamps. Dressing; not a prop.
4. **Evidence (Claim 67).** 2.0 m, 8 kg, one edge plasma-cut (the Yard's), one edge the original break: unzipped filaments on one half of the width and a clean, bright shear on the other half. The object the Question is argued over in season six. See `evidence-objects.md`.

## Physics

- Light. Any line moves it; Scrapper lines 30% faster. 400 N on 1 kg is 400 m/s²; it reaches the 25 m/s cap in 0.8 m. 25 N s on impact. It cannot stagger anyone. It is the one prop in the game with no impact value, which makes it the honest answer to "is it dangerous": no, it is valuable.
- Drag: fragments have an authored drag coefficient so that force volumes carry them: the Updraft lifts loose scrap strips off the Anchor Rim bowl and spirals them up the Stump as the gust's visual tell (budget 20 in the air at once); the Pulse scatters them outward; the Hold flood floats them (density 1.6, they sink, but the rush carries them along the floor).
- Dogs seat in Weave. **PROPOSED:** for the Dog variants in 04, Weave counts as metal (Mag seats at higher load, Spike slides).
- Not destructible, not volatile, not cuttable. A Cutter does nothing to Weave (the lore reason load cables cannot be cut: Weave core).
- Lifetime: pooled scrap strips persist (they are dressing); spilled billet fragments persist until round reset.

## Interactions

- **Haul** a fragment: it arrives instantly and harmlessly. **Jack**: it leaves at V and is gone. **Hang** a fragment off an enemy's back plate: 8 kg of dead weight that chatters when they move, which is a sound tell on a stealthy Climber and the only tactical use anyone has found.
- Evidence fragment pickup: see `evidence-objects.md`.
- Chain: a fragment in a fire patch is unaffected. A fragment struck by a drive dog piston is unaffected. Weave does not break; that is the point of it, and of the Question.

## States

`Loose`, `Airborne (volume)`, `Tagged (evidence)`, `Collected (evidence, removed from world)`.

## VFX and SFX hooks

`SFX_Weave_Chatter` (spring-steel flutter, pitch by length), `SFX_Weave_Slide_Concrete`, `SFX_Weave_Pickup_Evidence`; `VFX_Weave_Sheen` (the thin-film material's angle-driven shift, no particle), `VFX_Weave_Spiral_Updraft` (the airborne pool).

## Technical

- LOD0 120 tris per strip with a two-bone bend, LOD1 a quad with the sheen material, LOD2 culled beyond 40 m except the evidence variant.
- Textures: `T_Weave_Face` 1K (lattice normal, thin-film LUT), `T_Weave_Edges` 512.
- Pivot at the strip's center. Sockets: `Dog_Seat`, `Tag`, `Prayer_Decal`.
- Implementation: pooled Light rigid bodies with a custom drag term for force volumes; sheen clamp exposed as a material parameter collection value tied to the line emissive ceiling. Evidence variant is a separate actor class (see `evidence-objects.md`).
- Collision: `Kinetic_Prop`. Embedded variant is part of its host mesh.

## Reference search terms

carbon fiber ribbon iridescent, thin film interference on black, spring steel strapping curl, torn kevlar edge filaments, boron fiber tape, holographic carbon weave close up.
