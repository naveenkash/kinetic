# Hanging house (large)

Heavy destructible. Heavy tier, 2,800 kg: any harness can deflect it a few degrees and pump it with the Sway, but no line swings it dangerously on its own (05 v0.2.1), and its hangers are rated 1,000 N attachments. 450 × 350 × 480 cm (**PROPOSED**), two storeys, hung from two cables through a roof truss, with a rooftop water tank. Appears only in Hangtown, the Mid Hang: two or three per match, on the main cables nearest the market ring. The Well is directly below.

## Role in play

The large house is the biggest thing in Hangtown that a Scrapper's line can move. It is the Swing's heaviest pendulum, the Anchor Drop's best load, and the one object whose fracture threshold is lower than its own Stall speed, which makes it a bomb that goes off on contact.

| Rig | Use |
|---|---|
| The Swing | Gravity, per 05 v0.2.1: cut one slack cable so the house drops onto its surviving cable, settles to 40°, and its low corner sweeps the market ring at about 5 m/s (**PROPOSED** speed); ride the Cable Day settle; or pump a Haul with the Sway. A line alone deflects it 2° at most. |
| Eviction | Cut either slack cable (0.6 s hold), or tear a rated 1,000 N hanger with a Breaker's line or a Doubler at overspin. One cut or tear: the house hangs by one cable at 40°, everything inside slides out. Two: it drops. |
| The Anchor Drop | Enemy Hung to the house, both cables cut. |
| The Pendulum | Self-dog to the truss and swing under it. A Hang never tests the rated hangers: break-free is active pull only (05 §6.2). |
| The Guillotine | A storey's worth of contents comes down when a wall peels. |

- **Scrapper (400 N):** deflects it 0.8° on its cables (15 cm); released, 0.14 m/s, nothing. The Scrapper's Swing is a cut cable. Cuts cables faster than anyone. No 30% bonus on a Heavy object.
- **Rigger (600 N):** deflects it 1.3° (22 cm). Hangs enemies to it. Sets lines across its doorways: the house moves, the Clothesline moves with it.
- **Breaker (1,000 N):** meets a hanger's rated 1,000 N break-free exactly (inclusive) and tears it after the 1.0 s groan, dropping the house onto one cable without a Cutter. On both cables, deflects it 2.1° (36 cm); released, 0.36 m/s, 1,008 N·s, a flip.
- **Climber (400 N):** perches on the roof tank, Slingshots off the upper wall and keeps 17.4 of their 18 m/s Kick; the house takes 0.6 m/s. Longest line reaches it from the Upper Hang.
- **Doubler (800 N, 1,200 N overspin):** base 800 N does not reach the rated hanger; a single line at 1,200 N overspin tears one in the 0.8 s before the snap; split lines (480 N, 720 N overspin) never do. Two lines: one nudging the house (a split 480 N deflects it 1.0°), one holding an enemy in its path. A full overspin snap delivers 1,500 N·s: 0.54 m/s on the house, over its 0.43 m/s Stall speed.

## Appearance

Silhouette: a tall box with a lean-to second storey set back on one side, a balcony with a timber rail, a shed roof, and a cylindrical water tank on the roof. Two cables rise from the truss ends to hanger eyes. Laundry lines cross to the next house.

| Part | Material | Base color | Rough / Metal |
|---|---|---|---|
| Lower cladding (salvaged sheet) | `MI_Steel_Painted_Worn` | `MI_Hanger_Paint` pair | 0.50–0.75 / 0 |
| Upper cladding, balcony | `MI_Plywood_Weathered` | `#9C8564` | 0.80–0.92 / 0 |
| Frame, floors, truss | `MI_Timber_Dark` | `#4A3522` | 0.70–0.85 / 0 |
| Roof | `MI_Steel_Rusted` corrugated | `#7A3F1C` | 0.75–0.95 / 0.25 |
| Truss plates, hanger eyes | `MI_Steel_Bare` | `#5C5F63` | 0.50–0.65 / 1 |
| Rated tape on both hanger eyes and the first 50 cm of each cable | `MI_Tape_Rated` | `#F2C230` / `#141414`, 50 mm chevrons | 0.50–0.60 / 0 |
| Rooftop tank | `MI_HDPE_Blue` | `#1F4FA3` | 0.40–0.55 / 0 |
| Slack cable tape | `MI_Tape_Slack` | `#FF7A00`, emissive 0.15 | 0.40–0.50 / 0 |
| Awnings | `MI_Tarp_Blue` | `#2B5F9E` | 0.55–0.70 / 0 |

Wear story. The lower storey is older: thicker paint, more drips, chalk at child height. The upper storey is newer plywood going silver. Balcony rail polished by forearms. Rust streaks from every roof screw. Two HTR condemnation notices, one per door, orange header, `NOTICE OF DEMOLITION`. The neighborhood stencil at 1.6 m beside the lower door, and a hand-painted house number. Cooking smoke above the chimney, warm dust `#6E5A3C` on every horizontal. At night under the Office's lights the house is a dark mass with four tape marks above it: yellow-and-black on each hanger eye (rated, a Breaker's job, per 03 rule 7) and an orange ring on each slack cable (cuttable, any Cutter). Rated and orange tape are never within 1 m of each other. Interior bulbs are 0.5 emissive, warm, and never compete with a line.

## Variants

| Variant | Notes |
|---|---|
| Market side | Lower storey is a shopfront with a roll-down shutter. Default nearest the ring. |
| Family | Two balconies, more laundry, a drip net under the floor. |
| Paint pairs | Red and mustard, teal and violet. Never the same pair as a neighbor. |
| Tank empty | Rooftop tank translucent and light. House mass **PROPOSED** 2,350 kg. |
| Condemned | Both notices. One in three houses, all of them after an HTR Claim win (**PROPOSED**). |
| One cable cut | State: hangs at 40° from the surviving cable. |

## Physics

| Property | Value |
|---|---|
| Tier / mass | Heavy, 2,800 kg |
| Collision | Lower box 450 × 350 × 260 cm, upper box 300 × 350 × 220 cm, tank cylinder r 60 × 110 cm. Cables are constraints. |
| Center of mass | 150 cm above the lower floor, offset 30 cm toward the upper storey. |
| Friction | Hanging: 0, a pendulum (05 §6.1a). Once down on a platform: 0.6 wood, 0.5 steel, and it is scenery that breaks, not a load that moves. |
| Restitution | 0.1 |
| Break-free | Swinging on its cables: 0 N. Tearing one hanger: 1,000 N, rated (05 v0.2.1): Breaker, or Doubler at overspin; inclusive. Cutter on either slack cable: 0.6 s hold. |
| Fracture | 3,500 N·s, per the destructibles table (the small house is 2,000). Fragments: 4 Heavy wall slabs at 700, 700, 450, 450 kg (**PROPOSED**), 1 Medium roof at 250 kg (**PROPOSED**), Light contents totaling 250 kg. The rooftop tank is its own object. |
| Cable length | **PROPOSED** 10 m. Pendulum period about 6.3 s. |

Line acceleration (T / m): 400 N 0.14 m/s²; 600 N 0.21; 800 N 0.29; 1,000 N 0.36; 1,200 N 0.43. Those figures apply to a house sliding free. On its 10 m cables it is a pendulum and steady deflection is atan(T / mg) with mg = 27.5 kN: 400 N holds it 0.8° off plumb (15 cm); 600 N 1.3° (22 cm); 800 N 1.7° (29 cm); 1,000 N 2.1° (36 cm); 1,200 N 2.5° (44 cm). Released, the bottom-of-swing speeds are 0.14, 0.22, 0.29, 0.36, 0.43 m/s: at best a flip against a player, never a fracture. Lethal motion comes from gravity (05 v0.2.1: cut cable, Cable Day settle, Sway pumping). A cut slack cable drops the house one tier (**PROPOSED** 4 m) and it passes the bottom at 8.9 m/s, 24,920 N·s; a single cut or hanger tear leaves it settling to 40° on the surviving cable with the low corner sweeping at about 5 m/s (**PROPOSED** speed).

Against a player (2,800 kg × relative speed): stagger 300 N·s at 0.11 m/s; flip 600 at 0.21 m/s; Stall 1,200 at 0.43 m/s. At 2 m/s: 5,600 N·s. From a 7 m drop (11.7 m/s): 32,760 N·s. At the Haul cap (25 m/s): 70,000 N·s.

The fracture threshold is 3,500 N·s, scaled by mass from the small house's 2,000. For this house that is a self-impact of 1.25 m/s, a walking pace, and 0.82 m/s above its own Stall speed against a player. No line swings it past 0.43 m/s on its cables, so a line alone never breaks it; a cut cable does, and the one-cable 40° settle does. Once it is moving under gravity it breaks on anything solid, and the design intent is that it does: the slabs are the weapon. A player's body counts against the threshold, but a 220 kg Breaker flipped into it at 10 m/s (2,200 N·s) only dents it; the same Breaker breaks a small house. A 700 kg slab Stalls at 1.7 m/s, a 450 kg slab at 2.7 m/s. Each slab falling 1 m (4.4 m/s) delivers 3,080 and 1,980 N·s. Crush: the house's weight is 27.5 kN; a pin is a Stall at 0.5 s. The 8 kN crush threshold is unreachable by any line; it comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1). A single slab (4.4 to 6.9 kN) does not reach 8 kN by weight alone; it needs a line or a slope behind it.

## Interactions

- **Haul to it:** the player is pulled in; the house barely swings. From below, a Haul does not lift them (only a Breaker's 1,000 N lifts a 90 kg Climber outright); they Hang and climb by Reel at 1.0 m/s (05 §3.2). If a cable is cut mid-Haul they ride it down.
- **Jack from it:** a Slingshot wall. The Kick's V splits by inverse mass: a Breaker (V 10) leaves at 9.3 m/s and the house takes 0.7 m/s; a Climber (V 18) leaves at 17.4 m/s and the house takes 0.6 m/s. Either nudge is over the house's 0.43 m/s Stall speed. Dog A house, Dog B enemy, Jack: no Kick with a Dog in a Catch Plate (05 §3.2); the enemy skids at T over their mass and the house deflects under 1°.
- **Hang from it:** a Pendulum or Gallows anchor from the truss, for any class, up to the class Hang load: break-free is tested against active pull only (Haul, Jack, the Kick), so the player's static load passes through the rated 1,000 N hangers without unseating them (05 §6.2). The Anchor Drop: Hang an enemy to the house, then cut both cables or tear both hangers.
- **Object ↔ object:** this house into a small house at 1 m/s relative: 2,800 N·s on both. The small house (2,000) shatters; the large house (3,500) is dented and keeps swinging. At 1.25 m/s relative and above, both shatter. Into a market stall: gone. Into the Lift counterweight's guide frame: the house breaks, the frame does not.
- **Object ↔ kinematic:** the Lift counterweight passing at 3 m/s relative delivers 8,400 N·s. The house breaks. The counterweight also severs any line from the house to a player that it passes through (the snap, both Dogs release; 05 §3.5).
- **Water:** none in Hangtown. The rooftop tank is its own object and releases a force volume when it ruptures.
- **Updraft, Breathing:** not present. Force volumes act on free bodies only, authored on a 100 kg reference body and capped at 2 × a_ref per body (05 §6.6); a cable-hung house would feel nothing until a cable is cut, and its spilled Light contents would feel the full authored value, capped at 2 × a_ref.
- **Floats / slides / rolls:** swings. On one cable it hangs at 40° and spills contents toward the low corner (Cable Day behavior, pre-authored).
- **Chain reactions:** house drops onto a swing bridge, the bridge drops its occupants, the slabs continue through the Low Hang nets (×2.0 Stall surface) and into the Well.

## States

| State | Trigger | Visual | Physics |
|---|---|---|---|
| Intact | authored | Hung level on two cables | Kinematic sway outside the four-house budget, dynamic inside it |
| Dented | impact 1,500–3,500 N·s | Lower cladding caved, balcony rail gone, shutters open | None |
| One cable | one slack cable cut or one hanger torn | Hangs at 40°, contents slide out as Light props | Single-point pendulum, 2,800 kg |
| Broken | over 3,500 N·s | Walls peel into four slabs, roof lifts, upper storey collapses into the lower | 4 Heavy slabs, 1 Medium roof, Light contents 10 s |
| Destroyed | in the Well | Gone | Removed |

## VFX and SFX hooks

- Impact: `SFX_Impact_House_Timber_L`, `_Sheet_L`, lower pitch than the small house. `SFX_Contents_Rattle_L` over 300 N·s.
- Cable cut: `SFX_Cable_Fray` (0.6 s), `SFX_Cable_Part`, then `SFX_House_Tilt_Groan` for the 40° drop.
- Hanger tear (1,000 N, rated): `SFX_Groan_Rated_House` for a 1.0 s minimum lead, `NS_Dust_Rated` from the eye, then `SFX_Lurch_Steel_H` and `NS_Dust_Lurch` at release.
- Fracture: `SFX_House_Shatter_L`, `NS_Impact_Timber_H`, `NS_Debris_Contents_L`, `NS_Dust_Plaster_L`, `NS_Laundry_Scatter`.
- Emissive: slack tape 0.15, interior bulbs 0.5 warm, off at Broken.

## Technical

- LOD0 16,000 tris; LOD1 8,000; LOD2 3,000; LOD3 600. Nanite off (geometry collection). Static LOD2 proxy for distant kinematic sway.
- Textures: 2 × 2K (lower storey and roof, upper storey and contents) plus `T_Trim_Timber_01`, stencil atlas, chalk decals.
- Pivot: the primary hang point, midway between the two hanger eyes on the truss. X+ toward the lower door. Z+ up.
- Dog sockets: `SKT_Dog_Hook_01`, `SKT_Dog_Hook_02` (hanger eyes, `Metal`), `SKT_Dog_Top_01` (truss ridge, `Metal`), `SKT_Dog_Top_02` (tank saddle, `Metal`), `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01` (lower cladding, `Metal`), `SKT_Dog_SideE_01` (upper plywood, `Wood`), `SKT_Dog_Door_01`, `SKT_Dog_Door_02` (`Wood`), `SKT_Dog_Bot_01` (floor joist, `Wood`). Fragments: `SKT_Dog_F1_01` through `F4_01` slabs, `SKT_Dog_F5_01` roof.
- Chaos: `GC_HouseLarge_*`. L0 whole, damage threshold 3,500 (**PROPOSED** 1 unit = 1 N·s). L1: 4 slabs, roof, upper-storey cluster, contents cluster, threshold 1,500. L2: contents into Light props, threshold 300, `RemoveOnMaxSleep` 10 s. Anchor field `FS_Anchor_HouseLarge` with two anchor points, each releasing at 1,000 N of line force (rated, inclusive, after the 1.0 s groan), a Cutter hold, or Cable Day's script; losing one switches the constraint to the surviving eye and plays the authored 40° settle. Damage propagation off so the house lands whole and breaks on contact.
- Replication: RC-H when dynamic (15 Hz), RC-K kinematic sway outside the four-house budget. Fracture under RC-GC; slabs stay RC-H, roof RC-M.

## Reference search terms

two-storey stilt house corrugated plywood, hillside shanty balcony timber rail, rooftop plastic water tank blue, suspended building on cables concept, demolition notice door orange, plywood weathered silver delaminating, timber truss hanger eye bolt.
