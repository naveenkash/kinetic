# BALLAST: Signature Plays

_Arena: Ballast. File purpose: fully specified plays unique to this arena, with positions, class, inputs in 05 section 10 style, timing against the Breathing and the Shear, the number that makes each lethal, and the counter. Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Inventions are marked **PROPOSED**. Zone names match layout.md; t is seconds since "live," with the Breathing's slide window at t 10.5 to 34.5 of every 90 s cycle._

_Jack launches in this file: every Jack from a self-dog is the Kick (05 section 3.2), the 0.3 s capacitor launch to the class speed V of 18 / 14 / 12 / 14 / 10 m/s (Climber / Scrapper / Rigger / Doubler / Breaker), split by inverse mass between the two ends, followed by the motor push at T to class line length. Where an input below says Jack from a self-dog to a wall or object, read "Jack (the Kick at V)". The Kick never fires when either Dog is in another player's Catch Plate (the interlock): a Jack against a player is the motor push at T, ramped over 0.3 s, which is what keeps Overboard inside the panic window._


## 1. Avalanche on the Lean

- **Where.** Either Lower Bay. Puller on the cross-catwalk at the dorsal side of the Belly rack (Y -4). Target on the Belly strip of the Lower deck (Y +5 to +11) or crossing toward the gutter.
- **Class.** Rigger, Doubler or Breaker at any time (500 N stack break-free). Climber or Scrapper only in the slide window (**PROPOSED** 250 N at hull pitch 74 degrees or more).
- **Inputs.** 1. Dog A the bottom container of the Belly rack's near stack, low on its Belly face. 2. Self-dog B and brace (Rigger, Breaker), or Dog B the dorsal rack frame (everyone else). 3. Haul.
- **Timing.** Fire at t 11 to 30. The Haul ramps over 0.4 s, the stack lurches 0.2 s after it passes break-free, and the bottom container leaves the end-stop; the two above follow. The stack sits on the rack's dead rollers (friction 0.02), so once the end-stop lets go gravity does the rest: 3.3 to 4.3 m/s² down the rack, 6.8 m/s at the rack's foot, then an 8 m skid on bare grating that carries it across the 5 m Belly strip and into the gutter. The line only has to start it.
- **Why it works.** A 2,000 kg container at 0.6 m/s is 1,200 N·s. These arrive at 6 m/s.
- **Counter.** Hear the rack dogs tick and leave the Belly strip during the slide window. Cut the Haul line in the 0.6 s between Dog seat and lurch. A braced Breaker on the strip is not safe: brace breaks on any object impact over 300 N·s.

## 2. Clear the Mouth

- **Where.** The Spine. Breaker on the Z 34 starboard dog ledge. Target: anyone on the Mouth Plate, including the crew holding the Claim marker.
- **Class.** Breaker alone, or a crew whose lines on the piston sum to 1,000 N along its axis (two Riggers and anyone; a Rigger and a Doubler), all pulling from the same side of the shaft.
- **Inputs.** 1. Dog A the Z 42 port piston's face (11.3 m away, inside the 20 m line). 2. Self-dog B. 3. Brace. 4. Haul.
- **Timing.** 1,000 N meets the rated threshold. 1.0 s structural groan, then the piston slides 2 m out of its housing and drops. It passes the Breaker 4 m away, hits the Mouth Plate 2.3 s after release at 22 m/s, fractures it (2,500 N·s threshold against 880,000 N·s), and comes to rest at the Spine Foot. Total: 3.3 s from Haul to impact. The marker falls to the Foot; if it lands under the piston's footprint it resets to the fallback position (**PROPOSED**, layout.md).
- **Why it works.** 40,000 kg. Nothing on the Plate survives contact, and the Plate itself is gone for the round.
- **Counter.** The groan is heard across the Bays; 4 m of sprint clears the Plate in under 1 s. A teammate within 1.5 m of the Breaker cuts the line during the groan. Haul a steel crate into the braced Breaker: 300 N·s breaks the brace and the Haul with it. In Claim, the crew that expects this holds the marker from the cross-catwalk's bay ends, not the Plate centre.

## 3. The Fray Drop

- **Where.** The Spine. Doubler on the Spine collar's port platform (the Z 26 piston end). Target: anyone on the Mouth Plate or the ladder below Z 34.
- **Class.** Doubler.
- **Inputs.** 1. Dog A the Z 34 starboard piston face. 2. Dog B the port shaft wall at Z 30, 2 m from where you stand (not yourself: at overspin the line's 9.2 m/s² would take you off the collar before the piston moves). 3. Haul. 4. Hold for overspin.
- **Timing.** Tension passes 1,000 N at 0.5 s of overspin. The rated groan starts there and lasts 1.0 s: the piston releases at 1.5 s. The line snaps at 1.8 s. Release between 1.5 and 1.8 s or the snap launches you off the collar at 10 m/s into a 10 m fall. The octave jump at 1.0 s is the cue: release half a second after the pitch jumps.
- **Why it works.** The only solo non-Breaker way to move 40 t, on a 0.3 s window everyone in the shaft can hear.
- **Counter.** The octave jump gives 0.5 s before release and 2.3 s more before the piston reaches the Plate. Anyone on the Plate leaves it. A teammate cuts the Doubler's line before 1.5 s. The Doubler who misses the window is a free kill: staggered 0.8 s and falling.

## 4. The Draw

- **Where.** The Spine, between the Z 26 port piston and the Z 34 starboard piston (an 11.3 m rod, 8 m across and 8 m up). Target: a Climber doing the Ascension from the Mouth Plate.
- **Class.** Rigger (two set lines, 12 s life, stiffer stagger). Any class can set one.
- **Inputs.** 1. Dog A the Z 26 piston face. 2. Dog B the Z 34 piston face. 3. Hang. 4. Release: a set line. 5. Set the second line from the Z 34 piston to the Belly face at Z 30, covering the drift lane.
- **Timing.** Set at t 5 to 15 so both lines live through the slide window, when the Breathing's 6 degree gravity offset drifts an Ascension 3 m toward the Belly face. A Climber passes Z 30 at 13 m/s; a Hang line at over 6 m/s flips them. Flipped in a shaft means falling 14 m to the Plate.
- **Why it works.** 13 m/s against a 6 m/s flip threshold, and 14 m of fall against a 7.3 m Stall fall.
- **Counter.** The set line brightens within 5 m. A Climber can fire and cut mid-air: start the Cutter 5 m out (0.2 s warm-up plus 0.15 s swing covers 4.5 m at 13 m/s). Or launch in the recovery half of the cycle, t 45 to 90, when there is no drift and the dorsal lane is clear.

## 5. Hold Flood

- **Where.** The Hold. Scrapper on the Hold Ledge at X -14 (the welding cart). Target: anyone in the Wash.
- **Class.** Scrapper (any class can do it; the Scrapper's 30% bonus on Medium under 100 kg makes the window shortest).
- **Inputs.** 1. Dog A a propane cylinder racked on the welding cart (200 N break-free). 2. Dog B the starboard water tank at Y -7, 12 m away. 3. Haul.
- **Timing.** 400 N on 45 kg is 8.9 m/s², 11.6 with the bonus. The cylinder crosses 12 m and arrives at 16.7 m/s, over its 8 m/s trigger: blast 1,500 N·s at the contact point, which meets the tank's 1,500 N·s fracture threshold. The rush volume starts 1.0 s after the boom: 12 m/s² on a 100 kg reference body toward the nose for 3 s, so a Climber feels 13.3 m/s², a Scrapper 10.9, a Rigger or Doubler 9.2, a Breaker 5.5. Fire during the lean (t 10 to 30) so the Wash shove adds its 2.2 to 5.3 m/s² to the rush.
- **Why it works.** The rush exceeds every unbraced foot-hold, the Breaker's 5 included, and a braced Rigger's 9 by a hair. A Rigger in the cone is at the Sump in 1.5 s and at the rent at 13 m/s in 2.2 s; a Climber gets there in 1.8 s. The Scrapper on the ledge at Z 6 is above the volume.
- **Counter.** Hang to the dorsal wall or a tank bolt before the rush begins: the rush is a 1,200 N force on any body, under every class Hang load. A braced Breaker (20 m/s²) stands. Stay out of the Wash during the lean.

## 6. Crown Ascension

- **Where.** The Spine, Mouth Plate to the Spine head kerb: 28 m.
- **Class.** Climber (30 m vertical reach). A Doubler reaches 27 m and lands on the Z 42 ledge instead.
- **Inputs.** 1. Self-dog A. 2. Dog B the Mouth Plate grating at your feet, dorsal edge. 3. Jack (the Kick: 18 m/s straight up, then the 400 N push to 50 m). 4. Perch on the kerb at the top (arrival speed 4.6 m/s). 5. Re-dog from the perch.
- **Timing.** Launch at t 45 to 90 or t 0 to 10, when the hull is neutral or recovering. In the slide window the 6 degree offset drifts the flight 3 m toward the overhanging Belly face, and meeting it at 10 m/s or more is a Stall against static. The Draw lives in that same window.
- **Why it works.** 28 m in 2.9 s, past every ladder route, to the tier that holds the drive dogs and the Haul cradles.
- **Counter.** A Breaker on the kerb Hauls the arriving Climber: 1,000 N on 90 kg is 11 m/s², into the kerb or over the Spine head. The Draw. Anyone watching the Plate sees the self-dog and has 2.9 s.

## 7. Shear Caboose

- **Where.** From the Starboard Upper Gallery across the shaft to the Port Upper Gallery (nearest points 8 m apart). Target: anyone on or within a line of the port gallery or the port stair's top flight.
- **Class.** Any. Rigger preferred (12 s set line).
- **Inputs.** 1. Dog A the enemy's plate. 2. Dog B the Port Upper Gallery grating. 3. Hang. 4. Release (the rod persists as a set line).
- **Timing.** Fire at T-3 of the Shear siren. At T-0 the gallery is parented to the falling wall panel and becomes a kinematic on an authored path: the enemy is tethered to something leaving (05 section 8, player to kinematic, Hang: "player rides it"). They have about 2 s before the panel clears the deck edge and their line drags them over it.
- **Why it works.** The panel is infinite mass. Nothing on the far end of that rod matters except the Cutter.
- **Counter.** Cut. The siren is 5 s long and the line sings from the moment it lands, so the only way to be caught is to be looking elsewhere. Or be 25 m from the port gallery when the siren starts, which is where everyone should be.

## 8. Shear Avalanche

- **Where.** Port Lower Bay, the port Belly rack (Y 0 to +6). Target: anyone on the port deck's Belly strip or the port end of the cross-catwalk.
- **Class.** Any (the Hang does the work; the Shear supplies the force).
- **Inputs.** 1. Dog A the enemy's plate. 2. Dog B the port Belly rack's bottom container. 3. Hang.
- **Timing.** Fire at T-3. The port Belly rack's stacks carry a strain flag by default; at T+1.0 the Shear shake releases their bottom containers. On the rack's dead rollers the container is off the end-stop at T+1.2, across the Belly strip at T+2.5 and into the gutter at T+3.0, its west end passing over the open gorge on the way. The enemy follows on the rod into a 16 m drop onto the Wash unless they cut. This is the Anchor Drop with the arena pushing the load.
- **Why it works.** 2,000 kg rolling into a 16 m drop, tethered to 130 kg, on a schedule the Marshal announces. A 16 m fall onto 0.6 m of water over steel is a Stall.
- **Counter.** Cut before T+3.0 (5 s of siren plus 3 s). A Hang line to a resting container reads dim like a set line; look for it when the siren starts. Or stand starboard of the Mouth Plate from T-5 onward.

## 9. Gutter Overboard

- **Where.** Lower Bay Belly strip (Y +5 to +11). Enemy between you and the gutter at Y +11.
- **Class.** Breaker (braced). A Rigger pusher works only against Climbers and Scrappers.
- **Inputs.** 1. Self-dog A. 2. Brace. 3. Dog B the enemy's plate. 4. Jack. No Kick: Dog B is in a Catch Plate, so this is the motor push at T ramped over 0.3 s (05 section 3.2 interlock).
- **Timing.** 1,000 N on a 130 kg Rigger is 7.7 m/s². Their feet leave the grating when the ramp passes their 5 m/s² foot-hold, about 0.2 s in; the braced Breaker takes none of it. Six metres of strip take about 1.4 s including the ramp, and the Rigger leaves the deck at 9.6 m/s, above the 8 m/s ledge-grab limit, into a 16 m fall onto 0.6 m of water over steel (x1.0 at that depth). A Climber goes in 1.2 s at 11.5 m/s. A Breaker victim (4.5 m/s², under their 5 m/s² foot-hold) skids for 1.8 s and leaves at 7.4 m/s, slow enough to grab the edge: against a Breaker, wait for the lean or add a second line. A Rigger pusher's 600 N gets a Rigger to only 7.4 m/s, so a Rigger pushes Climbers (9 m/s in 1.5 s) and Scrappers (8.1 m/s in 1.6 s).
- **Why it works.** 16 m of fall against a 7.3 m Stall fall, an exit speed above the ledge grab, and a 6 m strip that reads as safe because it has a wall on one side. 1.2 to 1.8 s from Dog seat to the edge sits inside the panic window.
- **Counter.** Cut (chest plate: the line is already in view). Hang to the deck grating in front of you. Never stand on the Belly strip with a Breaker in Jack range, which is anywhere in the bay.

## 10. Rack Guillotine

- **Where.** Upper Gallery over the Lower deck. Load: a steel crate from the stair-head pair. Anchor: the dorsal rack frame top at Z 32. Target: anyone walking the Lower deck under the dorsal rack's shadow.
- **Class.** Rigger (the 12 s set line makes it practical). Any class can set it with 8 s.
- **Inputs.** 1. Dog A the crate. 2. Dog B the rack frame top. 3. Hang (the crate is about 100 kg, 980 N, within every class Hang load). 4. Shove the crate off the gallery edge; it swings out on the 4 m rod and hangs at Z 28 over the Lower deck. 5. Release: a set line. 6. When the enemy walks under it, recall your own set line (or cut it): gravity drops the crate (05 section 10, v0.2.2). A second line from the crate to the enemy in Haul only steers it.
- **Timing.** 12 m of free fall: 1.56 s from recall to impact, arriving at 15.3 m/s. Set it during the recovery half of the Breathing; on the lean the hanging crate rests 0.4 m toward the Belly and swings, which widens the drop zone and makes the timing worse.
- **Why it works.** 100 kg at 15.3 m/s is 1,530 N·s against a 1,200 N·s Stall, and recall is instant (a Scrapper's) or 0.6 s (a Rigger's), with no line on the victim to cut.
- **Counter.** The hanging crate is lit by the gallery flood and the set line brightens within 5 m: look up on the Lower deck and do not walk under it. Cut the set line from the stack top before anyone is under it. Once it is falling, 1.56 s to step 1 m sideways.

## 11. Drop the Gantry

- **Where.** Crown Deck. Breaker at the port Belly column foot (-16, +5, 44). Target: anyone in the 4 m lane under the gantry beam (Y -8 to -4), which is the lane between the Spine head and the Haul cradles.
- **Class.** Breaker alone, or a crew whose lines on the column sum to 1,000 N along one axis (two Riggers and anyone; a Rigger and a Doubler).
- **Inputs.** 1. Dog A the column at its foot plate. 2. Self-dog B. 3. Brace. 4. Haul.
- **Timing.** 1.0 s rated groan with sparks at the column foot, then the beam's port end drops 8 m and hits the deck 1.3 s later at 12.5 m/s. The beam then lies across the deck as a 1.2 m barrier for the rest of the round. 2.3 s from Haul to impact. The best moment is Haul mode's last 6 m, when a crew is dragging the spool across that lane toward a cradle.
- **Why it works.** 3,000 kg across 36 m of deck. Everyone under it is crushed (8 kN for 0.5 s) or struck past 1,200 N·s.
- **Counter.** The groan and the sparks. The lane is taped yellow-black on the deck; step 2 m out of it. After the drop the beam is cover, and a Haul crew can line the spool over it with a Hang from the far cradle.
