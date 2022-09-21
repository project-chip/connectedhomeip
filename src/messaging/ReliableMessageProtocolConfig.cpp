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

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
static Optional<System::Clock::Timeout> idleRetransTimeoutOverride   = NullOptional;
static Optional<System::Clock::Timeout> activeRetransTimeoutOverride = NullOptional;

void OverrideLocalMRPConfig(System::Clock::Timeout idleRetransTimeout, System::Clock::Timeout activeRetransTimeout)
{
    idleRetransTimeoutOverride.SetValue(idleRetransTimeout);
    activeRetransTimeoutOverride.SetValue(activeRetransTimeout);
}

void ClearLocalMRPConfigOverride()
{
    activeRetransTimeoutOverride.ClearValue();
    idleRetransTimeoutOverride.ClearValue();
}
#endif

ReliableMessageProtocolConfig GetDefaultMRPConfig()
{
    // Default MRP intervals are defined in spec <2.11.3. Parameters and Constants>
    static constexpr const System::Clock::Milliseconds32 idleRetransTimeout   = 4000_ms32;
    static constexpr const System::Clock::Milliseconds32 activeRetransTimeout = 300_ms32;
    return ReliableMessageProtocolConfig(idleRetransTimeout, activeRetransTimeout);
}

Optional<ReliableMessageProtocolConfig> GetLocalMRPConfig()
{
    ReliableMessageProtocolConfig config(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL, CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL);

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    DeviceLayer::ConnectivityManager::SEDIntervalsConfig sedIntervalsConfig;

    if (DeviceLayer::ConnectivityMgr().GetSEDIntervalsConfig(sedIntervalsConfig) == CHIP_NO_ERROR)
    {
        // Increase local MRP retry intervals by SED intervals. That is, intervals for
        // which the device can be at sleep and not be able to receive any messages).
        config.mIdleRetransTimeout += sedIntervalsConfig.IdleIntervalMS;
        config.mActiveRetransTimeout += sedIntervalsConfig.ActiveIntervalMS;
    }
#endif

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (idleRetransTimeoutOverride.HasValue())
    {
        config.mIdleRetransTimeout = idleRetransTimeoutOverride.Value();
    }

    if (activeRetransTimeoutOverride.HasValue())
    {
        config.mActiveRetransTimeout = activeRetransTimeoutOverride.Value();
    }
#endif

    return (config == GetDefaultMRPConfig()) ? Optional<ReliableMessageProtocolConfig>::Missing()
                                             : Optional<ReliableMessageProtocolConfig>::Value(config);
}

} // namespace chip
