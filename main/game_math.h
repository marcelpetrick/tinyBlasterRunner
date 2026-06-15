#pragma once

#include <stdbool.h>
#include <stdint.h>

uint32_t rng_next(uint32_t *s);
int32_t  rng_range(uint32_t *s, int32_t lo, int32_t hi);
bool     aabb(float ax, float ay, float aw, float ah,
              float bx, float by, float bw, float bh);
