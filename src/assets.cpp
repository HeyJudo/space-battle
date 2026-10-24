// assets.cpp - loads the fonts and sprites listed in assets.h.
#include "assets.h"
#include "background.h"

Assets assets;

// Every size, in pixels, that some screen draws text at.
static const int DISPLAY_SIZES[] = { 18, 22, 24, 34, 40, 44, 116, 150 };
static const int MONO_SIZES[] = { 12, 13, 15, 16 };
static const int DISPLAY_COUNT = sizeof(DISPLAY_SIZES) / sizeof(DISPLAY_SIZES[0]);
static const int MONO_COUNT = sizeof(MONO_SIZES) / sizeof(MONO_SIZES[0]);

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
        displayFonts[i] = LoadFontEx("assets/fonts/Rajdhani-Bold.ttf", DISPLAY_SIZES[i], nullptr, 0);
        SetTextureFilter(displayFonts[i].texture, TEXTURE_FILTER_BILINEAR);
    }
    for (int i = 0; i < MONO_COUNT; i++) {
        monoFonts[i] = LoadFontEx("assets/fonts/ShareTechMono-Regular.ttf", MONO_SIZES[i], nullptr, 0);
        SetTextureFilter(monoFonts[i].texture, TEXTURE_FILTER_BILINEAR);
    }

    assets.halberd = LoadSmoothTexture("assets/sprites/playerShip2_orange.png");
    assets.warden = LoadSmoothTexture("assets/sprites/enemyBlack3.png");

    // The Kenney shield is blue. Making it grey first lets a tint color it any way we like.
    Image shield = LoadImage("assets/sprites/shield3.png");
    ImageColorGrayscale(&shield);
    ImageColorBrightness(&shield, 60);
    assets.shield = LoadTextureFromImage(shield);
    SetTextureFilter(assets.shield, TEXTURE_FILTER_BILINEAR);
    UnloadImage(shield);

    assets.planet = MakePlanetTexture();
    assets.planetBands = MakePlanetBandsTexture();
    assets.vignette = MakeVignetteTexture();
}

void UnloadAssets() {
    for (int i = 0; i < DISPLAY_COUNT; i++) UnloadFont(displayFonts[i]);
    for (int i = 0; i < MONO_COUNT; i++) UnloadFont(monoFonts[i]);
    UnloadTexture(assets.halberd);
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
