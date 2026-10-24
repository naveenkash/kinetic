# Swing Bridge

One-line: structure, destructible and cable-hung, Heavy tier, 420 kg, 12 m span x 1.2 m wide, deck 0.9 m below its two suspension cables, Hangtown Mid Hang: eleven bridges linking houses, the market platform, and the Lift landings.

## Role in play

Swing bridges are how non-Climbers cross the Mid Hang. They move with the Sway, so a Hang line from a bridge to a house changes length every three seconds. Each end hangs from a slack cable (orange tape, see `environment/slack-cable.md`): cut the near end and the bridge drops into a pendulum from the far end, carrying whoever is on it in a 12 m arc. That is the Cut-Loose, and it is the first thing every Hanger teaches.

| Harness | Relationship |
|---|---|
| Climber | Never uses one. Perches on the suspension cable above it. |
| Scrapper | Fastest Cutter in the league. 0.6 s hold cuts a near end while an enemy is mid-span. |
| Rigger | Clothesline at the entry plank. A Hang from the bridge to a house is a tripwire that moves. |
| Doubler | Two lines split to 720 N each at overspin, under the 800 N hitch break-free: a Doubler cannot unhitch a bridge. They snap a player off it instead (1,500 N s at full overspin, a Stall). |
| Breaker | Walks it. At 220 kg a Breaker on the mid-span adds 2.2 kN of weight to the cables, within rating, and the deck's Sway drift doubles under them. |

## Appearance

Silhouette: a shallow catenary of two 24 mm steel cables with a timber slat deck slung beneath on rope hangers every 0.6 m. Rope handrails at 1.0 m. The ends land on timber bearers bolted to house frames or platform edges.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Suspension cable | galvanized wire rope, greased | #6E7378 | 0.4 | 0.9 |
| Slack hitch | same rope, frayed, orange tape | #FF8A00 tape | 0.7 | 0.0 |
| Deck slats | mixed timber, painted ends | #8C7355, ends #2F6FAD | 0.85 | 0.0 |
| Hangers | polyester rope, sun-bleached | #D9CBB0 | 0.9 | 0.0 |
| Bearers | hull plate offcut, Weave-faced | #1B1F26 | 0.35 | 0.6 |

Weathering: the slats are different ages. Half were replaced after last season's Cable Day and are brighter. Handrails are worn dark at hand height. Stencils and markings: the neighborhood stencil (a blue hook) on the bearers; chalk arrows and children's hopscotch grids on the slats; an Office condemnation notice stapled at one end of every bridge on the HTR claim side (see `environment/condemnation-notice.md`). String lights (2700 K, warm, 40 lm per bulb) run along one handrail on bridges within 20 m of the market platform. The Office does not remove them because residents complained and LayCast liked the look.

Read by tier: from the Upper Hang, dark lines with warm bulbs in a slow drift. From the Mid Hang, a doorway-width run with the ravine visible between slats. From the Low Hang, a silhouette against the Office floodlights.

## Variants

1. **Standard.** 12 m, slat deck, rope hangers. 420 kg.
2. **Market approach.** 8 m, string lights, wider deck (1.6 m), 380 kg. Four of these feed the market platform.
3. **Plank-and-cable.** 15 m, single central cable with slats bolted to it, no handrails. The Low Hang version. Flips a sprinting player who stops short.
4. **Cut district (seasonal).** Bearers only, cables hanging empty. Dressing in a lost district.

## Physics

- Tier: Heavy (420 kg). Fractures at 2,000 N s (**PROPOSED**, matches the hanging house figure since it shares construction) into Medium slat bundles and Light rope.
- Anchoring: each end is a cable constraint to the house or platform frame. Break-free for the whole bridge: 800 N (chained). A Breaker's 1,000 N or a Doubler's single-line 800 N on a bridge end unhitches it; any lesser line sings.
- Slack hitch: 0.6 s continuous Cutter contact cuts one end. The bridge then swings as a pendulum of length 12 m about the far end. Period 2 pi sqrt(12 / 9.8) = 7.0 s; peak speed at the bottom of a quarter-swing from horizontal is sqrt(2 g L) = 15.3 m/s at the free end.
- A player on the deck during a Cut-Loose: airborne immediately (the floor falls away). If they hold on (no mechanic; they fall) they fall. A player standing within 1 m of the far end drops 1 m and ledge-grabs (under 8 m/s). Anyone further out is in free fall toward the next tier at 12 m below (15.3 m/s, scrapped on a hard landing, 18 m/s needed on nets at x2.0, so Low Hang drip nets save them).
- The swinging deck is a Heavy object: anyone it hits at the bottom of the arc takes 420 kg x 15 m/s = 6,300 N s. Scrapped.
- Sway: the bridge is a constraint child of two anchor points that move on the Sway animation. Deck lateral drift is up to 0.8 m at mid-span with a 6 s period.

## Interactions

- **Haul** a player to the far bearer while they stand mid-span: they come at you along the deck and hit the Clothesline you set at the end.
- **Jack** from a bridge mid-span: the recoil swings the bridge 0.5 m and the Slingshot carries you to a roof.
- **Hang** from a bridge cable to yourself and step off: the Pendulum. The cable is a good anchor for everyone but a Breaker (20 m line, too short for the arcs that matter).
- Chain: a cut bridge swings into a house (2,000 N s fracture) and breaks its wall. Two bridges cut from the same house meet under it.
- Cable Day: bridges on the failed cable side are pre-authored to drop at their market end and hang vertical.

## States

| State | Visual |
|---|---|
| Intact | Catenary even, slats flat, bulbs on. |
| Stressed | A Breaker mid-span: hitch rope creaks, tape stretches, 0.3 m extra sag. |
| Failing | Hitch strands part one by one over the 0.6 s cut, frays spark under the Cutter. |
| Failed | Hanging vertical from one end, slats swinging, bulbs still lit and swaying. |

## VFX and SFX hooks

- `SFX_Bridge_Creak_Loop` driven by load and Sway.
- `SFX_SlackCable_Cut_Hold` (0.6 s build) and `SFX_SlackCable_Part`.
- `VFX_Rope_Fray_Sparks` during the cut hold, `VFX_Slat_Shed` during the swing.
- `SFX_Bridge_Impact_House` on contact with any Heavy body.

## Technical

- LOD0 3,200 tris (cables as geometry), LOD1 1,400 (cables as camera-facing ribbons), LOD2 500, LOD3 card. Deck slats are instanced.
- Textures: `T_Hangtown_Timber` 2K, `T_Rope_Tile` 1K, `T_Hangtown_Decal_Atlas` 2K (stencils, chalk, notices).
- Pivot at the near bearer top. Sockets: `Hitch_Near`, `Hitch_Far` (Cutter targets, 0.4 m radius), `Rail_L_0..11`, `Rail_R_0..11`, `Bulb_0..19`.
- Implementation: deck as a chain of 20 rigid bodies with distance constraints (not cloth), two end constraints to the Sway anchor proxies. On cut: destroy the near constraint, enable full simulation on the chain, switch the far constraint to a free hinge. Pre-authored swing path (spline) overrides simulation for the Cable Day case so the result is deterministic.
- Collision: `Kinetic_WorldDynamic` for the deck (solid to players, rides the Sway), `Kinetic_Prop` for shed slats. The slack hitch has a `Kinetic_Cuttable` overlap sphere.
- Budget: four fully dynamic bridges at once (shares the Hangtown dynamic-house budget). Others play kinematic Sway only.

## Reference search terms

rope suspension footbridge Nepal, cable footbridge slat deck, favela footbridge string lights, wire rope catenary bridge, frayed wire rope close up.
