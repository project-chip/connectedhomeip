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
        // TODO: revisit (value is never true)
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

CHIP_ERROR DiscoveryImplPlatform::SetupHostname(chip::ByteSpan macOrEui64)
{
    char nameBuffer[17];
    CHIP_ERROR error = MakeHostName(nameBuffer, sizeof(nameBuffer), macOrEui64);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to create mdns hostname: %s", ErrorStr(error));
        return error;
    }
    error = ChipMdnsSetHostname(nameBuffer);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to setup mdns hostname: %s", ErrorStr(error));
        return error;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::Advertise(const CommissionAdvertisingParameters & params)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    MdnsService service;
    char discriminatorBuf[6];
    char vendorProductBuf[12];
    char commissioningModeBuf[2];
    char additionalPairingBuf[2];
    char deviceTypeBuf[6];
    char deviceNameBuf[33];
    char rotatingIdBuf[37];
    char pairingHintBuf[11];
    char pairingInstrBuf[129];
    TextEntry textEntries[9];
    size_t textEntrySize = 0;
    char shortDiscriminatorSubtype[6];
    char longDiscriminatorSubtype[8];
    char vendorSubType[8];
    char commissioningModeSubType[4];
    char openWindowSubType[4];
    char deviceTypeSubType[8];
    const char * subTypes[6];
    size_t subTypeSize = 0;
    
    if (!mMdnsInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ReturnErrorOnFailure(SetupHostname(params.GetMac()));

    snprintf(service.mName, sizeof(service.mName), "%016" PRIX64, mCommissionInstanceName);
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        strncpy(service.mType, "_chipc", sizeof(service.mType));
    }
    else
    {
        strncpy(service.mType, "_chipd", sizeof(service.mType));
    }
    service.mProtocol = MdnsServiceProtocol::kMdnsProtocolUdp;

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

    if (params.GetDeviceType().HasValue())
    {
        snprintf(deviceTypeBuf, sizeof(deviceTypeBuf), "%u", params.GetDeviceType().Value());
        textEntries[textEntrySize++] = { "DT", reinterpret_cast<const uint8_t *>(deviceTypeBuf),
                                         strnlen(deviceTypeBuf, sizeof(deviceTypeBuf)) };
    }

    if (params.GetDeviceName().HasValue())
    {
        snprintf(deviceNameBuf, sizeof(deviceNameBuf), "%s", params.GetDeviceName().Value());
        textEntries[textEntrySize++] = { "DN", reinterpret_cast<const uint8_t *>(deviceNameBuf),
                                         strnlen(deviceNameBuf, sizeof(deviceNameBuf)) };
    }

    // Following fields are for nodes and not for commissioners
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        snprintf(discriminatorBuf, sizeof(discriminatorBuf), "%04u", params.GetLongDiscriminator());
        textEntries[textEntrySize++] = { "D", reinterpret_cast<const uint8_t *>(discriminatorBuf),
                                        strnlen(discriminatorBuf, sizeof(discriminatorBuf)) };
    
        snprintf(commissioningModeBuf, sizeof(commissioningModeBuf), "%u", params.GetCommissioningMode() ? 1 : 0);
        textEntries[textEntrySize++] = { "CM", reinterpret_cast<const uint8_t *>(commissioningModeBuf),
                                            strnlen(commissioningModeBuf, sizeof(commissioningModeBuf)) };
                                            
        if (params.GetCommissioningMode() && params.GetOpenWindowCommissioningMode()) 
        {
            snprintf(additionalPairingBuf, sizeof(additionalPairingBuf), "1");
            textEntries[textEntrySize++] = { "AP", reinterpret_cast<const uint8_t *>(additionalPairingBuf),
                                                strnlen(additionalPairingBuf, sizeof(additionalPairingBuf)) };
        }

        if (params.GetRotatingId().HasValue())
        {
            snprintf(rotatingIdBuf, sizeof(rotatingIdBuf), "%s", params.GetRotatingId().Value());
            textEntries[textEntrySize++] = { "RI", reinterpret_cast<const uint8_t *>(rotatingIdBuf),
                                            strnlen(rotatingIdBuf, sizeof(rotatingIdBuf)) };
        }

        if (params.GetPairingHint().HasValue())
        {
            snprintf(pairingHintBuf, sizeof(pairingHintBuf), "%u", params.GetPairingHint().Value());
            textEntries[textEntrySize++] = { "PH", reinterpret_cast<const uint8_t *>(pairingHintBuf),
                                            strnlen(pairingHintBuf, sizeof(pairingHintBuf)) };
        }

        if (params.GetPairingInstr().HasValue())
        {
            snprintf(pairingInstrBuf, sizeof(pairingInstrBuf), "%s", params.GetPairingInstr().Value());
            textEntries[textEntrySize++] = { "PI", reinterpret_cast<const uint8_t *>(pairingInstrBuf),
                                            strnlen(pairingInstrBuf, sizeof(pairingInstrBuf)) };
        }

        snprintf(shortDiscriminatorSubtype, sizeof(shortDiscriminatorSubtype), "_S%03u", params.GetShortDiscriminator());
        subTypes[subTypeSize++] = shortDiscriminatorSubtype;
        snprintf(longDiscriminatorSubtype, sizeof(longDiscriminatorSubtype), "_L%04u", params.GetLongDiscriminator());
        subTypes[subTypeSize++] = longDiscriminatorSubtype;
        snprintf(commissioningModeSubType, sizeof(commissioningModeSubType), "_C%u", params.GetCommissioningMode() ? 1 : 0);
        subTypes[subTypeSize++] = commissioningModeSubType;
        if (params.GetCommissioningMode() && params.GetOpenWindowCommissioningMode()) 
        {
            snprintf(openWindowSubType, sizeof(openWindowSubType), "_A1");
            subTypes[subTypeSize++] = openWindowSubType;
        }
    }
    if (params.GetVendorId().HasValue())
    {
        snprintf(vendorSubType, sizeof(vendorSubType), "_V%u", params.GetVendorId().Value());
        subTypes[subTypeSize++] = vendorSubType;
    }
    if (params.GetDeviceType().HasValue())
    {
        snprintf(deviceTypeSubType, sizeof(deviceTypeSubType), "_T%u", params.GetDeviceType().Value());
        subTypes[subTypeSize++] = deviceTypeSubType;
    }

    service.mTextEntries   = textEntries;
    service.mTextEntrySize = textEntrySize;
    service.mPort          = CHIP_PORT;
    service.mInterface     = INET_NULL_INTERFACEID;
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;
    service.mAddressType   = Inet::kIPAddressType_Any;
    error                  = ChipMdnsPublishService(&service);

    PrintEntries(&service);
    return error;
}

void DiscoveryImplPlatform::PrintEntries(const MdnsService * service) 
{
    printf("printEntries port=%d, mTextEntrySize=%d, mSubTypeSize=%d\n", (int)(service->mPort), (int)(service->mTextEntrySize), (int)(service->mSubTypeSize));
    for (int i=0; i<(int)service->mTextEntrySize; i++) 
    {
        printf(" entry [%d] : %s %s\n", i, service->mTextEntries[i].mKey, (char*)(service->mTextEntries[i].mData));
    }

    for (int i=0; i<(int)service->mSubTypeSize; i++) 
    {
        printf(" type [%d] : %s\n", i, service->mSubTypes[i]);
    }
}

CHIP_ERROR DiscoveryImplPlatform::Advertise(const OperationalAdvertisingParameters & params)
{
    MdnsService service;
    CHIP_ERROR error = CHIP_NO_ERROR;

    mOperationalAdvertisingParams = params;
    // TODO: There may be multilple device/fabrid ids after multi-admin.

    // According to spec CRI and CRA intervals should not exceed 1 hour (3600000 ms).
    // TODO: That value should be defined in the ReliableMessageProtocolConfig.h,
    // but for now it is not possible to access it from src/lib/mdns. It should be
    // refactored after creating common DNS-SD layer.
    constexpr uint32_t kMaxCRMPRetryInterval = 3600000;
    // kMaxCRMPRetryInterval max value is 3600000, what gives 7 characters and newline
    // necessary to represent it in the text form.
    constexpr uint8_t kMaxCRMPRetryBufferSize = 7 + 1;
    char crmpRetryIntervalIdleBuf[kMaxCRMPRetryBufferSize];
    char crmpRetryIntervalActiveBuf[kMaxCRMPRetryBufferSize];
    TextEntry crmpRetryIntervalEntries[2];
    size_t textEntrySize = 0;
    uint32_t crmpRetryIntervalIdle, crmpRetryIntervalActive;
    int writtenCharactersNumber;
    params.GetCRMPRetryIntervals(crmpRetryIntervalIdle, crmpRetryIntervalActive);

    // TODO: Issue #5833 - CRMP retry intervals should be updated on the poll period value
    // change or device type change.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (chip::DeviceLayer::ConnectivityMgr().GetThreadDeviceType() ==
        chip::DeviceLayer::ConnectivityManager::kThreadDeviceType_SleepyEndDevice)
    {
        uint32_t sedPollPeriod;
        ReturnErrorOnFailure(chip::DeviceLayer::ThreadStackMgr().GetPollPeriod(sedPollPeriod));
        // Increment default CRMP retry intervals by SED poll period to be on the safe side
        // and avoid unnecessary retransmissions.
        crmpRetryIntervalIdle += sedPollPeriod;
        crmpRetryIntervalActive += sedPollPeriod;
    }
#endif

    if (crmpRetryIntervalIdle > kMaxCRMPRetryInterval)
    {
        ChipLogProgress(Discovery, "CRMP retry interval idle value exceeds allowed range of 1 hour, using maximum available",
                        chip::ErrorStr(error));
        crmpRetryIntervalIdle = kMaxCRMPRetryInterval;
    }
    writtenCharactersNumber = snprintf(crmpRetryIntervalIdleBuf, sizeof(crmpRetryIntervalIdleBuf), "%u", crmpRetryIntervalIdle);
    VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber < kMaxCRMPRetryBufferSize),
                        CHIP_ERROR_INVALID_STRING_LENGTH);
    crmpRetryIntervalEntries[textEntrySize++] = { "CRI", reinterpret_cast<const uint8_t *>(crmpRetryIntervalIdleBuf),
                                                  strlen(crmpRetryIntervalIdleBuf) };

    if (crmpRetryIntervalActive > kMaxCRMPRetryInterval)
    {
        ChipLogProgress(Discovery, "CRMP retry interval active value exceeds allowed range of 1 hour, using maximum available",
                        chip::ErrorStr(error));
        crmpRetryIntervalActive = kMaxCRMPRetryInterval;
    }
    writtenCharactersNumber =
        snprintf(crmpRetryIntervalActiveBuf, sizeof(crmpRetryIntervalActiveBuf), "%u", crmpRetryIntervalActive);
    VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber < kMaxCRMPRetryBufferSize),
                        CHIP_ERROR_INVALID_STRING_LENGTH);
    crmpRetryIntervalEntries[textEntrySize++] = { "CRA", reinterpret_cast<const uint8_t *>(crmpRetryIntervalActiveBuf),
                                                  strlen(crmpRetryIntervalActiveBuf) };

    ReturnErrorOnFailure(SetupHostname(params.GetMac()));
    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), params.GetFabricId(), params.GetNodeId()));
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mPort          = CHIP_PORT;
    service.mTextEntries   = crmpRetryIntervalEntries;
    service.mTextEntrySize = textEntrySize;

    service.mInterface   = INET_NULL_INTERFACEID;
    service.mAddressType = Inet::kIPAddressType_Any;
    printf("DiscoveryImplPlatform::Advertise calling ChipMdnsPublishService\n");
    error                = ChipMdnsPublishService(&service);

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
        }
        else if (deliminatorFound)
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
