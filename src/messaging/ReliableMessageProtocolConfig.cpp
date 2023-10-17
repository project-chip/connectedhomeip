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

#include <messaging/ReliableMessageMgr.h>

#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/ICDManagementServer.h> // nogncheck
#include <app/icd/ICDManager.h>          // nogncheck
#endif

namespace chip {

using namespace System::Clock::Literals;

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
static Optional<System::Clock::Timeout> idleRetransTimeoutOverride   = NullOptional;
static Optional<System::Clock::Timeout> activeRetransTimeoutOverride = NullOptional;
static Optional<System::Clock::Timeout> activeThresholdTimeOverride  = NullOptional;

void OverrideLocalMRPConfig(System::Clock::Timeout idleRetransTimeout, System::Clock::Timeout activeRetransTimeout,
                            System::Clock::Timeout activeThresholdTime)
{
    idleRetransTimeoutOverride.SetValue(idleRetransTimeout);
    activeRetransTimeoutOverride.SetValue(activeRetransTimeout);
    activeThresholdTimeOverride.SetValue(activeThresholdTime);
}

void ClearLocalMRPConfigOverride()
{
    activeRetransTimeoutOverride.ClearValue();
    idleRetransTimeoutOverride.ClearValue();
    activeThresholdTimeOverride.ClearValue();
}
#endif

ReliableMessageProtocolConfig GetDefaultMRPConfig()
{
    // Default MRP intervals are defined in spec <2.11.3. Parameters and Constants>
    static constexpr const System::Clock::Milliseconds32 idleRetransTimeout   = 300_ms32;
    static constexpr const System::Clock::Milliseconds32 activeRetransTimeout = 300_ms32;
    static constexpr const System::Clock::Milliseconds16 activeThresholdTime  = 4000_ms16;
    return ReliableMessageProtocolConfig(idleRetransTimeout, activeRetransTimeout, activeThresholdTime);
}

Optional<ReliableMessageProtocolConfig> GetLocalMRPConfig()
{
    ReliableMessageProtocolConfig config(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL, CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL);
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // TODO ICD LIT shall not advertise the SII key
    // Increase local MRP retry intervals by ICD polling intervals. That is, intervals for
    // which the device can be at sleep and not be able to receive any messages).
    config.mIdleRetransTimeout += app::ICDManager::GetSlowPollingInterval();
    config.mActiveRetransTimeout += app::ICDManager::GetFastPollingInterval();
    config.mActiveThresholdTime = System::Clock::Milliseconds16(ICDManagementServer::GetInstance().GetActiveModeThresholdMs());
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

    if (activeThresholdTimeOverride.HasValue())
    {
        config.mActiveThresholdTime = activeRetransTimeoutOverride.Value();
    }
#endif

    return (config == GetDefaultMRPConfig()) ? Optional<ReliableMessageProtocolConfig>::Missing()
                                             : Optional<ReliableMessageProtocolConfig>::Value(config);
}

System::Clock::Timestamp GetRetransmissionTimeout(System::Clock::Timestamp activeInterval, System::Clock::Timestamp idleInterval,
                                                  System::Clock::Timestamp lastActivityTime,
                                                  System::Clock::Timestamp activityThreshold)
{
    auto timeSinceLastActivity = (System::SystemClock().GetMonotonicTimestamp() - lastActivityTime);

    // Calculate the retransmission timeout and take into account that an active/idle state change can happen
    // in the middle.
    System::Clock::Timestamp timeout(0);
    for (uint8_t i = 0; i < CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS + 1; i++)
    {
        auto baseInterval = ((timeSinceLastActivity + timeout) < activityThreshold) ? activeInterval : idleInterval;
        timeout += Messaging::ReliableMessageMgr::GetBackoff(baseInterval, i, /* computeMaxPossible */ true);
    }

    return timeout;
}

} // namespace chip
