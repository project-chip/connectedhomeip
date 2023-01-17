/*
 *
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

#include "ICDSubscribtionCallback.h"
#include <platform/CHIPDeviceConfig.h>

CHIP_ERROR ICDSubscribtionCallback::OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler,
                                                            chip::Transport::SecureSession & aSecureSession)
{
    using namespace chip::System::Clock;

    Seconds32 interval_s32 = std::chrono::duration_cast<Seconds32>(CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL);

    if (interval_s32 > Seconds16::max())
    {
        interval_s32 = Seconds16::max();
    }

    Seconds16 interval_s16 = interval_s32;

    return aReadHandler.SetReportingIntervals(interval_s16.count());
}
