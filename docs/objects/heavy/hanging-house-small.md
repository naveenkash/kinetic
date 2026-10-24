# Hanging house (small)

Heavy destructible. Heavy tier, 1,200 kg. 300 × 250 × 260 cm (**PROPOSED**), one room, hung from a single cable through a roof beam at a rated 1,000 N attachment (05 v0.2.1). Appears only in Hangtown: the Mid Hang (dense, around the market ring) and the Low Hang (the lowest shanties over the drip nets). The Well is directly below every one of them.

## Role in play

A hanging house is a wrecking ball with a kitchen in it. It swings on its cable with the Sway, it drops when its slack cable is cut or a Breaker tears its rated hanger, and two of them swung together by gravity shatter. It is Hangtown's thesis statement and the Eviction's body.

| Rig | Use |
|---|---|
| Eviction | Cut the house's orange-taped slack cable (0.6 s Cutter hold), or Haul its rated 1,000 N hanger with a Breaker's line or a Doubler at overspin. It drops a tier, or into the Well, onto whoever is below. |
| The Swing | Gravity, per 05 v0.2.1: cut the slack cable so the house drops a tier onto its reserve cable and swings at the speed of its fall, ride the Cable Day settle, or pump a Haul in time with the Sway. A line alone deflects a hung house only a few degrees (see Physics). |
| The Pendulum | Self-dog, Dog B the house's roof beam. Swing under it. A Hang never tests the rated hanger: break-free is active pull only (05 §6.2). |
| The Anchor Drop | Enemy Hung to a house, then cut the house's cable. |
| The Guillotine | A house already is one: cut its slack cable or tear its hanger and it drops by gravity; a line from house to enemy only steers it (05 §10). |

- **Scrapper (400 N):** deflects a hung house 1.9° (27 cm on its 8 m cable); released, it swings back at 0.30 m/s, a stagger. The Scrapper's Swing is the Cutter: drop the house a tier and let gravity swing it. Cuts cables fastest. Cannot touch the rated 1,000 N hanger. No 30% bonus on a Heavy object.
- **Rigger (600 N):** deflects it 2.9° (41 cm). Hangs an enemy to a house on a set line and lets a teammate cut the cable.
- **Breaker (1,000 N):** meets the rated 1,000 N hanger break-free exactly (thresholds are inclusive). Hauls a house off its cable without a Cutter, after the 1.0 s rated groan. On its cable, a Breaker deflects it 4.9° (68 cm); released, it swings back at 0.76 m/s, a flip.
- **Climber (400 N):** uses houses as perches and Pendulum pivots; a Hang to the roof beam loads the hanger statically and never unseats it. Jacks off a wall for a Slingshot: the 18 m/s Kick splits by inverse mass, the house takes 1.3 m/s and the Climber 16.7.
- **Doubler (800 N, 1,200 N overspin):** base 800 N does not reach the rated hanger; a single line at 1,200 N overspin tears it in the 0.8 s before the snap; split lines (480 N, 720 N overspin) never do. A full overspin snap delivers 1,500 N·s: 1.25 m/s on the house, over its 1.0 m/s Stall speed, while the Doubler is launched back at 10 m/s.

## Appearance

Silhouette: a box with a shed roof, a door on one short side, one shuttered window, a drip-net frame under the floor, and a single cable rising from a steel beam through the ridge to a hanger eye. Laundry lines off one corner. A chimney pipe.

| Part | Material | Base color | Rough / Metal |
|---|---|---|---|
| Wall cladding (salvaged sheet) | `MI_Steel_Painted_Worn` | `MI_Hanger_Paint` pair, two of teal `#2E8B8B`, red `#B5302A`, mustard `#D9A441`, violet `#6E4C9E` | 0.50–0.75 / 0 |
| Frame, floor | `MI_Timber_Dark` | `#4A3522` | 0.70–0.85 / 0 |
| Roof | `MI_Steel_Rusted` corrugated | `#7A3F1C` to `#A8562A` | 0.75–0.95 / 0.25 |
| Roof beam, hanger eye | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 / 1 |
| Rated tape on the hanger eye and the first 50 cm of cable above it | `MI_Tape_Rated` | `#F2C230` / `#141414`, 50 mm chevrons | 0.50–0.60 / 0 |
| Cable (above the eye) | `MI_Chain_Oiled` strand | `#3C3E42` | 0.30–0.45 / 1 |
| Slack cable tape | `MI_Tape_Slack` | `#FF7A00`, emissive 0.15 | 0.40–0.50 / 0 |
| Door curtain, awning | `MI_Canvas_Natural` | `#B9AE93` | 0.90–1.00 / 0 |
| Lashings | `MI_Rope_Sisal` | `#B59D6E` | 0.90–1.00 / 0 |

Wear story. Paint drips downward from each panel's top edge. The door frame is hand-polished at hip height. Rust runs from the roof screws. Chalk drawings on the lower cladding at child height: a kite, a hand, a tally. An HTR condemnation notice is stapled to the door: orange header `#E8620C`, `NOTICE OF DEMOLITION, HALVARD-TEAGUE RECLAMATION`, a claim number, a date. The neighborhood stencil (a kettle, a kite, or a hand, per district) sits by the door. Warm dust grime `#6E5A3C`, cooking smoke above the chimney. Under the Office's match lights the house reads as a dark mass with two tape marks above it: yellow-and-black on the hanger eye (rated, a Breaker's job, per 03 rule 7) and an orange ring on the slack cable (cuttable, any Cutter). The two never sit within 1 m of each other so the read is unambiguous. The orange tape emissive is 0.15 so it never competes with a singing line.

## Variants

| Variant | Notes |
|---|---|
| Kitchen | Chimney, pots on the Light contents list. Default Mid Hang. |
| Sleeper | No chimney, two shuttered windows, laundry on both corners. Low Hang. |
| Paint pairs | Teal and mustard, red and violet, mustard and teal. Never two houses adjacent with the same pair. |
| District stencil | Kettle (market side), kite (east anchor), hand (west anchor). |
| Condemned | HTR notice on the door. Every house in a season after an HTR Claim win; one in three otherwise (**PROPOSED**). |
| Swinging free | Post Cable Day state: cable lengthened on an authored swing path. |

## Physics

| Property | Value |
|---|---|
| Tier / mass | Heavy, 1,200 kg |
| Collision | Box 300 × 250 × 260 cm body. Roof beam box 320 × 15 × 15 cm. Cable is a constraint, not a body. |
| Center of mass | 90 cm above the floor, centered (floor and frame carry the mass). |
| Friction | Hanging: 0, a pendulum (05 §6.1a). Once dropped onto a platform: 0.6 on wood, 0.5 on steel, at which point it is scenery that breaks, not a load that moves. |
| Restitution | 0.1 |
| Break-free | Swinging on its cable: 0 N (any line swings it). Pulled off its hanger: 1,000 N, rated (05 v0.2.1): Breaker, or Doubler at overspin; inclusive. Cable cut by Cutter: 0.6 s hold on the orange tape. |
| Fracture | 2,000 N·s. Fragments: 3 Heavy wall pieces at 310 kg (**PROPOSED**), 1 Medium roof at 150 kg (**PROPOSED**), Light contents totaling 120 kg. |
| Cable length | **PROPOSED** 8 m from hanger eye to the load cable above. Pendulum period about 5.7 s. |

Line acceleration (T / m): 400 N 0.33 m/s²; 600 N 0.50; 800 N 0.67; 1,000 N 0.83; 1,200 N 1.00. Those figures apply to a house sliding free on a platform. On its 8 m cable the house is a pendulum and gravity's restoring force caps what a line can do: steady deflection is atan(T / mg) with mg = 11.8 kN. 400 N holds it 1.9° off plumb (27 cm); 600 N 2.9° (41 cm); 800 N 3.9° (54 cm); 1,000 N 4.9° (68 cm); 1,200 N 5.8° (81 cm). Released from those deflections the bottom-of-swing speeds are 0.30, 0.45, 0.60, 0.76, 0.90 m/s: a stagger to a flip, never a Stall, never a fracture. Lethal swings come from gravity (05 v0.2.1: cut cable, Cable Day settle, Sway pumping). A cut slack cable drops the house one tier (**PROPOSED** 4 m) onto its reserve cable, and it passes the bottom of that swing at 8.9 m/s, 10,680 N·s. Pumping a Haul in time with the 5.7 s period grows the arc over several cycles.

Against a player (1,200 kg × relative speed): stagger 300 N·s at 0.25 m/s; flip 600 at 0.50 m/s; Stall 1,200 at 1.0 m/s. At 2 m/s: 2,400 N·s. From a 7 m drop (11.7 m/s): 14,040 N·s. At the Haul cap (25 m/s): 30,000 N·s. Two small houses meeting after one has dropped a tier (8.9 m/s): 10,680 N·s, over the 2,000 N·s fracture, both shatter. Two houses nudged together by lines alone (under 0.9 m/s, under 1,080 N·s) dent each other and keep swinging. A player's body counts against the threshold: a 220 kg Breaker flipped into the house at 10 m/s is 2,200 N·s and breaks it. A wall piece (310 kg) Stalls at 3.9 m/s; falling 1 m (4.4 m/s) it delivers 1,364 N·s. Crush: the house's weight is 11.8 kN, over 8 kN, so a pin against a walkway is a Stall at 0.5 s. The 8 kN crush threshold is unreachable by any line; it comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1).

## Interactions

- **Haul to it:** the player is pulled to the house; it swings toward them at 0.33 to 0.83 m/s² while they close far faster (a Scrapper's 400 N on their own 110 kg is 3.6 m/s²). If it is above them a Haul does not lift them (1,000 N is under every harness's weight but a Climber's 883 N on a Breaker's line); they Hang and climb by Reel at 1.0 m/s (05 §3.2). If its cable is cut mid-Haul they fall with it.
- **Jack from it:** Slingshot wall. The Kick's V splits by inverse mass: a Breaker (V 10) leaves at 8.5 m/s and the house swings off at 1.5 m/s; a Climber (V 18) leaves at 16.7 m/s and the house takes 1.3 m/s. Both leave the house over its 1.0 m/s Stall speed. Dog A house, Dog B enemy, Jack: no Kick with a Dog in a Catch Plate (05 §3.2); the enemy skids at T over their mass and the house deflects a few degrees.
- **Hang from it:** a Pendulum or Gallows anchor from the roof beam, for any class, up to the class Hang load: break-free is tested against active pull only (Haul, Jack, the Kick), and the player's static load goes through the house into its rated 1,000 N hanger without unseating it (05 §6.2). An enemy Hung to a house is tethered to the Eviction: cut the cable or tear the hanger and the house takes them.
- **Object ↔ object:** house onto market stall (300 N·s) at any speed destroys it. House into house over 1.67 m/s relative shatters both. House onto a rooftop water tank over 1,500 N·s ruptures it.
- **Object ↔ kinematic:** the Lift counterweight passing a swinging house at 3 m/s relative delivers 3,600 N·s: the house breaks. It also severs any line from the house to a player that it passes through (the snap, both Dogs release; 05 §3.5).
- **Water:** none in Hangtown. Elsewhere, the timber floor floats for 4 s then sinks (**PROPOSED**).
- **Updraft, Breathing:** not present. Force volumes act on free bodies only, authored on a 100 kg reference body and capped at 2 × a_ref per body (05 §6.6); a cable-hung house would feel nothing until its cable is cut, and its spilled Light contents would feel the full authored value, capped at 2 × a_ref.
- **Floats / slides / rolls:** swings. Once dropped onto a platform it slides on 0.6 friction and tumbles off anything tilted past 31°.
- **Chain reactions:** a dropped house takes a swing bridge, the bridge swings its occupants, the house continues into the Well.

## States

| State | Trigger | Visual | Physics |
|---|---|---|---|
| Intact | authored | Hung, swaying | Kinematic sway when distant; dynamic within the 4-house budget |
| Dented | impact 1,000–2,000 N·s | One wall panel caved, window shutter gone, contents rattle | None |
| Broken | over 2,000 N·s | Walls peel outward, roof lifts off, contents spill | 3 Heavy walls, 1 Medium roof, Light contents 10 s |
| Destroyed | in the Well | Gone | Removed |

## VFX and SFX hooks

- Impact: `SFX_Impact_House_Timber`, `_Sheet`; pots and crockery layer `SFX_Contents_Rattle` on any impulse over 300 N·s.
- Cable cut: `SFX_Cable_Fray` during the 0.6 s hold, then `SFX_Cable_Part` and the house's own `SFX_House_Drop` creak.
- Hanger tear (1,000 N, rated): `SFX_Groan_Rated_House` for a 1.0 s minimum lead, `NS_Dust_Rated` from the eye, then `SFX_Lurch_Steel_H` and `NS_Dust_Lurch` at release.
- Fracture: `SFX_House_Shatter`, `NS_Impact_Timber_H`, `NS_Debris_Contents`, `NS_Dust_Plaster`.
- Emissive: slack tape 0.15. A single interior bulb, warm `#FFB066`, 0.5, off when broken.

## Technical

- LOD0 16,000 tris; LOD1 8,000; LOD2 3,000; LOD3 600. Nanite off (geometry collection). Distant kinematic houses use the LOD2 static proxy.
- Textures: 2 × 2K (cladding and roof, timber and contents) plus `T_Trim_Timber_01`, stencil atlas, chalk decals.
- Pivot: the primary hang point, the hanger eye on the roof beam. X+ toward the door. Z+ up.
- Dog sockets: `SKT_Dog_Top_01` (roof ridge, `Metal`), `SKT_Dog_Hook_01` (hanger eye, `Metal`), `SKT_Dog_Hook_02` (roof beam end, `Metal`), `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01` (cladding, `Metal`), `SKT_Dog_Door_01` (frame, `Wood`), `SKT_Dog_Bot_01` (floor joist, `Wood`). Fragments: `SKT_Dog_F1_01` through `F3_01` walls, `SKT_Dog_F4_01` roof.
- Chaos: `GC_HouseSmall_*`. L0 whole, damage threshold 2,000 (**PROPOSED** 1 unit = 1 N·s). L1: 3 walls, roof, contents cluster, threshold 800. L2: contents into Light props, threshold 300, `RemoveOnMaxSleep` 10 s. Anchor field `FS_Anchor_HouseSmall` holds the house to its cable constraint until the cable is cut, a line force of 1,000 N or more (rated, inclusive) tears the hanger after the 1.0 s groan, or Cable Day's scripted break. Damage propagation off so a dropped house lands whole and breaks on contact.
- Replication: RC-H while dynamic (15 Hz), RC-K kinematic sway when outside the four-house dynamic budget; swap is server-decided by distance to the nearest player and any live line. Fracture under RC-GC; wall pieces stay RC-H.

## Reference search terms

cable-hung shanty, favela stilt house corrugated, Hong Kong rooftop shack, suspended cable car cabin rusted, painted sheet metal shack chalk drawings, demolition notice stapled door, sisal rope lashing timber frame.
