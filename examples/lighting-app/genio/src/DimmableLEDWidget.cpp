/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#ifdef __PWM_DIMMABLE_LED__

#include "DimmableLEDWidget.h"
#include <assert.h>
#include <stdio.h>

#include "hal_pwm.h"

#define LIGHT_LEVEL_MIN (0)
#define LIGHT_LEVEL_MAX (100)
#define LIGHT_LEVEL_DEFAULT (50)
#define PWM_FREQUENCY_DEFAULT (1000)
#define LEVEL_TO_PWM_RATIO(level) ((level) *10)

static hal_pwm_channel_t convert_gpio_to_pwm_channel(hal_gpio_pin_t gpio)
{
    hal_pwm_channel_t ret = HAL_PWM_MAX;
    if (gpio >= HAL_GPIO_29 && gpio <= HAL_GPIO_40)
    {
        ret = (hal_pwm_channel_t) (gpio - HAL_GPIO_29);
    }
    else if (gpio >= HAL_GPIO_45 && gpio <= HAL_GPIO_52)
    {
        ret = (hal_pwm_channel_t) (gpio - HAL_GPIO_45);
    }
    else
    {
        assert(false);
    }

    return ret;
}

void DimmableLEDWidget::Init(hal_gpio_pin_t gpio)
{
    hal_pwm_channel_t pwm_channel = HAL_PWM_MAX;
    uint32_t total_count          = 0;
    mGpioPin                      = gpio;
    mState                        = false;
    mLevel                        = LIGHT_LEVEL_DEFAULT;

    hal_gpio_init(mGpioPin);
    if (mGpioPin >= HAL_GPIO_29 && mGpioPin <= HAL_GPIO_40)
    {
        assert(HAL_PINMUX_STATUS_OK == hal_pinmux_set_function(mGpioPin, 3));
    }
    else if (mGpioPin >= HAL_GPIO_45 && mGpioPin <= HAL_GPIO_52)
    {
        assert(HAL_PINMUX_STATUS_OK == hal_pinmux_set_function(mGpioPin, 5));
    }
    else
    {
        /* This GPIO is not supported PWM */
        assert(false);
    }

    pwm_channel = convert_gpio_to_pwm_channel(mGpioPin);

    assert(HAL_PWM_STATUS_OK == hal_pwm_init(pwm_channel, HAL_PWM_CLOCK_26MHZ));
    assert(HAL_PWM_STATUS_OK == hal_pwm_set_frequency(pwm_channel, PWM_FREQUENCY_DEFAULT, &total_count));
    assert(HAL_PWM_STATUS_OK == hal_pwm_set_duty_cycle(pwm_channel, LEVEL_TO_PWM_RATIO(mLevel)));
}

void DimmableLEDWidget::Set(bool state)
{
    hal_pwm_channel_t pwm_channel = convert_gpio_to_pwm_channel(mGpioPin);

    if (state && !mState)
    {
        mState = true;
        assert(HAL_PWM_STATUS_OK == hal_pwm_start(pwm_channel));
    }
    else if (!state && mState)
    {
        mState = false;
        assert(HAL_PWM_STATUS_OK == hal_pwm_stop(pwm_channel));
    }
}

bool DimmableLEDWidget::Get(void)
{
    return mState;
}

void DimmableLEDWidget::SetLevel(uint8_t level)
{
    hal_pwm_channel_t pwm_channel = convert_gpio_to_pwm_channel(mGpioPin);

    if (level >= LIGHT_LEVEL_MIN && level <= LIGHT_LEVEL_MAX && level != mLevel)
    {
        mLevel = level;
        assert(HAL_PWM_STATUS_OK == hal_pwm_set_duty_cycle(pwm_channel, LEVEL_TO_PWM_RATIO(mLevel)));
    }
}

uint8_t DimmableLEDWidget::GetLevel(void)
{
    return mLevel;
}

uint8_t DimmableLEDWidget::GetMaxLevel(void)
{
    return LIGHT_LEVEL_MAX;
}

uint8_t DimmableLEDWidget::GetMinLevel(void)
{
    return LIGHT_LEVEL_MIN;
}

#endif /* __PWM_DIMMABLE_LED__ */
