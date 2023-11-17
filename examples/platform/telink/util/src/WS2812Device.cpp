/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "WS2812Device.h"

#include <lib/support/CodeUtils.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(WS2812Device);

using namespace chip;

CHIP_ERROR WS2812Device::Init(const struct device * ws2812Device, uint32_t aChainLength)
{
    mWs2812Device = ws2812Device;
    mChainLength  = aChainLength;
    mState        = kRgbState_Off;
    memset(&mLedRgb, RGB_MAX_VALUE, sizeof(RgbColor_t));

    if (!device_is_ready(mWs2812Device))
    {
        LOG_ERR("Device %s is not ready", mWs2812Device->name);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    UpdateRgbLight();

    return CHIP_NO_ERROR;
}

void WS2812Device::UpdateRgbLight()
{
    int status;
    led_rgb setRgb = { 0 };

    if (mState == kRgbState_On)
    {
        setRgb.r = mLedRgb.r;
        setRgb.g = mLedRgb.g;
        setRgb.b = mLedRgb.b;
    }

    status = led_strip_update_rgb(mWs2812Device, &setRgb, mChainLength);
    if (status)
    {
        LOG_ERR("Couldn't update strip: %d", status);
    }
}

void WS2812Device::SetLevel(RgbColor_t * pRgb)
{
    if (pRgb != NULL)
    {
        memcpy(&mLedRgb, pRgb, sizeof(RgbColor_t));
    }

    UpdateRgbLight();
}

void WS2812Device::Set(bool aTurnOn)
{
    mState = aTurnOn ? kRgbState_On : kRgbState_Off;
    UpdateRgbLight();
}

bool WS2812Device::IsTurnedOn()
{
    return mState == kRgbState_On;
}
