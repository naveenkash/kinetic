# Demolition charge

Volatile. Light tier, 12 kg. 40 × 25 × 18 cm, a shaped linear charge in a steel carrier bolted to structure. Placed by prep crews only; never loose at round start. Appears in Ballast, the Bays (on the bay wall that sheds in the Shear) and the Spine (on drive housing panels); the Yard, the Stacks (on the control shack piers) and the Floor (on plank bridge abutments); Hangtown, the Upper Hang (on walkway hangers, never on houses); the Anchor, the Scaffold (on the maintenance clamps of the collapse quadrant) and the Rim (on crane bases).

## Role in play

The authored way to drop a specific wall. The charge is a trigger for a scripted structural break, with the biggest blast in the game around it. It is the only Light prop that can scrap a crew. Per 05 §6.4, Dogs seat in a charge without triggering it: a charge is a thing you move, not a thing you shoot. It triggers on impact over 6 m/s by an object or a player, or on a blast.

Why it is there: a cutting crew places charges where a structure will be dropped next shift. The prep crew leaves them live because the Accord says staged demolition material is site property, and the Office tags every one.

| Harness | What they do with it |
|---|---|
| Scrapper | Hauls a chair into it. 400 N on 6 kg is 67 m/s²; the chair is over 6 m/s within 0.3 m. Anything over 6 m/s sets it off. |
| Rigger | Hangs a load over the charge on a set line (within the 12 kN Rigger Hang load), then cuts or recalls it when an enemy passes: the load drops by gravity onto the charge and triggers it if it lands over 6 m/s, a 1.8 m fall. 600 N cannot pull a bolted charge (800 N). |
| Breaker | Hauls the charge off its mount at 1,000 N against the 800 N break-free. It is Light: 83 m/s², the 25 m/s cap in 3.8 m, and it detonates on arrival at the Breaker's chest. A Breaker Hauls it toward a static anchor instead. |
| Climber | Jacks a cylinder down onto it from the tier above. The Kick gives the cylinder 12 m/s at 0.3 s, over both triggers. |
| Doubler | 800 N pulls a bolted charge free. Or Hangs a nearby prop, overspins, and snaps it into the charge: 1,500 N·s on a 25 kg chain coil is 50 m/s. |

The charge's own structural consequence is per arena: which wall, which catwalk, which clamp. The charge doc owns the blast; the arena doc owns the fall.

## Appearance

Silhouette: a flat rectangular brick with a handle, a conduit stub, and two bolt lugs. Clean and machined. Nothing else in the Lay looks this new.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Carrier body | `MI_HTR_Grey_Matte` | `#4A4D50` | 0.60 to 0.70 | 0 |
| End caps, lugs | `MI_Steel_Bare` | `#5C5F63` | 0.50 | 1 |
| Status window | `MI_Glass_Lamp` | `#E8EEF2` | 0.05 | 0 |
| Arming conduit | `MI_Rubber_Black` | `#1E1F20` | 0.75 | 0 |
| Tape band | `MI_Tape_Rated` | `#F2C230` / `#141414` | 0.55 | 0 |

Wear: 0.1. Scuffs on the handle, nothing else. Boot print on the top face at 0.3 wear on charges placed low.

Markings: `OFFICE OF THE LAY / SITE PROPERTY / LIVE` in white on the top face, 2.5 cm letters, Marshal number, and a date. `HTR` block letters in orange `#E8620C` on the carrier end with tag `HTR-DC-0088` (HTR holds most demolition claims; on the Yard and Hangtown the Guild variant carries `LG-####` in ochre `#C8912A`). A single 5 cm band of yellow-and-black structural tape around the body, because the thing it is bolted to is a rated element.

Status window: a 2 cm emissive dot, dim amber `#D98A1E` at 0.4 units, pulsing at 0.5 Hz. This is the only emissive on any volatile, and it is kept below a dim set line (05 §11) so it never competes with a singing line. It exists so the charge reads as live in a darkened sector.

Per arena: on Ballast it is salt-crusted on the Crown and seep-stained in the Spine. In the Yard it sits behind a cage of rebar. In Hangtown it is wrapped in sisal where a Hanger tried to pull it off. On the Anchor it is dusted pale.

## Variants

| Variant | Difference |
|---|---|
| HTR (default) | Grey carrier, orange tag. |
| Guild | Ochre tag, hand-written date in paint marker. Same physics. |
| Caged (Yard) | A 1 cm rebar cage over the charge. **PROPOSED**: the cage is cosmetic and transmits impact; a 6 m/s hit on the cage triggers the charge. |
| Spent | Carrier split open, dark, window dead. Light prop, 7 kg, no trigger. |

## Physics

Light tier, 12 kg. Collision: one box 40 × 25 × 18 cm. Center of mass at the geometric center. Friction 0.6, restitution 0.1 (**PROPOSED**).

Break-free: bolted to structure, 800 N. A Doubler (800 N), a Breaker (1,000 N), or a Doubler at overspin (1,200 N) can pull it off its mount. A Climber or Scrapper (400 N) or a Rigger (600 N) cannot; their line sings against the mount. A Doubler's split lines at 480 N cannot; at 720 N overspin they cannot either. Pulling it off does not arm it. Once loose it is Light and loose on floor at 0 N.

Trigger: impact over 6 m/s to the charge itself by an object or a player, or a blast within range. Dogs seat without triggering (05 §6.4). Hiss and venting 0.3 s before the blast (the carrier's arming vent).

Haul, on a loose charge, other end static: 400 N is 33 m/s² (Scrapper bonus 30% on a Light prop; the 25 m/s cap is not raised), 600 N is 50 m/s², 800 N is 67, 1,000 N is 83, 1,200 N is 100 m/s². Every class reaches the 25 m/s cap inside 10 m. The charge passes 6 m/s within 0.6 m on a Scrapper line and 0.2 m on a Breaker's. Jack: the Kick gives a 12 kg charge almost all of V (12.6 of a Scrapper's 14 m/s) and T then takes it to the 30 m/s cap within 10 m.

Blast radius 5 m, impulse 2,500 N·s at center falling linearly to 0 at 5 m. Under more than 1 m of water, half impulse (05 §6.4).

| Distance | Impulse | Player result | Object result |
|---|---|---|---|
| 0 m | 2,500 N·s | Stall | 250 kg crate leaves at 10 m/s |
| 1 m | 2,000 N·s | Stall | 180 kg drum at 11 m/s, triggers |
| 2 m | 1,500 N·s | Stall | 45 kg cylinder at 33 m/s, triggers |
| 3 m | 1,000 N·s | flip | |
| 4 m | 500 N·s | stagger | |
| 5 m | 0 | nothing | |

Stall reaches to 2.6 m, flip to 3.8 m, stagger to 4.4 m. The charge's own impulse as a projectile is small:

| Speed | Case | Impulse | Result |
|---|---|---|---|
| 2 m/s | | 24 N·s | nothing |
| 11.7 m/s | 7 m drop | 140 N·s | nothing, but it triggers |
| 25 m/s | Haul cap, any class | 300 N·s | stagger, triggers |
| 30 m/s | Jack cap | 360 N·s | stagger, triggers |
| 50 m/s | terminal velocity, or a snap | 600 N·s | flip, triggers |

Thresholds: 300 N·s at 25 m/s, 600 at 50 m/s, 1,200 at 100 m/s, which it cannot reach (50 m/s terminal, 05 §6.3). A loose charge is a thrown bomb, not a thrown weight. Snap: 1,500 N·s × (F / 1,200 N) on 12 kg at full overspin is 125 m/s, capped at 50 m/s terminal: 600 N·s and a detonation on landing. Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker line (150 N·s) on a loose charge gives it 12.5 m/s; a Doubler line (120 N·s) 10 m/s; a Rigger line (90 N·s) 7.5 m/s; all over the 6 m/s trigger when it lands. A Climber or Scrapper line (60 N·s) gives 5 m/s, under. Cutting a heavy line on a loose charge is a detonation with a 0.5 s fuse. Crush: never.

## Interactions

Haul to it: the player is pulled to the charge. Below 800 N the charge does not move and the line sings against the mount. A Doubler's or Breaker's Haul tears it off and it flies to them at up to 25 m/s, which triggers it at their chest. Jack from it (self-dog to charge): a mounted charge is a static anchor; the Kick gives the player all of V, a Slingshot point, and the push at T (under 800 N for everyone but a Doubler or Breaker) leaves the charge where it is. Jack from a loose charge to an enemy's Catch Plate: no Kick (05 §3.2), a T push only; the 12 kg charge accelerates at T over 12 kg from rest (33 m/s² on a Scrapper line) and passes 6 m/s within 0.6 m, so it still arrives armed. Hang from it: break-free is tested against active pull only (05 §6.2), so a mounted charge is a solid Pendulum anchor up to the class Hang load and hanging from it never unseats it. A loose charge comes along as 12 kg of dead weight.

Object to object: anything Hauled into a mounted charge at over 6 m/s detonates it, and every Light prop on any line is over 6 m/s within a meter. A Kissed cylinder arriving at 25 m/s triggers the cylinder (8 m/s) and the charge (6 m/s) within the same 0.3 s. A Scatter that lands a cylinder on a charge is a 5 m Stall zone plus a wall. A flipped 130 kg Rigger landing on a mounted charge at 5 m/s does not trigger it; at 6 m/s they do, and player bodies count as impacting objects (05 §6.5).

Kinematics: the Breathing hull carries its charges with it; relative impact is zero. A Hauler never passes within 6 m of a mounted charge (level design rule, **PROPOSED**). A kinematic passing through a line severs it (05 §3.5): a Hauler crossing a line that holds a load over a charge drops that load onto the charge.

Water: sinks. A loose charge in the Hold's 40 cm seep water triggers on impact with a full blast; in water over 1 m deep the blast is 1,250 N·s at center and the Stall radius shrinks to 0.3 m, flip to 2.5 m. Force volumes act on players and free bodies only, scaled a_ref × min(100 / m, 2) (05 §6.6): a mounted charge (bolted, 800 N) is unaffected by the Updraft or the Pulse until it is pulled free; a loose 12 kg charge feels 2 times the authored acceleration (the cap), lifts and tumbles in the Updraft, and triggers on landing if it lands over 6 m/s, which is a 1.8 m fall. The Pulse (0.4 s out, 0.6 s in toward the Stump, every 75 s from 0:55) throws a loose charge off the Scaffold or into the Well. Breathing tilt: mounted charges do not care. A loose charge at μ 0.6 stays put at 6°.

Chain reactions: a charge blast triggers every cylinder within 4 m and every drum within 3 m of it, and every other charge within 5 m. Prep crews space charges 6 m apart so that one charge drops one element (**PROPOSED** spacing rule).

## States

| State | Visual | Physics |
|---|---|---|
| Intact, mounted | Amber dot pulsing. Tape band clean. | 800 N break-free. Trigger live. |
| Intact, loose | Bolt lugs sheared, conduit trailing 30 cm. Dot still pulsing. | Light prop, 0 N. Trigger live. |
| Armed | Dot goes solid white, vent hiss from the end cap, 0.3 s. | Committed. |
| Blown | Flash. The mounted element breaks (arena-authored). Carrier replaced by Spent. | Blast field once. |
| Spent | Split carrier, 7 kg Light, dead window. 10 s lifetime if loose; permanent if still bolted. | Inert. |

## VFX and SFX hooks

- Impact (loose carrier): sharp metal clack on steel, knock on wood, flat slap on concrete.
- Break-free from mount: bolt shear crack, `NS_Lurch_Sparks_Small`, 0.2 s before it moves.
- Armed: `NS_Vent_Charge` (grey cordite wisp from the end cap), `SFX_Vol_Arm_Charge`, a rising two-tone whine, audible at 30 m.
- Blast: `NS_Blast_Charge` (white flash 0.05 s, 5 m pressure ring, concrete dust, 3 s dust hang), `SFX_Vol_Blast_Charge` with a 0.4 s structural groan layer that leads into the arena's authored fall sound. Underwater: `NS_Blast_Charge_Sub`.
- Emissive: the status dot, 0.4 units amber, 0.5 Hz pulse; solid white at 1.0 unit for the armed 0.3 s.

## Technical

LOD0 5,000 tris (the lugs and conduit cost), LOD1 2,500, LOD2 1,000, LOD3 250. One 2K set plus the stencil decal atlas. Pivot at the bottom center of the mounting face.

Dog sockets:

| Socket | Zone | Surface |
|---|---|---|
| `SKT_Dog_Top_01` | Top face, center | Metal |
| `SKT_Dog_Valve_01` | Arming vent end cap | Metal |
| `SKT_Dog_Handle_01` | Carry handle | Metal |
| `SKT_Dog_SideN_01` | X+ end cap | Metal |

Not a geometry collection. Mounted charges are children of the structural element's geometry collection and sit in `FS_Anchor_Charge_<Element>` at 800 N. The Dog projectile's seat is excluded from the trigger's impact filter by channel; objects and player bodies are included. Blast is `FS_Blast_Charge`: Chaos Radial Vector field, radius 500 uu, 2,500 N·s, linear falloff, one shot, server event; halved when the origin is under more than 1 m of water. The same server event calls the arena's `BreakElement(<Element>)` which applies a strain field over the mounted element's L0 damage threshold. Replication RC-M while loose; while mounted it is a replicated static child with a replicated `bArmed` flag. The Stall, flip, and stagger results on players are applied by overlap from the server event, not from the physics impulse.

## Reference search terms

linear shaped charge demolition steel column; building implosion charge placement column wrap; demolition charge carrier box; controlled demolition prep RDX linear charge; blasting cap wire conduit steel; structural steel cutting charge bolted.
