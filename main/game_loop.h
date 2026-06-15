/** @file game_loop.h
 *  @brief Fixed-rate game loop with input polling, update, and render callbacks.
 *
 *  Drives the game at @p target_fps using FreeRTOS vTaskDelay. Passes the
 *  actual elapsed delta-time (dt_ms) to update and render so physics stays
 *  correct even if a frame runs long.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <stdbool.h>

#include "button_input.h"
#include "esp_err.h"

/** Callback invoked once per frame with all button events collected since the last frame. */
typedef esp_err_t (*game_loop_input_cb_t)(const button_event_t* events, size_t event_count, void* ctx);
typedef esp_err_t (*game_loop_step_cb_t)(uint32_t frame, uint32_t dt_ms, void* ctx);

typedef struct {
    uint32_t target_fps;
    uint32_t stats_interval_ms;
    void* ctx;
    game_loop_input_cb_t on_input;
    game_loop_step_cb_t on_update;
    game_loop_step_cb_t on_render;
    volatile bool* stop_requested; /* set to true from any context to exit the loop cleanly */
} game_loop_config_t;

esp_err_t game_loop_run(button_input_t* buttons, const game_loop_config_t* config);
