// background.cpp - space, stars, the planet and the screen overlays.
#include "background.h"
#include "assets.h"
#include "constants.h"
#include "ui.h"
#include <cmath>

// ---------- Stars ----------

// One repeating tile of stars. Positions are fractions of the tile size.
struct StarLayer {
    float tileW, tileH;
    float speed;        // pixels per second, drifting left
    float radius;
    float alpha;
    int count;
    float x[4], y[4];
    unsigned int color[4];
};

static const StarLayer FAR_STARS = { 300, 260, 9, 0.6f, 0.45f, 4,
    { 0.12f, 0.46f, 0.78f, 0.30f }, { 0.20f, 0.64f, 0.36f, 0.88f }, { 0xFFFFFFFF, 0xFFE6C4FF, 0xFFFFFFFF, 0xFFFFFFFF } };
static const StarLayer MID_STARS = { 460, 400, 18, 0.8f, 0.65f, 3,
    { 0.22f, 0.66f, 0.88f }, { 0.40f, 0.12f, 0.78f }, { 0xFFFFFFFF, 0xFFD9A8FF, 0xFFFFFFFF } };
static const StarLayer NEAR_STARS = { 720, 560, 40, 1.1f, 0.9f, 2,
    { 0.40f, 0.80f }, { 0.30f, 0.70f }, { 0xFFFFFFFF, 0xFFE6C4FF } };
static const StarLayer DUST = { 220, 200, 0, 0.6f, 0.5f, 3,
    { 0.10f, 0.60f, 0.30f }, { 0.10f, 0.40f, 0.70f }, { 0xFFB347CC, 0xFFB34799, 0xFF783CB3 } };

// Draws a star tile again and again to cover the screen, shifted by (shiftX, shiftY).
static void DrawStarLayer(const StarLayer& layer, float shiftX, float shiftY) {
    float ox = fmodf(shiftX, layer.tileW);
    float oy = fmodf(shiftY, layer.tileH);
    if (ox > 0) ox -= layer.tileW;
    if (oy > 0) oy -= layer.tileH;
    for (float tx = ox; tx < VIRTUAL_W; tx += layer.tileW) {
        for (float ty = oy; ty < VIRTUAL_H; ty += layer.tileH) {
            for (int i = 0; i < layer.count; i++) {
                Vector2 p = { tx + layer.x[i] * layer.tileW, ty + layer.y[i] * layer.tileH };
                DrawCircleV(p, layer.radius, Col(layer.color[i], layer.alpha));
            }
        }
    }
}

void DrawBackLayers(float t, bool reduceMotion) {
    float drift = reduceMotion ? 0.0f : t;
    // Nebula: three soft colored glows that slowly "breathe".
    float breathe = reduceMotion ? 0.0f : Pulse(t, 28.0f);
    float grow = 1.0f + 0.06f * breathe;
    float fade = 1.0f - 0.2f * breathe;
    Vector2 shift = { -12.8f * breathe, 7.2f * breathe };
    Vector2 a = { 148 + shift.x, 170 + shift.y };
    Vector2 b = { 1070 + shift.x, 84 + shift.y };
    Vector2 c = { 640 + shift.x, 792 + shift.y };
    FillEllipseGradient(a, 538 * grow, 242 * grow, a, Col(COL_RED, 0.16f * fade), Col(COL_RED, 0));
    FillEllipseGradient(b, 484 * grow, 302 * grow, b, Col(COL_AMBER, 0.13f * fade), Col(COL_AMBER, 0));
    FillEllipseGradient(c, 645 * grow, 242 * grow, c, Col(0x8C2814FF, 0.22f * fade), Col(0x8C2814FF, 0));

    DrawStarLayer(FAR_STARS, -drift * FAR_STARS.speed, 0);
    DrawStarLayer(MID_STARS, -drift * MID_STARS.speed, 0);
}

void DrawFrontLayers(float t, bool reduceMotion) {
    float drift = reduceMotion ? 0.0f : t;
    DrawStarLayer(NEAR_STARS, -drift * NEAR_STARS.speed, 0);
    // Dust drifts down and to the left.
    DrawStarLayer(DUST, -drift * 36.7f, drift * 33.3f);
}

// ---------- Planet ----------

const int PLANET_SIZE = 640;  // the planet picture is made at this size, then scaled

static Color Mix(Color a, Color b, float k) {
    return ColorLerp(a, b, k < 0 ? 0 : (k > 1 ? 1 : k));
}

// Builds the planet picture pixel by pixel: a dark ball lit from the top left,
// with an amber rim light and a deep shadow at the bottom right.
Texture2D MakePlanetTexture() {
    const int S = PLANET_SIZE;
    const float R = S / 2.0f;
    Image image = GenImageColor(S, S, BLANK);
    Color* pixels = (Color*)image.data;
    Color stop0 = GetColor(0x4A2F1CFF), stop1 = GetColor(0x24160DFF), stop2 = GetColor(0x0F0907FF);
    Color amber = GetColor(COL_AMBER);
    for (int y = 0; y < S; y++) {
        for (int x = 0; x < S; x++) {
            float px = x + 0.5f, py = y + 0.5f;
            float dist = sqrtf((px - R) * (px - R) + (py - R) * (py - R));
            float inside = R - dist + 0.5f;  // soft 1-pixel edge
            if (inside <= 0) continue;

            // Base color: a radial gradient centered up and to the left.
            float d = sqrtf((px - 0.32f * S) * (px - 0.32f * S) + (py - 0.28f * S) * (py - 0.28f * S)) / (0.99f * S);
            Color c = (d < 0.38f) ? Mix(stop0, stop1, d / 0.38f) : Mix(stop1, stop2, (d - 0.38f) / 0.28f);

            // Shadow: everything outside a copy of the ball nudged up-left goes dark.
            float sx = px - (R - 0.053f * S), sy = py - (R - 0.039f * S);
            float shadow = 0.5f + (sqrtf(sx * sx + sy * sy) - R) / (0.105f * S);
            c = Mix(c, BLACK, 0.8f * (shadow < 0 ? 0 : shadow > 1 ? 1 : shadow));

            // Rim light: everything outside a copy nudged down-right glows amber.
            float lx = px - (R + 0.037f * S), ly = py - (R + 0.024f * S);
            float light = 0.5f + (sqrtf(lx * lx + ly * ly) - R) / (0.066f * S);
            c = Mix(c, amber, 0.38f * (light < 0 ? 0 : light > 1 ? 1 : light));

            c.a = (unsigned char)(255 * (inside > 1 ? 1 : inside));
            pixels[y * S + x] = c;
        }
    }
    Texture2D texture = LoadTextureFromImage(image);
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    UnloadImage(image);
    return texture;
}

// Faint diagonal bands across the planet, in their own picture so they can turn slowly.
Texture2D MakePlanetBandsTexture() {
    const int S = PLANET_SIZE;
    const float R = S / 2.0f;
    const float PERIOD = 0.058f * S, BAND = 0.021f * S;
    Image image = GenImageColor(S, S, BLANK);
    Color* pixels = (Color*)image.data;
    for (int y = 0; y < S; y++) {
        for (int x = 0; x < S; x++) {
            float px = x + 0.5f - R, py = y + 0.5f - R;
            float inside = R - sqrtf(px * px + py * py) + 0.5f;
            if (inside <= 0) continue;
            float along = px * 0.208f + py * 0.978f + 4 * S;  // distance along the 168-degree direction
            float inBand = (fmodf(along, PERIOD) < BAND) ? 1.0f : 0.0f;
            float alpha = 0.07f * inBand * (inside > 1 ? 1 : inside);
            pixels[y * S + x] = { 255, 179, 71, (unsigned char)(255 * alpha) };
        }
    }
    Texture2D texture = LoadTextureFromImage(image);
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    UnloadImage(image);
    return texture;
}

void DrawPlanet(Vector2 center, float radius, float t, bool reduceMotion) {
    // Outer glow: a big soft circle behind the planet.
    float glowR = radius + 90;
    float glowAlpha = 0.18f * glowR / 90.0f;
    DrawCircleGradient((int)center.x, (int)center.y, glowR, Col(0xFF7828FF, glowAlpha > 1 ? 1 : glowAlpha), Col(0xFF7828FF, 0));

    Rectangle source = { 0, 0, (float)PLANET_SIZE, (float)PLANET_SIZE };
    Rectangle dest = { center.x, center.y, radius * 2, radius * 2 };
    Vector2 origin = { radius, radius };
    DrawTexturePro(assets.planet, source, dest, origin, 0, Col(0xFFFFFFFF));
    float spin = reduceMotion ? 0.0f : t / 600.0f * 360.0f;  // one turn every 10 minutes
    DrawTexturePro(assets.planetBands, source, dest, origin, spin, Col(0xFFFFFFFF));
}

// ---------- Overlays ----------

Texture2D MakeVignetteTexture() {
    const int W = 320, H = 180;
    Image image = GenImageColor(W, H, BLANK);
    Color* pixels = (Color*)image.data;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            // Distance from the middle, where 1 means the corners.
            float u = (x + 0.5f) / W - 0.5f, v = (y + 0.5f) / H - 0.5f;
            float e = sqrtf(u * u + v * v) / 0.7071f;
            float k = (e - 0.55f) / 0.45f;
            float alpha = 0.7f * (k < 0 ? 0 : k > 1 ? 1 : k);
            pixels[y * W + x] = { 0, 0, 0, (unsigned char)(255 * alpha) };
        }
    }
    Texture2D texture = LoadTextureFromImage(image);
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    UnloadImage(image);
    return texture;
}

void DrawAlertWash(float strength) {
    if (strength <= 0) return;
    Vector2 top = { VIRTUAL_W / 2.0f, 0 };
    FillEllipseGradient(top, 780, 520, top, Col(COL_RED, 0.26f * strength), Col(COL_RED, 0));
}

void DrawScreenOverlays(float t, bool reduceMotion) {
    DrawTexturePro(assets.vignette, { 0, 0, (float)assets.vignette.width, (float)assets.vignette.height },
                   { 0, 0, (float)VIRTUAL_W, (float)VIRTUAL_H }, { 0, 0 }, 0, WHITE);
    // Scanlines: a thin dark line every 3 pixels, like an old CRT monitor.
    for (int y = 0; y < VIRTUAL_H; y += 3)
        DrawRectangle(0, y, VIRTUAL_W, 1, { 0, 0, 0, 51 });
    // A faint band of light that rolls down the screen every 7 seconds.
    if (!reduceMotion) {
        float y = -120 + fmodf(t / 7.0f, 1.0f) * 960;
        Color clear = { 255, 200, 140, 0 }, glow = { 255, 200, 140, 13 };
        DrawRectangleGradientV(0, (int)y, VIRTUAL_W, 60, clear, glow);
        DrawRectangleGradientV(0, (int)y + 60, VIRTUAL_W, 60, glow, clear);
    }
}
