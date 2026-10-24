// viewport.cpp - draws the game at 1280x720 and scales it to any window size.
#include "viewport.h"
#include "constants.h"

// A RenderTexture2D is a picture we can draw into instead of the window.
static RenderTexture2D target;

// How much the virtual screen is scaled, and where its top-left corner lands.
static float Scale() {
    float sx = (float)GetScreenWidth() / VIRTUAL_W;
    float sy = (float)GetScreenHeight() / VIRTUAL_H;
    return (sx < sy) ? sx : sy;
}

static Vector2 Offset() {
    float s = Scale();
    return { (GetScreenWidth() - VIRTUAL_W * s) / 2.0f, (GetScreenHeight() - VIRTUAL_H * s) / 2.0f };
}

void InitViewport() {
    target = LoadRenderTexture(VIRTUAL_W, VIRTUAL_H);
    // Bilinear filtering keeps text smooth when the picture is stretched.
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}

void UnloadViewport() {
    UnloadRenderTexture(target);
}

void BeginVirtual() {
    BeginTextureMode(target);
}

void EndVirtual() {
    EndTextureMode();
}

void PresentVirtual() {
    ClearBackground(BLACK);  // the letterbox bars
    float s = Scale();
    Vector2 o = Offset();
    // Render textures are stored upside down in OpenGL, so the source height is negative.
    Rectangle source = { 0, 0, (float)VIRTUAL_W, -(float)VIRTUAL_H };
    Rectangle dest = { o.x, o.y, VIRTUAL_W * s, VIRTUAL_H * s };
    DrawTexturePro(target.texture, source, dest, { 0, 0 }, 0.0f, WHITE);
}

Vector2 VirtualMouse() {
    // On phones the finger position comes from the touch list.
    Vector2 p = (GetTouchPointCount() > 0) ? GetTouchPosition(0) : GetMousePosition();
    float s = Scale();
    Vector2 o = Offset();
    return { (p.x - o.x) / s, (p.y - o.y) / s };
}

bool IsPortrait() {
    return GetScreenHeight() > GetScreenWidth();
}
