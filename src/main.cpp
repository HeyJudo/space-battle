// main.cpp - starts raylib, loads everything and runs the frame loop.
#include "raylib.h"
#include "rlgl.h"
#include "assets.h"
#include "audio.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"
#include "viewport.h"

// Shown instead of the game while the window is taller than it is wide.
static void DrawRotateHint() {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    DrawRectangle(0, 0, (int)w, (int)h, Col(COL_BG, 0.94f));
    float size = w / 12;
    DrawIcon(Icon::Restart, { w / 2, h / 2 - size * 1.4f }, size * 1.6f, Col(COL_AMBER));
    DrawLabelCentered(FontKind::Display, TXT_ROTATE, w / 2, h / 2, size * 0.75f, 0.1f, Col(COL_TEXT));
    DrawLabelCentered(FontKind::Mono, TXT_ROTATE_HINT, w / 2, h / 2 + size, size * 0.4f, 0.08f, Col(COL_DIM));
}

void UpdateDrawFrame() {
    // Sound waits for the first click or key press, so nothing blasts on launch.
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

    // Started from the bin folder (for example by double-clicking)? The assets are one level up.
    if (!DirectoryExists("assets"))
        ChangeDirectory(TextFormat("%s/..", GetApplicationDirectory()));

    InitViewport();
    LoadAssets();
    StartGame();

    SetTargetFPS(60);
    while (!WindowShouldClose())
        UpdateDrawFrame();

    StopAudio();
    UnloadAssets();
    UnloadViewport();
    CloseWindow();
    return 0;
}
