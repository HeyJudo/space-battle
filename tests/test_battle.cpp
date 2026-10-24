// test_battle.cpp - checks every rule in SPEC.md "Game rules" with plain asserts.
// Run it with the VS Code "test" task (or: make test). If a rule breaks,
// assert stops the program and prints the file and line of the failed check.
#include <cassert>
#include <cstdio>
#include "battle.h"
#include "constants.h"

// Peeks at what the Warden will do next without using up the real dice:
// it rolls on a copy of the Rng. 1 = attack, 2 = shields, 3 = repair.
static int PeekWardenAction(Rng rng) {
    return RollRange(rng, 1, 3);
}

// The raw (not yet halved) damage the Warden will roll if it attacks.
static int PeekWardenDamage(Rng rng) {
    RollRange(rng, 1, 3);
    return RollRange(rng, FIRE_MIN, FIRE_MAX);
}

// Finds a seed where the Warden's next move is the wanted action.
static Rng RngWhereWardenDoes(int action) {
    for (unsigned int seed = 1; ; seed++) {
        Rng rng = MakeRng(seed);
        if (PeekWardenAction(rng) == action)
            return rng;
    }
}

static bool HasEvent(const std::vector<Event>& events, EventType type) {
    for (const Event& e : events)
        if (e.type == type)
            return true;
    return false;
}

static int AmountOf(const std::vector<Event>& events, EventType type) {
    for (const Event& e : events)
        if (e.type == type)
            return e.amount;
    assert(false && "event not found");
    return 0;
}

static void TestStart() {
    Battle b = NewBattle();
    assert(b.playerHull == 100 && b.wardenHull == 100);
    assert(b.engagement == 1);
    assert(b.result == Result::Ongoing);
}

static void TestDamageRange() {
    for (unsigned int seed = 1; seed <= 2000; seed++) {
        Rng rng = MakeRng(seed);
        Battle b = NewBattle();
        std::vector<Event> events = PlayerMove(b, Command::Fire, rng);
        int damage = AmountOf(events, EventType::PlayerFired);
        assert(damage >= 10 && damage <= 25);
        assert(b.wardenHull == 100 - damage);
    }
}

static void TestHullNeverBelowZero() {
    for (unsigned int seed = 1; seed <= 500; seed++) {
        Rng rng = MakeRng(seed);
        Battle b = NewBattle();
        b.wardenHull = 3;
        PlayerMove(b, Command::Fire, rng);
        assert(b.wardenHull == 0);
    }
    Rng rng = RngWhereWardenDoes(1);
    Battle b = NewBattle();
    b.playerHull = 2;
    WardenMove(b, rng);
    assert(b.playerHull == 0);
}

static void TestRepairCapsAndReportsGain() {
    for (unsigned int seed = 1; seed <= 500; seed++) {
        Rng rng = MakeRng(seed);
        Battle b = NewBattle();
        b.playerHull = 95;
        std::vector<Event> events = PlayerMove(b, Command::Repair, rng);
        assert(b.playerHull == 100);
        assert(AmountOf(events, EventType::PlayerRepaired) == 5);  // not the raw roll
    }
    // Far from the cap, the full roll counts.
    Rng rng = MakeRng(7);
    Battle b = NewBattle();
    b.playerHull = 40;
    std::vector<Event> events = PlayerMove(b, Command::Repair, rng);
    int gained = AmountOf(events, EventType::PlayerRepaired);
    assert(gained >= 15 && gained <= 30);
    assert(b.playerHull == 40 + gained);

    // The Warden's repair follows the same cap.
    Rng wardenRng = RngWhereWardenDoes(3);
    Battle w = NewBattle();
    w.wardenHull = 90;
    std::vector<Event> wardenEvents = WardenMove(w, wardenRng);
    assert(w.wardenHull == 100);
    assert(AmountOf(wardenEvents, EventType::WardenRepaired) == 10);
}

static void TestPlayerShieldsHalveThisRoundOnly() {
    Rng rng = RngWhereWardenDoes(1);
    int raw = PeekWardenDamage(rng);

    Battle b = NewBattle();
    PlayerMove(b, Command::Shields, rng);  // Shields rolls no dice
    assert(b.playerShields);
    std::vector<Event> events = WardenMove(b, rng);
    assert(HasEvent(events, EventType::PlayerShieldsHeld));
    assert(AmountOf(events, EventType::WardenFired) == raw / 2);  // integer division
    assert(raw / 2 <= 12);
    assert(b.playerHull == 100 - raw / 2);

    // Next round: shields are gone, so a new hit is not halved.
    assert(!b.playerShields);
    Battle next = b;
    Rng attackRng = RngWhereWardenDoes(1);
    int raw2 = PeekWardenDamage(attackRng);
    int hullBefore = next.playerHull;
    std::vector<Event> events2 = WardenMove(next, attackRng);
    assert(!HasEvent(events2, EventType::PlayerShieldsHeld));
    assert(next.playerHull == hullBefore - raw2);

    // Picking another command after Shields also drops them.
    Battle c = NewBattle();
    Rng r = MakeRng(3);
    PlayerMove(c, Command::Shields, r);
    PlayerMove(c, Command::Repair, r);
    assert(!c.playerShields);
}

static void TestWardenShieldsHalveNextFireOnly() {
    Rng rng = RngWhereWardenDoes(2);
    Battle b = NewBattle();
    std::vector<Event> events = WardenMove(b, rng);
    assert(HasEvent(events, EventType::WardenRaisedShields));
    assert(b.wardenShields);

    // The next Fire is halved with integer division.
    Rng peek = rng;
    int raw = RollRange(peek, FIRE_MIN, FIRE_MAX);
    std::vector<Event> fire = PlayerMove(b, Command::Fire, rng);
    assert(HasEvent(fire, EventType::WardenShieldsHeld));
    assert(AmountOf(fire, EventType::PlayerFired) == raw / 2);
    assert(b.wardenHull == 100 - raw / 2);
    assert(!b.wardenShields);

    // The Fire after that is full strength again.
    Rng peek2 = rng;
    int raw2 = RollRange(peek2, FIRE_MIN, FIRE_MAX);
    int hullBefore = b.wardenHull;
    std::vector<Event> fire2 = PlayerMove(b, Command::Fire, rng);
    assert(!HasEvent(fire2, EventType::WardenShieldsHeld));
    assert(b.wardenHull == hullBefore - raw2);

    // Any other command clears the Warden's shields without using them.
    Battle c = NewBattle();
    c.wardenShields = true;
    Rng r = MakeRng(11);
    PlayerMove(c, Command::Shields, r);
    assert(!c.wardenShields);
}

static void TestWardenDoesNotActAfterDefeat() {
    for (unsigned int seed = 1; seed <= 200; seed++) {
        Rng rng = MakeRng(seed);
        Battle b = NewBattle();
        b.wardenHull = 5;
        std::vector<Event> events = PlayerMove(b, Command::Fire, rng);
        assert(HasEvent(events, EventType::Victory));
        assert(b.result == Result::Victory);

        Battle before = b;
        std::vector<Event> warden = WardenMove(b, rng);
        assert(warden.empty());
        assert(b.playerHull == before.playerHull && b.wardenHull == 0);
        assert(b.engagement == before.engagement);
    }
}

static void TestDefeat() {
    Rng rng = RngWhereWardenDoes(1);
    Battle b = NewBattle();
    b.playerHull = 5;
    std::vector<Event> events = WardenMove(b, rng);
    assert(HasEvent(events, EventType::Defeat));
    assert(b.result == Result::Defeat);
    // Nothing moves once the battle is over.
    assert(PlayerMove(b, Command::Fire, rng).empty());
}

static void TestRetreat() {
    Rng rng = MakeRng(5);
    Battle b = NewBattle();
    std::vector<Event> events = PlayerMove(b, Command::Retreat, rng);
    assert(HasEvent(events, EventType::PlayerRetreated));
    assert(b.result == Result::Withdrawn);
    assert(WardenMove(b, rng).empty());
}

// A scripted battle: the debrief stats must add up.
static void TestStatsAddUp() {
    Rng rng = MakeRng(2024);
    Battle b = NewBattle();
    int dealt = 0;
    int rounds = 0;
    Command script[] = {Command::Fire, Command::Shields, Command::Fire, Command::Repair, Command::Fire};
    for (Command c : script) {
        std::vector<Event> events = PlayerMove(b, c, rng);
        for (const Event& e : events)
            if (e.type == EventType::PlayerFired)
                dealt += e.amount;
        WardenMove(b, rng);
        rounds++;
        if (b.result != Result::Ongoing)
            break;
    }
    assert(b.damageDealt == dealt);
    if (b.result == Result::Ongoing)
        assert(b.engagement == rounds + 1);  // the next engagement is waiting
}

// Port of the mockups' headless play-test: many random games, checking the
// same invariants on every move.
static void TestRandomGames() {
    int victories = 0;
    for (unsigned int seed = 1; seed <= 5000; seed++) {
        Rng rng = MakeRng(seed);
        Battle b = NewBattle();
        int dealt = 0;
        int moves = 0;
        while (b.result == Result::Ongoing) {
            // A random player that leans on Fire. With equal odds both ships
            // out-repair each other and a game can drift on for thousands of rounds.
            int pick = RollRange(rng, 0, 9);
            Command c = Command::Fire;
            if (pick >= 6) c = Command::Shields;
            if (pick >= 8) c = Command::Repair;
            int missing = HULL_MAX - b.playerHull;
            int engagement = b.engagement;
            std::vector<Event> events = PlayerMove(b, c, rng);
            for (const Event& e : events) {
                if (e.type == EventType::PlayerFired) {
                    dealt += e.amount;
                    assert(e.amount >= FIRE_MIN / 2 && e.amount <= FIRE_MAX);
                }
                if (e.type == EventType::PlayerRepaired)
                    assert(e.amount >= 0 && e.amount <= missing);  // never shows more than was missing
            }
            bool shielded = b.playerShields;
            int wardenMissing = HULL_MAX - b.wardenHull;
            std::vector<Event> warden = WardenMove(b, rng);
            for (const Event& e : warden) {
                if (e.type == EventType::WardenFired && shielded)
                    assert(e.amount <= 12);  // Shields let at most 12 through
                if (e.type == EventType::WardenRepaired)
                    assert(e.amount >= 0 && e.amount <= wardenMissing);
            }
            assert(b.playerHull >= 0 && b.playerHull <= HULL_MAX);
            assert(b.wardenHull >= 0 && b.wardenHull <= HULL_MAX);
            if (b.result == Result::Ongoing)
                assert(b.engagement == engagement + 1);
            moves++;
            assert(moves < 1000);  // every game ends
        }
        assert(b.result == Result::Victory || b.result == Result::Defeat);
        assert(b.damageDealt == dealt);
        if (b.result == Result::Victory)
            victories++;
    }
    printf("  random play: %d of 5000 games won\n", victories);
}

int main() {
    TestStart();
    TestDamageRange();
    TestHullNeverBelowZero();
    TestRepairCapsAndReportsGain();
    TestPlayerShieldsHalveThisRoundOnly();
    TestWardenShieldsHalveNextFireOnly();
    TestWardenDoesNotActAfterDefeat();
    TestDefeat();
    TestRetreat();
    TestStatsAddUp();
    TestRandomGames();
    printf("All battle tests passed.\n");
    return 0;
}
