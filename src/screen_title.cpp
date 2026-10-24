// screen_title.cpp - the Title screen: game name, the Halberd on a radar,
// a distant Warden contact, and the Start / How to Play / Settings menu.
#include "raylib.h"
#include "audio.h"
#include "background.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"
#include <cmath>

static Button startButton, howToButton, settingsButton;

void EnterTitle() {
    startButton = MakeButton({ 112, 410, 378, 62 }, TXT_START, TXT_START_HINT, "ENTER", Icon::Play, true);
    howToButton = MakeButton({ 112, 482, 378, 62 }, TXT_HOWTO, TXT_HOWTO_HINT, "H", Icon::Book, false);
    settingsButton = MakeButton({ 112, 554, 378, 62 }, TXT_SETTINGS, TXT_SETTINGS_HINT, "S", Icon::Gear, false);
}

void UpdateTitle() {
    bool start = UpdateButton(startButton, true) || IsKeyPressed(KEY_ENTER);
    bool howTo = UpdateButton(howToButton, true) || IsKeyPressed(KEY_H);
    bool openSettings = UpdateButton(settingsButton, true) || IsKeyPressed(KEY_S);
    if (start || howTo || openSettings)
        PlaySfx(Sfx::Click);
    if (start) GoToScreen(Screen::Battle);
    else if (howTo) GoToScreen(Screen::HowTo);
    else if (openSettings) GoToScreen(Screen::Settings);
}

// Rings and a turning radar sweep behind the ship.
static void DrawRadar(Vector2 c, float now, bool animate) {
    // Dashed outer ring that turns slowly.
    float turn = animate ? now / 40.0f * 360.0f : 0.0f;
    for (int i = 0; i < 120; i++) {
        float a = turn + i * 3.0f;
        DrawRing(c, 209.5f, 210.5f, a, a + 1.5f, 2, Col(COL_AMBER, 0.22f));
    }
    DrawRing(c, 149.5f, 150.5f, 0, 360, 96, Col(COL_AMBER, 0.22f));
    DrawRing(c, 59.5f, 60.5f, 0, 360, 64, Col(COL_AMBER, 0.35f));

    // The sweep: a wedge that is brightest at its leading edge and fades over 55 degrees.
    float lead = (animate ? now / 3.6f * 360.0f : 0.0f) - 90.0f;  // -90 = straight up
    const int SLICES = 22;
    for (int i = 0; i < SLICES; i++) {
        float a0 = lead + 55.0f * i / SLICES;
        float a1 = lead + 55.0f * (i + 1) / SLICES;
        float alpha = 0.32f * (1.0f - (float)i / SLICES);
        DrawCircleSector(c, 176, a0, a1, 2, Col(COL_AMBER, alpha));
    }
}

// The little equalizer bars in the footer.
static void DrawEqualizer(float x, float bottom, float now, bool animate) {
    const float offsets[5] = { 0.0f, -0.3f, -0.6f, -0.15f, -0.45f };
    for (int i = 0; i < 5; i++) {
        float k = animate ? Pulse(now - offsets[i], 1.8f) : 0.5f;
        float h = 3 + 13 * k;
        DrawRectangleRec({ x + i * 7, bottom - h, 4, h }, Col(COL_AMBER));
    }
}

void DrawTitle() {
    float now = (float)GetTime();
    float t = ScreenTime();
    bool motion = !settings.reduceMotion;

    ClearBackground(GetColor(COL_BG));
    DrawBackLayers(now, settings.reduceMotion);
    // The planet drifts very slowly.
    Vector2 planet = { 1180 + (motion ? sinf(now / 9) * 6 : 0), 850 + (motion ? cosf(now / 11) * 4 : 0) };
    DrawPlanet(planet, 430, now, settings.reduceMotion);
    DrawFrontLayers(now, settings.reduceMotion);
    DrawHazardTape(0, 12, false, 0.9f, now, motion);

    // Boot lines type themselves out one after another.
    DrawTyped(FontKind::Mono, TXT_BOOT_1, 96, 44, 13, 0.12f, Col(COL_DIM), t, 0.1f, 0.9f, false);
    DrawTyped(FontKind::Mono, TXT_BOOT_2, 96, 64, 13, 0.12f, Col(COL_DIM), t, 0.6f, 0.9f, false);
    DrawTyped(FontKind::Mono, TXT_BOOT_3, 96, 84, 13, 0.12f, Col(COL_RED_SOFT), t, 1.1f, 0.9f, true);

    // Title, tagline and menu rise into place one after another.
    float rise = EaseIn(t, 0.2f, 0.7f);
    SetUiFade(rise);
    float dy = 18 * (1 - rise);
    DrawGlitchLabel(TXT_TITLE_1, 90, 88 + dy, 116, 0.03f, Col(COL_TEXT), now, motion);
    DrawGlitchLabel(TXT_TITLE_2, 90, 188 + dy, 116, 0.03f, Col(COL_AMBER), now + 0.4f, motion);

    rise = EaseIn(t, 0.35f, 0.7f);
    SetUiFade(rise);
    dy = 18 * (1 - rise);
    DrawRectangleRec({ 96, 327 + dy, 56, 3 }, Col(COL_AMBER));
    DrawLabel(FontKind::Mono, TXT_TAGLINE, 164, 322 + dy, 13, 0.14f, Col(COL_DIM));

    rise = EaseIn(t, 0.45f, 0.7f);
    SetUiFade(rise);
    dy = 18 * (1 - rise);
    Rectangle menu = { 96, 356 + dy, 410, 276 };
    DrawPanel(menu, TXT_MENU, TXT_MENU_RIGHT);
    Button start = startButton, howTo = howToButton, settingsB = settingsButton;
    start.rect.y += dy; howTo.rect.y += dy; settingsB.rect.y += dy;
    DrawButton(start, true, now);
    DrawButton(howTo, true, now);
    DrawButton(settingsB, true, now);
    SetUiFade(1);

    // The Halberd on its radar.
    Vector2 radar = { 910, 360 };
    DrawRadar(radar, now, motion);
    float bob = motion ? -8 * Pulse(now, 4.6f) : 0;
    Vector2 ship = { 930, 360 + bob };
    DrawGlow(ship, 120, Col(COL_AMBER, 0.2f));
    DrawFlame({ ship.x - 56, ship.y }, 70, 22, true, now, motion);
    DrawShip(assets.halberd, ship, 200, 0, WHITE);

    // A distant Warden contact, locked on.
    Vector2 contact = { 1125, 115 };
    DrawGlow(contact, 44, Col(COL_RED, 0.3f));
    DrawShip(assets.warden, contact, 58, 0, WHITE);
    DrawLockOn({ 1074, 68, 102, 94 }, now, motion);
    Color alert = Blink(now, 1.2f) ? Col(0xFF6A58FF) : Col(0xFF6A58FF, 0.35f);
    DrawLabelRight(FontKind::Mono, TXT_CONTACT_1, 1184, 166, 12, 0.14f, alert);
    DrawLabelRight(FontKind::Mono, TXT_CONTACT_2, 1184, 180, 12, 0.14f, alert);

    // Footer.
    DrawLabel(FontKind::Mono, TXT_FOOTER_LEFT, 96, 672, 12, 0.14f, Col(COL_DIM));
    DrawEqualizer(624, 686, now, motion);
    DrawLabelRight(FontKind::Mono, TXT_FOOTER_RIGHT, 1184, 672, 12, 0.14f, Col(COL_DIM));
    DrawHazardTape(708, 12, false, 0.6f, now, motion);

    DrawScreenOverlays(now, settings.reduceMotion);
}
