/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "LedOnOff.h"
#include "app.h"

#include <system/SystemClock.h>

namespace chip::NXP::App {

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))

void LedOnOff::Init(uint8_t index, bool inverted)
{
    mLastChangeTimeMS = 0;
    mOnTimeMS         = 0;
    mOffTimeMS        = 0;
    mIndex            = index;
    mState            = false;
    mOnLogic          = !inverted;

    Set(false);
}

void LedOnOff::Set(uint8_t level)
{
    mLastChangeTimeMS = mOnTimeMS = mOffTimeMS = 0;
    DoSet(level != 0);
}

void LedOnOff::Animate(uint32_t onTimeMS, uint32_t offTimeMS)
{
    if (onTimeMS && offTimeMS)
    {
        mOnTimeMS  = onTimeMS;
        mOffTimeMS = offTimeMS;
    }

    if (mOnTimeMS && mOffTimeMS)
    {
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = mState ? mOnTimeMS : mOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            DoSet(!mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LedOnOff::DoSet(bool state)
{
    mState = state;

    if (state)
    {
        (void) LED_TurnOnOff((led_handle_t) g_ledHandle[mIndex], mOnLogic);
    }
    else
    {
        (void) LED_TurnOnOff((led_handle_t) g_ledHandle[mIndex], !mOnLogic);
    }
}

#endif /* (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)) */

} // namespace chip::NXP::App
