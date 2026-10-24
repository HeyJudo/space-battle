// main.cpp - UI kit showcase (temporary: the real screens replace this in the next step)
#include "raylib.h"
#include "rlgl.h"
#include "assets.h"
#include "background.h"
#include "constants.h"
#include "ui.h"
#include "viewport.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

static float t = 0;
static Button fire, shields;
static HullBar bar;

void UpdateDrawFrame() {
#if defined(PLATFORM_WEB)
    double cssW, cssH;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    if ((int)cssW != GetScreenWidth() || (int)cssH != GetScreenHeight())
        SetWindowSize((int)cssW, (int)cssH);
#endif
    t += GetFrameTime();
    UpdateButton(fire, true);
    UpdateButton(shields, true);
    int hull = 100 - ((int)(t / 2) % 5) * 20;
    UpdateHullBar(bar, hull, GetFrameTime());

    BeginVirtual();
    ClearBackground(GetColor(COL_BG));
    DrawBackLayers(t, false);
    DrawPlanet({ 1180, 850 }, 430, t, false);
    DrawFrontLayers(t, false);
    DrawHazardTape(0, 12, false, 0.9f, t, true);
    DrawPanel({ 96, 356, 410, 276 }, "COMMAND", "SELECT ONE");
    DrawCornerTicks({ 96, 356, 410, 276 });
    DrawButton(fire, true, t);
    DrawButton(shields, true, t);
    DrawHullBar(bar, { 600, 100, 324, 18 }, false, t, false);
    DrawHullBar(bar, { 600, 140, 324, 18 }, true, t, false);
    DrawHexBadge({ 560, 220 }, 72, assets.halberd, PanelTone::Amber);
    DrawHexBadge({ 660, 220 }, 72, assets.warden, PanelTone::Red);
    DrawLockOn({ 920, 250, 180, 160 }, t, true);
    DrawShip(assets.warden, { 1010, 330 }, 150, 0, WHITE);
    DrawGlitchLabel("SPACE", 90, 112, 116, 0.03f, Col(COL_TEXT), t, true);
    DrawTyped(FontKind::Mono, "> HOSTILE CONTACT ......... DETECTED", 96, 300, 13, 0.12f, Col(COL_RED_SOFT), t, 0.2f, 0.9f, true);
    for (int i = 1; i <= 13; i++)
        DrawIcon((Icon)i, { 560.0f + (i - 1) * 50, 660 }, 26, Col(COL_AMBER));
    DrawScreenOverlays(t, false);
    EndVirtual();

    BeginDrawing();
    PresentVirtual();
    EndDrawing();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Space Battle");
    rlDisableBackfaceCulling();
    InitViewport();
    LoadAssets();
    fire = MakeButton({ 112, 410, 378, 62 }, "START BATTLE", "Engage the Warden", "ENTER", Icon::Play, true);
    shields = MakeButton({ 112, 482, 378, 62 }, "HOW TO PLAY", "Four moves, one goal", "H", Icon::Book, false);
    bar = MakeHullBar(100);
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) UpdateDrawFrame();
#endif
    UnloadAssets();
    UnloadViewport();
    CloseWindow();
    return 0;
}
