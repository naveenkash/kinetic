# KINETIC — Core Mechanics Specification

_Canonical numbers. Every other document defers to this one. Units: meters, seconds, kilograms, newtons. Unreal units are centimeters; multiply lengths by 100 in-engine._

## 1. Design pillars (the test for every feature)

1. **Everything is physics.** No hitscan, no damage numbers, no health. If a thing hurts a player, it is because mass met velocity.
2. **Read before you react.** Every lethal thing telegraphs: a singing line, a groaning structure, a klaxon. A player who dies without a tell is a bug.
3. **Mass is destiny.** Five harnesses, five masses. The outcome of any line is predictable from the two masses on its ends.
4. **Everything was already there.** No player brings a weapon. Players bring a tool and a knife. The arena brings the violence.
5. **The knife is always enough, if you're fast.** Every tether on a player can be cut in time by a player who reacts in time. No unescapable kills.

## 2. The player

### 2.1 Body

| | Value |
|---|---|
| Capsule | 1.9 m tall, 0.4 m radius |
| Camera | First-person by default. Third-person over-shoulder available in casual modes only, locked at 90° FOV. Rated play is first-person. |
| Field of view | 100° horizontal default, 90–110 configurable |
| Eye height | 1.7 m |

### 2.2 Harness classes

| | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|
| Mass (kg) | 90 | 110 | 130 | 130 | 220 |
| Run speed (m/s) | 7.5 | 7.0 | 6.2 | 6.2 | 5.0 |
| Sprint (m/s) | 9.0 | 8.5 | 7.5 | 7.5 | 6.0 |
| Jump height (m) | 1.4 | 1.3 | 1.1 | 1.1 | 0.8 |
| Mantle height (m) | 2.6 | 2.4 | 2.2 | 2.2 | 1.8 |
| Air control | 0.8 | 0.6 | 0.4 | 0.4 | 0.2 |
| Crouch speed (m/s) | 3.5 | 3.2 | 2.8 | 2.8 | 2.2 |
| Brace (see 5.4) | no | no | yes | no | yes, enhanced |

Air control is the fraction of ground acceleration available while airborne.

### 2.3 Movement verbs

- **Run, sprint, crouch, jump, mantle, slide** (sprint + crouch; 1.2 s, passes under Hang lines set above 0.7 m; crouch passes under lines above 1.0 m).
- **Firing while airborne** is allowed for every class. A Climber additionally keeps its full momentum through the Dog fire animation (the Updraft kit); other classes lose 15% of horizontal speed on each airborne fire.
- **Perch** (Climber only): hold against any surface for up to 1.5 s, then drop. Can fire Dogs while perched.
- **Brace** (Rigger, Breaker): hold crouch while stationary. See 5.4.
- **Ledge grab**: automatic within 0.5 m of a ledge while falling at under 8 m/s. Above 8 m/s, no grab. Players pulled off edges fast don't get to grab.
- **Ladders**: 2.0 m/s climb (Breaker 1.6 m/s). Spool usable while climbing; Cutter usable one-handed.
- No wall-running. Verticality comes from the Spool.

### 2.4 Falling

Fall damage is impact (see 7). Terminal velocity 50 m/s. A player hits the Stall threshold at a vertical impact of 12 m/s, which is a fall of roughly 7.3 m onto a hard surface. Soft surfaces (nets, water, scrap heaps) multiply the threshold: see 7.3.

## 3. The Spool (Smart Cable)

### 3.1 Firing Dogs

- **Input:** primary fire launches Dog A. Primary fire again launches Dog B. A third press releases the current line and launches a new Dog A.
- **Dog projectile:** physical projectile with gravity, 0.08 m radius. Speed by class in 3.4. Max flight distance = class line length. A Dog that exceeds range or hits nothing falls as a harmless prop and auto-recalls in 1 s.
- **Dog seats in:** any static world surface, any dynamic object, any player's Catch Plate (chest, back, left and right shoulder). Dogs do not seat in other lines, in Hooks, or in the Marshal gallery.
- **Self-dog:** secondary input seats Dog A (or B) in the player's own chest plate instantly. This is how you Slingshot or Pendulum.
- **Between A and B:** the line exists but is slack and inert. It renders dim. It can be walked through. The player has 4 s to fire B or A auto-recalls.
- **Mode selection:** three buttons or a radial. Mode can be changed at any time while a line is live, with the class switch delay from 3.4 (0.15 to 0.35 s). Mode can be pre-selected before firing.
- **Release:** dedicated input, or fire a new Dog A. A released Hang line becomes a set line (3.6). Released Haul and Jack lines simply vanish.
- **Recall:** both Dogs return to the Spool in the class recall time from 3.4 (0.4 to 0.9 s). Scrapper: instant.

### 3.2 Modes

Let L be the current distance between Dogs, L₀ the distance when the mode was engaged, and T the class tension (3.4).

**Haul (Contract).** The line applies force T along itself pulling the Dogs together, ramping from 0 to T over 0.4 s. It continues until L ≤ 0.5 m (the Dogs meet) or the line is released or cut. Maximum closing speed is capped at 25 m/s; above this the line goes slack. The ramp is the panic window's first component. T is the Spool motor's pull limit; it is deliberately modest (3.4) so that Heavy objects fall rather than fly and players are dragged, not fired.

**Jack (Repel).** The line drives the Dogs apart to a relative **launch speed V** (class value in 3.4) within 0.3 s, splitting the velocity between the two ends by inverse mass, then keeps pushing at T until L reaches the class line length, at which point the line releases automatically. Maximum separation speed 30 m/s. Jacking from a self-dog to a static surface is a launch: the player receives the whole of V, the wall receives nothing. A Climber launched at 18 m/s at 45° travels about 33 m before landing. The 0.3 s launch phase needs far more force than T (about 5 kN for a Climber off a wall). In fiction this is **the Kick**: the Spool dumps its capacitor into the line once per Jack and then falls back to motor pull T. The Kick is why Jack feels violent and Haul feels relentless. **Interlock:** the Kick does not fire when either Dog is seated in another player's Catch Plate (a scrapped body's plates count; the interlock reads the plate, not the pulse); a Jack against a player is the motor push at T only, ramped over 0.3 s. (Fiction: an Accord safety interlock in every sanctioned Spool. The Fray's red-match Spools have it filed off.) A self-dog Jack to a wall or object still Kicks. This keeps Overboard inside the panic window: 600 N on a 130 kg Rigger is a 4.6 m/s² skid, about two seconds to cross nine meters.

**Hang (Suspend).** The line becomes a rigid rod of length L₀. It holds up to the class **Hang load** (3.4) before slipping. Hang loads are roughly ten times the pull tension: the filament is strong, the Spool motor is the limit. A Rigger can hang 1,200 kg but can only pull with 600 N. Slipping extends the line 0.5 m per 0.1 s over the load, with a loud creak, until the load drops under the limit. A Hang line is a solid for players (see 3.5) and a constraint for objects. It can be used as a pendulum pivot, a tripwire, a bridge, or a barricade.

**Reel (within Hang).** While in Hang, holding the Haul input shortens the rod at 1.0 m/s and holding the Jack input lengthens it at 1.0 m/s, bearing up to the class Hang load the whole time. This is a winch, not a pull: slow, strong, and silent except for the ratchet. It is how a Rigger hoists a Breaker (the Gallows), how a crew lifts a load into Guillotine position, and how a hanging player climbs. Reeling against a load above the Hang load slips instead of lifting.

### 3.3 Force resolution

The line applies equal and opposite force to both Dogs. What moves is decided by mass:

- Two free bodies: each accelerates by T / m. A 220 kg Breaker and a 90 kg Climber on a Haul close at a 71:29 split in the Climber's disfavor.
- One body static (world geometry): the free body takes all the acceleration.
- One body braced (5.4): treated as static if the line's tension is below the brace threshold; otherwise the brace breaks and the body is free.
- Rated loads (6.3): static to any line with T under 1.0 kN. Only a Breaker's line, or a Doubler at overspin, moves them.
- Objects seated to the world via their own supports (a container on a stack) have a **break-free threshold** (6.2) that the line's force must exceed to move them at all.

### 3.4 Class tuning

| | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|
| Line length (m) | 50 | 20 | 40 | 30 | 20 |
| Pull tension T (N) | 400 | 400 | 600 | 800 (1,200 overspin) | 1,000 |
| Jack launch speed V (m/s) | 18 | 14 | 12 | 14 | 10 |
| Hang load (kN) | 4 | 4 | 12 | 8 | 10 |
| Dog speed (m/s) | 140 | 160 | 120 | 120 | 90 |
| Mode switch (s) | 0.20 | 0.15 | 0.25 | 0.25 | 0.35 |
| Recall (s) | 0.4 | instant | 0.6 | 0.6 | 0.9 |
| Set lines | 1 | 1 | 2 | 1 | 1 |
| Set line life (s) | 8 | 8 | 12 | 8 | 8 |
| Active lines | 1 | 1 | 1 | 2 | 1 |

Conversion to feel: a Breaker's 1,000 N on a 130 kg Rigger is 7.7 m/s², just under 0.8 g, enough to take them off their feet (5.3) and drag them 15 m in about two seconds. A Rigger's 600 N on a 100 kg barrel is 6 m/s². On a 2,000 kg container it is 0.5 m/s², which is why containers lurch and fall rather than fly: a line starts the Avalanche, gravity finishes it. A Scrapper's 400 N on a 45 kg propane cylinder is 8.9 m/s²: it reaches 19 m/s over a 20 m line, about 850 N·s, a flip but not a Stall (7.1). Scrappers chain; they do not pull straight.

### 3.5 Lines as objects

- A **live Haul or Jack line** is not solid. Players and objects pass through it. It renders bright and audibly sings; brightness and pitch scale with the force it is currently applying.
- A **Hang line** is solid to players. A player hitting a Hang line while moving faster than 3 m/s is **staggered** (0.6 s loss of control, 1.0 s against a Rigger's line). Faster than 6 m/s, the player is **flipped**: ragdolled over the line, landing on the far side with their momentum preserved. Crouching passes under lines set above 1.0 m. Sliding passes under lines set above 0.7 m.
- Hang lines are **not solid to objects**; they are constraints for whatever they are attached to and ignored by everything else. (A container falling onto a tripwire does not catch on it. Tested against the alternative: solid lines create physics explosions.)
- **Any player can cut any line** they can reach with the Cutter (4.2), including their own, set lines, and enemy lines.
- A line attached to a player who is scrapped releases immediately.
- A line's Dog that was seated in an object that shatters reseats in the largest fragment.
- **A kinematic severs any line it passes through.** A Hauler through a trench bridge, the Gantry load through a Gallows, the Maintenance Ring through a set line: the line snaps (4.2 recoil), both Dogs release. Lines attached to the kinematic itself are not severed by it. Hooks (recovery drones) are exempt: they pass through lines without cutting them.

### 3.6 Set lines

Releasing a Hang line leaves it in place as a set line for the class set-line life. It behaves exactly as a live Hang line (solid to players, holds objects) but applies no new force. When a set line expires it goes slack and recalls over 0.5 s with a visible fade. A player can have the class number of set lines; setting another retires the oldest. The owner can recall a set line early at any time (this is how a Guillotine is dropped).

### 3.7 Overspin (Doubler only)

Holding the current mode's input past the normal engagement escalates tension from T toward 1,200 N over 1.0 s. At 1.0 s the line's pitch jumps an octave (the overspin tell). At 1.8 s the line **snaps**:

- Both Dogs release.
- The Doubler is launched opposite the line's direction at 10 m/s and staggered 0.8 s.
- Whatever was on the far Dog receives the stored energy as a single impulse of **1,500 N·s × (F / 1,200 N)**, where F is the force at the instant of the snap. At full overspin against a player this is a Stall (7.1). A line snapped early delivers proportionally less.
- The line is unavailable for 3 s.

A Doubler can release at any point before 1.8 s and keep the elevated tension gained so far. Overspin is available on all three modes. On Hang it raises the Hang load instead.

### 3.8 Two active lines (Doubler only)

A Doubler can fire A, B, then C, D without releasing the first. Tension is split: each line gets 60% of T (480 N; overspin raises each to 720 N). Each line can be in a different mode. Releasing one restores the other to full T over 0.3 s.

## 4. The Cutter (Thermal Knife)

### 4.1 Draw and cut

| | Standard | Hot | Long | Scrapper class bonus |
|---|---|---|---|---|
| Warm-up (draw to cutting) | 0.20 s | 0.12 s | 0.26 s | −0.05 s |
| Swing | 0.15 s | 0.15 s | 0.15 s | |
| Recovery | 0.30 s | 0.30 s | 0.30 s | |
| Reach (m) | 1.5 | 1.1 | 2.0 | |

Breaker class penalty: +0.10 s warm-up. Rigger class bonus: −0.03 s.

The Cutter is held in the off hand and does not interrupt Spool use except during the swing.

### 4.2 Cutting a line

A swing cuts any line whose segment passes within reach and within a 70° cone of the camera's forward vector. The line is severed at the point nearest the blade. Both Dogs release. The snap plays, scaled by the tension at the moment of cutting. A cut line delivers a small recoil of 150 N·s per 1,000 N of force to both ends, below the stagger threshold; only the Doubler's overspin snap (3.7) is dangerous.

**Cutting a line attached to yourself:** the line's attachment point on your body is always within reach. The requirement is the 70° cone: **you must look at the line.** A line seated in your back plate requires turning the camera until the line's path is in front of you. This is the panic window's skill test. The HUD marks the direction of any line on your body with an edge indicator (see UI document), never the line itself.

### 4.3 The panic window, by the numbers

A Breaker Hauls a Rigger (130 kg) toward a 20 m drop from 15 m away:

| Time | State |
|---|---|
| 0.00 s | Dog seats in the back plate. Line sings. Edge indicator appears. |
| 0.26 s | Ramp passes 650 N: 5 m/s² on 130 kg, the Rigger's foot-hold (5.3). Feet leave the floor. Skid becomes drag. |
| 0.40 s | Haul at full 1,000 N. Rigger accelerating at 7.7 m/s². The Breaker is braced and takes none of it. |
| 1.0 s | Rigger has moved about 2.5 m, now at 5.5 m/s. |
| 1.6 s | About 8 m moved, 10 m/s. |
| 2.2 s | 15 m. Over the edge at roughly 15 m/s. Hooks spin up at 2.7 s. |

The Rigger's Cutter needs 0.17 s warm-up + 0.15 s swing after they have turned to face the line, and a back-plate line needs a 150° turn. That leaves roughly 1.5 s to find it. **Design target: 1.0 to 2.0 s from Dog seat to the point of no return, in every lethal configuration.** Shorter configurations (a short Haul into a barrel) are allowed only when the Hauler had to approach to within 5 m, which is its own tell.

### 4.4 The Cutter as a melee

A Cutter swing that hits a player's Catch Plate does nothing lethal. It **unseats** any Dog in that plate and staggers the target 0.3 s. It is a tool for freeing a teammate, not a weapon. There is no melee kill.

### 4.5 Cutting map cables (Hangtown only)

Slack cables marked with orange tape are cut by any Cutter in 0.6 s of continuous contact (a hold, not a swing). The structure they support responds per the arena document.

## 5. Players as physics bodies

### 5.1 Grounded vs airborne

A grounded player moving under their own power is a kinematic character. A line force on a grounded player is applied as a **drag**: the player's movement is overridden by the line's acceleration while their feet remain planted and animate a skid. When the line's acceleration exceeds the class **foot-hold** (5.3), or the player is pulled upward, or they leave a ledge, they become **airborne**: a simulated ragdoll-with-pose (physical body, animated upper body) that the line acts on fully.

An airborne player regains control on landing if their landing impact is under the stagger threshold (7.1). Otherwise they stagger, then regain control.

### 5.2 Hoisted

A player pulled upward by a line with their feet off the ground is hoisted. Hoisted players can look, cut, and fire Dogs. They cannot move. Release drops them.

### 5.3 Foot-hold

| | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|
| Foot-hold (m/s²) | 3 | 4 | 5 | 5 | 5 |

Line acceleration on the player above this value pulls them off their feet. Below it, the player is dragged in a grounded skid (5.1) and keeps their footing. Examples: a Breaker's 1,000 N takes every unbraced class off its feet except another Breaker (4.5 m/s²). A Doubler's full overspin (1,200 N, 5.5 m/s²) takes an unbraced Breaker off its feet; a braced one (20 m/s²) never moves. A Scrapper's 400 N takes only a Climber (4.4 m/s²) off its feet; everyone else skids.

### 5.4 Brace

Rigger and Breaker can brace: crouch while stationary for 0.3 s. A braced player's foot-hold rises to 9 m/s² (Rigger) or 20 m/s² (Breaker). A braced Rigger holds against a Breaker's line (7.7 m/s²) but not a Doubler's full overspin (9.2 m/s²). A braced Breaker is static to every line in the game; only an object impact above the stagger threshold or a kinematic breaks the brace. Bracing ends when the player moves, jumps, or is hit by an object above the stagger threshold.

### 5.5 Catch Plates

Four plates: chest, back, left shoulder, right shoulder. A Dog seats in whichever is nearest its impact point. The plate determines where the line leaves the body, which determines how far the player must turn to see it. Chest: already visible. Shoulders: 60–90° turn. Back: 150°+ turn. Backs are the kill plate.

## 6. Objects

### 6.1 Mass tiers

| Tier | Mass | Examples | Moves on |
|---|---|---|---|
| **Light** | under 30 kg | chairs, cans, chain, tools, laundry, single planks and boards | Any line. Scrapper lines move these 30% faster. |
| **Medium** | 30–300 kg | barrels, propane cylinders, crates, carts, scaffold board stacks, market stalls | Any line. Scrapper lines move Medium objects under 100 kg 30% faster (a propane cylinder yes, a fuel drum no). The bonus is a 30% higher travel and arrival speed on those props (implemented as 1.69× line force on them), applied to Haul and to the post-Kick Jack push, never to the Kick's launch speed V. |
| **Heavy** | 300–3,000 kg | shipping containers, billet crates, hanging houses, counterweights, the Claim marker, the Haul spool | Any line, slowly, when staged on rollers or skids (6.1a). T / m is well under 1 g. Falls rather than flies. |
| **Rated** | over 3,000 kg or structural | columns, drive dogs, crane counterweights, anchor pins, maintenance clamps | Breaker line (1,000 N) or Doubler overspin only. Marked with yellow-and-black tape. |

### 6.1a Heavy objects, friction, and rollers

At these tensions a Heavy object resting on a floor does not slide: a 900 kg marker on steel at friction 0.3 needs 2,600 N. So the rule, in fiction and in engine, is that **anything Heavy that is meant to move sits on rollers, skids, greased runners, or hangs.** Prep crews are riggers; they stage loads the way riggers do. Authored placements:

| Heavy placement | Effective friction | Starts moving at |
|---|---|---|
| On rollers or a wheeled cradle | 0.02 to start, 0.005 rolling | about 0.2 N per kg to start, then 0.05 N per kg: a Rigger starts 3,000 kg, a Climber 2,000 kg, and every extra line adds speed |
| On skids or greased runners | 0.05 | about 0.5 N per kg: a Breaker moves 2,000 kg; a Rigger 1,200 kg |
| Hanging (house, Gantry load, Guillotine load) | 0 (pendulum) | any line swings it slightly; see the swing rule below |
| Directly on the floor | 0.3 or more | no line. Treat as fixed. Mark it; it is scenery that breaks, not a load that moves |

The Claim marker sits on polished runners (0.05): a Rigger or heavier moves it, slowly. The Haul spool sits on rollers (0.02) and weighs 2,000 kg: 400 N to start, so any class can move it alone, but a crew with three lines adding as vectors (6.2a) moves it three times faster, which is the point of the mode. Water tanks are fixed: they rupture (6.5, 6.6); they do not move. Containers in Ballast ride the magnetic racks' dead rollers, which is why the Breathing slides them. Any stack meant to Avalanche sits on rollers (the bottom unit is dragged out and the rest fall); the Yard's billet stacks are on roller pallets for that reason. A stack on skids is tipped, not dragged, which is a slower, different play.

### 6.2 Break-free threshold

Objects at rest in a placed state (a container in a stack, a barrel in a rack, a house on its cable) have a break-free threshold: the line force required to start them moving. Below it, the line sings and the object creaks and does not move. At or above it, the object releases with a visible lurch. **All thresholds in this document are inclusive:** a Doubler's 800 N starts a bolted object, a Breaker's 1,000 N starts a rated load.

**Break-free is tested against active pull only** (Haul and Jack force, and the Kick). A Hang line does not tug; it holds. The static load a Hang line puts on a placed object is carried by the object's own placement, so a player can Pendulum from a racked cylinder, a stacked container, a bolted beam, or a rated column without unseating it, up to the class Hang load. A Hang line to a loose object (break-free 0) simply brings the object along as dead weight. Rated loads take several seconds of Haul after the 1.0 s groan before visible motion (0.17 to 0.27 m/s²); that slowness is intended and is the tell.

| Placement | Break-free (N) | Who can start it |
|---|---|---|
| Loose on floor | 0 | Anyone |
| In a rack or cradle | 200 | Anyone |
| In a stack (friction) | 500 | Rigger, Doubler, Breaker. Not Climber or Scrapper (400 N). |
| Bolted or chained (removable) | 800 | Doubler, Breaker |
| Rated / structural | 1,000 | Breaker alone; Doubler at overspin; or any two or more lines whose active pull sums to 1,000 N along the same axis (6.2a). It takes a Breaker or a crew. |
| Hanging house on its load cable (Hangtown) | 1,000 | Breaker; Doubler at overspin; or any Cutter on the house's slack cable (4.5) |

### 6.3 Rated loads

Tier is recomputed whenever an object's mass changes (a rack that loses its cylinders, a barrel that empties). 50 m/s is a hard cap on the speed of every simulated object and player, however it was reached.

At these tensions a line cannot swing a Heavy hanging object into anything dangerous on its own: a Breaker's 1,000 N deflects a 1,200 kg house to a steady 0.76 m/s, a flip at most. **Swings come from gravity.** Cut a slack cable, let Cable Day settle a platform, or pump the Sway with a Hang line timed to the arc; the line starts the motion, the drop or the sway supplies the energy. Arena documents describe plays on that basis.

### 6.2a Multiple lines on one object

Pull forces from several lines on one object add as vectors. Hang loads from several lines on one object add: two Riggers can hang 2,400 kg between them. This is the only way to hang a 2,000 kg Haul spool (19.6 kN) and the reason Haul mode is a crew mode.

Rated loads are the map's big levers. Each one has an authored consequence when moved (a column drops a ceiling section; a clamp drops a scaffold ring). The 1,000 N break-free is a **commit trigger**, not a physics slide: once active pull reaches it for 0.6 s on the authored axis, the load's scripted timeline carries the motion (the 1.0 s groan, then the fall), and friction and mass no longer matter. The load becomes a simulated body only when its timeline hands it to the world (the piston drops, the slab tips). Rated loads reset only between rounds.

### 6.4 Volatiles

| Volatile | Trigger | Effect |
|---|---|---|
| **Propane cylinder** (Medium, 45 kg) | Impact over 8 m/s, or another blast | Blast radius 4 m. Impulse 1,500 N·s at center falling linearly to 0 at 4 m. Launches everything. Players within 2 m are Stalled and players at 2–4 m are flipped: these are authored results that override the falloff, so a blast is always readable. |
| **Fuel drum** (Medium, 180 kg) | Impact over 10 m/s | Blast radius 3 m, impulse 1,200 N·s. Leaves a 3 m fire patch for 6 s: fire staggers on entry and pushes outward at 4 m/s. No damage over time; there is no health to damage. |
| **Demolition charge** (Light, 12 kg, placed on structure by prep crews) | Impact over 6 m/s to the charge itself by an object or a player, or a blast. **Dogs seat in a charge without triggering it**; a charge is a thing you move, not a thing you shoot. | Blast radius 5 m, impulse 2,500 N·s. Breaks the structural element it is attached to. Charges are the authored way to drop a specific wall. |
| **Compressed-gas manifold** (fixed world geometry; cannot be moved by any line) | Line Jack at or above 600 N applied to it, or blast | Vents: a 6 m directional jet at 15 m/s for 3 s. Pushes, does not kill. |

Chain reactions are allowed and intended. Volatiles seated to each other by a Haul (the Kiss) trigger on contact. Blasts under more than 1 m of water deliver half impulse and no fire patch.

### 6.5 Destructibles

Objects that break apart on impact or load. Each has a **fracture threshold** (impulse) and a fragment pattern. Fragments inherit the object's tier if large enough, otherwise become Light props with a 10 s lifetime. A player's body counts as an impacting object against these thresholds (a flipped 130 kg Rigger at 5 m/s delivers 650 N·s and breaks a market stall).

| Destructible | Fracture threshold (N·s) | Fragments |
|---|---|---|
| Market stall, furniture | 300 | Light planks |
| Scaffold section | 800 | Light boards, Light clamps, one Medium frame piece |
| Hanging house, small (1,200 kg) | 2,000 | Heavy wall pieces, Medium roof, Light contents |
| Hanging house, large (2,800 kg) | 3,500 | Heavy wall pieces, Medium roof, Light contents |
| Catwalk section | 2,500 | Medium grate pieces |
| Walkway on cable | 2,500 | Medium deck pieces, Light rail |
| Plank bridge | 600 | Light planks |
| Shipping container | 6,000 | Does not fracture; dents. Doors blow open and spill contents. |
| Water tank | 1,500 | Medium shell pieces; releases a force volume (6.6) |

### 6.6 Force volumes

Non-object hazards: water rush, updraft, Pulse, gas jet. Each applies acceleration to everything inside it by a profile authored per arena. Players inside a force volume are airborne if the volume's acceleration exceeds their foot-hold. Force volumes apply a force, authored as the acceleration a 100 kg reference body would feel: a body of mass m feels a_ref × 100 / m. A 90 kg Climber is lifted harder than a 220 kg Breaker by the same gust. The resulting acceleration on any body is capped at 2 × a_ref so Light props do not become permanent airborne debris in a gust core. Force volumes act on players and on free bodies only: loose objects (break-free 0) and objects held by a player's line rather than by a placement (a Hang-held load swings in a gust). Racked, stacked, bolted, rated, and cable-hung map objects (houses, Gantry loads) are unaffected until something else frees them. Force volumes never Stall directly; they move things into things that do.

### 6.7 Kinematics

Haulers, the Gantry, the Maintenance Ring, the Lift, the Breathing hull. Infinite mass, scripted path. Nothing moves them. Anything they hit takes the full relative impact. A line between a kinematic and anything else moves the anything else. Some kinematics carry a **rated attachment** (the Lift's counterweight on its guide, the Gantry's load on its magnet): the carried part is kinematic while attached and becomes a simulated rated load when active pull on it reaches its 1,000 N break-free (a Breaker alone, a Doubler at overspin, or a crew summing on one axis), which releases it from the path.

## 7. Impact, stagger, and the Stall

### 7.1 Thresholds on a player

| Event | Stagger | Flip | Stall (scrapped) |
|---|---|---|---|
| Player hits static world | relative speed 5 m/s | 8 m/s | **12 m/s** |
| Player hits kinematic | 4 m/s | 6 m/s | **9 m/s** |
| Object hits player | impulse 300 N·s | 600 N·s | **1,200 N·s** |
| Player crushed (pinned between two bodies with closing force) | | | **force over 8 kN for 0.5 s** (unreachable by any line; comes from kinematics and from stacked or falling weight over about 800 kg) |
| Player hits another player | both: 5 m/s | 8 m/s | **14 m/s** (both scrapped) |
| Blast | see 6.4 | | |

Stagger: 0.6 s with movement and Spool inputs locked; look and Cutter stay live, because the panic window must survive a stagger. Camera shake. Flip: ragdoll until landing, then stagger. Stall: match over for that player.

Impulse examples: a 45 kg propane cylinder arriving on a Haul at the 25 m/s cap is 1,125 N·s (flip); the same cylinder at 27 m/s off a Jack push (cap 30 m/s) is 1,215 N·s (Stall), which is why Scrappers Jack cylinders into people rather than Haul them. A 180 kg drum at 7 m/s is 1,260 (Stall). A 12 kg plank would need 100 m/s. A 2,000 kg container at 0.6 m/s is 1,200: a container does not need speed.

### 7.2 Mass matters on both ends

A Breaker (220 kg) hitting a wall at 12 m/s Stalls like anyone else: the threshold is speed against static. But a Breaker hitting a Climber at 10 m/s delivers 2,200 N·s to the Climber (Stall) while taking the Climber's 900 N·s back (flip only). Player-on-player uses the object-impulse table on each party separately, computed from the other's mass and the relative speed. The 14 m/s symmetric rule in 7.1 is the floor for equal masses.

### 7.3 Surface modifiers

| Surface | Threshold multiplier (applies to stagger, flip, and Stall) |
|---|---|
| Hard (steel, concrete) | ×1.0 |
| Wood, planks | ×1.2 |
| Scrap heap, laundry, drip nets | ×2.0 |
| Water over 1 m deep | ×3.0 |
| Stall Gel pad (prep-crew placed, rare) | ×4.0 |

### 7.4 The Stall sequence

1. Threshold crossed. Harness locks the ragdoll in its current pose with gel VFX. 0.2 s.
2. Scrap Card displays to the killer and victim. Marshal calls it. (A scrapped Climber at 90 kg is a Medium object under 100 kg; the Scrapper bonus applies to it. Riggers find this funny.)
3. Body persists as a non-fracturing prop at the class mass (90 to 220 kg; tier Medium for all classes) for 4 s, then a Hook arrives and lifts it out over 2 s. Exception: bodies that enter the Anchor's Well are not retrieved by Hooks (see the Anchor in 03); they simply fall out of sight. Bodies can be used as props during those 4 s. This is in poor taste and riggers do it.
4. All lines attached to the body release at step 1.

### 7.5 Ring-out

A kill volume at each arena's floor. Entering it is a Stall with a distinct Scrap Card ("down the Well"). Hooks spin up audibly when a player falls past the lowest playable floor, 0.5 s before the volume.

## 8. The interaction matrix

What each mode does between each pair of things. "Mover" is decided by 3.3.

| A ↔ B | Haul | Jack | Hang |
|---|---|---|---|
| **Player ↔ static world** | Player is pulled to the point. Grapple. | Player is launched away. Slingshot. | Player hangs from the point. Pendulum if moving, perch if still. |
| **Player ↔ player** | Both pulled together; lighter moves more. Collision at closing speed. | Both pushed apart; lighter moves more. Edge threat. | Rigid rod between them. Neither can leave the other's radius. Both can be swung by a third force. |
| **Player ↔ Light/Medium object** | Object flies to player. Player is hit at closing speed if they don't dodge. | Object launched away from player; player receives recoil equal to object's share. | Object hangs off player as dead weight, or player hangs off object if object is caught on something. |
| **Player ↔ Heavy object** | Player is pulled to object (object barely moves). If object is above, the player climbs by Reel. If object is falling, player is dragged with it. | Player launched away from object. Object barely moves. | Player tethered to object. If object goes over an edge, player follows unless they cut. The Anchor Drop. |
| **Player ↔ Rated** | Player pulled to it. Only a Breaker's line moves the load, and then the authored consequence fires. | Player launched. | Player anchored to the most solid thing on the map. |
| **Player ↔ kinematic** | Player dragged along its path. The Caboose. | Player launched off it. | Player rides it on a rigid tether. |
| **Object ↔ static world** | Object pulled to the point. Guillotine set-up when the point is overhead. | Object launched away. The Scatter half. | Object hung from the point. Guillotine armed. Pendulum weight. |
| **Object ↔ object** | Objects collide at closing speed. The Kiss. | Objects fly apart. The Scatter. | Rigid bar between them. A two-barrel bola. |
| **Object ↔ player (enemy)** | Object flies into the enemy. | Enemy pushed away from the object, or over an edge. | Enemy tethered to the object. |
| **Object ↔ kinematic** | Object dragged along. | Object launched. | Object rides. |
| **Static ↔ static** | Line sings, nothing moves. Wasted line. | Same. | Tripwire. Clothesline. Barricade. Bridge. |

## 9. Match rules

### 9.1 Formats

| Mode | Teams | Rounds | Round time | Reserve (respawn) | Win |
|---|---|---|---|---|---|
| **Scrap** | 3v3 | best of 5 | 2:30 | none | Last crew standing. Time-out: crew with more riggers; tie: sudden death, an arena-specific shrink event defined in each arena's events document |
| **Claim** | 3v3 | 1 | 8:00 | 8 s reserve timer, 3 reserves per crew per match | First crew to 100 Claim points (1 per second holding the marker uncontested) |
| **Haul** | 3v3 | 2 (swap sides) | 4:00 each | 8 s, unlimited reserves | Deliver the 2,000 kg spool to your truck, or get it furthest |
| **Free Scrap** | 8 solo | 1 | 6:00 | 10 s, unlimited | Most scraps |

### 9.2 Round flow (Scrap)

1. **Drop** (6 s): crews enter via their Drop animation at opposite spawns. Lines disabled.
2. **Live**: Marshal calls "live." Everything enabled.
3. **Mid-match event** fires with a 5 s siren at 1:15 remaining in a Scrap round, at 4:00 remaining in Claim, at 2:00 remaining in each Haul half, and at 3:00 remaining in Free Scrap. The arena documents' "midpoint" means these times.
4. **Round end**: Marshal calls it. 4 s replay of the final scrap from the victim's camera (the panic window from the inside is the game's best advertisement).
5. **Reset**: all objects reset to authored positions. Rated-load consequences reset. Set lines cleared. Scrap heaps and other prop pools do not replenish within a round; what is thrown stays thrown until reset.

### 9.3 Spawns

Two spawn zones per arena, at opposite ends of the high or mid tier, each 10 m across, never within 25 m of a Well edge measured along a walkable route (arenas where every tier overhangs the Well, such as the Anchor, spawn on the Rim's crane decks). Reserve riggers drop at their crew's spawn or, if it is contested (an enemy within 15 m), at the alternate.

### 9.4 Claim marker

A 4 m radius salvage marker (a Weave billet on a pallet, physical, Heavy tier). It can be moved by lines. Points accrue to a crew with at least one rigger inside and no enemies inside. Moving the marker is legal (its 441 N runner start is tested against the vector sum of all lines on it, so three Scrappers move it together); it resets to its authored position if it enters a Well.

## 10. Named rigs, mechanically

| Rig | Exact inputs | Fails if |
|---|---|---|
| **Gallows** | Dog A overhead anchor. Dog B enemy plate. Hang (the enemy is now tethered at that distance and cannot walk away). Hold Haul to Reel: the rod shortens at 1 m/s and the enemy hoists, slowly, bearing up to the class Hang load. A Breaker's 1,000 N Haul lifts a Climber outright (11 m/s²); everyone else hoists by Reel. | Enemy cuts; the Reel is slow by design, so the window is long and the threat is where they end up. Teammate Jacks the enemy off the line's arc. |
| **Guillotine** | Dog A load. Dog B anchor overhead. Hang. Release (set line, load hangs). When the enemy walks under, recall or cut your own set line: the load drops by gravity. A second line (load to enemy, Haul) only steers it. The load must be within your class Hang load to hang in the first place (a Rigger hangs 1,200 kg, a Climber 400 kg); heavier needs a teammate's second line (6.2a). | Set line expires. Enemy sees the hanging load (it's lit). Someone cuts the set line. |
| **Clothesline** | Dog A wall. Dog B opposite wall at 1.2 m. Hang. Release. | Enemy crouches or slides. Enemy cuts it. Timer. |
| **Slingshot** | Self-dog A. Dog B wall behind the direction you want to go. Jack. | Mistimed release; a Haul on you mid-flight. |
| **Pendulum** | Self-dog A. Dog B overhead anchor. Hang. Run off the edge. Cut or release at the apex. | Cut before apex; land badly (7.1). |
| **Kiss** | Dog A volatile. Dog B volatile. Haul. | Jacked apart. Volatiles' break-free threshold (racked cylinders need 200 N). |
| **Scatter** | Dog A volatile. Dog B volatile. Jack. They fly apart at up to 30 m/s and trigger on impact. | Only dangerous if there's something to hit. |
| **Caboose** | Dog A enemy plate. Dog B kinematic (Hauler, Gantry load). Haul or Hang. | Enemy cuts. Enemy Hangs to something heavier. |
| **Avalanche** | Dog A bottom container of a stack. Dog B anywhere toward you (or self-dog and brace). Haul above the 500 N stack break-free. | Under 500 N (Climber, Scrapper: can't). Standing in the fall line. |
| **Overboard** | Self-dog A. Dog B enemy. Jack, with an edge behind the enemy and something solid behind you (or brace). No Kick against a player (3.2 interlock): this is a T push, a skid of a few m/s², which is what gives the victim their window. | Enemy cuts; enemy Hangs to the floor; you're lighter than them and go the other way. |
| **Anchor Drop** | Dog A enemy. Dog B Heavy object near an edge. Hang. Push the object over (Jack from a second line, a teammate, or a kinematic). | Enemy cuts before the object clears the edge. Object's break-free. |

## 11. Tells (the readability contract)

Every lethal system has an audio tell, a visual tell, and a lead time. Minimums:

| System | Audio | Visual | Lead |
|---|---|---|---|
| Dog seated in your plate | distinct seat clank, directional | edge indicator | 0 s (the window starts here) |
| Line under force | singing, pitch = force | emissive, brightness = force | continuous |
| Overspin | octave jump at 1.0 s | line flickers white | 0.8 s to snap |
| Set line | low hum | dim emissive, brightens within 5 m of a player | continuous |
| Hang load slipping | creak | line jitters | 0.3 s per 0.5 m |
| Break-free | lurch, scrape | object shudders | 0.2 s |
| Rated load moving | structural groan | dust, authored debris | 1.0 s minimum before consequence |
| Kinematic approach | klaxon, then lights | lights | 3 s (Hauler), 2 s (Gantry drop) |
| Mid-match event | siren | authored sag or sparks | 5 s |
| Force volume | arena-specific (gust, rumble) | particles | 1 s |
| Ring-out imminent | Hooks spin up | | 0.5 s |
| Volatile armed by impact | hiss | venting | 0.3 s before blast |

## 11a. Team tints (canonical)

Team A **Amber #E69F00**, Team B **Sky #56B4E9**, third crew in 3v3v3 **Rose #CC79A7**, neutral and set lines **Bone #E8E6E0**. Chosen from a colorblind-safe palette. Enemy lines additionally carry a 1 m dark tick pattern so team is never carried by color alone. Line glow, Catch Plate rim bands, HUD crew markers, and Dog trails all use these and nothing else.

## 12. Tuning intent

- A Scrap round between even crews should average 90 s.
- A coordinated crew should land a two-step rig (Guillotine, Anchor Drop) roughly once per round. Solo one-step rigs (Slingshot into a Haul) should be the common kill.
- Across all arenas, target elimination mix: 40% object impact, 25% ring-out, 20% player-into-world, 10% blast, 5% crush. If ring-outs exceed 35% on a map, add floor.
- Panic-window survival rate for a median player should be about 50% at launch. Below 35%: lengthen the Haul ramp. Above 65%: shorten it.
