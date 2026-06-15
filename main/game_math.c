#include "game_math.h"

uint32_t rng_next(uint32_t *s)
{
    *s = (*s * 1664525u) + 1013904223u;
    return *s;
}

/* Returns a value in [lo, hi] INCLUSIVE. hi must be > lo and < INT32_MAX. */
int32_t rng_range(uint32_t *s, int32_t lo, int32_t hi)
{
    if (hi <= lo) return lo;
    uint32_t range = (uint32_t)(hi - lo) + 1u;
    return lo + (int32_t)(rng_next(s) % range);
}

bool aabb(float ax, float ay, float aw, float ah,
          float bx, float by, float bw, float bh)
{
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}
