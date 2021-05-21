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
        if (publisher->mIsCommissionableNodePublishing)
        {
            publisher->Advertise(publisher->mCommissionableNodeAdvertisingParams);
        }
        if (publisher->mIsCommissionerPublishing)
        {
            publisher->Advertise(publisher->mCommissionerAdvertisingParams);
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
    // add newline to lengths for TXT entries
    char discriminatorBuf[kKeyDiscriminatorMaxLength + 1];
    char vendorProductBuf[kKeyVendorProductMaxLength + 1];
    char commissioningModeBuf[kKeyCommissioningModeMaxLength + 1];
    char additionalPairingBuf[kKeyAdditionalPairingMaxLength + 1];
    char deviceTypeBuf[kKeyDeviceTypeMaxLength + 1];
    char deviceNameBuf[kKeyDeviceNameMaxLength + 1];
    char rotatingIdBuf[kKeyRotatingIdMaxLength + 1];
    char pairingHintBuf[kKeyPairingHintMaxLength + 1];
    char pairingInstrBuf[kKeyPairingInstructionMaxLength + 1];
    // size of textEntries array should be count of Bufs above
    TextEntry textEntries[9];
    size_t textEntrySize = 0;
    // add underscore, character and newline to lengths for sub types (ex. _S<ddd>)
    char shortDiscriminatorSubtype[kSubTypeShortDiscriminatorMaxLength + 3];
    char longDiscriminatorSubtype[kSubTypeLongDiscriminatorMaxLength + 4];
    char vendorSubType[kSubTypeVendorMaxLength + 3];
    char commissioningModeSubType[kSubTypeCommissioningModeMaxLength + 3];
    char openWindowSubType[kSubTypeAdditionalPairingMaxLength + 3];
    char deviceTypeSubType[kSubTypeDeviceTypeMaxLength + 3];
    // size of subTypes array should be count of SubTypes above
    const char * subTypes[6];
    size_t subTypeSize = 0;

    if (!mMdnsInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ReturnErrorOnFailure(SetupHostname(params.GetMac()));

    snprintf(service.mName, sizeof(service.mName), "%08" PRIX32 "%08" PRIX32, static_cast<uint32_t>(mCommissionInstanceName >> 32),
             static_cast<uint32_t>(mCommissionInstanceName));
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

        if (MakeServiceSubtype(shortDiscriminatorSubtype, sizeof(shortDiscriminatorSubtype),
                               DiscoveryFilter(DiscoveryFilterType::kShort, params.GetShortDiscriminator())) == CHIP_NO_ERROR)
        {
            subTypes[subTypeSize++] = shortDiscriminatorSubtype;
        }
        if (MakeServiceSubtype(longDiscriminatorSubtype, sizeof(longDiscriminatorSubtype),
                               DiscoveryFilter(DiscoveryFilterType::kLong, params.GetLongDiscriminator())) == CHIP_NO_ERROR)
        {
            subTypes[subTypeSize++] = longDiscriminatorSubtype;
        }
        if (MakeServiceSubtype(commissioningModeSubType, sizeof(commissioningModeSubType),
                               DiscoveryFilter(DiscoveryFilterType::kCommissioningMode, params.GetCommissioningMode() ? 1 : 0)) ==
            CHIP_NO_ERROR)
        {
            subTypes[subTypeSize++] = commissioningModeSubType;
        }
        if (params.GetCommissioningMode() && params.GetOpenWindowCommissioningMode())
        {
            if (MakeServiceSubtype(openWindowSubType, sizeof(openWindowSubType),
                                   DiscoveryFilter(DiscoveryFilterType::kCommissioningModeFromCommand, 1)) == CHIP_NO_ERROR)
            {
                subTypes[subTypeSize++] = openWindowSubType;
            }
        }
    }

    if (params.GetVendorId().HasValue())
    {
        if (MakeServiceSubtype(vendorSubType, sizeof(vendorSubType),
                               DiscoveryFilter(DiscoveryFilterType::kVendor, params.GetVendorId().Value())) == CHIP_NO_ERROR)
        {
            subTypes[subTypeSize++] = vendorSubType;
        }
    }
    if (params.GetDeviceType().HasValue())
    {
        if (MakeServiceSubtype(deviceTypeSubType, sizeof(deviceTypeSubType),
                               DiscoveryFilter(DiscoveryFilterType::kDeviceType, params.GetDeviceType().Value())) == CHIP_NO_ERROR)
        {
            subTypes[subTypeSize++] = deviceTypeSubType;
        }
    }

    service.mTextEntries   = textEntries;
    service.mTextEntrySize = textEntrySize;
    service.mPort          = CHIP_PORT;
    service.mInterface     = INET_NULL_INTERFACEID;
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;
    service.mAddressType   = Inet::kIPAddressType_Any;
    error                  = ChipMdnsPublishService(&service);

    if (error == CHIP_NO_ERROR)
    {
        if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
        {
            mCommissionableNodeAdvertisingParams = params;
            mIsCommissionableNodePublishing      = true;
        }
        else
        {
            mCommissionerAdvertisingParams = params;
            mIsCommissionerPublishing      = true;
        }
    }

#ifdef DETAIL_LOGGING
    PrintEntries(&service);
#endif
    return error;
}

#ifdef DETAIL_LOGGING
void DiscoveryImplPlatform::PrintEntries(const MdnsService * service)
{
    printf("printEntries port=%d, mTextEntrySize=%d, mSubTypeSize=%d\n", (int) (service->mPort), (int) (service->mTextEntrySize),
           (int) (service->mSubTypeSize));
    for (int i = 0; i < (int) service->mTextEntrySize; i++)
    {
        printf(" entry [%d] : %s %s\n", i, service->mTextEntries[i].mKey, (char *) (service->mTextEntries[i].mData));
    }

    for (int i = 0; i < (int) service->mSubTypeSize; i++)
    {
        printf(" type [%d] : %s\n", i, service->mSubTypes[i]);
    }
}
#endif

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
    TextEntry crmpRetryIntervalEntries[OperationalAdvertisingParameters::kNumAdvertisingTxtEntries];
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
    service.mSubTypeSize   = 0;
    error                  = ChipMdnsPublishService(&service);

    if (error == CHIP_NO_ERROR)
    {
        mIsOperationalPublishing = true;
    }

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::StopPublishDevice()
{
    CHIP_ERROR error = ChipMdnsStopPublish();

    if (error == CHIP_NO_ERROR)
    {
        mIsOperationalPublishing        = false;
        mIsCommissionableNodePublishing = false;
        mIsCommissionerPublishing       = false;
    }
    return error;
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

    ChipLogProgress(Discovery, "Node ID resolved for 0x" ChipLogFormatX64, ChipLogValueX64(nodeData.mPeerId.GetNodeId()));
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
