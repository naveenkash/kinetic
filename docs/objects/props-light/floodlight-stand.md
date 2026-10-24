# Floodlight stand

Light prop. Light tier, 22 kg. 90 cm tripod footprint, mast 240 cm tall, twin 30 × 20 cm LED heads on a crossbar, cable to a floor socket. Appears in the Yard (the Floor, around the control shack and at each trench crossing; the Rails, on the crane walkway as a `Clamped` variant), Hangtown (the Mid Hang market platform and the Upper Hang walkways, where the Office puts up the match lights), and the Anchor (the Scaffold, one per level per quadrant; the Rim, near the broadcast towers). Not in Ballast, which is lit by the sky and the Crown's work lights.

## Role in play

The floodlight stand is the one Light prop that changes the room when it moves. It is a local light source. Knock it over and a 6 m patch of floor goes dark except for lines, which is the whole point of the Visibility Clause (03 rule 8). It staggers at the Haul cap and flips at the Jack cap, and it falls like a tree, which players learn to read.

- Named rigs: the Clothesline, with the light pointed at the attacker's side so the line is in glare. The Gallows, with the stand as the "anchor" in a bad plan: it is not an anchor, it comes down, and the Rigger who tried it learns something. Blind play: Haul a stand down to kill the light on your own doorway.
- Scrapper: Hauls the stand over into an enemy for a stagger and a dark corner, then Hauls the barrel they can no longer see coming. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: sets a Clothesline across the lit side of a doorway and a floodlight in the eyes of anyone coming through. Then knocks the stand over once the enemy commits. As a lure: Dog the stand, Dog an overhead anchor, Hang, and Reel it up (within Hang, holding Haul shortens the rod at 1.0 m/s, holding Jack lengthens it, bearing up to the class Hang load) until the lit stand hangs at head height in a doorway; release it as a set line and wait behind the glare.
- Breaker: walks through it. A fallen stand under a Breaker's feet is a tripping hazard for everyone but the Breaker.
- Climber: a stand's crossbar is a 2.4 m perch. Climbers Dog the head, Hang, and swing the light around to blind a gallery. The head is a Dog magnet because it is bright.
- Doubler: a stand on a full 1,200 N Snapback receives 1,500 N·s: 68 m/s on 22 kg, capped at the 50 m/s terminal velocity, arriving with 1,100 N·s. A hard flip, not a Stall; 1,200 N·s would need 55 m/s and nothing in the game moves faster than 50. The light strobes on impact.

## Appearance

Silhouette: a tall thin T on three splayed legs, with a cable trailing to the floor. The two heads are bright rectangles; the mast is a dark line. When fallen, a long diagonal with a glare at one end.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Mast and legs (38 mm tube) | `MI_Steel_Painted_Worn` | `#F2C230` Office yellow; HTR variant `#E8620C`; Guild variant `#C8912A` | 0.45 to 0.70 | 1 where chipped |
| Crossbar and head yokes | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |
| Head housings (die-cast) | `MI_HTR_Grey_Matte` | `#4A4D50` | 0.60 | 0 |
| Lenses | `MI_Glass_Lamp` | `#E8EEF2` | 0.05 | 0 |
| Cable (20 m, coiled at base) | `MI_Rubber_Black` | `#1E1F20` | 0.80 | 0 |
| Sandbag on one leg | `MI_Canvas_Natural` | `#B9AE93` | 0.95 | 0 |

Wear story: paint chipped at the leg feet and along the mast where it is carried on a shoulder. The telescoping mast collar is polished bare. Heads are scorched grey-brown above the lens from heat. The cable has tape repairs in whatever color the crew had: Office white, Guild ochre, hanger red. Moth bodies on the lens in Hangtown.

Stencils: Office stands carry `OFFICE OF THE LAY / MATCH LIGHTING / UNIT 14` in white on the mast and a LayCast sticker on the head housing. HTR stands carry `HTR-RC-6002`. Guild stands are ochre with the hitch glyph. A sponsor strip on the crossbar: `Nurri Stall-Gel` cyan on Office stands in the Anchor; `Kessl Harness Leasing` navy on the Yard's.

Under arena lighting: it is arena lighting. Each head is a spot light actor, 60° cone, 8,000 lm, 4,500 K (**PROPOSED**), attenuation 12 m. The line emissive is tuned to read at full against a floodlight at 3 m; confirm in the Anchor's darkened sector that a stand pointed at a line does not wash it out.

## Variants

| Variant | Difference |
|---|---|
| `Office_Tripod` | Default. Yellow. Loose on floor, sandbag on one leg. |
| `HTR_Tripod` | Orange. Yard and Rim compound. |
| `Guild_Tripod` | Ochre, older pattern, one head. 18 kg. |
| `Clamped` | Mast only, scaffold-clamped to a rail. No tripod. 14 kg. Break-free 800 N (bolted or chained). Rails, Scaffold. |
| `Fallen` | Pre-authored on its side, one head cracked, flickering. Scrap heaps. |
| `Dead` | Heads dark, lens cracked. 22 kg. No light actor. |

## Physics

- Tier: Light. Mass: 22 kg (Guild 18 kg, Clamped 14 kg).
- Collision: mast capsule radius 2 cm × 240 cm; three leg capsules radius 1.5 cm × 90 cm; crossbar capsule 1.5 cm × 100 cm; two head boxes 30 × 20 × 10 cm. Cable is not collision.
- Center of mass: 95 cm above the floor, on the mast axis. High and narrow: it tips at 14° of lean (**PROPOSED**).
- Friction 0.6, restitution 0.15 (**PROPOSED**).
- Break-free: loose on floor 0 N. `Clamped` variant, bolted or chained, 800 N: a Doubler (800 N, at threshold) or a Breaker (1,000 N) rips a clamped light off a rail; a Rigger, Climber, or Scrapper only makes it creak.
- Haul acceleration, tripod, with the far end static: 18.2 m/s² at 400 N (Climber, Scrapper), 27.3 at 600 N (Rigger), 36.4 at 800 N (Doubler), 45.5 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 17.2 m, 11.5 m, 8.6 m, 6.9 m. The stand tips at the first newton, so it arrives mast-first and spinning.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a standing tripod gives it 150 N·s, 6.8 m/s, and it goes over.
- Fracture: 450 N·s (**PROPOSED**): the crossbar separates and a head breaks. A player's body counts against the threshold: a flipped 90 kg Climber landing on a stand at 5 m/s is 450 N·s and breaks it. Fragments: mast with legs (14 kg Light), crossbar with one head (6 kg Light), one loose head (2 kg Light), 10 s lifetime for the smaller two. The light actor on the loose head stays on for 2 s, strobes, then dies.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 550 N·s | Stagger |
| Scrapper Haul | 25 m/s cap | 550 N·s | Stagger |
| Jack | 30 m/s cap | 660 N·s | Flip |
| Fall from 7 m | 11.7 m/s | 257 N·s | Nothing |
| Tipping over onto a player from standing | head at 6 m/s | 132 N·s | Nothing, but the player is now in the dark |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 1,100 N·s | Flip |

A floodlight stand cannot Stall a player on a normal line. Its effect is light: the 12 m attenuation radius goes dark, or points somewhere new, and players in it lose the floor read while every line in it gets brighter by contrast.

## Interactions

- Haul to it: the stand tips toward you at the first newton and arrives mast-first. The heads swing past your face: a 0.3 s glare overlay (cosmetic).
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the stand off at 14.5 m/s and takes 3.5 m/s backward; a 110 kg Scrapper (V 14) sends it at 11.7 m/s and takes 2.3 m/s. Then T pushes the stand over backward at 18.2 to 45.5 m/s² by class toward the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1) and its light sweeps the ceiling. The Kick never fires with a Dog in a Catch Plate: a Jack from the stand to an enemy is the T push alone, the stand leaving from rest at 18.2 m/s² on a 400 N line and reaching 25 m/s at 17.2 m, while the enemy skids back at T over their mass. Jacking off a `Clamped` stand with a line under 800 N treats it as static and gives you the whole of V.
- Hang from it: break-free is tested against active pull only (Haul, Jack, and the Kick), so the `Clamped` stand's 800 N value does not apply to a hanging player. What the clamp does under a player's weight is an authoring call: it is a wind clamp on a 38 mm tube, not a lifting point, so a hanging harness tips the mast over the rail and comes down with it after 0.5 s of creak (**PROPOSED** physical behaviour). A tripod stand loose on a floor holds nothing: a Hang to it brings 22 kg of dead weight along, lamp swinging.
- Object to object: a stand falling across a scrap heap sheds its heads into the heap pool. A stand Hauled into a propane cylinder at 25 m/s triggers it. A stand across a trench is a bridge nobody should trust: the mast box takes 220 kg at mid-span and fractures (450 N·s from a Breaker's landing).
- Kinematic: a Hauler flattens a stand in a trench and drags the cable, which pulls the floor socket and sparks. The Gantry magnet takes a steel stand within 1 m. The Lift counterweight sweeps stands off the Mid Hang. The Maintenance Ring severs any Clothesline it passes through (05 §6.7: the snap, 150 N·s per 1,000 N recoil to both ends, both Dogs release), so a Clothesline lit by a stand on the Scaffold top lasts until the Ring comes round; lines attached to the Ring itself are not severed.
- Water: sinks; the heads short with a spark burst and die.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 22 kg would be 4.5, so a loose tripod feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: it goes over at the Breath's peak and tumbles, beam sweeping, and the sector it lit goes dark. A `Clamped` stand (800 N) is unaffected until freed, while a stand hung on a player's Hang line as a lure is a free body and swings in the gust; give its beam a cosmetic 1° wobble on the gust as a tell (**PROPOSED**). The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) walks fallen tripods toward the Stump over a match.
- Breathing tilt: not in Ballast.
- Chain reactions: a stand's cable is a tripwire: a fallen stand's cable across a walkway staggers nobody (it is not a Hang line), but the cable mesh is visible and players treat it as one. Leave it.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Standing, both heads lit | 22 kg, light on |
| Dented | Leaning on a bent leg, one head flickering at 4 Hz | No change; light flicker |
| Broken | Fallen, crossbar off, one head dark | 14 kg mast plus two Light fragments; one head lit for 2 s then off |
| Destroyed | Mast bent, lenses gone | 14 kg Dead variant; no light |

## VFX and SFX hooks

- Impact on steel: `SFX_Mast_Steel` (long tube ring, 1.2 s decay). Concrete: `SFX_Mast_Concrete`. Wood: `SFX_Mast_Wood`. Water: `SFX_Mast_Splash` plus `SFX_Electric_Short`.
- Falling: `SFX_Mast_Topple` (a creak, then the hit) and the head's hum pitch bends as it swings.
- Light events: `SFX_Floodlight_Flicker`, `SFX_Floodlight_Die` (a tick and a fading hum). `NS_Sparks_Socket` when the cable rips its floor socket. `NS_Glass_Lens` on lens break (30 shards, 1 s).
- Break-free from a clamp: `SFX_Clamp_Shear`, `NS_Sparks_S`.
- Emissive: lens material emissive 20 at full, driven by the light actor's intensity; drops to 0 over 0.3 s on death. The head housing's rear vents glow faint orange `#E8620C` at emissive 0.5 when on.

## Technical

- LOD0 2,000 tris, LOD1 1,000, LOD2 400, LOD3 120 (mast plus two emissive cards). One 1K set; heads share `T_Trim_Steel_01`.
- Pivot: bottom center of the tripod footprint. `Clamped` variant, the clamp center.
- Dog sockets: `SKT_Dog_Top_01` (crossbar center, Metal), `SKT_Dog_Frame_01` (mast at 120 cm, Metal), `SKT_Dog_Frame_02` (rear leg at mid-length, Metal), `SKT_Dog_Hook_01` (head yoke, the usual Climber Hang point, Metal). Mag Dogs hold at full load.
- Light: `SpotLight` component per head, 60° outer cone, 8,000 lm, 4,500 K, attenuation 12 m, shadows on for LOD0 and LOD1 distance only (**PROPOSED**). The light sector is registered with the arena's lighting manager so a dead stand reports its radius as dark.
- Chaos: `GC_FloodlightStand`. L0 whole, L1 mast and legs, crossbar and head, loose head. Damage threshold L0 = 450 (1 unit = 1 N·s, **PROPOSED** calibration). Anchor field `FS_Anchor_Floodlight_Clamped` for the Clamped variant, released at 800 N.
- Replication: RC-L for the body. Light state (on, flicker, dead) is a replicated byte so all clients darken the same sector the same frame.

## Reference search terms

LED site floodlight tripod twin head yellow; telescopic work light stand sandbag leg; fallen construction light stand night; scaffold clamp mounted floodlight; film set work light cable taped repair; floodlight lens heat scorch housing.
