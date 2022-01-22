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
#include <lib/core/Optional.h>
#include <lib/dnssd/Advertiser.h>
#include <platform/CHIPDeviceLayer.h>
#include <stddef.h>
#include <system/TimeSource.h>

namespace chip {
namespace app {

class DLL_EXPORT DnssdServer
{
public:
    static constexpr System::Clock::Timestamp kTimeoutCleared = System::Clock::kZero;

    /// Provides the system-wide implementation of the service advertiser
    static DnssdServer & Instance()
    {
        static DnssdServer instance;
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

    /// (Re-)starts the Dnssd server
    /// - if device has not yet been commissioned, then commissioning mode will show as enabled (CM=1)
    /// - if device has been commissioned, then commissioning mode will be disabled.
    void StartServer();

    /// (Re-)starts the Dnssd server, using the provided commissioning mode.
    void StartServer(Dnssd::CommissioningMode mode);

    CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize);

    /// Generates the (random) instance name that a CHIP device is to use for pre-commissioning DNS-SD
    CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen);

private:
    /// Overloaded utility method for commissioner and commissionable advertisement
    /// This method is used for both commissioner discovery and commissionable node discovery since
    /// they share many fields.
    ///   commissionableNode = true : advertise commissionable node
    ///   commissionableNode = false : advertise commissioner
    CHIP_ERROR Advertise(bool commissionableNode, chip::Dnssd::CommissioningMode mode);

    /// Set MDNS commissioner advertisement
    CHIP_ERROR AdvertiseCommissioner();

    /// Set MDNS commissionable node advertisement
    CHIP_ERROR AdvertiseCommissionableNode(chip::Dnssd::CommissioningMode mode);

    Time::TimeSource<Time::Source::kSystem> mTimeSource;

    void ClearTimeouts()
    {
        mDiscoveryExpiration = kTimeoutCleared;
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
        mExtendedDiscoveryExpiration = kTimeoutCleared;
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    }

    // Helper for StartServer.
    void StartServer(Optional<Dnssd::CommissioningMode> mode);

    uint16_t mSecuredPort   = CHIP_PORT;
    uint16_t mUnsecuredPort = CHIP_UDC_PORT;

    /// schedule next discovery expiration
    CHIP_ERROR ScheduleDiscoveryExpiration();
    int16_t mDiscoveryTimeoutSecs                 = CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS;
    System::Clock::Timestamp mDiscoveryExpiration = kTimeoutCleared;

    /// return true if expirationMs is valid (not cleared and not in the future)
    bool OnExpiration(System::Clock::Timestamp expiration);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    /// get the current extended discovery timeout (from persistent storage)
    int16_t GetExtendedDiscoveryTimeoutSecs();

    /// schedule next extended discovery expiration
    CHIP_ERROR ScheduleExtendedDiscoveryExpiration();

    System::Clock::Timestamp mExtendedDiscoveryExpiration = kTimeoutCleared;
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
};

} // namespace app
} // namespace chip
