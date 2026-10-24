// screen_settings.cpp - Settings: sound effects, music and reduce motion,
// each with an ON / OFF switch. Changes apply right away.
#include "raylib.h"
#include "audio.h"
#include "background.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"
#include "viewport.h"

static Button backButton;

// Each option row, from top to bottom.
struct OptionRow {
    Icon icon;
    const char* label;
    bool* value;      // the setting this row switches
    bool showMeter;   // audio rows show a level meter; reduce motion shows a hint instead
};

static OptionRow rows[3];
const int ROW_COUNT = 3;
const float PANEL_X = 330, PANEL_Y = 150, PANEL_W = 620;
const float ROW_H = 89;

static float RowTop(int i) {
    return PANEL_Y + PANEL_HEADER_H + i * ROW_H;
}

// Where the ON and OFF halves of a row's switch are.
static Rectangle SwitchRect(int i, bool onHalf) {
    float x = PANEL_X + PANEL_W - 22 - 172;
    return { onHalf ? x : x + 86, RowTop(i) + 21, 86, 46 };
}

void EnterSettings() {
    rows[0] = { Icon::Speaker, TXT_SOUND_EFFECTS, &settings.soundEffects, true };
    rows[1] = { Icon::Music, TXT_MUSIC, &settings.music, true };
    rows[2] = { Icon::Motion, TXT_REDUCE_MOTION, &settings.reduceMotion, false };
    float footerY = RowTop(ROW_COUNT);
    backButton = MakeButton({ PANEL_X + PANEL_W - 22 - 200, footerY + 18, 200, 62 }, TXT_BACK, nullptr, "ESC", Icon::None, false);
}

void UpdateSettings() {
    for (int i = 0; i < ROW_COUNT; i++) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = VirtualMouse();
            if (CheckCollisionPointRec(m, SwitchRect(i, true))) { *rows[i].value = true; PlaySfx(Sfx::Click); }
            if (CheckCollisionPointRec(m, SwitchRect(i, false))) { *rows[i].value = false; PlaySfx(Sfx::Click); }
        }
    }
    if (UpdateButton(backButton, true) || IsKeyPressed(KEY_ESCAPE)) {
        PlaySfx(Sfx::Click);
        GoToScreen(Screen::Title);
    }
}

// Ten little bars; they gently pulse while the sound is on.
static void DrawLevelMeter(float x, float y, bool on, float now, bool animate) {
    for (int i = 0; i < 10; i++) {
        Rectangle cell = { x + i * 19, y, 16, 8 };
        if (!on) {
            DrawRectangleRec(cell, Col(0x2C2118FF));
        } else if (i < 7) {
            float k = animate ? 1.0f - 0.45f * Pulse(now + i * 0.4f, 2.2f) : 1.0f;
            DrawRectangleRec({ cell.x - 2, cell.y - 2, 20, 12 }, Col(COL_AMBER, 0.15f * k));
            DrawRectangleRec(cell, Col(COL_AMBER, k));
        } else {
            DrawRectangleRec(cell, Col(0x4A3624FF));
        }
    }
}

// One half of an ON / OFF switch. The whole switch is one cut-corner shape;
// the scissor clips it to this half, so ON keeps the top-left cut and OFF the bottom-right one.
static void DrawSwitchHalf(Rectangle frame, Rectangle half, const char* text, bool selected, bool hovered) {
    BeginScissorMode((int)half.x, (int)half.y, (int)half.width, (int)half.height);
    if (selected) {
        FillChamfer(frame, 10, Col(COL_AMBER_LIGHT), Col(COL_AMBER_SHADE));
        DrawRectangleRec({ half.x, half.y + half.height - 3, half.width - 3, 3 }, Col(0x783A00FF, 0.45f));
    } else if (hovered) {
        FillChamfer(frame, 10, Col(COL_AMBER, 0.08f), Col(COL_AMBER, 0.08f));
    }
    EndScissorMode();
    Color c = selected ? Col(COL_INK) : (hovered ? Col(COL_TEXT) : Col(COL_DIM));
    DrawLabelCentered(FontKind::Mono, text, half.x + half.width / 2 + 1, half.y + 15, 14, 0.16f, c);
}

void DrawSettings() {
    float now = (float)GetTime();
    float t = ScreenTime();
    bool motion = !settings.reduceMotion;

    ClearBackground(GetColor(COL_BG));
    DrawBackLayers(now, settings.reduceMotion);
    DrawPlanet({ 10, 790 }, 350, now, settings.reduceMotion);
    DrawFrontLayers(now, settings.reduceMotion);
    DrawHazardTape(0, 8, false, 0.9f, now, motion);

    float rise = EaseIn(t, 0.0f, 0.7f);
    SetUiFade(rise);
    float dy = 18 * (1 - rise);
    float footerY = RowTop(ROW_COUNT);
    Rectangle panel = { PANEL_X, PANEL_Y + dy, PANEL_W, footerY + 99 - PANEL_Y };
    DrawPanel(panel, TXT_SETTINGS, TXT_SETTINGS_RIGHT);
    DrawCornerTicks(panel);

    Vector2 mouse = VirtualMouse();
    for (int i = 0; i < ROW_COUNT; i++) {
        float y = RowTop(i) + dy;
        if (i > 0)
            DrawRectangleRec({ PANEL_X + 1, y, PANEL_W - 2, 1 }, Col(COL_AMBER, 0.14f));
        Rectangle socket = { PANEL_X + 22, y + 18, 52, 52 };
        DrawIconSocket(socket, 9, false);
        DrawIcon(rows[i].icon, { socket.x + 26, socket.y + 26 }, 24, Col(COL_AMBER));
        DrawLabel(FontKind::Display, rows[i].label, PANEL_X + 90, y + 21, 24, 0.1f, Col(COL_TEXT));
        if (rows[i].showMeter)
            DrawLevelMeter(PANEL_X + 90, y + 56, *rows[i].value, now, motion);
        else
            DrawLabel(FontKind::Mono, TXT_REDUCE_MOTION_HINT, PANEL_X + 90, y + 54, 12, 0.08f, Col(COL_DIM));

        // The ON / OFF switch, in a cut-corner frame.
        Rectangle on = SwitchRect(i, true), off = SwitchRect(i, false);
        on.y += dy; off.y += dy;
        Rectangle frame = { on.x, on.y, 172, 46 };
        DrawSwitchHalf(frame, on, TXT_ON, *rows[i].value, CheckCollisionPointRec(mouse, on));
        DrawSwitchHalf(frame, off, TXT_OFF, !*rows[i].value, CheckCollisionPointRec(mouse, off));
        OutlineChamfer(frame, 10, Col(COL_AMBER, 0.35f));
    }

    DrawRectangleRec({ PANEL_X + 1, footerY + dy, PANEL_W - 2, 1 }, Col(COL_AMBER, 0.2f));
    DrawLabel(FontKind::Mono, TXT_APPLY, PANEL_X + 22, footerY + dy + 43, 12, 0.14f, Col(COL_DIM));
    Button back = backButton;
    back.rect.y += dy;
    DrawButton(back, true, now);
    SetUiFade(1);

    DrawScreenOverlays(now, settings.reduceMotion);
}
