// game.h - which screen is showing, the player's settings, and the debrief
// numbers passed from Battle to Victory or Defeat.
// Each screen has three functions: Enter (reset when it opens), Update (input
// and timers) and Draw (pictures only).
#pragma once

enum class Screen { Title, HowTo, Settings, Battle, Victory, Defeat };

struct Settings {
    bool soundEffects;
    bool music;
    bool reduceMotion;  // no screen shake, glitch, CRT sweep or star drift
};

// The numbers shown on the Victory and Defeat screens.
struct Debrief {
    int engagements;
    int damageDealt;
    int playerHull;
    int wardenHull;
};

// Shared game state. Settings last until the browser tab is closed.
extern Settings settings;
extern Debrief debrief;

void StartGame();             // opens the Title screen
void GoToScreen(Screen next);
Screen CurrentScreen();
float ScreenTime();           // seconds since the current screen opened
void UpdateGame();
void DrawGame();

// ---------- The screens (one .cpp file each) ----------
void EnterTitle();
void UpdateTitle();
void DrawTitle();

void EnterHowTo();
void UpdateHowTo();
void DrawHowTo();

void EnterSettings();
void UpdateSettings();
void DrawSettings();

void EnterBattle();
void UpdateBattle();
void DrawBattle();

// Victory and Defeat share one file: they only differ in colors and words.
void EnterResult();
void UpdateResult();
void DrawResult();
