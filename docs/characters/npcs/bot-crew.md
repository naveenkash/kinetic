# Bot Crew (Tutorial)

_The crew the Greenhand faces in their first Claim Match, in the Yard, at the end of the Apprenticeship. Three HTR Probationers in orange. They are riggers in fiction and scripted opponents in fact._

## Who they are

HTR Probationers: Grade 1 on HTR's ladder (04). Bulk-leased Breakers with sequential Kessl serials, numbered not named, which is how HTR treats its reserves and its new hires alike. Teodora Quispe introduces them in one line and the player fights them in the Yard. They exist so the player's first real Haul lands on a slow, heavy, honest target and the first panic window has a long ramp.

## Appearance

| Element | Spec (**PROPOSED** unless from the bible) |
|---|---|
| Class | Breaker, all three. Standard Breaker mesh, 220 kg, 0.82 m shoulders. |
| Livery | HTR orange `#FF5A1F` and matte grey `#5A5E62`, factory fresh. `Wear` at 0. No tallies. |
| Numbers | P-01, P-02, P-03 on the back plate and the right shoulder, white, 6 cm. PROBATIONER stenciled under the number. |
| Visor | A grey full-face visor down on all three, HTR logo on the brow. No head mesh needed; the visor is the head. |
| Lease plate | `K61-B-00001`, `00002`, `00003`. Leased, SPONSORED HTR. |
| Dogs | HTR machined Standard Dogs. |
| Cutter | Standard, factory, Kessl inspection sticker still on the handle. |
| Telemetry | Live. They Stall like anyone else. |

The visor does two jobs: it saves three heads, and it makes the bots read as HTR's interchangeable manpower, which is what the Guild thinks of HTR.

## Behavior

Scripted, slow, and legible. The bot crew exists to be read.

| Beat | Behavior |
|---|---|
| Drop | The HTR cargo Drop, all three on one Hook sling, 5.0 s. |
| Opening | Walk the honest route along the Floor toward the Claim marker in the Stacks. Never sprint. |
| Approach | At 15 m from the player, P-01 fires Dog A at the player's nearest plate with a telegraphed 1 s wind-up pose. Dog speed 90 m/s. |
| Haul | On a seat, P-01 braces (0.3 s) and Hauls. The 0.4 s ramp to 1,000 N. The player is dragged toward P-01 or toward the nearest trench. This is the panic window taught in the Apprenticeship, now live. |
| If cut | P-01 recalls (0.9 s) and does not re-fire for 6 s. P-02 steps forward instead. |
| Avalanche | P-02 attempts one Avalanche on an authored billet stack when the player stands within 6 m of it, with a 2 s wind-up. The stack settles slowly (03). |
| Caboose | P-03 attempts one Caboose when a Hauler klaxon sounds and the player is within 10 m of the trench. The klaxon-to-train gap is 3 s; the player has it all. |
| Defense | Bots cut lines on their own plates after 1.4 s (**PROPOSED**), slower than a median player, so a Gallows on them works if the player commits. |
| On the marker | Bots hold the marker and stand still. A Breaker standing still is the easiest Gallows in the game. |
| Difficulty | One setting. The tutorial is under fifteen minutes total (04). |

Bots never use Jack, never overspin (they cannot), never move rated loads in the tutorial, and never stand on the Floor near the Shear Press pit. They are not allowed to scrap the player more than twice; after the second, the third attempt fails on purpose (a Dog that misses by 1 m).

## Animation

The full Breaker class set, plus:

| Clip | Notes |
|---|---|
| Telegraphed wind-up | 1 s shoulder-and-arm pose before firing, bigger than any player would use |
| Confused recall | On a cut, a 0.5 s look at the Spool before the 0.9 s recall |
| Hold the marker | A wide-stance idle, hands on hips |
| Scrapped | The standard Stall lock; the Hook lift is the player's first time seeing one on an enemy |

## Audio

- No Face voice. HTR crew radio: short, clipped, filtered. "Probationer one, Hauling." "Line cut." "Resetting." "Marker held." Six lines total, plus a Stall grunt.
- Footsteps and Spool are the Breaker set: the 0.7x pitch, the heel plates.
- The player hears Teodora over the top for the whole match. Her bark set covers the bots; the bots' own lines sit under hers.

## Scrap Card

When a bot scraps the player: the HTR work-order template with the number P-01 instead of a name. When the player scraps a bot: the player's own card, and the Ledger logs it as the first scrap. Teodora's Ledger entry for the match: three sentences, written after the match ends.

## Technical

| Item | Spec |
|---|---|
| Mesh | Breaker class mesh with a visor head variant (1.5k tris) in place of a Face head. One material instance with `ReserveIndex` reused for the P-number decal. |
| AI | A behavior tree with four tasks (walk route, fire and Haul, Avalanche, Caboose) and a scripted schedule. No general combat AI. |
| Physics | Standard `PA_Rigger_Breaker`. The bots are real bodies: the player can Gallows, Caboose, or Overboard them and the physics resolves as in 05. |
| Spawning | Authored spawn at the Yard's second zone. No reserves. |
| Reuse | The visor head and P-number decal are reusable for any future HTR fodder (bot fill in Free Scrap, **PROPOSED**). |

## Reference search terms

factory-fresh hi-vis workwear, full-face grey welding visor, forklift certification class, numbered trainee coveralls, corporate onboarding uniform.
