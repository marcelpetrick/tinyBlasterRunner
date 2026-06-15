/** @file graphics.h
 *  @brief High-level 2-D drawing API built on top of st7789_display.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "st7789_display.h"

/** Context holding a reference to the display and its dimensions. */
typedef struct {
    st7789_display_t* display;
    uint16_t screen_width;
    uint16_t screen_height;
} graphics_t;

/** Bind a graphics context to a display handle. */
esp_err_t graphics_init(graphics_t* graphics, st7789_display_t* display);
/** Clear the screen to a solid RGB565 colour. */
esp_err_t graphics_clear(graphics_t* graphics, uint16_t color565);
/** Fill a rectangle with a solid colour. */
esp_err_t graphics_fill_rect(graphics_t* graphics, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                             uint16_t color565);
/** Draw an unfilled rectangle outline. */
esp_err_t graphics_draw_rect(graphics_t* graphics, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                             uint16_t color565);
/** Blit an RGB565 bitmap to the screen. */
esp_err_t graphics_draw_bitmap(graphics_t* graphics, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                               const uint16_t* pixels, size_t pixel_count);
/** Render a string using the built-in 5×7 font. @p scale is pixel zoom factor. */
esp_err_t graphics_draw_text(graphics_t* graphics, uint16_t x, uint16_t y, const char* text, uint16_t color565,
                             uint16_t scale);
/** Draw a sprite with per-pixel transparency using @p transparent_key as the colour key. */
esp_err_t graphics_draw_sprite(graphics_t* graphics, int32_t x, int32_t y, uint16_t width, uint16_t height,
                               const uint16_t* pixels, uint16_t transparent_key);
/** Draw a sprite at integer pixel scale with colour-key transparency. */
esp_err_t graphics_draw_sprite_scaled(graphics_t* graphics, int32_t x, int32_t y, uint16_t width, uint16_t height,
                                      const uint16_t* pixels, uint16_t transparent_key, uint8_t scale);
