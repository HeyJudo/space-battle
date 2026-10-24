// screenshots.cpp - a helper for checking the look, not part of the game.
// It opens each screen, lets its intro animations finish, and saves a
// 1280x720 PNG into build/shots/ to compare with docs/design/*.png.
// Run it with: make shots
#include "raylib.h"
#include "rlgl.h"
#include "assets.h"
#include "constants.h"
#include "game.h"
#include "viewport.h"

static void Capture(Screen screen, float seconds, const char* name, RenderTexture2D target) {
    GoToScreen(screen);
    for (float t = 0; t < seconds; t += 1.0f / 60) {
        BeginDrawing();          // keeps the frame timer ticking
        UpdateGame();
        BeginTextureMode(target);
        DrawGame();
        EndTextureMode();
        EndDrawing();
    }
    Image image = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&image);   // render textures come out upside down
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);  // drop alpha, like PresentVirtual does
    ExportImage(image, TextFormat("build/shots/%s.png", name));
    UnloadImage(image);
}

int main() {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(VIRTUAL_W, VIRTUAL_H, "screenshots");
    rlDisableBackfaceCulling();
    SetTargetFPS(60);
    InitViewport();
    LoadAssets();
    MakeDirectory("build/shots");
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_W, VIRTUAL_H);

    Capture(Screen::Title, 2.5f, "title", target);
    Capture(Screen::Battle, 1.0f, "battle", target);
    Capture(Screen::HowTo, 1.5f, "howto", target);
    Capture(Screen::Settings, 1.0f, "settings", target);
    debrief = { 7, 118, 42, 0 };   // the sample numbers drawn in the design PNGs
    Capture(Screen::Victory, 2.5f, "victory", target);
    debrief = { 9, 81, 0, 19 };
    Capture(Screen::Defeat, 2.5f, "defeat", target);

    UnloadRenderTexture(target);
    UnloadAssets();
    CloseWindow();
    return 0;
}
