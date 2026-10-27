# Space Battle: all-C++ web game (spec)

Status: ready-for-agent. Visual direction approved by the client: **Red Alert v2**.

## Problem Statement

The client has a working console game, `reference/original_main.cpp`: a turn-based duel between your ship and an enemy ship, played by typing 1 to 4 into a terminal. It must be turned into a game people play in a web browser, with a polished, premium interface. Her requirements:

- The whole game is written in C++. The only non-C++ file allowed is the small HTML loader that Emscripten generates.
- It runs as a website (a link), and also works on a phone.
- It is developed in **VS Code**.
- It stays simple enough for a 1st or 2nd year student to read, explain and present.
- Her game rules stay exactly as written, plus the rule changes she approved (listed under Game rules).

The terminal version has no visuals, no sound, and breaks if you type a letter (it loops forever).

## Solution

A raylib game written in plain C++, compiled to WebAssembly with Emscripten and drawn on one 1280x720 canvas that scales to fit the browser window. Players click (or tap, or press keys) instead of typing. The battle plays out with animations, sound, floating damage numbers and a combat log in a military "red alert" voice. The game goes Title → Battle → Victory or Defeat, with How to Play and Settings reachable from the Title.

The approved look is in `docs/design/*.png`: title, battle, howto, settings, victory and defeat. The playable mockups live on the Claude Design canvas "Space Battle UI Concepts", row "Red Alert v2". `docs/design/ra2-reference.css` lists every visual trait as CSS. Treat it as a reference to translate into raylib drawing code, not as code to ship.

## Glossary

- **Halberd**: the player's ship.
- **Warden**: the enemy ship.
- **Hull**: health, from 0 to 100.
- **Fire, Shields, Repair, Retreat**: the four player commands. Her code calls them Attack, Defend, Heal and Quit.
- **Engagement**: one round, meaning one player command followed by one enemy move. The turn counter shows it.
- **Tactical feed**: the combat log.
- **Debrief**: the stats shown after a battle ends.

## Game rules (single source of truth)

1. Both ships start at hull 100.
2. **Fire** deals a random 10 to 25 damage to the Warden (`rand() % 16 + 10`). The Warden's hull never goes below 0.
3. **Shields**: the next Warden attack this round is halved using integer division (`damage /= 2`). Shields reset at the start of every round.
4. **Repair** restores a random 15 to 30 (`rand() % 16 + 15`), capped at 100. *Approved change:* the feed and the floating number show the amount actually gained. At 95 hull a roll of 22 shows "+5".
5. **Retreat**: *approved change:* it first asks "Abandon the fight?" with Stay / Retreat. Confirming ends the battle as "Withdrawn" and goes back to Title.
6. If the Warden reaches 0 after the player's move, the battle ends in Victory immediately, and the Warden does not act.
7. Otherwise the Warden picks 1, 2 or 3 with equal chance:
   - **1, attack**: 10 to 25 damage to the Halberd, halved if the player raised Shields this round. The Halberd's hull never goes below 0.
   - **2, shields**: *approved change (her original code did nothing here):* the player's **next Fire** is halved with integer division. The shield bubble stays visible until that Fire lands or the player picks another command, which clears it.
   - **3, repair**: 15 to 30, capped at 100. The feed shows the actual amount gained.
8. If the Halberd reaches 0, the battle ends in Defeat.
9. Debrief stats: engagements played, total damage dealt, and hull left (Victory) or Warden hull (Defeat).

## User Stories

1. As a player, I want a title screen with the game name, my ship and a clear Start button, so that I know what the game is and how to begin.
2. As a player, I want the title screen to feel alive (moving stars, a drifting planet, a radar sweep, an engine flame), so that the game feels premium from the first second.
3. As a player, I want to open How to Play from the title, so that I learn the four commands before fighting.
4. As a player, I want How to Play to show each command with its icon, key and exact numbers, so that I can plan my moves.
5. As a player, I want How to Play to explain what the Warden can do and what the goal is, so that the enemy's moves make sense.
6. As a player, I want a Settings screen with sound effects on/off and music on/off, so that I can play quietly.
7. As a player, I want settings to apply immediately and to last until I close the tab, so that I set them once per visit.
8. As a player, I want to see both ships facing each other, so that the fight reads like a duel.
9. As a player, I want each ship's name, a portrait badge and a segmented hull bar with the number, so that I can read the state at a glance.
10. As a player, I want the hull bar to drop quickly and leave a fading "ghost" trail, so that I can see how much damage was just done.
11. As a player, I want a hull bar to blink when it is at 30 or below, so that I notice danger.
12. As a player, I want an engagement counter and a phase readout (Awaiting orders / Hostile targeting / Incoming), so that I always know whose turn it is.
13. As a player, I want four command buttons with icons, so that I can act with a click or tap.
14. As a player, I want keys 1 to 4 to trigger the same commands, so that I can play from the keyboard.
15. As a player, I want buttons to show hover, pressed and disabled states, so that the game feels responsive.
16. As a player, I want the buttons locked while the turn animates, so that I cannot spam inputs or act out of turn.
17. As a player, I want Fire to draw a beam to the Warden, flash and shake it, and pop a damage number, so that every hit feels good.
18. As a player, I want Shields to show a bubble around my ship, so that I know I am protected.
19. As a player, I want a blocked hit to show "SHIELDED" and use a softer deflect sound, so that I can see the shield worked.
20. As a player, I want Repair to show a green glow and a "+N" number, so that I see exactly what I gained.
21. As a player, I want a short "Hostile targeting" pause with a red alert wash, a red hazard tape and a klaxon before the Warden acts, so that the enemy turn has tension and I can follow it.
22. As a player, I want red lock-on brackets around the Warden during my turn, so that the screen tells me it is my move.
23. As a player, I want every move written to the tactical feed in a military voice, with the newest line on top and typed out, so that I can review what happened.
24. As a player, I want Retreat to ask for confirmation, so that I do not quit by accident.
25. As a player, I want the losing ship to explode or drift away when a battle ends, so that the ending is clear.
26. As a player, I want a Victory screen with a stamped title, rotating light rays, my ship and debrief stats that count up, so that winning feels rewarding.
27. As a player, I want a Defeat screen with a screen shake, red alarm wash, sparks and my ship tumbling, so that losing feels dramatic.
28. As a player, I want Play again / Try again and Main menu on both result screens, so that I can continue or go back.
29. As a player, I want Esc to go back from How to Play and Settings, so that the keyboard works everywhere.
30. As a player, I want sound effects for fire, hits, shields, repairs, the alarm, buttons, victory and defeat, so that the game feels alive.
31. As a player, I want low background music I can turn off, so that it sets the mood without annoying me.
32. As a player, I want sound to start after my first click, so that the browser does not block it and nothing blasts on load.
33. As a phone player, I want the game to scale to my screen and respond to taps, so that I can play on mobile.
34. As a phone player holding the phone upright, I want a "rotate your phone" hint, so that I get the full landscape view.
35. As a player, I want animations to stay smooth (60 fps), so that the game feels polished.
36. As a player who prefers less motion, I want a "Reduce motion" option in Settings that stops shakes and flicker, so that I can play comfortably.
37. As the client, I want the rules to match my original code plus the approved changes exactly, so that the game is still my game.
38. As the client, I want the code split into a few small, clearly named files with comments, so that I can explain it in class.
39. As the client, I want the game logic separate from the drawing code, so that I can show where my original rules live.
40. As the client, I want the built website files already included, so that I can run it in VS Code with Live Server without installing compilers.
41. As the client, I want one VS Code task to rebuild the web version, so that I can change the C++ and see the result.
42. As the client, I want a desktop build of the same code, so that I can test quickly and show it offline if the internet fails.
43. As the client, I want a README that explains how to run, build and host it (for example on GitHub Pages), so that I can submit a link.
44. As the client, I want the tests for the rules to run with one command, so that I can prove the logic is correct.
45. As the client, I want every asset to be free to use (CC0 or OFL) with credits listed, so that my submission is safe.

## Implementation Decisions

- **Stack:** C++17 and raylib 5.x. The web build uses Emscripten (`emcc`) with raylib compiled for `PLATFORM_WEB`, plus a native desktop build of the same source. The editor is VS Code, with tasks for build-web, build-desktop and test.
- **Code style for 1st/2nd year students:** plain structs, enums and free functions. No inheritance hierarchies, no templates, no smart-pointer gymnastics, and lambdas only where raylib needs them. Every file starts with a short comment saying what it does. Comments explain *why* and any raylib call a beginner would not know.
- **Main loop:** on the web, `emscripten_set_main_loop` calls one `UpdateDrawFrame` function. Desktop uses the usual `while (!WindowShouldClose())` loop calling the same function.
- **Fixed virtual resolution:** draw everything to a 1280x720 `RenderTexture2D`, then scale it to the window with letterboxing. Mouse and touch positions are mapped back into the 1280x720 space. All layout numbers in the design PNGs are in that space.
- **Modules:**
  1. **Battle logic (the only test seam).** Has no raylib include. It holds a `Battle` struct (hulls, engagement number, player shields flag, enemy shields flag, result, stats) and two functions: one applies a player command, and one runs the enemy's move. Each returns a short list of **events** (for example *fired 18*, *was shielded*, *repaired +5*, *enemy raised shields*, *victory*) that the presentation turns into animations, sounds and feed lines. Randomness comes from a small `Rng` struct passed in, seedable, so tests are deterministic. Its range helper reproduces `rand() % 16 + 10` style rolls.
  2. **Screens:** an enum state machine with Title, HowTo, Settings, Battle, Victory and Defeat. Retreat confirmation is an overlay inside Battle. Each screen has an update function and a draw function.
  3. **Turn sequencer:** a small timed queue inside Battle that plays the logic's events with the approved timing:
     - Player shot lands 0.13 s after the click.
     - "Hostile targeting" starts at 0.45 s.
     - The enemy acts at 0.95 s, and its shot lands 0.14 s later.
     - The round ends and buttons unlock 0.6 s after that.
     - Beams cross the gap in 0.1 s, then fade over 0.2 s.
     - The result screen follows about 0.7 s after a ship reaches 0.
  4. **UI kit:** drawing helpers shared by all screens:
     - Panels: a cut-corner (chamfer) panel with gradient border and header strip.
     - Buttons: a button with icon socket, hover sheen, pressed offset and disabled tint, plus the primary amber variant with pulsing glow.
     - Status pieces: segmented hull bar with ghost trail, hex portrait badge, lock-on brackets, scrolling hazard tape.
     - Text: typed-out text and occasional glitch text.
  5. **Background and effects:**
     - Background: three-layer parallax starfield, nebula glow and a planet drawn as a gradient circle with an amber rim.
     - Screen overlays: scanlines plus a slow light sweep, vignette, red alert wash.
     - Ship effects: engine flame flicker, beam, shield bubble, floating numbers, hit shake and flash, explosion, screen shake.
  6. **Audio:** sound effects and one music stream, with on/off flags from Settings. `InitAudioDevice` runs after the first user input on the web.
  7. **Content:** all player-facing strings (the feed lines in the Red Alert voice, button labels, screen text) live in one place, so wording changes in one place.
- **Feed lines (approved Red Alert voice):**
  - Battle start: "RED ALERT. Hostile Warden closing."
  - Fire: "WEAPONS FREE. N DMG CONFIRMED ON TARGET."
  - Shields: "SHIELDS TO FULL. Next hit will be halved."
  - Repair: "DAMAGE CONTROL. Hull patched +N."
  - Enemy attack: "WE ARE HIT. N DMG TO HULL."
  - Your shields absorb a hit: "Shields held. Damage halved."
  - Enemy raises shields: "Warden raising shields. Next shot halved."
  - Enemy shields absorb your shot: "Warden shields held. Our damage halved."
  - Enemy repair: "Warden running repairs. Hull +N."
  - Retreat: "Helm, full reverse. You left the battle."
  - Each line is prefixed with `[NN]`, the engagement number.
- **Palette:**

  | Hex | Role |
  |---|---|
  | `#0B0807` | Background |
  | `#FFB347` | Amber: the player, primary actions |
  | `#F29A2E` | Amber shade |
  | `#FF4433` | Red: the Warden, alarms |
  | `#9BE36B` | Green: repair |
  | `#F6E6CC` | Text |
  | `#AD977C` | Dim text |

  Panels use a vertical gradient from about `#241A12` to `#0F0B08`.
- **Fonts:** Rajdhani Bold for titles, labels and numbers. Share Tech Mono for the feed, codes and readouts. Both are SIL OFL from Google Fonts and load as TTF at the sizes used, since raylib rasterizes per size.
- **Assets:**
  - Ships and shield: Kenney Space Shooter Redux, CC0. `playerShip2_orange` is the Halberd, `enemyBlack3` is the Warden, `shield3` is the bubble. They are drawn rotated 90 degrees so the ships face each other.
  - Sounds: Kenney Sci-Fi Sounds and Interface Sounds, CC0.
  - Music: a CC0 loop. If none fits, a quiet generated drone.
  - Icons: Kenney Game Icons, CC0, or simple line icons drawn with raylib shapes.
  - Web packaging: assets are preloaded into the Emscripten virtual file system (`--preload-file`).
- **Web output:** a `web/` folder holding the `index.html` from a minimal custom Emscripten shell (dark background, centered canvas, no Emscripten logo or console), plus the `.js`, `.wasm` and `.data` files. The built output is committed so Live Server works with no toolchain.
- **Input:** mouse, touch (raylib maps a tap to a mouse press) and keyboard shortcuts. On Title: Enter, H and S. In Battle: 1, 2, 3, 4. On How to Play and Settings: Esc goes back. On result screens: R plays again, M goes to the menu.
- **Reduce motion setting:** turns off screen shake, glitch, CRT sweep and parallax drift. Damage feedback switches to flashes only. Ships keep only a gentle bob (no drift, lunge or recoil).

## Testing Decisions

- **One seam: the battle logic module.** Tests call its public functions with a seeded `Rng` and assert on the returned state and events. They never look inside how it computes them. Rendering, audio and screens are checked by eye against `docs/design/*.png`.
- **Test style:** one plain `assert`-based test program, built natively and run by the VS Code "test" task. There is no test framework, so a student can read it.
- **Cases to cover (from Game rules):**
  - Damage stays in the 10 to 25 range.
  - Hull never goes below 0.
  - Repair caps at 100 and reports the actual amount gained.
  - Player shields halve only that round's enemy hit, with integer division, and reset next round.
  - Enemy shields halve only the next Fire and then clear.
  - The Warden does not act after it reaches 0.
  - Victory and Defeat are detected.
  - Retreat ends the battle as Withdrawn.
  - Stats add up across a scripted sequence.
- **Prior art:** the mockups' headless play-test ran 200 random games per screen with exactly these invariants (hull in range, Defend lets at most 12 through, Repair never shows more than was missing, buttons lock every turn, every game ends). Port the same invariants as a randomized loop over many seeds, in addition to the targeted cases.

## Out of Scope

- The other three explored directions (Cyan Command, Neon Arcade, Deep Mission).
- Multiplayer, accounts, leaderboards, saves between visits, achievements.
- Difficulty levels or new moves beyond the four commands.
- Localization (English only).
- Any HTML/CSS/JavaScript UI. The only web file outside C++ is the generated loader page.

## Further Notes

- **Name and characters:** the client confirmed the name **Space Battle**. Ship names Halberd and Warden are placeholders she has seen and did not object to.
- **Expect a tougher enemy:** with the approved enemy shields, the Warden is noticeably tougher. In random-play simulation the win rate dropped to about 27%. If she asks for an easier game, tune it with constants, not rule changes.
- **Loading the web build:** browsers block WebAssembly from `file://`, so the web build has to be served (Live Server or GitHub Pages). Put that in the README.
- **Credits:** list them on the How to Play screen or in the README: Kenney (CC0), Rajdhani and Share Tech Mono (OFL), and the music source.
- **Decisions made while building:**
  - The professor's review asked for more ship movement and faster shots, so the turn timings were shortened and ships now move (see Ship motion).
  - Ship motion: each ship drifts in a slow figure-eight (about 52 px tall, 36 px wide), with different periods (6.0 s and 7.4 s) so they never move in sync, and a slight tilt (about 5 degrees) that follows the up and down motion. A ship lunges about 26 px toward the enemy when it fires and eases back, and is pushed about 18 px away from the attacker when hit. Flames, glows, shields, beams, floating numbers, lock-on brackets and sparks all follow the ship's current position. The first version was too subtle, so the motion was made bigger but slower so it glides. On the Battle screen the stars stream about 7x faster than on other screens, and the near stars become short streaks, to give a Galaga-like sense of both ships flying forward; the other screens keep the slow drift, and Reduce motion stops the streaming.
  - Settings has a third row, Reduce motion (ON / OFF), below the two audio rows shown in `docs/design/settings.png`.
  - The debrief header reads "AFTER ACTION" where the mockups said "SAMPLE NUMBERS".
  - In the Retreat dialog, Esc means Stay, and Enter or 4 means Retreat.
  - The feed's speaker icon switches sound effects and music on or off together.
  - Music is a quiet drone generated in code (`src/audio.cpp`), since no CC0 loop was picked.
  - "Damage dealt" adds up each Fire's rolled damage, including a final shot that overshoots 0.
  - The randomized test plays a random player that picks Fire 60% of the time. With equal odds, both ships out-repair the damage and a game can run for thousands of rounds. That player wins about 70% of games.
- **No issue tracker:** this spec lives in the repo. Run `/setup-matt-pocock-skills` to publish it to a tracker.
