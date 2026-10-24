# Structural Column

One-line: rated load, structural, 4,200 kg, 0.6 m x 0.6 m box section x 10.5 m tall, the Yard: the two columns under the control shack at (-62, +35) and (-62, +41) on the North Dock (shack floor Z 10.5, roof Z 14, bolted to the west portal leg; either column is live, per `arenas/the-yard/layout.md`), and Ballast: the deck stanchions under the Lower Bay decks (Z 16) above the Hold (one live per round).

## Role in play

The column is the generic rated load from 02's Breaker description: pull the column, drop the ceiling. In the Yard it drops the control shack 10.5 m onto the North Dock with its roof floodlights, and darkens the west third of the Rails (03 lists both as what breaks). In Ballast it drops a Lower Bay catwalk pair into the Hold. It is the Breaker's area-denial tool: it does not need an enemy in place, it changes what the map is.

| Harness | Relationship |
|---|---|
| Climber | The control shack roof (Z 14) is the Stacks tier's best perch and the launch point for the Rails; a Breaker on the North Dock at the column's foot is the signal to leave it. |
| Scrapper | Cannot move it. Can hold the Breaker off the column with props for the 0.6 s commit window. |
| Rigger | Can Hang the shack's doorway shut and make the Breaker come around. |
| Doubler | Overspin commits it; the recoil throws the Doubler 10 m/s away across the dock. Commit it from the NC1 mesa top with a 30 m line. |
| Breaker | Owner. Braces on the North Dock apron 6 m from the column, Hauls the base plate across its fall axis. A crew summing to 1,000 N on that axis commits it too (05 section 6.2). |

## Appearance

Silhouette: a square box column with a flared base plate on eight bolts and a cap plate under the shack's floor beam. Yellow-and-black tape in a 2 m band from the base. A diagonal knee brace on the dead columns; cut away on the live one (visible stub, bright cut face).

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Column | welded box section, Yard primer red | #8C3B2E | 0.65 | 0.5 |
| Base and cap plates | steel, bolted | #5A5E63 | 0.6 | 0.8 |
| Bolts | hex, eight, two cut on the live column | #8A8F93 | 0.5 | 0.9 |
| Knee brace | angle steel, same red | #8C3B2E | 0.65 | 0.5 |
| Tape | Office | #F2C500 / #141414 | 0.7 | 0.0 |

Weathering (Yard): primer red faded to pink on the sun side, soot-black from Hauler exhaust on the trench side, rust bleeding from every weld. Markings: Guild fabrication stamp `LG-C4-COL-02`, a load rating stencil `SWL 60 t`, HTR's orange `CUT 01 SEQ B` on the live column, and a Guild chalk note under it: `TWO BOLTS - DON'T`. Ballast variant: Consortium grey stanchion, seep-scaled, same tape and markings in the climber's font.

Lighting: the column stands under the shack's own floodlights; the tape band is the brightest painted surface on the Rails. Ballast: Office work lamp.

Read by tier: Yard Rails: a red post at your elbow with a taped foot. Stacks: a red line under the shack 20 m up. Floor: a sliver between mesas. Ballast: a grey post rising from the Hold through B-Low.

## Variants

1. **Yard shack column, live.** 10.5 m. Drops the shack with its roof floodlights onto the North Dock and darkens the west third of the Rails.
2. **Yard shack column, dead.** Knee brace intact, eight bolts, no tape band (structural but not rated for play; see `environment/structural-tape.md` rule on live-only taping). Pulling the live one takes the shack; the dead one goes with it.
3. **Ballast deck stanchion, live.** 10 m, round section, Consortium pattern, from the Hold floor to a Lower Bay deck. Drops two catwalk sections into the Wash.
4. **Stabilized (seasonal).** Bright new bolts and a Guild seal; untaped and dead.

## Physics

- Tier: Rated. Mass 4,200 kg. Break-free 1,000 N. The column does not slide; it buckles at the base when its two remaining bolts shear and the knee-brace stub can no longer carry the moment.
- Commit: 1,000 N sustained 0.6 s across the fall axis (within 45 degrees). The axis is authored toward the open side (Yard: toward the Stacks; Ballast: toward the shaft).
- Fall (Yard): the column folds at the base. The shack corner above it drops 1.5 m in 0.4 s and the shack's floor beam, now cantilevered from the portal leg, fails at its bolts 0.8 s later. The shack (18,000 kg, **PROPOSED**) drops 10.5 m onto the North Dock (X -66 to -58, Y +34 to +44), arriving at 14.3 m/s. The roof floodlights (see `environment/floodlight-mast.md`) go with it. Anyone on it: scrapped against a kinematic at 14.3 m/s. Anyone under it: crushed.
- Fall (Ballast): the stanchion drops its two catwalk sections (fracture event) 16 m into the Wash at 17.7 m/s. Grate pieces into 0.6 m water: no multiplier, 2,500 N s each, scrapped on contact.
- Players: anyone on the shack is airborne at 0.4 s and lands with it at 14.3 m/s, scrapped. A Climber who Slingshots off the shack at the groan survives. Anyone under it on the North Dock has 1.0 s of groan plus 1.2 s of beam failure plus 1.5 s of fall: 3.7 s of tell. This is a long consequence on purpose: it is a map change, not a kill rig, and the kill is for the inattentive.
- Lighting after: the west third of the Rails (X -80 to -27) and the North Dock below lose their floodlights. Lines are the light.

## Authored consequence

| Time | Event |
|---|---|
| 0.0 s | Commit. Lurch: base plate lifts 30 mm on the pulled side, bolt heads shear with two cracks. |
| 0.0 to 1.0 s | Groan. The box section rings, the shack above creaks and its windows spider, dust falls from the cap plate, the floodlights flicker. Marshal: "Column's going. Clear the shack." |
| 1.0 s | Release. Column buckles at the tape band and folds outward. |
| 1.4 s | Shack corner drops 1.5 m. Loose props in the shack (chairs, the maintenance log evidence object if present, a kettle) slide out the door. |
| 2.2 s | Leg bolts fail. The shack drops. Roof lamps die one by one over 0.6 s. The shack is kinematic for the fall and severs every line it passes through (05 section 3.5). |
| 3.7 s | Impact on the North Dock. Everything under its 8 m x 10 m footprint is crushed; dust wall 15 m for 4 s (visual). |
| After | The shack is a 3.5 m high crushed ruin on the North Dock with a 30 degree roof slope: new cover, a new mantle onto the North Dock's floodlight mast ladder, and a dead landmark. The west portal leg keeps its cage ladder; the Rails lose the Shack Perch route and the west third of their light. |

What breaks: both columns, the shack, its roof heads, props under it, lines in its path. Reset between rounds.

## Interactions

- **Haul** the base plate across the axis: commit. The Breaker is on the North Dock apron 6 m from the column, outside the shack's footprint by authored axis; a Breaker who pulls from under the shack is under the shack.
- **Jack** off a dead column: a Slingshot. Off the live one during the groan: still works; the column is static until release.
- **Hang** from the column: a Hang line never unseats a placed object (05 section 6.2), so any class anchors to a live column up to the class Hang load without starting it, and hoists by Reel. Hang an enemy to the column cap before the commit: they ride the fold's arc and land under the shack. The Gallows and the Avalanche in one pull.
- Chain: the shack's generator drum lands over 10 m/s and blasts, lighting the new ruin for 6 s; the roof lamps burst (Light glass, cosmetic); the maintenance log evidence object (Claim 63) lands in the open on the dock.

## States

| State | Visual |
|---|---|
| Intact | Straight, taped, two cut bolts visible to anyone who crouches. |
| Stressed | Base plate rocks 5 mm, tape flutters, shack windows tick. |
| Failing | Groan, flicker, fold. |
| Failed | Folded stub, dark sector, ruin below. |

## VFX and SFX hooks

`SFX_Column_Bolt_Shear` x2, `SFX_Column_Ring_Groan`, `SFX_Column_Buckle`, `SFX_Shack_Slide`, `SFX_Shack_Impact`; `VFX_Window_Spider`, `VFX_Flood_Flicker_Die`, `VFX_Stacks_Dust_Wall`; Marshal VO; light sector toggle event `Lighting.Sector.RailsEast = Off`.

## Technical

- LOD0 1,600 tris (column), shack separate; LOD1 700; LOD2 200.
- Textures: `T_Yard_Steel_Trim` 2K, `T_Yard_Decal_Atlas` 2K; Ballast variant uses `T_Ballast_Steel_Trim`.
- Pivot at base plate center. Sockets: `Base_Pull` (Haul target), `Cap`, `Brace_Stub`, `Tape_Band`, `FoldHinge`.
- Implementation: column fold is a two-bone skeletal animation (deterministic). The shack is a kinematic timeline to the authored impact pose, then a mesh swap to the ruin; its swept volume severs lines during the fall. Light sector toggle is a replicated gameplay tag read by the lighting manager; confirm line emissives read at full in the dark sector before tuning anything else (03 engine note, the Anchor, applies here too).
- Collision: `Kinetic_RatedLoad` seated, `Kinetic_Kinematic` during fold and slide, `Kinetic_WorldStatic` after.

## Reference search terms

steel box column base plate anchor bolts, elevated control cabin on stilts rail yard, column buckling failure, crane rail deck handrail, demolition pre-cut column.
