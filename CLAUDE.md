# Space Battle

A turn-based space duel for a client who is a 1st or 2nd year student. It is all C++, built with raylib, and runs as a native Windows desktop app (no web build). She presents and explains this code in class, so **readable beats clever**.

## Read first

- `SPEC.md` is the source of truth for the game rules, screens, timing, feed wording, palette, assets and tests. Read it before starting any feature, and update it whenever a decision changes.
- `docs/design/*.png` is the approved look (Red Alert v2), drawn at 1280x720. Match it when building or changing any screen.
- `docs/design/ra2-reference.css` describes each visual trait in CSS. Translate it into raylib drawing calls; it is a reference, not shipped code.
- `reference/original_main.cpp` is the client's original console game. Her rules come from here, plus the approved changes listed in `SPEC.md`.

## Conventions

- **Logic and drawing stay separate.** The battle logic module includes no raylib headers and returns *events*. Screens turn events into animations, sounds and feed lines. The logic module is the only place tests touch.
- **Beginner C++17:** plain structs, enums and free functions, one responsibility per file, and a header comment on every file saying what it is for. Comment the *why* and any raylib call a student would not recognize.
- **One home per value:** game numbers (damage ranges, timings, colors) live in one constants header, and player-facing text lives in one strings file.
- **Draw in virtual space:** render to a 1280x720 render texture, then scale it to the window with letterboxing. Map mouse and touch input back into 1280x720 before hit-testing.
- **Randomness goes through the seedable `Rng`** passed into the logic, so tests stay deterministic.
- Use the glossary words from `SPEC.md` everywhere, in code and UI: Halberd, Warden, hull, Fire / Shields / Repair / Retreat, engagement, tactical feed, debrief.

## Done means

- The logic tests pass. They are plain `assert`s in one native test program.
- The desktop build runs, and the screen matches its `docs/design` PNG.
- Every asset you add is CC0 or OFL and is credited in the README.
