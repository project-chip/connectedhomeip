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

namespace {

#if CHIP_CONFIG_MDNS_CACHE_SIZE > 0
static DnssdCache<CHIP_CONFIG_MDNS_CACHE_SIZE> sDnssdCache;
#endif

static void HandleNodeResolve(void * context, DnssdService * result, CHIP_ERROR error)
{
    ResolverDelegateProxy * proxy = static_cast<ResolverDelegateProxy *>(context);

    if (CHIP_NO_ERROR != error)
    {
        proxy->Release();
        return;
    }

    DiscoveredNodeData nodeData;

    Platform::CopyString(nodeData.hostName, result->mHostName);
    Platform::CopyString(nodeData.instanceName, result->mName);

    if (result->mAddress.HasValue())
    {
        nodeData.ipAddress[0] = result->mAddress.Value();
        nodeData.interfaceId  = result->mInterface;
        nodeData.numIPs       = 1;
    }
    else
    {
        nodeData.numIPs = 0;
    }

    nodeData.port = result->mPort;

    for (size_t i = 0; i < result->mTextEntrySize; ++i)
    {
        ByteSpan key(reinterpret_cast<const uint8_t *>(result->mTextEntries[i].mKey), strlen(result->mTextEntries[i].mKey));
        ByteSpan val(result->mTextEntries[i].mData, result->mTextEntries[i].mDataSize);
        FillNodeDataFromTxt(key, val, nodeData);
    }

    proxy->OnNodeDiscovered(nodeData);
    proxy->Release();
}

static void HandleNodeIdResolve(void * context, DnssdService * result, CHIP_ERROR error)
{
    ResolverDelegateProxy * proxy = static_cast<ResolverDelegateProxy *>(context);
    if (CHIP_NO_ERROR != error)
    {
        proxy->OnOperationalNodeResolutionFailed(PeerId(), error);
        proxy->Release();
        return;
    }

    VerifyOrDie(proxy != nullptr);

    if (result == nullptr)
    {
        proxy->OnOperationalNodeResolutionFailed(PeerId(), CHIP_ERROR_UNKNOWN_RESOURCE_ID);
        proxy->Release();
        return;
    }

    VerifyOrDie(proxy != nullptr);

    PeerId peerId;
    error = ExtractIdFromInstanceName(result->mName, &peerId);
    if (CHIP_NO_ERROR != error)
    {
        proxy->OnOperationalNodeResolutionFailed(PeerId(), error);
        proxy->Release();
        return;
    }

    VerifyOrDie(proxy != nullptr);

    ResolvedNodeData nodeData;
    Platform::CopyString(nodeData.mHostName, result->mHostName);
    nodeData.mInterfaceId = result->mInterface;
    nodeData.mAddress[0]  = result->mAddress.ValueOr({});
    nodeData.mPort        = result->mPort;
    nodeData.mNumIPs      = 1;
    nodeData.mPeerId      = peerId;
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
    nodeData.PrioritizeAddresses();
#if CHIP_CONFIG_MDNS_CACHE_SIZE > 0
    LogErrorOnFailure(sDnssdCache.Insert(nodeData));
#endif
    proxy->OnOperationalNodeResolved(nodeData);
    proxy->Release();
}

static void HandleNodeBrowse(void * context, DnssdService * services, size_t servicesSize, CHIP_ERROR error)
{
    ResolverDelegateProxy * proxy = static_cast<ResolverDelegateProxy *>(context);

    for (size_t i = 0; i < servicesSize; ++i)
    {
        proxy->Retain();
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
    proxy->Release();
}

CHIP_ERROR AddPtrRecord(DiscoveryFilter filter, const char ** entries, size_t & entriesCount, char * buffer, size_t bufferLen)
{
    ReturnErrorOnFailure(MakeServiceSubtype(buffer, bufferLen, filter));
    entries[entriesCount++] = buffer;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AddPtrRecord(DiscoveryFilterType type, const char ** entries, size_t & entriesCount, char * buffer, size_t bufferLen,
                        CommissioningMode value)
{
    VerifyOrReturnError(value != CommissioningMode::kDisabled, CHIP_NO_ERROR);
    return AddPtrRecord(DiscoveryFilter(type), entries, entriesCount, buffer, bufferLen);
}

CHIP_ERROR AddPtrRecord(DiscoveryFilterType type, const char ** entries, size_t & entriesCount, char * buffer, size_t bufferLen,
                        uint64_t value)
{
    return AddPtrRecord(DiscoveryFilter(type, value), entries, entriesCount, buffer, bufferLen);
}

template <class T>
CHIP_ERROR AddPtrRecord(DiscoveryFilterType type, const char ** entries, size_t & entriesCount, char * buffer, size_t bufferLen,
                        chip::Optional<T> value)
{
    VerifyOrReturnError(value.HasValue(), CHIP_NO_ERROR);
    return AddPtrRecord(type, entries, entriesCount, buffer, bufferLen, value.Value());
}

CHIP_ERROR ENFORCE_FORMAT(4, 5)
    CopyTextRecordValue(char * buffer, size_t bufferLen, int minCharactersWritten, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int charactersWritten = vsnprintf(buffer, bufferLen, format, args);
    va_end(args);

    return charactersWritten >= minCharactersWritten ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_STRING_LENGTH;
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, bool value)
{
    return CopyTextRecordValue(buffer, bufferLen, 1, "%d", value);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, uint16_t value)
{
    return CopyTextRecordValue(buffer, bufferLen, 1, "%u", value);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, uint32_t value)
{
    return CopyTextRecordValue(buffer, bufferLen, 1, "%" PRIu32, value);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, uint16_t value1, uint16_t value2)
{
    return CopyTextRecordValue(buffer, bufferLen, 3, "%u+%u", value1, value2);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, const char * value)
{
    return CopyTextRecordValue(buffer, bufferLen, 0, "%s", value);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, CommissioningMode value)
{
    return CopyTextRecordValue(buffer, bufferLen, static_cast<uint16_t>(value));
}

template <class T>
CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, chip::Optional<T> value)
{
    VerifyOrReturnError(value.HasValue(), CHIP_ERROR_WELL_UNINITIALIZED);
    return CopyTextRecordValue(buffer, bufferLen, value.Value());
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, chip::Optional<uint16_t> value1, chip::Optional<uint16_t> value2)
{
    VerifyOrReturnError(value1.HasValue(), CHIP_ERROR_WELL_UNINITIALIZED);
    return value2.HasValue() ? CopyTextRecordValue(buffer, bufferLen, value1.Value(), value2.Value())
                             : CopyTextRecordValue(buffer, bufferLen, value1.Value());
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, const chip::Optional<ReliableMessageProtocolConfig> optional,
                               bool isIdle)
{
    VerifyOrReturnError(optional.HasValue(), CHIP_ERROR_WELL_UNINITIALIZED);

    auto retryInterval = isIdle ? optional.Value().mIdleRetransTimeout : optional.Value().mActiveRetransTimeout;

    if (retryInterval > kMaxRetryInterval)
    {
        ChipLogProgress(Discovery, "MRP retry interval %s value exceeds allowed range of 1 hour, using maximum available",
                        isIdle ? "idle" : "active");
        retryInterval = kMaxRetryInterval;
    }

    return CopyTextRecordValue(buffer, bufferLen, retryInterval.count());
}

template <class T>
CHIP_ERROR CopyTxtRecord(TxtFieldKey key, char * buffer, size_t bufferLen, const T & params)
{
    switch (key)
    {
    case TxtFieldKey::kTcpSupported:
        return CopyTextRecordValue(buffer, bufferLen, params.GetTcpSupported());
    case TxtFieldKey::kMrpRetryIntervalIdle:
    case TxtFieldKey::kMrpRetryIntervalActive:
        return CopyTextRecordValue(buffer, bufferLen, params.GetMRPConfig(), key == TxtFieldKey::kMrpRetryIntervalIdle);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

CHIP_ERROR CopyTxtRecord(TxtFieldKey key, char * buffer, size_t bufferLen, const CommissionAdvertisingParameters & params)
{
    switch (key)
    {
    case TxtFieldKey::kVendorProduct:
        return CopyTextRecordValue(buffer, bufferLen, params.GetVendorId(), params.GetProductId());
    case TxtFieldKey::kDeviceType:
        return CopyTextRecordValue(buffer, bufferLen, params.GetDeviceType());
    case TxtFieldKey::kDeviceName:
        return CopyTextRecordValue(buffer, bufferLen, params.GetDeviceName());
    case TxtFieldKey::kLongDiscriminator:
        return CopyTextRecordValue(buffer, bufferLen, params.GetLongDiscriminator());
    case TxtFieldKey::kRotatingDeviceId:
        return CopyTextRecordValue(buffer, bufferLen, params.GetRotatingDeviceId());
    case TxtFieldKey::kPairingInstruction:
        return CopyTextRecordValue(buffer, bufferLen, params.GetPairingInstruction());
    case TxtFieldKey::kPairingHint:
        return CopyTextRecordValue(buffer, bufferLen, params.GetPairingHint());
    case TxtFieldKey::kCommissioningMode:
        return CopyTextRecordValue(buffer, bufferLen, params.GetCommissioningMode());
    default:
        return CopyTxtRecord(key, buffer, bufferLen, static_cast<BaseAdvertisingParams<CommissionAdvertisingParameters>>(params));
    }
}

template <class T>
CHIP_ERROR AddTxtRecord(TxtFieldKey key, TextEntry * entries, size_t & entriesCount, char * buffer, size_t bufferLen,
                        const T & params)
{
    CHIP_ERROR error = CopyTxtRecord(key, buffer, bufferLen, params);
    VerifyOrReturnError(CHIP_ERROR_WELL_UNINITIALIZED != error, CHIP_NO_ERROR);
    VerifyOrReturnError(CHIP_NO_ERROR == error, error);

    entries[entriesCount++] = { Internal::txtFieldInfo[static_cast<int>(key)].keyStr, reinterpret_cast<const uint8_t *>(buffer),
                                strnlen(buffer, bufferLen) };
    return CHIP_NO_ERROR;
}

} // namespace

DiscoveryImplPlatform DiscoveryImplPlatform::sManager;

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
        if (publisher->mIsOperationalNodePublishing)
        {
            publisher->Advertise(publisher->mOperationalNodeAdvertisingParams);
        }

        if (publisher->mIsCommissionableNodePublishing)
        {
            publisher->Advertise(publisher->mCommissionableNodeAdvertisingParams);
        }

        if (publisher->mIsCommissionerNodePublishing)
        {
            publisher->Advertise(publisher->mCommissionerNodeAdvertisingParams);
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
    if (maxLength < (chip::Dnssd::Commission::kInstanceNameMaxLength + 1))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return chip::Encoding::BytesToUppercaseHexString(&mCommissionableInstanceName[0], sizeof(mCommissionableInstanceName),
                                                     instanceName, maxLength);
}

void DiscoveryImplPlatform::HandleDnssdPublish(void * context, const char * type, CHIP_ERROR error)
{
    if (CHIP_NO_ERROR == error)
    {
        ChipLogProgress(Discovery, "mDNS service published: %s", type);
    }
    else
    {
        ChipLogProgress(Discovery, "mDNS service published error: %s", chip::ErrorStr(error));
    }
}

CHIP_ERROR DiscoveryImplPlatform::PublishService(const char * serviceType, TextEntry * textEntries, size_t textEntrySize,
                                                 const char ** subTypes, size_t subTypeSize,
                                                 const OperationalAdvertisingParameters & params)
{
    return PublishService(serviceType, textEntries, textEntrySize, subTypes, subTypeSize, params.GetPort(), params.GetMac(),
                          DnssdServiceProtocol::kDnssdProtocolTcp, params.GetPeerId());
}

CHIP_ERROR DiscoveryImplPlatform::PublishService(const char * serviceType, TextEntry * textEntries, size_t textEntrySize,
                                                 const char ** subTypes, size_t subTypeSize,
                                                 const CommissionAdvertisingParameters & params)
{
    return PublishService(serviceType, textEntries, textEntrySize, subTypes, subTypeSize, params.GetPort(), params.GetMac(),
                          DnssdServiceProtocol::kDnssdProtocolUdp, PeerId());
}

CHIP_ERROR DiscoveryImplPlatform::PublishService(const char * serviceType, TextEntry * textEntries, size_t textEntrySize,
                                                 const char ** subTypes, size_t subTypeSize, uint16_t port,
                                                 const chip::ByteSpan & mac, DnssdServiceProtocol protocol, PeerId peerId)
{
    ReturnErrorCodeIf(mDnssdInitialized == false, CHIP_ERROR_INCORRECT_STATE);

    DnssdService service;
    ReturnErrorOnFailure(MakeHostName(service.mHostName, sizeof(service.mHostName), mac));
    ReturnErrorOnFailure(protocol == DnssdServiceProtocol::kDnssdProtocolTcp
                             ? MakeInstanceName(service.mName, sizeof(service.mName), peerId)
                             : GetCommissionableInstanceName(service.mName, sizeof(service.mName)));
    strncpy(service.mType, serviceType, sizeof(service.mType));
    service.mAddressType   = Inet::IPAddressType::kAny;
    service.mInterface     = Inet::InterfaceId::Null();
    service.mProtocol      = protocol;
    service.mPort          = port;
    service.mTextEntries   = textEntries;
    service.mTextEntrySize = textEntrySize;
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;

    ReturnErrorOnFailure(ChipDnssdPublishService(&service, HandleDnssdPublish, this));

#ifdef DETAIL_LOGGING
    printf("printEntries port=%u, mTextEntrySize=%zu, mSubTypeSize=%zu\n", port, textEntrySize, subTypeSize);

    for (size_t i = 0; i < textEntrySize; i++)
    {
        printf(" entry [%zu] : %s %s\n", i, textEntries[i].mKey, (char *) (textEntries[i].mData));
    }

    for (size_t i = 0; i < subTypeSize; i++)
    {
        printf(" type [%zu] : %s\n", i, subTypes[i]);
    }
#endif

    return CHIP_NO_ERROR;
}

#define PREPARE_RECORDS(Type)                                                                                                      \
    TextEntry textEntries[Type##AdvertisingParameters::kTxtMaxNumber];                                                             \
    size_t textEntrySize = 0;                                                                                                      \
    const char * subTypes[Type::kSubTypeMaxNumber];                                                                                \
    size_t subTypeSize = 0;

#define ADD_TXT_RECORD(Name)                                                                                                       \
    char Name##Buf[kKey##Name##MaxLength + 1];                                                                                     \
    ReturnErrorOnFailure(AddTxtRecord(TxtFieldKey::k##Name, textEntries, textEntrySize, Name##Buf, sizeof(Name##Buf), params));

#define ADD_PTR_RECORD(Name)                                                                                                       \
    char Name##SubTypeBuf[kSubType##Name##MaxLength + 1];                                                                          \
    ReturnErrorOnFailure(AddPtrRecord(DiscoveryFilterType::k##Name, subTypes, subTypeSize, Name##SubTypeBuf,                       \
                                      sizeof(Name##SubTypeBuf), params.Get##Name()));

#define PUBLISH_RECORDS(Type)                                                                                                      \
    ReturnErrorOnFailure(PublishService(k##Type##ServiceName, textEntries, textEntrySize, subTypes, subTypeSize, params));         \
    m##Type##NodeAdvertisingParams = params;                                                                                       \
    mIs##Type##NodePublishing      = true;                                                                                         \
    return CHIP_NO_ERROR;

CHIP_ERROR DiscoveryImplPlatform::Advertise(const OperationalAdvertisingParameters & params)
{
    PREPARE_RECORDS(Operational);

    ADD_TXT_RECORD(MrpRetryIntervalIdle);
    ADD_TXT_RECORD(MrpRetryIntervalActive);
    ADD_TXT_RECORD(TcpSupported);

    ADD_PTR_RECORD(CompressedFabricId);

    PUBLISH_RECORDS(Operational);
}

CHIP_ERROR DiscoveryImplPlatform::Advertise(const CommissionAdvertisingParameters & params)
{
    PREPARE_RECORDS(Commission);

    ADD_TXT_RECORD(VendorProduct);
    ADD_TXT_RECORD(DeviceType);
    ADD_TXT_RECORD(DeviceName);
    ADD_TXT_RECORD(MrpRetryIntervalIdle);
    ADD_TXT_RECORD(MrpRetryIntervalActive);
    ADD_TXT_RECORD(TcpSupported);

    ADD_PTR_RECORD(VendorId);
    ADD_PTR_RECORD(DeviceType);

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissioner)
    {
        PUBLISH_RECORDS(Commissioner);
    }
    else
    {
        ADD_TXT_RECORD(LongDiscriminator);
        ADD_TXT_RECORD(CommissioningMode);
        ADD_TXT_RECORD(RotatingDeviceId);
        ADD_TXT_RECORD(PairingHint);
        ADD_TXT_RECORD(PairingInstruction);

        ADD_PTR_RECORD(ShortDiscriminator);
        ADD_PTR_RECORD(LongDiscriminator);
        ADD_PTR_RECORD(CommissioningMode);

        PUBLISH_RECORDS(Commissionable);
    }
}

CHIP_ERROR DiscoveryImplPlatform::RemoveServices()
{
    ReturnErrorOnFailure(ChipDnssdRemoveServices());

    mIsOperationalNodePublishing    = false;
    mIsCommissionableNodePublishing = false;
    mIsCommissionerNodePublishing   = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::FinalizeServiceUpdate()
{
    return ChipDnssdFinalizeServiceUpdate();
}

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.ResolveNodeId(peerId, type);
}

bool DiscoveryImplPlatform::ResolveNodeIdFromInternalCache(const PeerId & peerId, Inet::IPAddressType type)
{
    if (InitImpl() != CHIP_NO_ERROR)
    {
        return false;
    }
    return mResolverProxy.ResolveNodeIdFromInternalCache(peerId, type);
}

CHIP_ERROR DiscoveryImplPlatform::FindCommissionableNodes(DiscoveryFilter filter)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryImplPlatform::FindCommissioners(DiscoveryFilter filter)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.FindCommissioners(filter);
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

CHIP_ERROR ResolverProxy::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->Retain();

    DnssdService service;

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), peerId));
    strncpy(service.mType, kOperationalServiceName, sizeof(service.mType));
    service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mAddressType = type;
    return ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeIdResolve, mDelegate);
}

bool ResolverProxy::ResolveNodeIdFromInternalCache(const PeerId & peerId, Inet::IPAddressType type)
{
#if CHIP_CONFIG_MDNS_CACHE_SIZE > 0
    if (mDelegate != nullptr)
    {
        /* see if the entry is cached and use it.... */
        ResolvedNodeData nodeData;
        if (sDnssdCache.Lookup(peerId, nodeData) == CHIP_NO_ERROR)
        {
            mDelegate->OnOperationalNodeResolved(nodeData);
            mDelegate->Release();
            return true;
        }
    }
#endif
    return false;
}

CHIP_ERROR ResolverProxy::FindCommissionableNodes(DiscoveryFilter filter)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->Retain();

    char serviceName[kMaxCommissionableServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionableNode));

    return ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                           Inet::InterfaceId::Null(), HandleNodeBrowse, mDelegate);
}

CHIP_ERROR ResolverProxy::FindCommissioners(DiscoveryFilter filter)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->Retain();

    char serviceName[kMaxCommissionerServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionerNode));

    return ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                           Inet::InterfaceId::Null(), HandleNodeBrowse, mDelegate);
}

} // namespace Dnssd
} // namespace chip
