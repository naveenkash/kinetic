# Bible changelog

## 2026-10-24, 05-core-mechanics v0.2 (tension rescale and rulings)

Every document written before this change must be updated to these values.

### Rescaled: pull tension is now in newtons, ten times lower

| | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|
| Pull tension T | 400 N | 400 N | 600 N | 800 N (1,200 N overspin) | 1,000 N |
| Jack launch speed V (new) | 18 m/s | 14 m/s | 12 m/s | 14 m/s | 10 m/s |
| Hang load (unchanged) | 4 kN | 4 kN | 12 kN | 8 kN | 10 kN |
| Foot-hold (new) | 3 m/s² | 4 m/s² | 5 m/s² | 5 m/s² | 7 m/s² |
| Braced foot-hold | | | 9 m/s² | | 20 m/s² |

Doubler two-line split: 480 N each (720 N overspin). Rated loads move at 1,000 N. Manifold vents at a Jack over 600 N.

Break-free: loose 0 N, rack 200 N, stack 500 N, bolted 800 N, rated 1,000 N.

### Jack redefined
Jack drives the two ends apart to relative speed V within 0.3 s (split by inverse mass), then pushes at T until max length. Self-dog to wall: the player gets all of V.

### Snap
Normal cut: 150 N·s recoil per 1,000 N to both ends (below stagger). Doubler overspin snap: 1,500 N·s × (F / 1,200 N) to the far end.

### Rulings
- Dogs seat in demolition charges without triggering them. Charges trigger from object or player impact over 6 m/s, or a blast.
- Propane 2 m Stall / 2–4 m flip are authored overrides of the falloff.
- Gas manifold is fixed world geometry; no line moves it.
- Multiple lines on one object: pull forces add as vectors; Hang loads add. Two Riggers hang 2,400 kg.
- Scrapper 30% speed bonus: Light props and Medium props under 100 kg.
- Scaffold section fragments: Light boards, Light clamps, one Medium frame piece. Single boards are Light; board stacks are Medium.
- Hanging house fracture: small 2,000 N·s, large 3,500 N·s.
- Player bodies count as impacting objects against fracture thresholds.
- Blasts under more than 1 m of water: half impulse, no fire.
- Surface multipliers apply to stagger, flip, and Stall thresholds.
- Object tier is recomputed when mass changes. Terminal velocity 50 m/s for everything.
- Kinematics may carry a rated attachment (Lift counterweight, Gantry load) that becomes a simulated rated load when a Breaker's line exceeds 1,000 N.
- Stagger locks movement and Spool only; look and Cutter stay live.
- Mid-match event: 1:15 remaining in Scrap, 4:00 in Claim, 2:00 per Haul half.
- Slide passes under Hang lines above 0.7 m; crouch above 1.0 m.
- Third person: casual only, locked at 90° FOV.
- Mode switch and recall are per class (section 3.4).
- Bodies in the Anchor's Well are not retrieved by Hooks.
- Sudden death is an arena-specific shrink event defined per arena.
- Airborne Dog fire allowed for all; Climber keeps full momentum, others lose 15% horizontal speed.

### Panic-window reference case (Breaker Hauls Rigger, back plate, 15 m from a drop)
0.26 s feet leave floor; 0.40 s full 1,000 N (7.7 m/s²); 1.0 s at 2.5 m, 5.5 m/s; 2.2 s over the edge at 15 m/s.

## 2026-10-24, follow-up rulings (v0.2.1)

- Teodora Quispe is 72, not 58. She did not write the First Rule; she enforces it.
- Paloma Reyes-Okafor was not born on Collar Seven; her grandmother survived its fall and Paloma speaks of it as inherited memory.
- Six Collars came down on land (Two through Seven), One into the sea, five lost upward.
- Weight classes: Light = Climber, Standard = Scrapper, Rigger, Doubler, Heavy = Breaker.
- A scrapped body keeps its class mass (90 to 220 kg), tier Medium.
- Team tints are canonical in 05 section 11a: Amber #E69F00, Sky #56B4E9, Rose #CC79A7, Bone #E8E6E0, with a 1 m dark tick on enemy lines. Any other team tint proposal is superseded.
- The Jack launch phase is called the Kick (capacitor discharge); it is separate from motor pull T.
- All thresholds are inclusive (at or above).
- Launch format is 3v3; 3v3v3 is post-launch.
- Hanging houses attach at a rated 1,000 N; only a Breaker, a Doubler at overspin, or a cut slack cable drops one.
- Lines cannot swing Heavy hanging objects dangerously on their own; house and platform swings come from gravity (cut cable, Cable Day settle, Sway pumping).
- Break-free is tested against active pull (Haul, Jack, Kick) only. Hang lines hold without unseating placed objects; players can Pendulum from anything up to the class Hang load.
- 50 m/s is a hard velocity cap on everything, not only free fall.
- Rated loads moving at 0.17 to 0.27 m/s² after the groan is intended.
- Breaker unbraced foot-hold is 5 m/s² (was 7) so a Doubler's overspin can take an unbraced Breaker off its feet and Brace matters for Breakers.
- Scrapper 30% bonus applies to Haul acceleration and the Jack push, not the Kick's V.
- Crush is unreachable by lines; it comes from kinematics and stacked or falling weight.
- Heavy objects move only when staged on rollers (friction 0.02) or skids (0.05); on a bare floor they are fixed scenery. Marker on runners, spool on rollers, water tanks fixed. See 05 section 6.1a.
- Ballast's Haul route is the Spine, on a counterbalanced ratchet rail (200 N pawl drag, 4,000 kg moving mass, any class moves it). Supersedes the earlier Bays-deck note.
- Scrapper bonus applies to a scrapped Climber body (90 kg). Prop pools do not replenish within a round.
- Breaker foot-hold change (7 to 5) applies everywhere; worked examples in object docs updated.

## 2026-10-24, v0.2.2 (play-preserving rulings from the arena pass)

- **Reel:** within Hang, holding Haul shortens the rod at 1.0 m/s and holding Jack lengthens it, bearing up to the class Hang load. The Gallows hoists by Reel (slow, strong); only a Breaker's Haul lifts a Climber outright. Hanging players climb by Reel.
- **Kick interlock:** the Kick never fires when either Dog is in another player's Catch Plate; a Jack against a player is the T push only. Overboard stays inside the panic window.
- **Guillotine step 2** is recall or cut your own set line; gravity drops the load. Owners can recall set lines early.
- **Kinematics sever any line they pass through** (snap, both Dogs release).
- **Force volumes act on players and loose objects only.**
- Free Scrap mid-match event at 3:00 remaining.
- Spawn distance from the Well is measured along walkable routes; the Anchor spawns on Rim crane decks.
- Ballast's Spine is the forward 72 m of the shaft. The Anchor's Rim deck is 240 m up the Stump; the Crown is 60 m above the Rim. The Pulse fires every 75 s from 0:55.
- Rated loads: a Breaker alone, a Doubler at overspin, or any lines summing to 1,000 N on one axis (a crew). 02 updated.
- Force volumes: authored as acceleration on a 100 kg reference body; a body feels a_ref × 100 / m. Loose objects and players only.
- Fracture thresholds added: walkway on cable 2,500 N·s, plank bridge 600 N·s.
- Ladders: 2.0 m/s (Breaker 1.6).
- The Pulse: 0.4 s outward shove then 0.6 s inhale toward the Stump. The Anchor Well's ring-out tell is the Stump's hum dropping an octave.
- Gantry: Shift Change drops whatever the magnet holds; an empty magnet drops nothing and the next pick comes early.
- Arena geometry authority is docs/arenas/*/layout.md (Anchor Rim datum 0, Scaffold 6 to 38 m, Ring 42, Crown 60; Ballast Spine forward 72 m).
- Rollers: 0.02 to start, 0.005 rolling, so any class starts the Haul spool and extra lines add speed. Marker start threshold is tested against the vector sum of lines.
- Force volumes: "free bodies" includes objects held only by a player's line (a Hang-held load swings in the Updraft); cable-hung map objects such as houses do not.
- Scrapper bonus is 30% on travel and arrival speed (1.69× force on qualifying props). Force-volume acceleration capped at 2 × a_ref per body. Scrapped bodies' plates still trigger the Kick interlock. Overboard by a light class against a Breaker being nearly inert is intended.
- Rated break-free is a commit trigger; the authored timeline carries the motion regardless of friction or mass (item 53). Stacks meant to Avalanche sit on rollers; the Yard's billet stacks are on roller pallets, not skids (item 54).
- Yard Rails are 36 m in 03. Rated attachments on kinematics release on any active pull reaching 1,000 N (crew sums count).
- Hooks do not sever lines.
