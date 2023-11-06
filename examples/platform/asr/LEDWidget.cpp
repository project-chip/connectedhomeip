/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "AppConfig.h"
#include <platform/CHIPDeviceLayer.h>

/*******************************************************************************
 * Macro Definitions
 *******************************************************************************/
/* Allowed TCPWM compare value for maximum brightness */
#define LED_MAX_BRIGHTNESS (100u)

/* Allowed TCPWM compare value for minimum brightness*/
#define LED_MIN_BRIGHTNESS (0u)

#define PWM_LED_FREQ_HZ (1000u) /* in Hz */

/* subtracting from 100 since the LED is connected in active low configuration */
#define GET_DUTY_CYCLE(x) (100 - x)

// PWM period in micro seconds
#define LED_PWM_PERIOD_US (255u)

#ifdef LIGHT_SELECT_RGB
static void show_pwm(duet_pwm_dev_t * pwm_dev, uint8_t val)
{
    duet_pwm_config_t pwm_cfg;

    if (val < LED_MIN_BRIGHTNESS)
        val = LED_MIN_BRIGHTNESS;
    pwm_cfg.duty_cycle = (float) (val) / LED_PWM_PERIOD_US;
    pwm_cfg.freq       = PWM_LED_FREQ_HZ;

    duet_pwm_para_chg(pwm_dev, pwm_cfg);
}

static void init_pwm(duet_pwm_dev_t * pwm_dev, uint8_t ledNum, uint8_t val)
{
    pwm_dev->port              = ledNum;
    pwm_dev->config.duty_cycle = val;
    pwm_dev->config.freq       = PWM_LED_FREQ_HZ;
    pwm_dev->priv              = NULL;
    duet_pwm_init(pwm_dev);
}
#endif

void LEDWidget::Init(uint8_t ledNum)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mState            = 0;
    mbrightness       = LED_MIN_BRIGHTNESS;
    mHue              = 0;
    mSaturation       = 0;
#ifdef LIGHT_SELECT_RGB
    if (ledNum == LIGHT_RGB_GREEN)
    {
#ifdef CFG_PLF_RV32
        asr_pinmux_config(LIGHT_RGB_RED_PAD, PF_PWM);
#endif
        init_pwm(&pwm_led_g, ledNum, LED_MIN_BRIGHTNESS);
    }
    else if (ledNum == LIGHT_RGB_BLUE)
    {
#ifdef CFG_PLF_RV32
        asr_pinmux_config(LIGHT_RGB_BLUE_PAD, PF_PWM);
#endif
        init_pwm(&pwm_led_b, ledNum, LED_MIN_BRIGHTNESS);
    }
    else
    {
#ifdef CFG_PLF_RV32
        asr_pinmux_config(LIGHT_RGB_GREEN_PAD, PF_PWM);
#endif
        init_pwm(&pwm_led, ledNum, LED_MIN_BRIGHTNESS);
    }
#else
    gpio.port   = ledNum;
    gpio.config = DUET_OUTPUT_PUSH_PULL;
    duet_gpio_init(&gpio);
#endif
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
        uint64_t stateDurMS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS);
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
#ifdef LIGHT_SELECT_RGB
        (state) ? PWM_start() : PWM_stop();
#else
        (state) ? duet_gpio_output_low(&gpio) : duet_gpio_output_high(&gpio);
#endif
    }
    mState = state;
}

#ifdef LIGHT_SELECT_RGB
void LEDWidget::RGB_init()
{
    Init(LIGHT_RGB_RED);   // red light of RGB
    Init(LIGHT_RGB_GREEN); // green light of RGB
    Init(LIGHT_RGB_BLUE);  // blue light of RGB
}

void LEDWidget::PWM_start()
{
    if (!mState)
    {
        /* Start PWM to turn the LED on */
        if (0 != duet_pwm_start(&pwm_led))
        {
            ASR_LOG("PWM failed to start!");
        }

        /* Start PWM to turn the LED on */
        if (0 != duet_pwm_start(&pwm_led_g))
        {
            ASR_LOG("PWM failed to start!");
        }

        /* Start PWM to turn the LED on */
        if (0 != duet_pwm_start(&pwm_led_b))
        {
            ASR_LOG("PWM failed to start!");
        }
        mState = 1;
    }
}

void LEDWidget::PWM_stop()
{
    SetBrightness(LED_MIN_BRIGHTNESS);
    mbrightness = LED_MIN_BRIGHTNESS;
    mState      = 0;
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    mbrightness = brightness;
    SetColor(mHue, mSaturation);
}

void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    uint8_t red, green, blue;
    uint8_t sSaturation, sbrightness;
    uint16_t sHue;

    sbrightness = (mState) ? mbrightness : 0;

    sHue        = static_cast<uint16_t>(Hue) * 360 / 254;       // sHue [0, 360]
    sSaturation = static_cast<uint8_t>(Saturation) * 100 / 254; // sSaturation [0 , 100]

    HSB2rgb(sHue, sSaturation, sbrightness, red, green, blue);
    ASR_LOG("brightness: %d, red: %d, green: %d, blue: %d", sbrightness, red, green, blue);

    mHue        = Hue;
    mSaturation = Saturation;

    showRGB(red, green, blue);
}

void LEDWidget::HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue)
{
    uint16_t i       = Hue / 60;
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
}

void LEDWidget::showRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    show_pwm(&pwm_led, red);

    show_pwm(&pwm_led_g, green);

    show_pwm(&pwm_led_b, blue);
}
#endif
