# Fuel drum

Volatile. Medium tier, 180 kg full. 58 cm diameter, 88 cm tall (a 200 L closed-head drum). Appears in Ballast, the Bays (generator fuel at the catwalk landings) and the Hold (pre-Severance Meridian cargo, half-rusted); the Yard, the Floor (crane and Hauler diesel at the trench heads); Hangtown, the Low Hang (cooking oil and generator fuel under the garden terraces); the Anchor, the Rim (crane fuel beside each rim crane).

## Role in play

The heavy bomb. Four times a cylinder's mass, a smaller blast, and a fire patch that keeps the area denied for 6 s. It is dangerous before it explodes: 180 kg at 6.7 m/s is 1,200 N·s, a Stall, and it needs 10 m/s to trigger. A Hauled drum kills by mass first and fire second. It is over 100 kg, so the Scrapper's 30% bonus does not apply.

Why it is there: cranes, generators, and Haulers burn diesel. The prep crew stages the fuel where the plant needs it. The Office stencils it.

| Harness | What they do with it |
|---|---|
| Scrapper | 400 N is 2.2 m/s²: 9.4 m/s at the 20 m line end, 1,690 N·s, a Stall by mass alone, and under the 10 m/s trigger, so no blast. The Scrapper's one straight pull that works. |
| Rigger | The Guillotine: Hangs it from an overhead anchor (180 kg is 1.8 kN, within every class Hang load), Reels it up at 1.0 m/s, sets the line, and cuts or recalls it when the enemy passes. The drum falls by gravity: from 4 m it arrives at 8.9 m/s, 1,600 N·s, a Stall with no blast; from 5.1 m or more it passes 10 m/s and blasts as well. A second line only steers it. 600 N pulls it from a stack. |
| Breaker | 1,000 N is 5.6 m/s²: 15 m/s at 20 m, 2,700 N·s, and it triggers on arrival. Kisses it with a cylinder: the cylinder arms the drum. |
| Climber | Jacks from it: the Kick splits V 18 by mass, 12 m/s to the Climber and 6 m/s to the drum. The drum then crawls at 2.2 m/s². A launch pad that follows you. |
| Doubler | Hangs it on one line, overspins, and snaps it at 1.8 s: 1,500 N·s on 180 kg is 8.3 m/s, a Stall on whoever it meets, no blast, no fire. |

## Appearance

Silhouette: a squat ribbed drum, two rolling hoops, a closed top with a 2 inch bung and a 3/4 inch vent. Wider than it looks tall. Reads as "drum" at 50 m by the hoops alone.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Body | `MI_Drum_Fuel_Red` | `#8E2A1F`, oxidizing to `#B04A3A` | 0.45 to 0.65 | 0 |
| Hoops, chimes | `MI_Steel_Bare` | `#5C5F63` | 0.50 to 0.65 | 1 |
| Rust mask | `MI_Steel_Rusted` | `#7A3F1C` | 0.75 to 0.95 | 0.3 |
| Bung plugs | `MI_Steel_Bare` | | 0.5 | 1 |
| Stencils | `MI_Decal_Stencil` | white, black | | |

Wear: hoops polished bright from rolling. Paint lost in a 10 cm band at each hoop and on the top chime where it was stood on. Rust blooms from the bottom chime up. Diesel drips darken the body below the bung in a fan shape (`MI_Decal_Grime` `#2B2622`).

Markings: `OFFICE OF THE LAY / SITE PROPERTY`, white, 5 cm letters, with Marshal number. A black stencilled `DIESEL` and a UN flammable diamond. Supplier band **PROPOSED** `BASIN PETROL CO-OP` in white on a black band. Ballast's Hold drums carry the pre-Severance `MERIDIAN` hexagon in `MI_Meridian_Cyan` `#2A9FB5` at 0.9 wear with cargo code `C5-HAZ-07`.

Under arena lighting the drum is a dark red mass. No emissive. In the Yard it sits in rail grease. In Hangtown it is strapped with sisal and half the drums are cut down into stoves (those are scrap-kit pieces, not volatiles). On the Anchor it is pale with dust and bird lime on the top.

## Variants

| Variant | Difference |
|---|---|
| Full diesel (default) | 180 kg. Triggers. Fire patch. |
| Empty | 20 kg, Light tier (tier recomputes on mass change, 05 §6.3). Does not trigger. Booms hollow. Floats high. Scrapper lines move it 30% faster. |
| Half full | 100 kg, Medium, at the edge of the Scrapper bonus (under 100 kg qualifies; 100 does not). Triggers. Fire patch 2 m for 4 s (**PROPOSED** scaled by fill). Liquid slosh audible. |
| Meridian (Ballast Hold) | Cyan livery at 0.9 wear, rust at 0.8. Same physics. |
| HTR | `MI_HTR_Grey_Matte` with an orange top. Tag `HTR-RC-3310`. Wear 0.2. |
| Dented | Top chime crushed, one hoop sprung. Cosmetic. |

## Physics

Medium tier, 180 kg. Collision: one cylinder, radius 29 cm, height 88 cm. Center of mass at 42 cm when full, 44 cm when empty. Friction 0.50 steel on concrete, 0.40 on steel, 0.30 wet (**PROPOSED**). Restitution 0.10. Tips past 33°. On its side it rolls freely.

Break-free: loose on floor 0 N; in a cradle or on a drum rack 200 N (anyone); strapped to a pallet or chained 800 N (Doubler, Breaker only).

Haul, other end static or braced:

| Class | T | a | Line | Arrival speed | Impulse | Result |
|---|---|---|---|---|---|---|
| Climber | 400 N | 2.2 m/s² | 50 m | 14.8 m/s | 2,670 N·s | Stall, triggers |
| Scrapper | 400 N | 2.2 m/s² | 20 m | 9.4 m/s | 1,690 N·s | Stall, no trigger |
| Rigger | 600 N | 3.3 m/s² | 40 m | 16.2 m/s | 2,920 N·s | Stall, triggers |
| Doubler | 800 N | 4.4 m/s² | 30 m | 16.2 m/s | 2,920 N·s | Stall, triggers |
| Doubler overspin | 1,200 N | 6.7 m/s² | 30 m | 20 m/s | 3,600 N·s | Stall, triggers |
| Breaker | 1,000 N | 5.6 m/s² | 20 m | 15 m/s | 2,700 N·s | Stall, triggers |

The drum reaches 1,200 N·s (6.7 m/s) after 10 m on a Scrapper line and 4 m on a Breaker line. It reaches the 10 m/s trigger after 23 m (Scrapper, never), 15 m (Rigger), 11 m (Doubler), 9 m (Breaker). A short Haul kills without a blast; a long one kills and burns. The 25 m/s Haul cap is never reached.

Jack: the Kick splits V by inverse mass, then T pushes. A Scrapper's (110 kg) Kick gives the drum 5.3 m/s and the Scrapper 8.7 m/s; the drum then reaches 10.8 m/s at 20 m and triggers on whatever it hits. A Breaker gives it 5.5 m/s and takes 4.5; the drum reaches 15.9 m/s at 20 m. A Climber gives it 6 m/s and takes 12 (a Slingshot with a 180 kg backstop) and the drum reaches 16 m/s at 50 m.

Snap: 1,500 N·s × (F / 1,200 N). Full overspin on a drum: 8.3 m/s, 1,500 N·s, Stall by impulse, under the trigger. Cut recoil: 150 N·s per 1,000 N; a Breaker-line cut moves the drum 0.8 m/s.

Trigger: impact over 10 m/s, or a blast. Hiss and venting 0.3 s before the blast. Blast radius 3 m, impulse 1,200 N·s at center falling linearly to 0 at 3 m. Then a 3 m fire patch for 6 s: a force volume that staggers a player on entry and pushes outward at 4 m/s. No damage over time; there is no health to damage. Under more than 1 m of water: half impulse and no fire patch (05 §6.4).

| Distance | Impulse | Player result | Object result |
|---|---|---|---|
| 0 m | 1,200 N·s | Stall | a 45 kg cylinder leaves at 27 m/s and triggers |
| 1 m | 800 N·s | flip | a 180 kg drum leaves at 4.4 m/s |
| 2 m | 400 N·s | stagger | |
| 3 m | 0 | enters fire patch: stagger, pushed out at 4 m/s | |
| 4 m | | nothing | |

The drum's own impulse by speed: 2 m/s (walked into, rolled), 360 N·s, stagger; 6.7 m/s, 1,200 N·s, Stall, no trigger; 11.7 m/s (7 m drop), 2,106 N·s, Stall and trigger; 25 m/s (Haul cap), 4,500 N·s; 30 m/s (Jack cap), 5,400 N·s. Thresholds: 300 N·s at 1.7 m/s, 600 at 3.3 m/s, 1,200 at 6.7 m/s. Crush: 1,766 N of weight, under 8 kN. A drum pressed onto a pinned player by a Breaker's 1,000 N Haul is 2.8 kN, still under; the drum crushes only with another load on top of it.

## Interactions

Haul to it: the player is pulled to the drum and the drum comes to them at T over 180 kg. A Climber (4.4 m/s² on 90 kg, over their 3 m/s² foot-hold) leaves their feet and meets the drum in the middle; a Breaker (4.5 m/s², under 5) skids and the drum does most of the moving. Jack from it (self-dog to drum): the Kick gives the player the larger share of V and the drum rolls off slowly. Jack from the drum to an enemy's Catch Plate: no Kick (05 §3.2), a T push only; the drum accelerates at T over 180 kg from rest (2.2 m/s² on a Scrapper line, 5.6 on a Breaker's) and the enemy skids at T over their mass. The drum reaches its 6.7 m/s Stall speed after 10 m on a Scrapper's push and 4 m on a Breaker's, so the T-push Jack is slower than a Haul but still lethal at the line's end. Hang from it: break-free is tested against active pull only (05 §6.2), so a cradled or strapped drum holds a Pendulum up to the class Hang load without leaving its placement, and a hanging player climbs to it by Reel. A loose drum comes along as 180 kg of dead weight; Reeled up to an anchor it is a Guillotine that needs 4 m of fall and is released by cut or recall.

Object to object: the Kiss of a drum and a cylinder closes at up to 25 m/s. The cylinder triggers at contact (over 8 m/s); its blast triggers the drum. Two drums Kissed on a Breaker line close at 15 m/s each way, 30 m/s relative, over the 10 m/s trigger, and both blow. A drum rolling into a stack at 2 m/s does nothing to the stack.

Kinematics deliver full relative impact. A Hauler sends a drum ahead of it at the Hauler's speed, exploding, into whatever is in the trench. The Lift counterweight passing through the Mid Hang detonates any drum hung in its path. A kinematic passing through a line severs it (05 §3.5): a Hauler crossing the Hang line that holds a drum over its trench snaps the line and the drum drops into the trench ahead of it, which is the same result by a different route. Lines seated in the kinematic itself are not severed.

Water: a full drum floats with 8 cm of freeboard (**PROPOSED**; 200 L displacement against 180 kg). Empty it floats on its side. In the Hold the water shove on the Breathing moves floating drums toward the nose at the water's speed, and a floating drum arriving at 2 m/s against a player in water is 360 N·s against a ×3.0 threshold for stagger, flip, and Stall: nothing. A drum that blows in water over 1 m deep delivers 600 N·s at center and no fire. On the Breathing an upright drum neither slides nor tips at 6°; on its side it rolls at 0.7 m/s² and reaches the 3.3 m/s flip speed after 8 m. Force volumes act on players and free bodies only, scaled a_ref × min(100 / m, 2) (05 §6.6): a loose 180 kg drum feels 0.56 times the authored acceleration. In the Anchor's Updraft it does not lift (**PROPOSED**: the authored Updraft a_ref is under what 0.56 times would need to beat gravity). A drum hung on a player's Hang line is a free body and swings in the gust; a Guillotine drum sways and its drop point moves with it. A cradled drum is unaffected until freed. The Pulse (0.4 s outward shove, 0.6 s inhale toward the Stump, every 75 s from 0:55) rolls a loose drum on its side 0.56 times as hard as it shoves a player, and a cradled drum does not move.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | As above. | Full behavior. |
| Dented | Top chime crushed, hoop sprung, bung weeping. | Unchanged. After any impact 5 to 10 m/s. |
| Armed | Bung blows, fuel sheet out of the vent, hiss. 0.3 s. | Committed. |
| Blown | Lid gone, body split and peeled (Light fragment, 8 kg, 10 s), fire patch 3 m. | Blast field once. Fire volume 6 s. |
| Destroyed | Scorch decal 4 m across, pooled residue decal. | Nothing. |

## VFX and SFX hooks

- Impact: deep steel boom with liquid slosh on steel and concrete, thud on wood, heavy splash in water. Empty variant rings hollow.
- Rolling: a continuous rumble with hoop clicks, pitch by speed.
- Break-free from a cradle: strap snap, `NS_Lurch_Dust_Small`.
- Armed: `NS_Vent_Fuel` (vapor shimmer and a fuel sheet from the bung), `SFX_Vol_Hiss_Drum`, lower than the cylinder's hiss.
- Blast: `NS_Blast_Drum` (black-orange fireball 0.2 s, 3 m pressure ring, lid tumbling). `SFX_Vol_Blast_Drum`. Underwater: `NS_Blast_Drum_Sub`, foam dome, no fireball.
- Fire: `NS_Fire_Patch_Drum` (3 m pool fire, 6 s, heat shimmer, black smoke column to 8 m). Emissive from the fire only, 1.5 units, below a singing line at full load.

## Technical

LOD0 5,000 tris, LOD1 2,500, LOD2 1,000, LOD3 250. One 2K set plus the stencil decal atlas. Pivot at the bottom center.

Dog sockets:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Top_01` | Top, center | Metal |
| `SKT_Dog_Valve_01` | Top, on the bung | Metal |
| `SKT_Dog_SideN_01` | Body between hoops, X+ | Metal |
| `SKT_Dog_SideS_01` | Body between hoops, X- | Metal |
| `SKT_Dog_Rim_01` | Top chime | Metal |
| `SKT_Dog_Bot_01` | Bottom center | Metal |

Mesh swap on Blown, not a geometry collection. Blast field `FS_Blast_Drum`: Chaos Radial Vector, radius 300 uu, 1,200 N·s, linear falloff, one shot, server event; halved and fire suppressed when the origin is under more than 1 m of water. Fire patch is a force volume actor `FV_Fire_Drum`: radial push 4 m/s, stagger on entry, 6 s lifetime. Racked drums sit in `FS_Anchor_DrumRack` (200 N) or `FS_Anchor_Pallet` (800 N). Tier recomputes from current mass for the empty and half variants. Replication RC-M, 20 Hz. The fire volume is server-spawned and its lifetime is replicated by start time.

## Reference search terms

200 litre steel drum closed head; diesel drum rusted red; oil drum rolling hoops wear; 55 gallon drum dented; drum cradle rack; fuel drum stencil UN flammable; shipyard fuel storage pallet strapped drums.
