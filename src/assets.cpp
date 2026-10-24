// assets.cpp - loads the fonts and sprites listed in assets.h.
#include "assets.h"
#include "background.h"

Assets assets;

// Every size, in pixels, that some screen draws text at.
static const int DISPLAY_SIZES[] = { 18, 22, 24, 34, 40, 44, 116, 150 };
static const int MONO_SIZES[] = { 12, 13, 15, 16 };
static const int DISPLAY_COUNT = sizeof(DISPLAY_SIZES) / sizeof(DISPLAY_SIZES[0]);
static const int MONO_COUNT = sizeof(MONO_SIZES) / sizeof(MONO_SIZES[0]);

// (ascent - descent) / units per em, read from each font file's header.
const float DISPLAY_SCALE = 1.276f;
const float MONO_SCALE = 1.127f;

static Font displayFonts[DISPLAY_COUNT];
static Font monoFonts[MONO_COUNT];

static Texture2D LoadSmoothTexture(const char* path) {
    Texture2D texture = LoadTexture(path);
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    return texture;
}

void LoadAssets() {
    for (int i = 0; i < DISPLAY_COUNT; i++) {
        // LoadFontEx(file, size, NULL, 0) builds the basic ASCII characters at that size.
        int pixels = (int)(DISPLAY_SIZES[i] * DISPLAY_SCALE + 0.5f);
        displayFonts[i] = LoadFontEx("assets/fonts/Rajdhani-Bold.ttf", pixels, nullptr, 0);
        SetTextureFilter(displayFonts[i].texture, TEXTURE_FILTER_BILINEAR);
    }
    for (int i = 0; i < MONO_COUNT; i++) {
        int pixels = (int)(MONO_SIZES[i] * MONO_SCALE + 0.5f);
        monoFonts[i] = LoadFontEx("assets/fonts/ShareTechMono-Regular.ttf", pixels, nullptr, 0);
        SetTextureFilter(monoFonts[i].texture, TEXTURE_FILTER_BILINEAR);
    }

    assets.halberd = LoadSmoothTexture("assets/sprites/playerShip2_orange.png");
    assets.warden = LoadSmoothTexture("assets/sprites/enemyBlack3.png");

    // A grey copy of the Halberd for the Defeat screen. Each pixel's red, green and
    // blue become one brightness value; its transparency (alpha) stays as it was.
    Image wreck = LoadImage("assets/sprites/playerShip2_orange.png");
    ImageFormat(&wreck, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color* pixels = (Color*)wreck.data;
    for (int i = 0; i < wreck.width * wreck.height; i++) {
        unsigned char grey = (unsigned char)((pixels[i].r * 30 + pixels[i].g * 59 + pixels[i].b * 11) / 100);
        pixels[i].r = pixels[i].g = pixels[i].b = grey;
    }
    assets.halberdWreck = LoadTextureFromImage(wreck);
    SetTextureFilter(assets.halberdWreck, TEXTURE_FILTER_BILINEAR);
    UnloadImage(wreck);

    // The Kenney shield is white, so a tint can color it amber or red.
    assets.shield = LoadSmoothTexture("assets/sprites/shield3.png");

    assets.planet = MakePlanetTexture();
    assets.planetBands = MakePlanetBandsTexture();
    assets.vignette = MakeVignetteTexture();
}

void UnloadAssets() {
    for (int i = 0; i < DISPLAY_COUNT; i++) UnloadFont(displayFonts[i]);
    for (int i = 0; i < MONO_COUNT; i++) UnloadFont(monoFonts[i]);
    UnloadTexture(assets.halberd);
    UnloadTexture(assets.halberdWreck);
    UnloadTexture(assets.warden);
    UnloadTexture(assets.shield);
    UnloadTexture(assets.planet);
    UnloadTexture(assets.planetBands);
    UnloadTexture(assets.vignette);
}

Font GetFont(FontKind kind, float size) {
    const int* sizes = (kind == FontKind::Display) ? DISPLAY_SIZES : MONO_SIZES;
    Font* fonts = (kind == FontKind::Display) ? displayFonts : monoFonts;
    int count = (kind == FontKind::Display) ? DISPLAY_COUNT : MONO_COUNT;
    // Pick the first loaded size that is big enough, so text is only ever scaled down.
    for (int i = 0; i < count; i++)
        if (sizes[i] >= size)
            return fonts[i];
    return fonts[count - 1];
}

float FontScale(FontKind kind) {
    return (kind == FontKind::Display) ? DISPLAY_SCALE : MONO_SCALE;
}
