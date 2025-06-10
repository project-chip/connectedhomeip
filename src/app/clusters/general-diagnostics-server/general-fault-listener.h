/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <clusters/GeneralDiagnostics/Events.h>
#include <clusters/GeneralFaultListenerConfig.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {

/**
 * @brief A class that listens to General daignostic reboot and fault detection
 */
class GeneralFaultListener
{
public:
    GeneralFaultListener()          = default;
    virtual ~GeneralFaultListener() = default;

#if ENABLE_GENERAL_DIAGNOSTICS_ON_DEVICE_REBOOT
    /**
      * @brief
      *   Called after the current device is rebooted.
      */
    virtual void OnDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason) = 0;

    static void GlobalNotifyDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason)
    {
        if (GeneralFaultListener * listener = GetGlobalListener(); listener != nullptr)
        {
            listener->OnDeviceReboot(bootReason);
        }
    }
#endif

#if ENABLE_GENERAL_DIAGNOSTICS_ON_HARDWARE_FAULTS_DETECT
    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    virtual void OnHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                        const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current) = 0;

    static void GlobalNotifyHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                                 const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current)
    {
        if (GeneralFaultListener * listener = GetGlobalListener(); listener != nullptr)
        {
            listener->OnHardwareFaultsDetect(previous, current);
        }
    }
#endif

#if ENABLE_GENERAL_DIAGNOSTICS_ON_RADIO_FAULTS_DETECT
    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    virtual void OnRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                                     const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current) = 0;

    static void GlobalNotifyRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                                              const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current)
    {
        if (GeneralFaultListener * listener = GetGlobalListener(); listener != nullptr)
        {
            listener->OnRadioFaultsDetect(previous, current);
        }
    }
#endif

#if ENABLE_GENERAL_DIAGNOSTICS_ON_NETWORK_FAULTS_DETECT
    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    virtual void OnNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                                       const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current) = 0;

    static void GlobalNotifyNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                                                const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current)
    {
        if (GeneralFaultListener * listener = GetGlobalListener(); listener != nullptr)
        {
            listener->OnNetworkFaultsDetect(previous, current);
        }
    }
#endif

    /**
     * @brief
     *   Returns the set global general fault listener. If there isn't one set, it may return nullptr
     */
    static GeneralFaultListener * GetGlobalListener();

    /**
     * @brief
     *   Sets the global general fault listener.
     */
    static void SetGlobalListener(GeneralFaultListener * newValue);
};

} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
