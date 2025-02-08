#pragma once

#include "color.h"

enum via_volume_light_value {
    id_volume_starting_brightness = 1,
    id_volume_fade_speed,
    id_volume_color
};

typedef struct {
    HSV hsv;
    uint8_t fade_speed;
} volume_light_control_t;

typedef union {
    uint32_t raw;
    volume_light_control_t vol_conf;
} custom_config;

extern custom_config g_custom_config;

void kb_config_set_value( uint8_t *data );
void kb_config_get_value( uint8_t *data );
void kb_config_save(void);
void custom_config_init(void);