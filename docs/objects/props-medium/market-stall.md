# Market stall

Medium prop. Mass tier Medium, 90 kg. 240 × 120 × 220 cm (L × W × H to the awning ridge). Appears in Hangtown (the Mid Hang, twelve on the market ring, four on the swing-bridge landings) and the Yard (the Floor, two at the Guild's shift canteen by the control shack).

## Role in play

The weakest destructible in the game. It fractures at 300 N·s, which a Rigger reaches by walking into it at 3.3 m/s, and 05 §6.5 counts player bodies: a flipped 130 kg Rigger at 5 m/s is 650 N·s and breaks it. It is cover, clutter, a Clothesline anchor, and a source of Light planks. It is also a resident's livelihood on LayCast, and Hangers notice who breaks them.

| Rig | Use |
|---|---|
| The Clothesline | Awning poles at 2.2 m and the counter at 0.9 m are two anchor heights in one object. |
| The Guillotine | Hang a stall overhead (0.9 kN, within every class Hang load), then recall or cut your own set line; it drops by gravity. It needs a 9 m fall to reach its 13.3 m/s Stall speed; from 4 m it lands at 8.9 m/s, 801 N·s, a flip, and bursts. |
| The Scatter | Haul a stall into a crowd; it bursts and the planks keep going. |
| Overboard | A stall at the market ring's edge is a soft wall that stops being a wall. |

- **Scrapper:** at 90 kg the stall is under the 100 kg Medium limit, so it moves 30% faster on a Scrapper line: 400 N is 4.4 m/s², 13.3 m/s over 20 m, times 1.3 is 17.2 m/s. The planks it drops are Light and get the same bonus.
- **Rigger:** uses the awning frame as a set-line anchor. Never moves the stall; it is the room.
- **Breaker:** walks through it. 220 kg at 5 m/s delivers 1,100 N·s to the stall. It is gone.
- **Climber:** perches on the ridge pole. Jacks off the counter.
- **Doubler:** Hauls two stalls together. 180 kg of timber meeting over an enemy; each line at 480 N moves its stall at 5.3 m/s².

## Appearance

Silhouette: a counter box on four legs, two rear posts rising to a sloped awning, a canvas sheet over the awning frame, goods strapped to the counter. Reads as a house-shaped outline with a soft top.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Posts, counter frame | `MI_Timber_Rough` | `#7A5C3A` | 0.78–0.90 | 0 |
| Counter top | `MI_Plywood_Weathered` | `#9C8564` | 0.80–0.92 | 0 |
| Awning | `MI_Canvas_Natural` `#B9AE93` or `MI_Tarp_Blue` `#2B5F9E` | | 0.90–1.00 / 0.55–0.70 | 0 |
| Lashings | `MI_Rope_Sisal` | `#B59D6E` | 0.90–1.00 | 0 |
| Paint | `MI_Hanger_Paint` two of `#2E8B8B`, `#B5302A`, `#D9A441`, `#6E4C9E` | | 0.50–0.75 | 0 |
| Fittings | `MI_Steel_Rusted` | `#7A3F1C` | 0.75–0.95 | 0.3 |

Wear: counter front polished by elbows at 90 cm. Paint drips on the posts. Canvas sun-faded on the ridge, dark with soot under the awning. Chalk prices on the counter front. A condemnation notice stapled to the rear post, corner torn.

Text: hand-painted name boards: `VILCA / FISH`, `KITE REPAIRS`, `GEL + TAPE`. Notices read `OFFICE OF THE LAY / CLAIM 61-3 / STRUCTURE CONDEMNED`. Yard canteen stalls read `LG CANTEEN / SHIFT 2`.

Lighting: Office match lights come from above, so the awning is lit and the counter is in shadow. The canvas glows slightly with subsurface 0.2. A singing line crossing under an awning is the brightest thing in frame, as required.

## Variants

| Variant | Difference |
|---|---|
| Canvas | Natural canvas awning, teal and mustard paint. |
| Tarp | Blue poly tarp awning, red and violet paint. Louder in the normal map. |
| Yard canteen | Guild ochre posts, galvanized counter top, no goods. |
| Pre-broken | Counter collapsed, one post down, awning hanging. Spawns on Cable Day's dropped half-ring. |
| Loaded | Goods: laundry bundles, buckets, paint cans strapped on (Light props, released on fracture). |

## Physics

- Tier Medium, 90 kg.
- Collision: counter box 240 × 60 × 90 cm; two post capsules 220 cm tall, radius 5 cm; awning box 240 × 120 × 6 cm at 200 cm, sloped 15°. (**PROPOSED**)
- Center of mass: 70 cm above the floor, 20 cm rear of the counter center. Tips backward at 18° on a slope. (**PROPOSED**)
- Friction 0.6 wood on wood, 0.55 on steel. Restitution 0.1. (**PROPOSED**)
- Break-free (05 §6.2): loose 0 N, anyone. Lashed to the market ring rail 800 N, Doubler and Breaker only. A Scrapper cannot pull a lashed stall off the ring; they cut the lashing or break the stall where it stands. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Fracture (05 §6.5, market stall): 300 N·s. Fragments: Light planks. Specifically 8 planks at 6 kg, 2 posts at 9 kg, 1 canvas sheet at 4 kg, 1 counter top at 11 kg. All Light, 10 s lifetime. The counter top is the largest fragment, so a seated Dog reseats there.

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d); Scrapper arrival × 1.3):

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 4.4 m/s² | 50 m | 21.0 m/s | 1,890 N·s |
| Scrapper | 400 N | 4.4 m/s² | 20 m | 17.2 m/s | 1,548 N·s |
| Rigger | 600 N | 6.7 m/s² | 40 m | 23.1 m/s | 2,079 N·s |
| Doubler | 800 N | 8.9 m/s² | 30 m | 23.1 m/s | 2,079 N·s |
| Doubler overspin | 1,200 N | 13.3 m/s² | 30 m | 25.0 m/s (cap) | 2,250 N·s |
| Breaker | 1,000 N | 11.1 m/s² | 20 m | 21.1 m/s | 1,899 N·s |

Impact on a player (impulse = 90 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 2.0 m/s (walking) | 180 N·s | Nothing |
| 3.3 m/s | 300 N·s | Stagger; the stall fractures on this hit |
| 6.7 m/s | 600 N·s | Flip; fractures |
| 11.7 m/s (7 m drop) | 1,053 N·s | Flip; fractures |
| 13.3 m/s | 1,200 N·s | Stall; fractures. A Scrapper needs 12 m of run-up, a Breaker 8 m |
| 25 m/s (Haul cap, overspin only) | 2,250 N·s | Stall; fractures |

The stall delivers its impulse, then breaks. Fragments carry the remaining velocity; the 11 kg counter top at 25 m/s is 275 N·s, under stagger. A Doubler Snap puts 1,500 N·s into the stall at the Dog: the stall bursts on the snap and its fragments leave at up to 16.7 m/s, none over 184 N·s.

Jack (05 §3.2 v0.2): the stall is Kicked to V × M / (M + m) within 0.3 s (the Kick), then accelerates at T / 90 kg. Climber (V 18): stall 9.0 m/s, Climber 9.0 back. Scrapper (V 14): stall 7.7, Scrapper 6.3. Rigger (V 12): stall 7.1, Rigger 4.9. Doubler (V 14): stall 8.3, Doubler 5.7. Breaker (V 10): stall 7.1, Breaker 2.9. Every Jack starts the stall at a flip speed and it bursts on the first thing it meets. The Scrapper bonus applies to the post-Kick push, not to V (05 §6.1: 1.69× force, ×1.3 arrival speed).

Crush: a stall cannot pin anyone. It fractures long before any force matters, and no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg).

## Interactions

- **Haul to it:** the player is pulled to the stall. 400 N on a 90 kg Climber and on a 90 kg stall: 4.4 m/s² each, a 50:50 split, closing at 8.9 m/s². A 220 kg Breaker at 1,000 N skids (4.5 m/s², under the 5 m/s² foot-hold) while the stall arrives at 11.1 m/s² and bursts on them; at 12 m/s closing the Breaker takes 1,080 N·s, a flip, per 05 §7.2.
- **Jack from it:** the split above. Against anyone heavier than 90 kg the stall takes most of V.
- **Hang from it:** a Hang to a lashed stall holds on the lashing up to the class Hang load without starting it; the awning frame is a safe overhead anchor for a hung Light prop. A Hang to a loose stall drags it as 90 kg (0.9 kN) of dead weight.
- **Object to object:** anything Medium touching it over 3.3 m/s breaks it. Two stalls Hauled together both break.
- **Kinematic:** the Lift counterweight sweeping through a stall at 3 m/s delivers 270 N·s and knocks it over intact; at 4 m/s it breaks.
- **Water:** floats. Not relevant on its home maps.
- **Updraft and Pulse:** not present on its home maps. Where a stall is carried onto the Anchor, force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects) and scale as a_ref × 100 / m: 1.1× on a loose 90 kg stall, 2× (cap) on the 4 kg canvas fragment, which is lifted and drifts.
- **Breathing:** not present in Ballast.
- **Sway (Hangtown):** stalls are children of the market ring and move with it. On Cable Day the dropped half-ring tilts to 40° and every stall on it slides to the low edge, breaks against the rail, and the planks go into the Well.
- **Cut recoil:** 150 N·s per 1,000 N of force. On the stall itself under 1 m/s; on a loose 6 kg plank a Breaker-line cut is 25 m/s.
- Slides, tips, does not roll.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Standing, goods on the counter. | One body, 90 kg. |
| Dented | Awning torn, one post cracked, goods spilled. | No change. |
| Broken | Fractured: planks, posts, canvas, counter top loose. | Twelve Light bodies. Pile surface ×1.2 wood on stagger, flip, and Stall. |
| Destroyed | Fragments despawned after 10 s; nothing replenishes within the round (05 §9.2). Name board persists as a Light prop. | |

## VFX and SFX hooks

- Impact: `SFX_Stall_Creak` under 300 N·s, `SFX_Timber_Hit_Wood`, `_Steel`, `_Concrete`.
- Break-free (lashed): rope snap, `SFX_BreakFree_Rope`. Under 800 N the lashing creaks and the stall rocks.
- Fracture: `SFX_Stall_Burst` (timber crack, canvas flap, goods scatter), `NS_Stall_Fracture` with splinters, dust, a chalk puff, and the goods spawn; `NS_Canvas_Fall` on the awning fragment.
- No emissive. The name board is a candidate for a small LayCast sponsor light in the canteen variant.

## Technical

- LOD0 6,000 tris, LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set plus `T_Trim_Timber_01`. Canvas uses two-sided cloth material, not simulated cloth.
- Pivot: bottom center of the footprint. X+ is the counter front.
- Dog sockets: `SKT_Dog_Top_01` (ridge pole center) `Wood`; `SKT_Dog_Frame_01`, `_02` (rear posts at 150 cm) `Wood`; `SKT_Dog_Frame_03`, `_04` (awning front corners) `Wood`; `SKT_Dog_SideN_01` (counter front) `Wood`; `SKT_Dog_Rim_01` (counter top edge) `Wood`. Fragments: `SKT_Dog_F01_01` (counter top), `SKT_Dog_F02_01`, `_F03_01` (posts), `SKT_Dog_F04_01` to `_F11_01` (planks), all `Wood`; `SKT_Dog_F12_01` (canvas) `Fabric`.
- Geometry collection `GC_MarketStall`: L0 whole (damage threshold 300), L1 twelve fragments (threshold 100,000). L2 none. Goods are separate Light actors attached by a constraint that releases at L0 break.
- Anchor field `FS_Anchor_MarketStall` on lashed instances, released at 800 N or at Cable Day's scripted event.
- Replication RC-M intact; RC-GC on fracture, `ReplicationAbandonAfterLevel` = 1.

## Reference search terms

Timber market stall canvas awning; hillside shanty market stall painted; informal market counter plywood chalk prices; tarp awning stall night lights; market stall collapsed timber; favela market stall photo.
