ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes
OS_DETECTION_ENABLE = yes
CONSOLE_ENABLE = yes
VIA_EEPROM_CUSTOM_CONFIG = yes  # Enable custom EEPROM
# Optional: Specify the size (in bytes) if you need more than 1 byte
VIA_EEPROM_CUSTOM_CONFIG_SIZE = 4

SRC +=  \
		km_util.c \
		custom_config.c