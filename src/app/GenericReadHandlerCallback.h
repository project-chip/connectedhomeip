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
#pragma once

#include "ReadHandler.h"

namespace chip {
namespace app {

class GenericReadHandlerCallback : public ReadHandler::ApplicationCallback
{
     CHIP_ERROR OnSubscriptionRequested(ReadHandler & aReadHandler, Transport::SecureSession & aSecureSession)  override
     {
        uint16_t interval_u16 = 0;
        uint32_t interval_u32 = CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL.count() / 1000; 

        // Avoid overflow when converting to uint16_t
        interval_u32 = interval_u32 > std::numeric_limits<std::uint16_t>::max() ? std::numeric_limits<std::uint16_t>::max() : interval_u32;

        interval_u16 = static_cast<uint16_t>(interval_u32);
        interval_u16 = interval_u16 > kSubscriptionMaxIntervalPublisherLimit ? kSubscriptionMaxIntervalPublisherLimit : interval_u16;

        aReadHandler.SetReportingIntervals(interval_u16);

        return CHIP_NO_ERROR;
    }
};

} // app
} // chip