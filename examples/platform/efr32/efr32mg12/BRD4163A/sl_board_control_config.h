#ifndef SL_BOARD_CONTROL_CONFIG_H
#define SL_BOARD_CONTROL_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q SL_BOARD_ENABLE_VCOM> Enable Virtual COM UART
// <i> Default: 0
#define SL_BOARD_ENABLE_VCOM                    0

// <q SL_BOARD_ENABLE_DISPLAY> Enable Display
// <i> Default: 0
#define SL_BOARD_ENABLE_DISPLAY                 0

// <q SL_BOARD_ENABLE_SENSOR_RHT> Enable Relative Humidity and Temperature sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_RHT              0

// <q SL_BOARD_DISABLE_MEMORY_SPI> Disable SPI Flash
// <i> Default: 1
#define SL_BOARD_DISABLE_MEMORY_SPI             1

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_BOARD_ENABLE_VCOM
// $[GPIO_SL_BOARD_ENABLE_VCOM]
#define SL_BOARD_ENABLE_VCOM_PORT                gpioPortA
#define SL_BOARD_ENABLE_VCOM_PIN                 5
// [GPIO_SL_BOARD_ENABLE_VCOM]$

// <gpio> SL_BOARD_ENABLE_DISPLAY
// $[GPIO_SL_BOARD_ENABLE_DISPLAY]
#define SL_BOARD_ENABLE_DISPLAY_PORT             gpioPortD
#define SL_BOARD_ENABLE_DISPLAY_PIN              15
// [GPIO_SL_BOARD_ENABLE_DISPLAY]$

// <gpio> SL_BOARD_ENABLE_SENSOR_RHT
// $[GPIO_SL_BOARD_ENABLE_SENSOR_RHT]
#define SL_BOARD_ENABLE_SENSOR_RHT_PORT          gpioPortB
#define SL_BOARD_ENABLE_SENSOR_RHT_PIN           10
// [GPIO_SL_BOARD_ENABLE_SENSOR_RHT]$

// <<< sl:end pin_tool >>>

#endif // SL_BOARD_CONTROL_CONFIG_H
