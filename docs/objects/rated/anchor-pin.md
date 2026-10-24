# Anchor Pin

One-line: rated load, structural, 4,000 kg, a 0.5 m diameter x 3.2 m steel pin through a cast anchor head, 2.0 m x 1.6 m x 1.4 m above the deck, Hangtown Collar rims: eight anchor heads (four per rim), one per main load cable, with one live pin per rim per round.

## Role in play

An anchor pin is the only rated load in Hangtown that a Breaker can reach on solid ground. Pull it and the main load cable it holds goes slack, the cable falls into the ravine from the rim end, and the walkway run hung on it folds down its whole length (03: "a Breaker who pulls one drops a whole cable's worth of walkway"). The houses hung from that cable's hangers swing free. It is Cable Day on demand, for one cable, and it is the HTR play that Hangers have nightmares about.

| Harness | Relationship |
|---|---|
| Climber | The pin's cable is a Climber's road along the top. A Breaker at the anchor head means a Climber leaves that cable. |
| Scrapper | Cannot move it. Keeps a propane cylinder from the prep store next to the live pin as a deterrent: the Kiss on a committing Breaker. |
| Rigger | A Clothesline between the anchor head and the headframe at 1.2 m stops a Breaker's approach for 12 s. |
| Doubler | Overspin from the first walkway section, 20 m out: commits the pin and drops the walkway they are standing on. The Fray does this on stream for the clip. |
| Breaker | Owner. Approach along the rim, brace at 5 m, Haul the pin head along the cable axis. The pull is away from the ravine. |

## Appearance

Silhouette: a cast steel anchor head bolted to the rim deck, with the main cable's socket entering it from the ravine side at 20 degrees and the pin passing through both cheeks horizontally. The pin's exposed end carries a keeper plate. Yellow-and-black tape around both cheeks and a tape cross on the keeper plate.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Anchor head | cast steel, Guild ochre primer under Office grey | #B8752A under #6E7378 | 0.6 | 0.8 |
| Pin | forged steel, greased | #3C3F42 | 0.3 | 1.0 |
| Keeper plate | steel, four bolts (two cut on live) | #8A8F93 | 0.5 | 0.9 |
| Cable socket | poured zinc socket, 90 mm cable | #A8ADB1 / #6E7378 | 0.45 | 0.9 |
| Deck bolts | 12 x M48, rusted | #7A3B1E | 0.8 | 0.6 |
| Tape | Office | #F2C500 / #141414 | 0.7 | 0.0 |

Weathering: the heads were cast in L.4 by the Guild from Collar scrap and show casting flash and hand-ground fettling. Grease on the pin has collected sixty years of grit. The keeper plates on live pins have two bright cut bolt faces. Markings: `VILCA'S HANG  CABLE 3  L.4  ROSA VILCA` cast into the head (the grandmother; the heads are the oldest objects on the map). HTR orange `CUT 03 SEQ A` sprayed over it on the live pin. A chalk cross and the word `NO` on the live pin in a child's hand. Lighting: the Office work lamp on the tape; the rim is otherwise dim.

Read by tier: from the far rim, a lit dot at the cable's root. From the Mid Hang, invisible; what you see is the cable above you and the walkway on it. From the rim, a knee-high head with a pin you could put your arm through.

## Variants

1. **Live.** Two keeper bolts cut, taped, lamp. One per rim.
2. **Dead.** Four bolts, no tape, no lamp.
3. **Pulled (seasonal cut district).** Empty head, pin lying beside it, cable gone over the edge. Permanent dressing when HTR holds the Hangtown Claim.
4. **Cable Day head.** The anchor head of the cable that fails in the event: identical to Dead but its socket is modeled frayed and its sound stem starts the 5 s siren.

## Physics

- Tier: Rated. Mass 4,000 kg (pin plus head; the pin alone is 500 kg but it does not move without the head's keeper failing). Break-free 1,000 N: a Breaker alone, a Doubler at overspin, or any lines summing to 1,000 N along the pin's axis (a crew; 05 section 6.2). The 1,000 N is a 0.6 s commit trigger; after it the authored timeline carries the pin and cable regardless of friction or line.
- Commit: 1,000 N sustained 0.6 s along the pin's axis (within 45 degrees of pulling it out of the head, away from the ravine). A pull toward the ravine shudders and does not commit.
- Pin withdrawal: 1.2 s after release (the cable's tension drags the pin against the cheeks; the authored sequence shows it walking out). Then the cable socket leaves the head at the cable's stored tension: the socket and 2 m of cable whip across the rim deck at 20 m/s (05 section 7.1: object hits player, 300 kg x 20 m/s = 6,000 N s, scrapped) along an authored 8 m arc that ends in the ravine. The arc is marked by a painted sector on the deck.
- Cable fall: the main cable (see `environment/load-cable.md`) falls from the rim end; the far end stays anchored at the opposite rim. The walkway run folds from the near end at 10 m per 0.3 s (see `structures/walkway-on-cable.md`): a 180 m run takes 5.4 s to finish falling. Houses hung from this cable's hangers become free pendulums on their remaining slack cables along pre-authored swing paths (as Cable Day).
- Players on the walkway: those within 10 m of the rim have 1.0 s of groan plus 1.2 s of pin walk to get off. Those 100 m out have 5.2 s. Everyone on the run eventually falls 12 m to the Mid Hang (15.3 m/s) unless they Hang to a house or another cable, which is the intended counter.

## Authored consequence

| Time | Event |
|---|---|
| 0.0 s | Commit. Lurch: the head rocks 20 mm on its deck bolts, keeper plate bolts shear. |
| 0.0 to 1.0 s | Groan, 1.0 s lead. The whole cable sings down its length: a 180 m long rising tone that everyone on the Upper Hang hears directionally. Walkway strips flicker along the run from the rim end. Marshal: "Cable three's anchor is moving." |
| 1.0 s | Release. Pin begins walking out. |
| 2.2 s | Pin clears the cheeks and drops on the deck (500 kg Heavy prop, static afterward). Socket whips across the painted sector and over the edge. Cable falls. |
| 2.2 to 7.6 s | Walkway folds down its length. Houses on the cable swing. The falling cable is kinematic on its spline and severs every line it passes through (05 section 3.5): Gallows and Pendulums hung from it, Hang footbridges under it. The far rim's anchor head takes the shock with a bang audible on both rims. |
| After | A dead cable hangs in a catenary into the ravine from the far rim. The walkway is gone. Two to three houses swing on slack cables, now cuttable at both ends. The rim deck has a 500 kg pin lying on it: Heavy on a bare floor, fixed scenery per 05 section 6.1a, a new Dog anchor and a 0.5 m step. |

What breaks: the keeper plate, the socket seating, the walkway run, the strips on it, the hangers' tyre wraps (Light debris). Houses do not break unless they meet something. Reset between rounds.

## Interactions

- **Haul** the pin head along its axis from a brace 5 m back: commit. Unbraced, the Breaker takes 4.5 m/s² toward the static pin (under their 5 m/s² foot-hold: a 1 m skid over the commit). Braced (20 m/s²), nothing.
- **Jack** off the anchor head: a Slingshot along the rim.
- **Hang** from the pin head or the cable socket: a Hang line never unseats a placed object (05 section 6.2), so any class anchors a Pendulum or Gallows to a live pin up to the class Hang load without committing it. Hang an enemy to the cable socket before committing: they are carried over the edge with the socket. The Anchor Drop with 400 m of drop.
- Chain: the socket whip crosses the prep store sector on the east rim: the Office placed the store 2 m outside the painted arc, and a propane cylinder Hauled 2 m into the arc by a Scrapper before the commit becomes a 20 m/s blast on the Breaker's position.

## States

| State | Visual |
|---|---|
| Intact | Pin seated, keeper on, tape clean, lamp on. |
| Stressed | Head rocks, grease squeezes from the cheeks, tape stretches. |
| Failing | Groan, strips flickering down the run, pin walking. |
| Failed | Empty head, pin on the deck, socket gone, cable over the edge, painted arc scored bright. |

## VFX and SFX hooks

`SFX_AnchorPin_Lurch`, `SFX_Cable_Sing_180m` (spatialized along the cable spline), `SFX_Pin_Walk`, `SFX_Socket_Whip`, `SFX_FarAnchor_Bang`; `VFX_Grease_Squeeze`, `VFX_Deck_Score`, `VFX_Strip_Fail_Sequence`; Marshal VO; walkway and house event triggers.

## Technical

- LOD0 3,400 tris, LOD1 1,500, LOD2 500, LOD3 box.
- Textures: `T_AnchorHead_Cast` 2K, `T_Hangtown_Steel_Trim` 2K, `T_Hangtown_Decal_Atlas` 2K.
- Pivot at the pin's axis center. Sockets: `Pin_End` (Haul target), `Keeper`, `Socket`, `Whip_Path` (spline), `Lamp`, `PinRest` (where the dropped pin lands).
- Implementation: pin walk and socket whip are a timeline (server time-synced). The cable is a spline mesh whose control points blend to the fallen catenary over 5.4 s. Walkway sections release on the per-section timer. House swings use the Cable Day pre-authored spline set with the cable index as a parameter. The dropped pin becomes a Heavy rigid body with `Kinetic_Heavy`.
- Collision: `Kinetic_RatedLoad` seated, `Kinetic_NoCollide` during the walk (the head's cheeks remain `Kinetic_WorldStatic`), `Kinetic_Kinematic` on the whip arc.

## Reference search terms

suspension bridge cable anchorage pin, open spelter socket wire rope, zinc poured socket, cast steel clevis anchor, dam anchorage keeper plate.
