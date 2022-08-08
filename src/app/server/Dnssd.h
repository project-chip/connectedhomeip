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

#include <app/server/CommissioningModeProvider.h>
#include <credentials/FabricTable.h>
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
    uint16_t GetSecuredPort() const { return mSecuredPort; }

    /// Sets the unsecure Matter port
    void SetUnsecuredPort(uint16_t port) { mUnsecuredPort = port; }

    /// Gets the unsecure Matter port
    uint16_t GetUnsecuredPort() const { return mUnsecuredPort; }

    /// Sets the interface id used for advertising
    void SetInterfaceId(Inet::InterfaceId interfaceId) { mInterfaceId = interfaceId; }

    /// Gets the interface id used for advertising
    Inet::InterfaceId GetInterfaceId() { return mInterfaceId; }

    //
    // Override the referenced fabric table from the default that is present
    // in Server::GetInstance().GetFabricTable() to something else.
    //
    void SetFabricTable(FabricTable * table)
    {
        VerifyOrDie(table != nullptr);
        mFabricTable = table;
    }

    // Set the commissioning mode provider to use.  Null provider will mean we
    // assume the commissioning mode is kDisabled.
    void SetCommissioningModeProvider(CommissioningModeProvider * provider) { mCommissioningModeProvider = provider; }

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    /// Sets the extended discovery timeout. Value will be persisted across reboots
    void SetExtendedDiscoveryTimeoutSecs(int32_t secs);

    /// Callback from Extended Discovery Expiration timer
    /// Checks if extended discovery has expired and if so,
    /// stops commissionable node advertising
    /// Extended Discovery Expiration refers here to commissionable node advertising when NOT in commissioning mode
    void OnExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState);
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

    /// Start operational advertising
    CHIP_ERROR AdvertiseOperational();

    /// (Re-)starts the Dnssd server, using the commissioning mode from our
    /// commissioning mode provider.
    void StartServer();

    /// (Re-)starts the Dnssd server, using the provided commissioning mode.
    void StartServer(Dnssd::CommissioningMode mode);

    CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize);

    /// Generates the (random) instance name that a CHIP device is to use for pre-commissioning DNS-SD
    CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen);

    /**
     * @brief Overrides configuration so that commissionable advertisement will use an
     *        ephemeral discriminator such as one set for ECM. If the Optional has no
     *        value, the default basic discriminator is used as usual.
     *
     * @param[in] discriminator Ephemeral discriminator to override if it HasValue(), otherwise reverts
     *                          to default.
     * @return CHIP_NO_ERROR on success or CHIP_ERROR_INVALID_ARGUMENT on invalid value
     */
    CHIP_ERROR SetEphemeralDiscriminator(Optional<uint16_t> discriminator);

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

    //
    // Check if we have any valid operational credentials present in the fabric table and return true
    // if we do.
    //
    bool HaveOperationalCredentials();

    Time::TimeSource<Time::Source::kSystem> mTimeSource;

    FabricTable * mFabricTable                             = nullptr;
    CommissioningModeProvider * mCommissioningModeProvider = nullptr;

    uint16_t mSecuredPort          = CHIP_PORT;
    uint16_t mUnsecuredPort        = CHIP_UDC_PORT;
    Inet::InterfaceId mInterfaceId = Inet::InterfaceId::Null();

    // Ephemeral discriminator to use instead of the default if set
    Optional<uint16_t> mEphemeralDiscriminator;

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    /// Get the current extended discovery timeout (set by
    /// SetExtendedDiscoveryTimeoutSecs, or the configuration default if not set).
    int32_t GetExtendedDiscoveryTimeoutSecs();

    /// schedule next extended discovery expiration
    CHIP_ERROR ScheduleExtendedDiscoveryExpiration();

    // mExtendedDiscoveryExpiration, if not set to kTimeoutCleared, is used to
    // indicate that we should be advertising extended discovery right now.
    System::Clock::Timestamp mExtendedDiscoveryExpiration = kTimeoutCleared;
    Optional<int32_t> mExtendedDiscoveryTimeoutSecs       = NullOptional;

    // The commissioning mode we are advertising right now.  Used to detect when
    // we need to start extended discovery advertisement.  We start this off as
    // kEnabledBasic, so that when we first start up we do extended discovery
    // advertisement if we don't enter commissioning mode.
    Dnssd::CommissioningMode mCurrentCommissioningMode = Dnssd::CommissioningMode::kEnabledBasic;
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
};

} // namespace app
} // namespace chip
