// battle.h - the Space Battle rules. No drawing and no raylib here.
// The screens call PlayerMove and WardenMove, then turn the returned events
// into animations, sounds and tactical feed lines.
#pragma once
#include <vector>
#include "rng.h"

// The four player commands (her original code: Attack, Defend, Heal, Quit).
enum class Command { Fire, Shields, Repair, Retreat };

enum class Result { Ongoing, Victory, Defeat, Withdrawn };

// Something that happened during a move. "amount" is only used by some types.
enum class EventType {
    WardenShieldsHeld,    // the Warden's shields halved our Fire
    PlayerFired,          // amount = damage dealt to the Warden
    PlayerRaisedShields,
    PlayerRepaired,       // amount = hull actually gained
    PlayerRetreated,
    PlayerShieldsHeld,    // our shields halved the Warden's hit
    WardenFired,          // amount = damage dealt to the Halberd
    WardenRaisedShields,
    WardenRepaired,       // amount = hull actually gained
    Victory,
    Defeat
};

struct Event {
    EventType type;
    int amount;
};

// Everything the rules need to remember between moves.
struct Battle {
    int playerHull;       // the Halberd
    int wardenHull;       // the Warden
    int engagement;       // round number, starts at 1
    bool playerShields;   // halves the Warden's hit this round
    bool wardenShields;   // halves the player's next Fire
    Result result;
    int damageDealt;      // debrief: total damage the player dealt
};

Battle NewBattle();

// The player's command. Ends the battle on Victory or Retreat.
std::vector<Event> PlayerMove(Battle& battle, Command command, Rng& rng);

// The Warden's reply. Does nothing if the battle is already over.
// When the battle goes on, it also starts the next engagement.
std::vector<Event> WardenMove(Battle& battle, Rng& rng);
