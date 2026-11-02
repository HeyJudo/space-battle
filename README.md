# Space Battle

A turn-based space duel for Windows, written entirely in C++ with [raylib](https://www.raylib.com/). It runs in its own window. Your ship, the **Halberd**, faces the **Warden**. Each engagement you pick one command, then the Warden answers. Bring its hull to 0 before yours runs out.

The approved look is in [`docs/design/`](docs/design/). The rules come from the original console game in [`reference/original_main.cpp`](reference/original_main.cpp), plus three approved changes:

- The Warden's Shields halve your next Fire.
- Repair shows the hull you actually gained.
- Retreat asks before you leave.

[`SPEC.md`](SPEC.md) has every rule, timing and text line.

## Play it

1. Open [Releases](https://github.com/HeyJudo/space-battle/releases) and download **SpaceBattle.zip** from the latest release.
2. Right-click the zip, choose **Extract All**, then **Extract**.
3. Double-click **space-battle.exe** in the extracted folder. Keep the `assets` folder next to it.
4. If Windows shows "Windows protected your PC", click **More info**, then **Run anyway**. It appears because the game isn't signed.

To build the `.exe` yourself, see [Build it yourself](#build-it-yourself).

| Where | Keys |
|---|---|
| Title | Enter: start, H: how to play, S: settings |
| Battle | 1 Fire, 2 Shields, 3 Repair, 4 Retreat |
| Retreat dialog | Esc: stay, Enter or 4: retreat |
| How to Play, Settings | Esc: back |
| Victory, Defeat | R: play again, M: main menu |

Mouse clicks work everywhere too. Sound starts after your first click or key press, so nothing blasts on launch. The window can be resized; keep it wider than it is tall.

## How the code is organized

| File | What it does |
|---|---|
| `src/battle.h/.cpp` | **The rules.** No drawing, no raylib. `PlayerMove` and `WardenMove` change the `Battle` and return a list of events. |
| `src/rng.h` | A small random number generator you can seed, so the tests always roll the same dice. |
| `src/constants.h` | Every game number: damage ranges, timings, screen size, colors. |
| `src/strings.h` | Every piece of text the player sees, including the tactical feed lines. |
| `src/main.cpp` | Opens the window and runs the frame loop. |
| `src/game.h/.cpp` | Which screen is showing, the settings, and the debrief numbers. |
| `src/screen_*.cpp` | One file per screen. Each has `Enter`, `Update` (input and timers) and `Draw`. |
| `src/screen_battle.cpp` | Plays each round back with timed steps, and turns rule events into animations, sounds and feed lines. |
| `src/ui.h/.cpp` | The drawing kit: cut-corner panels, buttons, hull bars, hex badges, hazard tape, icons, text. |
| `src/background.h/.cpp` | Stars, nebula, planet, red alert wash, scanlines and vignette. |
| `src/viewport.h/.cpp` | Draws at 1280x720, then scales that to fit any window size. |
| `src/assets.h/.cpp`, `src/audio.h/.cpp` | Loading fonts and sprites; sound effects and the music drone. |
| `tests/test_battle.cpp` | The rule tests (plain `assert`s). |
| `tools/screenshots.cpp` | Saves a picture of every screen, to compare with `docs/design/`. |

## Build it yourself

### One-time installs (Windows)

1. **w64devkit and raylib 5.5.** Make a folder `C:\raylib`, then download into it:
   - `w64devkit-x64-2.x.x.7z.exe` from [w64devkit releases](https://github.com/skeeto/w64devkit/releases). Run it and extract into `C:\raylib`, which gives `C:\raylib\w64devkit`. It contains `g++` and `make`.
   - `raylib-5.5_win64_mingw-w64.zip` from [raylib 5.5](https://github.com/raysan5/raylib/releases/tag/5.5). Unzip it into `C:\raylib`.
If you put these somewhere else, change the path at the top of the `Makefile`, and in `.vscode/tasks.json` and `.vscode/c_cpp_properties.json`.

### VS Code tasks

Open **Terminal > Run Task...** and pick one:

- **test** runs the rule tests. It prints `All battle tests passed.` when every rule holds.
- **build-desktop** builds `bin/space-battle.exe` (Ctrl+Shift+B). **run-desktop** builds it and starts it. F5 starts it in the debugger.

The tasks run `make`, so the same commands work in a terminal that has `C:\raylib\w64devkit\bin` on its PATH:

```bash
make test
```

```bash
make run
```

```bash
make shots
```

`make shots` saves 1280x720 pictures of every screen into `build/shots/`.

## Credits

| Asset | Author | License |
|---|---|---|
| Ship and shield sprites ([Space Shooter Redux](https://kenney.nl/assets/space-shooter-redux)) | Kenney | CC0 |
| Sound effects ([Sci-Fi Sounds](https://kenney.nl/assets/sci-fi-sounds), [Interface Sounds](https://kenney.nl/assets/interface-sounds)) | Kenney | CC0 (license files in `assets/sounds/`) |
| [Rajdhani](https://fonts.google.com/specimen/Rajdhani) font | Indian Type Foundry | SIL Open Font License (`assets/fonts/OFL-Rajdhani.txt`) |
| [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono) font | Carrois Apostrophe | SIL Open Font License (`assets/fonts/OFL-ShareTechMono.txt`) |
| Background music | Generated in code (`src/audio.cpp`) | Part of this project |
| [raylib](https://github.com/raysan5/raylib) | Ramon Santamaria and contributors | zlib/libpng |
