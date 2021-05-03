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

    ReturnErrorOnFailure(SetupHostname(params.GetMac()));

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
    // TODO: There may be multilple device/fabric ids after multi-admin.

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
    writtenCharactersNumber =
        snprintf(crmpRetryIntervalIdleBuf, sizeof(crmpRetryIntervalIdleBuf), "%" PRIu32, crmpRetryIntervalIdle);
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
        snprintf(crmpRetryIntervalActiveBuf, sizeof(crmpRetryIntervalActiveBuf), "%" PRIu32, crmpRetryIntervalActive);
    VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber < kMaxCRMPRetryBufferSize),
                        CHIP_ERROR_INVALID_STRING_LENGTH);
    crmpRetryIntervalEntries[textEntrySize++] = { "CRA", reinterpret_cast<const uint8_t *>(crmpRetryIntervalActiveBuf),
                                                  strlen(crmpRetryIntervalActiveBuf) };

    ReturnErrorOnFailure(SetupHostname(params.GetMac()));
    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), params.GetPeerId()));
    strncpy(service.mType, "_chip", sizeof(service.mType));
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    service.mPort          = CHIP_PORT;
    service.mTextEntries   = crmpRetryIntervalEntries;
    service.mTextEntrySize = textEntrySize;
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

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    ReturnErrorOnFailure(Init());

    MdnsService service;

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), peerId));
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
        mgr->mResolverDelegate->OnNodeIdResolutionFailed(PeerId(), error);
        return;
    }

    if (result == nullptr)
    {
        ChipLogError(Discovery, "Node ID resolve not found");
        mgr->mResolverDelegate->OnNodeIdResolutionFailed(PeerId(), CHIP_ERROR_UNKNOWN_RESOURCE_ID);
        return;
    }

    ResolvedNodeData nodeData;

    error = ExtractIdFromInstanceName(result->mName, &nodeData.mPeerId);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Node ID resolved failed with %s", chip::ErrorStr(error));
        mgr->mResolverDelegate->OnNodeIdResolutionFailed(PeerId(), error);
        return;
    }

    nodeData.mInterfaceId = result->mInterface;
    nodeData.mAddress     = result->mAddress.ValueOr({});
    nodeData.mPort        = result->mPort;

    ChipLogProgress(Discovery, "Node ID resolved for %" PRIX64, nodeData.mPeerId.GetNodeId());
    mgr->mResolverDelegate->OnNodeIdResolved(nodeData);
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
