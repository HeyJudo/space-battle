// screen_battle.cpp - the Battle screen.
// The rules in battle.cpp decide a whole round instantly. This screen then
// plays the round back slowly with a "turn sequencer": a list of timed steps
// (shot fired, shot lands, Warden targets, Warden acts, round ends). Each step
// turns the rule events into animations, sounds and tactical feed lines.
#include "raylib.h"
#include "audio.h"
#include "background.h"
#include "battle.h"
#include "constants.h"
#include "game.h"
#include "strings.h"
#include "ui.h"
#include "viewport.h"
#include <cmath>
#include <ctime>
#include <string>
#include <vector>

// ---------- Rules state ----------

static Battle battle;  // the true state, already a round ahead while animations play
static Rng rng;
static std::vector<Event> playerEvents;  // this round's events, waiting to be shown
static std::vector<Event> wardenEvents;

// ---------- Turn sequencer ----------

enum class Step { PlayerShot, PlayerLands, HostileTargeting, WardenActs, WardenLands, RoundEnds, ShowResult, BackToTitle };

struct TimedStep {
    float time;  // seconds after the command was given
    Step step;
};

static std::vector<TimedStep> steps;
static float roundClock;

// ---------- What the screen shows ----------

enum class Phase { AwaitingOrders, HostileTargeting, Incoming, StandDown };

static Phase phase;
static int shownPlayerHull, shownWardenHull, shownEngagement;
static HullBar playerBar, wardenBar;
static bool playerBubble, wardenBubble;
static float playerBubbleAge, wardenBubbleAge;
static float alertLevel;  // 0..1, fades the red alert wash in and out

struct FloatingText {
    bool onWarden;
    std::string text;
    unsigned int color;
    bool small;    // "SHIELDED" uses small mono text
    float age;
};

struct FeedLine {
    int engagement;
    std::string text;
    unsigned int color;
    float age;
};

struct Particle {
    Vector2 pos, vel;
    float age, life, size;
    unsigned int color;
};

static std::vector<FloatingText> floats;
static std::vector<FeedLine> feed;  // newest first
static std::vector<Particle> particles;

// Seconds since each effect started. A big number means "not playing".
const float NEVER = 99.0f;
static float playerHitAge, wardenHitAge, playerHealAge, wardenHealAge;
static float playerBeamAge, wardenBeamAge, playerDeadAge, wardenDeadAge, shakeAge;
static float shakeStrength;

static bool confirmOpen;
static float confirmAge;
static Button fireButton, shieldsButton, repairButton, retreatButton, stayButton, leaveButton;
static const Rectangle MUTE_RECT = { 1206, 548, 34, 26 };

// Ship positions (centers) in 1280x720 space.
static const Vector2 HALBERD_POS = { 270, 330 };
static const Vector2 WARDEN_POS = { 1010, 330 };

// ---------- Small helpers ----------

static void AddFeed(const char* text, unsigned int color) {
    feed.insert(feed.begin(), { shownEngagement, text, color, 0.0f });
    if (feed.size() > 5)
        feed.pop_back();
}

static void AddFloat(bool onWarden, const char* text, unsigned int color, bool small) {
    floats.push_back({ onWarden, text, color, small, 0.0f });
}

static void Explode(Vector2 at) {
    for (int i = 0; i < 46; i++) {
        float angle = GetRandomValue(0, 359) * DEG2RAD;
        float speed = (float)GetRandomValue(60, 340);
        unsigned int colors[3] = { COL_AMBER, COL_RED, 0xFFF3DDFF };
        particles.push_back({ at, { cosf(angle) * speed, sinf(angle) * speed }, 0.0f,
                              GetRandomValue(50, 120) / 100.0f, (float)GetRandomValue(2, 5), colors[i % 3] });
    }
}

static void Shake(float strength) {
    if (settings.reduceMotion)
        return;
    shakeAge = 0.0f;
    shakeStrength = strength;
}

static bool HasEvent(const std::vector<Event>& events, EventType type) {
    for (const Event& e : events)
        if (e.type == type)
            return true;
    return false;
}

static bool IsBusy() {
    return !steps.empty() || battle.result != Result::Ongoing;
}

// ---------- Events become pictures, sounds and words ----------

static void ShowEvents(const std::vector<Event>& events) {
    bool deflected = HasEvent(events, EventType::WardenShieldsHeld) || HasEvent(events, EventType::PlayerShieldsHeld);
    for (const Event& e : events) {
        switch (e.type) {
            case EventType::WardenShieldsHeld:
                AddFloat(true, TXT_SHIELDED, COL_AMBER, true);
                AddFeed(FEED_WARDEN_SHIELDS_HELD, COL_TEXT);
                break;
            case EventType::PlayerFired:
                shownWardenHull = (shownWardenHull > e.amount) ? shownWardenHull - e.amount : 0;
                AddFloat(true, TextFormat("-%d", e.amount), 0xFF5A48FF, false);
                AddFeed(TextFormat(FEED_FIRE, e.amount), COL_AMBER);
                wardenHitAge = 0.0f;
                PlaySfx(deflected ? Sfx::Deflect : Sfx::Hit);
                break;
            case EventType::PlayerRaisedShields:
                playerBubble = true;
                playerBubbleAge = 0.0f;
                AddFeed(FEED_SHIELDS, COL_TEXT);
                PlaySfx(Sfx::Shield);
                break;
            case EventType::PlayerRepaired:
                shownPlayerHull += e.amount;
                AddFloat(false, TextFormat("+%d", e.amount), COL_GREEN, false);
                AddFeed(TextFormat(FEED_REPAIR, e.amount), COL_GREEN);
                playerHealAge = 0.0f;
                PlaySfx(Sfx::Repair);
                break;
            case EventType::PlayerRetreated:
                AddFeed(FEED_RETREAT, COL_TEXT);
                break;
            case EventType::PlayerShieldsHeld:
                AddFloat(false, TXT_SHIELDED, COL_AMBER, true);
                AddFeed(FEED_SHIELDS_HELD, COL_TEXT);
                break;
            case EventType::WardenFired:
                shownPlayerHull = (shownPlayerHull > e.amount) ? shownPlayerHull - e.amount : 0;
                AddFloat(false, TextFormat("-%d", e.amount), 0xFF5A48FF, false);
                AddFeed(TextFormat(FEED_WARDEN_FIRE, e.amount), COL_RED_SOFT);
                playerHitAge = 0.0f;
                Shake(deflected ? 3.0f : 7.0f);
                PlaySfx(deflected ? Sfx::Deflect : Sfx::Hit);
                break;
            case EventType::WardenRaisedShields:
                wardenBubble = true;
                wardenBubbleAge = 0.0f;
                AddFeed(FEED_WARDEN_SHIELDS, COL_RED_SOFT);
                PlaySfx(Sfx::Shield);
                break;
            case EventType::WardenRepaired:
                shownWardenHull += e.amount;
                AddFloat(true, TextFormat("+%d", e.amount), COL_GREEN, false);
                AddFeed(TextFormat(FEED_WARDEN_REPAIR, e.amount), COL_RED_SOFT);
                wardenHealAge = 0.0f;
                PlaySfx(Sfx::Repair);
                break;
            case EventType::Victory:
                wardenDeadAge = 0.0f;
                wardenBubble = false;
                phase = Phase::StandDown;
                Explode(WARDEN_POS);
                Shake(12.0f);
                PlaySfx(Sfx::Explosion);
                break;
            case EventType::Defeat:
                playerDeadAge = 0.0f;
                playerBubble = false;
                phase = Phase::StandDown;
                Explode(HALBERD_POS);
                Shake(14.0f);
                PlaySfx(Sfx::Explosion);
                break;
        }
    }
}

static void RunStep(Step step) {
    switch (step) {
        case Step::PlayerShot:
            playerBeamAge = 0.0f;
            PlaySfx(Sfx::Laser);
            break;
        case Step::PlayerLands:
            wardenBubble = false;  // any command uses up or clears the Warden's shields
            ShowEvents(playerEvents);
            break;
        case Step::HostileTargeting:
            phase = Phase::HostileTargeting;
            PlaySfx(Sfx::Alarm);
            break;
        case Step::WardenActs:
            phase = Phase::Incoming;
            if (HasEvent(wardenEvents, EventType::WardenFired)) {
                wardenBeamAge = 0.0f;
                PlaySfx(Sfx::EnemyLaser);
            }
            break;
        case Step::WardenLands:
            ShowEvents(wardenEvents);
            break;
        case Step::RoundEnds:
            phase = Phase::AwaitingOrders;
            playerBubble = false;
            shownEngagement = battle.engagement;
            break;
        case Step::ShowResult:
            debrief.engagements = battle.engagement;
            debrief.damageDealt = battle.damageDealt;
            debrief.playerHull = battle.playerHull;
            debrief.wardenHull = battle.wardenHull;
            GoToScreen(battle.result == Result::Victory ? Screen::Victory : Screen::Defeat);
            break;
        case Step::BackToTitle:
            GoToScreen(Screen::Title);
            break;
    }
}

// The player gave a command: decide the whole round now, then schedule how it plays out.
static void GiveCommand(Command command) {
    PlaySfx(Sfx::Click);
    playerEvents = PlayerMove(battle, command, rng);
    wardenEvents = WardenMove(battle, rng);
    roundClock = 0.0f;
    steps.clear();

    if (command == Command::Retreat) {
        phase = Phase::StandDown;
        ShowEvents(playerEvents);
        steps.push_back({ T_RETREAT, Step::BackToTitle });
        return;
    }

    float lands = 0.0f;
    if (command == Command::Fire) {
        steps.push_back({ 0.0f, Step::PlayerShot });
        lands = T_PLAYER_SHOT_LANDS;
    }
    steps.push_back({ lands, Step::PlayerLands });

    if (battle.result == Result::Victory) {
        steps.push_back({ lands + T_RESULT_SCREEN, Step::ShowResult });
        return;
    }

    steps.push_back({ T_HOSTILE_TARGETING, Step::HostileTargeting });
    steps.push_back({ T_WARDEN_ACTS, Step::WardenActs });
    float wardenLands = T_WARDEN_ACTS;
    if (HasEvent(wardenEvents, EventType::WardenFired))
        wardenLands += T_WARDEN_SHOT_LANDS;
    steps.push_back({ wardenLands, Step::WardenLands });

    if (battle.result == Result::Defeat)
        steps.push_back({ wardenLands + T_RESULT_SCREEN, Step::ShowResult });
    else
        steps.push_back({ wardenLands + T_ROUND_END, Step::RoundEnds });
}

// ---------- Enter / Update ----------

void EnterBattle() {
    battle = NewBattle();
    rng = MakeRng((unsigned int)time(nullptr));
    playerEvents.clear();
    wardenEvents.clear();
    steps.clear();
    phase = Phase::AwaitingOrders;
    shownPlayerHull = battle.playerHull;
    shownWardenHull = battle.wardenHull;
    shownEngagement = battle.engagement;
    playerBar = MakeHullBar(shownPlayerHull);
    wardenBar = MakeHullBar(shownWardenHull);
    playerBubble = wardenBubble = false;
    alertLevel = 0.0f;
    floats.clear();
    feed.clear();
    particles.clear();
    playerHitAge = wardenHitAge = playerHealAge = wardenHealAge = NEVER;
    playerBeamAge = wardenBeamAge = playerDeadAge = wardenDeadAge = shakeAge = NEVER;
    confirmOpen = false;
    AddFeed(FEED_START, COL_TEXT);

    fireButton = MakeButton({ 38, 554, 381, 58 }, TXT_FIRE, TXT_FIRE_HINT, "1", Icon::Crosshair, true);
    fireButton.glow = false;
    shieldsButton = MakeButton({ 429, 554, 381, 58 }, TXT_SHIELDS, TXT_SHIELDS_HINT, "2", Icon::Shield, false);
    repairButton = MakeButton({ 38, 622, 381, 58 }, TXT_REPAIR, TXT_REPAIR_HINT, "3", Icon::Wrench, false);
    retreatButton = MakeButton({ 429, 622, 381, 58 }, TXT_RETREAT, TXT_RETREAT_HINT, "4", Icon::Exit, false);
    stayButton = MakeButton({ 435, 0, 200, 62 }, TXT_STAY, nullptr, nullptr, Icon::None, true);
    stayButton.centered = true;
    leaveButton = MakeButton({ 645, 0, 200, 62 }, TXT_RETREAT, nullptr, nullptr, Icon::None, false);
    leaveButton.centered = true;
}

static void UpdateConfirm() {
    confirmAge += GetFrameTime();
    bool stay = UpdateButton(stayButton, true) || IsKeyPressed(KEY_ESCAPE);
    bool leave = UpdateButton(leaveButton, true) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_FOUR);
    if (stay) {
        PlaySfx(Sfx::Click);
        confirmOpen = false;
    } else if (leave) {
        confirmOpen = false;
        GiveCommand(Command::Retreat);
    }
}

void UpdateBattle() {
    float dt = GetFrameTime();

    // Run every step whose time has come, in order.
    roundClock += dt;
    while (!steps.empty() && steps.front().time <= roundClock) {
        Step step = steps.front().step;
        steps.erase(steps.begin());
        RunStep(step);
        if (step == Step::ShowResult || step == Step::BackToTitle)
            return;  // another screen has taken over
    }

    // Timers for the effects.
    playerHitAge += dt; wardenHitAge += dt; playerHealAge += dt; wardenHealAge += dt;
    playerBeamAge += dt; wardenBeamAge += dt; playerDeadAge += dt; wardenDeadAge += dt; shakeAge += dt;
    playerBubbleAge += dt; wardenBubbleAge += dt;
    for (FloatingText& f : floats) f.age += dt;
    for (FeedLine& line : feed) line.age += dt;
    for (Particle& p : particles) {
        p.age += dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.x *= 0.96f;
        p.vel.y *= 0.96f;
    }
    // Remove finished floating numbers and sparks.
    for (int i = (int)floats.size() - 1; i >= 0; i--)
        if (floats[i].age > 1.2f) floats.erase(floats.begin() + i);
    for (int i = (int)particles.size() - 1; i >= 0; i--)
        if (particles[i].age > particles[i].life) particles.erase(particles.begin() + i);

    UpdateHullBar(playerBar, shownPlayerHull, dt);
    UpdateHullBar(wardenBar, shownWardenHull, dt);
    bool hot = (phase == Phase::HostileTargeting || phase == Phase::Incoming);
    float target = hot ? 1.0f : 0.0f;
    alertLevel += (target - alertLevel) * (1.0f - expf(-dt * 10.0f));

    // The mute button in the feed header switches all sound on or off.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(VirtualMouse(), MUTE_RECT) && !confirmOpen) {
        bool anyOn = settings.soundEffects || settings.music;
        settings.soundEffects = !anyOn;
        settings.music = !anyOn;
    }

    if (confirmOpen) {
        UpdateConfirm();
        return;
    }

    bool ready = !IsBusy();
    bool fire = UpdateButton(fireButton, ready) || (ready && IsKeyPressed(KEY_ONE));
    bool shields = UpdateButton(shieldsButton, ready) || (ready && IsKeyPressed(KEY_TWO));
    bool repair = UpdateButton(repairButton, ready) || (ready && IsKeyPressed(KEY_THREE));
    bool retreat = UpdateButton(retreatButton, ready) || (ready && IsKeyPressed(KEY_FOUR));
    if (fire) GiveCommand(Command::Fire);
    else if (shields) GiveCommand(Command::Shields);
    else if (repair) GiveCommand(Command::Repair);
    else if (retreat) {
        PlaySfx(Sfx::Click);
        confirmOpen = true;
        confirmAge = 0.0f;
    }
}

// ---------- Drawing ----------

// A ship card: hex portrait, name, hull number and hull bar.
static void DrawShipCard(bool warden, float now) {
    float x = warden ? 816.0f : 24.0f;
    Rectangle card = { x, 26, 440, 96 };
    DrawPanel(card, nullptr, nullptr);
    const char* number = TextFormat("%d", (int)roundf(warden ? wardenBar.shown : playerBar.shown));
    if (!warden) {
        DrawHexBadge({ 72, 74 }, 72, assets.halberd, PanelTone::Amber);
        DrawLabel(FontKind::Display, TXT_HALBERD, 122, 52.5f, 22, 0.14f, Col(COL_TEXT));
        float nameW = MeasureLabel(FontKind::Display, TXT_HALBERD, 22, 0.14f);
        DrawLabel(FontKind::Mono, TXT_HULL, 122 + nameW + 18, 62, 12, 0.16f, Col(COL_DIM));
        DrawLabelRight(FontKind::Display, number, 446, 40, 34, 0.0f, Col(COL_TEXT));
        DrawHullBar(playerBar, { 122, 87, 324, 18 }, false, now, settings.reduceMotion);
    } else {
        DrawHexBadge({ 1208, 74 }, 72, assets.warden, PanelTone::Red);
        float hullW = MeasureLabel(FontKind::Mono, TXT_HULL, 12, 0.16f);
        DrawLabel(FontKind::Mono, TXT_HULL, 1158 - hullW, 62, 12, 0.16f, Col(COL_DIM));
        DrawLabelRight(FontKind::Display, TXT_WARDEN, 1158 - hullW - 18, 52.5f, 22, 0.14f, Col(COL_TEXT));
        DrawLabel(FontKind::Display, number, 828, 40, 34, 0.0f, Col(COL_TEXT));
        DrawHullBar(wardenBar, { 828, 87, 322, 18 }, true, now, settings.reduceMotion);
    }
}

// The engagement number and the phase readout, top center.
static void DrawPlate(float now) {
    Rectangle plate = { 522, 26, 236, 96 };
    DrawPanel(plate, nullptr, nullptr);
    DrawLabelCentered(FontKind::Mono, TXT_ENGAGEMENT, 640, 40, 12, 0.2f, Col(COL_DIM));
    DrawLabelCentered(FontKind::Display, TextFormat("%02d", shownEngagement), 640, 48.5f, 40, 0.0f, Col(COL_AMBER));

    const char* label = TXT_PHASE_ORDERS;
    if (phase == Phase::HostileTargeting) label = TXT_PHASE_TARGETING;
    if (phase == Phase::Incoming) label = TXT_PHASE_INCOMING;
    if (phase == Phase::StandDown) label = TXT_PHASE_OVER;
    bool hot = (phase == Phase::HostileTargeting || phase == Phase::Incoming);
    float w = 8 + 8 + MeasureLabel(FontKind::Mono, label, 12, 0.14f);
    float x = 640 - w / 2;
    Color light = hot ? Col(COL_RED, Blink(now, 0.5f) || settings.reduceMotion ? 1.0f : 0.35f) : Col(COL_GREEN);
    DrawRectangleRec({ x - 3, 97, 14, 14 }, Fade(light, 0.25f));
    DrawRectangleRec({ x, 100, 8, 8 }, light);
    DrawLabel(FontKind::Mono, label, x + 16, 98, 12, 0.14f, hot ? Col(COL_RED_SOFT) : Col(COL_TEXT));
}

// Shake and flash from a hit: keyframes at 15%, 35% and 60% of half a second.
static Vector2 HitOffset(float age) {
    if (age >= 0.5f || settings.reduceMotion) return { 0, 0 };
    float p = age / 0.5f;
    if (p < 0.15f) return { -12 * p / 0.15f, 4 * p / 0.15f };
    if (p < 0.35f) { float k = (p - 0.15f) / 0.2f; return { -12 + 21 * k, 4 - 7 * k }; }
    if (p < 0.60f) { float k = (p - 0.35f) / 0.25f; return { 9 - 13 * k, -3 + 3 * k }; }
    float k = (p - 0.6f) / 0.4f;
    return { -4 + 4 * k, 0 };
}

static void DrawShipWithEffects(bool warden, float now) {
    Texture2D sprite = warden ? assets.warden : assets.halberd;
    Vector2 home = warden ? WARDEN_POS : HALBERD_POS;
    float hitAge = warden ? wardenHitAge : playerHitAge;
    float healAge = warden ? wardenHealAge : playerHealAge;
    float deadAge = warden ? wardenDeadAge : playerDeadAge;
    bool motion = !settings.reduceMotion;

    // Gentle bobbing; the Warden bobs half a cycle later.
    float bob = motion ? -8 * Pulse(now + (warden ? 2.3f : 0.0f), 4.6f) : 0;
    Vector2 shake = HitOffset(hitAge);
    Vector2 pos = { home.x + shake.x, home.y + bob + shake.y };

    // Destroyed: flash, grow, spin a little and fade away.
    float scale = 1.0f, spin = 0.0f, alpha = 1.0f;
    if (deadAge < 1.1f) {
        float p = deadAge / 1.1f;
        scale = 1.0f + 0.5f * p * p;
        spin = 18 * p * p;
        alpha = 1.0f - p;
    } else if (deadAge < NEVER) {
        return;
    }

    unsigned int glow = warden ? COL_RED : COL_AMBER;
    DrawGlow(pos, 95, Col(glow, 0.16f * alpha));
    if (healAge < 0.8f)
        DrawGlow(pos, 120, Col(COL_GREEN, 0.45f * sinf(PI * healAge / 0.8f)));
    if (warden)
        DrawFlame({ pos.x + 58, pos.y + 10 }, 44, 16, false, now, motion);
    else
        DrawFlame({ pos.x - 58, pos.y + 17 }, 70, 22, true, now, motion);
    DrawShip(sprite, pos, 150 * scale, spin, Col(0xFFFFFFFF, alpha));

    // Bright flash: draw the ship again with additive blending, which brightens it.
    float flash = 0.0f;
    if (hitAge < 0.25f) flash = (hitAge < 0.075f) ? hitAge / 0.075f : 1.0f - (hitAge - 0.075f) / 0.175f;
    if (deadAge < 0.4f) flash = 1.0f - deadAge / 0.4f;
    if (healAge < 0.8f) flash = 0.4f * sinf(PI * healAge / 0.8f);
    if (flash > 0) {
        BeginBlendMode(BLEND_ADDITIVE);
        Color tint = (healAge < 0.8f) ? Col(COL_GREEN, flash) : Col(0xFFD8A0FF, flash);
        DrawShip(sprite, pos, 150 * scale, spin, tint);
        EndBlendMode();
    }

    // Shield bubble, popping in from half size.
    bool bubble = warden ? wardenBubble : playerBubble;
    if (bubble) {
        float age = warden ? wardenBubbleAge : playerBubbleAge;
        float pop = EaseIn(age, 0, 0.3f);
        float size = 210 * (0.5f + 0.5f * pop);
        Color tint = warden ? Col(COL_RED, 0.85f * pop) : Col(COL_AMBER, 0.85f * pop);
        Rectangle source = { 0, 0, (float)assets.shield.width, (float)assets.shield.height };
        DrawTexturePro(assets.shield, source, { home.x, home.y + bob, size, size }, { size / 2, size / 2 }, warden ? -90.0f : 90.0f, tint);
    }
}

// Beams: shoot across in 0.2 s, then fade while getting thicker. Drawn in 6 choppy steps.
static void DrawBeam(float age, bool fromWarden) {
    if (age >= 0.4f) return;
    float p = floorf(age / 0.4f * 6) / 6;
    float reach = (p < 0.5f) ? p / 0.5f : 1.0f;
    float alpha = (p < 0.5f) ? 1.0f : 1.0f - (p - 0.5f) / 0.5f;
    float thick = (p < 0.5f) ? 4 : 4 + 6 * (p - 0.5f) / 0.5f;
    float length = 560 * reach;
    float y = 330 - thick / 2;
    unsigned int color = fromWarden ? COL_RED : COL_AMBER;
    Color hot = fromWarden ? Col(0xFFE0D8FF, alpha) : Col(0xFFF6E0FF, alpha);
    float x = fromWarden ? 920 - length : 360;
    // Glow, then the core: fading in from the gun end, hottest at the tip.
    DrawRectangleRec({ x, y - 6, length, thick + 12 }, Col(color, 0.18f * alpha));
    float fadeIn = length * 0.15f;
    if (!fromWarden) {
        DrawRectangleGradientH((int)x, (int)y, (int)fadeIn, (int)thick, Col(color, 0), Col(color, alpha));
        DrawRectangleGradientH((int)(x + fadeIn), (int)y, (int)(length - fadeIn), (int)thick, Col(color, alpha), hot);
    } else {
        DrawRectangleGradientH((int)x, (int)y, (int)(length - fadeIn), (int)thick, hot, Col(color, alpha));
        DrawRectangleGradientH((int)(x + length - fadeIn), (int)y, (int)fadeIn, (int)thick, Col(color, alpha), Col(color, 0));
    }
}

static void DrawFloats() {
    int smallCount[2] = { 0, 0 };
    for (const FloatingText& f : floats) {
        float p = f.age / 1.2f;
        float alpha = (p < 0.15f) ? p / 0.15f : 1.0f - (p - 0.15f) / 0.85f;
        float rise = 10 - 80 * p;  // from 10 px below the start to 70 px above
        float x = f.onWarden ? WARDEN_POS.x : HALBERD_POS.x;
        float y = 226 + rise;
        if (f.small) {
            y += 46 + 18 * smallCount[f.onWarden ? 1 : 0]++;
            DrawLabelCentered(FontKind::Mono, f.text.c_str(), x + 1, y + 2, 16, 0.2f, Col(0x000000FF, alpha));
            DrawLabelCentered(FontKind::Mono, f.text.c_str(), x, y, 16, 0.2f, Col(f.color, alpha));
        } else {
            DrawLabelCentered(FontKind::Display, f.text.c_str(), x, y + 2, 40, 0.0f, Col(0x000000FF, alpha * 0.8f));
            DrawLabelCentered(FontKind::Display, f.text.c_str(), x, y, 40, 0.0f, Col(f.color, alpha));
        }
    }
}

static void DrawParticles() {
    for (const Particle& p : particles) {
        float alpha = 1.0f - p.age / p.life;
        DrawCircleV(p.pos, p.size * (0.5f + alpha), Col(p.color, alpha));
    }
    // A bright burst right where the ship blew up.
    float deadAge = (playerDeadAge < wardenDeadAge) ? playerDeadAge : wardenDeadAge;
    if (deadAge < 0.6f) {
        Vector2 at = (playerDeadAge < wardenDeadAge) ? HALBERD_POS : WARDEN_POS;
        float k = deadAge / 0.6f;
        DrawGlow(at, 60 + 200 * k, Col(0xFFE6B4FF, 0.8f * (1 - k)));
    }
}

static void DrawFeed(float now) {
    Rectangle panel = { 840, 540, 416, 156 };
    DrawPanel(panel, TXT_FEED, nullptr, PanelTone::Amber, 43);  // taller header: it holds the mute button
    bool soundOn = settings.soundEffects || settings.music;
    DrawIcon(soundOn ? Icon::Speaker : Icon::SpeakerOff, { MUTE_RECT.x + 17, MUTE_RECT.y + 13 }, 18, Col(COL_AMBER));

    for (int i = 0; i < (int)feed.size(); i++) {
        const FeedLine& line = feed[i];
        float y = 594.5f + i * 20.15f;
        const char* stamp = TextFormat("[%02d]", line.engagement);
        DrawLabel(FontKind::Mono, stamp, 858, y, 13, 0.0f, Col(COL_DIM));
        float textX = 858 + MeasureLabel(FontKind::Mono, stamp, 13, 0.0f) + 8;
        // The newest line types itself out, in 14 steps over 0.35 seconds.
        int length = (int)line.text.size();
        int shown = length;
        if (line.age < 0.35f)
            shown = (int)(floorf(line.age / 0.35f * 14) / 14 * length);
        std::string visible = line.text.substr(0, shown);
        DrawLabel(FontKind::Mono, visible.c_str(), textX, y, 13, 0.0f, Col(line.color));
        if (i == 0 && Blink(now, 1.0f)) {
            float w = MeasureLabel(FontKind::Mono, visible.c_str(), 13, 0.0f);
            DrawLabel(FontKind::Mono, "_", textX + w, y, 13, 0.0f, Col(COL_AMBER));
        }
    }
}

static void DrawConfirm(float now) {
    DrawRectangle(0, 0, VIRTUAL_W, VIRTUAL_H, { 8, 6, 4, 199 });
    // The dialog pops in: it grows from 85% with a little overshoot.
    float p = EaseIn(confirmAge, 0, 0.35f);
    float pop = 0.85f + 0.15f * p + 0.04f * sinf(PI * p);
    Rectangle panel = { 380, 232, 520, 256 };
    Camera2D camera = { };
    camera.target = { 640, 360 };
    camera.offset = { 640, 360 };
    camera.zoom = pop;
    BeginMode2D(camera);
    SetUiFade(p);
    DrawPanel(panel, TXT_CONFIRM_HEADER, TXT_CONFIRM_RIGHT);
    DrawLabelCentered(FontKind::Display, TXT_CONFIRM_TITLE, 640, panel.y + 60, 44, 0.08f, Col(COL_AMBER));
    DrawLabelCentered(FontKind::Display, TXT_CONFIRM_TEXT, 640, panel.y + 114, 18, 0.0f, Col(COL_DIM));
    stayButton.rect.y = panel.y + 162;
    leaveButton.rect.y = panel.y + 162;
    DrawButton(stayButton, true, now);
    DrawButton(leaveButton, true, now);
    SetUiFade(1);
    EndMode2D();
}

void DrawBattle() {
    float now = (float)GetTime();
    bool motion = !settings.reduceMotion;
    bool hot = (phase == Phase::HostileTargeting || phase == Phase::Incoming);

    ClearBackground(GetColor(COL_BG));

    // Screen shake moves everything except the CRT overlay.
    Camera2D camera = { };
    camera.zoom = 1.0f;
    if (shakeAge < 0.5f) {
        float k = 1.0f - shakeAge / 0.5f;
        camera.offset = { sinf(shakeAge * 80) * shakeStrength * k, cosf(shakeAge * 63) * shakeStrength * 0.5f * k };
    }
    BeginMode2D(camera);

    DrawBackLayers(now, settings.reduceMotion);
    DrawPlanet({ 10, 740 }, 310, now, settings.reduceMotion);
    DrawFrontLayers(now, settings.reduceMotion);
    float wash = motion ? 1.0f - 0.65f * Pulse(now, 0.9f) : 0.7f;
    DrawAlertWash(alertLevel * wash);
    DrawHazardTape(0, 8, hot, 0.9f, now, motion);

    DrawShipCard(false, now);
    DrawPlate(now);
    DrawShipCard(true, now);

    DrawShipWithEffects(false, now);
    DrawShipWithEffects(true, now);
    // Red lock-on brackets around the Warden while it is the player's move.
    if (phase == Phase::AwaitingOrders && !IsBusy() && !confirmOpen)
        DrawLockOn({ 920, 250, 180, 160 }, now, motion);
    DrawBeam(playerBeamAge, false);
    DrawBeam(wardenBeamAge, true);
    DrawFloats();
    DrawParticles();

    DrawPanel({ 24, 540, 800, 156 }, nullptr, nullptr);
    bool ready = !IsBusy() && !confirmOpen;
    DrawButton(fireButton, ready, now);
    DrawButton(shieldsButton, ready, now);
    DrawButton(repairButton, ready, now);
    DrawButton(retreatButton, ready, now);
    DrawFeed(now);

    EndMode2D();
    DrawScreenOverlays(now, settings.reduceMotion);
    if (confirmOpen)
        DrawConfirm(now);
}
