// ui.cpp - the shared drawing kit. See ui.h for what each piece is.
// The look comes from docs/design/ra2-reference.css, translated into raylib calls.
#include "ui.h"
#include "constants.h"
#include "viewport.h"
#include "rlgl.h"  // raylib's low-level drawing, used for shapes with color gradients
#include <cmath>

static float uiFade = 1.0f;

Color Col(unsigned int hex, float alpha) {
    Color c = GetColor(hex);
    c.a = (unsigned char)(c.a * alpha * uiFade);
    return c;
}

void SetUiFade(float alpha) {
    uiFade = alpha;
}

// A raw color (not from the palette) that still respects the UI fade.
static Color Rgba(int r, int g, int b, float alpha) {
    return { (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)(255 * alpha * uiFade) };
}

static float Clamp01(float v) {
    return (v < 0) ? 0 : (v > 1) ? 1 : v;
}

// ---------- Animation helpers ----------

float EaseIn(float t, float delay, float duration) {
    float p = Clamp01((t - delay) / duration);
    float inv = 1.0f - p;
    return 1.0f - inv * inv * inv;  // fast start, gentle stop
}

float Pulse(float t, float period) {
    return 0.5f - 0.5f * cosf(2.0f * PI * t / period);
}

bool Blink(float t, float period) {
    return fmodf(t, period) < period / 2.0f;
}

// ---------- Text ----------

void DrawLabel(FontKind kind, const char* text, float x, float y, float size, float spacingEm, Color color) {
    DrawTextEx(GetFont(kind, size), text, { x, y }, size, size * spacingEm, color);
}

float MeasureLabel(FontKind kind, const char* text, float size, float spacingEm) {
    return MeasureTextEx(GetFont(kind, size), text, size, size * spacingEm).x;
}

void DrawLabelRight(FontKind kind, const char* text, float rightX, float y, float size, float spacingEm, Color color) {
    DrawLabel(kind, text, rightX - MeasureLabel(kind, text, size, spacingEm), y, size, spacingEm, color);
}

void DrawLabelCentered(FontKind kind, const char* text, float centerX, float y, float size, float spacingEm, Color color) {
    DrawLabel(kind, text, centerX - MeasureLabel(kind, text, size, spacingEm) / 2, y, size, spacingEm, color);
}

bool DrawTyped(FontKind kind, const char* text, float x, float y, float size, float spacingEm,
               Color color, float t, float delay, float duration, bool caret) {
    int length = TextLength(text);
    int shown = (int)(Clamp01((t - delay) / duration) * length);
    // TextSubtext gives the first "shown" letters of the text.
    DrawLabel(kind, TextSubtext(text, 0, shown), x, y, size, spacingEm, color);
    bool done = (shown >= length);
    if (done && caret && Blink(t, 1.0f)) {
        float w = MeasureLabel(kind, text, size, spacingEm);
        DrawLabel(kind, "_", x + w + size * spacingEm, y, size, spacingEm, color);
    }
    return done;
}

void DrawGlitchLabel(const char* text, float x, float y, float size, float spacingEm, Color color, float t, bool enabled) {
    DrawLabel(FontKind::Display, text, x, y, size, spacingEm, color);
    if (!enabled)
        return;
    // Every 6 seconds, for a fraction of a second, two offset copies flash on top.
    float phase = fmodf(t, 6.0f) / 6.0f;
    if (phase >= 0.92f && phase < 0.94f) DrawLabel(FontKind::Display, text, x - 4, y + 1, size, spacingEm, Col(COL_RED, 0.8f));
    if (phase >= 0.94f && phase < 0.96f) DrawLabel(FontKind::Display, text, x + 3, y - 2, size, spacingEm, Col(COL_RED, 0.6f));
    if (phase >= 0.93f && phase < 0.95f) DrawLabel(FontKind::Display, text, x + 4, y, size, spacingEm, Rgba(255, 233, 199, 0.5f));
    if (phase >= 0.95f && phase < 0.97f) DrawLabel(FontKind::Display, text, x - 2, y + 2, size, spacingEm, Rgba(255, 233, 199, 0.4f));
}

// ---------- Shapes ----------

// Fills a convex shape whose color changes in a straight line from "from" (c0) to "to" (c1).
// It draws a fan of triangles from the first point, each corner with its own color,
// and the graphics card blends the colors between corners.
static void FillPolygonLinear(const Vector2* points, int count, Vector2 from, Vector2 to, Color c0, Color c1) {
    Vector2 dir = { to.x - from.x, to.y - from.y };
    float lengthSq = dir.x * dir.x + dir.y * dir.y;
    Color colors[16];
    for (int i = 0; i < count; i++) {
        float k = lengthSq > 0 ? Clamp01(((points[i].x - from.x) * dir.x + (points[i].y - from.y) * dir.y) / lengthSq) : 0;
        colors[i] = ColorLerp(c0, c1, k);
    }
    rlSetTexture(rlGetTextureIdDefault());
    rlBegin(RL_TRIANGLES);
    for (int i = 1; i < count - 1; i++) {
        int corners[3] = { 0, i, i + 1 };
        for (int j : corners) {
            rlColor4ub(colors[j].r, colors[j].g, colors[j].b, colors[j].a);
            rlVertex2f(points[j].x, points[j].y);
        }
    }
    rlEnd();
    rlSetTexture(0);
}

static void ChamferPoints(Rectangle r, float cut, Vector2* p) {
    p[0] = { r.x + cut, r.y };
    p[1] = { r.x + r.width, r.y };
    p[2] = { r.x + r.width, r.y + r.height - cut };
    p[3] = { r.x + r.width - cut, r.y + r.height };
    p[4] = { r.x, r.y + r.height };
    p[5] = { r.x, r.y + cut };
}

void FillChamfer(Rectangle r, float cut, Color top, Color bottom) {
    Vector2 p[6];
    ChamferPoints(r, cut, p);
    FillPolygonLinear(p, 6, { r.x, r.y }, { r.x, r.y + r.height }, top, bottom);
}

void OutlineChamfer(Rectangle r, float cut, Color color) {
    // Shrink by half a pixel so 1-pixel lines land exactly on pixels.
    Rectangle in = { r.x + 0.5f, r.y + 0.5f, r.width - 1, r.height - 1 };
    Vector2 p[6];
    ChamferPoints(in, cut, p);
    for (int i = 0; i < 6; i++)
        DrawLineV(p[i], p[(i + 1) % 6], color);
}

void FillEllipseGradient(Vector2 center, float rx, float ry, Vector2 focus, Color inner, Color outer) {
    const int SEGMENTS = 48;
    rlSetTexture(rlGetTextureIdDefault());
    rlBegin(RL_TRIANGLES);
    for (int i = 0; i < SEGMENTS; i++) {
        float a0 = 2 * PI * i / SEGMENTS;
        float a1 = 2 * PI * (i + 1) / SEGMENTS;
        rlColor4ub(inner.r, inner.g, inner.b, inner.a);
        rlVertex2f(focus.x, focus.y);
        rlColor4ub(outer.r, outer.g, outer.b, outer.a);
        rlVertex2f(center.x + cosf(a0) * rx, center.y + sinf(a0) * ry);
        rlVertex2f(center.x + cosf(a1) * rx, center.y + sinf(a1) * ry);
    }
    rlEnd();
    rlSetTexture(0);
}

// ---------- Panels ----------

// The panel border glows bright at the top-left and bottom-right corners and
// fades in between (CSS: linear-gradient(155deg, .85, .14 32%, .12 68%, .65)).
static float BorderAlpha(float k) {
    if (k < 0.32f) return 0.85f + (0.14f - 0.85f) * (k / 0.32f);
    if (k < 0.68f) return 0.14f + (0.12f - 0.14f) * ((k - 0.32f) / 0.36f);
    return 0.12f + (0.65f - 0.12f) * ((k - 0.68f) / 0.32f);
}

static void GradientChamferBorder(Rectangle r, float cut, unsigned int hex) {
    Rectangle in = { r.x + 0.5f, r.y + 0.5f, r.width - 1, r.height - 1 };
    Vector2 p[6];
    ChamferPoints(in, cut, p);
    // The 155-degree direction, and how far along it each point sits (0 to 1).
    Vector2 dir = { 0.4226f, 0.9063f };
    float span = r.width * dir.x + r.height * dir.y;
    const int STEPS = 12;  // each edge is split into short pieces so the fade looks smooth
    for (int i = 0; i < 6; i++) {
        Vector2 a = p[i];
        Vector2 b = p[(i + 1) % 6];
        for (int s = 0; s < STEPS; s++) {
            Vector2 q0 = { a.x + (b.x - a.x) * s / STEPS, a.y + (b.y - a.y) * s / STEPS };
            Vector2 q1 = { a.x + (b.x - a.x) * (s + 1) / STEPS, a.y + (b.y - a.y) * (s + 1) / STEPS };
            Vector2 mid = { (q0.x + q1.x) / 2 - r.x, (q0.y + q1.y) / 2 - r.y };
            float k = Clamp01((mid.x * dir.x + mid.y * dir.y) / span);
            DrawLineV(q0, q1, Col(hex, BorderAlpha(k)));
        }
    }
}

void DrawPanel(Rectangle r, const char* title, const char* rightLabel, PanelTone tone) {
    const float CUT = 16;
    unsigned int edge = (tone == PanelTone::Red) ? COL_RED : COL_AMBER;
    FillChamfer(r, CUT, Col(COL_PANEL_TOP, 0.95f), Col(COL_PANEL_BOTTOM, 0.96f));
    GradientChamferBorder(r, CUT, edge);
    // A thin light line along the top edge.
    DrawRectangleGradientH((int)(r.x + CUT), (int)r.y + 1, (int)((r.width - CUT) * 0.7f), 1, Rgba(255, 226, 180, 0.6f), Rgba(255, 226, 180, 0));

    if (title == nullptr)
        return;
    // Header strip: a glow that fades out to the right, with a line under it.
    float h = PANEL_HEADER_H;
    float fadeEnd = r.x + r.width * 0.8f;
    Vector2 strip[5] = { { r.x + CUT, r.y + 1 }, { fadeEnd, r.y + 1 }, { fadeEnd, r.y + h }, { r.x + 1, r.y + h }, { r.x + 1, r.y + CUT } };
    FillPolygonLinear(strip, 5, { r.x, r.y }, { fadeEnd, r.y }, Col(edge, 0.16f), Col(edge, 0.0f));
    DrawRectangle((int)r.x + 1, (int)(r.y + h), (int)r.width - 2, 1, Col(edge, tone == PanelTone::Red ? 0.25f : 0.2f));

    Color titleColor = (tone == PanelTone::Red) ? Col(COL_RED_SOFT) : Col(COL_AMBER);
    DrawLabel(FontKind::Mono, TextFormat("> %s", title), r.x + 20, r.y + 12, 13, 0.18f, titleColor);
    if (rightLabel != nullptr)
        DrawLabelRight(FontKind::Mono, rightLabel, r.x + r.width - 20, r.y + 12, 13, 0.18f, Col(COL_DIM));
}

void DrawCornerTicks(Rectangle r) {
    Color c = Col(COL_AMBER);
    float right = r.x + r.width - 8;
    float bottom = r.y + r.height - 8;
    DrawRectangleRec({ right - 14, r.y + 8, 14, 2 }, c);
    DrawRectangleRec({ right - 2, r.y + 8, 2, 14 }, c);
    DrawRectangleRec({ r.x + 8, bottom - 2, 14, 2 }, c);
    DrawRectangleRec({ r.x + 8, bottom - 14, 2, 14 }, c);
}

// ---------- Icons ----------

void DrawIcon(Icon icon, Vector2 center, float size, Color color) {
    float k = size / 24.0f;      // icons are designed on a 24x24 grid
    float thick = 2.0f * k;
    // P turns a point on the 24x24 grid into a screen position.
    auto P = [&](float x, float y) { return Vector2{ center.x + (x - 12) * k, center.y + (y - 12) * k }; };
    auto Line = [&](float x1, float y1, float x2, float y2) { DrawLineEx(P(x1, y1), P(x2, y2), thick, color); };
    auto Ring = [&](float cx, float cy, float radius, float a0, float a1) {
        DrawRing(P(cx, cy), radius * k - thick / 2, radius * k + thick / 2, a0, a1, 32, color);
    };
    auto Poly = [&](const float* xy, int count) {
        for (int i = 0; i < count - 1; i++)
            Line(xy[i * 2], xy[i * 2 + 1], xy[i * 2 + 2], xy[i * 2 + 3]);
    };

    switch (icon) {
        case Icon::None:
            break;
        case Icon::Crosshair:
            Ring(12, 12, 7, 0, 360);
            Line(12, 2, 12, 7); Line(12, 17, 12, 22); Line(2, 12, 7, 12); Line(17, 12, 22, 12);
            break;
        case Icon::Shield: {
            const float pts[] = { 12, 3, 20, 6, 20, 12, 19, 15.5f, 16.5f, 18.6f, 12, 21, 7.5f, 18.6f, 5, 15.5f, 4, 12, 4, 6, 12, 3 };
            Poly(pts, 11);
            break;
        }
        case Icon::Wrench:
            Ring(15, 9, 4, -15, 285);   // the open jaw
            DrawLineEx(P(5.5f, 18.5f), P(12.2f, 11.8f), thick * 1.5f, color);
            break;
        case Icon::Exit:
            Line(14, 4, 20, 4); Line(20, 4, 20, 20); Line(20, 20, 14, 20);
            Line(10, 8, 6, 12); Line(6, 12, 10, 16); Line(6, 12, 16, 12);
            break;
        case Icon::Play:
            DrawTriangle(P(7, 4), P(7, 20), P(20, 12), color);
            break;
        case Icon::Book:
            DrawRectangleLinesEx({ P(4, 5).x, P(4, 5).y, 7 * k, 14 * k }, thick, color);
            DrawRectangleLinesEx({ P(13, 5).x, P(13, 5).y, 7 * k, 14 * k }, thick, color);
            break;
        case Icon::Gear:
            Ring(12, 12, 3.2f, 0, 360);
            Line(12, 2, 12, 5); Line(12, 19, 12, 22); Line(2, 12, 5, 12); Line(19, 12, 22, 12);
            Line(4.9f, 4.9f, 7, 7); Line(17, 17, 19.1f, 19.1f); Line(4.9f, 19.1f, 7, 17); Line(17, 7, 19.1f, 4.9f);
            break;
        case Icon::Restart:
            Ring(12, 12, 8, -135, 180);
            Line(4, 4, 4, 9); Line(4, 9, 9, 9);
            break;
        case Icon::Home:
            Line(3, 11, 12, 4); Line(12, 4, 21, 11);
            Line(5, 10, 5, 20); Line(5, 20, 19, 20); Line(19, 20, 19, 10);
            break;
        case Icon::Speaker:
        case Icon::SpeakerOff: {
            const float pts[] = { 4, 9, 8, 9, 13, 5, 13, 19, 8, 15, 4, 15, 4, 9 };
            Poly(pts, 7);
            if (icon == Icon::Speaker) {
                Ring(13, 12, 4.5f, -45, 45);
                Ring(13, 12, 8, -45, 45);
            } else {
                Line(17, 9, 22, 15); Line(22, 9, 17, 15);
            }
            break;
        }
        case Icon::Music:
            Line(9, 18, 9, 5); Line(9, 5, 20, 3); Line(20, 3, 20, 16);
            Ring(6, 18, 3, 0, 360); Ring(17, 16, 3, 0, 360);
            break;
        case Icon::Motion:
            for (int i = 0; i < 12; i++) {
                float x0 = 3 + i * 1.5f, x1 = x0 + 1.5f;
                Line(x0, 12 - 4 * sinf((x0 - 3) / 18 * 2 * PI), x1, 12 - 4 * sinf((x1 - 3) / 18 * 2 * PI));
            }
            break;
    }
}

void DrawIconSocket(Rectangle r, float cut, bool filled) {
    if (filled) {
        FillChamfer(r, cut, Col(COL_AMBER_LIGHT), Col(COL_AMBER_SHADE));
    } else {
        FillChamfer(r, cut, Col(COL_AMBER, 0.22f), Col(COL_AMBER, 0.05f));
        OutlineChamfer(r, cut, Col(COL_AMBER, 0.42f));
    }
}

// ---------- Buttons ----------

Button MakeButton(Rectangle rect, const char* label, const char* hint, const char* key, Icon icon, bool primary) {
    Button b;
    b.rect = rect;
    b.label = label;
    b.hint = hint;
    b.key = key;
    b.icon = icon;
    b.primary = primary;
    b.centered = false;
    b.hovered = false;
    b.hoverTime = 10.0f;
    return b;
}

bool UpdateButton(Button& button, bool enabled) {
    bool over = enabled && CheckCollisionPointRec(VirtualMouse(), button.rect);
    if (over && !button.hovered)
        button.hoverTime = 0.0f;  // restart the sheen each time the mouse arrives
    button.hovered = over;
    button.hoverTime += GetFrameTime();
    return over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DrawKeyHint(const char* key, float rightX, float centerY, Color text, Color border) {
    float w = MeasureLabel(FontKind::Mono, key, 12, 0.0f) + 14;
    Rectangle box = { rightX - w, centerY - 10, w, 20 };
    DrawRectangleLinesEx(box, 1, border);
    DrawLabel(FontKind::Mono, key, box.x + 7, box.y + 4, 12, 0.0f, text);
}

// A light band that sweeps across a button, leaning like a "/".
static void DrawSheen(Rectangle r, float progress) {
    float bandW = r.width * 0.35f;
    float left = r.x - r.width * 0.5f + progress * r.width * 1.7f;
    float lean = r.height * 0.364f / 2;  // tan(20 degrees) times half the height
    Color clear = Rgba(255, 236, 200, 0);
    Color bright = Rgba(255, 236, 200, 0.28f);
    float mid = left + bandW / 2;
    Vector2 leftHalf[4] = { { left + lean, r.y }, { mid + lean, r.y }, { mid - lean, r.y + r.height }, { left - lean, r.y + r.height } };
    Vector2 rightHalf[4] = { { mid + lean, r.y }, { left + bandW + lean, r.y }, { left + bandW - lean, r.y + r.height }, { mid - lean, r.y + r.height } };
    // BeginScissorMode clips drawing to the button's rectangle.
    BeginScissorMode((int)r.x, (int)r.y, (int)r.width, (int)r.height);
    FillPolygonLinear(leftHalf, 4, { left - lean, 0 }, { mid - lean, 0 }, clear, bright);
    FillPolygonLinear(rightHalf, 4, { mid - lean, 0 }, { left + bandW - lean, 0 }, bright, clear);
    EndScissorMode();
}

void DrawButton(const Button& button, bool enabled, float t) {
    const float CUT = 10;
    Rectangle r = button.rect;
    bool pressed = enabled && button.hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    if (pressed)
        r.y += 2;

    if (button.primary) {
        // Pulsing amber glow: soft outlines that grow outward and fade.
        float strength = enabled ? 1.0f - 0.6f * Pulse(t, 2.4f) : 0.0f;
        for (int i = 1; i <= 7; i++) {
            float g = i * 2.0f;
            Rectangle grown = { r.x - g, r.y - g, r.width + 2 * g, r.height + 2 * g };
            OutlineChamfer(grown, CUT + g * 0.41f, Col(COL_AMBER, 0.2f * strength * (1.0f - i / 8.0f)));
        }
        if (button.hovered)
            FillChamfer(r, CUT, Rgba(255, 208, 138, 1), Rgba(247, 169, 64, 1));
        else
            FillChamfer(r, CUT, Col(COL_AMBER_LIGHT), Col(COL_AMBER_SHADE));
        // Darker lip along the bottom and a bright line along the top.
        Vector2 lip[4] = { { r.x, r.y + r.height - 4 }, { r.x + r.width - CUT + 4, r.y + r.height - 4 }, { r.x + r.width - CUT, r.y + r.height }, { r.x, r.y + r.height } };
        FillPolygonLinear(lip, 4, { 0, 0 }, { 0, 1 }, Rgba(120, 58, 0, 0.45f), Rgba(120, 58, 0, 0.45f));
        DrawRectangleRec({ r.x + CUT, r.y, r.width - CUT, 1 }, Rgba(255, 244, 220, 0.8f));
        // The sheen sweeps across every 3.2 seconds, easing in and out.
        float s = fmodf(t, 3.2f) / 3.2f;
        DrawSheen(r, s * s * (3 - 2 * s));
    } else {
        if (button.hovered) {
            FillChamfer(r, CUT, Rgba(64, 46, 30, 0.95f), Rgba(28, 20, 13, 0.95f));
            for (int i = 1; i <= 6; i++) {
                float g = i * 2.0f;
                Rectangle shrunk = { r.x + g, r.y + g, r.width - 2 * g, r.height - 2 * g };
                OutlineChamfer(shrunk, CUT - g * 0.41f, Col(COL_AMBER, 0.06f * (1.0f - i / 7.0f)));
            }
            OutlineChamfer(r, CUT, Col(COL_AMBER));
            if (button.hoverTime < 0.65f)
                DrawSheen(r, button.hoverTime / 0.65f);
        } else {
            FillChamfer(r, CUT, Rgba(44, 32, 22, 0.95f), Rgba(20, 14, 10, 0.95f));
            OutlineChamfer(r, CUT, Col(COL_AMBER, 0.32f));
        }
        DrawRectangleRec({ r.x + CUT, r.y + 1, r.width - CUT - 1, 1 }, Rgba(255, 226, 180, 0.18f));
    }

    // Icon socket on the left.
    float textX = r.x + 18;
    if (button.icon != Icon::None) {
        Rectangle socket = { r.x + 7, r.y + (r.height - 48) / 2, 48, 48 };
        if (button.primary) {
            FillChamfer(socket, 8, Col(COL_INK, 0.12f), Col(COL_INK, 0.12f));
            OutlineChamfer(socket, 8, Col(COL_INK, 0.35f));
        } else {
            FillChamfer(socket, 8, Col(COL_AMBER, 0.2f), Col(COL_AMBER, 0.06f));
            OutlineChamfer(socket, 8, Col(COL_AMBER, 0.4f));
        }
        Vector2 c = { socket.x + 24, socket.y + 24 };
        DrawIcon(button.icon, c, 22, button.primary ? Col(COL_INK) : Col(COL_AMBER));
        textX = socket.x + 48 + 14;
    }

    Color labelColor = button.primary ? Col(COL_INK) : Col(COL_TEXT);
    Color hintColor = button.primary ? Rgba(91, 58, 18, 1) : Col(COL_DIM);
    float centerY = r.y + r.height / 2;
    if (button.centered) {
        DrawLabelCentered(FontKind::Display, button.label, r.x + r.width / 2, centerY - 12, 22, 0.1f, labelColor);
    } else if (button.hint != nullptr) {
        DrawLabel(FontKind::Display, button.label, textX, centerY - 20, 22, 0.1f, labelColor);
        DrawLabel(FontKind::Mono, button.hint, textX, centerY + 6, 12, 0.08f, hintColor);
    } else {
        DrawLabel(FontKind::Display, button.label, textX, centerY - 12, 22, 0.1f, labelColor);
    }

    if (button.key != nullptr) {
        Color border = button.primary ? Rgba(91, 58, 18, 0.4f) : Col(COL_AMBER, 0.3f);
        DrawKeyHint(button.key, r.x + r.width - 18, centerY, hintColor, border);
    }

    // Disabled: cover the button with see-through background so it looks faded.
    if (!enabled)
        FillChamfer(r, CUT, Col(COL_BG, 0.62f), Col(COL_BG, 0.62f));
}

// ---------- Status pieces ----------

HullBar MakeHullBar(int hull) {
    HullBar bar;
    bar.shown = (float)hull;
    bar.ghost = (float)hull;
    bar.ghostWait = 0.0f;
    return bar;
}

void UpdateHullBar(HullBar& bar, int hull, float dt) {
    float target = (float)hull;
    if (target < bar.shown - 0.5f && bar.ghost <= bar.shown + 0.5f)
        bar.ghostWait = 0.35f;  // a new hit: the ghost waits a moment before following
    // Move a fixed share of the remaining distance each second: fast at first, then gentle.
    bar.shown += (target - bar.shown) * (1.0f - expf(-dt * 10.0f));
    if (bar.ghost < bar.shown) {
        bar.ghost = bar.shown;  // repairs: no ghost needed
    } else if (bar.ghostWait > 0) {
        bar.ghostWait -= dt;
    } else {
        bar.ghost += (bar.shown - bar.ghost) * (1.0f - expf(-dt * 3.5f));
    }
}

void DrawHullBar(const HullBar& bar, Rectangle r, bool fromRight, float t, bool reduceMotion) {
    unsigned int edge = fromRight ? COL_RED : COL_AMBER;
    // The bar is a slanted box (a parallelogram).
    Vector2 p[4] = { { r.x + 7, r.y }, { r.x + r.width, r.y }, { r.x + r.width - 7, r.y + r.height }, { r.x, r.y + r.height } };
    FillPolygonLinear(p, 4, { 0, 0 }, { 0, 1 }, Rgba(0, 0, 0, 0.55f), Rgba(0, 0, 0, 0.55f));
    for (int i = 0; i < 4; i++)
        DrawLineV(p[i], p[(i + 1) % 4], Col(edge, fromRight ? 0.45f : 0.4f));

    Rectangle inner = { r.x + 6, r.y + 3, r.width - 12, r.height - 6 };
    float ghostW = inner.width * bar.ghost / HULL_MAX;
    float fillW = inner.width * bar.shown / HULL_MAX;
    float ghostX = fromRight ? inner.x + inner.width - ghostW : inner.x;
    float fillX = fromRight ? inner.x + inner.width - fillW : inner.x;
    DrawRectangleRec({ ghostX, inner.y, ghostW, inner.height }, Rgba(255, 243, 221, 0.55f));

    // Below 30 hull the fill blinks to warn the player.
    bool low = bar.shown <= LOW_HULL + 0.5f;
    float fillAlpha = (low && !Blink(t, 0.7f)) ? 0.35f : 1.0f;
    if (reduceMotion && low)
        fillAlpha = 0.7f;
    Color top = fromRight ? Rgba(255, 139, 127, fillAlpha) : Rgba(255, 213, 144, fillAlpha);
    Color bottom = fromRight ? Rgba(217, 47, 31, fillAlpha) : Rgba(242, 154, 46, fillAlpha);
    for (int i = 1; i <= 3; i++) {
        float g = i * 2.0f;
        DrawRectangleLinesEx({ fillX - g, inner.y - g, fillW + 2 * g, inner.height + 2 * g }, 2, Col(edge, 0.12f * fillAlpha / i));
    }
    DrawRectangleGradientV((int)fillX, (int)inner.y, (int)fillW, (int)inner.height, top, bottom);

    // Dark gaps every 19 pixels turn the bar into segments.
    for (float x = r.x + 17; x < r.x + r.width - 6; x += 19)
        DrawRectangleRec({ x, r.y + 1, 2, r.height - 2 }, Rgba(11, 8, 7, 0.95f));
}

// A hexagon with pointy left and right ends.
static void HexPoints(Vector2 c, float size, Vector2* p) {
    float h = size / 2;
    p[0] = { c.x - h * 0.5f, c.y - h * 0.94f };
    p[1] = { c.x + h * 0.5f, c.y - h * 0.94f };
    p[2] = { c.x + h, c.y };
    p[3] = { c.x + h * 0.5f, c.y + h * 0.94f };
    p[4] = { c.x - h * 0.5f, c.y + h * 0.94f };
    p[5] = { c.x - h, c.y };
}

void DrawHexBadge(Vector2 center, float size, Texture2D ship, PanelTone tone) {
    bool red = (tone == PanelTone::Red);
    Vector2 outer[6];
    HexPoints(center, size, outer);
    Color rimLight = red ? Rgba(255, 139, 127, 1) : Rgba(255, 207, 122, 1);
    Color rimDark = red ? Rgba(110, 26, 18, 1) : Rgba(122, 74, 20, 1);
    FillPolygonLinear(outer, 6, { center.x - size * 0.17f, center.y - size * 0.47f }, { center.x + size * 0.17f, center.y + size * 0.47f }, rimLight, rimDark);

    // Inside: a radial glow, drawn as triangles from the center outward.
    Vector2 inner[6];
    HexPoints(center, size - 5, inner);
    Color mid = red ? Rgba(58, 23, 18, 1) : Rgba(58, 38, 23, 1);
    Color edge = red ? Rgba(18, 10, 9, 1) : Rgba(18, 13, 10, 1);
    for (int i = 0; i < 6; i++) {
        Vector2 tri[3] = { center, inner[i], inner[(i + 1) % 6] };
        rlSetTexture(rlGetTextureIdDefault());
        rlBegin(RL_TRIANGLES);
        for (int j = 0; j < 3; j++) {
            Color c = (j == 0) ? mid : edge;
            rlColor4ub(c.r, c.g, c.b, (unsigned char)(255 * uiFade));
            rlVertex2f(tri[j].x, tri[j].y);
        }
        rlEnd();
        rlSetTexture(0);
    }
    DrawShip(ship, center, size * 0.61f, 0, Col(0xFFFFFFFF));
}

void DrawLockOn(Rectangle r, float t, bool animate) {
    float pulse = animate ? Pulse(t, 1.6f) : 0.0f;
    float grow = 1.0f + 0.1f * pulse;
    Vector2 c = { r.x + r.width / 2, r.y + r.height / 2 };
    float hw = r.width / 2 * grow, hh = r.height / 2 * grow;
    Color red = Col(COL_RED, 1.0f - 0.45f * pulse);
    const float L = 18, T = 2;
    float left = c.x - hw, right = c.x + hw, top = c.y - hh, bottom = c.y + hh;
    DrawRectangleRec({ left, top, L, T }, red);
    DrawRectangleRec({ left, top, T, L }, red);
    DrawRectangleRec({ right - L, top, L, T }, red);
    DrawRectangleRec({ right - T, top, T, L }, red);
    DrawRectangleRec({ left, bottom - T, L, T }, red);
    DrawRectangleRec({ left, bottom - L, T, L }, red);
    DrawRectangleRec({ right - L, bottom - T, L, T }, red);
    DrawRectangleRec({ right - T, bottom - L, T, L }, red);
}

void DrawHazardTape(float y, float height, bool red, float alpha, float t, bool animate) {
    const float PERIOD = 33.94f;   // one stripe plus one gap, measured along the tape
    const float STRIPE = 16.97f;   // a 12 px stripe at 45 degrees
    DrawRectangleRec({ 0, y, (float)VIRTUAL_W, height }, Col(COL_BG, alpha));
    float shift = animate ? fmodf(t / 1.4f, 1.0f) * PERIOD : 0.0f;
    Color c = Col(red ? COL_RED : COL_AMBER, alpha);
    BeginScissorMode(0, (int)y, VIRTUAL_W, (int)height);
    for (float x = -PERIOD * 2 + shift; x < VIRTUAL_W + PERIOD; x += PERIOD) {
        Vector2 stripe[4] = { { x + height, y }, { x + height + STRIPE, y }, { x + STRIPE, y + height }, { x, y + height } };
        FillPolygonLinear(stripe, 4, { 0, 0 }, { 0, 1 }, c, c);
    }
    EndScissorMode();
}

void DrawShip(Texture2D ship, Vector2 center, float width, float rotation, Color tint) {
    float scale = width / ship.width;
    Rectangle source = { 0, 0, (float)ship.width, (float)ship.height };
    Rectangle dest = { center.x, center.y, ship.width * scale, ship.height * scale };
    Vector2 origin = { dest.width / 2, dest.height / 2 };  // spin around the middle
    DrawTexturePro(ship, source, dest, origin, 90.0f + rotation, tint);
}
