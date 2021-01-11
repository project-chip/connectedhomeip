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

#include "DiscoveryManager.h"

#include <inttypes.h>

#include "lib/mdns/platform/Mdns.h"
#include "lib/support/logging/CHIPLogging.h"
#include "platform/CHIPDeviceConfig.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CodeUtils.h"
#include "support/ErrorStr.h"
#include "support/RandUtils.h"

#if CHIP_ENABLE_MDNS

namespace {

uint8_t HexToInt(char c)
{
    if ('0' <= c && c <= '9')
    {
        return static_cast<uint8_t>(c - '0');
    }
    else if ('a' <= c && c <= 'f')
    {
        return static_cast<uint8_t>(0x0a + c - 'a');
    }
    else if ('A' <= c && c <= 'F')
    {
        return static_cast<uint8_t>(0x0a + c - 'A');
    }

    return UINT8_MAX;
}

constexpr uint64_t kUndefinedNodeId = 0;

} // namespace
#endif

namespace chip {
namespace Mdns {

DiscoveryManager DiscoveryManager::sManager;

CHIP_ERROR DiscoveryManager::Init()
{
#if CHIP_ENABLE_MDNS
    CHIP_ERROR error;

    mUnprovisionedInstanceName = GetRandU64();
    SuccessOrExit(error = ChipMdnsInit(HandleMdnsInit, HandleMdnsError, this));
exit:
    return error;
#else
    return CHIP_NO_ERROR;
#endif // CHIP_ENABLE_MDNS
}

void DiscoveryManager::HandleMdnsInit(void * context, CHIP_ERROR initError)
{
#if CHIP_ENABLE_MDNS
    DiscoveryManager * publisher = static_cast<DiscoveryManager *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mMdnsInitialized = true;
    }
    else
    {
        ChipLogError(Discovery, "mDNS initialization failed with %s", chip::ErrorStr(initError));
    }
#endif // CHIP_ENABLE_MDNS
}

void DiscoveryManager::HandleMdnsError(void * context, CHIP_ERROR error)
{
#if CHIP_ENABLE_MDNS
    DiscoveryManager * publisher = static_cast<DiscoveryManager *>(context);
    if (error == CHIP_ERROR_FORCED_RESET && publisher->mIsPublishing)
    {
        publisher->StartPublishDevice();
    }
    else
    {
        ChipLogError(Discovery, "mDNS error: %s", chip::ErrorStr(error));
    }
#endif // CHIP_ENABLE_MDNS
}

CHIP_ERROR DiscoveryManager::StartPublishDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface)
{
#if CHIP_ENABLE_MDNS
    CHIP_ERROR error;

    // TODO: after multi-admin is decided we may need to publish both _chipc._udp and _chip._tcp service
    if (!mIsPublishing)
    {
        SuccessOrExit(error = SetupHostname());
    }
    else if (chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned() != mIsPublishingProvisionedDevice)
    {
        SuccessOrExit(error = StopPublishDevice());
        // Set hostname again in case the mac address changes when shifting from soft-AP to station
        SuccessOrExit(error = SetupHostname());
    }
    mIsPublishingProvisionedDevice = chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned();

    if (mIsPublishingProvisionedDevice)
    {
        error = PublishProvisionedDevice(addressType, interface);
    }
    else
    {
        error = PublishUnprovisionedDevice(addressType, interface);
    }
    mIsPublishing = true;
exit:
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_MDNS
}

CHIP_ERROR DiscoveryManager::SetupHostname()
{
#if CHIP_ENABLE_MDNS
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
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_MDNS
}

CHIP_ERROR DiscoveryManager::PublishUnprovisionedDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface)
{
#if CHIP_ENABLE_MDNS
    CHIP_ERROR error = CHIP_NO_ERROR;
    MdnsService service;
    uint16_t discriminator;
    uint16_t vendorID;
    uint16_t productID;
    char discriminatorBuf[5];  // hex representation of 16-bit discriminator
    char vendorProductBuf[10]; // "FFFF+FFFF"
    // TODO: The text entry will be updated in the spec, update accordingly.
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    TextEntry entries[3] = { { "D", nullptr, 0 }, { "VP", nullptr, 0 }, { "RI", nullptr, 0 } };
#else
    TextEntry entries[2] = { { "D", nullptr, 0 }, { "VP", nullptr, 0 } };
#endif
    VerifyOrExit(mMdnsInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    ChipLogProgress(Discovery, "setup mdns service");
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(discriminator));
    snprintf(service.mName, sizeof(service.mName), "%016" PRIX64, mUnprovisionedInstanceName);
    strncpy(service.mType, "_chipc", sizeof(service.mType));
    service.mProtocol = MdnsServiceProtocol::kMdnsProtocolUdp;
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetVendorId(vendorID));
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetProductId(productID));
    snprintf(discriminatorBuf, sizeof(discriminatorBuf), "%04X", discriminator);
    snprintf(vendorProductBuf, sizeof(vendorProductBuf), "%04X+%04X", vendorID, productID);
    entries[0].mData     = reinterpret_cast<const uint8_t *>(discriminatorBuf);
    entries[0].mDataSize = strnlen(discriminatorBuf, sizeof(discriminatorBuf));
    entries[1].mData     = reinterpret_cast<const uint8_t *>(vendorProductBuf);
    entries[1].mDataSize = strnlen(discriminatorBuf, sizeof(vendorProductBuf));

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    // Rotating Device ID
    entries[2].mData     = reinterpret_cast<const uint8_t *>(CHIP_ROTATING_DEVICE_ID);
    entries[2].mDataSize = strnlen(CHIP_ROTATING_DEVICE_ID, sizeof(CHIP_ROTATING_DEVICE_ID));
#endif
    service.mTextEntryies  = entries;
    service.mTextEntrySize = sizeof(entries) / sizeof(TextEntry);
    service.mPort          = CHIP_PORT;
    service.mInterface     = interface;
    service.mAddressType   = addressType;
    error                  = ChipMdnsPublishService(&service);

exit:
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_MDNS
}

CHIP_ERROR DiscoveryManager::PublishProvisionedDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface)
{
#if CHIP_ENABLE_MDNS
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
    service.mInterface     = interface;
    service.mAddressType   = addressType;
    error                  = ChipMdnsPublishService(&service);

exit:
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_MDNS
}

CHIP_ERROR DiscoveryManager::StopPublishDevice()
{
#if CHIP_ENABLE_MDNS
    mIsPublishing = false;
    return ChipMdnsStopPublish();
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_MDNS
}

CHIP_ERROR DiscoveryManager::RegisterResolveDelegate(ResolveDelegate * delegate)
{
    if (mResolveDelegate != nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    else
    {
        mResolveDelegate = delegate;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR DiscoveryManager::ResolveNodeId(uint64_t nodeId, uint64_t fabricId, Inet::IPAddressType type)
{
#if CHIP_ENABLE_MDNS
    MdnsService service;

    snprintf(service.mName, sizeof(service.mName), "%" PRIX64 "-%" PRIX64, nodeId, fabricId);
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol    = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mAddressType = type;
    return ChipMdnsResolve(&service, INET_NULL_INTERFACEID, HandleNodeIdResolve, this);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_MDNS
}

void DiscoveryManager::HandleNodeIdResolve(void * context, MdnsService * result, CHIP_ERROR error)
{
#if CHIP_ENABLE_MDNS
    DiscoveryManager * mgr = static_cast<DiscoveryManager *>(context);

    if (mgr->mResolveDelegate == nullptr)
    {
        return;
    }
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Node ID resolved failed with %s", chip::ErrorStr(error));
        mgr->mResolveDelegate->HandleNodeIdResolve(error, kUndefinedNodeId, MdnsService{});
    }
    else if (result == nullptr)
    {
        ChipLogError(Discovery, "Node ID resolve not found");
        mgr->mResolveDelegate->HandleNodeIdResolve(CHIP_ERROR_UNKNOWN_RESOURCE_ID, kUndefinedNodeId, MdnsService{});
    }
    else
    {
        // Parse '%x-%x' from the name
        uint64_t nodeId       = 0;
        bool deliminatorFound = false;

        for (size_t i = 0; i < sizeof(result->mName) && result->mName[i] != 0; i++)
        {
            if (result->mName[i] == '-')
            {
                deliminatorFound = true;
                break;
            }
            else
            {
                uint8_t val = HexToInt(result->mName[i]);

                if (val == UINT8_MAX)
                {
                    break;
                }
                else
                {
                    nodeId = nodeId * 16 + val;
                }
            }
        }

        if (deliminatorFound)
        {
            ChipLogProgress(Discovery, "Node ID resolved for %" PRIX64, nodeId);
            mgr->mResolveDelegate->HandleNodeIdResolve(error, nodeId, *result);
        }
        else
        {
            ChipLogProgress(Discovery, "Invalid service entry from node %" PRIX64, nodeId);
            mgr->mResolveDelegate->HandleNodeIdResolve(error, kUndefinedNodeId, *result);
        }
    }
#endif // CHIP_ENABLE_MDNS
}

} // namespace Mdns
} // namespace chip
