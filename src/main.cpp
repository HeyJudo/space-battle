// main.cpp - starts raylib, loads everything and runs the frame loop.
// The same UpdateDrawFrame runs on desktop (a normal loop) and on the web
// (the browser calls it once per screen refresh).
#include "raylib.h"
#include "rlgl.h"
#include "assets.h"
#include "audio.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"
#include "viewport.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

// Shown instead of the game while a phone is held upright.
static void DrawRotateHint() {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    DrawRectangle(0, 0, (int)w, (int)h, Col(COL_BG, 0.94f));
    float size = w / 12;
    DrawIcon(Icon::Restart, { w / 2, h / 2 - size * 1.4f }, size * 1.6f, Col(COL_AMBER));
    DrawLabelCentered(FontKind::Display, TXT_ROTATE, w / 2, h / 2, size * 0.75f, 0.1f, Col(COL_TEXT));
    DrawLabelCentered(FontKind::Mono, TXT_ROTATE_HINT, w / 2, h / 2 + size, size * 0.4f, 0.08f, Col(COL_DIM));
}

void UpdateDrawFrame() {
#if defined(PLATFORM_WEB)
    // Keep the canvas the same size as the browser window (the page CSS stretches it).
    double cssW, cssH;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    if ((int)cssW != GetScreenWidth() || (int)cssH != GetScreenHeight())
        SetWindowSize((int)cssW, (int)cssH);
#endif

    // Browsers only allow sound after the player interacts with the page.
    bool anyInput = GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetTouchPointCount() > 0;
    if (anyInput && !AudioStarted())
        StartAudio();

    UpdateGame();
    UpdateAudio();

    BeginVirtual();
    DrawGame();
    EndVirtual();

    BeginDrawing();
    PresentVirtual();
    if (IsPortrait())
        DrawRotateHint();
    EndDrawing();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(VIRTUAL_W, VIRTUAL_H, "Space Battle");
    SetExitKey(KEY_NULL);        // Esc means "back" in this game, not "quit"
    rlDisableBackfaceCulling();  // our gradient shapes don't care which way their corners wind

#if !defined(PLATFORM_WEB)
    // Started from the bin folder (for example by double-clicking)? The assets are one level up.
    if (!DirectoryExists("assets"))
        ChangeDirectory(TextFormat("%s/..", GetApplicationDirectory()));
#endif

    InitViewport();
    LoadAssets();
    StartGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
        UpdateDrawFrame();
#endif

    StopAudio();
    UnloadAssets();
    UnloadViewport();
    CloseWindow();
    return 0;
}
