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

#include "Discovery_ImplPlatform.h"

#include <inttypes.h>

#include "lib/mdns/platform/Mdns.h"
#include "lib/support/logging/CHIPLogging.h"
#include "platform/CHIPDeviceConfig.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CodeUtils.h"
#include "support/ErrorStr.h"
#include "support/RandUtils.h"

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

namespace chip {
namespace Mdns {

DiscoveryImplPlatform DiscoveryImplPlatform::sManager;

DiscoveryImplPlatform::DiscoveryImplPlatform()
{
    mCommissionInstanceName = GetRandU64();
    CHIP_ERROR error        = ChipMdnsInit(HandleMdnsInit, HandleMdnsError, this);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to initialize platform mdns: %s", ErrorStr(error));
    }
}

CHIP_ERROR DiscoveryImplPlatform::Start(Inet::InetLayer * inetLayer, uint16_t port)
{
    return CHIP_NO_ERROR;
}

void DiscoveryImplPlatform::HandleMdnsInit(void * context, CHIP_ERROR initError)
{
    DiscoveryImplPlatform * publisher = static_cast<DiscoveryImplPlatform *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mMdnsInitialized = true;
    }
    else
    {
        ChipLogError(Discovery, "mDNS initialization failed with %s", chip::ErrorStr(initError));
    }
}

void DiscoveryImplPlatform::HandleMdnsError(void * context, CHIP_ERROR error)
{
    DiscoveryImplPlatform * publisher = static_cast<DiscoveryImplPlatform *>(context);
    if (error == CHIP_ERROR_FORCED_RESET)
    {
        if (publisher->mIsOperationalPublishing)
        {
            publisher->Advertise(publisher->mOperationalAdvertisingParams);
        }
        if (publisher->mIsCommissionalPublishing)
        {
            publisher->Advertise(publisher->mCommissioningdvertisingParams);
        }
    }
    else
    {
        ChipLogError(Discovery, "mDNS error: %s", chip::ErrorStr(error));
    }
}

CHIP_ERROR DiscoveryImplPlatform::SetupHostname()
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

CHIP_ERROR DiscoveryImplPlatform::Advertise(const CommissionAdvertisingParameters & params)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    MdnsService service;
    char discriminatorBuf[6];
    char vendorProductBuf[12];
    char pairingInstrBuf[128];
    TextEntry textEntries[4];
    size_t textEntrySize = 0;
    char shortDiscriminatorSubtype[6];
    char longDiscriminatorSubtype[7];
    char vendorSubType[8];
    const char * subTypes[3];
    size_t subTypeSize = 0;

    if (!mMdnsInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    snprintf(service.mName, sizeof(service.mName), "%016" PRIX64, mCommissionInstanceName);
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissioning)
    {
        strncpy(service.mType, "_chipc", sizeof(service.mType));
    }
    else
    {
        strncpy(service.mType, "_chipd", sizeof(service.mType));
    }
    service.mProtocol = MdnsServiceProtocol::kMdnsProtocolUdp;

    snprintf(discriminatorBuf, sizeof(discriminatorBuf), "%04u", params.GetLongDiscriminator());
    textEntries[textEntrySize++] = { "D", reinterpret_cast<const uint8_t *>(discriminatorBuf),
                                     strnlen(discriminatorBuf, sizeof(discriminatorBuf)) };
    if (params.GetVendorId().HasValue())
    {
        if (params.GetProductId().HasValue())
        {
            snprintf(vendorProductBuf, sizeof(vendorProductBuf), "%u+%u", params.GetVendorId().Value(),
                     params.GetProductId().Value());
        }
        else
        {
            snprintf(vendorProductBuf, sizeof(vendorProductBuf), "%u", params.GetVendorId().Value());
        }
        textEntries[textEntrySize++] = { "VP", reinterpret_cast<const uint8_t *>(vendorProductBuf),
                                         strnlen(vendorProductBuf, sizeof(vendorProductBuf)) };
    }
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    textEntries[textEntrySize++] = { "RI", reinterpret_cast<const uint8_t *>(CHIP_ROTATING_DEVICE_ID),
                                     strlen(CHIP_ROTATING_DEVICE_ID) };
#endif
    if (params.GetPairingHint().HasValue() && params.GetPairingInstr().HasValue())
    {
        snprintf(pairingInstrBuf, sizeof(pairingInstrBuf), "%s+%u", params.GetPairingInstr().Value(),
                 params.GetPairingHint().Value());
        textEntries[textEntrySize++] = { "P", reinterpret_cast<const uint8_t *>(pairingInstrBuf),
                                         strnlen(pairingInstrBuf, sizeof(pairingInstrBuf)) };
    }

    snprintf(shortDiscriminatorSubtype, sizeof(shortDiscriminatorSubtype), "_S%03u", params.GetShortDiscriminator());
    subTypes[subTypeSize++] = shortDiscriminatorSubtype;
    snprintf(longDiscriminatorSubtype, sizeof(longDiscriminatorSubtype), "_L%04u", params.GetLongDiscriminator());
    subTypes[subTypeSize++] = longDiscriminatorSubtype;
    if (params.GetVendorId().HasValue())
    {
        snprintf(vendorSubType, sizeof(vendorSubType), "_V%u", params.GetVendorId().Value());
        subTypes[subTypeSize++] = vendorSubType;
    }

    service.mTextEntryies  = textEntries;
    service.mTextEntrySize = textEntrySize;
    service.mPort          = CHIP_PORT;
    service.mInterface     = INET_NULL_INTERFACEID;
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;
    service.mAddressType   = Inet::kIPAddressType_Any;
    error                  = ChipMdnsPublishService(&service);

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::Advertise(const OperationalAdvertisingParameters & params)
{
    MdnsService service;
    CHIP_ERROR error = CHIP_NO_ERROR;

    mOperationalAdvertisingParams = params;
    // TODO: There may be multilple device/fabrid ids after multi-admin.
    snprintf(service.mName, sizeof(service.mName), "%" PRIX64 "-%" PRIX64, params.GetNodeId(), params.GetFabricId());
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mPort          = CHIP_PORT;
    service.mTextEntryies  = nullptr;
    service.mTextEntrySize = 0;
    service.mInterface     = INET_NULL_INTERFACEID;
    service.mAddressType   = Inet::kIPAddressType_Any;
    error                  = ChipMdnsPublishService(&service);

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::StopPublishDevice()
{
    mIsOperationalPublishing  = false;
    mIsCommissionalPublishing = false;
    return ChipMdnsStopPublish();
}

CHIP_ERROR DiscoveryImplPlatform::RegisterResolveDelegate(ResolveDelegate * delegate)
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

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(uint64_t nodeId, uint64_t fabricId, Inet::IPAddressType type)
{
    MdnsService service;

    snprintf(service.mName, sizeof(service.mName), "%" PRIX64 "-%" PRIX64, nodeId, fabricId);
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol    = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mAddressType = type;
    return ChipMdnsResolve(&service, INET_NULL_INTERFACEID, HandleNodeIdResolve, this);
}

void DiscoveryImplPlatform::HandleNodeIdResolve(void * context, MdnsService * result, CHIP_ERROR error)
{
    DiscoveryImplPlatform * mgr = static_cast<DiscoveryImplPlatform *>(context);

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
}

ServiceAdvertiser & chip::Mdns::ServiceAdvertiser::Instance()
{
    return DiscoveryImplPlatform::GetInstance();
}

} // namespace Mdns
} // namespace chip
