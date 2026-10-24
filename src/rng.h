// rng.h - a tiny seedable random number generator.
// The game seeds it from the clock. The tests seed it with fixed numbers,
// so every test run rolls exactly the same dice.
#pragma once

struct Rng {
    unsigned int state;
};

inline Rng MakeRng(unsigned int seed) {
    Rng rng;
    rng.state = (seed == 0) ? 1 : seed;  // xorshift gets stuck on 0
    return rng;
}

// Next raw number, using the "xorshift" recipe: three shifts mixed with XOR.
inline unsigned int NextRandom(Rng& rng) {
    unsigned int x = rng.state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng.state = x;
    return x;
}

// A number from low to high, both included. RollRange(rng, 10, 25) works the
// same way as the original rand() % 16 + 10.
inline int RollRange(Rng& rng, int low, int high) {
    int count = high - low + 1;
    return (int)(NextRandom(rng) % count) + low;
}
