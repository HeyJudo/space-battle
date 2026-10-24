// assets.h - fonts and pictures, loaded once when the game starts.
// Paths are relative to the assets/ folder, which the web build packs inside
// index.data and the desktop build reads from the project folder.
#pragma once
#include "raylib.h"

// Rajdhani Bold for titles, labels and numbers; Share Tech Mono for the feed and readouts.
enum class FontKind { Display, Mono };

struct Assets {
    Texture2D halberd;   // the player's ship (Kenney playerShip2_orange)
    Texture2D warden;    // the enemy ship (Kenney enemyBlack3)
    Texture2D shield;    // the shield bubble, turned grey so we can tint it amber or red
    Texture2D planet;    // drawn once in code at startup (see background.cpp)
    Texture2D planetBands;
    Texture2D vignette;
};

extern Assets assets;

void LoadAssets();
void UnloadAssets();

// The loaded font closest to the wanted pixel size. raylib draws each font
// from a picture made at one size, so we load the sizes the screens use.
Font GetFont(FontKind kind, float size);
