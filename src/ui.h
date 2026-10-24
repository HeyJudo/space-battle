// ui.h - the shared drawing kit for every screen: colors, text, cut-corner
// panels, buttons, hull bars, hex badges, hazard tape and lock-on brackets.
// Everything is drawn in 1280x720 virtual space.
#pragma once
#include "raylib.h"
#include "assets.h"

// ---------- Color and fade ----------

// A palette color (see constants.h) with extra transparency: Col(COL_AMBER, 0.4f).
// It also applies the current UI fade, used when panels fade in.
Color Col(unsigned int hex, float alpha = 1.0f);

// Everything drawn after this is see-through by this much (1 = solid). Reset to 1 when done.
void SetUiFade(float alpha);

// ---------- Animation helpers ----------

// 0 to 1 over "duration" seconds, starting "delay" seconds after t = 0, with a soft landing.
float EaseIn(float t, float delay, float duration);

// A smooth 0..1..0 wave that repeats every "period" seconds.
float Pulse(float t, float period);

// True for the first half of every "period" (a hard on/off blink).
bool Blink(float t, float period);

// ---------- Text ----------

// spacingEm is the letter spacing as a fraction of the size, like CSS "letter-spacing: .1em".
void DrawLabel(FontKind kind, const char* text, float x, float y, float size, float spacingEm, Color color);
void DrawLabelRight(FontKind kind, const char* text, float rightX, float y, float size, float spacingEm, Color color);
void DrawLabelCentered(FontKind kind, const char* text, float centerX, float y, float size, float spacingEm, Color color);
float MeasureLabel(FontKind kind, const char* text, float size, float spacingEm);

// Types the text out one letter at a time, starting at "delay". Returns true once fully shown.
bool DrawTyped(FontKind kind, const char* text, float x, float y, float size, float spacingEm,
               Color color, float t, float delay, float duration, bool caret);

// Big title text that now and then splits into red and cream copies for a moment.
void DrawGlitchLabel(const char* text, float x, float y, float size, float spacingEm, Color color, float t, bool enabled);

// ---------- Shapes ----------

// A rectangle with the top-left and bottom-right corners cut off at 45 degrees.
void FillChamfer(Rectangle r, float cut, Color top, Color bottom);
void OutlineChamfer(Rectangle r, float cut, Color color);

// A filled ellipse that fades from "inner" at the focus point to "outer" at the edge.
void FillEllipseGradient(Vector2 center, float rx, float ry, Vector2 focus, Color inner, Color outer);

// ---------- Panels ----------

enum class PanelTone { Amber, Red };

// The main cut-corner panel with a glowing gradient border.
// If title is not null, a header strip is drawn with "> TITLE" and the right label.
void DrawPanel(Rectangle r, const char* title, const char* rightLabel, PanelTone tone = PanelTone::Amber);
const float PANEL_HEADER_H = 38.0f;

// Small amber corner marks (top right and bottom left).
void DrawCornerTicks(Rectangle r);

// ---------- Icons ----------

enum class Icon { None, Crosshair, Shield, Wrench, Exit, Play, Book, Gear, Restart, Home, Speaker, SpeakerOff, Music, Motion };

// Line icons drawn with raylib shapes, on a 24x24 grid scaled to "size".
void DrawIcon(Icon icon, Vector2 center, float size, Color color);

// The cut-corner square an icon sits in.
void DrawIconSocket(Rectangle r, float cut, bool filled);

// ---------- Buttons ----------

struct Button {
    Rectangle rect;
    const char* label;
    const char* hint;      // small text under the label, or nullptr
    const char* key;       // keyboard shortcut box on the right, or nullptr
    Icon icon;             // Icon::None means no icon socket
    bool primary;          // the solid amber "go" style with a pulsing glow
    bool centered;         // label centered (used by small dialog buttons)
    bool hovered;
    float hoverTime;       // seconds since the mouse arrived, for the light sheen
};

Button MakeButton(Rectangle rect, const char* label, const char* hint, const char* key, Icon icon, bool primary);

// Returns true on the frame the button is clicked or tapped.
bool UpdateButton(Button& button, bool enabled);
void DrawButton(const Button& button, bool enabled, float t);

// A small keyboard hint box like [ENTER].
void DrawKeyHint(const char* key, float rightX, float centerY, Color text, Color border);

// ---------- Status pieces ----------

// A hull bar that drops quickly and leaves a fading "ghost" behind.
struct HullBar {
    float shown;        // the bright fill, 0 to 100
    float ghost;        // the pale trail
    float ghostWait;    // seconds before the ghost starts to follow
};

HullBar MakeHullBar(int hull);
void UpdateHullBar(HullBar& bar, int hull, float dt);
// fromRight = true for the Warden, whose bar empties toward the right.
void DrawHullBar(const HullBar& bar, Rectangle r, bool fromRight, float t, bool reduceMotion);

// A hexagon portrait with a ship inside.
void DrawHexBadge(Vector2 center, float size, Texture2D ship, PanelTone tone);

// Four red corner brackets that pulse around a target.
void DrawLockOn(Rectangle r, float t, bool animate);

// Scrolling black-and-amber hazard stripes.
void DrawHazardTape(float y, float height, bool red, float alpha, float t, bool animate);

// A ship sprite rotated 90 degrees so the ships face each other.
void DrawShip(Texture2D ship, Vector2 center, float width, float rotation, Color tint);
