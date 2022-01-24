/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <platform_stdlib.h>

#ifdef CONFIG_PLATFORM_8710C
#include "rtl8710c_pin_name.h"

typedef struct hal_gpio_pin_adapter_s
{
    uint8_t pin_name;     ///< chip pin name to locate the GPIO pin by port & pin index
    uint8_t port_idx;     ///< the GPIO IP port index of the GPIO pin
    uint8_t pin_idx;      ///< the GPIO IP pin index of the GPIO port
    uint8_t debounce_idx; ///< the index of GPIO debounce function allocated to this GPIO pin, 0 ~ 15
    uint32_t bit_mask;    ///< the bit mask to read/write register

    volatile uint32_t * in_port;   ///< the IN port address
    volatile uint32_t * out0_port; ///< the OUT port address for write 0
    volatile uint32_t * out1_port; ///< the OUT port address for write 1
    volatile uint32_t * outt_port; ///< the OUT port address for toggling
} hal_gpio_adapter_t, *phal_gpio_adapter_t;

typedef enum
{
    PIN_INPUT = 0,
    PIN_OUTPUT
} PinDirection;

typedef enum
{
    PA_0  = PIN_A0,
    PA_1  = PIN_A1,
    PA_2  = PIN_A2,
    PA_3  = PIN_A3,
    PA_4  = PIN_A4,
    PA_5  = PIN_A5,
    PA_6  = PIN_A6,
    PA_7  = PIN_A7,
    PA_8  = PIN_A8,
    PA_9  = PIN_A9,
    PA_10 = PIN_A10,
    PA_11 = PIN_A11,
    PA_12 = PIN_A12,
    PA_13 = PIN_A13,
    PA_14 = PIN_A14,
    PA_15 = PIN_A15,
    PA_16 = PIN_A16,
    PA_17 = PIN_A17,
    PA_18 = PIN_A18,
    PA_19 = PIN_A19,
    PA_20 = PIN_A20,
    PA_21 = PIN_A21,
    PA_22 = PIN_A22,
    PA_23 = PIN_A23,

    PB_0  = PIN_B0,
    PB_1  = PIN_B1,
    PB_2  = PIN_B2,
    PB_3  = PIN_B3,
    PB_4  = PIN_B4,
    PB_5  = PIN_B5,
    PB_6  = PIN_B6,
    PB_7  = PIN_B7,
    PB_8  = PIN_B8,
    PB_9  = PIN_B9,
    PB_10 = PIN_B10,
    PB_11 = PIN_B11,
    PB_12 = PIN_B12,

    // Arduino connector namings
    /*
        A0          = PA_0,
        A1          = PA_1,
        A2          = PA_4,
        A3          = PB_0,
        A4          = PC_1,
        A5          = PC_0,
        D0          = PA_3,
        D1          = PA_2,
        D2          = PA_10,
        D3          = PB_3,
        D4          = PB_5,
        D5          = PB_4,
        D6          = PB_10,
        D7          = PA_8,
        D8          = PA_9,
        D9          = PC_7,
        D10         = PB_6,
        D11         = PA_7,
        D12         = PA_6,
        D13         = PA_5,
        D14         = PB_9,
        D15         = PB_8,
    */

    // Generic signals namings
    /*
        LED1        = PB_4,
        LED2        = PB_5,
        LED3        = PB_6,
        LED4        = PB_7,
        USER_BUTTON = PA_3,
        SERIAL_TX   = PA_7,
        SERIAL_RX   = PA_6,
        USBTX       = PA_7,
        USBRX       = PA_6,
        I2C_SCL     = PC_5,
        I2C_SDA     = PC_4,
        SPI_MOSI    = PC_2,
        SPI_MISO    = PC_3,
        SPI_SCK     = PC_1,
        SPI_CS      = PC_0,
        PWM_OUT     = PD_4,
    */
    // Not connected
    NC = (uint32_t) 0xFFFFFFFF
} PinName;

typedef enum
{
    PullNone    = 0,
    PullUp      = 1,
    PullDown    = 2,
    OpenDrain   = 3,
    PullDefault = PullNone
} PinMode;

struct gpio_s
{
    hal_gpio_adapter_t adapter;
};

#else

typedef enum
{
    PORT_A = 0,
    PORT_B = 1,
} GPIO_PORT;

typedef enum
{
    PIN_INPUT = 0,
    PIN_OUTPUT
} PinDirection;

typedef enum
{
    PullNone    = 0, // IN HIGHZ
    PullUp      = 1,
    PullDown    = 2,
    PullDefault = PullNone
} PinMode;

/* (((port)<<5)|(pin)) */
typedef enum
{
    PA_0  = (PORT_A << 5 | 0),
    PA_1  = (PORT_A << 5 | 1),
    PA_2  = (PORT_A << 5 | 2),
    PA_3  = (PORT_A << 5 | 3),
    PA_4  = (PORT_A << 5 | 4),
    PA_5  = (PORT_A << 5 | 5),
    PA_6  = (PORT_A << 5 | 6),
    PA_7  = (PORT_A << 5 | 7),
    PA_8  = (PORT_A << 5 | 8),
    PA_9  = (PORT_A << 5 | 9),
    PA_10 = (PORT_A << 5 | 10),
    PA_11 = (PORT_A << 5 | 11),
    PA_12 = (PORT_A << 5 | 12),
    PA_13 = (PORT_A << 5 | 13),
    PA_14 = (PORT_A << 5 | 14),
    PA_15 = (PORT_A << 5 | 15),
    PA_16 = (PORT_A << 5 | 16),
    PA_17 = (PORT_A << 5 | 17),
    PA_18 = (PORT_A << 5 | 18),
    PA_19 = (PORT_A << 5 | 19),
    PA_20 = (PORT_A << 5 | 20),
    PA_21 = (PORT_A << 5 | 21),
    PA_22 = (PORT_A << 5 | 22),
    PA_23 = (PORT_A << 5 | 23),
    PA_24 = (PORT_A << 5 | 24),
    PA_25 = (PORT_A << 5 | 25),
    PA_26 = (PORT_A << 5 | 26),
    PA_27 = (PORT_A << 5 | 27),
    PA_28 = (PORT_A << 5 | 28),
    PA_29 = (PORT_A << 5 | 29),
    PA_30 = (PORT_A << 5 | 30),
    PA_31 = (PORT_A << 5 | 31),

    PB_0  = (PORT_B << 5 | 0),
    PB_1  = (PORT_B << 5 | 1),
    PB_2  = (PORT_B << 5 | 2),
    PB_3  = (PORT_B << 5 | 3),
    PB_4  = (PORT_B << 5 | 4),
    PB_5  = (PORT_B << 5 | 5),
    PB_6  = (PORT_B << 5 | 6),
    PB_7  = (PORT_B << 5 | 7),
    PB_8  = (PORT_B << 5 | 8),
    PB_9  = (PORT_B << 5 | 9),
    PB_10 = (PORT_B << 5 | 10),
    PB_11 = (PORT_B << 5 | 11),
    PB_12 = (PORT_B << 5 | 12),
    PB_13 = (PORT_B << 5 | 13),
    PB_14 = (PORT_B << 5 | 14),
    PB_15 = (PORT_B << 5 | 15),
    PB_16 = (PORT_B << 5 | 16),
    PB_17 = (PORT_B << 5 | 17),
    PB_18 = (PORT_B << 5 | 18),
    PB_19 = (PORT_B << 5 | 19),
    PB_20 = (PORT_B << 5 | 20),
    PB_21 = (PORT_B << 5 | 21),
    PB_22 = (PORT_B << 5 | 22),
    PB_23 = (PORT_B << 5 | 23),
    PB_24 = (PORT_B << 5 | 24),
    PB_25 = (PORT_B << 5 | 25),
    PB_26 = (PORT_B << 5 | 26),
    PB_27 = (PORT_B << 5 | 27),
    PB_28 = (PORT_B << 5 | 28),
    PB_29 = (PORT_B << 5 | 29),
    PB_30 = (PORT_B << 5 | 30),
    PB_31 = (PORT_B << 5 | 31),

    VBAT_MEAS = (0x7 << 5 | 2),
    AD_0      = PB_4,      // CH0
    AD_1      = PB_5,      // CH1
    AD_2      = PB_6,      // CH2
    AD_3      = PB_7,      // CH3
    AD_4      = PB_1,      // CH4
    AD_5      = PB_2,      // CH5
    AD_6      = PB_3,      // CH6
    AD_7      = VBAT_MEAS, // CH7

    // Not connected
    NC = (uint32_t) 0xFFFFFFFF
} PinName;

typedef struct gpio_s
{
    PinName pin;
} gpio_t;

#endif

typedef struct gpio_s gpio_t;

extern "C" void gpio_init(gpio_t * obj, PinName pin);
extern "C" uint32_t gpio_set(PinName pin);
extern "C" void gpio_mode(gpio_t * obj, PinMode mode);
extern "C" void gpio_dir(gpio_t * obj, PinDirection direction);
extern "C" void gpio_write(gpio_t * obj, int value);
extern "C" int gpio_read(gpio_t * obj);

class LEDWidget
{
public:
    void Init(PinName gpioNum);
    void Set(bool state);

private:
    PinName mGPIONum;
    bool mState;
    void DoSet(bool state);
};
