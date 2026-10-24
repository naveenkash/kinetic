# Harness Shared Components

_Every harness, regardless of class, carries the same five Office-regulated systems: the Stall Gel system, the telemetry pack, the lease plate, tape wraps, and tallies. This document specifies them once. The class docs give placement._

## 1. Stall Gel system

**Fiction.** When telemetry reads an impact that would have been lethal, the gel floods, the harness locks, and the rigger is scrapped. Stall-Gel is a Nurri product and the Nurri logo is on every reservoir. The Office publishes a 0.3% fatality rate. The Guild's Ledger says 2%.

### Reservoirs

| Class | Reservoirs | Capacity each (**PROPOSED**) | Placement |
|---|---|---|---|
| Climber | 2 flat packs | 0.6 L | Under the back fin, 2 cm thick |
| Scrapper | 1 cylinder plus a retail refill bottle | 1.2 L plus 0.5 L | Single spine rail; bottle zip-tied beside it |
| Rigger | 2 cylinders, 7 x 22 cm | 0.8 L | Spine frame rails at kidney height |
| Doubler | 2 cylinders, labels removed | 0.8 L | Cut-down spine frame; Fray runners chalk odds on them |
| Breaker | 2 flat tanks | 1.2 L | Inside the shell; level LEDs on the back plate flanks |

Each reservoir has a sight glass showing gel at `#E8D9B0`, 60% opacity, roughness 0.1. Sockets `S_Gel_L` and `S_Gel_R` on `spine_02`.

### Channels

Translucent 4 mm lines run from the reservoirs under the straps to the hips, shoulders, neck, and elbows. Normally invisible under wraps and straps; on the Climber they show as faint lines under the white wrap.

### The Stall sequence, visually (05 §7.4)

| Time | What happens |
|---|---|
| 0.00 s | Threshold crossed. All constraints lock over 0.2 s; the body freezes in its current pose. |
| 0.00 to 0.20 s | `GelFlood` 0 to 1: gel blooms from the channel outlets at every joint as a translucent amber-white swell 2 to 4 cm thick, with a wet specular. Reservoir sight glasses drain to empty. Plate rim bands go red. |
| 0.20 s | Scrap Card to killer and victim. Marshal calls it. |
| 0.20 to 4.00 s | Body persists as a Medium-tier prop at its class mass (90 to 220 kg). Gel holds its shape, surface skins over (roughness 0.1 to 0.4). |
| 4.00 to 6.00 s | A Hook lifts the body out by `S_Hook_Lift`. |

SFX: a pressurized hiss and a wet click of the lock, 0.2 s, then silence from the body. VFX: `FX_Stall_Gel` (joint blooms), `FX_Stall_Lock` (a single frame of white at the plates).

### Stall Gel pads

Rare, prep-crew-placed landing pads, 2 x 2 m, 20 cm thick, Nurri-branded, amber `#E8D9B0` over a grey base. Stall threshold x4.0 on landing (05 §7.3). They are the only soft surface that advertises itself.

## 2. Telemetry pack

The Office's box. It decides whether the impact would have been lethal, and it feeds the broadcast.

| Property | Spec |
|---|---|
| Size | 14 x 10 x 4 cm (Climber 10 x 8 x 3). Integrated into the Breaker's back plate. |
| Placement | `S_Telemetry` on `spine_03`, under the back plate. Scrapper: strapped to the belt because the back plate is low. |
| Finish | ABS `#2B2E31`, roughness 0.8. A lead Office seal, 2 cm white disc, on the lid. Serial etched. |
| Status LED (**PROPOSED**) | 5 mm, on the lid edge, visible from behind. White: live. Blue: airborne or hoisted. Red: scrapped. Off: Drop phase (lines disabled). |
| Tamper | The Fray tampers with everything except this. A broken seal is a forfeit. Doubler models show file marks stopping 1 cm from the seal. |

Audio: a soft double beep on "Live," a flat tone on Stall, heard only by the wearer.

## 3. Lease plate and serial

Every rigger starts owing on their harness lease. Kessl Harness Leasing is the sponsor whose name is on the tag.

| Property | Spec |
|---|---|
| Size | 6 x 3 cm (Breaker 8 x 4, Climber 5 x 2.5) |
| Placement | `S_LeasePlate`, lower-left of the chest plate |
| Serial format (**PROPOSED**) | `K<claim year>-<class letter>-<5 digits>`. Example: `K61-R-04418`. Class letters: C Climber, S Scrapper, R Rigger, D Doubler, B Breaker. |
| States | **Leased:** serial only. **CLEAR:** serial struck through with a chisel line and CLEAR stamped beneath, the Weight paid off. **SPONSORED:** the sponsor's mark stamped beside the serial. **Filed:** the Fray grinds the plate to a bare rectangle; some engrave SOLD in the blank. The Office fines this every match and the Fray pays. |
| Faction defaults | Guild Faces: CLEAR. HTR: sequential serials, bulk-leased, SPONSORED by HTR. Ascendancy: SPONSORED with a sect mark. Hangers: leased, often re-stamped. Fray: filed. |
| Material | Brass (Guild, Hangers), stainless (HTR, Ascendancy), bare steel (Fray). Stamped lettering 2 mm deep with dark fill. |

The player's serial is generated from their Ledger and rendered by `LeaseSerial` into a decal atlas. Reaching Clear changes the decal; it is a visible milestone.

## 4. Tape wraps

Tape appears in two places: on the harness straps and Spool mount as faction and crew identity, and on the Line within 10 cm of the Spool as Rope Marks.

| Faction | Default tape | Width | Pattern note |
|---|---|---|---|
| Guild | Ochre `#C8862B` and black `#141211` | 2.5 cm | Hand-wrapped, slightly uneven |
| HTR | Grey `#5A5E62` and orange `#FF5A1F` | 2.5 cm | Machine-wrapped, perfect |
| Ascendancy | Silver `#C9CDD2` on white wrap | 2 cm | Smooth, overlapping |
| Hangers | Sun yellow `#E8C23A` and rust `#7A3B22`, or whatever was in the bin | Any | Mixed rolls |
| Fray | Neon green `#39FF14`, magenta `#FF2BD6`, yellow `#FFE600` on black | 2.5 cm | Full saturation, never in a team-tint hue |

### Rope Mark bands on the Line

| Tier | Band | Placement from the Spool exit |
|---|---|---|
| Tape | One 1 cm band in the rig's color | 3 cm |
| Band | Second 1 cm band | 5 cm |
| Mark | Third band plus a title on the Scrap Card | 7 cm |

Rig colors (**PROPOSED**): Gallows black, Guillotine white, Clothesline red, Slingshot sky, Pendulum green, Kiss pink, Scatter orange, Caboose grey, Avalanche brown, Overboard blue, Anchor Drop gold. Eleven rigs, up to 33 bands; the shader shows the three highest tiers.

The prep crew removes any unofficial band (a band for a rig the Office does not recognize) before every match. Faces who wear one anyway get it removed on camera.

Material: tape is a decal layer on the strap and line materials, `TapeMask` plus `TapeColorA`, `TapeColorB`, roughness 0.4, slight edge lift on the hand-wrapped variants.

## 5. Tallies

Earned, never bought. Painted onto the plates by the Office's prep crew between matches, in white `#F2F2F0`, with a stencil brush. Visible in-match.

| Mark | Meaning (**PROPOSED**) | Form |
|---|---|---|
| Vertical stroke | One scrap | 2.5 cm tall, 4 mm wide, groups of five with the fifth struck diagonally |
| Small circle | One Well (ring-out) credited to you | 1.5 cm |
| Horizontal bar | One seasonal Claim won with your faction | 4 cm wide |
| Triangle | One Rope Mark title | 1.5 cm |

- Anchors: `S_Tally_01` to `S_Tally_08`. 01 to 04 on the chest plate (upper right working down), 05 and 06 on the right shoulder, 07 and 08 on the back plate. Each anchor holds a block of up to 25 strokes.
- Blocks never cover more than 15% of a plate (README rule 4). Past that, the oldest block is painted over with a single large numeral (**PROPOSED**).
- HTR plates are replaced every match, so the prep crew repaints HTR tallies onto fresh plates before every Drop. Rook Adeyemi's block is the largest on any Face.
- Tallies are rendered through `TallyMask`, generated from the Ledger.
- The paint wears: `Wear` adds edge chipping to older blocks.

## Technical summary

| Item | Spec |
|---|---|
| Sockets | `S_Gel_L`, `S_Gel_R`, `S_Telemetry`, `S_LeasePlate`, `S_Tally_01` to `S_Tally_08`, `S_Hook_Lift` |
| Material params | `GelFlood`, `TelemetryState` (0 off, 1 live, 2 airborne, 3 scrapped), `LeaseSerial`, `LeaseState`, `TapeMask`, `TapeColorA`, `TapeColorB`, `TallyMask`, `RopeMarkTier`, `Wear` |
| Textures | Shared 2K atlas for serials, tallies, sponsor marks, and tape patterns |
| Replication | `TelemetryState` and `GelFlood` replicated; everything else is cosmetic from the player's profile |

## Reference search terms

hydraulic fluid sight glass, airbag deployment high-speed stills, tamper-evident lead seal, engraved brass equipment tag, hand-stamped serial numbers, stencil-painted mission tallies on aircraft, hockey stick tape wrap patterns.
