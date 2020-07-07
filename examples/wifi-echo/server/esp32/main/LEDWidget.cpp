/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 * @file LEDWidget.cpp
 *
 * Implements an LED Widget controller that is usually tied to a GPIO
 * It also updates the display widget if it's enabled
 */

#include "LEDWidget.h"
#include "Display.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <lib/core/CHIPStandardAttributes.h>

#if CONFIG_HAVE_DISPLAY
// The Y position of the LED Status message on screen as a
// percentage of the screen's height.
#define LED_STATUS_POSITION 85
// Position the LED Indicator at the bottom right corner
#define LED_INDICATOR_X 92
#define LED_INDICATOR_Y 88
// The radius of the LED Indicator
#define LED_INDICATOR_R_PX 16

static const char * onMsg  = "LIGHT: ON";
static const char * offMsg = "LIGHT: OFF";

#endif
using namespace ::chip::DataModel;
extern const char * TAG;

void LEDWidget::Init(gpio_num_t gpioNum)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;
    mState            = false;
    mError            = false;
    errorTimer        = NULL;

    if (gpioNum < GPIO_NUM_MAX)
    {
        gpio_set_direction(gpioNum, GPIO_MODE_OUTPUT);
    }
}

void LEDWidget::Set(bool state)
{
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
    CHIPBaseCluster::Set(kAttributeIdOnOff, CHIPValueBool(state));
}

int LEDWidget::Set(uint8_t attrId, const CHIPValue & value)
{
    if (attrId == kAttributeIdOnOff)
    {
        printf("Setting value to %d\n", CHIPValueToBool(value));
        DoSet(CHIPValueToBool(value));
        /* Update our internal data model as well */
        CHIPBaseCluster::Set(attrId, value);
    }
    return SUCCESS;
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

void ClearErrorState(TimerHandle_t handle)
{
#if CONFIG_HAVE_DISPLAY
    LEDWidget * pWidget = (LEDWidget *) pvTimerGetTimerID(handle);
    pWidget->mError     = false;
    pWidget->Display();
    // If a status change occured, wake the display
    WakeDisplay();
#endif
}

void LEDWidget::BlinkOnError()
{
#if CONFIG_HAVE_DISPLAY
    mError = true;
    if (errorTimer != NULL)
    {
        xTimerDelete(errorTimer, 0);
    }
    errorTimer = xTimerCreate("ErrorTimer", pdMS_TO_TICKS(2000), false, this, ClearErrorState);
    xTimerStart(errorTimer, 0);
    Display();
    // If a status change occured, wake the display
    WakeDisplay();
#endif
}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        int64_t nowUS            = ::esp_timer_get_time();
        int64_t stateDurUS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS) * 1000LL;
        int64_t nextChangeTimeUS = mLastChangeTimeUS + stateDurUS;

        if (nowUS > nextChangeTimeUS)
        {
            DoSet(!mState);
            mLastChangeTimeUS = nowUS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;
    if (mGPIONum < GPIO_NUM_MAX)
    {
        gpio_set_level(mGPIONum, (state) ? 1 : 0);
    }
    if (stateChange)
    {
#if CONFIG_HAVE_DISPLAY

        Display();
        // If a status change occured, wake the display
        WakeDisplay();
#endif
    }
}

#if CONFIG_HAVE_DISPLAY
void LEDWidget::Display()
{
    uint16_t msgX    = 0;
    uint16_t msgY    = (DisplayHeight * LED_STATUS_POSITION) / 100;
    uint16_t circleX = (LED_INDICATOR_X * DisplayWidth) / 100;
    uint16_t circleY = (LED_INDICATOR_Y * DisplayHeight) / 100;

    // Wipe the Light Status Area
    ClearRect(0, LED_STATUS_POSITION);
    // Wipe the status circle
    TFT_fillCircle(circleX, circleY, LED_INDICATOR_R_PX, TFT_BLACK);
    // Display the Light Status on screen
    TFT_setFont(DEJAVU24_FONT, NULL);
    // Draw the default "Off" indicator
    TFT_drawCircle(circleX, circleY, LED_INDICATOR_R_PX, TFT_DARKGREY);

    if (mError)
    {
        TFT_print((char *) "Recv Error", msgX, msgY);
        // Draw the "Error" indicator
        TFT_fillCircle(circleX, circleY, LED_INDICATOR_R_PX, TFT_RED);
    }
    else
    {
        if (mState)
        {
            TFT_print((char *) onMsg, msgX, msgY);
            // Draw the "ON" indicator
            TFT_fillCircle(circleX, circleY, LED_INDICATOR_R_PX, TFT_GREEN);
        }
        else
        {
            TFT_print((char *) offMsg, msgX, msgY);
        }
    }
}
#endif
