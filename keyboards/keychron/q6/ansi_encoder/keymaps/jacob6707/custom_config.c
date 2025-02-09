#include "custom_config.h"
#include "eeconfig.h"
#include "via.h"

custom_config g_custom_config;

void custom_config_init(void) {
    g_custom_config.raw = eeconfig_read_kb();
    if (g_custom_config.raw == 0) {
        g_custom_config.vol_conf.hsv.h = 85;
        g_custom_config.vol_conf.hsv.s = 255;
        g_custom_config.vol_conf.hsv.v = 255;
        g_custom_config.vol_conf.fade_speed = 182;
        eeconfig_update_kb(g_custom_config.raw);    
    }
}

void kb_config_set_value( uint8_t *data )
{
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch ( *value_id )
    {
        case id_volume_starting_brightness:
        {
            g_custom_config.vol_conf.hsv.v = *value_data;
            break;
        }
        case id_volume_fade_speed:
        {
            g_custom_config.vol_conf.fade_speed = *value_data;
            break;
        }
        case id_volume_color:
        {
            g_custom_config.vol_conf.hsv.h = value_data[0];
            g_custom_config.vol_conf.hsv.s = value_data[1];
            break;
        }
    }
}

void kb_config_get_value( uint8_t *data )
{
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch ( *value_id )
    {
        case id_volume_starting_brightness:
        {
            *value_data = g_custom_config.vol_conf.hsv.v;
            break;
        }
        case id_volume_fade_speed:
        {
            *value_data = g_custom_config.vol_conf.fade_speed;
            break;
        }
        case id_volume_color:
        {
            value_data[0] = g_custom_config.vol_conf.hsv.h;
            value_data[1] = g_custom_config.vol_conf.hsv.s;
            break;
        }
    }
}

void kb_config_save(void)
{
    eeconfig_update_kb(g_custom_config.raw);
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

    if ( *channel_id == id_custom_channel ) {
        switch ( *command_id )
        {
            case id_custom_set_value:
            {
                kb_config_set_value(value_id_and_data);
                break;
            }
            case id_custom_get_value:
            {
                kb_config_get_value(value_id_and_data);
                break;
            }
            case id_custom_save:
            {
                kb_config_save();
                break;
            }
            default:
            {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }
        return;
    }

    // Return the unhandled state
    *command_id = id_unhandled;

    // DO NOT call raw_hid_send(data,length) here, let caller do this
}