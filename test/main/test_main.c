#include <stdlib.h>
#include "unity.h"
#include "game_math.h"

void setUp(void) {}
void tearDown(void) {}

// ---------------------------------------------------------------
// rng_range
// ---------------------------------------------------------------

void test_rng_range_lo_equals_hi(void)
{
    uint32_t s = 42;
    TEST_ASSERT_EQUAL_INT32(5, rng_range(&s, 5, 5));
}

void test_rng_range_always_in_bounds(void)
{
    uint32_t s = 0xDEADBEEFu;
    for (int i = 0; i < 2000; i++) {
        int32_t v = rng_range(&s, -10, 10);
        TEST_ASSERT_TRUE(v >= -10);
        TEST_ASSERT_TRUE(v <= 10);
    }
}

void test_rng_range_covers_endpoints(void)
{
    uint32_t s = 1;
    bool saw_lo = false, saw_hi = false;
    for (int i = 0; i < 5000 && !(saw_lo && saw_hi); i++) {
        int32_t v = rng_range(&s, 0, 9);
        if (v == 0) saw_lo = true;
        if (v == 9) saw_hi = true;
    }
    TEST_ASSERT_TRUE(saw_lo);
    TEST_ASSERT_TRUE(saw_hi);
}

// ---------------------------------------------------------------
// aabb
// ---------------------------------------------------------------

void test_aabb_clear_overlap(void)
{
    TEST_ASSERT_TRUE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 5.0f, 5.0f, 10.0f, 10.0f));
}

void test_aabb_partial_overlap_x(void)
{
    TEST_ASSERT_TRUE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 9.0f, 0.0f, 10.0f, 10.0f));
}

void test_aabb_touching_x_not_overlap(void)
{
    /* a ends at x=10, b starts at x=10: strict less-than means no overlap */
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, 10.0f, 10.0f));
}

void test_aabb_touching_y_not_overlap(void)
{
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 10.0f));
}

void test_aabb_separated_x(void)
{
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 20.0f, 0.0f, 10.0f, 10.0f));
}

void test_aabb_separated_y(void)
{
    TEST_ASSERT_FALSE(aabb(0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 20.0f, 10.0f, 10.0f));
}

void test_aabb_b_inside_a(void)
{
    TEST_ASSERT_TRUE(aabb(0.0f, 0.0f, 20.0f, 20.0f, 5.0f, 5.0f, 5.0f, 5.0f));
}

// ---------------------------------------------------------------
// Runner
// ---------------------------------------------------------------

int main(void)
{
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

    return UNITY_END();
}
