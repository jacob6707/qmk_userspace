#pragma once

#include <stdint.h>

typedef int32_t fixed_t;

fixed_t bezier_curve(fixed_t t);
uint8_t get_fadeout_value(uint8_t x);
void find_max_x_coord(uint8_t* max_x_coord);
uint16_t map_to_time_range(uint8_t input);
void reset_rgb_matrix_noeeprom(void);