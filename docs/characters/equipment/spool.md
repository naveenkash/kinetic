# The Spool

_The wrist device. The Smart Cable. A rigger's load-moving tool, descended from the Strand's field-coupled climber drive, worn on the right forearm._

## Physical description

A drum housing clamped to the right forearm. Inside, a drum holds the live filament. Outside, two Dogs sit in cradles on the housing's outer face, and a line exit with a ceramic guide sits at the wrist end. A ring around the drum face lights in team tint and shows mode. An Office inspection seal is fixed to every sanctioned Spool at the elbow end.

| Class | Housing length x diameter | Position on forearm | Drum material | Mass (**PROPOSED**) |
|---|---|---|---|---|
| Climber | 9 x 6 cm cuff | Wrist, like a bracelet | Titanium, silver tape | 0.9 kg |
| Scrapper | 16 x 9 cm, plus bolted-on capacitor can | Mid-forearm | Brass (Guild cradle), modified | 2.1 kg |
| Rigger | 16 x 10 cm | Mid-forearm | Brass face, steel drum | 2.4 kg |
| Doubler | 28 cm twin housing, two 11 cm drums | Full forearm, elbow to wrist | Bare steel, heat-scored | 4.6 kg |
| Breaker | 22 x 14 cm | Low, against the elbow | Anodized aluminum shell | 5.8 kg |

Drum capacity is the class line length plus 10% (Climber 55 m, Rigger 44 m, Doubler 33 m per drum, Scrapper and Breaker 22 m). The filament is 6 mm in fiction and rendered thicker; see `line.md`.

### Moving parts

- **Drum.** Spins to pay out on fire and on Jack, winds in on Haul and on recall. Visible through a slot on all classes except the Breaker, whose shell has a window.
- **Brake.** A clamp that visibly closes on the drum when Hang engages. On the Rigger it is a brass lever that drops; on the Breaker a hydraulic pad; on the Climber a thin latch.
- **Dog cradles.** Two per drum. A docked Dog sits nose-out. An empty cradle is a visible hole: the player and anyone close can count Dogs in flight.
- **Line exit.** A ceramic eye at the wrist end. The line leaves here. The first 10 cm of line carries the Rope Mark tape bands.
- **Capacitor.** A sealed cell behind the drum. Once per Jack it dumps into the line for 0.3 s, far above motor pull T, and then the Spool falls back to T. Riggers call the dump **the Kick**. The Doubler has one per drum. The Scrapper's is the external can bolted to the housing, the same component moved outside by a modder. **PROPOSED** look: a white flare through the drum slot (the shell window on the Breaker) that peaks at 0 s and is gone by 0.3 s, a 2 mm housing jolt, and a wisp of ozone haze from the exit.
- **Interlock.** An Office-sealed cutout on the capacitor circuit. The Kick never fires when either Dog is seated in another player's Catch Plate; a Jack against a player is the motor push at T only. Visible as a second small seal beside the inspection seal. Red-match Spools have it filed off. Sanctioned ones, including every Fray Spool the Office lets onto a claim, have it intact.
- **Mode ring.** A ring of 12 segments around the drum face, team tint. Haul: segments sweep inward toward the center. Jack: segments sweep outward. Hang: all segments solid. Off: dim.

## Visual states

| State | Drum | Ring | Other |
|---|---|---|---|
| Idle | Still | Dim (10%) | Both Dogs docked |
| Dog A fired, waiting for B | Slow turn | Dim, one segment blinking | A cradle empty. Slack line dim. 4 s timer on the blink rate. |
| Live Haul | Winding in, speed tracks closing speed | Inward sweep, brightness = `ForceNorm` | Housing emissive at the exit scales with force |
| Kick (first 0.3 s of Jack) | Free-spins paying out | All segments white, then outward sweep | Capacitor flare at the housing, `KickFlare` 1 to 0 over 0.3 s. Dog cradles glow briefly. |
| Live Jack (after the Kick) | Paying out at the push rate | Outward sweep, brightness = `ForceNorm` at T | |
| Hang | Locked, brake closed | Solid | Brake clamp visible |
| Hang slipping | Jerks 0.5 m per 0.1 s | Solid, flickers on each slip | Creak |
| Reel in (Hang, holding Haul) | Turns slowly, winding in at 1 m/s, brake half-open | Solid, with one segment stepping inward per 0.25 m | A ratchet click every 0.25 m. Bears up to the class Hang load; against more than that it slips instead. The Gallows hoist. |
| Reel out (Hang, holding Jack) | Turns slowly, paying out at 1 m/s | Solid, one segment stepping outward per 0.25 m | Same click. No Kick: this is Hang, not Jack. |
| Overspin (Doubler) | Winding, 0 to 1.0 s accelerating as force climbs from 800 toward 1,200 N | Sweep speed doubles, at 1.0 s ring goes white | Heat score on the gauntlet brightens from `Overspin` 0 to 1; housing shakes at 2 mm amplitude |
| Snap | Drum free-spins 0.3 s then stops | Ring flashes white, then off | Recoil 4 cm; Spool unavailable 3 s with ring dark |
| Recall | Winding in at class speed (0.4 / instant / 0.6 / 0.6 / 0.9 s) | Segments collapse to center | Dogs reappear in cradles with a small flash |
| Set line left behind | Drum freed, line detached | One segment held dim per set line | Rigger shows two |

## VFX hooks

- `FX_Spool_Fire`: a short puff of dust from the exit and a 0.1 s tracer in team tint along the Dog's path.
- `FX_Spool_Exit_Glow`: emissive bloom at the exit scaled by `Force_N`, same curve as the line (`line.md`).
- `FX_Spool_Kick`: white flare through the drum slot, a housing jolt, and an ozone wisp from the exit, 0.3 s. Doubler fires it per drum.
- `FX_Spool_Overspin_Heat`: heat shimmer and sparks from the lower drum, Doubler only.
- `FX_Spool_Snap_Recoil`: a radial spark burst and a smoke wisp from the exit.
- `FX_Spool_Recall`: Dog streaks back along the line and docks with a flash.

## SFX hooks

| Event | Sound | Class pitch multiplier |
|---|---|---|
| Fire | A pneumatic thunk | Climber 1.2, Scrapper 1.3, Rigger 1.0, Doubler 0.95 and 1.05 (twin), Breaker 0.7 |
| Drum spin | Whine, pitch tracks drum speed | As above |
| Mode switch | Click (Climber, Scrapper), ratchet (Rigger), detent clunk (Breaker), double click (Doubler) | |
| Kick | A capacitor crack and a short whoosh, 0.3 s, louder than the fire thunk. The Scrapper's external can adds a rattle. Silent (no Kick) when a Dog is in a player's plate. | As the fire thunk |
| Reel | A ratchet click every 0.25 m, four per metre at 1 m/s, over a faint motor hum. The only Spool sound that is quieter under load. | Click pitch by class: Climber high, Breaker a detent clunk |
| Hang brake | Clack | |
| Slip | Creak, 0.1 s per 0.5 m | |
| Overspin | Rising whine; octave jump at 1.0 s, the loudest Spool sound in the game | Doubler |
| Snap | Universal snap (see `line.md`), plus a drum free-spin rattle | |
| Recall | Wind, class duration | |

The Spool spin pitch is a class tell. A player who hears a 0.7x fire thunk behind them knows a Breaker is there.

## Technical

| Item | Spec |
|---|---|
| Sockets | `S_Spool` root on `lowerarm_r`. `S_Spool_LineExit` (and `S_Spool_LineExit_2` on Doubler) for line origin and projectile spawn. `S_Spool_DogA_Rest` to `S_Spool_DogD_Rest` for docked Dog meshes. |
| Mesh | Separate skeletal mesh attached to the character, with a drum bone (`spool_drum`) and a brake bone (`spool_brake`) for procedural spin and clamp. Doubler has `spool_drum_2`, `spool_brake_2`. |
| Poly | LOD0 6k tris (Doubler 11k), LOD1 2.5k, LOD2 800. First-person uses LOD0 always. |
| Textures | 2K set per class. First-person gauntlet on the Doubler 4K. |
| Material params | `TeamTint`, `Force_N`, `ForceNorm`, `Overspin`, `KickFlare`, `ModeIndex` (0 off, 1 Haul, 2 Jack, 3 Hang), `SpinRate`, `SetLineCount`, `Wear`, `Heat` (Doubler gauntlet score brightness). |
| Audio params | `Force_N` to pitch via the shared line curve; `SpinRate` to drum whine; `ClassPitch` per class. |
| Emissive curve | Exit glow intensity = 0.3 + 42 x `ForceNorm`^1.5 (`ForceNorm` = `Force_N` / 1,200), identical to the line core so the glow is continuous from housing to Dog. A class at its own T reads: Climber and Scrapper 8.4, Rigger 15.1, Doubler 23.2, Breaker 32.2, overspin ceiling 42.3. |
| Replication | `ModeIndex`, `Force_N` quantized to 25 N at 10 Hz, interpolated client-side. Drum spin is cosmetic and local. |
| Office seal | A 2 cm disc decal at the elbow end, white, present on every sanctioned Spool. The Scrapper's and Doubler's seals are intact; everything around them is not. |

## Reference search terms

industrial cable winch drum and brake, rescue hoist housing, fly-fishing reel drag mechanism, climbing ascender cam, anodized ratchet strap winch, brass fishing reel patina, blued steel heat score.
