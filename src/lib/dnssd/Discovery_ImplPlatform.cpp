/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#include <lib/dnssd/IPAddressSorter.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Dnssd {

namespace {

static void HandleNodeResolve(void * context, DnssdService * result, const Span<Inet::IPAddress> & addresses, CHIP_ERROR error)
{
    ResolverDelegateProxy * proxy = static_cast<ResolverDelegateProxy *>(context);

    if (CHIP_NO_ERROR != error)
    {
        proxy->Release();
        return;
    }

    DiscoveredNodeData nodeData;

    Platform::CopyString(nodeData.resolutionData.hostName, result->mHostName);
    Platform::CopyString(nodeData.commissionData.instanceName, result->mName);

    nodeData.resolutionData.interfaceId = result->mInterface;

    IPAddressSorter::Sort(addresses, result->mInterface);

    size_t addressesFound = 0;
    for (auto & ip : addresses)
    {
        if (addressesFound == ArraySize(nodeData.resolutionData.ipAddress))
        {
            // Out of space.
            ChipLogProgress(Discovery, "Can't add more IPs to DiscoveredNodeData");
            break;
        }
        nodeData.resolutionData.ipAddress[addressesFound] = ip;
        ++addressesFound;
    }

    nodeData.resolutionData.numIPs = addressesFound;

    nodeData.resolutionData.port = result->mPort;

    for (size_t i = 0; i < result->mTextEntrySize; ++i)
    {
        ByteSpan key(reinterpret_cast<const uint8_t *>(result->mTextEntries[i].mKey), strlen(result->mTextEntries[i].mKey));
        ByteSpan val(result->mTextEntries[i].mData, result->mTextEntries[i].mDataSize);
        FillNodeDataFromTxt(key, val, nodeData.resolutionData);
        FillNodeDataFromTxt(key, val, nodeData.commissionData);
    }

    nodeData.LogDetail();
    proxy->OnNodeDiscovered(nodeData);
    proxy->Release();
}

static void HandleNodeIdResolve(void * context, DnssdService * result, const Span<Inet::IPAddress> & addresses, CHIP_ERROR error)
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
    Platform::CopyString(nodeData.resolutionData.hostName, result->mHostName);
    nodeData.resolutionData.interfaceId = result->mInterface;
    nodeData.resolutionData.port        = result->mPort;
    nodeData.operationalData.peerId     = peerId;

    size_t addressesFound = 0;
    for (auto & ip : addresses)
    {
        if (addressesFound == ArraySize(nodeData.resolutionData.ipAddress))
        {
            // Out of space.
            ChipLogProgress(Discovery, "Can't add more IPs to ResolvedNodeData");
            break;
        }
        nodeData.resolutionData.ipAddress[addressesFound] = ip;
        ++addressesFound;
    }
    nodeData.resolutionData.numIPs = addressesFound;

    for (size_t i = 0; i < result->mTextEntrySize; ++i)
    {
        ByteSpan key(reinterpret_cast<const uint8_t *>(result->mTextEntries[i].mKey), strlen(result->mTextEntries[i].mKey));
        ByteSpan val(result->mTextEntries[i].mData, result->mTextEntries[i].mDataSize);
        FillNodeDataFromTxt(key, val, nodeData.resolutionData);
    }

    nodeData.LogNodeIdResolved();
    proxy->OnOperationalNodeResolved(nodeData);
    proxy->Release();
}

static void HandleNodeBrowse(void * context, DnssdService * services, size_t servicesSize, bool finalBrowse, CHIP_ERROR error)
{
    ResolverDelegateProxy * proxy = static_cast<ResolverDelegateProxy *>(context);

    if (error != CHIP_NO_ERROR)
    {
        // We don't have access to the ResolverProxy here to clear out its
        // mDiscoveryContext.  The underlying implementation of
        // ChipDnssdStopBrowse needs to handle a possibly-stale reference
        // safely, so this won't lead to crashes, but it can lead to
        // mis-behavior if a stale mDiscoveryContext happens to match a newer
        // browse operation.
        //
        // TODO: Have a way to clear that state here.
        proxy->Release();
        return;
    }

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
            Inet::IPAddress * address = &(services[i].mAddress.Value());
            HandleNodeResolve(context, &services[i], Span<Inet::IPAddress>(address, 1), error);
        }
    }

    if (finalBrowse)
    {
        // We don't have access to the ResolverProxy here to clear out its
        // mDiscoveryContext.  The underlying implementation of
        // ChipDnssdStopBrowse needs to handle a possibly-stale reference
        // safely, so this won't lead to crashes, but it can lead to
        // mis-behavior if a stale mDiscoveryContext happens to match a newer
        // browse operation.
        //
        // TODO: Have a way to clear that state here.
        proxy->Release();
    }
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
    case TxtFieldKey::kSleepyIdleInterval:
    case TxtFieldKey::kSleepyActiveInterval:
        return CopyTextRecordValue(buffer, bufferLen, params.GetLocalMRPConfig(), key == TxtFieldKey::kSleepyIdleInterval);
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
    VerifyOrReturnError(mState == State::kUninitialized, CHIP_NO_ERROR);
    mState = State::kInitializing;

    ReturnErrorOnFailure(ChipDnssdInit(HandleDnssdInit, HandleDnssdError, this));
    UpdateCommissionableInstanceName();

    return CHIP_NO_ERROR;
}

void DiscoveryImplPlatform::Shutdown()
{
    VerifyOrReturn(mState != State::kUninitialized);
    mResolverProxy.Shutdown();
    ChipDnssdShutdown();
    mState = State::kUninitialized;
}

void DiscoveryImplPlatform::HandleDnssdInit(void * context, CHIP_ERROR initError)
{
    DiscoveryImplPlatform * publisher = static_cast<DiscoveryImplPlatform *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mState = State::kInitialized;

        // TODO: this is wrong, however we need resolverproxy initialized
        // otherwise DiscoveryImplPlatform is not usable.
        //
        // We rely on the fact that resolverproxy does not use the endpoint
        // nor does DiscoveryImplPlatform use it (since init will be called
        // twice now)
        //
        // The problem is that:
        //   - DiscoveryImplPlatform contains a ResolverProxy
        //   - ResolverProxy::Init calls Dnssd::Resolver::Instance().Init
        // which results in a recursive dependency (proxy initializes the
        // class that it is contained in).
        publisher->mResolverProxy.Init(nullptr);

        // Post an event that will start advertising
        DeviceLayer::ChipDeviceEvent event;
        event.Type = DeviceLayer::DeviceEventType::kDnssdPlatformInitialized;

        CHIP_ERROR error = DeviceLayer::PlatformMgr().PostEvent(&event);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Posting DNS-SD platform initialized event failed with %" CHIP_ERROR_FORMAT, error.Format());
        }
    }
    else
    {
        ChipLogError(Discovery, "DNS-SD initialization failed with %" CHIP_ERROR_FORMAT, initError.Format());
        publisher->mState = State::kUninitialized;
    }
}

void DiscoveryImplPlatform::HandleDnssdError(void * context, CHIP_ERROR error)
{
    if (error == CHIP_ERROR_FORCED_RESET)
    {
        DeviceLayer::ChipDeviceEvent event;
        event.Type = DeviceLayer::DeviceEventType::kDnssdRestartNeeded;
        error      = DeviceLayer::PlatformMgr().PostEvent(&event);

        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to post DNS-SD restart event: %" CHIP_ERROR_FORMAT, error.Format());
        }
    }
    else
    {
        ChipLogError(Discovery, "DNS-SD error: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

CHIP_ERROR DiscoveryImplPlatform::GetCommissionableInstanceName(char * instanceName, size_t maxLength) const
{
    if (maxLength < (chip::Dnssd::Commission::kInstanceNameMaxLength + 1))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return chip::Encoding::BytesToUppercaseHexString(&mCommissionableInstanceName[0], sizeof(mCommissionableInstanceName),
                                                     instanceName, maxLength);
}

CHIP_ERROR DiscoveryImplPlatform::UpdateCommissionableInstanceName()
{
    uint64_t random_instance_name = chip::Crypto::GetRandU64();
    static_assert(sizeof(mCommissionableInstanceName) == sizeof(random_instance_name), "Not copying the right amount of data");
    memcpy(&mCommissionableInstanceName[0], &random_instance_name, sizeof(mCommissionableInstanceName));
    return CHIP_NO_ERROR;
}

void DiscoveryImplPlatform::HandleDnssdPublish(void * context, const char * type, const char * instanceName, CHIP_ERROR error)
{
    if (CHIP_NO_ERROR == error)
    {
        ChipLogProgress(Discovery, "mDNS service published: %s; instance name: %s", StringOrNullMarker(type),
                        StringOrNullMarker(instanceName));
    }
    else
    {
        ChipLogError(Discovery, "mDNS service published error: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

CHIP_ERROR DiscoveryImplPlatform::PublishService(const char * serviceType, TextEntry * textEntries, size_t textEntrySize,
                                                 const char ** subTypes, size_t subTypeSize,
                                                 const OperationalAdvertisingParameters & params)
{
    return PublishService(serviceType, textEntries, textEntrySize, subTypes, subTypeSize, params.GetPort(), params.GetInterfaceId(),
                          params.GetMac(), DnssdServiceProtocol::kDnssdProtocolTcp, params.GetPeerId());
}

CHIP_ERROR DiscoveryImplPlatform::PublishService(const char * serviceType, TextEntry * textEntries, size_t textEntrySize,
                                                 const char ** subTypes, size_t subTypeSize,
                                                 const CommissionAdvertisingParameters & params)
{
    return PublishService(serviceType, textEntries, textEntrySize, subTypes, subTypeSize, params.GetPort(), params.GetInterfaceId(),
                          params.GetMac(), DnssdServiceProtocol::kDnssdProtocolUdp, PeerId());
}

CHIP_ERROR DiscoveryImplPlatform::PublishService(const char * serviceType, TextEntry * textEntries, size_t textEntrySize,
                                                 const char ** subTypes, size_t subTypeSize, uint16_t port,
                                                 Inet::InterfaceId interfaceId, const chip::ByteSpan & mac,
                                                 DnssdServiceProtocol protocol, PeerId peerId)
{
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);

    DnssdService service;
    ReturnErrorOnFailure(MakeHostName(service.mHostName, sizeof(service.mHostName), mac));
    ReturnErrorOnFailure(protocol == DnssdServiceProtocol::kDnssdProtocolTcp
                             ? MakeInstanceName(service.mName, sizeof(service.mName), peerId)
                             : GetCommissionableInstanceName(service.mName, sizeof(service.mName)));
    Platform::CopyString(service.mType, serviceType);
    service.mAddressType   = Inet::IPAddressType::kAny;
    service.mInterface     = interfaceId;
    service.mProtocol      = protocol;
    service.mPort          = port;
    service.mTextEntries   = textEntries;
    service.mTextEntrySize = textEntrySize;
    service.mSubTypes      = subTypes;
    service.mSubTypeSize   = subTypeSize;

    ReturnErrorOnFailure(ChipDnssdPublishService(&service, HandleDnssdPublish, this));

#ifdef DETAIL_LOGGING
    printf("printEntries port=%u, mTextEntrySize=%u, mSubTypeSize=%u\n", port, static_cast<unsigned int>(textEntrySize),
           static_cast<unsigned int>(subTypeSize));

    for (size_t i = 0; i < textEntrySize; i++)
    {
        printf(" entry [%u] : %s %s\n", static_cast<unsigned int>(i), StringOrNullMarker(textEntries[i].mKey),
               StringOrNullMarker((char *) (textEntries[i].mData)));
    }

    for (size_t i = 0; i < subTypeSize; i++)
    {
        printf(" type [%u] : %s\n", static_cast<unsigned int>(i), StringOrNullMarker(subTypes[i]));
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
    ReturnErrorOnFailure(PublishService(k##Type##ServiceName, textEntries, textEntrySize, subTypes, subTypeSize, params));

CHIP_ERROR DiscoveryImplPlatform::Advertise(const OperationalAdvertisingParameters & params)
{
    PREPARE_RECORDS(Operational);

    ADD_TXT_RECORD(SleepyIdleInterval);
    ADD_TXT_RECORD(SleepyActiveInterval);
    ADD_TXT_RECORD(TcpSupported);

    ADD_PTR_RECORD(CompressedFabricId);

    PUBLISH_RECORDS(Operational);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::Advertise(const CommissionAdvertisingParameters & params)
{
    PREPARE_RECORDS(Commission);

    ADD_TXT_RECORD(VendorProduct);
    ADD_TXT_RECORD(DeviceType);
    ADD_TXT_RECORD(DeviceName);
    ADD_TXT_RECORD(SleepyIdleInterval);
    ADD_TXT_RECORD(SleepyActiveInterval);
    ADD_TXT_RECORD(TcpSupported);

    ADD_PTR_RECORD(VendorId);
    ADD_PTR_RECORD(DeviceType);

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissioner)
    {
        PUBLISH_RECORDS(Commissioner);
        return CHIP_NO_ERROR;
    }

    ADD_TXT_RECORD(LongDiscriminator);
    ADD_TXT_RECORD(CommissioningMode);
    ADD_TXT_RECORD(RotatingDeviceId);
    ADD_TXT_RECORD(PairingHint);
    ADD_TXT_RECORD(PairingInstruction);

    ADD_PTR_RECORD(ShortDiscriminator);
    ADD_PTR_RECORD(LongDiscriminator);
    ADD_PTR_RECORD(CommissioningMode);

    PUBLISH_RECORDS(Commissionable);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::RemoveServices()
{
    ReturnErrorOnFailure(ChipDnssdRemoveServices());

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::FinalizeServiceUpdate()
{
    return ChipDnssdFinalizeServiceUpdate();
}

bool DiscoveryImplPlatform::IsInitialized()
{
    return mState == State::kInitialized;
}

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(const PeerId & peerId)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.ResolveNodeId(peerId);
}

void DiscoveryImplPlatform::NodeIdResolutionNoLongerNeeded(const PeerId & peerId)
{
    char name[Common::kInstanceNameMaxLength + 1];
    ReturnOnFailure(MakeInstanceName(name, sizeof(name), peerId));

    ChipDnssdResolveNoLongerNeeded(name);
}

CHIP_ERROR DiscoveryImplPlatform::DiscoverCommissionableNodes(DiscoveryFilter filter)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryImplPlatform::DiscoverCommissioners(DiscoveryFilter filter)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.DiscoverCommissioners(filter);
}

CHIP_ERROR DiscoveryImplPlatform::StopDiscovery()
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.StopDiscovery();
}

CHIP_ERROR DiscoveryImplPlatform::ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId)
{
    ReturnErrorOnFailure(InitImpl());
    return mResolverProxy.ReconfirmRecord(hostname, address, interfaceId);
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

CHIP_ERROR ResolverProxy::ResolveNodeId(const PeerId & peerId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Discovery, "Resolving " ChipLogFormatX64 ":" ChipLogFormatX64 " ...",
                    ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()));

    DnssdService service;

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), peerId));
    Platform::CopyString(service.mType, kOperationalServiceName);
    service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mAddressType = Inet::IPAddressType::kAny;

    mDelegate->Retain();

    CHIP_ERROR err = ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeIdResolve, mDelegate);
    if (err != CHIP_NO_ERROR)
    {
        mDelegate->Release();
    }
    return err;
}

void ResolverProxy::NodeIdResolutionNoLongerNeeded(const PeerId & peerId)
{
    char name[Common::kInstanceNameMaxLength + 1];
    ReturnOnFailure(MakeInstanceName(name, sizeof(name), peerId));

    ChipDnssdResolveNoLongerNeeded(name);
}

ResolverProxy::~ResolverProxy()
{
    Shutdown();
}

CHIP_ERROR ResolverProxy::DiscoverCommissionableNodes(DiscoveryFilter filter)
{
    StopDiscovery();

    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->Retain();

    if (filter.type == DiscoveryFilterType::kInstanceName)
    {
        // when we have the instance name, no need to browse, only need to resolve
        DnssdService service;

        ReturnErrorOnFailure(MakeServiceSubtype(service.mName, sizeof(service.mName), filter));
        Platform::CopyString(service.mType, kCommissionableServiceName);
        service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolUdp;
        service.mAddressType = Inet::IPAddressType::kAny;
        return ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeResolve, mDelegate);
    }

    char serviceName[kMaxCommissionableServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionableNode));

    intptr_t browseIdentifier;
    ReturnErrorOnFailure(ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                                         Inet::InterfaceId::Null(), HandleNodeBrowse, mDelegate, &browseIdentifier));
    mDiscoveryContext.Emplace(browseIdentifier);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ResolverProxy::DiscoverCommissioners(DiscoveryFilter filter)
{
    StopDiscovery();

    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->Retain();

    if (filter.type == DiscoveryFilterType::kInstanceName)
    {
        // when we have the instance name, no need to browse, only need to resolve
        DnssdService service;

        ReturnErrorOnFailure(MakeServiceSubtype(service.mName, sizeof(service.mName), filter));
        Platform::CopyString(service.mType, kCommissionerServiceName);
        service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolUdp;
        service.mAddressType = Inet::IPAddressType::kAny;
        return ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeResolve, mDelegate);
    }

    char serviceName[kMaxCommissionerServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionerNode));

    intptr_t browseIdentifier;
    ReturnErrorOnFailure(ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                                         Inet::InterfaceId::Null(), HandleNodeBrowse, mDelegate, &browseIdentifier));
    mDiscoveryContext.Emplace(browseIdentifier);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ResolverProxy::StopDiscovery()
{
    if (!mDiscoveryContext.HasValue())
    {
        // No discovery going on.
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = ChipDnssdStopBrowse(mDiscoveryContext.Value());
    mDiscoveryContext.ClearValue();
    return err;
}

CHIP_ERROR ResolverProxy::ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId)
{
    return ChipDnssdReconfirmRecord(hostname, address, interfaceId);
}

} // namespace Dnssd
} // namespace chip
