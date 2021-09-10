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

#include <app/server/Mdns.h>

#include <inttypes.h>

#include <core/Optional.h>
#include <mdns/Advertiser.h>
#include <mdns/ServiceNaming.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <support/Span.h>
#include <support/logging/CHIPLogging.h>
#include <system/TimeSource.h>
#include <transport/FabricTable.h>

#include <app/server/Server.h>

namespace chip {
namespace app {
namespace Mdns {

namespace {

bool HaveOperationalCredentials()
{
    // Look for any fabric info that has a useful operational identity.
    for (const Transport::FabricInfo & fabricInfo : GetGlobalFabricTable())
    {
        if (fabricInfo.IsInitialized())
        {
            return true;
        }
    }

    ChipLogProgress(Discovery, "Failed to find a valid admin pairing. Node ID unknown");
    return false;
}

// Requires an 8-byte mac to accommodate thread.
chip::ByteSpan FillMAC(uint8_t (&mac)[8])
{
    memset(mac, 0, 8);
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (chip::DeviceLayer::ThreadStackMgr().GetPrimary802154MACAddress(mac) == CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Using Thread extended MAC for hostname.");
        return chip::ByteSpan(mac, 8);
    }
#endif
    if (DeviceLayer::ConfigurationMgr().GetPrimaryWiFiMACAddress(mac) == CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Using wifi MAC for hostname");
        return chip::ByteSpan(mac, 6);
    }
    ChipLogError(Discovery, "Wifi mac not known. Using a default.");
    uint8_t temp[6] = { 0xEE, 0xAA, 0xBA, 0xDA, 0xBA, 0xD0 };
    memcpy(mac, temp, 6);
    return chip::ByteSpan(mac, 6);
}

} // namespace

uint16_t gSecuredPort   = CHIP_PORT;
uint16_t gUnsecuredPort = CHIP_UDC_PORT;

void SetSecuredPort(uint16_t port)
{
    gSecuredPort = port;
}

uint16_t GetSecuredPort()
{
    return gSecuredPort;
}

void SetUnsecuredPort(uint16_t port)
{
    gUnsecuredPort = port;
}

uint16_t GetUnsecuredPort()
{
    return gUnsecuredPort;
}

#define TIMEOUT_CLEARED 0
Time::TimeSource<Time::Source::kSystem> gTimeSource;

int16_t gDiscoveryTimeoutSecs   = CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS;
uint64_t gDiscoveryExpirationMs = TIMEOUT_CLEARED;

void SetDiscoveryTimeoutSecs(int16_t secs)
{
    gDiscoveryTimeoutSecs = secs;
}

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

constexpr const char kExtendedDiscoveryTimeoutKeypairStorage[] = "ExtDiscKey";

uint64_t gExtendedDiscoveryExpirationMs = TIMEOUT_CLEARED;

void SetExtendedDiscoveryTimeoutSecs(int16_t secs)
{
    ChipLogDetail(Discovery, "SetExtendedDiscoveryTimeoutSecs %d", secs);
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kExtendedDiscoveryTimeoutKeypairStorage, &secs, sizeof(secs));
}

int16_t GetExtendedDiscoveryTimeoutSecs()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int16_t secs;

    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kExtendedDiscoveryTimeoutKeypairStorage, &secs, sizeof(secs));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to get extended timeout configuration err: %s", chip::ErrorStr(err));
        secs = CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS;
    }

    ChipLogDetail(Discovery, "GetExtendedDiscoveryTimeoutSecs %d", secs);
    return secs;
}

CHIP_ERROR ScheduleExtendedDiscoveryExpiration();
CHIP_ERROR AdvertiseCommissioner();
CHIP_ERROR AdvertiseCommissionableNode(chip::Mdns::CommissioningMode mode);

void HandleExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    if (gExtendedDiscoveryExpirationMs == TIMEOUT_CLEARED)
    {
        ChipLogDetail(Discovery, "HandleExtendedDiscoveryTimeout callback for cleared session");
        return;
    }
    uint64_t now = gTimeSource.GetCurrentMonotonicTimeMs();
    if (gExtendedDiscoveryExpirationMs > now)
    {
        ChipLogDetail(Discovery, "HandleExtendedDiscoveryTimeout callback for reset session");
        return;
    }

    // reset advertising
    CHIP_ERROR err;
    err = chip::Mdns::ServiceAdvertiser::Instance().StopPublishDevice();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to stop ServiceAdvertiser: %s", chip::ErrorStr(err));
    }
    err = chip::Mdns::ServiceAdvertiser::Instance().Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to start ServiceAdvertiser: %s", chip::ErrorStr(err));
    }

    ChipLogDetail(Discovery, "Extended discovery time out");

    // restart operational (if needed)
    err = app::Mdns::AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise operational node: %s", chip::ErrorStr(err));
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = app::Mdns::AdvertiseCommissioner();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise commissioner: %s", chip::ErrorStr(err));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    gExtendedDiscoveryExpirationMs = TIMEOUT_CLEARED;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

void HandleDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    if (gDiscoveryExpirationMs == TIMEOUT_CLEARED)
    {
        ChipLogDetail(Discovery, "HandleDiscoveryTimeout callback for cleared session");
        return;
    }
    uint64_t now = gTimeSource.GetCurrentMonotonicTimeMs();
    if (gDiscoveryExpirationMs > now)
    {
        ChipLogDetail(Discovery, "HandleDiscoveryTimeout callback for reset session");
        return;
    }

    // reset advertising
    CHIP_ERROR err;
    err = chip::Mdns::ServiceAdvertiser::Instance().StopPublishDevice();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to stop ServiceAdvertiser: %s", chip::ErrorStr(err));
    }
    err = chip::Mdns::ServiceAdvertiser::Instance().Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to start ServiceAdvertiser: %s", chip::ErrorStr(err));
    }

    ChipLogDetail(Discovery, "Discovery time out");

    // restart operational (if needed)
    err = app::Mdns::AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise operational node: %s", chip::ErrorStr(err));
    }

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    int16_t extTimeout = GetExtendedDiscoveryTimeoutSecs();
    if (extTimeout != CHIP_DEVICE_CONFIG_DISCOVERY_DISABLED)
    {
        err = app::Mdns::AdvertiseCommissionableNode(chip::Mdns::CommissioningMode::kDisabled);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise extended commissionable node: %s", chip::ErrorStr(err));
        }
        // set timeout
        ScheduleExtendedDiscoveryExpiration();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = app::Mdns::AdvertiseCommissioner();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise commissioner: %s", chip::ErrorStr(err));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    gDiscoveryExpirationMs = TIMEOUT_CLEARED;
}

void ClearTimeouts()
{
    gDiscoveryExpirationMs = TIMEOUT_CLEARED;
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    gExtendedDiscoveryExpirationMs = TIMEOUT_CLEARED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
}

CHIP_ERROR ScheduleDiscoveryExpiration()
{
    if (gDiscoveryTimeoutSecs == CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT)
    {
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(Discovery, "Scheduling Discovery timeout in secs=%d", gDiscoveryTimeoutSecs);

    gDiscoveryExpirationMs = gTimeSource.GetCurrentMonotonicTimeMs() + static_cast<uint64_t>(gDiscoveryTimeoutSecs) * 1000;

    ReturnErrorOnFailure(DeviceLayer::SystemLayer.StartTimer(static_cast<uint32_t>(gDiscoveryTimeoutSecs) * 1000,
                                                             HandleDiscoveryExpiration, nullptr));

    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
CHIP_ERROR ScheduleExtendedDiscoveryExpiration()
{
    int16_t extendedDiscoveryTimeoutSecs = GetExtendedDiscoveryTimeoutSecs();
    if (extendedDiscoveryTimeoutSecs == CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT)
    {
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(Discovery, "Scheduling Extended Discovery timeout in secs=%d", extendedDiscoveryTimeoutSecs);

    gExtendedDiscoveryExpirationMs =
        gTimeSource.GetCurrentMonotonicTimeMs() + static_cast<uint64_t>(extendedDiscoveryTimeoutSecs) * 1000;

    ReturnErrorOnFailure(DeviceLayer::SystemLayer.StartTimer(static_cast<uint32_t>(extendedDiscoveryTimeoutSecs) * 1000,
                                                             HandleExtendedDiscoveryExpiration, nullptr));

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen)
{
    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();
    return mdnsAdvertiser.GetCommissionableInstanceName(buffer, bufferLen);
}

/// Set MDNS operational advertisement
CHIP_ERROR AdvertiseOperational()
{
    for (const Transport::FabricInfo & fabricInfo : GetGlobalFabricTable())
    {
        if (fabricInfo.IsInitialized())
        {
            uint8_t mac[8];

            const auto advertiseParameters = chip::Mdns::OperationalAdvertisingParameters()
                                                 .SetPeerId(fabricInfo.GetPeerId())
                                                 .SetMac(FillMAC(mac))
                                                 .SetPort(GetSecuredPort())
                                                 .SetMRPRetryIntervals(CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL,
                                                                       CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL)
                                                 .EnableIpV4(true);

            auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();

            ChipLogProgress(Discovery, "Advertise operational node " ChipLogFormatX64 "-" ChipLogFormatX64,
                            ChipLogValueX64(advertiseParameters.GetPeerId().GetCompressedFabricId()),
                            ChipLogValueX64(advertiseParameters.GetPeerId().GetNodeId()));
            // Should we keep trying to advertise the other operational
            // identities on failure?
            ReturnErrorOnFailure(mdnsAdvertiser.Advertise(advertiseParameters));
        }
        else
        {
            ChipLogProgress(Discovery, "Advertise operational node - fabric not initialized");
        }
    }
    return CHIP_NO_ERROR;
}

/// Overloaded utility method for commissioner and commissionable advertisement
/// This method is used for both commissioner discovery and commissionable node discovery since
/// they share many fields.
///   commissionableNode = true : advertise commissionable node
///   commissionableNode = false : advertise commissioner
CHIP_ERROR Advertise(bool commissionableNode, chip::Mdns::CommissioningMode mode)
{
    auto advertiseParameters = chip::Mdns::CommissionAdvertisingParameters()
                                   .SetPort(commissionableNode ? GetSecuredPort() : GetUnsecuredPort())
                                   .EnableIpV4(true);
    advertiseParameters.SetCommissionAdvertiseMode(commissionableNode ? chip::Mdns::CommssionAdvertiseMode::kCommissionableNode
                                                                      : chip::Mdns::CommssionAdvertiseMode::kCommissioner);

    advertiseParameters.SetCommissioningMode(mode);

    char pairingInst[chip::Mdns::kKeyPairingInstructionMaxLength + 1];

    uint8_t mac[8];
    advertiseParameters.SetMac(FillMAC(mac));

    uint16_t value;
    if (DeviceLayer::ConfigurationMgr().GetVendorId(value) != CHIP_NO_ERROR)
    {
        ChipLogProgress(Discovery, "Vendor ID not known");
    }
    else
    {
        advertiseParameters.SetVendorId(chip::Optional<uint16_t>::Value(value));
    }

    if (DeviceLayer::ConfigurationMgr().GetProductId(value) != CHIP_NO_ERROR)
    {
        ChipLogProgress(Discovery, "Product ID not known");
    }
    else
    {
        advertiseParameters.SetProductId(chip::Optional<uint16_t>::Value(value));
    }

    if (DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(value) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Setup discriminator not known. Using a default.");
        value = 840;
    }
    advertiseParameters.SetShortDiscriminator(static_cast<uint8_t>(value & 0xFF)).SetLongDiscriminator(value);

    if (DeviceLayer::ConfigurationMgr().IsCommissionableDeviceTypeEnabled() &&
        DeviceLayer::ConfigurationMgr().GetDeviceType(value) == CHIP_NO_ERROR)
    {
        advertiseParameters.SetDeviceType(chip::Optional<uint16_t>::Value(value));
    }

    char deviceName[chip::Mdns::kKeyDeviceNameMaxLength + 1];
    if (DeviceLayer::ConfigurationMgr().IsCommissionableDeviceNameEnabled() &&
        DeviceLayer::ConfigurationMgr().GetDeviceName(deviceName, sizeof(deviceName)) == CHIP_NO_ERROR)
    {
        advertiseParameters.SetDeviceName(chip::Optional<const char *>::Value(deviceName));
    }

#if CHIP_ENABLE_ROTATING_DEVICE_ID
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    ReturnErrorOnFailure(GenerateRotatingDeviceId(rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer)));
    advertiseParameters.SetRotatingId(chip::Optional<const char *>::Value(rotatingDeviceIdHexBuffer));
#endif

    if (mode != chip::Mdns::CommissioningMode::kEnabledEnhanced)
    {
        if (DeviceLayer::ConfigurationMgr().GetInitialPairingHint(value) != CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "DNS-SD Pairing Hint not set");
        }
        else
        {
            advertiseParameters.SetPairingHint(chip::Optional<uint16_t>::Value(value));
        }

        if (DeviceLayer::ConfigurationMgr().GetInitialPairingInstruction(pairingInst, sizeof(pairingInst)) != CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "DNS-SD Pairing Instruction not set");
        }
        else
        {
            advertiseParameters.SetPairingInstr(chip::Optional<const char *>::Value(pairingInst));
        }
    }
    else
    {
        if (DeviceLayer::ConfigurationMgr().GetSecondaryPairingHint(value) != CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "DNS-SD Pairing Hint not set");
        }
        else
        {
            advertiseParameters.SetPairingHint(chip::Optional<uint16_t>::Value(value));
        }

        if (DeviceLayer::ConfigurationMgr().GetSecondaryPairingInstruction(pairingInst, sizeof(pairingInst)) != CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "DNS-SD Pairing Instruction not set");
        }
        else
        {
            advertiseParameters.SetPairingInstr(chip::Optional<const char *>::Value(pairingInst));
        }
    }

    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();

    ChipLogProgress(Discovery, "Advertise commission parameter vendorID=%u productID=%u discriminator=%04u/%02u",
                    advertiseParameters.GetVendorId().ValueOr(0), advertiseParameters.GetProductId().ValueOr(0),
                    advertiseParameters.GetLongDiscriminator(), advertiseParameters.GetShortDiscriminator());
    return mdnsAdvertiser.Advertise(advertiseParameters);
}

/// Set MDNS commissioner advertisement
CHIP_ERROR AdvertiseCommissioner()
{
    return Advertise(false /* commissionableNode */, chip::Mdns::CommissioningMode::kDisabled);
}

/// Set MDNS commissionable node advertisement
CHIP_ERROR AdvertiseCommissionableNode(chip::Mdns::CommissioningMode mode)
{
    return Advertise(true /* commissionableNode */, mode);
}

/// (Re-)starts the minmdns server
/// - if device has not yet been commissioned, then commissioning mode will show as enabled (CM=1, AC=0)
/// - if device has been commissioned, then commissioning mode will reflect the state of mode argument
void StartServer(chip::Mdns::CommissioningMode mode)
{
    ChipLogDetail(Discovery, "Mdns StartServer mode=%d", static_cast<int>(mode));

    ClearTimeouts();

    CHIP_ERROR err;
    err = chip::Mdns::ServiceAdvertiser::Instance().StopPublishDevice();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to stop ServiceAdvertiser: %s", chip::ErrorStr(err));
    }

    err = chip::Mdns::ServiceAdvertiser::Instance().Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to start ServiceAdvertiser: %s", chip::ErrorStr(err));
    }

    err = app::Mdns::AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise operational node: %s", chip::ErrorStr(err));
    }

    if (HaveOperationalCredentials())
    {
        ChipLogError(Discovery, "Have operational credentials");
        if (mode != chip::Mdns::CommissioningMode::kDisabled)
        {
            err = app::Mdns::AdvertiseCommissionableNode(mode);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to advertise commissionable node: %s", chip::ErrorStr(err));
            }
            // no need to set timeout because callers are currently doing that and their timeout might be longer than the default
        }
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
        else if (GetExtendedDiscoveryTimeoutSecs() != CHIP_DEVICE_CONFIG_DISCOVERY_DISABLED)
        {
            err = app::Mdns::AdvertiseCommissionableNode(mode);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to advertise extended commissionable node: %s", chip::ErrorStr(err));
            }
            // set timeout
            ScheduleExtendedDiscoveryExpiration();
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DISCOVERY
        ChipLogProgress(Discovery, "Start dns-sd server - no current nodeId");
        err = app::Mdns::AdvertiseCommissionableNode(chip::Mdns::CommissioningMode::kEnabledBasic);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise unprovisioned commissionable node: %s", chip::ErrorStr(err));
        }
        // set timeout
        ScheduleDiscoveryExpiration();
#endif
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = app::Mdns::AdvertiseCommissioner();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise commissioner: %s", chip::ErrorStr(err));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize)
{
    char serialNumber[chip::DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];
    size_t serialNumberSize                = 0;
    uint16_t lifetimeCounter               = 0;
    size_t rotatingDeviceIdValueOutputSize = 0;

    ReturnErrorOnFailure(
        chip::DeviceLayer::ConfigurationMgr().GetSerialNumber(serialNumber, sizeof(serialNumber), serialNumberSize));
    ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().GetLifetimeCounter(lifetimeCounter));
    return AdditionalDataPayloadGenerator().generateRotatingDeviceId(lifetimeCounter, serialNumber, serialNumberSize,
                                                                     rotatingDeviceIdHexBuffer, rotatingDeviceIdHexBufferSize,
                                                                     rotatingDeviceIdValueOutputSize);
}
#endif

} // namespace Mdns
} // namespace app
} // namespace chip
