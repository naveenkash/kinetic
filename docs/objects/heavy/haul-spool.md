# Haul spool

Heavy. Heavy tier, 2,000 kg: a steel cable spool wound with recovered Meridian Weave ribbon. 160 cm flange diameter, 120 cm wide, 60 cm core, with 10 cm axle stubs protruding 15 cm each side. The payload of Haul mode. Appears in the Yard, the Floor (the authored Haul route runs on rollers between the trenches from the west gate to each crew's truck) and Ballast, the Spine (the spool rides a carriage on a counterbalanced ratchet rail from the Hold dock to the Crown dock). Not placed in Hangtown; 03 recommends against it. Not placed on the Anchor.

## Role in play

Two tonnes of the thing everyone is fighting for. Haul mode is a payload mode where the payload is pure physics: no capture zone, no push bar. The spool moves only under lines, gravity, and whatever hits it. Per 05 §6.1a, "The Haul spool sits on rollers (0.02) and weighs 2,000 kg: 400 N to start, so any class can move it alone, but a crew with three lines adding as vectors (6.2a) moves it three times faster, which is the point of the mode." Lines never lift the spool; in Ballast a counterweight does the lifting and the lines beat a pawl. Per 05 §6.2a and §10, pull forces from several lines add as vectors and Hang loads add across lines; this is the only way to hang a 2,000 kg Haul spool (19.6 kN) and the reason Haul mode is a crew mode.

Why it is there: the Yard presses hull sections into Weave billets and spools recovered ribbon for shipping. The spool is the shift's output. The Guild runs the Yard and marks the spool; the two trucks at the Floor's ends belong to the contesting crews. In Ballast, HTR's cutters installed the Spine rail to bring cargo up out of the Hold one carriage at a time.

| Harness | What they do with it |
|---|---|
| Scrapper | 400 N starts it on the Yard rollers (392 N start force) and rolls it at 0.15 m/s² once moving (0.005 rolling, 05 §6.1a). On the Spine rail, 0.05 m/s². Chocks it with a crate when the enemy pulls. |
| Rigger | 0.25 m/s² on rollers, 0.10 on the rail. Hangs it to a trench grate clear of the Hauler's path so an enemy cannot roll it off the route (a Hauler passing through that line would sever it); a Rigger's 12 kN Hang load holds most of its 19.6 kN weight with one more line. Clotheslines the route behind it. |
| Breaker | 0.45 m/s² on rollers (4.2 m/s at the end of a 20 m line, 8,400 N·s), 0.20 on the rail. Braces at the truck and Hauls the spool home. Lines it to an enemy and rolls it over them. |
| Climber | Jacks off the flange to leapfrog it: the Kick, V 18 split against 2,000 kg, is 17.2 m/s for the Climber. On the Spine, perches above the carriage and Hauls from a ledge the others cannot reach. |
| Doubler | One line on the spool, one on the enemy, Hang. The enemy is now in front of a rolling 2,000 kg. At overspin, 1,200 N is 0.55 m/s² on rollers, 0.25 on the rail. |

A crew of three pulling together (Breaker 1,000 + Rigger 600 + Scrapper 400 = 2,000 N, adding as vectors) gives 0.95 m/s² on the Yard rollers: 6.2 m/s after 20 m, and the spool coasts 380 m on its own while the lines re-fire. On a 120 m route (**PROPOSED** Yard route length) an uninterrupted crew delivers in under a minute of pulling. On the Spine rail the same crew makes about 0.45 m/s².

## Appearance

Silhouette: a wide cable reel, two thick round flanges and a dark wound core. The ribbon on the core is wound flat, so the core reads as a smooth black cylinder with a faint spiral seam. Axle stubs make it readable as a thing that rolls. In Ballast it sits in a steel carriage with the axle stubs in bearing blocks and a toothed rail behind it.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Flanges (dished steel) | `MI_Steel_Painted_Worn`, Guild ochre `#C8912A` | | 0.55 to 0.75 | 0 / 1 chipped |
| Flange rims, tread | `MI_Steel_Bare` | `#5C5F63` | 0.50 | 1 |
| Core, axle | `MI_Steel_Bare` | | 0.55 | 1 |
| Wound ribbon | `MI_Weave` | `#1A2430`, iridescent `#3A6E7F` | 0.18 to 0.28 | 1, anisotropy 0.8 around the core |
| Keeper straps (2) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.4 | 1 |
| Rust | `MI_Steel_Rusted` | `#7A3F1C` | 0.85 | 0.3 |
| Yard rollers (set dressing) | `MI_Steel_Bare`, greased `MI_Decal_Grime` | | 0.3 | 1 |
| Spine carriage and rail (**PROPOSED** visuals) | `MI_HTR_Grey_Matte` carriage, `MI_Steel_Bare` rail teeth, `MI_HTR_Orange_PowderCoat` pawl lever | | 0.5 | 1 on teeth |

Wear: ochre worn through on the flange rims to bright steel in a 15 cm tread band. Rust in the flange dish where water pools. Hand polish on the axle stubs where Dogs seat. The ribbon does not wear. The rail teeth are bright on their upper faces where the pawl rides.

Markings: the Guild hitch-knot glyph in `MI_Guild_Black` `#1C1B19` on each flange, 60 cm across, with the Ledger lot `LG-4471` and `WEAVE / 1,960 KG NET` in stencil. On the rim, repeating: `KEEP CLEAR / ROLLS`. A white Office tag `OFFICE OF THE LAY / CLAIM PAYLOAD` on the keeper strap. On the Yard variant a LayCast plate on the camera-side flange: `KESSL HARNESS LEASING`. The Spine carriage carries `HTR-RC-0450` and a stencilled `COUNTERBALANCED / DO NOT RIDE` in orange. No HTR marks on the spool itself, ever; HTR wants it and does not own it.

Under arena lighting the flanges are a warm ochre disc and the core is a black band with a moving specular highlight as it rolls. No emissive. In the Yard it carries slag and rail grease on the tread; in Ballast's Hold it starts green-black to the seep line.

## Variants

| Variant | Difference |
|---|---|
| Yard (default) | Ochre flanges, Guild glyph. Chocked at the west gate in a cradle (200 N) on a roller track. |
| Ballast | Same spool, rust 0.7, Meridian cargo stencil under the Guild glyph. Seated in the Spine carriage at the Hold dock; axle stubs in bearing blocks. |
| Empty reel | 400 kg, Heavy. Bare core. Not used in Haul mode; Yard set dressing on the Stacks, on a bare floor, so fixed scenery per 05 §6.1a. |
| Dented | Flange rim folded 10 cm on one side. Rolls with a thump once per revolution. Cosmetic otherwise. |

## Physics

Heavy tier, 2,000 kg. Collision: two cylinders, radius 80 cm, thickness 6 cm, for the flanges; one cylinder radius 55 cm, length 108 cm, for the wound core (so the tread is the flanges, not the ribbon); two capsules radius 5 cm, length 15 cm, for the axle stubs. Center of mass at the axle center. Restitution 0.05. On the Yard route it rides a roller track (05 §6.1a: rollers are 0.02 to start and 0.005 rolling): start force 0.02 × 19.6 kN = 392 N, so every class starts it; once rolling, resistance is 0.005, 98 N, 0.05 m/s² of coasting deceleration. Off the roller track, on a bare floor, it is fixed scenery per 05 §6.1a: no line moves it.

Break-free: in its start cradle, 200 N (anyone). On the rollers, the 392 N start force. In the Spine carriage, the 200 N pawl drag.

Yard rollers, net acceleration (T − 98 N) / 2,000 kg after the start, other end static or braced:

| Puller | T | Net acceleration | Speed at line end | Impulse into a player |
|---|---|---|---|---|
| Climber | 400 N | 0.15 m/s² | 3.9 m/s at 50 m | 7,800 N·s, Stall |
| Scrapper | 400 N | 0.15 m/s² | 2.4 m/s at 20 m | 4,800 N·s |
| Rigger | 600 N | 0.25 m/s² | 4.5 m/s at 40 m | 9,000 N·s |
| Doubler | 800 N | 0.35 m/s² | 4.6 m/s at 30 m | 9,200 N·s |
| Doubler overspin | 1,200 N | 0.55 m/s² | 1.0 m/s at the 1.8 s snap | 2,000 N·s |
| Breaker | 1,000 N | 0.45 m/s² | 4.2 m/s at 20 m | 8,400 N·s |
| Crew, 2,000 N (05 §6.2a) | | 0.95 m/s² | 6.2 m/s after 20 m | 12,400 N·s |

All under 1 g. The 25 m/s Haul cap is never reached on the flat.

Ballast Spine rail (05 §6.1a canon: 200 N pawl drag, 4,000 kg moving mass, any class moves it, lines never lift the spool). The spool sits in a carriage on a vertical ratchet rail; a counterweight on the far side of the Spine balances the spool's 19.6 kN, so the moving mass is the spool plus the counterbalance carriage, 4,000 kg, and the line only has to beat the pawl's 200 N drag. Acceleration is (T − 200 N) / 4,000 kg:

| Puller | T | Acceleration up the rail | Climb at line end |
|---|---|---|---|
| Climber | 400 N | 0.05 m/s² | 2.2 m/s at 50 m |
| Scrapper | 400 N | 0.05 m/s² | 1.4 m/s at 20 m |
| Rigger | 600 N | 0.10 m/s² | 2.8 m/s at 40 m |
| Doubler | 800 N | 0.15 m/s² | 3.0 m/s at 30 m |
| Breaker | 1,000 N | 0.20 m/s² | 2.8 m/s at 20 m |
| Crew, forces adding as vectors | up to about 2,000 N | about 0.45 m/s² | 4.2 m/s after 20 m |

The pawl holds position the instant the line releases or is cut: no run-back. The spool climbs in increments and never falls. The carriage is not modeled as free, so the Breathing's 6° tilt neither helps nor hinders the climb; the pawl holds through the cycle. Rail length Hold dock to Crown dock 120 m (**PROPOSED**, matching the Yard route). Rail pitch 10 cm per tooth, carriage 200 × 140 × 60 cm, pawl lever 40 cm (**PROPOSED** visuals).

Hanging it: 19.6 kN of weight against Hang loads of 4 (Climber, Scrapper), 12 (Rigger), 8 (Doubler), 10 kN (Breaker). No single line holds it off the ground; a single line slips at 0.5 m per 0.1 s with a creak. Hang loads add across lines (05 §6.2a, §10): Rigger plus Breaker is 22 kN and holds; two Riggers (24 kN) hold; Rigger plus Doubler (20 kN) holds; Breaker plus Doubler (18 kN) slips. In the Spine the counterweight carries the load and no Hang is needed.

As a projectile: 2,000 kg. 300 N·s at 0.15 m/s, 600 at 0.3 m/s, 1,200 at 0.6 m/s. Rolling at a walk, 2 m/s, it is 4,000 N·s. At 6.2 m/s off a crew pull, 12,400 N·s. Rolled into a Yard trench, a 4 m drop at 8.9 m/s, is 17,700 N·s to anyone below, well under the 50 m/s terminal velocity. A full-overspin snap delivers 1,500 N·s, 0.75 m/s, enough to Stall a player pinned in front of it. Cut recoil is 150 N·s per 1,000 N, nothing on 2,000 kg. Crush: 19.6 kN of weight, over the 8 kN threshold that no line reaches and that comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1). Pinned under a flange for 0.5 s is a Stall. Pinned between the rolling spool and a trench wall is a crush before the spool stops.

Not destructible. A demolition charge at 0 m gives it 1.25 m/s on the rollers, enough to Stall; on the rail the pawl holds.

## Interactions

Haul to it: the player is pulled to the spool and the spool rolls toward the player at under 0.5 m/s². On flat ground the player arrives first and had better step aside, because the spool keeps coming and coasts. Jack from it: a Slingshot off two tonnes; the Kick gives the player nearly all of V and the spool rolls back a few centimeters. Hang from it: a 2,000 kg Pendulum anchor on the flat that does not start rolling under a Hang, because break-free is tested against active pull only (05 §6.2), and a cradled spool stays chocked under a hanging player. If the spool itself is hanging it is a 19.6 kN load, which breaks any single Hang load and holds on two.

On the Spine rail: Dogs seat in the spool as usual. A Haul from above (a self-dog from a Spine ledge, or a Dog in the shaft wall above the carriage) pulls the carriage up the rail at the table rates; a Haul from below pulls the player up to the carriage, because the pawl does not run back. A player Hung to the carriage climbs to it by Reel (holding Haul within Hang shortens the rod at 1.0 m/s, bearing up to the class Hang load), which is how a crew re-stacks on the carriage between pulls. A Jack from the spool to an enemy's Catch Plate does not Kick (05 §3.2); it is a T push only, and on the rollers T still clears the 392 N start force on every class line. Jacking off the carriage is a Slingshot off 4,000 kg. Enemy counterplay is on the pullers, not the payload: cut their lines (the pawl holds what was gained), Jack them off their ledges, or Clothesline the Spine ladders. The pawl is one-way; nothing in the match lowers the carriage (**PROPOSED**: the HTR release lever is locked out for the match and stencilled so).

Object to object: Medium props in its path on the Floor are rolled over or pushed; a 250 kg crate chocked against the tread stops it under 1 m/s and is shoved above that. Heavy props stop it. Two spools do not exist in one match.

Kinematics: a Hauler meeting the spool in a trench delivers full relative impact. A Hauler at 8 m/s sends the spool off at 8 m/s, 16,000 N·s to whatever is ahead, and the spool does not derail the Hauler. This is the Yard's Haul-mode disaster and it is intended. A kinematic also severs any line it passes through (05 §3.5): a Hauler crossing a Hang line that chocks the spool to a grate, or a crew's pull line strung across its trench, snaps it and both Dogs release; lines seated in the Hauler itself are not severed. The Gantry magnet does not pick the spool (**PROPOSED**: the Gantry picks hull sections only). In Ballast the Breathing hull carries the rail, the carriage, and the counterweight together; the relative motion is zero and the pawl holds.

Water: sinks; 2,000 kg in a 2.4 m³ envelope. At the Hold dock it stands in 40 cm of seep water in its carriage. Updraft: not present. Breathing tilt: covered above.

## States

| State | Visual | Physics |
|---|---|---|
| Intact, cradled (Yard) | Chocked at the start point on the roller track, keeper straps on. | 200 N break-free, then the 392 N start force. |
| Rolling (Yard) | Straps trailing, tread bright, ribbon highlight sweeping, rollers spinning under it. | Full. Coasts at 0.05 m/s² deceleration. |
| Docked (Ballast) | In the carriage at the Hold dock, pawl engaged, counterweight at the top of its run. | 200 N pawl drag. |
| Climbing (Ballast) | Carriage rising tooth by tooth, pawl clicking, counterweight descending the far wall. | (T − 200 N) / 4,000 kg. Holds on release. |
| Dented | Flange rim folded, periodic thump. After any impact over 4,000 N·s. | Post-start rolling resistance doubles to 0.01 (**PROPOSED**). |
| Delivered | Inside a crew's truck volume, or locked at the Crown dock. Flanges glow 0.3 units in the crew color for 2 s. | Round ends. |
| Down the Well | In the press pit. | Round ends for the hauling crew; Haul scores by furthest point reached. On the Spine rail this cannot happen. |

No Broken or Destroyed state.

## VFX and SFX hooks

- Rolling: a continuous heavy rumble, pitch by speed, with a flange ring once per revolution over 3 m/s and the roller track's chatter beneath it. In water, a bow slosh.
- Impact: a flat two-tonne thud that shakes the camera within 10 m. Against a trench wall, a concrete crack decal.
- Break-free from cradle: strap snap, `NS_Lurch_Dust_Medium`, 0.2 s before motion. Start on the rollers: a greased squeal, 0.2 s.
- Spine climb: `SFX_Spool_Pawl_Click` per tooth (10 per meter), `SFX_Spool_Counterweight_Rumble` from the far wall, `NS_Spool_Rail_Sparks` at the pawl. On release: a single heavy clack as the pawl seats, the tell that the gain is kept.
- Hang slip: the creak from 05 §11, line jitter, `NS_Spool_Slip_Sparks` at the Dog. When a second line takes the load the creak stops; that silence is the crew's tell.
- Delivered: `NS_Spool_Delivered` (crew-color ring sweep on the flanges), the Marshal's call.
- No emissive except the 2 s delivered glow.

## Technical

LOD0 16,000 tris, LOD1 8,000, LOD2 3,000, LOD3 600. Two 2K sets (flanges and axle; core and ribbon) plus `T_Trim_Steel_01`. The Spine carriage and rail are separate meshes on the Heavy budget with their own 2K set. Nanite on. Pivot at the axle center, per the rolling-object convention. X+ along the axle.

Dog sockets:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Hook_01` | Axle stub, X+ | Metal |
| `SKT_Dog_Hook_02` | Axle stub, X- | Metal |
| `SKT_Dog_Top_01` | Core, top of the ribbon | Metal (Weave counts as metal for Mag Dogs, **PROPOSED**) |
| `SKT_Dog_SideN_01` | Flange face, X+, at 60 cm radius | Metal |
| `SKT_Dog_SideS_01` | Flange face, X-, at 60 cm radius | Metal |
| `SKT_Dog_Rim_01` | Flange rim, X+ | Metal |
| `SKT_Dog_Rim_02` | Flange rim, X- | Metal |
| `SKT_Dog_Frame_01` | Keeper strap buckle | Metal |

A Dog on a flange face at 60 cm radius orbits the axle as the spool rolls; the line wraps visually but the constraint attaches to the socket, so a Haul on a rolling spool pulses once per revolution. This is accepted and reads as the spool fighting the line. Dogs on the axle stubs do not orbit; experienced players use them. Not a geometry collection. Anchor field `FS_Anchor_SpoolCradle`, 200 N. The Yard roller track is a physical material with the 0.02 start and 0.005 rolling coefficients. The Spine carriage is a prismatic constraint on the rail with a one-way ratchet: a 4,000 kg body (spool welded to carriage) with linear drag 200 N along the rail axis and a velocity clamp of zero in the downward direction; the counterweight is a kinematic visual driven by the carriage's rail position. Multiple pull lines are ordinary summed forces in Chaos; Hang-load summing is evaluated per body by adding the rated loads of every Hang constraint on it before the slip check. Truck delivery and the Crown dock are box overlaps checked server-side. Replication RC-H: server-authoritative, 15 Hz, never asleep while the Haul round is live (it is the objective), with angular velocity replicated so the ribbon highlight matches on all clients; the carriage replicates its rail position as a single float.

## Reference search terms

steel cable reel 1600mm flange; large wire rope drum spool wound; cable drum rolling yard; flanged steel reel rusted; carbon fiber tape spool wound flat; drum chocks wedge cradle; ironworker cable reel rigging; roller conveyor heavy coil track; rack and pinion construction hoist mast; counterbalanced material lift pawl ratchet.
