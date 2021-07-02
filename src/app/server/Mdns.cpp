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
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#endif
#include <protocols/secure_channel/PASESession.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <support/Span.h>
#include <support/logging/CHIPLogging.h>
#include <transport/AdminPairingTable.h>

#include <app/server/Server.h>

namespace chip {
namespace app {
namespace Mdns {

namespace {

NodeId GetCurrentNodeId()
{
    // TODO: once operational credentials are implemented, node ID should be read from them

    // TODO: once multi-admin is decided, figure out if a single node id
    // is sufficient or if we need multi-node-id advertisement. Existing
    // mdns advertises a single node id as parameter.

    // Search for one admin pairing and use its node id.
    auto pairing = GetGlobalAdminPairingTable().cbegin();
    if (pairing != GetGlobalAdminPairingTable().cend())
    {
        ChipLogProgress(Discovery, "Found admin pairing for admin %" PRIX16 ", node 0x" ChipLogFormatX64, pairing->GetAdminId(),
                        ChipLogValueX64(pairing->GetNodeId()));
        return pairing->GetNodeId();
    }

    ChipLogError(Discovery, "Failed to find a valid admin pairing. Node ID unknown");
    return chip::kTestDeviceNodeId;
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
#if CONFIG_DEVICE_LAYER
    if (DeviceLayer::ConfigurationMgr().GetPrimaryWiFiMACAddress(mac) == CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Using wifi MAC for hostname");
        return chip::ByteSpan(mac, 6);
    }
#endif
    ChipLogError(Discovery, "Wifi mac not known. Using a default.");
    uint8_t temp[6] = { 0xEE, 0xAA, 0xBA, 0xDA, 0xBA, 0xD0 };
    memcpy(mac, temp, 6);
    return chip::ByteSpan(mac, 6);
}

} // namespace

CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen)
{
    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();
    return mdnsAdvertiser.GetCommissionableInstanceName(buffer, bufferLen);
}

/// Set MDNS operational advertisement
CHIP_ERROR AdvertiseOperational()
{
    uint64_t fabricId = 5544332211; // default value

#if CONFIG_DEVICE_LAYER
    if (DeviceLayer::ConfigurationMgr().GetFabricId(fabricId) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Fabric ID not known. Using a default");
    }
#endif

    uint8_t mac[8];

    const auto advertiseParameters =
        chip::Mdns::OperationalAdvertisingParameters()
            .SetPeerId(PeerId().SetFabricId(fabricId).SetNodeId(GetCurrentNodeId()))
            .SetMac(FillMAC(mac))
            .SetPort(CHIP_PORT)
            .SetMRPRetryIntervals(CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL, CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL)
            .EnableIpV4(true);

    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();

    ChipLogProgress(Discovery, "Advertise operational node " ChipLogFormatX64 "-" ChipLogFormatX64,
                    ChipLogValueX64(advertiseParameters.GetPeerId().GetFabricId()),
                    ChipLogValueX64(advertiseParameters.GetPeerId().GetNodeId()));
    return mdnsAdvertiser.Advertise(advertiseParameters);
}

/// Set MDNS commissioner advertisement
CHIP_ERROR AdvertiseCommisioner()
{
    return Advertise(false);
}

/// Set MDNS commissionable node advertisement
CHIP_ERROR AdvertiseCommissionableNode()
{
    return Advertise(true);
}

/// commissionableNode
// CHIP_ERROR Advertise(chip::Mdns::CommssionAdvertiseMode mode)
CHIP_ERROR Advertise(bool commissionableNode)
{
    auto advertiseParameters = chip::Mdns::CommissionAdvertisingParameters().SetPort(CHIP_PORT).EnableIpV4(true);
    advertiseParameters.SetCommissionAdvertiseMode(commissionableNode ? chip::Mdns::CommssionAdvertiseMode::kCommissionableNode
                                                                      : chip::Mdns::CommssionAdvertiseMode::kCommissioner);

    uint8_t mac[8];
    advertiseParameters.SetMac(FillMAC(mac));

#if CHIP_ENABLE_ROTATING_DEVICE_ID
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    ReturnErrorOnFailure(GenerateRotatingDeviceId(rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer)));
    advertiseParameters.SetRotatingId(chip::Optional<const char *>::Value(rotatingDeviceIdHexBuffer));
#endif

#if CONFIG_DEVICE_LAYER
    // TODO: device can re-enter commissioning mode after being fully provisioned
    // (additionalPairing == true)
    bool notYetCommissioned = !DeviceLayer::ConfigurationMgr().IsFullyProvisioned();
    bool additionalPairing  = false;
    advertiseParameters.SetCommissioningMode(notYetCommissioned, additionalPairing);

    char pairingInst[chip::Mdns::kKeyPairingInstructionMaxLength + 1];

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
    advertiseParameters.SetShortDiscriminator(static_cast<uint8_t>(value & 0xFF)).SetLongDiscrimininator(value);

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

    if (notYetCommissioned)
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
#endif
    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();

    ChipLogProgress(Discovery, "Advertise commission parameter vendorID=%u productID=%u discriminator=%04u/%02u",
                    advertiseParameters.GetVendorId().ValueOr(0), advertiseParameters.GetProductId().ValueOr(0),
                    advertiseParameters.GetLongDiscriminator(), advertiseParameters.GetShortDiscriminator());
    return mdnsAdvertiser.Advertise(advertiseParameters);
}

/// (Re-)starts the minmdns server
void StartServer()
{
    ChipLogProgress(Discovery, "Start dns-sd server");
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CONFIG_DEVICE_LAYER
    err = chip::Mdns::ServiceAdvertiser::Instance().Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
    // TODO: advertise this only when really operational once we support both
    // operational and commisioning advertising is supported.
    if (DeviceLayer::ConfigurationMgr().IsFullyProvisioned())
    {
        err = app::Mdns::AdvertiseOperational();
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
        err = app::Mdns::AdvertiseCommissionableNode();
#endif
    }
    else
    {
// TODO: Thread devices are not able to advertise using mDNS before being provisioned,
// so configuraton should be added to enable commissioning advertising based on supported
// Rendezvous methods.
#if (!CHIP_DEVICE_CONFIG_ENABLE_THREAD || CHIP_DEVICE_CONFIG_ENABLE_UNPROVISIONED_MDNS)
        err = app::Mdns::AdvertiseCommissionableNode();
#endif
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    err = app::Mdns::AdvertiseCommisioner();
#endif

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to start mDNS server: %s", chip::ErrorStr(err));
    }
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize)
{
    char serialNumber[chip::DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];
    size_t serialNumberSize                = 0;
    uint16_t lifetimeCounter               = 0;
    size_t rotatingDeviceIdValueOutputSize = 0;

#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(
        chip::DeviceLayer::ConfigurationMgr().GetSerialNumber(serialNumber, sizeof(serialNumber), serialNumberSize));
    ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().GetLifetimeCounter(lifetimeCounter));
    return AdditionalDataPayloadGenerator().generateRotatingDeviceId(lifetimeCounter, serialNumber, serialNumberSize,
                                                                     rotatingDeviceIdHexBuffer, rotatingDeviceIdHexBufferSize,
                                                                     rotatingDeviceIdValueOutputSize);
#else
    error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}
#endif

} // namespace Mdns
} // namespace app
} // namespace chip
