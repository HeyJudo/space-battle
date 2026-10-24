// screen_howto.cpp - How to Play: the four commands with their keys and
// numbers, what the Warden does, and the goal.
#include "raylib.h"
#include "audio.h"
#include "background.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"

static Button gotItButton;

void EnterHowTo() {
    gotItButton = MakeButton({ 858, 482, 230, 62 }, TXT_GOT_IT, nullptr, "ESC", Icon::None, true);
}

void UpdateHowTo() {
    if (UpdateButton(gotItButton, true) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        PlaySfx(Sfx::Click);
        GoToScreen(Screen::Title);
    }
}

// One command card: icon, name, key and what it does.
static void DrawMoveCard(float x, float y, Icon icon, const char* name, const char* key, const char* text, bool highlight) {
    Rectangle card = { x, y, 442, 84 };
    FillChamfer(card, 10, Col(COL_AMBER, 0.07f), Col(COL_AMBER, 0.0f));
    OutlineChamfer(card, 10, Col(COL_AMBER, 0.18f));
    Rectangle socket = { x + 14, y + 14, 56, 56 };
    DrawIconSocket(socket, 9, highlight);
    DrawIcon(icon, { socket.x + 28, socket.y + 28 }, 26, highlight ? Col(COL_INK) : Col(COL_AMBER));
    DrawLabel(FontKind::Display, name, x + 84, y + 15, 24, 0.1f, Col(COL_TEXT));
    DrawLabelRight(FontKind::Mono, key, x + 428, y + 22, 12, 0.12f, Col(COL_DIM));
    DrawLabel(FontKind::Display, text, x + 84, y + 45, 18, 0.0f, Col(0xDCC8A8FF));
}

void DrawHowTo() {
    float now = (float)GetTime();
    float t = ScreenTime();
    bool motion = !settings.reduceMotion;

    ClearBackground(GetColor(COL_BG));
    DrawBackLayers(now, settings.reduceMotion);
    DrawPlanet({ 1260, -40 }, 380, now, settings.reduceMotion);
    DrawFrontLayers(now, settings.reduceMotion);
    DrawHazardTape(0, 8, false, 0.9f, now, motion);

    float rise = EaseIn(t, 0.0f, 0.7f);
    SetUiFade(rise);
    float dy = 18 * (1 - rise);
    Rectangle panel = { 170, 96 + dy, 940, 469 };
    DrawPanel(panel, TXT_HOWTO, TXT_HOWTO_RIGHT);
    DrawCornerTicks(panel);

    // The four cards rise one after another.
    struct Move { Icon icon; const char* name; const char* key; const char* text; };
    const Move moves[4] = {
        { Icon::Crosshair, TXT_FIRE, "KEY 1", TXT_HOWTO_FIRE },
        { Icon::Shield, TXT_SHIELDS, "KEY 2", TXT_HOWTO_SHIELDS },
        { Icon::Wrench, TXT_REPAIR, "KEY 3", TXT_HOWTO_REPAIR },
        { Icon::Exit, TXT_RETREAT, "KEY 4", TXT_HOWTO_RETREAT },
    };
    for (int i = 0; i < 4; i++) {
        float r = EaseIn(t, 0.1f + 0.08f * i, 0.7f);
        SetUiFade(r);
        float x = (i % 2 == 0) ? 192.0f : 646.0f;
        float y = (i < 2 ? 154.0f : 250.0f) + dy + 18 * (1 - r);
        DrawMoveCard(x, y, moves[i].icon, moves[i].name, moves[i].key, moves[i].text, i == 0);
    }

    // The enemy and the goal.
    float r = EaseIn(t, 0.45f, 0.7f);
    SetUiFade(r);
    float y = 350 + dy + 18 * (1 - r);
    Rectangle enemy = { 192, y, 896, 112 };
    FillChamfer(enemy, 10, Col(COL_RED, 0.07f), Col(COL_RED, 0.07f));
    OutlineChamfer(enemy, 10, Col(COL_RED, 0.3f));
    DrawShip(assets.warden, { 256, y + 56 }, 58, 0, Col(0xFFFFFFFF));
    DrawLockOn({ 214, y + 18, 84, 76 }, now, motion);
    float textX = 320;
    float gap = MeasureLabel(FontKind::Mono, "ENEMY  ", 15, 0.06f);
    DrawLabel(FontKind::Mono, TXT_ENEMY, textX, y + 22, 15, 0.06f, Col(COL_RED_SOFT));
    DrawLabel(FontKind::Mono, TXT_ENEMY_LINE_1, textX + gap, y + 22, 15, 0.06f, Col(COL_TEXT));
    DrawLabel(FontKind::Mono, TXT_ENEMY_LINE_2, textX, y + 47, 15, 0.06f, Col(COL_TEXT));
    DrawLabel(FontKind::Mono, TXT_GOAL, textX, y + 72, 15, 0.06f, Col(COL_AMBER));
    DrawLabel(FontKind::Mono, TXT_GOAL_LINE, textX + gap, y + 72, 15, 0.06f, Col(COL_TEXT));

    SetUiFade(rise);
    Button gotIt = gotItButton;
    gotIt.rect.y += dy;
    DrawButton(gotIt, true, now);
    SetUiFade(1);

    DrawScreenOverlays(now, settings.reduceMotion);
}
