/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
 */
#include <stdio.h>

#include <hosal_pwm.h>

#include "demo_pwm.h"
#include "mboard.h"

#define PWM_FREQ 1000
#define PWM_DUTY_CYCLE 10000

hosal_pwm_dev_t rgb_pwm[] = {

#if MAX_PWM_CHANNEL == 3
    {
        .port = LED_B_PIN_PORT,
        /* pwm config */
        .config.pin        = LED_B_PIN,
        .config.duty_cycle = 0,        // duty_cycle range is 0~10000 correspond to 0~100%
        .config.freq       = PWM_FREQ, // freq range is between 0~40MHZ
    },
    {
        .port = LED_R_PIN_PORT,
        /* pwm config */
        .config.pin        = LED_R_PIN,
        .config.duty_cycle = 0,        // duty_cycle range is 0~10000 correspond to 0~100%
        .config.freq       = PWM_FREQ, // freq range is between 0~40MHZ
    },
    {
        .port = LED_G_PIN_PORT,
        /* pwm config */
        .config.pin        = LED_G_PIN,
        .config.duty_cycle = 0,        // duty_cycle range is 0~10000 correspond to 0~100%
        .config.freq       = PWM_FREQ, // freq range is between 0~40MHZ
    }
#else
    {
        .port = LED_PIN_PORT,
        /* pwm config */
        .config.pin        = LED_PIN,
        .config.duty_cycle = 0,        // duty_cycle range is 0~10000 correspond to 0~100%
        .config.freq       = PWM_FREQ, // freq range is between 0~40MHZ
    },
#endif
};

void demo_pwm_init(void)
{
    /* init pwm with given settings */
    for (uint32_t i = 0; i < MAX_PWM_CHANNEL; i++)
    {
        hosal_pwm_init(rgb_pwm + i);
    }
}

void demo_pwm_start(void)
{
    /* start pwm */
    for (uint32_t i = 0; i < MAX_PWM_CHANNEL; i++)
    {
        hosal_pwm_start(rgb_pwm + i);
    }
}

void demo_pwm_change_param(hosal_pwm_config_t * para)
{
    /* change pwm param */
    for (uint32_t i = 0; i < MAX_PWM_CHANNEL; i++)
    {
        if (para[i].duty_cycle > PWM_DUTY_CYCLE)
        {
            para[i].duty_cycle = PWM_DUTY_CYCLE;
        }
        hosal_pwm_para_chg(rgb_pwm + i, para[i]);
    }
}

void demo_pwm_stop(void)
{
    for (uint32_t i = 0; i < MAX_PWM_CHANNEL; i++)
    {
        hosal_pwm_stop(rgb_pwm + i);
        hosal_pwm_finalize(rgb_pwm + i);
    }
}

void set_level(uint8_t currLevel)
{
    hosal_pwm_config_t para;

    if (currLevel <= 5 && currLevel >= 1)
    {
        currLevel = 5; // avoid demo off
    }

    para.duty_cycle = currLevel * PWM_DUTY_CYCLE / 254;
    para.freq       = PWM_FREQ;

    demo_pwm_change_param(&para);
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

    // change level to pwm duty_cycle
    // 0-254 to 0-10000
    hosal_pwm_config_t para[3];
    para[0].duty_cycle = blue * PWM_DUTY_CYCLE / 254;
    para[0].freq       = PWM_FREQ;
    para[1].duty_cycle = red * PWM_DUTY_CYCLE / 254;
    para[1].freq       = PWM_FREQ;
    para[2].duty_cycle = green * PWM_DUTY_CYCLE / 254;
    para[2].freq       = PWM_FREQ;

    demo_pwm_change_param(para);
#else
    set_level(currLevel);
#endif
}
