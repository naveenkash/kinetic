# KINETIC: In-Match HUD (the LayCast Package)

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 05 sections 3, 4.2, 7.4, 9.2, 11. Numbers in 05 are canonical. Anything marked **PROPOSED** is new here._

## 1. Stance

The HUD is LayCast's broadcast graphics package, seen from inside the harness. Every element must be justified as something the broadcaster would draw for four billion viewers, or something the Office requires the harness telemetry to show the rigger. Nothing else is allowed on screen.

Three rules:

1. **The line is the readout.** Tension, mode, ownership, and danger are read off the line itself (brightness, pitch, pattern, color). The HUD confirms; it does not replace.
2. **The HUD never draws a line.** Per 05 section 4.2, a line on your body is marked by direction only. The player must turn and find it.
3. **Nothing on the HUD may out-glow a line.** Maximum HUD element luminance is 70% of a line at 100 N. Peak alerts (siren banner, Stall) may hit 85% for under 0.5 s.

Camera: the HUD is identical in first and third person. Third person is casual only and locked at 90° FOV (05 section 2.1); Rated play is first person.

Coordinates below are screen-space percentages, origin top-left, (x, y). Sizes are percent of screen height unless marked w (percent of width). Safe area is 2% inset on all sides. All elements scale with the HUD scale option (80% to 120%).

## 2. Team tints

Tints are assigned by LayCast per match, not by faction. Faction colors live on livery; crew tints live on lines, markers, and HUD. Line glow color is team-locked (bible 04, Visibility Clause).

| Role | Name | Hex | Notes |
|---|---|---|---|
| Your crew | Amber | `#E69F00` | Always your crew in your own view |
| Enemy crew | Sky | `#56B4E9` | |
| Third crew (3v3v3, post-launch) | Rose | `#CC79A7` | |
| Neutral / Office | Bone | `#E8E6E0` | Marshal text, markers, kinematic warnings, untinted UI |
| Hazard | Caution | `#F2C200` on `#111111` | Rated-load tape, siren banner, Hauler lights. Always paired with the stripe pattern |
| Stall | Gel | `#8FE3CF` | Only used for the Stall flood and Scrap Card frame |

These tints are canonical in 05 section 11a; any other team tint proposal is superseded. They are the Okabe-Ito orange, sky blue, and reddish purple, chosen to separate under deuteranopia, protanopia, and tritanopia. Also canonical: enemy lines carry a dark 4 cm tick every 1 m along their length; own lines are unbroken. Third-crew lines (post-launch) carry a double tick. Tints are therefore never the only channel.

Option "Absolute tints" (default off): Crew 1 is always Amber, Crew 2 Sky, Crew 3 Rose, regardless of which crew you are on. Spectator and broadcast views always use absolute tints.

## 3. Main HUD wireframe

```
+------------------------------------------------------------------------------+
| CREW                         [ 2 ]  01:47  [ 1 ]              [feed]  Hitch  |
| > Rigger  YOU       o                                 hung Wreckingball      |
|   Climber Swift     o           ~~~ SHEAR IN 4 ~~~    Kite scrapped Surveyor |
|   Breaker Rook   [reserve 6]                           barrel 1,215 N.s      |
|                                                                              |
|                                                                              |
|                                                                              |
|  <                                                                           |
|  <                           ( . )  |                                        |
|  <                          \_ _ _/ |   <- tension arc, overspin bar         |
|                                   [cutter]                                   |
|                                                                              |
|                                                                              |
|                                                                              |
|                     MARSHAL: Live.                                           |
|                                                                              |
|                      . .   (set lines)                                       |
|                [HAUL] [JACK] [HANG]                                          |
|                                                            ^ ^ ^             |
+------------------------------------------------------------------------------+
```

Left-edge chevrons (<) are the line-on-body edge indicator pointing to a left-shoulder plate. Bottom-right chevrons (^) would be a back plate.

## 4. Elements

### 4.1 Crosshair

Position (50, 50). An open ring, 1.2 diameter, 2 px stroke at 1080p, center dot 3 px. Bone by default.

| State | Visual |
|---|---|
| No line, Dog A loaded | Ring with one tick at 12 o'clock |
| Dog A in flight | Tick leaves the ring in the flight direction, 0.1 s |
| A seated, waiting for B | Ring splits into two half-arcs. Left half lit Amber (A seated). Right half Bone. A thin drain runs clockwise around the outside over the 4 s auto-recall window (05 section 3.1) |
| A and B seated, line live | Both halves Amber. Ring stroke doubles for 0.15 s on seat |
| A missed (fell as prop) | Left half flickers Bone and empties over the 1 s auto-recall |
| Over a dog-able surface in range | Ring expands 20% |
| Over a Catch Plate | A plate glyph appears below the ring: chest, back, L, R. 0.8 high |
| Over a rated load | Four short Caution ticks outside the ring |
| Dog will not seat here (Mag on concrete, Spike on steel) | Ring hollow with a diagonal slash |
| Out of range (beyond class line length) | Ring at 50% opacity |
| Recall | Ring collapses to the dot over the class recall time (0.4, instant, 0.6, 0.6, 0.9 s) |
| Line severed by a kinematic (a Hauler, the Gantry load, the Maintenance Ring passes through it, 05 section 3.5) | Ring flashes Bone once; a LINE LOST tag, 1.6 high, at (50, 60) for 1 s, with the snap. Plays for the owner and for anyone the line was attached to |

Doubler: a second, smaller ring (0.8 diameter) sits 0.3 outside the first, showing the second line's A/B state. A bracket marks which line currently has mode focus.

### 4.2 Mode indicator

Position (50, 93). Three chips, HAUL, JACK, HANG, each 4.5 w by 2.2 high, 0.4 gap. Bone text on 40% black. Active mode: chip filled Amber, text black.

- **Pre-selected mode** (before a line exists): active chip outlined Amber, not filled.
- **Switch delay**: on input, a wipe crosses the new chip left to right over the class switch time (Climber 0.20, Scrapper 0.15, Rigger 0.25, Doubler 0.25, Breaker 0.35 s). The old chip empties at the same rate. Force does not change until the wipe completes.
- **Radial** (when open): the three chips lift to a 12-high ring around the crosshair at 10, 2, and 6 o'clock. Release selects. Radial closes in 0.1 s.
- **Doubler**: two stacked rows, 1.6 high each, labeled 1 and 2. The focused line's row is at 100% opacity, the other at 60%.
- **Jack, the Kick** (**PROPOSED**): when Jack engages, the JACK chip flashes Bone for one frame and a thin bar under it drains full to empty over 0.3 s: the capacitor discharging (05 section 3.2). The bar never appears when either Dog is in a player's Catch Plate (the Kick interlock, 05 section 3.2): a Jack against a person shows a plain 0.3 s ramp wipe like Haul, and no flash. The bar stays empty for the rest of that push (one Kick per Jack) and refills the instant a new line exists, so the player can read "Kick spent" against "Kick ready."
- **Reel** (Hang only, 05 section 3.2): while Haul or Jack is held in Hang, HANG stays the active chip and the held chip shows a ratchet arrow, inward for Haul (shorten), outward for Jack (lengthen). A rod-length readout appears at (50, 87), 1.6 high: current length in meters to one decimal, counting at 1.0 m/s, with the class Hang load fill as a thin bar beneath it that jitters at 100% (slip). The readout fades 0.5 s after the hold ends.
- **Line unavailable** (3 s after a snap): all chips hatched with a 3 s drain.

### 4.3 Tension arc

Position centered (50, 54). A 120° arc, 6 diameter, under the crosshair. Fills left to right from 0 to class T. Fill color is Amber at the same luminance ramp as the line emissive (see vfx.md). The arc is deliberately small; the player should be reading the line.

- Haul: arc fills over the 0.4 s ramp, then holds at T.
- Jack: the arc slams to full on the Kick frame and overshoots past the T mark for 0.3 s (a Bone tip beyond the arc's end, the capacitor), then settles back to T for the push. With a Dog in a player's plate the arc fills over 0.3 s with no overshoot (interlock).
- Hang: arc shows load against Hang load. At 100% the arc jitters 1 px at 10 Hz (slip tell, 05 section 11).
- Slack (A seated, no B): arc empty.
- Numeric newton label appears only with the "Numeric readouts" option.

### 4.4 Overspin meter (Doubler only)

Position (54, 50). Vertical bar, 0.6 w by 8 high, right of the crosshair. Fills bottom to top over 1.8 s while the mode input is held. A hard mark at 55.6% (1.0 s): above it the bar strobes white at 8 Hz, matching the line flicker. At 100% the bar flashes once and the chips go to the unavailable state. The meter resets on release and keeps a ghost mark at the released level for 1 s so the player can see what they earned.

### 4.5 Set-line timers

Position (50, 89), above the mode chips. One pip per class set-line slot (two for Rigger), 1.0 diameter. A set pip drains radially over the class life (8 s, Rigger 12 s). Drained pip fades over 0.5 s, matching the line's fade. Picking up a set line (Scrapper recall) empties the pip instantly. Every pip is also a recall affordance (05 section 3.6: owners recall set lines early; that is how a Guillotine drops): the oldest set line's pip carries a small recall glyph; pressing Recall with no live line recalls it, and when two set lines exist, holding Recall for 0.3 s expands the pips for a pick (**PROPOSED**). A recalled pip collapses instantly. A set line attached to your own body (a Gallows that was Hung, not yet Hauled) also shows the edge indicator at 60% opacity (see 4.6).

### 4.6 Line-on-body edge indicator

This is the panic-window UI, per 05 section 4.2 and 5.5. It shows **direction only**, only while the line is **outside the 70° cut cone**, and never draws the line.

Visual: a stack of three chevrons, 2.5 high, in the line owner's tint, placed on the screen edge in the direction the camera must turn. Pulses 2 Hz. Opacity scales with the line's current force: 50% at 0 N, 100% at class T. A thin 0.2-high bar inside the chevrons shows the force fill.

| Plate | Where the chevrons go | Behavior |
|---|---|---|
| Chest | No edge indicator. The line is on screen. A 1.5-wide bracket sits at the line's root point on screen, in tint, for 0.5 s after seat, then fades to 30% | Bracket, not chevrons. Reinforces that chest lines are already visible |
| Left shoulder | Left edge, at (2, 50) | Turn left 60° to 90°. Chevrons slide vertically with the line's pitch angle (upward Gallows: chevrons rise toward (2, 20)) |
| Right shoulder | Right edge, at (98, 50) | Mirror of left |
| Back | Bottom-center (50, 96) plus the side edge with the shorter turn | 150°+ turn. The side chevrons appear on whichever side gets the line into the cone fastest. If the line is directly behind, both sides show at 50% and the bottom at 100% |

Transitions:

- Seat: chevrons appear at frame 0 with the seat clank (lead 0 s).
- As the camera turns, chevrons move along the edge toward the line's screen-space entry point.
- The moment the line enters the 70° cone, the chevrons **vanish** and the Cutter glyph (4.7) gains a ring. From here the line itself is the target.
- Cut, release, or snap: all indicators vanish on the same frame.
- Severed by a kinematic: indicators vanish on the same frame and the LINE LOST cue (4.1) plays.

Multiple lines on body (two enemies, or a Doubler's pair): each gets its own chevron stack. Maximum three; a fourth is merged into the nearest.

Hang-set line on body: 60% opacity, no pulse, because it is applying no new force. It pulses the instant a Haul engages.

Accessibility: option "Edge indicator size" 100% to 200%. Option "Indicator audio" (default on) adds a directional tick in the indicator's direction at 2 Hz, in addition to the line singing.

### 4.7 Cutter glyph

Position (53.5, 53). A blade glyph, 1.4 high, Bone.

| State | Visual | Duration |
|---|---|---|
| Ready | Solid | |
| Warm-up | Glyph fills bottom to top | Class and Cutter warm-up (0.12 to 0.30 s) |
| Swing | Glyph flashes white once | 0.15 s |
| Recovery | Hollow, refills | 0.30 s |
| Line on body in cone | Solid plus a ring in the line's tint | While in cone |
| Cable hold (Hangtown) | Ring fills over 0.6 s | 0.6 s |

Warm-up and recovery share one glyph so the player learns a single rhythm.

### 4.8 Stagger, flip, hoisted

- **Stagger** (0.6 s, 1.0 s on a Rigger's line, 0.8 s after a snap, 0.3 s from a Cutter hit): mode chips desaturate to grey, crosshair ring thickens to 4 px, a 6% vignette pulse. Per 05 section 7.1, stagger locks movement and Spool only; look and the Cutter stay live, so the edge indicators and the Cutter glyph stay live at full opacity. Stagger never hides the panic-window UI.
- **Flip**: all HUD hides except crosshair, edge indicators, Cutter glyph, and a horizon tick at (50, 50) that stays world-level so the player keeps orientation. Restores on landing.
- **Hoisted**: a tag HOISTED at (50, 62), 1.6 high, Bone on 40% black, plus a small up-arrow glyph. The Cutter glyph pulses 2 Hz. Mode chips remain live (hoisted players can fire Dogs, 05 section 5.2).
- **Drag** (feet still down, skid): a ground-tick strip at (50, 58) shows the skid direction. Vanishes when airborne.

### 4.9 Crew status

Position (2, 3), stacked rows 2.4 high, 14 w. Per teammate: class silhouette icon (1.8 high, must read mass: Breaker wide, Climber narrow), name, state, and their top Rope Mark tape color as a 0.4-wide bar.

| State | Visual |
|---|---|
| Alive | Icon Amber, dot |
| In a live line (any mode) | Small line glyph next to icon |
| Line on their body | Enemy-tint chevron on their row, pulsing. Mirrors their own panic-window UI so a Spotter can call it |
| Staggered or hoisted | Icon flickers |
| Scrapped, no reserve (Scrap) | Row at 40%, icon crossed |
| Scrapped, reserve counting (Claim, Haul, Free Scrap) | Radial drain over the reserve timer (8 s, Free Scrap 10 s), label "reserve 6" counting down, Kessl Harness Leasing mark at 20% opacity |
| Reserves remaining (Claim) | Three small pips under the crew block, one per crew reserve |

Your own row is first and bold.

### 4.10 Round timer and score

Position (50, 3). The LayCast bug: a 16 w by 4 high plate, Bone text on 60% black, slight bottom bevel.

- Center: round clock, mm:ss, 2.4 high. Last 10 s of a round: digits Caution, tick each second.
- Scrap: round-win pips either side, three per crew (best of 5), tinted. Riggers remaining per crew shown as small silhouettes under the pips.
- Claim: two bars (3v3v3, post-launch: three) under the clock, 0 to 100, tinted. The holding crew's bar fills 1 per second, with a thin moving tick so the fill is visibly alive. Contested: both bars hatched, no fill.
- Haul: one track under the clock. A spool glyph slides along it; truck glyphs at each end in crew tints. The round 1 best mark is kept as a ghost tick in round 2.
- Free Scrap: your scraps and the leader's scraps, left and right.

### 4.11 Claim marker

World-space: a 4 m radius ring on the ground around the billet, 0.1 m wide, in the holding crew's tint, Bone when empty, hatched Amber and Sky when contested. A vertical beacon 1 m high rises from the billet. The marker sits on polished runners and moves, slowly, under a Rigger's line or heavier (05 section 6.1a); when it moves, the ring follows the billet. HUD-space: a marker glyph at the marker's screen position with distance in meters when off-screen (edge-clamped, 1.6 high). Marker in a Well: glyph flashes, then "RESET" for 2 s.

### 4.12 Haul spool

World-space: the 2,000 kg spool gets a Bone outline readable through walls at 20% opacity (Office telemetry). HUD: the track in 4.10 plus a distance-to-truck readout under the crosshair when your line is on the spool. The spool sits on rollers (05 section 6.1a): any class can start it alone, and three lines add as vectors, so a small 1, 2, or 3 lines tag sits beside the spool glyph on the track (**PROPOSED**) to show how much of the crew is pulling.

### 4.13 Mid-match event banner

Position (50, 14). 30 w by 4 high, Caution stripes on the left and right 3 w, black center, Bone text 2.2 high: "SHEAR IN 5", "SHIFT CHANGE IN 5", "CABLE DAY IN 5", "SCAFFOLD IN 5". Counts down the 5 s siren (05 section 9.2). The event fires at 1:15 remaining in a Scrap round, 4:00 remaining in Claim, 2:00 remaining in each Haul half, and 3:00 remaining in Free Scrap. At 0 the banner slams to the event name for 1 s, then collapses to a 6 w tag beside the clock for the rest of the round. Pulse countdown on the Anchor uses the same banner at half height: "PULSE 3, 2, 1, BRACE."

### 4.14 Marshal callouts

Position (50, 80). Subtitle, 1.8 high, Bone on 50% black, prefix "MARSHAL". Each callout holds 2 s. Full line list in audio.md. Callouts never stack; a new one replaces the old.

### 4.15 Kill feed

Position (98, 8), right-aligned, rows 1.8 high, max 4 rows, 6 s each, newest on top. Written in trade language, never "killed":

```
Hitch           hung  Wreckingball            Gallows
Kite        scrapped  the Surveyor      barrel 1,215 N.s
Swift          Well   (Overboard, the Verger)
Fuse         snapped  himself              overspin
Mother Line     cut   Snap's line
```

Names in tint. Rig names appear when the Rope Mark system recognizes the rig. "Cut" entries are shown because a cut is a match event on LayCast; they stay 3 s instead of 6. Spotter credit shows as a second name in brackets: "Hitch [Mother Line]".

### 4.16 Scrap Card

Per 05 section 7.4 and 9.2. Two views.

**Killer view**: position (82, 74), 18 w by 14 high, 3 s. Faction template (04 cosmetics). Shows the victim's name, plate, cause, impulse or speed, your Tonnage, and Spotter credit if any. Sponsor mark: Stall-Gel by Nurri, 10% opacity, bottom edge.

```
+----------------------------------+
| SCRAPPED              Nurri gel  |
|  the Surveyor        BREAKER     |
|  back plate . Caboose            |
|  Hauler impact  11.2 m/s         |
|  +640 t    Spotter: Hitch +410 t |
+----------------------------------+
```

**Victim view**: full lower-third at (50, 72), 50 w by 16 high, held through the 4 s replay from the victim's camera (05 section 9.2, round end) or 3 s in-round. This card teaches the panic window.

```
+------------------------------------------------------------------+
|  SCRAPPED by Wreckingball          Meridian Mutual, insuring the  |
|                                     Lay since the Lay             |
|  Line seated: BACK plate      Window: 1.6 s                       |
|  You turned 110 of 150 degrees. Cutter warm 0.17 s, swing 0.15 s. |
|  Cause: Hauler impact 11.2 m/s (Stall at 9 m/s vs kinematic)      |
|  [ down the Well variant: "Fell 38 m. Hooks dispatched." ]        |
+------------------------------------------------------------------+
```

The degrees-turned readout is the single most important number on the card. It tells the player whether they were slow to find the line or slow to cut.

Ring-out variant: title "DOWN THE WELL", height fallen, Meridian Mutual mark. Anchor Well variant: no Hooks line; the card reads "Retrieval: by other means."

### 4.17 Rig Calls (ping wheel)

Hold ping opens an 8-slot radial, 22 diameter, at screen center. Flick and release. Tap ping sends the contextual call (whatever the crosshair is over). Rig Calls are drawn from real rigging hand signals; each has a world-space marker and a bark.

| Slot | Call | Meaning | Marker |
|---|---|---|---|
| 12 | All stop | Hold. Do not engage | Open palm glyph on your position, 3 s |
| 1:30 | Slack | Release or give line. Stop pulling | Glyph on the target, 3 s |
| 3 | Take up | Haul it. Tighten | Glyph on the target, 3 s |
| 4:30 | Dog in | Attach here. Mark an anchor | Pin on the surface, 8 s |
| 6 | Load here | Mark an object worth moving | Pin on the object, follows it, 8 s |
| 7:30 | Edge | Danger: drop or incoming kinematic | Caution pin, 5 s |
| 9 | Line on me | I am tethered. Cut me or Jack me | Pulsing chevron on you, until cut |
| 10:30 | Cut it **PROPOSED** | Cut that line (set line or enemy line) | Pin on the line's nearest Dog, 5 s |

"Line on me" fires automatically if a line stays on your back plate for 1.0 s and the option "Auto call" is on (default on). Rig Call cosmetics (04) change the hand animation and bark voice, never the marker shape.

### 4.18 Spectator and broadcast overlay

| Element | Player | Spectator | Broadcast |
|---|---|---|---|
| Tints | Relative | Absolute | Absolute |
| Edge indicators | Own only | Shown for the followed player, at 50% | Hidden; the broadcast shows the line instead, with a tint outline |
| Tension arc | Own | Followed player | Per-line newton tag floating at the line midpoint, 1.6 high |
| Crew status | Own crew | Both crews, left and right | Both crews, with Face portraits and Grade |
| Kill feed | Yes | Yes | Yes, larger, with Rope Mark tape |
| Scrap Card | Killer and victim | Victim view | Victim view plus killer portrait |
| Marshal text | Yes | Yes | Yes |
| Mode chips | Own | Followed player's | Hidden; mode is read off the line pattern |
| Sponsor marks | 10% opacity | 20% | Full LayCast lower-thirds, pre-round segments |
| Free cam | No | Yes | Yes, plus authored rails |

Broadcast overlay adds a "line count" tag per crew (live lines plus set lines) and a Tonnage ticker on the bottom edge.

## 5. Accessibility options

| Option | Range | Default |
|---|---|---|
| HUD scale | 80% to 120% | 100% |
| Edge indicator size | 100% to 200% | 100% |
| Indicator audio tick | on, off | on |
| Numeric readouts (N, m/s) | on, off | off |
| Tint palette | Standard (Amber, Sky, Rose), High contrast (Amber `#E69F00`, Blue `#0072B2`, Rose `#CC79A7`), Mono (Bone with patterns only) | Standard |
| Absolute tints | on, off | off |
| Line tick pattern | on, off | on |
| Marshal subtitles | on, off, large | on |
| Siren banner flash | full, reduced (no stripe animation) | full |
| Stall vignette | full, reduced | full |
| Camera shake | 0% to 100% | 100% (see camera-and-feel.md) |
| Crosshair color | Bone, Amber, custom | Bone |
| Kill feed | full, short (3 s), off | full |
| Hold to ping / toggle | hold, toggle | hold |

No option may hide the edge indicator, the Cutter glyph, or the siren banner. Those three are the readability contract.
