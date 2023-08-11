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
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <protocols/secure_channel/PASESession.h>
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif
#include <credentials/FabricTable.h>
#include <setup_payload/SetupPayload.h>
#include <system/TimeSource.h>

#include <app/server/Server.h>

namespace chip {
namespace app {
namespace {

void OnPlatformEvent(const DeviceLayer::ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceLayer::DeviceEventType::kDnssdInitialized:
    case DeviceLayer::DeviceEventType::kDnssdRestartNeeded:
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    case DeviceLayer::DeviceEventType::kICDPollingIntervalChange:
#endif
        app::DnssdServer::Instance().StartServer();
        break;
    default:
        break;
    }
}

void OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    OnPlatformEvent(event);
}

} // namespace

constexpr System::Clock::Timestamp DnssdServer::kTimeoutCleared;

bool DnssdServer::HaveOperationalCredentials()
{
    VerifyOrDie(mFabricTable != nullptr);

    // Look for any fabric info that has a useful operational identity.
    return mFabricTable->FabricCount() != 0;
}

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

void DnssdServer::SetExtendedDiscoveryTimeoutSecs(int32_t secs)
{
    ChipLogDetail(Discovery, "Setting extended discovery timeout to %" PRId32 "s", secs);
    mExtendedDiscoveryTimeoutSecs = MakeOptional(secs);

    if (mExtendedDiscoveryExpiration != kTimeoutCleared &&
        mExtendedDiscoveryExpiration > mTimeSource.GetMonotonicTimestamp() + System::Clock::Seconds32(secs))
    {
        // Reset our timer to the new (shorter) timeout.
        ScheduleExtendedDiscoveryExpiration();
    }
}

int32_t DnssdServer::GetExtendedDiscoveryTimeoutSecs()
{
    return mExtendedDiscoveryTimeoutSecs.ValueOr(CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS);
}

/// Callback from Extended Discovery Expiration timer
void HandleExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    DnssdServer::Instance().OnExtendedDiscoveryExpiration(aSystemLayer, aAppState);
}

void DnssdServer::OnExtendedDiscoveryExpiration(System::Layer * aSystemLayer, void * aAppState)
{
    ChipLogDetail(Discovery, "Extended discovery timed out");

    mExtendedDiscoveryExpiration = kTimeoutCleared;

    // Reset our advertising, now that we have flagged ourselves as possibly not
    // needing extended discovery anymore.
    StartServer();
}

CHIP_ERROR DnssdServer::ScheduleExtendedDiscoveryExpiration()
{
    int32_t extendedDiscoveryTimeoutSecs = GetExtendedDiscoveryTimeoutSecs();
    if (extendedDiscoveryTimeoutSecs == CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT)
    {
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(Discovery, "Scheduling extended discovery timeout in %" PRId32 "s", extendedDiscoveryTimeoutSecs);

    mExtendedDiscoveryExpiration = mTimeSource.GetMonotonicTimestamp() + System::Clock::Seconds32(extendedDiscoveryTimeoutSecs);

    return DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(extendedDiscoveryTimeoutSecs),
                                                 HandleExtendedDiscoveryExpiration, nullptr);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

CHIP_ERROR DnssdServer::GetCommissionableInstanceName(char * buffer, size_t bufferLen)
{
    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    return mdnsAdvertiser.GetCommissionableInstanceName(buffer, bufferLen);
}

CHIP_ERROR DnssdServer::SetEphemeralDiscriminator(Optional<uint16_t> discriminator)
{
    VerifyOrReturnError(discriminator.ValueOr(0) <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    mEphemeralDiscriminator = discriminator;

    return CHIP_NO_ERROR;
}

/// Set MDNS operational advertisement
CHIP_ERROR DnssdServer::AdvertiseOperational()
{
    VerifyOrDie(mFabricTable != nullptr);

    for (const FabricInfo & fabricInfo : *mFabricTable)
    {
        if (!fabricInfo.ShouldAdvertiseIdentity())
        {
            continue;
        }

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
                                             .SetInterfaceId(GetInterfaceId())
                                             .SetLocalMRPConfig(GetLocalMRPConfig())
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
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdServer::Advertise(bool commissionableNode, chip::Dnssd::CommissioningMode mode)
{
    auto advertiseParameters = chip::Dnssd::CommissionAdvertisingParameters()
                                   .SetPort(commissionableNode ? GetSecuredPort() : GetUnsecuredPort())
                                   .SetInterfaceId(GetInterfaceId())
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
    uint32_t val32;
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(value) != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Vendor ID not known");
    }
    else
    {
        advertiseParameters.SetVendorId(chip::Optional<uint16_t>::Value(value));
    }

    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(value) != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Product ID not known");
    }
    else
    {
        advertiseParameters.SetProductId(chip::Optional<uint16_t>::Value(value));
    }

    if (DeviceLayer::ConfigurationMgr().IsCommissionableDeviceTypeEnabled() &&
        DeviceLayer::ConfigurationMgr().GetDeviceTypeId(val32) == CHIP_NO_ERROR)
    {
        advertiseParameters.SetDeviceType(chip::Optional<uint32_t>::Value(val32));
    }

    char deviceName[chip::Dnssd::kKeyDeviceNameMaxLength + 1];
    if (DeviceLayer::ConfigurationMgr().IsCommissionableDeviceNameEnabled() &&
        DeviceLayer::ConfigurationMgr().GetCommissionableDeviceName(deviceName, sizeof(deviceName)) == CHIP_NO_ERROR)
    {
        advertiseParameters.SetDeviceName(chip::Optional<const char *>::Value(deviceName));
    }

    advertiseParameters.SetLocalMRPConfig(GetLocalMRPConfig()).SetTcpSupported(Optional<bool>(INET_CONFIG_ENABLE_TCP_ENDPOINT));

    if (commissionableNode)
    {
        uint16_t discriminator = 0;
        CHIP_ERROR error       = DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery,
                         "Setup discriminator read error (%" CHIP_ERROR_FORMAT ")! Critical error, will not be commissionable.",
                         error.Format());
            return error;
        }

        // Override discriminator with temporary one if one is set
        discriminator = mEphemeralDiscriminator.ValueOr(discriminator);

        advertiseParameters.SetShortDiscriminator(static_cast<uint8_t>((discriminator >> 8) & 0x0F))
            .SetLongDiscriminator(discriminator);

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
        char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
        ReturnErrorOnFailure(GenerateRotatingDeviceId(rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer)));
        advertiseParameters.SetRotatingDeviceId(chip::Optional<const char *>::Value(rotatingDeviceIdHexBuffer));
#endif

        if (!HaveOperationalCredentials())
        {
            if (DeviceLayer::ConfigurationMgr().GetInitialPairingHint(value) != CHIP_NO_ERROR)
            {
                ChipLogDetail(Discovery, "DNS-SD Pairing Hint not set");
            }
            else
            {
                advertiseParameters.SetPairingHint(chip::Optional<uint16_t>::Value(value));
            }

            if (DeviceLayer::ConfigurationMgr().GetInitialPairingInstruction(pairingInst, sizeof(pairingInst)) != CHIP_NO_ERROR)
            {
                ChipLogDetail(Discovery, "DNS-SD Pairing Instruction not set");
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
                ChipLogDetail(Discovery, "DNS-SD Pairing Hint not set");
            }
            else
            {
                advertiseParameters.SetPairingHint(chip::Optional<uint16_t>::Value(value));
            }

            if (DeviceLayer::ConfigurationMgr().GetSecondaryPairingInstruction(pairingInst, sizeof(pairingInst)) != CHIP_NO_ERROR)
            {
                ChipLogDetail(Discovery, "DNS-SD Pairing Instruction not set");
            }
            else
            {
                advertiseParameters.SetPairingInstruction(chip::Optional<const char *>::Value(pairingInst));
            }
        }
    }

    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();

    ChipLogProgress(Discovery, "Advertise commission parameter vendorID=%u productID=%u discriminator=%04u/%02u cm=%u",
                    advertiseParameters.GetVendorId().ValueOr(0), advertiseParameters.GetProductId().ValueOr(0),
                    advertiseParameters.GetLongDiscriminator(), advertiseParameters.GetShortDiscriminator(),
                    to_underlying(advertiseParameters.GetCommissioningMode()));
    return mdnsAdvertiser.Advertise(advertiseParameters);
}

CHIP_ERROR DnssdServer::AdvertiseCommissioner()
{
    return Advertise(false /* commissionableNode */, chip::Dnssd::CommissioningMode::kDisabled);
}

CHIP_ERROR DnssdServer::AdvertiseCommissionableNode(chip::Dnssd::CommissioningMode mode)
{
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    mCurrentCommissioningMode = mode;
    if (mode != Dnssd::CommissioningMode::kDisabled)
    {
        // We're not doing extended discovery right now.
        DeviceLayer::SystemLayer().CancelTimer(HandleExtendedDiscoveryExpiration, nullptr);
        mExtendedDiscoveryExpiration = kTimeoutCleared;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

    return Advertise(true /* commissionableNode */, mode);
}

void DnssdServer::StartServer()
{
    Dnssd::CommissioningMode mode = Dnssd::CommissioningMode::kDisabled;
    if (mCommissioningModeProvider)
    {
        mode = mCommissioningModeProvider->GetCommissioningMode();
    }
    return StartServer(mode);
}

void DnssdServer::StopServer()
{
    // Make sure we don't hold on to a dangling fabric table pointer.
    mFabricTable = nullptr;

    DeviceLayer::PlatformMgr().RemoveEventHandler(OnPlatformEventWrapper, 0);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    if (mExtendedDiscoveryExpiration != kTimeoutCleared)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleExtendedDiscoveryExpiration, nullptr);
        mExtendedDiscoveryExpiration = kTimeoutCleared;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

    if (Dnssd::ServiceAdvertiser::Instance().IsInitialized())
    {
        auto err = Dnssd::ServiceAdvertiser::Instance().RemoveServices();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to remove advertised services: %" CHIP_ERROR_FORMAT, err.Format());
        }

        Dnssd::ServiceAdvertiser::Instance().Shutdown();
    }
}

void DnssdServer::StartServer(Dnssd::CommissioningMode mode)
{
    ChipLogProgress(Discovery, "Updating services using commissioning mode %d", static_cast<int>(mode));

    DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEventWrapper, 0);

    CHIP_ERROR err = Dnssd::ServiceAdvertiser::Instance().Init(chip::DeviceLayer::UDPEndPointManager());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to initialize advertiser: %" CHIP_ERROR_FORMAT, err.Format());
    }

    err = Dnssd::ServiceAdvertiser::Instance().RemoveServices();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to remove advertised services: %" CHIP_ERROR_FORMAT, err.Format());
    }

    err = AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise operational node: %" CHIP_ERROR_FORMAT, err.Format());
    }

    if (mode != Dnssd::CommissioningMode::kDisabled)
    {
        err = AdvertiseCommissionableNode(mode);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise commissionable node: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    else if (GetExtendedDiscoveryTimeoutSecs() != CHIP_DEVICE_CONFIG_DISCOVERY_DISABLED)
    {
        bool alwaysAdvertiseExtended = (GetExtendedDiscoveryTimeoutSecs() == CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT);
        // We do extended discovery advertising in three cases:
        // 1) We don't have a timeout for extended discovery.
        // 2) We are transitioning out of commissioning mode (basic or enhanced)
        //    and should therefore start extended discovery.
        // 3) We are resetting advertising while we are in the middle of an
        //    existing extended discovery advertising period.
        if (alwaysAdvertiseExtended || mCurrentCommissioningMode != Dnssd::CommissioningMode::kDisabled ||
            mExtendedDiscoveryExpiration != kTimeoutCleared)
        {
            err = AdvertiseCommissionableNode(mode);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to advertise extended commissionable node: %" CHIP_ERROR_FORMAT, err.Format());
            }
            if (mExtendedDiscoveryExpiration == kTimeoutCleared)
            {
                // set timeout
                ScheduleExtendedDiscoveryExpiration();
            }
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = AdvertiseCommissioner();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to advertise commissioner: %" CHIP_ERROR_FORMAT, err.Format());
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    err = Dnssd::ServiceAdvertiser::Instance().FinalizeServiceUpdate();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to finalize service update: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
CHIP_ERROR DnssdServer::GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize)
{
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    uint8_t rotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
    MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);
    size_t rotatingDeviceIdValueOutputSize = 0;

    ReturnErrorOnFailure(
        chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan));
    ReturnErrorOnFailure(
        chip::DeviceLayer::ConfigurationMgr().GetLifetimeCounter(additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter));
    additionalDataPayloadParams.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;

    return AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, rotatingDeviceIdHexBufferSize, rotatingDeviceIdValueOutputSize);
}
#endif

} // namespace app
} // namespace chip
