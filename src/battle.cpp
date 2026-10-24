// battle.cpp - the Space Battle rules, ported from reference/original_main.cpp
// plus the approved changes in SPEC.md (Warden shields, real repair amount,
// Retreat as a result).
#include "battle.h"
#include "constants.h"

Battle NewBattle() {
    Battle battle;
    battle.playerHull = HULL_MAX;
    battle.wardenHull = HULL_MAX;
    battle.engagement = 1;
    battle.playerShields = false;
    battle.wardenShields = false;
    battle.result = Result::Ongoing;
    battle.damageDealt = 0;
    return battle;
}

// Adds a repair roll to a hull, capped at HULL_MAX, and returns what was really gained.
// At 95 hull a roll of 22 only gains 5.
static int Repair(int& hull, Rng& rng) {
    int before = hull;
    hull += RollRange(rng, REPAIR_MIN, REPAIR_MAX);
    if (hull > HULL_MAX)
        hull = HULL_MAX;
    return hull - before;
}

std::vector<Event> PlayerMove(Battle& battle, Command command, Rng& rng) {
    std::vector<Event> events;
    if (battle.result != Result::Ongoing)
        return events;

    // Our shields only last for this round's Warden move.
    battle.playerShields = false;

    // The Warden's shields wait for our next Fire, but any other command clears them too.
    bool wardenWasShielded = battle.wardenShields;
    battle.wardenShields = false;

    switch (command) {
        case Command::Fire: {
            int damage = RollRange(rng, FIRE_MIN, FIRE_MAX);
            if (wardenWasShielded) {
                damage /= 2;  // integer division, like the original Defend
                events.push_back({EventType::WardenShieldsHeld, 0});
            }
            battle.wardenHull -= damage;
            if (battle.wardenHull < 0)
                battle.wardenHull = 0;
            battle.damageDealt += damage;
            events.push_back({EventType::PlayerFired, damage});
            break;
        }
        case Command::Shields:
            battle.playerShields = true;
            events.push_back({EventType::PlayerRaisedShields, 0});
            break;
        case Command::Repair: {
            int gained = Repair(battle.playerHull, rng);
            events.push_back({EventType::PlayerRepaired, gained});
            break;
        }
        case Command::Retreat:
            battle.result = Result::Withdrawn;
            events.push_back({EventType::PlayerRetreated, 0});
            return events;
    }

    // The Warden is destroyed: the battle ends before it can act.
    if (battle.wardenHull <= 0) {
        battle.result = Result::Victory;
        events.push_back({EventType::Victory, 0});
    }
    return events;
}

std::vector<Event> WardenMove(Battle& battle, Rng& rng) {
    std::vector<Event> events;
    if (battle.result != Result::Ongoing)
        return events;

    int action = RollRange(rng, 1, 3);  // rand() % 3 + 1

    if (action == 1) {
        // Attack, halved if we raised shields this round
        int damage = RollRange(rng, FIRE_MIN, FIRE_MAX);
        if (battle.playerShields) {
            damage /= 2;
            events.push_back({EventType::PlayerShieldsHeld, 0});
        }
        battle.playerHull -= damage;
        if (battle.playerHull < 0)
            battle.playerHull = 0;
        events.push_back({EventType::WardenFired, damage});
    } else if (action == 2) {
        // Shields: halves our next Fire
        battle.wardenShields = true;
        events.push_back({EventType::WardenRaisedShields, 0});
    } else {
        // Repair
        int gained = Repair(battle.wardenHull, rng);
        events.push_back({EventType::WardenRepaired, gained});
    }

    if (battle.playerHull <= 0) {
        battle.result = Result::Defeat;
        events.push_back({EventType::Defeat, 0});
    } else {
        // The round is over: shields drop and the next engagement starts.
        battle.playerShields = false;
        battle.engagement += 1;
    }
    return events;
}
