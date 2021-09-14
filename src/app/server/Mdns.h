/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/mdns/Advertiser.h>
#include <platform/CHIPDeviceLayer.h>
#include <stddef.h>
#include <system/TimeSource.h>

namespace chip {
namespace app {

#define TIMEOUT_CLEARED 0
class DLL_EXPORT MdnsServer
{
public:
    /// Provides the system-wide implementation of the service advertiser
    static MdnsServer & Instance()
    {
        static MdnsServer instance;
        return instance;
    }

    /// Sets the secure Matter port
    void SetSecuredPort(uint16_t port) { mSecuredPort = port; }

    /// Gets the secure Matter port
    uint16_t GetSecuredPort() { return mSecuredPort; }

    /// Sets the unsecure Matter port
    void SetUnsecuredPort(uint16_t port) { mUnsecuredPort = port; }

    /// Gets the unsecure Matter port
    uint16_t GetUnsecuredPort() { return mUnsecuredPort; }

    /// Sets the factory-new state commissionable node discovery timeout
    void SetDiscoveryTimeoutSecs(int16_t secs) { mDiscoveryTimeoutSecs = secs; }

    /// Gets the factory-new state commissionable node discovery timeout
    int16_t GetDiscoveryTimeoutSecs() { return mDiscoveryTimeoutSecs; }

    /// Callback from Discovery Expiration timer
    /// Checks if discovery has expired and if so,
    /// kicks off extend discovery (when enabled)
    /// otherwise, stops commissionable node advertising
    /// Discovery Expiration refers here to commissionable node advertising when in commissioning mode
    void OnDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    /// Sets the extended discovery timeout. Value will be persisted across reboots
    void SetExtendedDiscoveryTimeoutSecs(int16_t secs);

    /// Callback from Extended Discovery Expiration timer
    /// Checks if extended discovery has expired and if so,
    /// stops commissionable node advertising
    /// Extended Discovery Expiration refers here to commissionable node advertising when NOT in commissioning mode
    void OnExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState);
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

    /// Start operational advertising
    CHIP_ERROR AdvertiseOperational();

    /// (Re-)starts the minmdns server
    /// - if device has not yet been commissioned, then commissioning mode will show as enabled (CM=1, AC=0)
    /// - if device has been commissioned, then commissioning mode will reflect the state of mode argument
    void StartServer(chip::Mdns::CommissioningMode mode = chip::Mdns::CommissioningMode::kDisabled);

    CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize);

    /// Generates the (random) instance name that a CHIP device is to use for pre-commissioning DNS-SD
    CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen);

private:
    /// Overloaded utility method for commissioner and commissionable advertisement
    /// This method is used for both commissioner discovery and commissionable node discovery since
    /// they share many fields.
    ///   commissionableNode = true : advertise commissionable node
    ///   commissionableNode = false : advertise commissioner
    CHIP_ERROR Advertise(bool commissionableNode, chip::Mdns::CommissioningMode mode);

    /// Set MDNS commissioner advertisement
    CHIP_ERROR AdvertiseCommissioner();

    /// Set MDNS commissionable node advertisement
    CHIP_ERROR AdvertiseCommissionableNode(chip::Mdns::CommissioningMode mode);

    Time::TimeSource<Time::Source::kSystem> mTimeSource;

    void ClearTimeouts()
    {
        mDiscoveryExpirationMs = TIMEOUT_CLEARED;
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
        mExtendedDiscoveryExpirationMs = TIMEOUT_CLEARED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    }

    uint16_t mSecuredPort   = CHIP_PORT;
    uint16_t mUnsecuredPort = CHIP_UDC_PORT;

    /// schedule next discovery expiration
    CHIP_ERROR ScheduleDiscoveryExpiration();
    int16_t mDiscoveryTimeoutSecs   = CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS;
    uint64_t mDiscoveryExpirationMs = TIMEOUT_CLEARED;

    /// return true if expirationMs is valid (not cleared and not in the future)
    bool OnExpiration(uint64_t expirationMs);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    /// get the current extended discovery timeout (from persistent storage)
    int16_t GetExtendedDiscoveryTimeoutSecs();

    /// schedule next extended discovery expiration
    CHIP_ERROR ScheduleExtendedDiscoveryExpiration();

    uint64_t mExtendedDiscoveryExpirationMs = TIMEOUT_CLEARED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
};

} // namespace app
} // namespace chip
