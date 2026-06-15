#include <stdlib.h>
#include "unity.h"
#include "game_math.h"

void setUp(void) {
}
void tearDown(void) {
}

// ---------------------------------------------------------------
// rng_range
// ---------------------------------------------------------------

void test_rng_range_lo_equals_hi(void) {
    uint32_t s = 42;
    TEST_ASSERT_EQUAL_INT32(5, rng_range(&s, 5, 5));
}

void test_rng_range_always_in_bounds(void) {
    uint32_t s = 0xDEADBEEFu;
    for (int i = 0; i < 2000; i++) {
        int32_t v = rng_range(&s, -10, 10);
        TEST_ASSERT_TRUE(v >= -10);
        TEST_ASSERT_TRUE(v <= 10);
    }
}

void test_rng_range_covers_endpoints(void) {
    uint32_t s = 1;
    bool saw_lo = false, saw_hi = false;
    for (int i = 0; i < 5000 && !(saw_lo && saw_hi); i++) {
        int32_t v = rng_range(&s, 0, 9);
        if (v == 0)
            saw_lo = true;
        if (v == 9)
            saw_hi = true;
    }
    TEST_ASSERT_TRUE(saw_lo);
    TEST_ASSERT_TRUE(saw_hi);
}

// ---------------------------------------------------------------
// aabb
// ---------------------------------------------------------------

void test_aabb_clear_overlap(void) {
    TEST_ASSERT_TRUE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 5.0f, 5.0f, 10.0f, 10.0f));
}

void test_aabb_partial_overlap_x(void) {
    TEST_ASSERT_TRUE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 9.0f, 0.0f, 10.0f, 10.0f));
}

void test_aabb_touching_x_not_overlap(void) {
    /* a ends at x=10, b starts at x=10: strict less-than means no overlap */
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 10.0f, 10.0f));
}

void test_aabb_touching_y_not_overlap(void) {
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 10.0f));
}

void test_aabb_separated_x(void) {
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 20.0f, 0.0f, 10.0f, 10.0f));
}

void test_aabb_separated_y(void) {
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 20.0f, 10.0f, 10.0f));
}

void test_aabb_b_inside_a(void) {
    TEST_ASSERT_TRUE(aabb(0.0f, 0.0f, 20.0f, 20.0f, 5.0f, 5.0f, 5.0f, 5.0f));
}

// ---------------------------------------------------------------
// difficulty_scroll_speed
// ---------------------------------------------------------------

void test_diff_speed_at_start(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, SCROLL_SPEED_START, difficulty_scroll_speed(0));
}

void test_diff_speed_increases_after_one_phase(void) {
    float s0 = difficulty_scroll_speed(0);
    float s1 = difficulty_scroll_speed(DIFF_INTERVAL_MS);
    TEST_ASSERT_TRUE(s1 > s0);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, s0 + SCROLL_SPEED_STEP, s1);
}

void test_diff_speed_clamped_at_max(void) {
    /* Far enough in time to exceed the cap. */
    float s = difficulty_scroll_speed(DIFF_INTERVAL_MS * 1000u);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, SCROLL_SPEED_MAX, s);
}

void test_diff_speed_never_exceeds_max(void) {
    for (uint32_t ms = 0; ms < DIFF_INTERVAL_MS * 50u; ms += DIFF_INTERVAL_MS / 4u) {
        TEST_ASSERT_TRUE(difficulty_scroll_speed(ms) <= SCROLL_SPEED_MAX + 0.001f);
    }
}

// ---------------------------------------------------------------
// difficulty_spawn_interval
// ---------------------------------------------------------------

void test_spawn_interval_at_base_speed(void) {
    uint32_t iv = difficulty_spawn_interval(SPAWN_ENEMY_BASE, SCROLL_SPEED_START, SPAWN_ENEMY_MIN);
    TEST_ASSERT_EQUAL_UINT32(SPAWN_ENEMY_BASE, iv);
}

void test_spawn_interval_decreases_with_higher_speed(void) {
    uint32_t iv_slow = difficulty_spawn_interval(SPAWN_ENEMY_BASE, SCROLL_SPEED_START, 0u);
    uint32_t iv_fast = difficulty_spawn_interval(SPAWN_ENEMY_BASE, SCROLL_SPEED_MAX, 0u);
    TEST_ASSERT_TRUE(iv_fast < iv_slow);
}

void test_spawn_interval_respects_minimum(void) {
    uint32_t iv = difficulty_spawn_interval(SPAWN_ENEMY_BASE, SCROLL_SPEED_MAX, SPAWN_ENEMY_MIN);
    TEST_ASSERT_TRUE(iv >= SPAWN_ENEMY_MIN);
}

void test_spawn_interval_non_zero_at_max_speed(void) {
    uint32_t iv = difficulty_spawn_interval(SPAWN_COIN_BASE, SCROLL_SPEED_MAX, 0u);
    TEST_ASSERT_TRUE(iv > 0u);
}

// ---------------------------------------------------------------
// Runner
// ---------------------------------------------------------------

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_rng_range_lo_equals_hi);
    RUN_TEST(test_rng_range_always_in_bounds);
    RUN_TEST(test_rng_range_covers_endpoints);

    RUN_TEST(test_aabb_clear_overlap);
    RUN_TEST(test_aabb_partial_overlap_x);
    RUN_TEST(test_aabb_touching_x_not_overlap);
    RUN_TEST(test_aabb_touching_y_not_overlap);
    RUN_TEST(test_aabb_separated_x);
    RUN_TEST(test_aabb_separated_y);
    RUN_TEST(test_aabb_b_inside_a);

    RUN_TEST(test_diff_speed_at_start);
    RUN_TEST(test_diff_speed_increases_after_one_phase);
    RUN_TEST(test_diff_speed_clamped_at_max);
    RUN_TEST(test_diff_speed_never_exceeds_max);

    RUN_TEST(test_spawn_interval_at_base_speed);
    RUN_TEST(test_spawn_interval_decreases_with_higher_speed);
    RUN_TEST(test_spawn_interval_respects_minimum);
    RUN_TEST(test_spawn_interval_non_zero_at_max_speed);

    return UNITY_END();
}
