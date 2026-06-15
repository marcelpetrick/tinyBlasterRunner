/** @file st7789_display.h
 *  @brief ST7789 135x240 SPI display driver for the LilyGo T-Display ESP32.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

/** Opaque display handle. Allocated by st7789_display_init(). */
typedef struct st7789_display st7789_display_t;

/** Initialise SPI bus, configure the ST7789 controller and allocate the handle. */
esp_err_t st7789_display_init(st7789_display_t** display_out);
/** Free all resources associated with the display handle. */
void st7789_display_destroy(st7789_display_t* display);
/** Enable or disable the display backlight. */
esp_err_t st7789_display_set_backlight(st7789_display_t* display, bool enabled);
/** Return the pixel width of the display. */
uint16_t st7789_display_width(const st7789_display_t* display);
/** Return the pixel height of the display. */
uint16_t st7789_display_height(const st7789_display_t* display);
/** Fill the entire display with a single RGB565 colour. */
esp_err_t st7789_display_fill(st7789_display_t* display, uint16_t color565);
/** Draw a single pixel. High overhead — prefer push_pixels for bulk writes. */
esp_err_t st7789_display_draw_pixel(st7789_display_t* display, uint16_t x, uint16_t y, uint16_t color565);
/** Fill a rectangular region with a single RGB565 colour. */
esp_err_t st7789_display_draw_rect(st7789_display_t* display, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                   uint16_t color565);
/** Push a pre-rendered RGB565 pixel buffer into a rectangular window. */
esp_err_t st7789_display_push_pixels(st7789_display_t* display, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                     const uint16_t* pixels, size_t pixel_count);
