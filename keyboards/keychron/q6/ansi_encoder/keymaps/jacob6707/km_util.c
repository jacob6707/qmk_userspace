#include "km_util.h"
#include <stdint.h>
#include "rgb_matrix.h"

// Fixed-point arithmetic settings
typedef int32_t fixed_t;
#define FIXED_SHIFT 10 // 10 bits for fractional part
#define FIXED_SCALE (1 << FIXED_SHIFT)
#define TO_FIXED(x) ((fixed_t)((x) * FIXED_SCALE))
#define FROM_FIXED(x) ((x) / (double)FIXED_SCALE)

#define MIN_FADE_TIME 200
#define MAX_FADE_TIME 3000

// Control points for the Bézier curve (pre-scaled to fixed-point)
static const fixed_t P0 = TO_FIXED(0.0);
static const fixed_t P1 = TO_FIXED(1.12);
static const fixed_t P2 = TO_FIXED(1.01);
static const fixed_t P3 = TO_FIXED(1.0);

// Optimized Bézier curve calculation using fixed-point arithmetic
fixed_t bezier_curve(fixed_t t) {
    // Precompute terms
    fixed_t t2 = (t * t) >> FIXED_SHIFT; // t^2
    fixed_t t3 = (t2 * t) >> FIXED_SHIFT; // t^3
    fixed_t mt = TO_FIXED(1) - t; // 1 - t
    fixed_t mt2 = (mt * mt) >> FIXED_SHIFT; // (1 - t)^2
    fixed_t mt3 = (mt2 * mt) >> FIXED_SHIFT; // (1 - t)^3

    // Compute the y value using the Bézier formula
    fixed_t y = (mt3 * P0) >> FIXED_SHIFT;
    y += (3 * ((mt2 * t) >> FIXED_SHIFT) * P1) >> FIXED_SHIFT;
    y += (3 * ((mt * t2) >> FIXED_SHIFT) * P2) >> FIXED_SHIFT;
    y += (t3 * P3) >> FIXED_SHIFT;

    return y;
}

uint8_t get_fadeout_value(uint8_t x) {
    // Normalize x to fixed-point range [0, 1]
    fixed_t t = TO_FIXED((double)x / 255.0);

    // Compute the Bézier curve value
    fixed_t y = bezier_curve(t);

    // Scale the result back to the range [0, 255]
    uint8_t result = (uint8_t)(FROM_FIXED(y) * 255.0);

    // Debugging: Print intermediate values
    //printf("x = %d, t = %d, y = %d, result = %d\n", x, t, y, result);

    return result;
}

void find_max_x_coord(uint8_t *max_x_coord) {
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        if (g_led_config.point[i].x > *max_x_coord) {
            *max_x_coord = g_led_config.point[i].x;
        }
    }
}

// Convert a value [0, 255] to [MAX_TIME, MIN_TIME]
uint16_t map_to_time_range(uint8_t input) {
    // Calculate the difference between MAX and MIN
    uint16_t range = MAX_FADE_TIME - MIN_FADE_TIME;
    
    // Map the input to the range (reverse: 255 → MIN_TIME, 0 → MAX_TIME)
    return MAX_FADE_TIME - ((uint16_t)input * range) / 255;
}

void reset_rgb_matrix_noeeprom(void) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_BREATHING);
    rgb_matrix_sethsv_noeeprom(0,0,0);
    rgb_matrix_set_speed_noeeprom(64);
}