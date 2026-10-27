// constants.h - every game number in one place: rules, timings, layout size and colors.
// It has no raylib include, so the battle logic and the tests can use it too.
// Colors are written as 0xRRGGBBAA numbers; drawing code turns them into
// raylib colors with GetColor(COL_AMBER).
#pragma once

// ---------- Game rules (from the client's original code) ----------
const int HULL_MAX = 100;     // both ships start here and can never repair past it
const int FIRE_MIN = 10;      // Fire: rand() % 16 + 10
const int FIRE_MAX = 25;
const int REPAIR_MIN = 15;    // Repair: rand() % 16 + 15
const int REPAIR_MAX = 30;
const int LOW_HULL = 30;      // the hull bar blinks at or below this

// ---------- Turn timing in seconds (approved in SPEC.md) ----------
const float T_PLAYER_SHOT_LANDS = 0.13f;  // after the click
const float T_HOSTILE_TARGETING = 0.45f;  // after the click
const float T_WARDEN_ACTS = 0.95f;        // after the click
const float T_WARDEN_SHOT_LANDS = 0.14f;  // after the Warden acts
const float T_ROUND_END = 0.6f;           // after the Warden's move resolves
const float T_RESULT_SCREEN = 0.7f;       // after a ship reaches 0
const float T_RETREAT = 0.4f;             // after confirming Retreat

// ---------- Ship motion (Battle screen) ----------
// Beams: they cross the gap fast, then fade out. Total life = cross + fade.
const float BEAM_CROSS_TIME = 0.1f;
const float BEAM_LIFE = 0.3f;

// Drift: each ship traces a slow figure-eight. Different periods keep the two ships out of sync.
const float DRIFT_Y = 26.0f;               // pixels up and down (a 52 px range)
const float DRIFT_X = 18.0f;               // pixels left and right (a 36 px range)
const float HALBERD_DRIFT_PERIOD = 6.0f;   // seconds per figure-eight
const float WARDEN_DRIFT_PERIOD = 7.4f;
const float WARDEN_PHASE = 2.3f;           // the Warden runs this many seconds "later"
const float TILT_DEGREES = 5.0f;           // a slight roll that follows the up and down motion

// Lunge: a ship eases toward the enemy when it fires, then eases back.
const float LUNGE_PX = 26.0f;
const float LUNGE_TIME = 0.3f;             // total, matches the beam
const float LUNGE_PEAK_TIME = 0.08f;       // quick push out, slower glide back

// Recoil: a ship that is hit is pushed away from the attacker and settles back.
const float RECOIL_PX = 18.0f;
const float RECOIL_TIME = 0.5f;

// Streaming stars: on the Battle screen the star layers move this many times faster than
// on other screens, so both ships seem to fly forward (like Galaga).
const float BATTLE_STAR_SPEED = 7.0f;
const float STREAK_SECONDS = 0.05f;        // a fast star leaves a tail this long (tail = speed * this)

// Reduce motion keeps only this gentle bob.
const float BOB_PX = 8.0f;
const float BOB_PERIOD = 4.6f;

// ---------- Virtual screen: everything is drawn at this size, then scaled ----------
const int VIRTUAL_W = 1280;
const int VIRTUAL_H = 720;

// ---------- Palette (Red Alert v2) ----------
const unsigned int COL_BG = 0x0B0807FF;
const unsigned int COL_AMBER = 0xFFB347FF;     // the player and primary actions
const unsigned int COL_AMBER_SHADE = 0xF29A2EFF;
const unsigned int COL_AMBER_LIGHT = 0xFFC36AFF;
const unsigned int COL_RED = 0xFF4433FF;       // the Warden and alarms
const unsigned int COL_RED_SOFT = 0xFF8B7FFF;  // red text that must stay readable
const unsigned int COL_GREEN = 0x9BE36BFF;     // repair
const unsigned int COL_TEXT = 0xF6E6CCFF;
const unsigned int COL_DIM = 0xAD977CFF;
const unsigned int COL_INK = 0x1A0F05FF;       // dark text on amber buttons
const unsigned int COL_PANEL_TOP = 0x241A12FF; // panels fade from top to bottom
const unsigned int COL_PANEL_BOTTOM = 0x0F0B08FF;
