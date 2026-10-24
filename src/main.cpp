// main.cpp - toolchain smoke test (replaced by the real game in later steps)
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame() {
    BeginDrawing();
    ClearBackground(GetColor(0x0B0807FF));
    DrawText("SPACE BATTLE", 40, 40, 40, GetColor(0xFFB347FF));
    EndDrawing();
}

int main() {
    InitWindow(1280, 720, "Space Battle");
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) UpdateDrawFrame();
#endif
    CloseWindow();
    return 0;
}
