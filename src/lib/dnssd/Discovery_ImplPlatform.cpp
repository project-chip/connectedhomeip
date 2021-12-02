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

#include <lib/dnssd/Discovery_ImplPlatform.h>

#include <inttypes.h>

#include <crypto/RandUtils.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/dnssd/DnssdCache.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Dnssd {

DiscoveryImplPlatform DiscoveryImplPlatform::sManager;
#if CHIP_CONFIG_MDNS_CACHE_SIZE > 0
DnssdCache<CHIP_CONFIG_MDNS_CACHE_SIZE> DiscoveryImplPlatform::sDnssdCache;
#endif

DiscoveryImplPlatform::DiscoveryImplPlatform() = default;

CHIP_ERROR DiscoveryImplPlatform::InitImpl()
{
    ReturnErrorCodeIf(mDnssdInitialized, CHIP_NO_ERROR);
    ReturnErrorOnFailure(ChipDnssdInit(HandleDnssdInit, HandleDnssdError, this));

    uint64_t random_instance_name = chip::Crypto::GetRandU64();
    memcpy(&mCommissionableInstanceName[0], &random_instance_name, sizeof(mCommissionableInstanceName));

    return CHIP_NO_ERROR;
}

void DiscoveryImplPlatform::Shutdown()
{
    VerifyOrReturn(mDnssdInitialized);
    ChipDnssdShutdown();
}

void DiscoveryImplPlatform::HandleDnssdInit(void * context, CHIP_ERROR initError)
{
    DiscoveryImplPlatform * publisher = static_cast<DiscoveryImplPlatform *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mDnssdInitialized = true;

#if !CHIP_DEVICE_LAYER_NONE
        // Post an event that will start advertising
        chip::DeviceLayer::ChipDeviceEvent event;
        event.Type = chip::DeviceLayer::DeviceEventType::kDnssdPlatformInitialized;

        CHIP_ERROR error = chip::DeviceLayer::PlatformMgr().PostEvent(&event);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Posting DNS-SD platform initialized event failed with %s", chip::ErrorStr(error));
        }
#endif
    }
    else
    {
        ChipLogError(Discovery, "DNS-SD initialization failed with %s", chip::ErrorStr(initError));
        publisher->mDnssdInitialized = false;
    }
}

void DiscoveryImplPlatform::HandleDnssdError(void * context, CHIP_ERROR error)
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
        publisher->FinalizeServiceUpdate();
    }
    else
    {
        ChipLogError(Discovery, "DNS-SD error: %s", chip::ErrorStr(error));
    }
}

CHIP_ERROR DiscoveryImplPlatform::GetCommissionableInstanceName(char * instanceName, size_t maxLength)
{
    if (maxLength < (chip::Dnssd::Commissionable::kInstanceNameMaxLength + 1))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return chip::Encoding::BytesToUppercaseHexString(&mCommissionableInstanceName[0], sizeof(mCommissionableInstanceName),
                                                     instanceName, maxLength);
}

template <class Derived, size_t N_idle, size_t N_active, size_t N_tcp>
CHIP_ERROR AddCommonTxtElements(const BaseAdvertisingParams<Derived> & params, char (&mrpRetryIdleStorage)[N_idle],
                                char (&mrpRetryActiveStorage)[N_active], char (&tcpSupportedStorage)[N_tcp],
                                TextEntry txtEntryStorage[], size_t & txtEntryIdx)
{
    auto optionalMrp = params.GetMRPConfig();

    // TODO: Issue #5833 - MRP retry intervals should be updated on the poll period value
    // change or device type change.
    // TODO: Is this really the best place to set these? Seems like it should be passed
    // in with the correct values and set one level up from here.
#if CHIP_DEVICE_CONFIG_ENABLE_SED
    chip::DeviceLayer::ConnectivityManager::SEDPollingConfig sedPollingConfig;
    ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().GetSEDPollingConfig(sedPollingConfig));
    // Increment default MRP retry intervals by SED poll period to be on the safe side
    // and avoid unnecessary retransmissions.
    if (optionalMrp.HasValue())
    {
        auto mrp = optionalMrp.Value();
        optionalMrp.SetValue(ReliableMessageProtocolConfig(mrp.mIdleRetransTimeout + sedPollingConfig.SlowPollingIntervalMS,
                                                           mrp.mActiveRetransTimeout + sedPollingConfig.FastPollingIntervalMS));
    }
#endif
    if (optionalMrp.HasValue())
    {
        auto mrp = optionalMrp.Value();
        {
            if (mrp.mIdleRetransTimeout > kMaxRetryInterval)
            {
                ChipLogProgress(Discovery,
                                "MRP retry interval idle value exceeds allowed range of 1 hour, using maximum available");
                mrp.mIdleRetransTimeout = kMaxRetryInterval;
            }
            size_t writtenCharactersNumber =
                snprintf(mrpRetryIdleStorage, sizeof(mrpRetryIdleStorage), "%" PRIu32, mrp.mIdleRetransTimeout.count());
            VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber <= kTxtRetryIntervalIdleMaxLength),
                                CHIP_ERROR_INVALID_STRING_LENGTH);
            txtEntryStorage[txtEntryIdx++] = { "CRI", Uint8::from_const_char(mrpRetryIdleStorage), strlen(mrpRetryIdleStorage) };
        }

        {
            if (mrp.mActiveRetransTimeout > kMaxRetryInterval)
            {
                ChipLogProgress(Discovery,
                                "MRP retry interval active value exceeds allowed range of 1 hour, using maximum available");
                mrp.mActiveRetransTimeout = kMaxRetryInterval;
            }
            size_t writtenCharactersNumber =
                snprintf(mrpRetryActiveStorage, sizeof(mrpRetryActiveStorage), "%" PRIu32, mrp.mActiveRetransTimeout.count());
            VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber <= kTxtRetryIntervalActiveMaxLength),
                                CHIP_ERROR_INVALID_STRING_LENGTH);
            txtEntryStorage[txtEntryIdx++] = { "CRA", Uint8::from_const_char(mrpRetryActiveStorage),
                                               strlen(mrpRetryActiveStorage) };
        }
    }
    if (params.GetTcpSupported().HasValue())
    {
        size_t writtenCharactersNumber =
            snprintf(tcpSupportedStorage, sizeof(tcpSupportedStorage), "%d", params.GetTcpSupported().Value());
        VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber <= kKeyTcpSupportMaxLength),
                            CHIP_ERROR_INVALID_STRING_LENGTH);
        txtEntryStorage[txtEntryIdx++] = { "T", reinterpret_cast<const uint8_t *>(tcpSupportedStorage),
                                           strlen(tcpSupportedStorage) };
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::Advertise(const CommissionAdvertisingParameters & params)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    DnssdService service;
    // add newline to lengths for TXT entries
    char discriminatorBuf[kKeyDiscriminatorMaxLength + 1];
    char vendorProductBuf[kKeyVendorProductMaxLength + 1];
    char commissioningModeBuf[kKeyCommissioningModeMaxLength + 1];
    char deviceTypeBuf[kKeyDeviceTypeMaxLength + 1];
    char deviceNameBuf[kKeyDeviceNameMaxLength + 1];
    char rotatingIdBuf[kKeyRotatingIdMaxLength + 1];
    char pairingHintBuf[kKeyPairingHintMaxLength + 1];
    char pairingInstrBuf[kKeyPairingInstructionMaxLength + 1];
    char mrpRetryIntervalIdleBuf[kTxtRetryIntervalIdleMaxLength + 1];
    char mrpRetryIntervalActiveBuf[kTxtRetryIntervalActiveMaxLength + 1];
    char tcpSupportedBuf[kKeyTcpSupportMaxLength + 1];
    // size of textEntries array should be count of Bufs above
    TextEntry textEntries[CommissionAdvertisingParameters::kTxtMaxNumber];
    size_t textEntrySize = 0;
    // add null-character to the subtypes
    char shortDiscriminatorSubtype[kSubTypeShortDiscriminatorMaxLength + 1];
    char longDiscriminatorSubtype[kSubTypeLongDiscriminatorMaxLength + 1];
    char vendorSubType[kSubTypeVendorMaxLength + 1];
    char commissioningModeSubType[kSubTypeCommissioningModeMaxLength + 1];
    char deviceTypeSubType[kSubTypeDeviceTypeMaxLength + 1];
    // size of subTypes array should be count of SubTypes above
    const char * subTypes[Commissionable::kSubTypeMaxNumber];
    size_t subTypeSize = 0;

    if (!mDnssdInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    error = MakeHostName(service.mHostName, sizeof(service.mHostName), params.GetMac());
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to create dnssd hostname: %s", ErrorStr(error));
        return error;
    }

    ReturnErrorOnFailure(GetCommissionableInstanceName(service.mName, sizeof(service.mName)));

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        strncpy(service.mType, kCommissionableServiceName, sizeof(service.mType));
    }
    else
    {
        strncpy(service.mType, kCommissionerServiceName, sizeof(service.mType));
    }
    service.mProtocol = DnssdServiceProtocol::kDnssdProtocolUdp;

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
    AddCommonTxtElements<CommissionAdvertisingParameters>(params, mrpRetryIntervalIdleBuf, mrpRetryIntervalActiveBuf,
                                                          tcpSupportedBuf, textEntries, textEntrySize);

    // Following fields are for nodes and not for commissioners
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        snprintf(discriminatorBuf, sizeof(discriminatorBuf), "%u", params.GetLongDiscriminator());
        textEntries[textEntrySize++] = { "D", reinterpret_cast<const uint8_t *>(discriminatorBuf),
                                         strnlen(discriminatorBuf, sizeof(discriminatorBuf)) };

        snprintf(commissioningModeBuf, sizeof(commissioningModeBuf), "%u", static_cast<int>(params.GetCommissioningMode()));
        textEntries[textEntrySize++] = { "CM", reinterpret_cast<const uint8_t *>(commissioningModeBuf),
                                         strnlen(commissioningModeBuf, sizeof(commissioningModeBuf)) };

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
        if ((params.GetCommissioningMode() != CommissioningMode::kDisabled) &&
            (MakeServiceSubtype(commissioningModeSubType, sizeof(commissioningModeSubType),
                                DiscoveryFilter(DiscoveryFilterType::kCommissioningMode)) == CHIP_NO_ERROR))
        {
            subTypes[subTypeSize++] = commissioningModeSubType;
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
    service.mPort          = params.GetPort();
    service.mInterface     = Inet::InterfaceId::Null();
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;
    service.mAddressType   = Inet::IPAddressType::kAny;
    error                  = ChipDnssdPublishService(&service);

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
void DiscoveryImplPlatform::PrintEntries(const DnssdService * service)
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
    DnssdService service;
    CHIP_ERROR error = CHIP_NO_ERROR;

    char compressedFabricIdSub[kSubTypeCompressedFabricIdMaxLength + 1];
    const char * subTypes[Operational::kSubTypeMaxNumber];
    size_t subTypeSize = 0;

    mOperationalAdvertisingParams = params;
    // TODO: There may be multilple device/fabric ids after multi-admin.

    char mrpRetryIntervalIdleBuf[kTxtRetryIntervalIdleMaxLength + 1];
    char mrpRetryIntervalActiveBuf[kTxtRetryIntervalActiveMaxLength + 1];
    char tcpSupportedBuf[kKeyTcpSupportMaxLength + 1];
    TextEntry txtEntries[OperationalAdvertisingParameters::kTxtMaxNumber];
    size_t textEntrySize = 0;

    ReturnLogErrorOnFailure(AddCommonTxtElements(params, mrpRetryIntervalIdleBuf, mrpRetryIntervalActiveBuf, tcpSupportedBuf,
                                                 txtEntries, textEntrySize));

    if (MakeServiceSubtype(compressedFabricIdSub, sizeof(compressedFabricIdSub),
                           DiscoveryFilter(DiscoveryFilterType::kCompressedFabricId, params.GetPeerId().GetCompressedFabricId())) ==
        CHIP_NO_ERROR)
    {
        subTypes[subTypeSize++] = compressedFabricIdSub;
    }
    error = MakeHostName(service.mHostName, sizeof(service.mHostName), params.GetMac());
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to create dnssd hostname: %s", ErrorStr(error));
        return error;
    }
    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), params.GetPeerId()));
    strncpy(service.mType, kOperationalServiceName, sizeof(service.mType));
    service.mProtocol      = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mPort          = params.GetPort();
    service.mTextEntries   = txtEntries;
    service.mTextEntrySize = textEntrySize;
    service.mInterface     = Inet::InterfaceId::Null();
    service.mAddressType   = Inet::IPAddressType::kAny;
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;
    error                  = ChipDnssdPublishService(&service);

    if (error == CHIP_NO_ERROR)
    {
        mIsOperationalPublishing = true;
    }

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::RemoveServices()
{
    ReturnErrorOnFailure(ChipDnssdRemoveServices());

    mIsOperationalPublishing        = false;
    mIsCommissionableNodePublishing = false;
    mIsCommissionerPublishing       = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::FinalizeServiceUpdate()
{
    return ChipDnssdFinalizeServiceUpdate();
}

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type,
                                                Resolver::CacheBypass dnssdCacheBypass)
{
    ReturnErrorOnFailure(InitImpl());

#if CHIP_CONFIG_MDNS_CACHE_SIZE > 0
    if (dnssdCacheBypass == Resolver::CacheBypass::Off)
    {
        /* see if the entry is cached and use it.... */
        ResolvedNodeData nodeData;
        if (sDnssdCache.Lookup(peerId, nodeData) == CHIP_NO_ERROR)
        {
            mResolverDelegate->OnNodeIdResolved(nodeData);
            return CHIP_NO_ERROR;
        }
    }
#endif

    DnssdService service;

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), peerId));
    strncpy(service.mType, kOperationalServiceName, sizeof(service.mType));
    service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mAddressType = type;
    return ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeIdResolve, this);
}

void DiscoveryImplPlatform::HandleNodeBrowse(void * context, DnssdService * services, size_t servicesSize, CHIP_ERROR error)
{
    for (size_t i = 0; i < servicesSize; ++i)
    {
        // For some platforms browsed services are already resolved, so verify if resolve is really needed or call resolve callback
        if (!services[i].mAddress.HasValue())
        {
            ChipDnssdResolve(&services[i], services[i].mInterface, HandleNodeResolve, context);
        }
        else
        {
            HandleNodeResolve(context, &services[i], error);
        }
    }
}

void DiscoveryImplPlatform::HandleNodeResolve(void * context, DnssdService * result, CHIP_ERROR error)
{
    if (error != CHIP_NO_ERROR)
    {
        return;
    }
    DiscoveryImplPlatform * mgr = static_cast<DiscoveryImplPlatform *>(context);
    DiscoveredNodeData data;
    Platform::CopyString(data.hostName, result->mHostName);
    Platform::CopyString(data.instanceName, result->mName);

    if (result->mAddress.HasValue() && data.numIPs < DiscoveredNodeData::kMaxIPAddresses)
    {
        data.ipAddress[data.numIPs]   = result->mAddress.Value();
        data.interfaceId[data.numIPs] = result->mInterface;
        data.numIPs++;
    }

    data.port = result->mPort;

    for (size_t i = 0; i < result->mTextEntrySize; ++i)
    {
        ByteSpan key(reinterpret_cast<const uint8_t *>(result->mTextEntries[i].mKey), strlen(result->mTextEntries[i].mKey));
        ByteSpan val(result->mTextEntries[i].mData, result->mTextEntries[i].mDataSize);
        FillNodeDataFromTxt(key, val, data);
    }
    mgr->mResolverDelegate->OnNodeDiscoveryComplete(data);
}

CHIP_ERROR DiscoveryImplPlatform::FindCommissionableNodes(DiscoveryFilter filter)
{
    ReturnErrorOnFailure(InitImpl());
    char serviceName[kMaxCommissionableServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionableNode));

    return ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                           Inet::InterfaceId::Null(), HandleNodeBrowse, this);
}

CHIP_ERROR DiscoveryImplPlatform::FindCommissioners(DiscoveryFilter filter)
{
    ReturnErrorOnFailure(InitImpl());
    char serviceName[kMaxCommissionerServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionerNode));

    return ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                           Inet::InterfaceId::Null(), HandleNodeBrowse, this);
}

void DiscoveryImplPlatform::HandleNodeIdResolve(void * context, DnssdService * result, CHIP_ERROR error)
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

    // TODO: Expand the results to include all the addresses.
    Platform::CopyString(nodeData.mHostName, result->mHostName);
    nodeData.mInterfaceId = result->mInterface;
    nodeData.mAddress[0]  = result->mAddress.ValueOr({});
    nodeData.mPort        = result->mPort;
    nodeData.mNumIPs      = 1;
    // TODO: Use seconds?
    const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

    nodeData.mExpiryTime = currentTime + System::Clock::Seconds16(result->mTtlSeconds);

    for (size_t i = 0; i < result->mTextEntrySize; ++i)
    {
        ByteSpan key(reinterpret_cast<const uint8_t *>(result->mTextEntries[i].mKey), strlen(result->mTextEntries[i].mKey));
        ByteSpan val(result->mTextEntries[i].mData, result->mTextEntries[i].mDataSize);
        FillNodeDataFromTxt(key, val, nodeData);
    }

    nodeData.LogNodeIdResolved();
#if CHIP_CONFIG_MDNS_CACHE_SIZE > 0
    error = mgr->sDnssdCache.Insert(nodeData);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Discovery, "DnssdCache insert failed with %s", chip::ErrorStr(error));
    }
#endif
    mgr->mResolverDelegate->OnNodeIdResolved(nodeData);
}

DiscoveryImplPlatform & DiscoveryImplPlatform::GetInstance()
{
    return sManager;
}

ServiceAdvertiser & chip::Dnssd::ServiceAdvertiser::Instance()
{
    return DiscoveryImplPlatform::GetInstance();
}

Resolver & chip::Dnssd::Resolver::Instance()
{
    return DiscoveryImplPlatform::GetInstance();
}

} // namespace Dnssd
} // namespace chip
