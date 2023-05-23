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

#include "ICDUtil.h"

ICDUtil ICDUtil::sICDUtil;

CHIP_ERROR ICDUtil::OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler, chip::Transport::SecureSession & aSecureSession)
{
    uint16_t agreedMaxInterval    = CONFIG_CHIP_MAX_PREFERRED_SUBSCRIPTION_REPORT_INTERVAL;
    uint16_t requestedMinInterval = 0;
    uint16_t requestedMaxInterval = 0;
    aReadHandler.GetReportingIntervals(requestedMinInterval, requestedMaxInterval);

    if (requestedMaxInterval > agreedMaxInterval)
    {
        agreedMaxInterval = requestedMaxInterval;
    }
    else if (agreedMaxInterval > kSubscriptionMaxIntervalPublisherLimit)
    {
        agreedMaxInterval = kSubscriptionMaxIntervalPublisherLimit;
    }

    return aReadHandler.SetReportingIntervals(agreedMaxInterval);
}
