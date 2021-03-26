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

#include "ServiceNaming.h"
#include "lib/core/CHIPSafeCasts.h"
#include "lib/mdns/platform/Mdns.h"
#include "lib/support/ReturnMacros.h"
#include "lib/support/logging/CHIPLogging.h"
#include "platform/CHIPDeviceConfig.h"
#include "platform/CHIPDeviceLayer.h"
#include "setup_payload/AdditionalDataPayloadGenerator.h"
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

DiscoveryImplPlatform::DiscoveryImplPlatform() = default;

CHIP_ERROR DiscoveryImplPlatform::Init()
{
    if (!mMdnsInitialized)
    {
        ReturnErrorOnFailure(ChipMdnsInit(HandleMdnsInit, HandleMdnsError, this));
        mCommissionInstanceName = GetRandU64();
        mMdnsInitialized        = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::Start(Inet::InetLayer * inetLayer, uint16_t port)
{
    ReturnErrorOnFailure(Init());

    CHIP_ERROR error = ChipMdnsStopPublish();

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to initialize platform mdns: %s", ErrorStr(error));
    }

    error = SetupHostname();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to setup mdns hostname: %s", ErrorStr(error));
    }

    return error;
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
        publisher->mMdnsInitialized = false;
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

#if CHIP_ENABLE_ROTATING_DEVICE_ID
CHIP_ERROR DiscoveryImplPlatform::GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t & rotatingDeviceIdHexBufferSize)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    char serialNumber[chip::DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];
    size_t serialNumberSize  = 0;
    uint16_t lifetimeCounter = 0;
    SuccessOrExit(error =
                      chip::DeviceLayer::ConfigurationMgr().GetSerialNumber(serialNumber, sizeof(serialNumber), serialNumberSize));
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetLifetimeCounter(lifetimeCounter));
    SuccessOrExit(error = AdditionalDataPayloadGenerator().generateRotatingDeviceId(
                      lifetimeCounter, serialNumber, serialNumberSize, rotatingDeviceIdHexBuffer, rotatingDeviceIdHexBufferSize,
                      rotatingDeviceIdHexBufferSize));
exit:
    return error;
}
#endif

CHIP_ERROR DiscoveryImplPlatform::SetupHostname()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    static char hostname[17]; // Hostname is 64-bit EUI-64 expressed as a 16-character hexadecimal string.
    uint8_t eui64[8];
    chip::DeviceLayer::ThreadStackMgr().GetFactoryAssignedEUI64(eui64);
    snprintf(hostname, sizeof(hostname), "%02X%02X%02X%02X%02X%02X%02X%02X", eui64[0], eui64[1], eui64[2], eui64[3], eui64[4],
             eui64[5], eui64[6], eui64[7]);
#else
    uint8_t mac[6];    // 6 byte wifi mac
    char hostname[13]; // Hostname will be the hex representation of mac.

    ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().GetPrimaryWiFiMACAddress(mac));
    for (size_t i = 0; i < sizeof(mac); i++)
    {
        snprintf(&hostname[i * 2], sizeof(hostname) - i * 2, "%02X", mac[i]);
    }
#endif

    ReturnErrorOnFailure(ChipMdnsSetHostname(hostname));

    return CHIP_NO_ERROR;
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
    char longDiscriminatorSubtype[8];
    char vendorSubType[8];
    const char * subTypes[3];
    size_t subTypeSize = 0;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    size_t rotatingDeviceIdHexBufferSize = 0;
#endif

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
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    if (textEntrySize < ArraySize(textEntries))
    {
        ReturnErrorOnFailure(GenerateRotatingDeviceId(rotatingDeviceIdHexBuffer, rotatingDeviceIdHexBufferSize));
        // Rotating Device ID

        textEntries[textEntrySize++] = { "RI", Uint8::from_const_char(rotatingDeviceIdHexBuffer), rotatingDeviceIdHexBufferSize };
    }
    else
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }
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

    service.mTextEntries   = textEntries;
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

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), params.GetFabricId(), params.GetNodeId()));
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mPort          = CHIP_PORT;
    service.mTextEntries   = nullptr;
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

CHIP_ERROR DiscoveryImplPlatform::SetResolverDelegate(ResolverDelegate * delegate)
{
    VerifyOrReturnError(delegate == nullptr || mResolverDelegate == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mResolverDelegate = delegate;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(uint64_t nodeId, uint64_t fabricId, Inet::IPAddressType type)
{
    ReturnErrorOnFailure(Init());

    MdnsService service;

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), fabricId, nodeId));
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol    = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mAddressType = type;
    return ChipMdnsResolve(&service, INET_NULL_INTERFACEID, HandleNodeIdResolve, this);
}

void DiscoveryImplPlatform::HandleNodeIdResolve(void * context, MdnsService * result, CHIP_ERROR error)
{
    DiscoveryImplPlatform * mgr = static_cast<DiscoveryImplPlatform *>(context);

    if (mgr->mResolverDelegate == nullptr)
    {
        return;
    }

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Node ID resolved failed with %s", chip::ErrorStr(error));
        mgr->mResolverDelegate->OnNodeIdResolutionFailed(kUndefinedNodeId, error);
        return;
    }

    if (result == nullptr)
    {
        ChipLogError(Discovery, "Node ID resolve not found");
        mgr->mResolverDelegate->OnNodeIdResolutionFailed(kUndefinedNodeId, CHIP_ERROR_UNKNOWN_RESOURCE_ID);
        return;
    }

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

    ResolvedNodeData nodeData;
    nodeData.mInterfaceId = result->mInterface;
    nodeData.mAddress     = result->mAddress.ValueOr({});
    nodeData.mPort        = result->mPort;

    if (deliminatorFound)
    {
        ChipLogProgress(Discovery, "Node ID resolved for %" PRIX64, nodeId);
        mgr->mResolverDelegate->OnNodeIdResolved(nodeId, nodeData);
    }
    else
    {
        ChipLogProgress(Discovery, "Invalid service entry from node %" PRIX64, nodeId);
        mgr->mResolverDelegate->OnNodeIdResolved(kUndefinedNodeId, nodeData);
    }
}

DiscoveryImplPlatform & DiscoveryImplPlatform::GetInstance()
{
    return sManager;
}

ServiceAdvertiser & chip::Mdns::ServiceAdvertiser::Instance()
{
    return DiscoveryImplPlatform::GetInstance();
}

Resolver & chip::Mdns::Resolver::Instance()
{
    return DiscoveryImplPlatform::GetInstance();
}

} // namespace Mdns
} // namespace chip
