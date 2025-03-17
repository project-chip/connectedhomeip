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

#include <cstdint>
#include <lib/dnssd/Discovery_ImplPlatform.h>

#include <inttypes.h>

#include <app/icd/server/ICDServerConfig.h>
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
    DiscoveryContext * discoveryContext = static_cast<DiscoveryContext *>(context);

    if (error != CHIP_NO_ERROR && error != CHIP_ERROR_IN_PROGRESS)
    {
        discoveryContext->Release();
        return;
    }

    DiscoveredNodeData nodeData;

    result->ToDiscoveredCommissionNodeData(addresses, nodeData);

    nodeData.Get<CommissionNodeData>().LogDetail();
    discoveryContext->OnNodeDiscovered(nodeData);

    // CHIP_ERROR_IN_PROGRESS indicates that more results are coming, so don't release
    // the context yet.
    if (error == CHIP_NO_ERROR)
    {
        discoveryContext->Release();
    }
}

static void HandleNodeOperationalBrowse(void * context, DnssdService * result, CHIP_ERROR error)
{
    DiscoveryContext * discoveryContext = static_cast<DiscoveryContext *>(context);

    if (error != CHIP_NO_ERROR)
    {
        discoveryContext->Release();
        return;
    }

    DiscoveredNodeData nodeData;

    result->ToDiscoveredOperationalNodeBrowseData(nodeData);

    nodeData.Get<OperationalNodeBrowseData>().LogDetail();
    discoveryContext->OnNodeDiscovered(nodeData);
    discoveryContext->Release();
}

static void HandleNodeBrowse(void * context, DnssdService * services, size_t servicesSize, bool finalBrowse, CHIP_ERROR error)
{
    DiscoveryContext * discoveryContext = static_cast<DiscoveryContext *>(context);

    if (error != CHIP_NO_ERROR)
    {
        discoveryContext->ClearBrowseIdentifier();
        discoveryContext->Release();
        return;
    }

    for (size_t i = 0; i < servicesSize; ++i)
    {
        discoveryContext->Retain();
        // For some platforms browsed services are already resolved, so verify if resolve is really needed or call resolve callback

        auto & ipAddress = services[i].mAddress;

        // mType(service name) exactly matches with operational service name
        bool isOperationalBrowse = strcmp(services[i].mType, kOperationalServiceName) == 0;

        // For operational browse result we currently don't need IP address hence skip resolution and handle differently.
        if (isOperationalBrowse)
        {
            HandleNodeOperationalBrowse(context, &services[i], error);
        }
        // check whether SRV, TXT and AAAA records were received in DNS responses
        else if (strlen(services[i].mHostName) == 0 || services[i].mTextEntrySize == 0 || !ipAddress.has_value())
        {
            ChipDnssdResolve(&services[i], services[i].mInterface, HandleNodeResolve, context);
        }
        else
        {
            Inet::IPAddress * address = &(*ipAddress);
            HandleNodeResolve(context, &services[i], Span<Inet::IPAddress>(address, 1), error);
        }
    }

    if (finalBrowse)
    {
        discoveryContext->ClearBrowseIdentifier();
        discoveryContext->Release();
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
                        const std::optional<T> & value)
{
    VerifyOrReturnError(value.has_value(), CHIP_NO_ERROR);
    return AddPtrRecord(type, entries, entriesCount, buffer, bufferLen, *value);
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
CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, std::optional<T> value)
{
    VerifyOrReturnError(value.has_value(), CHIP_ERROR_UNINITIALIZED);
    return CopyTextRecordValue(buffer, bufferLen, *value);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, std::optional<uint16_t> value1, std::optional<uint16_t> value2)
{
    VerifyOrReturnError(value1.has_value(), CHIP_ERROR_UNINITIALIZED);
    return value2.has_value() ? CopyTextRecordValue(buffer, bufferLen, *value1, *value2)
                              : CopyTextRecordValue(buffer, bufferLen, *value1);
}

CHIP_ERROR CopyTextRecordValue(char * buffer, size_t bufferLen, const std::optional<ReliableMessageProtocolConfig> & optional,
                               TxtFieldKey key)
{
    VerifyOrReturnError((key == TxtFieldKey::kSessionIdleInterval || key == TxtFieldKey::kSessionActiveInterval ||
                         key == TxtFieldKey::kSessionActiveThreshold),
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(optional.has_value(), CHIP_ERROR_UNINITIALIZED);

    CHIP_ERROR err;
    if (key == TxtFieldKey::kSessionActiveThreshold)
    {
        err = CopyTextRecordValue(buffer, bufferLen, optional->mActiveThresholdTime.count());
    }
    else
    {
        bool isIdle        = (key == TxtFieldKey::kSessionIdleInterval);
        auto retryInterval = isIdle ? optional->mIdleRetransTimeout : optional->mActiveRetransTimeout;
        if (retryInterval > kMaxRetryInterval)
        {
            ChipLogProgress(Discovery, "MRP retry interval %s value exceeds allowed range of 1 hour, using maximum available",
                            isIdle ? "idle" : "active");
            retryInterval = kMaxRetryInterval;
        }
        err = CopyTextRecordValue(buffer, bufferLen, retryInterval.count());
    }

    return err;
}

template <class T>
CHIP_ERROR CopyTxtRecord(TxtFieldKey key, char * buffer, size_t bufferLen, const T & params)
{
    switch (key)
    {
    case TxtFieldKey::kTcpSupported:
        VerifyOrReturnError(params.GetTCPSupportModes() != TCPModeAdvertise::kNone, CHIP_ERROR_UNINITIALIZED);
        return CopyTextRecordValue(buffer, bufferLen, to_underlying(params.GetTCPSupportModes()));
    case TxtFieldKey::kSessionIdleInterval:
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // A ICD operating as a LIT should not advertise its slow polling interval
        // Returning UNINITIALIZED ensures that the SII string isn't added by the AddTxtRecord
        // without erroring out the action.
        VerifyOrReturnError(params.GetICDModeToAdvertise() != ICDModeAdvertise::kLIT, CHIP_ERROR_UNINITIALIZED);
        FALLTHROUGH;
#endif
    case TxtFieldKey::kSessionActiveInterval:
    case TxtFieldKey::kSessionActiveThreshold:
        return CopyTextRecordValue(buffer, bufferLen, params.GetLocalMRPConfig(), key);
    case TxtFieldKey::kLongIdleTimeICD:
        // The ICD key is only added to the advertissment when the device supports the ICD LIT feature-set.
        // Return UNINITIALIZED when the operating mode is kNone to ensure that the ICD string isn't added
        // by the AddTxtRecord without erroring out the action.
        VerifyOrReturnError(params.GetICDModeToAdvertise() != ICDModeAdvertise::kNone, CHIP_ERROR_UNINITIALIZED);
        return CopyTextRecordValue(buffer, bufferLen, (params.GetICDModeToAdvertise() == ICDModeAdvertise::kLIT));
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
    case TxtFieldKey::kCommissionerPasscode:
        return CopyTextRecordValue(buffer, bufferLen,
                                   static_cast<uint16_t>(params.GetCommissionerPasscodeSupported().value_or(false) ? 1 : 0));
    default:
        return CopyTxtRecord(key, buffer, bufferLen, static_cast<BaseAdvertisingParams<CommissionAdvertisingParameters>>(params));
    }
}

template <class T>
CHIP_ERROR AddTxtRecord(TxtFieldKey key, TextEntry * entries, size_t & entriesCount, char * buffer, size_t bufferLen,
                        const T & params)
{
    CHIP_ERROR error = CopyTxtRecord(key, buffer, bufferLen, params);
    VerifyOrReturnError(CHIP_ERROR_UNINITIALIZED != error, CHIP_NO_ERROR);
    VerifyOrReturnError(CHIP_NO_ERROR == error, error);

    entries[entriesCount++] = { Internal::txtFieldInfo[static_cast<int>(key)].keyStr, reinterpret_cast<const uint8_t *>(buffer),
                                strnlen(buffer, bufferLen) };
    return CHIP_NO_ERROR;
}

} // namespace

void DiscoveryImplPlatform::HandleNodeIdResolve(void * context, DnssdService * result, const Span<Inet::IPAddress> & addresses,
                                                CHIP_ERROR error)
{
    DiscoveryImplPlatform * impl = static_cast<DiscoveryImplPlatform *>(context);

    if (impl->mOperationalDelegate == nullptr)
    {
        ChipLogError(Discovery, "No delegate to handle node resolution data.");
        return;
    }

    if (CHIP_NO_ERROR != error)
    {
        impl->mOperationalDelegate->OnOperationalNodeResolutionFailed(PeerId(), error);
        return;
    }

    if (result == nullptr)
    {
        impl->mOperationalDelegate->OnOperationalNodeResolutionFailed(PeerId(), CHIP_ERROR_UNKNOWN_RESOURCE_ID);
        return;
    }

    PeerId peerId;
    error = ExtractIdFromInstanceName(result->mName, &peerId);
    if (CHIP_NO_ERROR != error)
    {
        impl->mOperationalDelegate->OnOperationalNodeResolutionFailed(PeerId(), error);
        return;
    }

    ResolvedNodeData nodeData;
    Platform::CopyString(nodeData.resolutionData.hostName, result->mHostName);
    nodeData.resolutionData.interfaceId = result->mInterface;
    nodeData.resolutionData.port        = result->mPort;
    nodeData.operationalData.peerId     = peerId;

    size_t addressesFound = 0;
    for (auto & ip : addresses)
    {
        if (addressesFound == MATTER_ARRAY_SIZE(nodeData.resolutionData.ipAddress))
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
    impl->mOperationalDelegate->OnOperationalNodeResolved(nodeData);
}

void DnssdService::ToDiscoveredOperationalNodeBrowseData(DiscoveredNodeData & nodeData)
{
    nodeData.Set<OperationalNodeBrowseData>();

    ExtractIdFromInstanceName(mName, &nodeData.Get<OperationalNodeBrowseData>().peerId);
    nodeData.Get<OperationalNodeBrowseData>().hasZeroTTL = (mTtlSeconds == 0);
}

void DnssdService::ToDiscoveredCommissionNodeData(const Span<Inet::IPAddress> & addresses, DiscoveredNodeData & nodeData)
{
    nodeData.Set<CommissionNodeData>();
    auto & discoveredData = nodeData.Get<CommissionNodeData>();

    Platform::CopyString(discoveredData.hostName, mHostName);
    Platform::CopyString(discoveredData.instanceName, mName);

    IPAddressSorter::Sort(addresses, mInterface);

    size_t addressesFound = 0;
    for (auto & ip : addresses)
    {
        if (addressesFound == MATTER_ARRAY_SIZE(discoveredData.ipAddress))
        {
            // Out of space.
            ChipLogProgress(Discovery, "Can't add more IPs to DiscoveredNodeData");
            break;
        }
        discoveredData.ipAddress[addressesFound] = ip;
        ++addressesFound;
    }

    discoveredData.interfaceId = mInterface;
    discoveredData.numIPs      = addressesFound;
    discoveredData.port        = mPort;

    for (size_t i = 0; i < mTextEntrySize; ++i)
    {
        ByteSpan key(reinterpret_cast<const uint8_t *>(mTextEntries[i].mKey), strlen(mTextEntries[i].mKey));
        ByteSpan val(mTextEntries[i].mData, mTextEntries[i].mDataSize);
        FillNodeDataFromTxt(key, val, discoveredData);
    }
}

Global<DiscoveryImplPlatform> DiscoveryImplPlatform::sManager;

CHIP_ERROR DiscoveryImplPlatform::InitImpl()
{
    VerifyOrReturnError(mState == State::kUninitialized, CHIP_NO_ERROR);
    mState = State::kInitializing;

    CHIP_ERROR err = ChipDnssdInit(HandleDnssdInit, HandleDnssdError, this);
    if (err != CHIP_NO_ERROR)
    {
        mState = State::kUninitialized;
        return err;
    }
    UpdateCommissionableInstanceName();

    return CHIP_NO_ERROR;
}

void DiscoveryImplPlatform::Shutdown()
{
    VerifyOrReturn(mState != State::kUninitialized);
    ChipDnssdShutdown();
    mState = State::kUninitialized;
}

void DiscoveryImplPlatform::HandleDnssdInit(void * context, CHIP_ERROR initError)
{
    DiscoveryImplPlatform * publisher = static_cast<DiscoveryImplPlatform *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mState = State::kInitialized;

        // Post an event that will start advertising
        DeviceLayer::ChipDeviceEvent event{ .Type = DeviceLayer::DeviceEventType::kDnssdInitialized };

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
    DiscoveryImplPlatform * publisher = static_cast<DiscoveryImplPlatform *>(context);

    if (error == CHIP_ERROR_FORCED_RESET)
    {
        // Restore dnssd state before restart, also needs to call ChipDnssdShutdown()
        publisher->Shutdown();

        DeviceLayer::ChipDeviceEvent event{ .Type = DeviceLayer::DeviceEventType::kDnssdRestartNeeded };
        error = DeviceLayer::PlatformMgr().PostEvent(&event);

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
    DnssdService service;
    ReturnErrorOnFailure(MakeHostName(service.mHostName, sizeof(service.mHostName), mac));
    ReturnErrorOnFailure(protocol == DnssdServiceProtocol::kDnssdProtocolTcp
                             ? MakeInstanceName(service.mName, sizeof(service.mName), peerId)
                             : GetCommissionableInstanceName(service.mName, sizeof(service.mName)));
    Platform::CopyString(service.mType, serviceType);
#if INET_CONFIG_ENABLE_IPV4
    service.mAddressType = Inet::IPAddressType::kAny;
#else
    service.mAddressType = Inet::IPAddressType::kIPv6;
#endif
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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);                                                              \
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

    ADD_TXT_RECORD(SessionIdleInterval);
    ADD_TXT_RECORD(SessionActiveInterval);
    ADD_TXT_RECORD(SessionActiveThreshold);
    ADD_TXT_RECORD(TcpSupported);
    ADD_TXT_RECORD(LongIdleTimeICD); // Optional, will not be added if related 'params' doesn't have a value

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
    ADD_TXT_RECORD(SessionIdleInterval);
    ADD_TXT_RECORD(SessionActiveInterval);
    ADD_TXT_RECORD(SessionActiveThreshold);
    ADD_TXT_RECORD(TcpSupported);
    ADD_TXT_RECORD(LongIdleTimeICD); // Optional, will not be added if related 'params' doesn't have a value

    ADD_PTR_RECORD(VendorId);
    ADD_PTR_RECORD(DeviceType);

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissioner)
    {
        ADD_TXT_RECORD(CommissionerPasscode);
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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(ChipDnssdRemoveServices());

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryImplPlatform::FinalizeServiceUpdate()
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return ChipDnssdFinalizeServiceUpdate();
}

bool DiscoveryImplPlatform::IsInitialized()
{
    return mState == State::kInitialized;
}

CHIP_ERROR DiscoveryImplPlatform::ResolveNodeId(const PeerId & peerId)
{
    // Resolve requests can only be issued once DNSSD is initialized and there is
    // no caching currently
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Discovery, "Resolving " ChipLogFormatX64 ":" ChipLogFormatX64 " ...",
                    ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()));

    DnssdService service;

    ReturnErrorOnFailure(MakeInstanceName(service.mName, sizeof(service.mName), peerId));
    Platform::CopyString(service.mType, kOperationalServiceName);
    service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolTcp;
    service.mAddressType = Inet::IPAddressType::kAny;

    return ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeIdResolve, this);
}

void DiscoveryImplPlatform::NodeIdResolutionNoLongerNeeded(const PeerId & peerId)
{
    char name[Common::kInstanceNameMaxLength + 1];
    ReturnOnFailure(MakeInstanceName(name, sizeof(name), peerId));
    ChipDnssdResolveNoLongerNeeded(name);
}

CHIP_ERROR DiscoveryImplPlatform::DiscoverCommissionableNodes(DiscoveryFilter filter, DiscoveryContext & context)
{
    ReturnErrorOnFailure(InitImpl());
    StopDiscovery(context);

    if (filter.type == DiscoveryFilterType::kInstanceName)
    {
        // When we have the instance name, no need to browse, only need to resolve.
        DnssdService service;

        ReturnErrorOnFailure(MakeServiceSubtype(service.mName, sizeof(service.mName), filter));
        Platform::CopyString(service.mType, kCommissionableServiceName);
        service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolUdp;
        service.mAddressType = Inet::IPAddressType::kAny;

        // Increase the reference count of the context to keep it alive until HandleNodeResolve is called back.
        CHIP_ERROR error = ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeResolve, context.Retain());

        if (error != CHIP_NO_ERROR)
        {
            context.Release();
        }

        return error;
    }

    char serviceName[kMaxCommissionableServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionableNode));

    intptr_t browseIdentifier;
    // Increase the reference count of the context to keep it alive until HandleNodeBrowse is called back.
    CHIP_ERROR error = ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                                       Inet::InterfaceId::Null(), HandleNodeBrowse, context.Retain(), &browseIdentifier);

    if (error == CHIP_NO_ERROR)
    {
        context.SetBrowseIdentifier(browseIdentifier);
    }
    else
    {
        context.Release();
    }

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::DiscoverCommissioners(DiscoveryFilter filter, DiscoveryContext & context)
{
    ReturnErrorOnFailure(InitImpl());
    StopDiscovery(context);

    if (filter.type == DiscoveryFilterType::kInstanceName)
    {
        // When we have the instance name, no need to browse, only need to resolve.
        DnssdService service;

        ReturnErrorOnFailure(MakeServiceSubtype(service.mName, sizeof(service.mName), filter));
        Platform::CopyString(service.mType, kCommissionerServiceName);
        service.mProtocol    = DnssdServiceProtocol::kDnssdProtocolUdp;
        service.mAddressType = Inet::IPAddressType::kAny;

        // Increase the reference count of the context to keep it alive until HandleNodeResolve is called back.
        CHIP_ERROR error = ChipDnssdResolve(&service, Inet::InterfaceId::Null(), HandleNodeResolve, context.Retain());

        if (error != CHIP_NO_ERROR)
        {
            context.Release();
        }
    }

    char serviceName[kMaxCommissionerServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionerNode));

    intptr_t browseIdentifier;
    // Increase the reference count of the context to keep it alive until HandleNodeBrowse is called back.
    CHIP_ERROR error = ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                                       Inet::InterfaceId::Null(), HandleNodeBrowse, context.Retain(), &browseIdentifier);

    if (error == CHIP_NO_ERROR)
    {
        context.SetBrowseIdentifier(browseIdentifier);
    }
    else
    {
        context.Release();
    }

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::DiscoverOperational(DiscoveryFilter filter, DiscoveryContext & context)
{
    ReturnErrorOnFailure(InitImpl());
    StopDiscovery(context);

    char serviceName[kMaxOperationalServiceNameSize];
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kOperational));

    intptr_t browseIdentifier;
    // Increase the reference count of the context to keep it alive until HandleNodeBrowse is called back.
    CHIP_ERROR error = ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolTcp, Inet::IPAddressType::kAny,
                                       Inet::InterfaceId::Null(), HandleNodeBrowse, context.Retain(), &browseIdentifier);

    if (error == CHIP_NO_ERROR)
    {
        context.SetBrowseIdentifier(browseIdentifier);
    }
    else
    {
        context.Release();
    }

    return error;
}

CHIP_ERROR DiscoveryImplPlatform::StartDiscovery(DiscoveryType type, DiscoveryFilter filter, DiscoveryContext & context)
{
    switch (type)
    {
    case DiscoveryType::kCommissionableNode:
        return DiscoverCommissionableNodes(filter, context);
    case DiscoveryType::kCommissionerNode:
        return DiscoverCommissioners(filter, context);
    case DiscoveryType::kOperational:
        return DiscoverOperational(filter, context);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

CHIP_ERROR DiscoveryImplPlatform::StopDiscovery(DiscoveryContext & context)
{
    const std::optional<intptr_t> browseIdentifier = context.GetBrowseIdentifier();
    if (!browseIdentifier.has_value())
    {
        // No discovery going on.
        return CHIP_NO_ERROR;
    }

    context.ClearBrowseIdentifier();
    return ChipDnssdStopBrowse(*browseIdentifier);
}

CHIP_ERROR DiscoveryImplPlatform::ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId)
{
    ReturnErrorOnFailure(InitImpl());

    return ChipDnssdReconfirmRecord(hostname, address, interfaceId);
}

DiscoveryImplPlatform & DiscoveryImplPlatform::GetInstance()
{
    return sManager.get();
}

#if CHIP_DNSSD_DEFAULT_PLATFORM

ServiceAdvertiser & GetDefaultAdvertiser()
{
    return DiscoveryImplPlatform::GetInstance();
}

Resolver & GetDefaultResolver()
{
    return DiscoveryImplPlatform::GetInstance();
}

#endif // CHIP_DNSSD_DEFAULT_PLATFORM

} // namespace Dnssd
} // namespace chip
