// game.cpp - the screen state machine: remembers which screen is showing
// and sends Update and Draw to it.
#include "game.h"
#include "raylib.h"

Settings settings = { true, true, false };
Debrief debrief = { 0, 0, 0, 0 };

static Screen current = Screen::Title;
static float screenTime = 0.0f;

void StartGame() {
    GoToScreen(Screen::Title);
}

void GoToScreen(Screen next) {
    current = next;
    screenTime = 0.0f;
    switch (current) {
        case Screen::Title:    EnterTitle(); break;
        case Screen::HowTo:    EnterHowTo(); break;
        case Screen::Settings: EnterSettings(); break;
        case Screen::Battle:   EnterBattle(); break;
        case Screen::Victory:
        case Screen::Defeat:   EnterResult(); break;
    }
}

float ScreenTime() {
    return screenTime;
}

Screen CurrentScreen() {
    return current;
}

void UpdateGame() {
    screenTime += GetFrameTime();
    switch (current) {
        case Screen::Title:    UpdateTitle(); break;
        case Screen::HowTo:    UpdateHowTo(); break;
        case Screen::Settings: UpdateSettings(); break;
        case Screen::Battle:   UpdateBattle(); break;
        case Screen::Victory:
        case Screen::Defeat:   UpdateResult(); break;
    }
}

void DrawGame() {
    switch (current) {
        case Screen::Title:    DrawTitle(); break;
        case Screen::HowTo:    DrawHowTo(); break;
        case Screen::Settings: DrawSettings(); break;
        case Screen::Battle:   DrawBattle(); break;
        case Screen::Victory:
        case Screen::Defeat:   DrawResult(); break;
    }
}
