// viewport.h - the 1280x720 "virtual screen".
// Every screen draws as if the window were exactly 1280x720. We draw into an
// off-screen texture of that size, then stretch it onto the real window,
// keeping the shape and adding black bars (letterboxing) where needed.
#pragma once
#include "raylib.h"

void InitViewport();
void UnloadViewport();

// Call between these two to draw in 1280x720 space.
void BeginVirtual();
void EndVirtual();

// Draws the finished virtual screen onto the real window. Call inside BeginDrawing.
void PresentVirtual();

// The mouse (or finger) position in 1280x720 space.
Vector2 VirtualMouse();

// True while a phone is held upright (taller than wide).
bool IsPortrait();
