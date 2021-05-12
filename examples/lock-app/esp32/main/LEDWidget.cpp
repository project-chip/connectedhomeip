#pragma once
#include "LEDWidget.h"
#include "AppTask.h"
#include <platform/CHIPDeviceLayer.h>

using namespace chip::System::Platform::Layer;
void LEDWidget::Init(gpio_num_t gpioNum)
{
    printf("LEDWidget::Init\n");
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;

    if (gpioNum < GPIO_NUM_MAX)
    {
        gpio_set_direction(gpioNum, GPIO_MODE_OUTPUT);
    }
}

void LEDWidget::Invert(void)
{
    printf("LEDWidget::Invert\n");
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    printf("LEDWidget::Set\n");
    mBlinkOnTimeMS  = 0;
    mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    printf("LEDWidget::Blink\n");
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    //    printf("LEDWidget::Blink(uint32_t\n");
    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;
    Animate();
}

void LEDWidget::Animate()
{
    //    printf("LEDWidget::Animate\n");
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        int64_t nowUS            = GetClock_MonotonicHiRes();
        int64_t stateDurUS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS) * 1000LL;
        int64_t nextChangeTimeUS = mLastChangeTimeUS + stateDurUS;

        if (nowUS > nextChangeTimeUS)
        {
            //            DoSet(!mState);
            mLastChangeTimeUS = nowUS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    printf("LEDWidget::DoSet\n");
    bool stateChange = (mState != state);
    mState           = state;
    if (mGPIONum < GPIO_NUM_MAX)
    {
        gpio_set_level(mGPIONum, (state) ? 1 : 0);
        GetAppTask().ButtonEventHandler(mGPIONum, state);
    }
}
