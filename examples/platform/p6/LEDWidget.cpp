/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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

#include "LEDWidget.h"

#include "cybsp.h"
#include "cyhal.h"
#include <platform/CHIPDeviceLayer.h>

/*******************************************************************************
 * Macro Definitions
 *******************************************************************************/
/* Allowed TCPWM compare value for maximum brightness */
#define LED_MAX_BRIGHTNESS (100u)

/* Allowed TCPWM compare value for minimum brightness*/
#define LED_MIN_BRIGHTNESS (1u)

#define PWM_LED_FREQ_HZ (1000000u) /* in Hz */

/* subtracting from 100 since the LED is connected in active low configuration */
#define GET_DUTY_CYCLE(x) (100 - x)

// PWM period in micro seconds
#define CY_RGB_LED_PWM_PERIOD_US (255u)

// Clock instance
static cyhal_clock_t clk_pwm;

void LEDWidget::Init(int ledNum)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLedNum           = ledNum;
    mState            = 0;
    mbrightness       = LED_MAX_BRIGHTNESS;

    if (CY_RSLT_SUCCESS !=
        cyhal_gpio_init((cyhal_gpio_t) ledNum, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF))
    {
        printf("GPIO Init failed for Led %d \r\n", ledNum);
    }
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeMS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

bool LEDWidget::Get()
{
    return mState;
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;
    Animate();
}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = mState ? mBlinkOnTimeMS : mBlinkOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            DoSet(!mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    if (mState != state)
    {
        cyhal_gpio_write((cyhal_gpio_t) mLedNum, ((state) ? CYBSP_LED_STATE_ON : CYBSP_LED_STATE_OFF));
    }
    mState = state;
}

void LEDWidget::RGB_init()
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    // Allocate and assign the clock for TCPWMs for RGB LED control
    result = cyhal_clock_allocate(&clk_pwm, CYHAL_CLOCK_BLOCK_PERIPHERAL_16BIT);
    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_clock_set_frequency(&clk_pwm, PWM_LED_FREQ_HZ, NULL);
    }

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_clock_set_enabled(&clk_pwm, true, true);
    }

    if (CY_RSLT_SUCCESS != cyhal_pwm_init(&pwm_red, CYBSP_LED_RGB_RED, &clk_pwm))
    {
        printf("PWM Init failed for RBG Red LED!");
    }
    if (CY_RSLT_SUCCESS != cyhal_pwm_init(&pwm_green, CYBSP_LED_RGB_GREEN, &clk_pwm))
    {
        printf("PWM Init failed for RBG Green LED!");
    }
    if (CY_RSLT_SUCCESS != cyhal_pwm_init(&pwm_blue, CYBSP_LED_RGB_BLUE, &clk_pwm))
    {
        printf("PWM Init failed for RBG blue LED!");
    }

    if (CY_RSLT_SUCCESS != cyhal_pwm_set_duty_cycle(&pwm_red, GET_DUTY_CYCLE(LED_MAX_BRIGHTNESS), PWM_LED_FREQ_HZ))
    {
        printf("PWM failed to set dutycycle for RBG Red LED!");
    }
    if (CY_RSLT_SUCCESS != cyhal_pwm_set_duty_cycle(&pwm_green, GET_DUTY_CYCLE(LED_MAX_BRIGHTNESS), PWM_LED_FREQ_HZ))
    {
        printf("PWM failed to set dutycycle for RBG Green LED!");
    }
    if (CY_RSLT_SUCCESS != cyhal_pwm_set_duty_cycle(&pwm_blue, GET_DUTY_CYCLE(LED_MAX_BRIGHTNESS), PWM_LED_FREQ_HZ))
    {
        printf("PWM failed to set dutycycle for RBG Blue LED!");
    }

    mState      = 0;
    mbrightness = LED_MAX_BRIGHTNESS;
    mHue        = 0;
    mSaturation = 0;
}

void LEDWidget::PWM_start()
{
    if (!mState)
    {
        /* Start PWM to turn the LED on */
        if (CY_RSLT_SUCCESS != cyhal_pwm_start(&pwm_led))
        {
            printf("PWM failed to start!");
        }
        mState = 1;
    }
}

void LEDWidget::PWM_stop()
{
    if (mState)
    {
        /* Stop PWM to turn the LED off */
        if (CY_RSLT_SUCCESS != cyhal_pwm_stop(&pwm_led))
        {
            printf("PWM failed to stop!");
        }
        mState = 0;
    }
}

void LEDWidget::RGB_set(bool state)
{
    if (mState != state)
    {
        if (state)
        {
            if (CY_RSLT_SUCCESS != cyhal_pwm_start(&pwm_red))
            {
                printf("PWM failed to start!");
            }
            if (CY_RSLT_SUCCESS != cyhal_pwm_start(&pwm_green))
            {
                printf("PWM failed to start!");
            }
            if (CY_RSLT_SUCCESS != cyhal_pwm_start(&pwm_blue))
            {
                printf("PWM failed to start!");
            }
            mState = 1;
        }
        else
        {
            if (CY_RSLT_SUCCESS != cyhal_pwm_stop(&pwm_red))
            {
                printf("PWM failed to start!");
            }
            if (CY_RSLT_SUCCESS != cyhal_pwm_stop(&pwm_green))
            {
                printf("PWM failed to start!");
            }
            if (CY_RSLT_SUCCESS != cyhal_pwm_stop(&pwm_blue))
            {
                printf("PWM failed to start!");
            }
            mState = 0;
        }
    }
}
void LEDWidget::SetBrightness(uint32_t led_brightness)
{
    mbrightness = (led_brightness < LED_MIN_BRIGHTNESS) ? LED_MIN_BRIGHTNESS
                                                        : static_cast<uint16_t>(led_brightness) * 100 / CY_RGB_LED_PWM_PERIOD_US;

    /* Set Brightness for RGB LED */
    if (pwm_red.pin)
    {
        HSB2rgb(mHue, mSaturation, mbrightness);
    }
    else
    {
        /* Drive the LED with brightness */
        if (CY_RSLT_SUCCESS != cyhal_pwm_set_duty_cycle(&pwm_led, GET_DUTY_CYCLE(mbrightness), PWM_LED_FREQ_HZ))
        {
            printf("PWM failed to set dutycycle!");
        }
    }
}

void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    uint8_t brightness = (mState) ? mbrightness : 0;

    mHue        = static_cast<uint16_t>(Hue) * 360 / 254;        // mHue [0, 360]
    mSaturation = static_cast<uint16_t>(Saturation) * 100 / 254; // mSaturation [0 , 100]

    HSB2rgb(mHue, mSaturation, brightness);
}

void LEDWidget::HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness)
{
    uint8_t red, green, blue;
    uint16_t i       = Hue / 60;
    brightness       = static_cast<uint16_t>(brightness) * CY_RGB_LED_PWM_PERIOD_US / 100;
    uint16_t rgb_max = brightness;
    uint16_t rgb_min = rgb_max * (100 - Saturation) / 100;
    uint16_t diff    = Hue % 60;
    uint16_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

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

    /* LED is configured as ACTIVE LOW */
    red   = CY_RGB_LED_PWM_PERIOD_US - red;
    green = CY_RGB_LED_PWM_PERIOD_US - green;
    blue  = CY_RGB_LED_PWM_PERIOD_US - blue;

    /* Set Period to control color setting */
    cyhal_pwm_set_period(&pwm_red, CY_RGB_LED_PWM_PERIOD_US, red);
    cyhal_pwm_set_period(&pwm_green, CY_RGB_LED_PWM_PERIOD_US, green);
    cyhal_pwm_set_period(&pwm_blue, CY_RGB_LED_PWM_PERIOD_US, blue);
}
