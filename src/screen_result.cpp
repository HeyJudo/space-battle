// screen_result.cpp - the Victory and Defeat screens. Same layout: a typed
// status line, a big stamped title, the debrief stats counting up, and
// Play again / Main menu. Victory shines amber; Defeat shakes in red.
#include "raylib.h"
#include "audio.h"
#include "background.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"
#include <cmath>

static Button againButton, menuButton;
static bool won;

void EnterResult() {
    won = (CurrentScreen() == Screen::Victory);
    againButton = MakeButton({ 96, 450, 250, 62 }, won ? TXT_PLAY_AGAIN : TXT_TRY_AGAIN, nullptr, "R", Icon::Restart, true);
    menuButton = MakeButton({ 358, 450, 250, 62 }, TXT_MAIN_MENU, nullptr, "M", Icon::Home, false);
    PlaySfx(won ? Sfx::Victory : Sfx::Defeat);
}

void UpdateResult() {
    bool again = UpdateButton(againButton, true) || IsKeyPressed(KEY_R);
    bool menu = UpdateButton(menuButton, true) || IsKeyPressed(KEY_M);
    if (again || menu)
        PlaySfx(Sfx::Click);
    if (again) GoToScreen(Screen::Battle);
    else if (menu) GoToScreen(Screen::Title);
}

// Light rays turning slowly behind the winning ship.
static void DrawRays(Vector2 c, float now, bool animate) {
    float turn = animate ? now / 50.0f * 360.0f : 0.0f;
    for (int i = 0; i < 20; i++) {
        float a0 = (turn + i * 18.0f) * DEG2RAD;
        float a1 = (turn + i * 18.0f + 6.0f) * DEG2RAD;
        Vector2 p0 = { c.x + cosf(a0) * 380, c.y + sinf(a0) * 380 };
        Vector2 p1 = { c.x + cosf(a1) * 380, c.y + sinf(a1) * 380 };
        FillTriangleFade(c, p0, p1, Col(COL_AMBER, 0.16f), Col(COL_AMBER, 0.0f));
    }
}

// The title "stamp": 0 to 1 with a small overshoot, like a rubber stamp hitting paper.
static float StampProgress(float t) {
    float p = (t - 0.15f) / 0.55f;
    if (p < 0) p = 0;
    if (p > 1) p = 1;
    float q = p - 1;
    return 1 + 2.70158f * q * q * q + 1.70158f * q * q;  // "ease out back"
}

// The three debrief numbers, counting up from zero after the title lands.
static void DrawDebrief(Rectangle panel, float t) {
    float k = EaseIn(t, 0.8f, 0.75f);
    int lastValue = won ? debrief.playerHull : debrief.wardenHull;
    const char* labels[3] = { TXT_TURNS, TXT_DAMAGE_DEALT, won ? TXT_HULL_LEFT : TXT_WARDEN_HULL };
    int values[3] = { debrief.engagements, debrief.damageDealt, lastValue };
    Color valueColors[3] = { Col(COL_TEXT), Col(COL_TEXT), won ? Col(COL_GREEN) : Col(COL_RED_SOFT) };
    float colW = panel.width / 3;
    for (int i = 0; i < 3; i++) {
        float x = panel.x + colW * i;
        if (i > 0)
            DrawRectangleRec({ x, panel.y + PANEL_HEADER_H + 1, 1, panel.height - PANEL_HEADER_H - 2 }, Col(won ? COL_AMBER : COL_RED, won ? 0.2f : 0.25f));
        int shown = (int)roundf(values[i] * k);
        const char* text = (i == 0) ? TextFormat("%02d", shown) : TextFormat("%d", shown);
        DrawLabel(FontKind::Mono, labels[i], x + 20, panel.y + PANEL_HEADER_H + 14, 12, 0.16f, Col(COL_DIM));
        DrawLabel(FontKind::Display, text, x + 20, panel.y + PANEL_HEADER_H + 25.5f, 44, 0.0f, valueColors[i]);
    }
}

void DrawResult() {
    float now = (float)GetTime();
    float t = ScreenTime();
    bool motion = !settings.reduceMotion;
    unsigned int tone = won ? COL_AMBER : COL_RED;

    ClearBackground(GetColor(COL_BG));

    // Defeat opens with a hard screen shake.
    Camera2D camera = { };
    camera.zoom = 1.0f;
    if (!won && motion && t > 0.1f && t < 0.8f) {
        float k = 1.0f - (t - 0.1f) / 0.7f;
        camera.offset = { sinf(t * 70) * 12 * k, cosf(t * 55) * 6 * k };
    }
    BeginMode2D(camera);

    DrawBackLayers(now, settings.reduceMotion);
    if (won) {
        Vector2 c = { 870, 330 };
        DrawRays(c, now, motion);
        float breathe = motion ? Pulse(now, 6.0f) : 0.5f;
        FillEllipseGradient(c, 147 + 10 * breathe, 147 + 10 * breathe, c, Col(0xFFECC8FF, 0.5f), Col(COL_AMBER, 0.0f));
    } else {
        // Pulsing red alarm wash and falling sparks.
        float pulse = motion ? 1.0f - 0.55f * Pulse(now, 1.4f) : 0.8f;
        Vector2 top = { 640, 0 };
        FillEllipseGradient(top, 800, 450, top, Col(COL_RED, 0.34f * pulse), Col(COL_RED, 0));
        Vector2 side = { 896, 432 };
        FillEllipseGradient(side, 700, 400, side, Col(COL_RED, 0.12f * pulse), Col(COL_RED, 0));
        float fall = motion ? fmodf(now / 3.0f, 1.0f) : 0.0f;
        for (float x = -160; x < VIRTUAL_W + 160; x += 160) {
            for (float y = -280; y < VIRTUAL_H + 140; y += 140) {
                float ox = x - fall * 160, oy = y + fall * 280;
                DrawCircleV({ ox + 32, oy + 28 }, 0.8f, Col(0xFF8B5AFF, 0.7f));
                DrawCircleV({ ox + 112, oy + 70 }, 0.6f, Col(COL_AMBER, 0.7f));
                DrawCircleV({ ox + 64, oy + 112 }, 0.8f, Col(COL_RED, 0.7f));
            }
        }
    }
    DrawFrontLayers(now, settings.reduceMotion);
    DrawHazardTape(0, 12, !won, 0.9f, now, motion);

    if (won) {
        float bob = motion ? -8 * Pulse(now, 4.6f) : 0;
        Vector2 ship = { 900, 330 + bob };
        DrawGlow(ship, 130, Col(0xFFC878FF, 0.3f));
        DrawFlame({ ship.x - 66, ship.y }, 90, 22, true, now, motion);
        DrawShip(assets.halberd, ship, 200, 0, WHITE);
    } else {
        // The Halberd tumbles, dark and burnt; the Warden holds the sector.
        float spin = motion ? 120 + fmodf(now / 26.0f, 1.0f) * 360 : 140;
        DrawShip(assets.halberdWreck, { 970, 350 }, 180, spin - 90, { 200, 170, 160, 150 });
        float bob = motion ? -8 * Pulse(now, 4.6f) : 0;
        Vector2 warden = { 1130, 126 + bob };
        DrawFlame({ warden.x + 34, warden.y - 6 }, 50, 16, false, now, motion);
        DrawGlow(warden, 50, Col(COL_RED, 0.3f));
        DrawShip(assets.warden, warden, 68, 0, WHITE);
        // Hairline cracks across the screen.
        Color crack = Col(0xFFD6C9FF, 0.22f);
        DrawLineV({ 940, 280 }, { 1200, 80 }, crack);
        DrawLineV({ 1010, 150 }, { 1180, 410 }, crack);
        DrawLineV({ 880, 310 }, { 1100, 230 }, crack);
    }

    // Status line, typed out.
    DrawTyped(FontKind::Mono, won ? TXT_VICTORY_LINE : TXT_DEFEAT_LINE, 96, 96, 15, 0.14f,
              won ? Col(COL_AMBER) : Col(COL_RED_SOFT), t, 0.5f, 0.9f, false);

    // The title lands like a rubber stamp: big and blurry, then snapping into place.
    const char* title = won ? TXT_VICTORY : TXT_DEFEAT;
    float stamp = StampProgress(t);
    float titleW = MeasureLabel(FontKind::Display, title, 150, 0.04f);
    Vector2 titleCenter = { 96 + titleW / 2, 190 };
    FillEllipseGradient(titleCenter, titleW * 0.6f, 90, titleCenter, Col(tone, 0.12f * (stamp > 1 ? 1 : stamp)), Col(tone, 0));
    Camera2D stampCam = camera;
    stampCam.target = titleCenter;
    stampCam.offset = { titleCenter.x + camera.offset.x, titleCenter.y + camera.offset.y };
    stampCam.zoom = 1.0f + 0.7f * (1.0f - stamp);
    EndMode2D();
    BeginMode2D(stampCam);
    SetUiFade(stamp > 1 ? 1 : stamp);
    DrawGlitchLabel(title, 96, 99, 150, 0.04f, Col(tone), now, motion);
    SetUiFade(1);
    EndMode2D();
    BeginMode2D(camera);

    float rise = EaseIn(t, 0.7f, 0.7f);
    SetUiFade(rise);
    Rectangle panel = { 96, 292 + 18 * (1 - rise), 520, 128 };
    DrawPanel(panel, TXT_DEBRIEF, TXT_DEBRIEF_RIGHT, won ? PanelTone::Amber : PanelTone::Red);
    DrawDebrief(panel, t);

    rise = EaseIn(t, 0.9f, 0.7f);
    SetUiFade(rise);
    Button again = againButton, menu = menuButton;
    again.rect.y += 18 * (1 - rise);
    menu.rect.y += 18 * (1 - rise);
    DrawButton(again, true, now);
    DrawButton(menu, true, now);
    SetUiFade(1);

    EndMode2D();
    DrawScreenOverlays(now, settings.reduceMotion);
}
