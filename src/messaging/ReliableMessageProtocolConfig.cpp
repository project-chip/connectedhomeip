/*
 *    Copyright (c) 2021 Project CHIP Authors
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
 *    @file
 *      This file defines the configuration parameters that are required
 *      for the CHIP Reliable Messaging Protocol.
 *
 */

#include <messaging/ReliableMessageProtocolConfig.h>

#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

namespace chip {

using namespace System::Clock::Literals;

ReliableMessageProtocolConfig GetLocalMRPConfig()
{
    ReliableMessageProtocolConfig config(CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL,
                                         CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL);

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    DeviceLayer::ConnectivityManager::SEDIntervalsConfig sedIntervalsConfig;

    if (DeviceLayer::ConnectivityMgr().GetSEDIntervalsConfig(sedIntervalsConfig) == CHIP_NO_ERROR)
    {
        // Increase default MRP retry intervals by SED intervals. That is, intervals for
        // which the device can be at sleep and not be able to receive any messages).
        config.mIdleRetransTimeout += sedIntervalsConfig.IdleIntervalMS;
        config.mActiveRetransTimeout += sedIntervalsConfig.ActiveIntervalMS;
    }
#endif

    return config;
}

} // namespace chip
