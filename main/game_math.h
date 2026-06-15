#pragma once

#include <stdbool.h>
#include <stdint.h>

// ---------------------------------------------------------------
// RNG
// ---------------------------------------------------------------
uint32_t rng_next(uint32_t* s);
int32_t rng_range(uint32_t* s, int32_t lo, int32_t hi);

// ---------------------------------------------------------------
// Collision
// ---------------------------------------------------------------
bool aabb(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);

// ---------------------------------------------------------------
// Difficulty tuning constants (shared with test suite)
// ---------------------------------------------------------------
#define DIFF_INTERVAL_MS 30000u
#define SCROLL_SPEED_START 2.0f
#define SCROLL_SPEED_MAX 4.0f
#define SCROLL_SPEED_STEP 0.10f
#define SPAWN_COIN_BASE 2000u
#define SPAWN_ENEMY_BASE 3000u
#define SPAWN_OBS_BASE 3500u
#define SPAWN_COIN_MIN 800u
#define SPAWN_ENEMY_MIN 1200u
#define SPAWN_OBS_MIN 1500u

// ---------------------------------------------------------------
// Difficulty scaling
// ---------------------------------------------------------------
/** Return scroll speed (px/frame) for the given total game time. */
float difficulty_scroll_speed(uint32_t game_ms);
/** Return spawn interval (ms) for a given base interval at the current scroll speed. */
uint32_t difficulty_spawn_interval(uint32_t base_ms, float scroll_speed, uint32_t min_ms);
