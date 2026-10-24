# Scrapped body

Prop state of an eliminated player. **Medium tier for every class; mass is the class mass**: Climber 90 kg, Scrapper 110 kg, Rigger 130 kg, Doubler 130 kg, Breaker 220 kg (05 §7.4: a "non-fracturing prop at the class mass, 90 to 220 kg; tier Medium for all classes"). Every value sits inside the 30 to 300 kg Medium band, so nothing changes tier on a Stall. Dimensions are the player ragdoll: 190 cm tall, 40 cm radius capsule envelope, locked in whatever pose the Stall found it in. Appears wherever a player is scrapped, in every arena and every tier, for exactly 4 s, then a Hook lifts it out over 2 s. Bodies that enter the Anchor's Well are not retrieved by Hooks (05 §7.4). Filed under heavy for production grouping; it is not Heavy tier.

## Role in play

A Medium prop with four Catch Plates that lasts 4 s. Bodies can be used as props during those 4 s. This is in poor taste and riggers do it. The Stall speed of a body is 1,200 N·s over its mass: a scrapped Climber Stalls at 13.3 m/s, a Scrapper's body at 10.9, a Rigger's or Doubler's at 9.2, a Breaker's at 5.5 m/s. Every class line gets every body to its Stall speed inside the line length. Per 05 §7.4, "A scrapped Climber at 90 kg is a Medium object under 100 kg; the Scrapper bonus applies to it. Riggers find this funny." No other body qualifies.

| Harness (puller) | What they do with it |
|---|---|
| Scrapper | 400 N. A Climber body (90 kg): 4.4 m/s², 13.3 m/s over 20 m, 1,200 N·s, exactly a Stall; 17.3 m/s with the bonus, 1,560 N·s. A Breaker body (220 kg): 1.8 m/s², 8.5 m/s over 20 m, 1,870 N·s, Stall. |
| Rigger | 600 N, 40 m line. A Climber body reaches the 25 m/s cap at 47 m, so 23 m/s at 40 m, 2,070 N·s. A Breaker body: 2.7 m/s², 14.7 m/s, 3,240 N·s. Hangs a body across a doorway on a set line: a Clothesline weight that is also a shield. |
| Breaker | 1,000 N, 20 m line. Climber body 11.1 m/s², 21 m/s, 1,890 N·s. Breaker body 4.5 m/s², 13.4 m/s, 2,950 N·s. Or Hauls a body onto a cylinder rack as trigger mass; a Breaker body passes 8 m/s after 7 m. |
| Climber | 400 N, 50 m line. Jacks off a body with the Dog in the gel: the Kick splits V 18 by mass. Against a Breaker body the Climber takes 12.8 m/s and the body 5.2. Against a Climber body, 9 and 9. With the Dog in one of the body's Catch Plates the Kick is suppressed (05 §3.2) and the Jack is a 400 N push only. |
| Doubler | 800 N, 30 m line. Climber body 8.9 m/s², 23 m/s, 2,070 N·s. Cannot use the Hook as a Gallows: lines on a body release at the Stall and the Hook lifts a bare body (05 §7.4), so an enemy Hung to the body is cut loose when the Hook engages. |

## Appearance

Silhouette: the harness weight class's own silhouette, frozen. A scrapped Heavy (the Breaker) is still the biggest shape in the room; a scrapped Light (the Climber) is still thin; the three Standard harnesses (Scrapper, Rigger, Doubler) read as Standard. The pose is the Stall pose: mid-fall, mid-drag, mid-sprint. Nothing about it ragdolls after the first 0.2 s.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Harness plates, livery | The player's own materials, unchanged | | | |
| Gel flood | `MI_StallGel` | `#3FE0D0` | 0.10 to 0.20 | 0, emissive 2.0, subsurface 0.6 |
| Gel seams | `MI_StallGel` at emissive 1.0, filling every plate gap and the Spool housing | | | |
| Catch Plates | The player's plates; the gel does not cover them | | | |

The gel floods from the Harness over 0.2 s, in sheets that pour out of every plate gap, then set. Set gel is translucent, holds the pose, and shows the livery underneath. Cyan is the only emissive on the body and it fades from 2.0 to 0.5 over the 4 s so that by the time a Hook arrives the body is dim. A singing line stays brighter at every point in the sequence.

Markings: none added. The player's Tallies, Rope Mark tape, and reserve livery remain legible. The gel pools 20 cm around the body in a cyan puddle decal (`MI_Decal_Grime` tinted `#3FE0D0`, emissive 0.3) that persists for the round.

Under arena lighting: a cyan-outlined figure. In a darkened sector it is the second-brightest thing after lines, which is accepted; the body is a tell of where the fight was.

## Variants

| Variant | Difference |
|---|---|
| By class (5) | Silhouette, plate layout, livery, and mass: 90, 110, 130, 130, 220 kg. All Medium. |
| Down the Well | No body. The Scrap Card reads "down the Well." Hooks spin up 0.5 s before the ring-out volume. |
| Anchor Well | No body, no Hook, per 05 §7.4. The Office does not discuss the retrieval. The sound is different. |
| Red match (lore only) | Never shipped. No gel, no Hook. The Guild's Ledger number. |

## Physics

Medium tier, class mass. Collision: the player's physics asset, with the per-bone bodies welded into their Stall pose (**PROPOSED**: a single compound body of the ragdoll's capsules, so Dogs seat on the correct plate and feet catch on ledges). Center of mass at the pelvis. Friction 0.6 on anything (gel is tacky). Restitution 0.0.

"Kinematic-ish" in engine terms, **PROPOSED**: a simulated Chaos body with linear damping 0.5 and angular damping 20, so it slides when pushed and does not tumble. It does not sleep. It has infinite mass against the Hook and normal mass against everything else. Lines act on it as on any Medium of its mass. Gravity applies, with the 50 m/s hard velocity cap: a Breaker body landing from 7 m at 11.7 m/s is 2,574 N·s to whoever is under it; a Climber body from the same height is 1,053 N·s, a flip.

Break-free: 0 N. Non-fracturing: no fracture threshold, no fragments. The body does count as an impacting object against other objects' fracture thresholds (05 §6.5): a Rigger body Hauled into a market stall at 2.5 m/s (325 N·s) breaks the stall.

Haul acceleration (T over body mass), other end static or braced:

| Body | Mass | 400 N | 600 N | 800 N | 1,000 N | 1,200 N | Stall speed |
|---|---|---|---|---|---|---|---|
| Climber | 90 kg | 4.4 m/s² | 6.7 | 8.9 | 11.1 | 13.3 | 13.3 m/s |
| Scrapper | 110 kg | 3.6 | 5.5 | 7.3 | 9.1 | 10.9 | 10.9 m/s |
| Rigger, Doubler | 130 kg | 3.1 | 4.6 | 6.2 | 7.7 | 9.2 | 9.2 m/s |
| Breaker | 220 kg | 1.8 | 2.7 | 3.6 | 4.5 | 5.5 | 5.5 m/s |

Speed over distance is the square root of 2 a d, capped at 25 m/s on a Haul. Arrival speeds at each class line length are in the harness table above. Jack: the Kick splits V by inverse mass between puller and body, then T pushes. A Scrapper (110 kg) Jacking a Breaker body (220 kg): the Kick gives the body 4.7 m/s and the Scrapper 9.3; a Breaker Jacking a Climber body: the body 7.1 m/s, the Breaker 2.9. Snap: a full-overspin snap delivers 1,500 N·s: 16.7 m/s on a Climber body, 6.8 m/s on a Breaker body, a Stall on whoever is in front either way. Cut recoil: 150 N·s per 1,000 N, 0.7 to 1.7 m/s depending on the body.

Impulse thresholds by body: a Climber body reaches 300 N·s at 3.3 m/s, 600 at 6.7, 1,200 at 13.3; a Breaker body at 1.4, 2.7, and 5.5 m/s. Walked into at 2 m/s: 180 N·s (Climber body, nothing) to 440 N·s (Breaker body, stagger). Crush: a Breaker body weighs 2,158 N, under the 8 kN threshold, which no line reaches and which comes from kinematics and from stacked or falling weight over about 800 kg (05 §7.1). A body crushes only as part of a larger falling mass.

Stall sequence per 05 §7.4: threshold crossed; the Harness locks the ragdoll in its current pose with gel VFX in 0.2 s; the Scrap Card shows to killer and victim and the Marshal calls it; the body persists 4 s; a Hook arrives and lifts it out over 2 s. Lines on the body release at the Stall (step 4), and the Hook lifts a bare body.

## Interactions

Haul to it: the player is pulled to the body and the body comes to them; each accelerates at T over its own mass. A live Climber on their own line (4.4 m/s², over their 3 m/s² foot-hold) leaves their feet; a live Breaker (4.5 m/s² on a Breaker line, under 5) skids. A Scrapper Hauling a Breaker body is 3.6 m/s² on themselves (under their 4 m/s² foot-hold) and 1.8 on the body: the Scrapper skids two meters for every one the body moves. Jack from it: with the Dog in the gel, the Kick splits V by mass. The Kick interlock (05 §3.2) never fires the Kick when either Dog is in a Catch Plate, and a scrapped body's plates count: a Dog in a body's plate suppresses the Kick, so a Jack from or to a body's plate is a T push only (a Breaker body pushed at 400 N accelerates at 1.8 m/s² from rest). A Jack from a body to a live enemy's plate is a T push only either way. Hang from it: a loose body (break-free 0) comes along as dead weight; Reeled up to an anchor it is a Guillotine load that lasts 4 s and drops by cut or recall. A Breaker body falling 4 m arrives at 8.9 m/s, 1,960 N·s, a Stall; a Climber body needs 9 m (13.3 m/s) to Stall by impulse.

Dogs seat in the body's four Catch Plates (chest, back, shoulders) exactly as on a live player, and in the gel. Lines attached at the moment of Stall release; new lines seat normally during the 4 s. Per 05 §7.4 the Hook lifts a bare body: every line seated in the body during the 4 s releases when the Hook engages at 4.0 s, so a body cannot be used as a Gallows anchor via the Hook. The Hook is not dog-able (05 §3.1).

Object to object: a body Hauled into a cylinder at over 8 m/s triggers it. A body under a falling crate is a Medium prop under a Medium prop.

Kinematics deliver full relative impact: a body on a Hauler path is carried at the Hauler's speed, which means a body Hauled into a Hauler's path arrives wherever the Hauler goes, at train speed. A kinematic passing through a line severs it (05 §3.5): a Hauler crossing the line that holds a body over its trench drops the body. Bodies ride the Breathing like any Medium at μ 0.6: they do not slide at 6°.

Water: sinks to the floor of the Hold's seep water (gel is denser than water, **PROPOSED**). Force volumes act on players and free bodies only, scaled a_ref × min(100 / m, 2) (05 §6.6): a body is loose (break-free 0), so a Climber body feels 1.1 times the authored acceleration (under the 2× cap), a Standard body 0.9 to 0.8 times, a Breaker body 0.45 times. In the Updraft the Climber body lifts (**PROPOSED**: the authored a_ref beats gravity at 1.1 times) and the others do not. The Pulse (0.4 s out, 0.6 s in toward the Stump, every 75 s from 0:55) slides bodies on the Scaffold, Climber bodies furthest. Breathing tilt: above.

Hook arrival: at 4.0 s a Hook descends from the nearest gallery edge, rotors audible from 3.0 s, engages the Harness's lift lug, and rises for 2 s. During the 2 s the body is kinematic on the Hook's path (RC-K) and nothing moves it. At 6.0 s it despawns above the gallery. Bodies in the Anchor's Well get no Hook; the ring-out volume removes them.

## States

| State | Visual | Physics |
|---|---|---|
| Stalling (0 to 0.2 s) | Gel sheets pour from every seam; pose locks. | Ragdoll freezes into the compound body. Lines release. |
| Scrapped (0.2 to 4.0 s) | Set gel, emissive fading 2.0 to 0.5. Scrap Card on the killer's and victim's HUD. | Medium, class mass, damped. Dogs seat. |
| Lifting (4.0 to 6.0 s) | Hook claw on the lift lug, body rising, gel dripping. | Kinematic on the Hook path. Lines release; the Hook lifts a bare body (05 §7.4). |
| Gone | Cyan puddle decal remains. | Nothing. |

No dented, broken, or destroyed state. Non-fracturing.

## VFX and SFX hooks

- Stall: `NS_Stall_GelFlood` (cyan sheets from seams, 0.2 s, then a set shimmer), `SFX_Stall_Flood`, a pressurized hiss and a lock clack. The Marshal calls the scrap 0.5 s later.
- Impact as a prop: a wet, heavy thud with a plate clank, pitched by class mass; on steel a ring, on wood a boom, in water a slap.
- Drag: a tacky squeal.
- Hook: `SFX_Hook_Rotors_Approach` from 3.0 s (directional, the sound of a ring-out before it happens, reused here), `NS_Hook_Downwash` (dust ring 3 m) on arrival, `SFX_Hook_Engage` clack at 4.0 s, rotors receding to 6.0 s. On the Anchor: no rotors; `SFX_AnchorWell_Silence`, a 2 s low tone.
- Replay: at round end the final scrap replays 4 s from the victim's camera, including the panic window and the gel flood from inside the helmet (a cyan iris wipe, 0.2 s).
- Emissive: gel at 2.0 fading to 0.5 over 4 s; puddle 0.3.

## Technical

No LODs of its own; it uses the player skeletal mesh LODs (LOD0 under 40,000 tris per the character budget, **PROPOSED**) with the gel as an additional material slot and a pose-freeze. Texture sets are the character's plus one shared 1K gel set. Pivot at the pelvis bone at the moment of Stall.

Dog sockets: the player's four Catch Plate sockets, retained with their player names because they are the player's: `Chest`, `Back`, `ShoulderL`, `ShoulderR`. These are the one exception to the object socket convention. Surface class Metal.

Not a geometry collection. No anchor field. Implemented as a server-spawned `ABodyProp` that takes the ragdoll pose, welds the physics asset bodies into one compound at the class mass, applies the damping above, sets the Scrapper-bonus flag when the mass is under 100 kg, and schedules the Hook at 4.0 s (or no Hook when the Stall was an Anchor Well ring-out). The body's capsule impacts are included in other objects' damage evaluation at class mass times relative speed. Replication RC-M during the 4 s (server-authoritative Chaos body at 20 Hz) and RC-K during the 2 s lift. The Scrap Card and Marshal call are reliable RPCs at step 2; the pose is replicated once as a bone transform array at step 1 and never again.

## Reference search terms

resin cast figure translucent cyan; rescue harness lift lug drone winch; frozen pose mannequin exoskeleton; gel encapsulation foam flood safety suit; helicopter rescue hoist strop; motocross body armor plates silhouette.
