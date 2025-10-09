/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
 */
#include <bflb_clock.h>
#include <bflb_gpio.h>
#include <bflb_pwm_v2.h>
#include <stdio.h>

#include "demo_pwm.h"
#include "mboard.h"

#if MAX_PWM_CHANNEL == 3
static const uint32_t rgb_pwm[MAX_PWM_CHANNEL][2] = {
    { LED_B_PIN, LED_B_PWM_CH },
    { LED_R_PIN, LED_R_PWM_CH },
    { LED_G_PIN, LED_G_PWM_CH },
};
#else
static const uint32_t rgb_pwm[][2] = {
    { LED_PIN, LED_PIN_PWM_CH },
};
#endif

struct bflb_device_s * bflb_device_pwm = NULL;

void demo_pwm_init(void)
{
    struct bflb_pwm_v2_config_s cfg = {
        .clk_source = BFLB_SYSTEM_PBCLK,
        .clk_div    = 40,
        .period     = 1000,
    };
    struct bflb_device_s * gpio = bflb_device_get_by_name("gpio");

    bflb_device_pwm = bflb_device_get_by_name("pwm_v2_0");

    for (int i = 0; i < sizeof(rgb_pwm) / sizeof(rgb_pwm[0]); i++)
    {
        bflb_gpio_init(gpio, rgb_pwm[i][0], GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    }

    bflb_pwm_v2_init(bflb_device_pwm, &cfg);
}

void demo_pwm_start(void)
{
    for (int i = 0; i < sizeof(rgb_pwm) / sizeof(rgb_pwm[0]); i++)
    {
        bflb_pwm_v2_channel_positive_start(bflb_device_pwm, rgb_pwm[i][1]);
    }

    bflb_pwm_v2_start(bflb_device_pwm);
}

void demo_pwm_stop(void)
{
    bflb_pwm_v2_stop(bflb_device_pwm);
}

void set_level(uint8_t currLevel)
{
    int period;

    if (currLevel <= 5 && currLevel >= 1)
    {
        currLevel = 5; // avoid demo off
    }
    period = (int) currLevel * 1000 / 254;

    for (int i = 0; i < sizeof(rgb_pwm) / sizeof(rgb_pwm[0]); i++)
    {
        bflb_pwm_v2_channel_set_threshold(bflb_device_pwm, rgb_pwm[i][1], 0, period);
    }
}

void set_color_red(uint8_t currLevel)
{
    set_color(currLevel, 0, 254);
}

void set_color_green(uint8_t currLevel)
{
    set_color(currLevel, 84, 254);
}

void set_color_yellow(uint8_t currLevel)
{
    set_color(currLevel, 42, 254);
}

void set_color(uint8_t currLevel, uint8_t currHue, uint8_t currSat)
{
#if MAX_PWM_CHANNEL == 3
    uint16_t hue = (uint16_t) currHue * 360 / 254;
    uint8_t sat  = (uint16_t) currSat * 100 / 254;

    if (currLevel <= 5 && currLevel >= 1)
    {
        currLevel = 5; // avoid demo off
    }

    if (sat > 100)
    {
        sat = 100;
    }

    uint16_t i       = hue / 60;
    uint16_t rgb_max = currLevel;
    uint16_t rgb_min = rgb_max * (100 - sat) / 100;
    uint16_t diff    = hue % 60;
    uint16_t rgb_adj = (rgb_max - rgb_min) * diff / 60;
    uint32_t red, green, blue;

    switch (i)
    {
    case 0:
        red   = rgb_max;
        green = rgb_min + rgb_adj;
        blue  = rgb_min;
        break;
    case 1:
        red   = rgb_max - rgb_adj;
        green = rgb_max;
        blue  = rgb_min;
        break;
    case 2:
        red   = rgb_min;
        green = rgb_max;
        blue  = rgb_min + rgb_adj;
        break;
    case 3:
        red   = rgb_min;
        green = rgb_max - rgb_adj;
        blue  = rgb_max;
        break;
    case 4:
        red   = rgb_min + rgb_adj;
        green = rgb_min;
        blue  = rgb_max;
        break;
    default:
        red   = rgb_max;
        green = rgb_min;
        blue  = rgb_max - rgb_adj;
        break;
    }

    bflb_pwm_v2_channel_set_threshold(bflb_device_pwm, rgb_pwm[0][1], 0, blue * 1000 / 254);
    bflb_pwm_v2_channel_set_threshold(bflb_device_pwm, rgb_pwm[1][1], 0, red * 1000 / 254);
    bflb_pwm_v2_channel_set_threshold(bflb_device_pwm, rgb_pwm[2][1], 0, green * 1000 / 254);
#else
    set_level(currLevel);
#endif
}
