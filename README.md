# Space Battle

A turn-based space duel in the browser, written entirely in C++ with [raylib](https://www.raylib.com/) and compiled to WebAssembly with Emscripten. Your ship, the **Halberd**, faces the **Warden**. Each engagement you pick one command, then the Warden answers. Bring its hull to 0 before yours runs out.

The approved look is in [`docs/design/`](docs/design/). The rules come from the original console game in [`reference/original_main.cpp`](reference/original_main.cpp), plus three approved changes:

- The Warden's Shields halve your next Fire.
- Repair shows the hull you actually gained.
- Retreat asks before you leave.

[`SPEC.md`](SPEC.md) has every rule, timing and text line.

## Play it

The built game is already in [`web/`](web/), so you can play it without installing any compilers.

1. Open this folder in VS Code and install the **Live Server** extension (VS Code suggests it when you open the folder).
2. Right-click `web/index.html` and choose **Open with Live Server**.

Browsers only run WebAssembly from a web server, so double-clicking `index.html` shows a blank page. Use Live Server, or any local server (for example `python -m http.server` inside `web/`).

| Where | Keys |
|---|---|
| Title | Enter: start, H: how to play, S: settings |
| Battle | 1 Fire, 2 Shields, 3 Repair, 4 Retreat |
| Retreat dialog | Esc: stay, Enter or 4: retreat |
| How to Play, Settings | Esc: back |
| Victory, Defeat | R: play again, M: main menu |

Mouse clicks and phone taps work everywhere too. Sound starts after your first click or key press, because browsers block sound until then. On a phone, turn it sideways.

## How the code is organized

| File | What it does |
|---|---|
| `src/battle.h/.cpp` | **The rules.** No drawing, no raylib. `PlayerMove` and `WardenMove` change the `Battle` and return a list of events. |
| `src/rng.h` | A small random number generator you can seed, so the tests always roll the same dice. |
| `src/constants.h` | Every game number: damage ranges, timings, screen size, colors. |
| `src/strings.h` | Every piece of text the player sees, including the tactical feed lines. |
| `src/main.cpp` | Opens the window and runs the frame loop (a normal loop on desktop, the browser's loop on the web). |
| `src/game.h/.cpp` | Which screen is showing, the settings, and the debrief numbers. |
| `src/screen_*.cpp` | One file per screen. Each has `Enter`, `Update` (input and timers) and `Draw`. |
| `src/screen_battle.cpp` | Plays each round back with timed steps, and turns rule events into animations, sounds and feed lines. |
| `src/ui.h/.cpp` | The drawing kit: cut-corner panels, buttons, hull bars, hex badges, hazard tape, icons, text. |
| `src/background.h/.cpp` | Stars, nebula, planet, red alert wash, scanlines and vignette. |
| `src/viewport.h/.cpp` | Draws at 1280x720, then scales that to fit any window or phone. |
| `src/assets.h/.cpp`, `src/audio.h/.cpp` | Loading fonts and sprites; sound effects and the music drone. |
| `src/shell.html` | The small page that loads the game in the browser. It is the only file that isn't C++. |
| `tests/test_battle.cpp` | The rule tests (plain `assert`s). |
| `tools/screenshots.cpp` | Saves a picture of every screen, to compare with `docs/design/`. |

## Build it yourself

### One-time installs (Windows)

1. **w64devkit and raylib 5.5.** Make a folder `C:\raylib`, then download into it:
   - `w64devkit-x64-2.x.x.7z.exe` from [w64devkit releases](https://github.com/skeeto/w64devkit/releases). Run it and extract into `C:\raylib`, which gives `C:\raylib\w64devkit`. It contains `g++` and `make`.
   - `raylib-5.5_win64_mingw-w64.zip` and `raylib-5.5_webassembly.zip` from [raylib 5.5](https://github.com/raysan5/raylib/releases/tag/5.5). Unzip both into `C:\raylib`.
2. **Emscripten**, only needed for the web build. In a terminal:

   ```bash
   git clone https://github.com/emscripten-core/emsdk.git C:/emsdk
   ```

   ```bash
   C:/emsdk/emsdk.bat install latest
   ```

   ```bash
   C:/emsdk/emsdk.bat activate latest
   ```

If you put these somewhere else, change the three paths at the top of the `Makefile`, and in `.vscode/tasks.json` and `.vscode/c_cpp_properties.json`.

### VS Code tasks

Open **Terminal > Run Task...** and pick one:

- **test** runs the rule tests. It prints `All battle tests passed.` when every rule holds.
- **build-desktop** builds `bin/space-battle.exe` (Ctrl+Shift+B). **run-desktop** builds it and starts it. F5 starts it in the debugger.
- **build-web** rebuilds `web/` from the C++. Reload Live Server to see your change.

The tasks run `make`, so the same commands work in a terminal that has `C:\raylib\w64devkit\bin` on its PATH:

```bash
make test
```

```bash
make run
```

```bash
make web
```

```bash
make shots
```

`make shots` saves 1280x720 pictures of every screen into `build/shots/`.

## Put it online with GitHub Pages

GitHub Pages can serve the `web/` folder as a website, which gives you a link to submit. Two things to know first:

- Pages on a **private** repository needs a paid GitHub plan. On a free account, make the repository public first (Settings > General > Danger Zone > Change visibility).
- Pages serves either the root of a branch or a `docs/` folder, not `web/`. So copy `web/` onto its own branch named `gh-pages`.

1. Build and commit `web/` (run **build-web**, then commit).
2. Push the `web/` folder to a `gh-pages` branch:

   ```bash
   git subtree push --prefix web origin gh-pages
   ```

3. On GitHub, open **Settings > Pages**. Under "Build and deployment", choose **Deploy from a branch**, then branch `gh-pages` and folder `/ (root)`, and save.
4. After a minute the game is live at `https://<your-username>.github.io/space-battle/`.

Repeat step 2 whenever you rebuild the game.

## Credits

| Asset | Author | License |
|---|---|---|
| Ship and shield sprites ([Space Shooter Redux](https://kenney.nl/assets/space-shooter-redux)) | Kenney | CC0 |
| Sound effects ([Sci-Fi Sounds](https://kenney.nl/assets/sci-fi-sounds), [Interface Sounds](https://kenney.nl/assets/interface-sounds)) | Kenney | CC0 (license files in `assets/sounds/`) |
| [Rajdhani](https://fonts.google.com/specimen/Rajdhani) font | Indian Type Foundry | SIL Open Font License (`assets/fonts/OFL-Rajdhani.txt`) |
| [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono) font | Carrois Apostrophe | SIL Open Font License (`assets/fonts/OFL-ShareTechMono.txt`) |
| Background music | Generated in code (`src/audio.cpp`) | Part of this project |
| [raylib](https://github.com/raysan5/raylib) | Ramon Santamaria and contributors | zlib/libpng |
