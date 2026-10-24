# KINETIC: UI Design System

_System spec v0.1, 2026-10-24, reconciled to 05-core-mechanics v0.2.2. One visual and interaction system for the in-match HUD ([ui-hud.md](ui-hud.md)), the frontend ([menus-and-frontend.md](menus-and-frontend.md)) and the web tools (admin console and web Ledger, [tech/backend-architecture.md](../tech/backend-architecture.md) section 17). Those documents say what each screen shows; this one says how everything is built: tokens, type, layout, glyphs, motion, components, accessibility, localization and the Unreal and web implementation. Team tints are canonical in bible 05 section 11a and are restated here, not changed. Anything marked **PROPOSED** is new here._

## 1. Principles

1. **The line is brighter than the UI.** No UI element may out-glow a line. HUD luminance is capped at 70% of a line at 100 N, with 85% for alerts under 0.5 s (ui-hud section 1). Menus that show a live line (Locker, training yard) follow the same cap.
2. **Amber means you.** Amber is your crew in your own view, and it is the selection and focus accent everywhere in the frontend. Sky only ever means the other crew. A player learns one color language in the menus and uses it in the match.
3. **Color is never the only channel.** Every tint is paired with a pattern, shape or position: enemy lines have ticks, Caution has stripes, plates have glyphs, chips have text.
4. **Two voices, two skins.** LayCast (broadcast, the HUD, lobby, post-match page 1, Claims map) and the Office (Ledger, shop, Contracts, Manifest, dialogs) each have a skin built from the same tokens (section 6).
5. **Gameplay times are data.** Any animation that shows a gameplay duration (mode switch, Cutter warm-up, recall, set-line life, reserve timer, overspin) reads its length from the data tables in [tech/project-structure.md](../tech/project-structure.md) section 4. UI never hard-codes a gameplay time.
6. **One source of tokens.** Colors, type, spacing and motion live in one tokens file that generates the Unreal assets, the web CSS and the Figma library (section 2).

## 2. Tokens: the single source

`Content/UI/Tokens/tokens.json` (**PROPOSED**), in the W3C design-tokens JSON format, owned by the UI designer. A CI step generates:

| Target | Generated asset | Used by |
|---|---|---|
| Unreal | `DA_UITokens` (data asset), `MPC_UI` (material parameter collection for tints and luminance caps), CommonUI style assets (`CTS_`, `CBS_`, `CBorS_`, section 14) | HUD and menus |
| Web | `tokens.css` (CSS custom properties) | Admin console, web Ledger |
| Figma | Variables import | Design library (section 17) |

Token names are semantic, never literal: `color.tint.crew`, not `color.amber`. Runtime code resolves semantic tints to the active palette (Standard, High contrast, Mono) and the active tint mode (relative or absolute).

## 3. Color

### 3.1 Gameplay tints (canonical, bible 05 section 11a)

| Token | Name | Hex | Means | Paired with |
|---|---|---|---|---|
| `color.tint.crew` | Amber | `#E69F00` | Your crew, your lines, your selection | Unbroken line |
| `color.tint.enemy` | Sky | `#56B4E9` | The other crew | 4 cm dark tick every 1 m on lines |
| `color.tint.third` | Rose | `#CC79A7` | Third crew, 3v3v3, post-launch | Double tick |
| `color.neutral.bone` | Bone | `#E8E6E0` | Office, Marshal, neutral markers, set lines, untinted UI text | |
| `color.hazard.caution` | Caution | `#F2C200` on `#111111` | Rated loads, siren banner, Hauler lights, last 10 s | Always the 45° stripe pattern |
| `color.stall.gel` | Gel | `#8FE3CF` | The Stall flood and the Scrap Card frame only | |

**Palettes** (ui-hud section 5): Standard (above); High contrast (Amber `#E69F00`, Blue `#0072B2`, Rose `#CC79A7`); Mono (Bone with patterns only). The palette swap changes `MPC_UI` and the token resolver; no widget stores a tint directly.

### 3.2 Surfaces

| Token | Value | Use |
|---|---|---|
| `color.plate.light` | `#000000` at 40% | HUD chips and tags with text 2.2 h or larger |
| `color.plate.medium` | `#000000` at 50% | Marshal subtitles |
| `color.plate.heavy` | `#000000` at 60% | LayCast bug, Scrap Cards, kill feed backing, any HUD body text under 2.2 h (**PROPOSED** rule; see section 12.3) |
| `color.ink.hazard` | `#111111` | Caution stripes, banner center |
| `color.laycast.frame` | `#00A9A5` | LayCast broadcast frames in menus, lobby and spectator views. **PROPOSED**: never on the in-match player HUD, so nothing teal competes with Gel |
| `color.office.paper` | `#EDE6D3` (**PROPOSED**) | Ledger, Contracts, Manifest, Office dialogs |
| `color.office.ink` | `#1C1A17` (**PROPOSED**) | Office text |
| `color.office.rule` | `#B9B09A` (**PROPOSED**) | Ledger ruled lines, table rules |
| `color.office.stamp` | `#9E2B25` (**PROPOSED**) | Inspection stamps, "Clear", locked-entry marks |
| `color.office.stamp-blue` | `#2B4C7E` (**PROPOSED**) | Rating band stamps (game-modes section 6.3) |

### 3.3 Faction and sponsor colors

Faction livery colors ([characters/README.md](../characters/README.md)) and sponsor colors ([objects/environment/sponsor-banners.md](../objects/environment/sponsor-banners.md)) appear in the frontend only: faction plates on the Allegiance screen, Contract headers, Face cards, sponsor marks. They never appear on gameplay readouts, crosshair, chips, edge indicators or the tension arc. Nurri's green and Corda's amber follow the sponsor document's rules.

### 3.4 Contrast reference

Computed relative-luminance contrast against `#111111`:

| Foreground | Ratio | Pass for |
|---|---|---|
| Bone `#E8E6E0` | about 15:1 | All text |
| Gel `#8FE3CF` | about 12:1 | All text |
| Caution `#F2C200` | about 11:1 | All text |
| Amber `#E69F00` | about 8:1 | All text |
| Sky `#56B4E9` | about 8:1 | All text |
| Blue `#0072B2` (high-contrast palette) | about 3.6:1 | Glyphs and large text only (3:1); never small text |
| `#111111` on Amber (active chip) | about 9:1 | All text |

Over a live 3D scene a translucent plate is never a guaranteed background, so contrast is verified in-engine by the readability test (section 16), not by these numbers alone.

## 4. Typography

### 4.1 Families (PROPOSED)

All licensed under the SIL Open Font License, so they ship in the game and on the web with no per-seat fee.

| Role | Family | Used for |
|---|---|---|
| Broadcast | Barlow Condensed (Medium, SemiBold) | LayCast bug, mode chips, banners, tags (HAUL, SCRAPPED, HOISTED), lobby headers |
| Interface | IBM Plex Sans (Regular, Medium) | Menus, settings, descriptions, Marshal subtitles, kill feed names |
| Numerals and Office | IBM Plex Mono (Regular, Medium) | Timers, newtons, Tonnage, the Ledger's typewriter numerals, serials |
| Stencil (display only) | Big Shoulders Stencil | Manifest crates, Office stamps. Never under 3 h, never body text |
| Fallback | Noto Sans CJK, Noto Sans | Japanese, Korean, Chinese and any glyph the primary families lack |

Numbers that change while visible (clock, tension, Tonnage counters, reel length) use tabular figures so they do not jitter.

### 4.2 Scale

HUD sizes are in h (percent of screen height, ui-hud section 1); pixels are at 1080p. Menu sizes are in pixels at 1080p and scale with the UI scale curve (section 5.2).

| Token | Size | 1080p px | Family | Use |
|---|---|---|---|---|
| `type.hud.tag` | 1.6 h | 17 | Broadcast | HOISTED, LINE LOST, rod-length readout, off-screen marker distance |
| `type.hud.body` | 1.8 h | 19 | Interface | Marshal subtitle, kill feed rows |
| `type.hud.chip` | 2.2 h | 24 | Broadcast | Mode chips, banner text |
| `type.hud.clock` | 2.4 h | 26 | Mono | Round clock |
| `type.menu.caption` | | 20 | Interface | Gates, footnotes, Office notes |
| `type.menu.body` | | 26 | Interface | All menu body text (**PROPOSED** minimum on TV and console) |
| `type.menu.label` | | 26 | Broadcast | Buttons, tabs, rail items |
| `type.menu.title` | | 36 | Broadcast | Screen titles (THE LEDGER, HARNESS) |
| `type.menu.display` | | 56 | Broadcast or Stencil | Grade-up, Clear, Cutter ceremony |
| `type.menu.number` | | 26 / 36 | Mono | Stats, prices, standings |

**PROPOSED**: on PC the menu body may drop to 22 px under a "Compact" UI option; it never drops on console.

### 4.3 Case and style

- Broadcast tags and screen titles are uppercase, three words or fewer (HAUL, DOWN THE WELL, SHEAR IN 5). This is the LayCast style in ui-hud and menus-and-frontend.
- Everything else is sentence case: descriptions, settings, dialogs, Office copy.
- Office copy is never bold for emphasis; LayCast copy may use one bold term per card.
- Units are SI and always shown with a space: `600 N`, `11.2 m/s`, `38 m`, `1,215 N·s`.

## 5. Units, layout and grid

### 5.1 HUD

Screen-space percentages, origin top-left, h for height-relative sizes, w for width-relative (ui-hud section 1). Safe area 2% on all sides; on console the platform safe zone applies on top of it through a safe-zone container (**PROPOSED**: the larger of the two wins). HUD scale option 80% to 120% scales sizes, never positions of the edge indicator, which always sits on the true screen edge.

### 5.2 Menus

- Reference resolution 1920 × 1080; Unreal's DPI scale curve on the shortest side, 1.0 at 1080, linear to 2.0 at 2160.
- 12-column grid, 96 px margins, 24 px gutters at 1080p (**PROPOSED**).
- Left rail 320 px with the seven items from menus-and-frontend section 2; content area is the remaining 10 columns.
- Spacing scale (`space.*`, px at 1080p): 4, 8, 12, 16, 24, 32, 48, 64, 96.
- Ultrawide: content stays in a 16:9 center region; backgrounds extend. The HUD anchors to the true edges.

### 5.3 Corners and lines

- `radius.none` 0 for LayCast plates and the bug (broadcast hardware look); `radius.small` 2 px for chips and buttons; Office paper has 0 radius and a 1 px `color.office.rule` border.
- Rule lines 1 px at 1080p; glyph strokes 2 px at 1080p (crosshair stroke, ui-hud 4.1).

## 6. Skins

| Skin | Where | Surface | Text | Accent | Texture |
|---|---|---|---|---|---|
| **LayCast** | HUD, lobby, matchmaking, post-match page 1, Claims map, spectator | Heavy plates on the live scene; `color.laycast.frame` frames in menus only | Bone | Amber (you), crew tints | None. Flat broadcast graphics |
| **Office** | Ledger, Contracts, Manifest, shop, settings, all confirm dialogs, post-match page 2 | `color.office.paper` | `color.office.ink` | `color.office.stamp`, Amber for selection | Paper grain and ruled lines at under 6% contrast so text stays clean |
| **Training yard** | Main menu, Harness select, Locker preview | The 3D Collar Four yard (menus-and-frontend section 2) | Bone on heavy plates | Amber | The world itself |

Both skins share every component (section 9). A skin changes only token values, never layout or behavior.

## 7. Glyphs and icons

One glyph library, `Content/UI/Glyphs/`, drawn on a 24-unit grid, 2 px stroke at 1080p, square caps and mitred joins (rigging hardware, not consumer app). **PROPOSED** set:

| Group | Glyphs |
|---|---|
| Plates | Chest, back, left shoulder, right shoulder (ui-hud 4.1) |
| Spool | Dog, line, line live, line lost, recall, ratchet in, ratchet out, set-line pip |
| Cutter | Blade (with fill states), cable-hold ring |
| Classes | Five silhouettes whose widths read mass: Climber narrowest, Breaker widest (ui-hud 4.9) |
| Modes | No icons. HAUL, JACK, HANG are always words |
| Objectives | Claim marker, Haul spool, truck, beacon |
| Rig Calls | Eight hand signals from ui-hud 4.17 |
| State | Scrapped (crossed), reserve drain, lock, gate, hatch pattern for unavailable |
| Hazard | Caution stripe tile, rated-load ticks |
| Input | Platform button glyphs, supplied by CommonUI input data per platform |

Glyphs are single-color and take their tint from the widget. No glyph uses Gel. Sponsor marks are illustrations, not glyphs, and live with the sponsor assets.

## 8. Motion

| Token | Duration | Curve | Used for |
|---|---|---|---|
| `motion.instant` | 0 | | Indicators vanishing on cut, release or snap (same frame, ui-hud 4.6) |
| `motion.snap` | 100 ms | ease-out | Radial close, banner slam, Dog tick leaving the ring |
| `motion.seat` | 150 ms | ease-out | Ring stroke double on seat, Cutter swing flash |
| `motion.screen` | 200 ms | ease-out | Menu screen push and pop, tab change |
| `motion.fade` | 500 ms | linear | Set-line fade, readout fade, drained pip fade |
| `motion.pulse` | 2 Hz | sine | Edge indicator, Cutter ring while hoisted |
| `motion.strobe` | 8 Hz | square | Overspin above 1.0 s (matches the line flicker) |
| `motion.data` | from data tables | linear | Mode-switch wipe, Cutter warm-up, recall collapse, set-line drain, reserve drain, Kick bar |

Reduced motion (**PROPOSED** option, plus the existing "Siren banner flash: reduced"): menu transitions become 0 ms fades, pulses become steady at 100% opacity, strobes become a steady white mark. Gameplay-timed drains stay, because they carry information.

## 9. Components

Every component exists once and is skinned by tokens. Unreal widget names follow [tech/project-structure.md](../tech/project-structure.md) section 3.

### 9.1 HUD components (specified in ui-hud section 4)

| Component | Widget | Notes |
|---|---|---|
| Crosshair | `WBP_Crosshair` | All states in ui-hud 4.1; Doubler second ring |
| Mode chips and radial | `WBP_ModeChips` | Wipe length from `DT_HarnessClass.ModeSwitch`; Kick bar |
| Tension arc | `WBP_TensionArc` | Material-driven; luminance ramp shared with the line emissive |
| Overspin meter | `WBP_OverspinMeter` | Doubler only |
| Set-line pips | `WBP_RadialDrain` | Shared radial drain primitive |
| Edge indicator | `WBP_EdgeIndicator` | Cannot be hidden by any option (readability contract) |
| Cutter glyph | `WBP_CutterGlyph` | Cannot be hidden |
| State tags | `WBP_StateTag` | HOISTED, LINE LOST, RESET |
| Crew status | `WBP_CrewRow` | Uses `WBP_RadialDrain` for reserves |
| LayCast bug | `WBP_LayCastBug` | Per-mode score variants |
| Objective markers | `WBP_WorldMarker` | Edge-clamped with distance |
| Event banner | `WBP_EventBanner` | Cannot be hidden |
| Marshal subtitle | `WBP_Subtitle` | Shared with menus and cinematics |
| Kill feed | `WBP_KillFeed`, `WBP_KillFeedRow` | Trade language only |
| Scrap Cards | `WBP_ScrapCard_Killer`, `WBP_ScrapCard_Victim` | Gel frame; faction template from cosmetics |
| Rig Call wheel | `WBP_RigCallWheel` | Eight slots |

### 9.2 Frontend components

| Component | Widget | Base class | States and rules |
|---|---|---|---|
| Rail item | `WBP_RailItem` | CommonButtonBase | Default, focused, selected (Amber bar on the left edge), notification dot |
| Button, primary | `WBP_Button` (style `CBS_Primary`) | CommonButtonBase | Amber fill, `#111111` text. One primary per screen |
| Button, secondary | `WBP_Button` (`CBS_Secondary`) | CommonButtonBase | Bone outline |
| Button, Office | `WBP_Button` (`CBS_Office`) | CommonButtonBase | Ink outline on paper |
| Hold-to-confirm | `WBP_HoldButton` | CommonButtonBase | For irreversible or costly actions (Allegiance switch, purchase with sponsor credit, forfeit vote): fills over 0.8 s (**PROPOSED**) |
| Tab | `WBP_Tab` | CommonTabListWidgetBase | Ledger tabs: Record, Entries, Rope Marks, Serial |
| List row | `WBP_ListRow` | CommonButtonBase | Locker items, Entries, settings rows |
| Stat row | `WBP_StatRow` | UserWidget | Label left, Mono number right, optional unit |
| Tradeoff row | `WBP_TradeoffRow` | UserWidget | Spool loadout: gain and cost always both visible; "Standard. Competitive." on the baseline |
| Progress bar | `WBP_Progress` | UserWidget | Weight, Grade, Manifest, Contract counters; tick marks for thresholds |
| Radial drain | `WBP_RadialDrain` | UserWidget | Shared with the HUD |
| Harness card | `WBP_HarnessCard` | CommonButtonBase | Silhouette first, mass, weight class, all numbers from 05 visible |
| Face card | `WBP_FaceCard` | CommonButtonBase | LayCast portrait, broadcast name, quote |
| Contract row | `WBP_ContractRow` | UserWidget | Faction voice, counter, reward line |
| Manifest crate | `WBP_ManifestCrate` | CommonButtonBase | Stencil label, Free and Sponsored columns, ticked state |
| Shop item | `WBP_ShopItem` | CommonButtonBase | Price in credit plus real-money equivalent where applicable; Visibility Clause line on every Tape item; no timers, no "sold out" (menus-and-frontend section 10) |
| Standing bar | `WBP_StandingBar` | UserWidget | Claims map, five factions, one decimal (game-modes section 7) |
| Map card | `WBP_MapCard` | UserWidget | Arena, mid-match event and its time, moving part |
| Lobby slot | `WBP_LobbySlot` | UserWidget | Silhouette, Face, Grade, Rope Mark tape |
| Dialog | `WBP_Dialog` | CommonActivatableWidget | Office skin, Office voice, never more than two buttons |
| Toast | `WBP_Toast` | UserWidget | Crew invite, match found, rewards ready (backend Realtime events) |
| Input glyph | `WBP_InputGlyph` | CommonActionWidget | Swaps per device automatically |
| Settings controls | `WBP_Toggle`, `WBP_Slider`, `WBP_Choice` | CommonUI | Every option shows its current value as text |

### 9.3 Shared states

| State | Visual |
|---|---|
| Focus (keyboard or gamepad) | 2 px Bone outline, 4 px outside the element, plus the Amber accent; focus is never shown by color alone |
| Hover (mouse) | Same as focus, at 60% |
| Pressed | Fill 10% darker for 100 ms |
| Selected | Amber fill or Amber left bar |
| Disabled | 40% opacity, no hover |
| Unavailable (gameplay) | Hatch pattern plus a drain where a timer applies (ui-hud 4.2) |
| Locked (progression) | Lock glyph plus the gate in Office phrasing ("Grade 3. Rigger.") |
| Loading | A single ratchet glyph turning in 45° steps, no spinners |

## 10. Input and navigation

- CommonUI owns focus, input routing and the screen stack. Layers: `Game` (HUD), `GameMenu` (pause, scoreboard, Rig Call wheel), `Menu` (frontend screens), `Modal` (dialogs, toasts).
- Back is always Escape or B (menus-and-frontend section 2). Any screen reaches the lobby in one input when a match is forming.
- Every screen is fully usable with gamepad, keyboard alone, and mouse; no hover-only information.
- Input glyphs come from CommonUI input data per platform and update the moment the last-used device changes.
- Holds use one shared duration per kind: hold-to-confirm 0.8 s, Rig Call wheel opens on hold, set-line pick on 0.3 s Recall hold (ui-hud 4.5).

## 11. Copy and voice

The rules live in menus-and-frontend section 1 and ui-hud. In short:

- Office: dry, procedural, no exclamation marks, informs rather than sells.
- LayCast: broadcast energy, one exclamation mark per screen at most.
- Trade language: never "killed"; scrapped, hung, cut, snapped, down the Well.
- Numbers on screen reconcile with the Ledger and with bible 05 to the digit.

## 12. Accessibility

### 12.1 The readability contract

No option may hide the edge indicator, the Cutter glyph or the siren banner (ui-hud section 5).

### 12.2 Options

All options in ui-hud section 5, plus (**PROPOSED**): UI scale for menus (90% to 150%); reduced motion (section 8); subtitle background opacity (50% to 100%) and speaker names; text-to-speech and speech-to-text for crew chat (required for in-game communication under US CVAA, see [tech/tech-stack-plan.md](../tech/tech-stack-plan.md) section 10); a menu narration mode using Unreal's screen-reader support where it works, with every widget carrying an accessible name.

### 12.3 Text rules

- Body text over a live scene sits on `color.plate.heavy` (60%) unless it is 2.2 h or larger (**PROPOSED**; the readability test may raise it).
- Minimum sizes: HUD 1.6 h; menu body 26 px at 1080p on console; captions 20 px.
- Text contrast at least 4.5:1, glyphs and large text at least 3:1, measured on the plate, and confirmed in-engine.
- No information in a text-only flash shorter than 1 s, except the readouts already timed by gameplay.

## 13. Localization

- All UI text is `FText` from string tables; no text baked into textures (stencils included: Manifest crate labels are live text in the Stencil family, with the Noto fallback).
- Allow 40% expansion for European languages; tags and chips are sized for the longest language, and the uppercase tags have per-language short forms where needed.
- Numbers, separators and dates use the culture formatter; units stay SI in every language.
- Launch languages are set by production; CJK is covered by the Noto fallback. Right-to-left languages are not in scope for launch.
- Pseudo-localization (accented, 40% longer) is a build option used in CI screenshot tests.

## 14. Unreal implementation

- **Module:** `KineticUI` ([tech/project-structure.md](../tech/project-structure.md) section 1).
- **Folders:** `Content/UI/Tokens/`, `Content/UI/Styles/` (CommonUI style assets), `Content/UI/Glyphs/`, `Content/UI/Fonts/`, `Content/UI/Components/` (`WBP_` building blocks), `Content/UI/HUD/`, `Content/UI/Menus/`.
- **Style asset prefixes (PROPOSED):** `CTS_` CommonTextStyle, `CBS_` CommonButtonStyle, `CBorS_` CommonBorderStyle, `DA_UITokens`, `MPC_UI`.
- **Tints:** widgets and HUD materials read tints from `MPC_UI`, so a palette or tint-mode change is one parameter write. The HUD luminance cap is also an `MPC_UI` scalar derived from the line emissive at 100 N ([vfx.md](vfx.md)).
- **Data flow:** each screen and HUD block has a view model object updated by events (line seated, mode changed, Stall, backend profile loaded, Realtime toast). UMG property bindings that poll every frame are not allowed. Whether to use Unreal's MVVM plugin or plain view-model objects is decided in the Phase 1 HUD spike (**CHECK** the plugin's status in 5.8).
- **Performance:** UI budget is 0.7 ms game thread and 0.5 ms render thread on the recommended PC and consoles ([tech/performance-budgets.md](../tech/performance-budgets.md) section 2). Static HUD blocks sit in invalidation boxes; the crosshair, tension arc and edge indicator are material-driven so their animation costs no layout.
- **Server build:** `KineticUI` is excluded from the server target.

## 15. Web implementation

The admin console and the web Ledger use the same tokens through the generated `tokens.css`, the Office skin, and the same font families self-hosted. Web components mirror the frontend list where it applies (button, tab, list row, stat row, progress bar, standing bar, dialog). Focus outlines and contrast follow section 12; web pages also meet WCAG 2.2 AA.

## 16. Testing

| Test | When | Pass |
|---|---|---|
| Readability screenshot ([tech/project-structure.md](../tech/project-structure.md) section 7) | Weekly, min-spec PC, darkened Anchor sector | Line emissive above the threshold; HUD below the 70% cap |
| HUD luminance | Every HUD change | No element above 70% of a 100 N line, alerts above it for under 0.5 s |
| Token contrast lint | Every tokens change | Section 3.4 ratios hold for every palette |
| Colorblind simulation | Every HUD and lobby change | Deuteranopia, protanopia and tritanopia captures still separate crew, enemy and hazard |
| Pseudo-localization | Nightly | No clipped or overlapping text on any screen |
| 10-foot test | Each milestone | Every menu readable at 3 m on a 55-inch TV at 1080p |
| Input parity | Each milestone | Every screen completed with gamepad only and with keyboard only |
| Playtest | Twice weekly from Phase 0 week 3 | "What killed you" answers never cite the HUD hiding something |

## 17. Figma library

Pages: Tokens (generated variables), Type, Glyphs, HUD components, Frontend components, Web components, Screens (one frame per wireframe in ui-hud and menus-and-frontend). Variables are imported from `tokens.json`; designers do not edit color values in Figma. Components carry the same names as the Unreal widgets so handoff is one-to-one.

## 18. Contradictions found

| # | Item | Where | Default here |
|---|---|---|---|
| 1 | Caution yellow is `#F2C200` in ui-hud section 2 and `#F2C500` in structural-tape | ui-hud; objects/environment/structural-tape | `#F2C200` for UI (05 section 11a family); tape keeps its physical paint value. Confirm one value |
| 2 | Guild ochre is `#C8862B` in characters/README and `#B8752A` in structural-tape | characters/README; structural-tape | `#C8862B` for UI faction plates |
| 3 | HTR orange is `#FF5A1F` in characters/README and `#E8541E` in structural-tape | same | `#FF5A1F` for UI |
| 4 | Fray magenta is `#FF2BD6` in characters/README and `#FF2A9D` in structural-tape | same | `#FF2BD6` for UI |
| 5 | HUD chips use Bone text on 40% black (ui-hud 4.2); small text on 40% may fail contrast over bright scenes | ui-hud | Keep 40% for 2.2 h text, 60% for smaller (section 12.3); the readability test decides |

## 19. Decisions to make

| # | Decision | Default here | Needed by |
|---|---|---|---|
| U1 | Font families | Section 4.1 | Phase 1 week 14, before the HUD pass |
| U2 | Office paper palette | Section 3.2 | Phase 1 week 14 |
| U3 | LayCast teal kept off the player HUD | Yes | Phase 1 week 14 |
| U4 | Menu minimum text size on console | 26 px at 1080p | Phase 1 |
| U5 | MVVM plugin or plain view models | Decide in the HUD spike | Phase 1 week 14 |
| U6 | Reduced-motion option | Add it | Phase 3 accessibility pass |
| U7 | Caution and faction hex values (section 18) | Section 18 defaults | Before the art pass |
