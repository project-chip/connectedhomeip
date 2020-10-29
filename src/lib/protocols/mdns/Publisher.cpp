/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Publisher.h"

#include <inttypes.h>

#include "lib/protocols/mdns/platform/Mdns.h"
#include "lib/support/logging/CHIPLogging.h"
#include "platform/CHIPDeviceBuildConfig.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CodeUtils.h"
#include "support/ErrorStr.h"
#include "support/RandUtils.h"

namespace chip {
namespace Protocols {
namespace Mdns {

CHIP_ERROR Publisher::Init()
{
    CHIP_ERROR error;

    mUnprovisionedInstanceName = GetRandU64();
    SuccessOrExit(error = ChipMdnsInit(HandleMdnsInit, HandleMdnsError, this));
    SuccessOrExit(error = SetupHostname());
exit:
    return error;
}

void Publisher::HandleMdnsInit(void * context, CHIP_ERROR initError)
{
    Publisher * publisher = static_cast<Publisher *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mInitialized = true;
    }
    else
    {
        ChipLogError(Discovery, "mDNS initialization failed with %s", chip::ErrorStr(initError));
    }
}

void Publisher::HandleMdnsError(void * context, CHIP_ERROR initError)
{
    ChipLogError(Discovery, "mDNS error: %s", chip::ErrorStr(initError));
}

CHIP_ERROR Publisher::StartPublishDevice(chip::Inet::InterfaceId interface)
{
    CHIP_ERROR error;

    // TODO: after multi-admin is decided we may need to publish both _chipc._udp and _chip._tcp service
    if (chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned() != mIsPublishingProvisionedDevice)
    {
        SuccessOrExit(error = StopPublishDevice());
        // Set hostname again in case the mac address changes when shifting from soft-AP to station
        SuccessOrExit(error = SetupHostname());
    }
    mIsPublishingProvisionedDevice = chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned();

    if (mIsPublishingProvisionedDevice)
    {
        error = PublishProvisionedDevice(interface);
    }
    else
    {
        error = PublishUnprovisionedDevice(interface);
    }
exit:
    return error;
}

CHIP_ERROR Publisher::SetupHostname()
{
    uint8_t mac[6];    // 6 byte wifi mac
    char hostname[13]; // Hostname will be the hex representation of mac.
    CHIP_ERROR error;

    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetPrimaryWiFiMACAddress(mac));
    for (size_t i = 0; i < sizeof(mac); i++)
    {
        snprintf(&hostname[i * 2], sizeof(hostname) - i * 2, "%02X", mac[i]);
    }
    SuccessOrExit(error = ChipMdnsSetHostname(hostname));

exit:
    return error;
}

CHIP_ERROR Publisher::PublishUnprovisionedDevice(chip::Inet::InterfaceId interface)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    MdnsService service;
    uint16_t discriminator;
    uint16_t vendorID;
    uint16_t productID;
    char discriminatorBuf[5];  // hex representation of 16-bit discriminator
    char vendorProductBuf[10]; // "FFFF+FFFF"
    // TODO: The text entry will be updated in the spec, update accordingly.
    TextEntry entries[2] = {
        { "D", nullptr, 0 },
        { "VP", nullptr, 0 },
    };

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    ChipLogProgress(Discovery, "setup mdns service");
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(discriminator));
    snprintf(service.mName, sizeof(service.mName), "%016" PRIx64, mUnprovisionedInstanceName);
    strncpy(service.mType, "_chipc", sizeof(service.mType));
    service.mProtocol = MdnsServiceProtocol::kMdnsProtocolUdp;
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetVendorId(vendorID));
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetProductId(productID));
    snprintf(discriminatorBuf, sizeof(discriminatorBuf), "%04X", discriminator);
    snprintf(vendorProductBuf, sizeof(vendorProductBuf), "%04X+%04X", vendorID, productID);
    entries[0].mData       = reinterpret_cast<const uint8_t *>(discriminatorBuf);
    entries[0].mDataSize   = strnlen(discriminatorBuf, sizeof(discriminatorBuf));
    entries[1].mData       = reinterpret_cast<const uint8_t *>(vendorProductBuf);
    entries[1].mDataSize   = strnlen(discriminatorBuf, sizeof(vendorProductBuf));
    service.mTextEntryies  = entries;
    service.mTextEntrySize = sizeof(entries) / sizeof(TextEntry);
    service.mPort          = CHIP_PORT;

    error = ChipMdnsPublishService(&service);

exit:
    return error;
}

CHIP_ERROR Publisher::PublishProvisionedDevice(chip::Inet::InterfaceId interface)
{
    uint64_t deviceId;
    uint64_t fabricId;
    MdnsService service;
    CHIP_ERROR error = CHIP_NO_ERROR;

    // TODO: There may be multilple device/fabrid ids after multi-admin.
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetFabricId(fabricId));
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetDeviceId(deviceId));
    snprintf(service.mName, sizeof(service.mName), "%" PRIX64 "-%" PRIX64, deviceId, fabricId);
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mPort          = CHIP_PORT;
    service.mTextEntryies  = nullptr;
    service.mTextEntrySize = 0;
    error                  = ChipMdnsPublishService(&service);
exit:
    return error;
}

CHIP_ERROR Publisher::StopPublishDevice()
{
    return ChipMdnsStopPublish();
}

} // namespace Mdns
} // namespace Protocols
} // namespace chip
