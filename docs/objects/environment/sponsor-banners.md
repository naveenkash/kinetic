# Sponsor Banners

One-line: environment, dressing with Light cloth and Medium frames, cloth 1 to 4 kg, frames 40 to 120 kg, formats 3 m x 1 m (rail banner), 6 m x 2 m (container face), 12 m x 3 m (crane leg wrap), 20 m x 1.5 m (Rim ribbon), all arenas: LayCast's sponsor layer, 20 to 40 placements per arena, densest at spawns and around the Claim marker where the cameras look.

## Role in play

Banners are the broadcast on top of the wreck (01, tone). They do nothing to a match except wave, tear, and tell the player where the cameras point. A banner across a doorway is not a Clothesline (cloth is not solid); a banner frame is a Medium prop and a Dog seat. They are also readable landmarks: a player on the Yard Floor knows the press pit is east because the Nurri ribbon runs along the east Floor. Sponsors are fictional and slightly embarrassing (02).

| Harness | Relationship |
|---|---|
| Climber | Slingshots through cloth. The Ascendancy's riggers wear no sponsor patch, and their crews ask for banners to be removed from the Crown, which the Office refuses. |
| Scrapper | Frames are 40 kg Medium under 100 kg: a Scrapper's 30% bonus prop. A rail banner frame at 13 m/s is 520 N s, a stagger. |
| Rigger | Hangs behind a banner: cloth hides a set line's dim glow until a player is within 5 m, which is exactly when it brightens. |
| Doubler | Fray stickers cover sponsor logos with odds. The Fray's own sponsor is the Fray. |
| Breaker | HTR's patch is on every Breaker; HTR is also a LayCast sponsor, which the Guild points out on air. |

## Sponsor list

Three from the bible, five added.

| Sponsor | Business | Tagline | Logo description | Colors |
|---|---|---|---|---|
| **Nurri** | Stall-Gel | "Nurri. You'll feel nothing." | A rounded teardrop with a smaller teardrop inside it, both leaning right as if falling; the word in soft lowercase. | #7FD8BE gel green on #F5F5F0 |
| **Kessl Harness Leasing** | Harness finance (the Weight) | "Kessl carries you." | A stylized shoulder plate seen from above, with a Catch Plate ring at its center and the K cut into the plate like a stencil. | #2B2B2B on #D9D4C7 |
| **Meridian Mutual** | Insurance | "Insuring the Lay since the Lay." | A horizontal line (the equator) with a short vertical tick at its center (the Strand) inside an oval; sixty-one small notches along the line. | #0E4C6B on #F2E8D0 |
| **Corda Filament Works** | Line filament, the Spool's consumable | "Sing it true." | Three parallel lines converging to a point, the center one drawn as a sine wave. | #E69F00 amber on #141414 |
| **Pettersen Dogs** | Dog manufacturer (Standard, Barb, Mag, Spike) | "Seats first time." | A blunt dog head in profile, drawn as four straight strokes, with a single dot for the eye. | #B3B6B8 steel on #1B1F26 |
| **Oyelaran Ceramics** | Cutter blades | "The last thing you need." | A short blade shape with a heat gradient from white tip to black tang, inside a square. | #F5F5F0 to #141414 gradient on #C8452B |
| **Basin Brewing Co.** | Beer, the Lay's | "Cold at the bottom." | A tankard whose handle is a Hook's grab arm; foam drawn as a cloud over a gorge. | #F4B860 on #3E7A5B |
| **Second Strand Lottery** | A state lottery funding the Consortium heirs' Second Strand project | "One ticket. One ride." | A thin vertical line from a dot (the Anchor) to a star, with a numbered ticket stub crossing it. | #56B4E9 sky on #0F1A1C |

Team-side banners: the two spawn compounds carry LayCast frames in the crews' team tints (05 section 11a: Amber #E69F00 for crew A, Sky #56B4E9 for crew B; Rose #CC79A7 is reserved for the third crew in the post-launch 3v3v3 format and is not placed at launch; Bone #E8E6E0 is LayCast's own neutral). Frame tint is the only place a team color appears on dressing, and it is non-emissive.

Notes for the writers' room: Nurri's green is never used on gameplay surfaces. Corda's amber is Team A's Amber (#E69F00) by design (the fiction: Corda supplies the Office's filament and the team tint came from the product); banners using it are kept out of the Hangtown Mid Hang where line reads matter most. Second Strand Lottery banners are vandalized most, by every faction.

## Appearance

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Cloth (mesh banner) | PVC mesh, 30% open | per sponsor | 0.7 | 0.0 |
| Cloth (ribbon) | woven polyester, solid | per sponsor | 0.6 | 0.0 |
| Frames | aluminium tube, LayCast teal | #00A9A5 | 0.5 | 0.8 |
| Bungees and ties | black elastic, cable ties | #141414 | 0.8 | 0.0 |
| Tears and patches | gaffer tape | #4A4A4A | 0.9 | 0.0 |

Weathering: new each season, torn by the end of a match day. Mesh banners show the structure behind them. Wind-slapped edges are frayed. Markings beyond the logo: LayCast's own corner tag on every banner, the Claim number, and the Fray's odds stickers. Lighting: banners are never lit directly; they are placed in the floods' rake so they read without competing with lines. No banner is emissive. The one exception is the Rim ribbon on the Anchor, which carries the season winner's colors as a woven band (04: colors on the Rim cranes; the ribbon is a **PROPOSED** extension).

Read by tier: from far, color blocks that mark the camera-facing sides of structures. From mid, logos. From close, taglines and vandalism.

## Variants

1. **Rail banner.** 3 m x 1 m mesh on a 40 kg tube frame zip-tied to a handrail. Frame is a Medium prop at 200 N break-free.
2. **Container face.** 6 m x 2 m mesh bungeed to a container's side. Cloth only; no frame prop.
3. **Crane leg wrap.** 12 m x 3 m solid cloth on the Yard Gantry legs and the Anchor Rim cranes. Static.
4. **Rim ribbon.** 20 m x 1.5 m ribbon on 1.5 m posts (Medium, 60 kg each) along the Anchor Rim's inner edge, the Yard's east Floor, Hangtown's rim compound. The Pulse throws Rim ribbon posts; see `force-volumes.md`.
5. **Vandalized.** Any of the above with a faction's mark (see `chalk-and-graffiti.md`).

## Physics

- Cloth: Light, not solid, no collision with players or lines. Simulated as cloth within 20 m of a camera, vertex-animated beyond. Cloth in a force volume (Updraft, Pulse, gas jet) animates with the volume's direction: banners are the force volumes' visual tell in the Rim and Floor sectors, which is why they are placed where they are.
- Frames: Medium (40 kg rail frame, 60 kg ribbon post), racked at 200 N. Any line pulls one free. A Scrapper's 400 N on 40 kg is 10 m/s² (times the 30% bonus); a frame arrives at 13 to 14 m/s over a 20 m line: 520 to 560 N s, a stagger. A Breaker's 1,000 N on 40 kg is 25 m/s², hitting the 25 m/s cap in 12 m: 1,000 N s, a flip.
- Fire: cloth within a fire patch burns away over 6 s (material state, no spread). Blast: cloth within 4 m shreds (Light fragments, 10 s).
- Pulse (the Anchor): ribbon posts on the Rim are thrown outward at the Pulse's edge acceleration; they are placed 2 m inside the Rim edge so the posts go over the edge, not into players. The ribbon then hangs over the Rim's outer face in strips.

## Interactions

- **Haul** a frame: ammunition. **Jack** off a frame: it moves instead of you (40 kg against 90 to 220). **Hang** cloth: nothing; Dogs seat in frames and in the structure behind cloth.
- A set line behind a mesh banner is hidden until 5 m; a Clothesline behind a Nurri banner across the market approach is the Guild's signature cheap shot and the Hangers' least favorite thing.

## States

`Fresh`, `Torn`, `Burned`, `Vandalized`, `Frame loose` (cloth collapsed), `Blown` (post-Pulse ribbon strips).

## VFX and SFX hooks

`SFX_Banner_Flap_Loop` (wind zones and force volumes), `SFX_Banner_Tear`, `SFX_Frame_ZipTie_Snap`, `SFX_Cloth_Burn`; `VFX_Cloth_Sim` (budgeted 12 per arena), `VFX_Banner_Shred`, `VFX_Cloth_Char`.

## Technical

- Cloth: Chaos Cloth on a 0.25 m grid for rail banners and ribbons within 20 m; vertex animation (two wind loops plus a volume-driven blend) beyond. Mesh banners use a masked material with the 30% open pattern so the structure behind reads.
- Textures: `T_Sponsor_Atlas` 4K (eight logos, taglines, LayCast tags, two tears per logo), `T_Vandal_Atlas` 2K shared with graffiti.
- Pivot at the frame's top-left. Sockets: `Frame_Dog_0..3`, `Tie_0..7`, `Sticker_0..3`.
- Placement: a sponsor placement sheet per arena with camera-facing checks; no banner within 3 m of a rated load's tape or a slack cable's tape (readability), none on the Crown.
- Collision: cloth `Kinetic_NoCollide`; frames `Kinetic_Prop`.

## Reference search terms

motorsport sponsor mesh banner fence, stadium perimeter ribbon board, scaffold wrap advertising torn, PVC mesh banner wind, crane leg advertising wrap, vandalized billboard sticker.
