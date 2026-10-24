# KINETIC: Audio Design

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 05 sections 3.5, 3.7, 4.2, 7.4, 7.5, 11 (the tells table) and the sound directives in 01 and 03. Numbers in 05 are canonical. Anything marked **PROPOSED** is new here._

## 1. Stance

Audio is a gameplay system before it is a mood. Bible 05 section 11 says every lethal thing has an audio tell with a lead time. The mix exists to make those tells win. If a player dies without hearing it, audio has a bug.

Three rules:

1. **Tells first.** The ranked mix priority in section 12 is a contract, not a guideline.
2. **Nothing plays over the Live state that is not a tell, a voice, or the world.** No music while the Marshal has called "live."
3. **Every tell is directional.** Seat clanks, singing lines, groans, Hooks, klaxons: all spatialized, all with HRTF on headphones.

## 2. The line singing system

The filament heats under load and sings. Pitch and brightness scale with force. This is the central sound of the game and the central readout (05 section 3.5).

### 2.1 Pitch map

Fundamental frequency as a function of current applied pull force F in newtons:

f(F) = 110 Hz × 2^(F / 400 N)

| Force | Pitch | Who sings here |
|---|---|---|
| 0 N (slack, A seated, no B) | no pitch; slack hum (2.4) | Any class between A and B |
| 100 N | 131 Hz (C3) | Ramp start, any class |
| 400 N | 220 Hz (A3) | Climber, Scrapper at full T |
| 480 N | 253 Hz | Each Doubler line when two are live |
| 600 N | 311 Hz (D#4) | Rigger at full T |
| 800 N | 440 Hz (A4) | Doubler at full T, single line |
| 1,000 N | 622 Hz (D#5) | Breaker at full T |
| 1,200 N | 880 Hz (A5) | Doubler at overspin ceiling |

Every 100 N is a quarter-octave; the note doubles every 400 N. A veteran hears the class on the other end of the line before seeing it. A Haul ramps from 0 to T over 0.4 s: a rising glissando ending on the class note. A Jack opens with the Kick (section 2.7) and settles onto the same note from above. Either way the engagement ends on the class note. Hang lines carry loads in kN, roughly ten times the pull tension (05 section 3.2), so they use the same curve through a ratio (**PROPOSED**): a Hang line sings at f(T × load / Hang load). A Rigger's Hang line holding nothing is nearly silent; holding its full 12 kN it sings the Rigger's note, D#4, with every partial; over the Hang load it climbs past the note and the slip creak joins it. Each class has one note whether it pulls or hangs.

### 2.2 Timbre

A bowed-wire model. Base: two detuned sines plus a resonant bandpass on filtered noise, bowed with a slow 6 Hz amplitude tremor. Force adds harmonics: at 100 N, two partials audible; at class T, six; at 1,200 N, all partials plus a metallic inharmonic cluster. Brightness (spectral centroid) scales linearly with F. Class flavor is a fixed filter curve per class, so two 400 N lines are the same note but distinguishable:

| Class | Timbre note |
|---|---|
| Climber | Thin, pure, long decay. Needle Dogs. |
| Scrapper | Buzzier, a 1.5 kHz rattle at any force. Modified Spool. |
| Rigger | Warm, stable, no rattle. Old-pattern. |
| Doubler | Two lines beat against each other (480 N each, detuned 3 cents apart on purpose). The beating is the two-line tell. |
| Breaker | Heavy fundamental, sub-harmonic at f/2 above 800 N. |

Rope Mark Tape and Line cosmetics never change the pitch map or brightness curve. The Longline, Hawser, and other sidegrades change T, so they change the final note; that is correct and informative.

### 2.3 Overspin

Per 05 section 3.7. Holding the mode input past engagement escalates force from T toward 1,200 N over 1.0 s, so pitch glides up continuously. At 1.0 s the pitch **jumps an octave** (f × 2) on top of the glide: a Doubler at 1,000 N at that instant goes from 622 Hz to 1,245 Hz in one frame. The octave jump is the tell with 0.8 s to snap. From 1.0 s to 1.8 s a 12 Hz tremolo deepens to full depth. At 1.8 s: the snap (2.6), at maximum scale.

### 2.4 Slack hum and set-line hum

- **Slack** (A seated, no B; or Haul past the 25 m/s cap): a 55 Hz hum, no partials, 20 dB under the singing level, audible within 4 m. It says "a line exists and is inert."
- **Set line** (released Hang, 05 section 3.6): a low hum, 55 Hz plus 110 Hz, with a slow 0.5 Hz swell. Audible within 8 m; within 5 m of any player it rises 6 dB (matches the emissive brightening in 05 section 11). On expiry, the hum pitches down a fifth and fades over 0.5 s with the visible fade.
- **Hang load slipping**: a creak, 0.3 s per 0.5 m, dry wood-on-wire, layered over the singing. Lead tell per 05 section 11.

### 2.5 Mixing multiple lines

Up to six lines sing as discrete voices. Priority for voice allocation:

1. Any line seated in the listener's Catch Plates.
2. Enemy live lines within 10 m.
3. Own crew's live lines.
4. Any set line within 5 m.
5. Everything else by distance.

Lines beyond the six voices sum into a single "chord bed" voice at the median pitch, filtered, at 12 dB under. A new line seated in the listener always steals a voice. Two voices at the same pitch within 2 Hz are detuned 4 cents apart so they do not phase-cancel.

### 2.6 The snap

Cutting or snapping a line releases everything stored in it (01). One sound, universal, never cosmetic (04). It plays scaled by tension at the moment of release:

| Tension at cut | Snap |
|---|---|
| under 100 N | Dry whip-crack, 0.2 s, local (20 m) |
| class T | Crack plus a metallic ring-off, 0.6 s, audible 60 m |
| 1,000 to 1,200 N | Full snap: crack, sub thump, ring-off with a descending pitch sweep from the singing note, 1.2 s, audible across the arena, with a 0.3 s LPF duck on everything else |

The ring-off starts on the line's last pitch, so the arena hears what class just lost a line. Rigger cutting a slack set line: barely a sound. Doubler at 1.8 s: the loudest sound in the game, by design.

Loud is not the same as dangerous. A normal cut recoils both ends by 150 N·s per 1,000 N (05 section 4.2), under the stagger threshold, so the snap is a sound and a lurch, nothing more. Only the Doubler's overspin snap carries an impulse to the far end, 1,500 N·s × (F / 1,200 N) (05 section 3.7), and only that snap gets the sub thump layer at full depth. The ear should learn the difference: a cut rings off; an overspin snap thumps.

Kinematics sever any line they pass through (05 section 3.5): a Hauler through a trench bridge, the Gantry load through a Gallows, the Maintenance Ring through a set line. The snap plays at the sever point at the line's tension, with a hard metallic shear transient from the kinematic layered on top. The owner, and anyone the line was attached to, also gets the HUD line-lost cue (ui-hud.md section 4.1).

### 2.7 The Kick

Per 05 section 3.2, a Jack begins with the Kick: the Spool dumps its capacitor into the line once, driving the ends apart to the class launch speed V within 0.3 s, then falls back to motor pull T. The Kick needs its own one-shot, distinct from the sustained Jack push, because it is the tell for a Slingshot, an Overboard, and a Scatter.

- **Sound**: a 40 ms capacitor charge whine (rising, 2 to 6 kHz), then a hard electrical crack with a sub thump and a filament whip transient, 120 ms, then a 150 ms discharge fizz from the Spool housing. Level and sub depth scale with class V: Climber (18 m/s) loudest and sharpest, Breaker (10 m/s) a duller thud. Audible 40 m. A self-dog Kick (Slingshot) is the same sound with a harness-strap crack layered on, so an enemy hears a rigger leaving the ground.
- **The line during the Kick**: the singing voice starts an octave above the class note and falls to the class note over 0.3 s as the capacitor hands off to the motor. A Haul rises to its note; a Jack falls to it. Veterans hear the mode from the direction of the glissando.
- **The push**: after the Kick, the sustained Jack is the class note, identical in pitch to a Haul. The Scrapper's 30% bonus applies to the push and to Haul acceleration, not to the Kick's V (05 section 6.1), so a Scrapper's Kick sounds like anyone's at 14 m/s.
- **Interlock (05 section 3.2):** no Kick when either Dog is in another player's Catch Plate. A Jack against a person plays no Kick one-shot; the line ramps to the class note over 0.3 s, a Haul in reverse. The absence is a tell: a Jack that cracks hit a wall or an object; a Jack that only sings hit a rigger. A self-dog Jack to a wall still Kicks.
- **One Kick per Jack.** A new Jack on the same line (release, re-engage) gets a new Kick. The sound is never repeated during a single push.

### 2.8 The Reel

Per 05 section 3.2, holding Haul in Hang shortens the rod at 1.0 m/s and holding Jack lengthens it, bearing the class Hang load. A winch, not a pull: slow, strong, and silent except for the ratchet.

- **Sound**: a quiet pawl-click loop, 8 clicks per meter (8 Hz at 1 m/s), dry and mechanical, 14 dB under the singing line, audible 10 m. Shortening clicks are tight; lengthening clicks are looser with a faint release hiss. Click pitch rises slightly with Hang load fraction; at 100% the slip creak (2.4) replaces the ratchet.
- **For the hoisted**: the victim of a Gallows hears the ratchet at the plate the Dog is in, spatialized, on the Tells submix with the line. The clicking at your back is the sound of going up.
- **Loop control**: starts on the first held frame, stops the frame the hold ends, no tail.

## 3. Dogs

### 3.1 Fire

Spool spin-up (40 ms whine, pitch set by class Dog speed: Scrapper highest at 160 m/s, Breaker lowest at 90 m/s) then a pneumatic thump. Dog flight: a short whir that Doppler-shifts, so a Dog passing the listener is heard. Dogs fired by the listener have an added mechanical click in the near field.

### 3.2 Seat, by surface

The seat is a gameplay tell (05 section 11: distinct seat clank, directional, lead 0 s). Each surface has its own seat so the player knows what they hit without looking.

| Surface | Seat sound |
|---|---|
| Steel (hull, rails, containers) | Hard clank, long metallic ring |
| Concrete | Dull thock, short, with grit |
| Wood, planks | Hollow knock, splinters |
| Rock | Sharp crack |
| Rope, cable | Twang, cable resonance |
| Dynamic prop (barrel, crate) | Surface seat plus a body resonance sized to mass tier (Light: tinny, Medium: drum, Heavy: boom) |
| Rated load | Surface seat plus a low structural note, so the player hears "rated" |
| Catch Plate | See 3.3 |
| Miss (falls as prop) | Dog clatters on whatever it lands on, then a 1 s rewind whir |

Dog cosmetics (04) may change the fire thump and the flight whir within a tuned band. They never change the Catch Plate seat.

### 3.3 Catch Plate seat

The most important half-second in the game. A Dog seating in a player's plate produces, for the victim:

- A **plate clank**: a hard, close, slightly ringing hit, played at the plate's position on the body, fully spatialized. Back plate seats are played 4 dB louder than chest, because the back is the kill plate and the player cannot see it.
- A **harness thud** felt through the whole rig: a non-spatial low layer, 60 Hz, 80 ms.
- The line starts singing from the plate immediately, so the singing voice is itself a direction cue, spatialized at the plate.

For everyone else: the same clank, spatialized at the victim, one tier louder than a steel seat, so a Spotter hears a teammate get dogged.

Self-dog: a softer seat plus a strap-tighten foley. It must not be mistaken for an enemy seat.

## 4. The Cutter

| Phase | Sound |
|---|---|
| Draw | Blade clear, 60 ms |
| Warm-up (0.12 to 0.30 s by class and model) | Rising thermal whine ending in a tick when cutting temperature is reached. The tick is the "ready" tell. Hot Cutter: shorter whine. Long: longer. |
| Swing (0.15 s) | Air cut, short |
| Cut | The snap (2.6), plus a filament hiss where the blade met line |
| Cut nothing | Air cut, nothing else. Silence is the feedback |
| Catch Plate hit (05 section 4.4) | Plate clang, a Dog unseat pop, no snap |
| Cable hold (Hangtown, 0.6 s) | Sustained hiss building to a strand-by-strand pop sequence, then the cable's own failure sound. This should feel like a transgression. |
| Recovery (0.30 s) | Blade cool tick |

Cutter cosmetics change handle foley only. The warm-up tick and the snap are universal.

## 5. The Stall sequence

Per 05 section 7.4.

1. Threshold crossed (0.0 s): impact sound, then a gel flood: a wet pressurized rush, 0.2 s, with a hard harness lock clunk. For the victim, a 200 ms low-pass sweep to 400 Hz on the full mix, then the mix returns over 1 s.
2. Scrap Card (0.2 s): LayCast card sting (section 11). Marshal calls it.
3. Body persists 4 s as a Medium prop: gelled thuds when moved. Riggers do this and it should sound faintly embarrassing.
4. Hook arrives at 4 s: rotors, a clamp, a 2 s lift with cable take-up.

## 6. Hooks and ring-outs

Hooks live below the lowest floor. Their rotors are the ambient floor of every arena except the Anchor. When a player falls past the lowest playable floor, the Hooks **spin up**: a rotor pitch rise over 0.5 s before the kill volume (05 section 7.5). For the falling player it is the last thing they hear. For everyone else it is a directional cue from below: someone is going down the Well.

The Anchor's Well: no Hooks (03; 05 section 7.4). The spin-up is replaced by the Stump's hum dropping an octave, 38 Hz to 19 Hz, felt more than heard, 0.5 s before the volume (05 v0.2.2), and nothing else. The drop is the point.

## 7. The Marshal

### 7.1 Character direction

One voice per match, drawn from a pool of six. Referees in an overhead gallery. Bored, fair, has seen everything. Never excited. Never cruel. Dry pacing, flat delivery, slight PA compression and a short room reverb from the gallery. Calls are short because the Marshal has been saying them for years. No bark exceeds a Hauler klaxon in length (04).

### 7.2 Callout list

| Trigger | Line |
|---|---|
| Drop begins | "Crews on the claim." |
| Lines enabled | "Live." |
| Scrap (impact) | "Scrapped." or "[Name], scrapped." |
| Scrap (Well) | "Down the Well." |
| Scrap (Anchor Well) | "Down the Well." Same call; a different tell. No Hooks spin up: the Stump's hum drops an octave 0.5 s before the volume (05 v0.2.2) |
| Scrap (crush) | "Pinned. Scrapped." |
| Scrap (blast) | "That was staged. Scrapped." |
| Reserve dropping | "Reserve." |
| Last reserve used | "Crew's out of reserves." |
| One rigger left on a crew | "One up." |
| Mid-match event, 5 s | "Event in five." then the arena call: "Shear." / "Shift change." / "Cable." / "Scaffold." |
| Pulse (Anchor) | "Pulse in three. Two. One. Brace." |
| Hauler approaching a player | None. The klaxon is the tell. |
| Rated load moving | "Rated load." |
| Overspin heard | "Fray line's hot." |
| Claim: marker taken | "Amber holds." / "Sky holds." |
| Claim: contested | "Contested." |
| Claim: marker moved | "Marker's moving." |
| Claim: marker in Well | "Marker's gone. Resetting." |
| Haul: spool moving | "Spool's moving." |
| Haul: delivered | "Delivered." |
| Round end | "Round." |
| Round end, time-out | "Time. Count." |
| Sudden death | "Sudden death." then the arena's own call: "Full shear." (Ballast), "Lockout." (the Yard), "Second cable." (Hangtown), "Held breath." (the Anchor). See game-modes-and-match-flow.md section 2.1 |
| Match end | "Claim's settled." |
| Forfeit | "Crew's forfeited. Logged." |
| Leaver backfill | "Office crew in." |
| Idle player, 20 s | "Office notes the rigger standing still." |
| Body used as prop | "Noted." |

Marshals never name a rig. LayCast does that in the kill feed.

## 8. Klaxons, sirens, structure

### 8.1 Kinematics (05 section 11: klaxon, then lights, 3 s Hauler, 2 s Gantry)

| Arena | Element | Sound |
|---|---|---|
| Yard | Hauler | Two-tone klaxon at 3.0 s, rail hum rising, wheel squeal on the curve. Audible 80 m. |
| Yard | Gantry drop | Magnet release warning: a single long horn at 2.0 s, then the magnet's hum cutting out. |
| Yard | Shear Press | Hydraulic ram cycle, audible everywhere, the Well's heartbeat. |
| Ballast | The Breathing | 90 s cycle: hull groan rising on the lean, chains swinging, water sloshing in the Hold. The groan peaks 1 s before the lean's maximum. |
| Hangtown | The Sway | Cable creak and house timber in overlapping slow arcs. |
| Hangtown | The Lift | Pulley clatter, counterweight rumble through the Mid Hang. |
| Anchor | Updraft | A 40 s cycle: a rising rush from the Well, 1 s before the gust. |
| Anchor | Maintenance Ring | Gear drive, constant, Doppler as it passes. |
| Anchor | Pulse | 3 s Marshal count plus a sub-bass swell from the Stump, then the shove. |

### 8.2 Mid-match event sirens (5 s lead)

One siren per arena, distinct so veterans know the map by ear: Ballast, a ship's horn; the Yard, a shift bell; Hangtown, a hand-cranked air-raid siren; the Anchor, the Office's electronic klaxon. All 5.0 s, all peaking at the top of the mix.

### 8.3 Structural groans

Every rated load and every scripted collapse groans **1.0 s minimum** before the consequence (05 section 11). Groans are per material: steel (long descending moan), concrete (crack and grind), timber (creak and crack), cable (twang and whine). The groan is placed at the element that will fail, not at the load being pulled.

## 9. Bodies and harnesses

### 9.1 Footsteps by mass

Five classes, three weights of footstep layer: Climber (90 kg) light and quick; Scrapper (110 kg) light with loose-plate rattle; Rigger and Doubler (130 kg) medium; Breaker (220 kg) heavy with a floor-resonance layer. Surface sets: steel, grate, concrete, wood, rope-walkway, scrap, water. A Breaker on a plank bridge should worry the player on the bridge.

### 9.2 Harness creak

Constant low foley bed per class on movement: canvas and brass (Rigger), machined plates (Breaker), minimal wraps (Climber), mismatched rattle (Scrapper), heat-scored housing tick (Doubler). The Doubler's Spool ticks faster after an overspin for 3 s.

### 9.3 Drag, airborne, hoisted

Drag skid: boot scrape by surface, rising with speed. Airborne: wind, rising to a roar at terminal velocity (50 m/s). Hoisted: harness strain creak plus the line singing at the chest, close.

## 10. Ambient beds

| Arena | Bed |
|---|---|
| Ballast | Wind through the open Crown, hull ticking, seep-water drip in the Hold, distant cutting torches from HTR's crew, the Hooks below the nose. |
| The Yard | Press cycle, rail hum, floodlight buzz, billet handling in the distance. The Yard does not stop. |
| Hangtown | Wind in cable, timber, laundry flapping, a radio left on in an evacuated house, mist. |
| The Anchor | The Stump's hum (a 38 Hz fundamental riggers claim to feel in their teeth), sea wind, broadcast tower electrical hiss. No Hooks. |

Beds sit 18 dB under the tells and duck a further 6 dB when any line sings within 10 m.

## 11. Music

**Stance: no music in Live.** From "Live" to "Round," the arena is the score: lines, groans, klaxons, the snap. Music masks tells and the tells must win.

Music plays:

- Main menu and Ledger: sparse, dry, industrial. Tape loops, metal percussion, no melody that insists.
- Lobby: LayCast's broadcast bed, upbeat, a little kitsch, sponsor-adjacent.
- Drop (6 s): a LayCast opening sting, cut dead on "Live."
- Round end: a 3 s sting under the victim-camera replay, then silence under the Scrap Card.
- Post-match: LayCast closing theme over page 1, nothing over page 2 (the Office does not have music).
- Sudden death: **PROPOSED** a single sustained drone, 1 note, 12 dB under, to mark the state. Cut if playtests show it masks anything.

## 12. Mix priorities

Ranked. A higher row ducks lower rows when they compete.

1. Catch Plate seat on the listener, and lines seated in the listener (singing voice).
2. The snap.
3. Overspin octave jump within 20 m.
4. Hooks spin-up (listener falling).
5. Mid-match event siren.
6. Kinematic klaxons and structural groans within 30 m.
7. Marshal voice.
8. Enemy lines singing within 10 m.
9. Volatile hiss (0.3 s before blast), break-free lurch, the Kick within 20 m.
10. Blasts, fractures, impacts.
11. Own crew lines, set-line hums, the Reel ratchet (rank 1 when the listener is the hoisted body).
12. Rig Calls and barks.
13. Footsteps, harness, foley.
14. Ambient beds.
15. LayCast stings.
16. Music.

Items 1 through 6 are the readability contract from 05 section 11. They are never ducked by anything below them, and they sidechain the full bus.

## 13. MetaSounds architecture

- **MS_LineVoice** (one instance per audible line, pooled, max 6 plus 1 chord bed): inputs Force_N, HangLoadRatio, Mode, Class, Overspin_t, Team, Length_m, ListenerAttached (bool), Distance. Internally: pitch map node (section 2.1), class timbre preset, overspin octave switch at 1.0 s, tremolo, slip creak layer. Outputs to the Tells submix when ListenerAttached, else to the Lines submix.
- **MS_Snap**: one-shot, input Tension_N, LastPitch_Hz, IsOverspinSnap, SeveredByKinematic. Drives crack layer gain, ring-off pitch, the kinematic shear layer, and a sidechain trigger.
- **MS_Kick**: one-shot, input Class_V, IsSelfDog, Distance, Interlocked (bool: no play when either Dog is in a player's plate). Drives the crack level, sub depth, and the strap layer; triggers MS_LineVoice's octave-above start.
- **MS_Reel**: loop, input Direction, Speed_mps, HangLoadRatio, ListenerAttached. 8 clicks per meter; stops on release; routes to Tells when the listener is the hoisted body.
- **MS_DogSeat**: input SurfaceType, MassTier, PlateIndex, IsListener. Selects the seat layer and plate loudness offset.
- **MS_Cutter**: input Warmup_s, Model. Drives the thermal whine length and the ready tick.
- **MS_Structure**: input Material, Lead_s (never under 1.0). Groan sustain is Lead_s minus 0.2.
- **MS_Kinematic**: per Hauler, Gantry, Ring, Lift. Spline-driven position, klaxon scheduler keyed to the 05 lead times.
- **MS_Marshal**: dialogue queue with priority; a new call replaces a lower-priority call in progress.
- **MS_Ambience**: per-arena bed with a Tilt or Sway parameter from the kinematic root.

Submix tree: Master > [Tells, Voice, World (Lines, Physics, Kinematics, Foley), Ambience, Broadcast (Stings, Music)]. Tells sidechains World by 6 dB and Ambience by 12 dB. The 6-voice line pool is managed by a priority component reading section 2.5.

## 14. Loudness targets

| Bus | Target |
|---|---|
| Full mix, integrated, a typical match | -23 LUFS, true peak -1 dBTP |
| Marshal voice, short-term | -20 LUFS |
| Catch Plate seat on listener, momentary | -14 LUFS |
| Full snap at 1,200 N, momentary | -10 LUFS (the loudest event) |
| Singing line at class T, 3 m | -24 LUFS |
| Ambient beds | -42 LUFS |
| Music, menus | -26 LUFS |
| Dynamic range presets | Full, Night (compressed, tells preserved), Headphone HRTF |

## 15. The broadcast layer

LayCast stingers: Drop, round end, Scrap Card, Claim taken, mid-match event name card. Short, brass-and-synth, slightly embarrassing, each under 2 s. Sponsors get audio idents only in the lobby and post-match ("Stall-Gel by Nurri" is a three-note jingle; riggers are sick of it).

**Crowd: none in the arena.** Decision and justification: the arenas are evacuated demolition sites. The gallery holds Marshals. Four billion people watch remotely. A crowd bed would be a lie about the space, and worse, a broadband mask over the tells, which section 12 forbids. Crowd sound exists only as a distant broadcast-feed layer under the round-end replay and the post-match page 1, heard as if through a stadium PA from the Rim, at 20 dB under. The exception is the Anchor's Rim, where the Office's gallery and LayCast towers are on the map: a faint, filtered broadcast murmur plays near the towers only, and dies when a tower comes down with its lights.
