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

#include <app/server/Dnssd.h>

#include <inttypes.h>

#include <lib/core/Optional.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/PASESession.h>
#if CHIP_ENABLE_ROTATING_DEVICE_ID
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif
#include <credentials/FabricTable.h>
#include <system/TimeSource.h>

#include <app/server/Server.h>

namespace chip {
namespace app {
namespace {

bool HaveOperationalCredentials()
{
    // Look for any fabric info that has a useful operational identity.
    for (const FabricInfo & fabricInfo : Server::GetInstance().GetFabricTable())
    {
        if (fabricInfo.IsInitialized())
        {
            return true;
        }
    }

    ChipLogProgress(Discovery, "Failed to find a valid admin pairing. Node ID unknown");
    return false;
}

void OnPlatformEvent(const DeviceLayer::ChipDeviceEvent * event)
{
    if (event->Type == DeviceLayer::DeviceEventType::kDnssdPlatformInitialized
#if CHIP_DEVICE_CONFIG_ENABLE_SED
        || event->Type == DeviceLayer::DeviceEventType::kSEDPollingIntervalChange
#endif
    )
    {
        app::DnssdServer::Instance().StartServer();
    }
}

void OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    OnPlatformEvent(event);
}

} // namespace

constexpr System::Clock::Timestamp DnssdServer::kTimeoutCleared;

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

constexpr const char kExtendedDiscoveryTimeoutKeypairStorage[] = "ExtDiscKey";

void DnssdServer::SetExtendedDiscoveryTimeoutSecs(int16_t secs)
{
    ChipLogDetail(Discovery, "SetExtendedDiscoveryTimeoutSecs %d", secs);
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kExtendedDiscoveryTimeoutKeypairStorage, &secs, sizeof(secs));
}

int16_t DnssdServer::GetExtendedDiscoveryTimeoutSecs()
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

/// Callback from Extended Discovery Expiration timer
void HandleExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    DnssdServer::Instance().OnExtendedDiscoveryExpiration(aSystemLayer, aAppState);
}

void DnssdServer::OnExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    if (!DnssdServer::OnExpiration(mExtendedDiscoveryExpiration))
    {
        ChipLogDetail(Discovery, "OnExtendedDiscoveryExpiration callback for cleared session");
        return;
    }

    ChipLogDetail(Discovery, "OnExtendedDiscoveryExpiration callback for valid session");

    mExtendedDiscoveryExpiration = kTimeoutCleared;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

/// Callback from Discovery Expiration timer
void HandleDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    DnssdServer::Instance().OnDiscoveryExpiration(aSystemLayer, aAppState);
}

bool DnssdServer::OnExpiration(System::Clock::Timestamp expirationMs)
{
    if (expirationMs == kTimeoutCleared)
    {
        ChipLogDetail(Discovery, "OnExpiration callback for cleared session");
        return false;
    }
    System::Clock::Timestamp now = mTimeSource.GetMonotonicTimestamp();
    if (expirationMs > now)
    {
        ChipLogDetail(Discovery, "OnExpiration callback for reset session");
        return false;
    }

    ChipLogDetail(Discovery, "OnExpiration - valid time out");

    CHIP_ERROR err = Dnssd::ServiceAdvertiser::Instance().Init(chip::DeviceLayer::UDPEndPointManager());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to initialize advertiser: %s", chip::ErrorStr(err));
    }

    // reset advertising
    err = Dnssd::ServiceAdvertiser::Instance().RemoveServices();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to remove advertised services: %s", chip::ErrorStr(err));
    }

    // restart operational (if needed)
    err = AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise operational node: %s", chip::ErrorStr(err));
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = AdvertiseCommissioner();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise commissioner: %s", chip::ErrorStr(err));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    err = Dnssd::ServiceAdvertiser::Instance().FinalizeServiceUpdate();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to finalize service update: %s", chip::ErrorStr(err));
    }

    return true;
}

void DnssdServer::OnDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    if (!DnssdServer::OnExpiration(mDiscoveryExpiration))
    {
        ChipLogDetail(Discovery, "OnDiscoveryExpiration callback for cleared session");
        return;
    }

    ChipLogDetail(Discovery, "OnDiscoveryExpiration callback for valid session");

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    int16_t extTimeout = GetExtendedDiscoveryTimeoutSecs();
    if (extTimeout != CHIP_DEVICE_CONFIG_DISCOVERY_DISABLED)
    {
        CHIP_ERROR err = AdvertiseCommissionableNode(chip::Dnssd::CommissioningMode::kDisabled);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise extended commissionable node: %s", chip::ErrorStr(err));
        }
        // set timeout
        ScheduleExtendedDiscoveryExpiration();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

    mDiscoveryExpiration = kTimeoutCleared;
}

CHIP_ERROR DnssdServer::ScheduleDiscoveryExpiration()
{
    if (mDiscoveryTimeoutSecs == CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT)
    {
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(Discovery, "Scheduling Discovery timeout in secs=%d", mDiscoveryTimeoutSecs);

    mDiscoveryExpiration = mTimeSource.GetMonotonicTimestamp() + System::Clock::Seconds16(mDiscoveryTimeoutSecs);

    return DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(mDiscoveryTimeoutSecs), HandleDiscoveryExpiration,
                                                 nullptr);
}

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
CHIP_ERROR DnssdServer::ScheduleExtendedDiscoveryExpiration()
{
    int16_t extendedDiscoveryTimeoutSecs = GetExtendedDiscoveryTimeoutSecs();
    if (extendedDiscoveryTimeoutSecs == CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT)
    {
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(Discovery, "Scheduling Extended Discovery timeout in secs=%d", extendedDiscoveryTimeoutSecs);

    mExtendedDiscoveryExpiration = mTimeSource.GetMonotonicTimestamp() + System::Clock::Seconds16(extendedDiscoveryTimeoutSecs);

    return DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(extendedDiscoveryTimeoutSecs),
                                                 HandleExtendedDiscoveryExpiration, nullptr);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

CHIP_ERROR DnssdServer::GetCommissionableInstanceName(char * buffer, size_t bufferLen)
{
    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    return mdnsAdvertiser.GetCommissionableInstanceName(buffer, bufferLen);
}

/// Set MDNS operational advertisement
CHIP_ERROR DnssdServer::AdvertiseOperational()
{
    for (const FabricInfo & fabricInfo : Server::GetInstance().GetFabricTable())
    {
        if (fabricInfo.IsInitialized())
        {
            uint8_t macBuffer[DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength];
            MutableByteSpan mac(macBuffer);
            if (chip::DeviceLayer::ConfigurationMgr().GetPrimaryMACAddress(mac) != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to get primary mac address of device. Generating a random one.");
                Crypto::DRBG_get_bytes(macBuffer, sizeof(macBuffer));
            }

            const auto advertiseParameters = chip::Dnssd::OperationalAdvertisingParameters()
                                                 .SetPeerId(fabricInfo.GetPeerId())
                                                 .SetMac(mac)
                                                 .SetPort(GetSecuredPort())
                                                 .SetMRPConfig(GetLocalMRPConfig())
                                                 .SetTcpSupported(Optional<bool>(INET_CONFIG_ENABLE_TCP_ENDPOINT))
                                                 .EnableIpV4(true);

            auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();

            ChipLogProgress(Discovery, "Advertise operational node " ChipLogFormatX64 "-" ChipLogFormatX64,
                            ChipLogValueX64(advertiseParameters.GetPeerId().GetCompressedFabricId()),
                            ChipLogValueX64(advertiseParameters.GetPeerId().GetNodeId()));
            // Should we keep trying to advertise the other operational
            // identities on failure?
            ReturnErrorOnFailure(mdnsAdvertiser.Advertise(advertiseParameters));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdServer::Advertise(bool commissionableNode, chip::Dnssd::CommissioningMode mode)
{
    auto advertiseParameters = chip::Dnssd::CommissionAdvertisingParameters()
                                   .SetPort(commissionableNode ? GetSecuredPort() : GetUnsecuredPort())
                                   .EnableIpV4(true);
    advertiseParameters.SetCommissionAdvertiseMode(commissionableNode ? chip::Dnssd::CommssionAdvertiseMode::kCommissionableNode
                                                                      : chip::Dnssd::CommssionAdvertiseMode::kCommissioner);

    advertiseParameters.SetCommissioningMode(mode);

    char pairingInst[chip::Dnssd::kKeyPairingInstructionMaxLength + 1];

    uint8_t macBuffer[DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength];
    MutableByteSpan mac(macBuffer);
    if (chip::DeviceLayer::ConfigurationMgr().GetPrimaryMACAddress(mac) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to get primary mac address of device. Generating a random one.");
        Crypto::DRBG_get_bytes(macBuffer, sizeof(macBuffer));
    }
    advertiseParameters.SetMac(mac);

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
    advertiseParameters.SetShortDiscriminator(static_cast<uint8_t>((value >> 8) & 0x0F)).SetLongDiscriminator(value);

    if (DeviceLayer::ConfigurationMgr().IsCommissionableDeviceTypeEnabled() &&
        DeviceLayer::ConfigurationMgr().GetDeviceTypeId(value) == CHIP_NO_ERROR)
    {
        advertiseParameters.SetDeviceType(chip::Optional<uint16_t>::Value(value));
    }

    char deviceName[chip::Dnssd::kKeyDeviceNameMaxLength + 1];
    if (DeviceLayer::ConfigurationMgr().IsCommissionableDeviceNameEnabled() &&
        DeviceLayer::ConfigurationMgr().GetCommissionableDeviceName(deviceName, sizeof(deviceName)) == CHIP_NO_ERROR)
    {
        advertiseParameters.SetDeviceName(chip::Optional<const char *>::Value(deviceName));
    }

#if CHIP_ENABLE_ROTATING_DEVICE_ID
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    ReturnErrorOnFailure(GenerateRotatingDeviceId(rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer)));
    advertiseParameters.SetRotatingDeviceId(chip::Optional<const char *>::Value(rotatingDeviceIdHexBuffer));
#endif

    advertiseParameters.SetMRPConfig(GetLocalMRPConfig()).SetTcpSupported(Optional<bool>(INET_CONFIG_ENABLE_TCP_ENDPOINT));

    if (mode != chip::Dnssd::CommissioningMode::kEnabledEnhanced)
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
            advertiseParameters.SetPairingInstruction(chip::Optional<const char *>::Value(pairingInst));
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
            advertiseParameters.SetPairingInstruction(chip::Optional<const char *>::Value(pairingInst));
        }
    }

    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();

    ChipLogProgress(Discovery, "Advertise commission parameter vendorID=%u productID=%u discriminator=%04u/%02u",
                    advertiseParameters.GetVendorId().ValueOr(0), advertiseParameters.GetProductId().ValueOr(0),
                    advertiseParameters.GetLongDiscriminator(), advertiseParameters.GetShortDiscriminator());
    return mdnsAdvertiser.Advertise(advertiseParameters);
}

CHIP_ERROR DnssdServer::AdvertiseCommissioner()
{
    return Advertise(false /* commissionableNode */, chip::Dnssd::CommissioningMode::kDisabled);
}

CHIP_ERROR DnssdServer::AdvertiseCommissionableNode(chip::Dnssd::CommissioningMode mode)
{
    return Advertise(true /* commissionableNode */, mode);
}

void DnssdServer::StartServer()
{
    return StartServer(NullOptional);
}

void DnssdServer::StartServer(Dnssd::CommissioningMode mode)
{
    return StartServer(MakeOptional(mode));
}

void DnssdServer::StartServer(Optional<Dnssd::CommissioningMode> mode)
{
    // Default to CommissioningMode::kDisabled if no value is provided.
    Dnssd::CommissioningMode modeValue = mode.ValueOr(Dnssd::CommissioningMode::kDisabled);

    ChipLogDetail(Discovery, "DNS-SD StartServer modeHasValue=%d modeValue=%d", mode.HasValue(), static_cast<int>(modeValue));

    ClearTimeouts();

    DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEventWrapper, 0);

    CHIP_ERROR err = Dnssd::ServiceAdvertiser::Instance().Init(chip::DeviceLayer::UDPEndPointManager());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to initialize advertiser: %s", chip::ErrorStr(err));
    }

    err = Dnssd::ServiceAdvertiser::Instance().RemoveServices();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to remove advertised services: %s", chip::ErrorStr(err));
    }

    err = AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise operational node: %s", chip::ErrorStr(err));
    }

    if (HaveOperationalCredentials())
    {
        ChipLogProgress(Discovery, "Have operational credentials");
        if (modeValue != chip::Dnssd::CommissioningMode::kDisabled)
        {
            err = AdvertiseCommissionableNode(modeValue);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to advertise commissionable node: %s", chip::ErrorStr(err));
            }
            // no need to set timeout because callers are currently doing that and their timeout might be longer than the default
        }
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
        else if (GetExtendedDiscoveryTimeoutSecs() != CHIP_DEVICE_CONFIG_DISCOVERY_DISABLED)
        {
            err = AdvertiseCommissionableNode(modeValue);
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
        err = AdvertiseCommissionableNode(mode.ValueOr(chip::Dnssd::CommissioningMode::kEnabledBasic));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise unprovisioned commissionable node: %s", chip::ErrorStr(err));
        }
        // set timeout
        ScheduleDiscoveryExpiration();
#endif
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = AdvertiseCommissioner();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise commissioner: %s", chip::ErrorStr(err));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    err = Dnssd::ServiceAdvertiser::Instance().FinalizeServiceUpdate();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to finalize service update: %s", chip::ErrorStr(err));
    }
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
CHIP_ERROR DnssdServer::GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize)
{
    char serialNumber[chip::DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];
    uint16_t lifetimeCounter               = 0;
    size_t rotatingDeviceIdValueOutputSize = 0;

    ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().GetSerialNumber(serialNumber, sizeof(serialNumber)));
    ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().GetLifetimeCounter(lifetimeCounter));
    return AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        lifetimeCounter, serialNumber, strlen(serialNumber), rotatingDeviceIdHexBuffer, rotatingDeviceIdHexBufferSize,
        rotatingDeviceIdValueOutputSize);
}
#endif

} // namespace app
} // namespace chip
