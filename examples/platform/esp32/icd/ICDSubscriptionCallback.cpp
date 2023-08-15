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

#include "ICDSubscriptionCallback.h"
#include <platform/CHIPDeviceConfig.h>

CHIP_ERROR ICDSubscriptionCallback::OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler,
                                                            chip::Transport::SecureSession & aSecureSession)
{
    using namespace chip::System::Clock;

    Seconds32 interval_s32 = std::chrono::duration_cast<Seconds32>(CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL);

    if (interval_s32 > Seconds16::max())
    {
        interval_s32 = Seconds16::max();
    }
    uint32_t decidedMaxInterval = interval_s32.count();

    uint16_t requestedMinInterval = 0;
    uint16_t requestedMaxInterval = 0;
    aReadHandler.GetReportingIntervals(requestedMinInterval, requestedMaxInterval);

    // If requestedMinInterval is greater than IdleTimeInterval, select next wake up time as max interval
    if (requestedMinInterval > decidedMaxInterval)
    {
        uint16_t ratio = requestedMinInterval / decidedMaxInterval;
        if (requestedMinInterval % decidedMaxInterval)
        {
            ratio++;
        }

        decidedMaxInterval *= ratio;
    }

    // Verify that decidedMaxInterval is an acceptable value
    if (decidedMaxInterval > Seconds16::max().count())
    {
        decidedMaxInterval = Seconds16::max().count();
    }

    // Verify that the decidedMaxInterval respects MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)
    uint16_t maximumMaxInterval = std::max(kSubscriptionMaxIntervalPublisherLimit, requestedMaxInterval);
    if (decidedMaxInterval > maximumMaxInterval)
    {
        decidedMaxInterval = maximumMaxInterval;
    }

    return aReadHandler.SetMaxReportingInterval(decidedMaxInterval);
}
