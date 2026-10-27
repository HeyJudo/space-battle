// background.h - the space behind every screen and the effects laid over it:
// nebula glow, three layers of drifting stars, space dust, the planet,
// the red alert wash, and the CRT look (scanlines, light sweep, vignette).
#pragma once
#include "raylib.h"

// Draw order on each screen: back layers, planet, front layers, screen content, overlays.
// speed multiplies how fast the stars move; above 1 they also get short streaks (Battle screen).
void DrawBackLayers(float t, bool reduceMotion, float speed = 1.0f);    // nebula and the two far star layers
void DrawPlanet(Vector2 center, float radius, float t, bool reduceMotion);
void DrawFrontLayers(float t, bool reduceMotion, float speed = 1.0f);   // near stars and dust
void DrawAlertWash(float strength);                 // red glow from the top edge, 0 to 1
void DrawScreenOverlays(float t, bool reduceMotion); // vignette, scanlines and sweep

// Pictures made in code at startup (called by LoadAssets).
Texture2D MakePlanetTexture();
Texture2D MakePlanetBandsTexture();
Texture2D MakeVignetteTexture();
