# Space Battle

A turn-based space duel for a client who is a 1st or 2nd year student. It is all C++, built with raylib, compiled to WebAssembly and played in the browser. She presents and explains this code in class, so **readable beats clever**.

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

## Web build gotchas

- On the web, the frame loop runs through `emscripten_set_main_loop(UpdateDrawFrame, 0, 1)`. Desktop calls the same `UpdateDrawFrame` from a `while (!WindowShouldClose())` loop.
- Browsers only allow audio after a user gesture, so call `InitAudioDevice` on the first click or key press.
- Assets reach the web build through `--preload-file assets`, so load them by paths relative to `assets/`.
- WebAssembly only loads over http. Test the web build with VS Code Live Server, never by opening `index.html` directly.
- Commit the built `web/` output so the client can run the game without installing Emscripten.

## Done means

- The logic tests pass. They are plain `assert`s in one native test program.
- The desktop build runs.
- The web build runs under Live Server, and the screen matches its `docs/design` PNG.
- Every asset you add is CC0 or OFL and is credited in the README.
