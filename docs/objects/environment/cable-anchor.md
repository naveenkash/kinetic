# Cable Anchor

One-line: environment, static world, infinite mass, the family of hardware where a cable meets a structure: hanger saddles (0.4 m), house hangers (0.8 m), headframe and gantry rope terminations (1.5 m), the Anchor's Stump tie plates (1.0 m), and the Yard's crane rail stops. All arenas; the distinct rated anchor pin is in `rated/anchor-pin.md`.

## Role in play

Cable anchors are the Dog seats players actually aim at. A Dog in a cable is a Dog in a line a Climber might cut from; a Dog in the anchor is a Dog in the world. They are also the readability layer for everything hung: an anchor's hardware tells you whether the cable it holds is a load cable (heavy saddle, no tape), a slack cable (shackle, orange tape), or a rated pin (yellow and black). Players learn the three silhouettes in the apprenticeship.

| Harness | Relationship |
|---|---|
| Climber | Perch points. Every saddle is a 1.5 s rest. |
| Scrapper | Dog seats for Overboard; the stubby Scrapper Dog seats best on hardware, worst on rope. |
| Rigger | The Gallows anchor: a house hanger is at exactly the height (4 m above a house roof) to hoist someone off it. |
| Doubler | Two anchors, two lines, one enemy between them: the Double Gallows. |
| Breaker | Dog seats for the brace: a Breaker braced and lined to an anchor is a wall. |

## Appearance

Three silhouettes, learned once:

| Family | Silhouette | Tape | Meaning |
|---|---|---|---|
| Load saddle | Cast U-saddle on the main cable, two U-bolts, a hanger bar below | none | Cannot be cut or moved. |
| Slack hitch | Shackle on an eye plate, rope knotted or swaged | orange | Cutter, 0.6 s. |
| Rated pin | Cast head, horizontal pin, keeper plate | yellow and black | Breaker, 1,000 N. |

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Saddles, eye plates | cast steel, Office grey repaint over ochre | #6E7378 over #B8752A | 0.6 | 0.8 |
| U-bolts, shackles | galvanized, bright wear | #A8ADB1 | 0.45 | 0.9 |
| Headframe terminations | wedge sockets, greased | #3C3F42 | 0.35 | 0.9 |
| Stump tie plates | Weave-faced steel | #1B1F26 | 0.3 | 0.6 |
| Rail stops (Yard) | fabricated steel, red primer, chevroned | #8C3B2E, #F2C500 / #141414 | 0.65 | 0.5 |

Weathering: every anchor in Hangtown has been painted by three authorities: Guild ochre (L.4), Office grey (annual), and the residents' blue where they could reach. HTR's orange survey dots on the ones scheduled for cutting. Stump tie plates on the Anchor are new and salt-fogged. Yard rail stops are oil-black. Markings: Guild inspection stamps (`LG 61` punched), Office paint rings, the residents' names scratched into house hangers (every house hanger carries the family's name, which the condemnation notices repeat). Lighting: none; the hardware catches lantern and strip light. The Office's rule that a singing line is always the brightest thing in frame holds here: no anchor is lit brighter than a line.

Read by tier: from far, a lump on a cable. From mid, a silhouette in one of three families. From close, a name.

## Variants

1. **Load saddle (Hangtown).** On the main cables every 2 m (walkway hangers) and 20 m (house hangers).
2. **House hanger (Hangtown).** A 0.8 m cast yoke on the main hanger with the house's primary cable eyes and slack-cable eyes. The house attaches here at a rated 1,000 N (05 section 6.2): a Breaker's Haul, a Doubler's overspin, or a Cutter on the slack segment drops it; a Hang line never does. On a small house's single cable the orange slack segment starts at least 1.5 m above the hanger eye, so a cutter stands on the roof to reach it and is visible doing so. The Gallows anchor.
3. **Headframe termination (Hangtown Lift, Yard Gantry).** Wedge socket and keeper. Kinematic family; moves with its machine.
4. **Stump tie plate (the Anchor).** Where a maintenance clamp's tie bars meet the Scaffold ring. Cuttable by nothing; released by the clamp.
5. **Rail stop (the Yard).** Buffer at each end of the Gantry rails and the trench tunnel mouths. Dog seat for the Caboose line's world end.
6. **Hook pad eye (all arenas).** The Office's recovery eyes near each Well; Dogs do not seat while a Hook is attached (05 section 3.1).

## Physics

- Static world, infinite mass, except headframe terminations (kinematic with their machine). Dogs seat with a 0.3 m hint radius on all variants.
- Perch surface for Climbers.
- None of these fail on their own. A load saddle's cable may fail (Cable Day, pin pull) and the saddle goes with it as part of the spline; a house hanger releases its house when the house's primary cable breaks (strain threshold, 03) or its slack cables are cut; a tie plate releases with its ring.
- Dog-to-anchor Hang loads: the anchor is the world; the Hang load limit is the class's (4 to 12 kN) and slips per 05 section 3.2. A Rigger's 12 kN Hang from a house hanger holds a 1,200 kg small house (11.8 kN) and does not hold a 2,800 kg large one (27.4 kN), which slips 0.5 m per 0.1 s creaking. Two Riggers (24 kN, 05 section 6.2a) hold either. This is the authored reason the Guild fields two Riggers in Hangtown.

## Interactions

- **Haul** self to a house hanger: a grapple 4 m up onto the yoke, a perch for a Climber and a hang for everyone else.
- **Jack** from a saddle: the Slingshot start for crossing between cables.
- **Hang** from a house hanger to an enemy on the roof below, then hold Haul to Reel: the Gallows, 4 m of hoist at 1 m/s under the class Hang load. From a load saddle 14 m above the Mid Hang: the Gallows with a drop. The yoke's two slack-cable eyes are 0.6 m from the Dog seat: a hoisted enemy within reach of them can cut their own line or the house's.
- Chain: a Dog in a saddle on a cable that fails rides the falling cable (the saddle is part of the spline): the line's world end is now moving at up to 20 m/s. Whoever is on the other end is Caboosed by a cable.

## States

Static. Headframe variants follow their machine. The only visual state is `Occupied` (a Dog seated; the Dog's own emissive) and, for Hook pad eyes, `HookAttached` (no Dog seat, blue Office light on the eye).

## VFX and SFX hooks

`SFX_Dog_Seat_Steel` (the seat clank is directional per 05 section 11; anchors use the heaviest variant), `SFX_Saddle_Tick` on Sway peaks, `SFX_Hitch_Creak` under load; `VFX_Dog_Seat_Sparks_Steel`.

## Technical

- LOD0 300 to 1,200 tris by variant, LOD1 half, LOD2 a box with a normal map, LOD3 culled (the cable or structure carries the silhouette beyond 40 m).
- Textures: one 2K atlas `T_CableAnchor_Atlas` for all variants with a tape mask channel so the same mesh family can show no tape, orange, or yellow and black.
- Pivot at the cable or pin axis. Sockets: `Dog_Seat` (hint center), `Perch`, `Eye_0..3` (slack-cable or hanger attachment), `Tape`.
- Implementation: instanced static meshes per structure; the Dog seat hint is a `Kinetic_DogHint` sphere that biases Dog projectile seating within 0.3 m. Hook pad eyes toggle to `Kinetic_NoDog` while a Hook is attached.
- Collision: `Kinetic_WorldStatic`, or `Kinetic_Kinematic` for headframe terminations.

## Reference search terms

suspension bridge hanger saddle, wire rope wedge socket, cast steel eye plate shackle, scaffold tie plate, crane rail end stop buffer, cable stay anchorage hardware.
