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

#include "Advertiser.h"

#include <inttypes.h>
#include <stdio.h>

#include "MinimalMdnsServer.h"
#include "ServiceNaming.h"

#include <app/icd/server/ICDServerConfig.h>
#include <crypto/RandUtils.h>
#include <lib/dnssd/Advertiser_ImplMinimalMdnsAllocator.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/ResponseSender.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/core/FlatAllocatedQName.h>
#include <lib/dnssd/minimal_mdns/responders/IP.h>
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/QueryResponder.h>
#include <lib/dnssd/minimal_mdns/responders/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/Txt.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/StringBuilder.h>

// Enable detailed mDNS logging for received queries
#undef DETAIL_LOGGING
// #define DETAIL_LOGGING

namespace chip {
namespace Dnssd {
namespace {

using chip::Platform::UniquePtr;
using namespace mdns::Minimal;

#ifdef DETAIL_LOGGING
const char * ToString(QClass qClass)
{
    switch (qClass)
    {
    case QClass::IN:
        return "IN";
    default:
        return "???";
    }
}

const char * ToString(QType qType)
{
    switch (qType)
    {
    case QType::ANY:
        return "ANY";
    case QType::A:
        return "A";
    case QType::AAAA:
        return "AAAA";
    case QType::TXT:
        return "TXT";
    case QType::SRV:
        return "SRV";
    case QType::PTR:
        return "PTR";
    default:
        return "???";
    }
}

void LogQuery(const QueryData & data)
{
    StringBuilder<128> logString;

    logString.Add("QUERY ").Add(ToString(data.GetClass())).Add("/").Add(ToString(data.GetType())).Add(": ");

    SerializedQNameIterator name = data.GetName();
    while (name.Next())
    {
        logString.Add(name.Value()).Add(".");
    }

    ChipLogDetail(Discovery, "%s", logString.c_str());
}
#else
void LogQuery(const QueryData & data) {}
#endif

// Max number of records for operational = PTR, SRV, TXT, A, AAAA, I subtype.
constexpr size_t kMaxOperationalRecords = 6;

/// Represents an allocated operational responder.
///
/// Wraps a QueryResponderAllocator.
class OperationalQueryAllocator : public chip::IntrusiveListNodeBase<>
{
public:
    using Allocator = QueryResponderAllocator<kMaxOperationalRecords>;

    /// Prefer to use `::New` for allocations instead of this direct call
    explicit OperationalQueryAllocator(Allocator * allocator) : mAllocator(allocator) {}
    ~OperationalQueryAllocator()
    {
        chip::Platform::Delete(mAllocator);
        mAllocator = nullptr;
    }

    Allocator * GetAllocator() { return mAllocator; }
    const Allocator * GetAllocator() const { return mAllocator; }

    /// Allocate a new entry for this type.
    ///
    /// May return null on allocation failures.
    static OperationalQueryAllocator * New()
    {
        Allocator * allocator = chip::Platform::New<Allocator>();

        if (allocator == nullptr)
        {
            return nullptr;
        }

        OperationalQueryAllocator * result = chip::Platform::New<OperationalQueryAllocator>(allocator);
        if (result == nullptr)
        {
            chip::Platform::Delete(allocator);
            return nullptr;
        }

        return result;
    }

private:
    Allocator * mAllocator = nullptr;
};

enum BroadcastAdvertiseType
{
    kStarted,     // Advertise at startup of all records added, as required by RFC 6762.
    kRemovingAll, // sent a TTL 0 for all records, as records are removed
};

class AdvertiserMinMdns : public ServiceAdvertiser,
                          public MdnsPacketDelegate, // receive query packets
                          public ParserDelegate      // parses queries
{
public:
    AdvertiserMinMdns() : mResponseSender(&GlobalMinimalMdnsServer::Server())
    {
        GlobalMinimalMdnsServer::Instance().SetQueryDelegate(this);

        CHIP_ERROR err = mResponseSender.AddQueryResponder(mQueryResponderAllocatorCommissionable.GetQueryResponder());

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to set up commissionable responder: %" CHIP_ERROR_FORMAT, err.Format());
        }

        err = mResponseSender.AddQueryResponder(mQueryResponderAllocatorCommissioner.GetQueryResponder());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to set up commissioner responder: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    ~AdvertiserMinMdns() override { ClearServices(); }

    // Service advertiser
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager) override;
    bool IsInitialized() override { return mIsInitialized; }
    void Shutdown() override;
    CHIP_ERROR RemoveServices() override;
    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override;
    CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) override;
    CHIP_ERROR FinalizeServiceUpdate() override;
    CHIP_ERROR GetCommissionableInstanceName(char * instanceName, size_t maxLength) const override;
    CHIP_ERROR UpdateCommissionableInstanceName() override;

    // MdnsPacketDelegate
    void OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info) override;

    // ParserDelegate
    void OnHeader(ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }
    void OnResource(ResourceType type, const ResourceData & data) override {}
    void OnQuery(const QueryData & data) override;

private:
    /// Advertise available records configured within the server.
    ///
    /// Establishes a type of 'Advertise all currently configured items'
    /// for a specific purpose (e.g. boot time advertises everything, shut-down
    /// removes all records by advertising a 0 TTL)
    void AdvertiseRecords(BroadcastAdvertiseType type);

    FullQName GetCommissioningTxtEntries(const CommissionAdvertisingParameters & params);
    FullQName GetOperationalTxtEntries(OperationalQueryAllocator::Allocator * allocator,
                                       const OperationalAdvertisingParameters & params);

    struct CommonTxtEntryStorage
    {
        // +2 for all to account for '=' and terminating nullchar
        char sessionIdleIntervalBuf[KeySize(TxtFieldKey::kSessionIdleInterval) + ValSize(TxtFieldKey::kSessionIdleInterval) + 2];
        char sessionActiveIntervalBuf[KeySize(TxtFieldKey::kSessionActiveInterval) + ValSize(TxtFieldKey::kSessionActiveInterval) +
                                      2];
        char sessionActiveThresholdBuf[KeySize(TxtFieldKey::kSessionActiveThreshold) +
                                       ValSize(TxtFieldKey::kSessionActiveThreshold) + 2];
        char tcpSupportedBuf[KeySize(TxtFieldKey::kTcpSupported) + ValSize(TxtFieldKey::kTcpSupported) + 2];
        char operatingICDAsLITBuf[KeySize(TxtFieldKey::kLongIdleTimeICD) + ValSize(TxtFieldKey::kLongIdleTimeICD) + 2];
    };
    template <class Derived>
    CHIP_ERROR AddCommonTxtEntries(const BaseAdvertisingParams<Derived> & params, CommonTxtEntryStorage & storage,
                                   char ** txtFields, size_t & numTxtFields)
    {

        if (const auto & optionalMrp = params.GetLocalMRPConfig(); optionalMrp.has_value())
        {
            auto mrp = *optionalMrp;

            // An ICD operating as a LIT shall not advertise its slow polling interval.
            // Don't include the SII key in the advertisement when operating as so.
            if (params.GetICDModeToAdvertise() != ICDModeAdvertise::kLIT)
            {
                if (mrp.mIdleRetransTimeout > kMaxRetryInterval)
                {
                    ChipLogProgress(Discovery,
                                    "MRP retry interval idle value exceeds allowed range of 1 hour, using maximum available");
                    mrp.mIdleRetransTimeout = kMaxRetryInterval;
                }
                size_t writtenCharactersNumber =
                    static_cast<size_t>(snprintf(storage.sessionIdleIntervalBuf, sizeof(storage.sessionIdleIntervalBuf),
                                                 "SII=%" PRIu32, mrp.mIdleRetransTimeout.count()));
                VerifyOrReturnError((writtenCharactersNumber > 0) &&
                                        (writtenCharactersNumber < sizeof(storage.sessionIdleIntervalBuf)),
                                    CHIP_ERROR_INVALID_STRING_LENGTH);

                txtFields[numTxtFields++] = storage.sessionIdleIntervalBuf;
            }

            {
                if (mrp.mActiveRetransTimeout > kMaxRetryInterval)
                {
                    ChipLogProgress(Discovery,
                                    "MRP retry interval active value exceeds allowed range of 1 hour, using maximum available");
                    mrp.mActiveRetransTimeout = kMaxRetryInterval;
                }
                size_t writtenCharactersNumber =
                    static_cast<size_t>(snprintf(storage.sessionActiveIntervalBuf, sizeof(storage.sessionActiveIntervalBuf),
                                                 "SAI=%" PRIu32, mrp.mActiveRetransTimeout.count()));
                VerifyOrReturnError((writtenCharactersNumber > 0) &&
                                        (writtenCharactersNumber < sizeof(storage.sessionActiveIntervalBuf)),
                                    CHIP_ERROR_INVALID_STRING_LENGTH);
                txtFields[numTxtFields++] = storage.sessionActiveIntervalBuf;
            }

            {
                size_t writtenCharactersNumber =
                    static_cast<size_t>(snprintf(storage.sessionActiveThresholdBuf, sizeof(storage.sessionActiveThresholdBuf),
                                                 "SAT=%u", mrp.mActiveThresholdTime.count()));
                VerifyOrReturnError((writtenCharactersNumber > 0) &&
                                        (writtenCharactersNumber < sizeof(storage.sessionActiveThresholdBuf)),
                                    CHIP_ERROR_INVALID_STRING_LENGTH);
                txtFields[numTxtFields++] = storage.sessionActiveThresholdBuf;
            }
        }

        if (params.GetTCPSupportModes() != TCPModeAdvertise::kNone)
        {
            size_t writtenCharactersNumber = static_cast<size_t>(snprintf(storage.tcpSupportedBuf, sizeof(storage.tcpSupportedBuf),
                                                                          "T=%d", static_cast<int>(params.GetTCPSupportModes())));
            VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber < sizeof(storage.tcpSupportedBuf)),
                                CHIP_ERROR_INVALID_STRING_LENGTH);
            txtFields[numTxtFields++] = storage.tcpSupportedBuf;
        }

        if (params.GetICDModeToAdvertise() != ICDModeAdvertise::kNone)
        {
            size_t writtenCharactersNumber =
                static_cast<size_t>(snprintf(storage.operatingICDAsLITBuf, sizeof(storage.operatingICDAsLITBuf), "ICD=%d",
                                             (params.GetICDModeToAdvertise() == ICDModeAdvertise::kLIT)));
            VerifyOrReturnError((writtenCharactersNumber > 0) && (writtenCharactersNumber < sizeof(storage.operatingICDAsLITBuf)),
                                CHIP_ERROR_INVALID_STRING_LENGTH);
            txtFields[numTxtFields++] = storage.operatingICDAsLITBuf;
        }
        return CHIP_NO_ERROR;
    }

    IntrusiveList<OperationalQueryAllocator> mOperationalResponders;

    // Max number of records for commissionable = 7 x PTR (base + 6 sub types - _S, _L, _D, _T, _C, _A), SRV, TXT, A, AAAA
    static constexpr size_t kMaxCommissionRecords = 11;
    QueryResponderAllocator<kMaxCommissionRecords> mQueryResponderAllocatorCommissionable;
    QueryResponderAllocator<kMaxCommissionRecords> mQueryResponderAllocatorCommissioner;

    OperationalQueryAllocator::Allocator * FindOperationalAllocator(const FullQName & qname);
    OperationalQueryAllocator::Allocator * FindEmptyOperationalAllocator();

    void ClearServices();

    ResponseSender mResponseSender;
    uint8_t mCommissionableInstanceName[sizeof(uint64_t)];

    bool mIsInitialized = false;

    // current request handling
    const chip::Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint16_t mMessageId                             = 0;

    const char * mEmptyTextEntries[1] = {
        "=",
    };
};

void AdvertiserMinMdns::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
#ifdef DETAIL_LOGGING
    char srcAddressString[chip::Inet::IPAddress::kMaxStringLength];
    VerifyOrDie(info->SrcAddress.ToString(srcAddressString) != nullptr);
    ChipLogDetail(Discovery, "Received an mDNS query from %s", srcAddressString);
#endif

    mCurrentSource = info;
    if (!ParsePacket(data, this))
    {
        ChipLogError(Discovery, "Failed to parse mDNS query");
    }
    mCurrentSource = nullptr;
}

void AdvertiserMinMdns::OnQuery(const QueryData & data)
{
    if (mCurrentSource == nullptr)
    {
        ChipLogError(Discovery, "INTERNAL CONSISTENCY ERROR: missing query source");
        return;
    }

    LogQuery(data);

    const ResponseConfiguration defaultResponseConfiguration;
    CHIP_ERROR err = mResponseSender.Respond(mMessageId, data, mCurrentSource, defaultResponseConfiguration);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to reply to query: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

CHIP_ERROR AdvertiserMinMdns::Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager)
{
    // TODO: Per API documentation, Init() should be a no-op if mIsInitialized
    // is true.  But we don't handle updates to our set of interfaces right now,
    // so rely on the logic in this function to shut down and restart the
    // GlobalMinimalMdnsServer to handle that.
    GlobalMinimalMdnsServer::Server().ShutdownEndpoints();

    if (!mIsInitialized)
    {
        UpdateCommissionableInstanceName();
    }

    // Re-set the server in the response sender in case this has been swapped in the
    // GlobalMinimalMdnsServer (used for testing).
    mResponseSender.SetServer(&GlobalMinimalMdnsServer::Server());

    ReturnErrorOnFailure(GlobalMinimalMdnsServer::Instance().StartServer(udpEndPointManager, kMdnsPort));

    ChipLogProgress(Discovery, "CHIP minimal mDNS started advertising.");

    AdvertiseRecords(BroadcastAdvertiseType::kStarted);

    mIsInitialized = true;

    return CHIP_NO_ERROR;
}

void AdvertiserMinMdns::Shutdown()
{
    VerifyOrReturn(mIsInitialized);

    AdvertiseRecords(BroadcastAdvertiseType::kRemovingAll);

    GlobalMinimalMdnsServer::Server().Shutdown();
    mIsInitialized = false;
}

CHIP_ERROR AdvertiserMinMdns::RemoveServices()
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    // Send a "goodbye" packet for each RR being removed, as defined in RFC 6762.
    // This allows mDNS clients to remove stale cached records which may not be re-added with
    // subsequent Advertise() calls. In the case the same records are re-added, this extra
    // is not harmful though suboptimal, so this is a subject to improvement in the future.
    AdvertiseRecords(BroadcastAdvertiseType::kRemovingAll);
    ClearServices();

    return CHIP_NO_ERROR;
}

void AdvertiserMinMdns::ClearServices()
{
    while (mOperationalResponders.begin() != mOperationalResponders.end())
    {
        auto it = mOperationalResponders.begin();

        // Need to free the memory once it is out of the list
        OperationalQueryAllocator * ptr = &*it;

        // Mark as unused
        ptr->GetAllocator()->Clear();

        CHIP_ERROR err = mResponseSender.RemoveQueryResponder(ptr->GetAllocator()->GetQueryResponder());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to remove query responder: %" CHIP_ERROR_FORMAT, err.Format());
        }

        mOperationalResponders.Remove(ptr);

        // Finally release the memory
        chip::Platform::Delete(ptr);
    }

    mQueryResponderAllocatorCommissionable.Clear();
    mQueryResponderAllocatorCommissioner.Clear();
}

OperationalQueryAllocator::Allocator * AdvertiserMinMdns::FindOperationalAllocator(const FullQName & qname)
{
    for (auto & it : mOperationalResponders)
    {
        if (it.GetAllocator()->GetResponder(QType::SRV, qname) != nullptr)
        {
            return it.GetAllocator();
        }
    }

    return nullptr;
}

OperationalQueryAllocator::Allocator * AdvertiserMinMdns::FindEmptyOperationalAllocator()
{
    OperationalQueryAllocator * result = OperationalQueryAllocator::New();

    if (result == nullptr)
    {
        return nullptr;
    }

    CHIP_ERROR err = mResponseSender.AddQueryResponder(result->GetAllocator()->GetQueryResponder());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to register query responder: %" CHIP_ERROR_FORMAT, err.Format());
        Platform::Delete(result);
        return nullptr;
    }

    mOperationalResponders.PushBack(result);
    return result->GetAllocator();
}

CHIP_ERROR AdvertiserMinMdns::Advertise(const OperationalAdvertisingParameters & params)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    char nameBuffer[Operational::kInstanceNameMaxLength + 1] = "";

    // need to set server name
    ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), params.GetPeerId()));

    QNamePart nameCheckParts[]  = { nameBuffer, kOperationalServiceName, kOperationalProtocol, kLocalDomain };
    FullQName nameCheck         = FullQName(nameCheckParts);
    auto * operationalAllocator = FindOperationalAllocator(nameCheck);
    if (operationalAllocator != nullptr)
    {
        operationalAllocator->Clear();
    }
    else
    {
        operationalAllocator = FindEmptyOperationalAllocator();
        if (operationalAllocator == nullptr)
        {
            ChipLogError(Discovery, "Failed to find an open operational allocator");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    FullQName serviceName = operationalAllocator->AllocateQName(kOperationalServiceName, kOperationalProtocol, kLocalDomain);
    FullQName instanceName =
        operationalAllocator->AllocateQName(nameBuffer, kOperationalServiceName, kOperationalProtocol, kLocalDomain);

    ReturnErrorOnFailure(MakeHostName(nameBuffer, sizeof(nameBuffer), params.GetMac()));
    FullQName hostName = operationalAllocator->AllocateQName(nameBuffer, kLocalDomain);

    if ((serviceName.nameCount == 0) || (instanceName.nameCount == 0) || (hostName.nameCount == 0))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!operationalAllocator->AddResponder<PtrResponder>(serviceName, instanceName)
             .SetReportAdditional(instanceName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    // We are the sole owner of our instanceName, so records keyed on the
    // instanceName should have the cache-flush bit set.
    SrvResourceRecord srvRecord(instanceName, hostName, params.GetPort());
    srvRecord.SetCacheFlush(true);
    if (!operationalAllocator->AddResponder<SrvResponder>(srvRecord).SetReportAdditional(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    TxtResourceRecord txtRecord(instanceName, GetOperationalTxtEntries(operationalAllocator, params));
    txtRecord.SetCacheFlush(true);
    if (!operationalAllocator->AddResponder<TxtResponder>(txtRecord).SetReportAdditional(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!operationalAllocator->AddResponder<IPv6Responder>(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!operationalAllocator->AddResponder<IPv4Responder>(hostName).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                       DiscoveryFilter(DiscoveryFilterType::kCompressedFabricId, params.GetPeerId().GetCompressedFabricId()));
    FullQName compressedFabricIdSubtype = operationalAllocator->AllocateQName(
        nameBuffer, kSubtypeServiceNamePart, kOperationalServiceName, kOperationalProtocol, kLocalDomain);
    ReturnErrorCodeIf(compressedFabricIdSubtype.nameCount == 0, CHIP_ERROR_NO_MEMORY);

    if (!operationalAllocator->AddResponder<PtrResponder>(compressedFabricIdSubtype, instanceName)
             .SetReportAdditional(instanceName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add device type PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Operational device'; instance name: %s.", instanceName.names[0]);

    AdvertiseRecords(BroadcastAdvertiseType::kStarted);

    ChipLogProgress(Discovery, "mDNS service published: %s.%s", StringOrNullMarker(instanceName.names[1]),
                    StringOrNullMarker(instanceName.names[2]));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdvertiserMinMdns::FinalizeServiceUpdate()
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdvertiserMinMdns::GetCommissionableInstanceName(char * instanceName, size_t maxLength) const
{
    if (maxLength < (Commission::kInstanceNameMaxLength + 1))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return chip::Encoding::BytesToUppercaseHexString(&mCommissionableInstanceName[0], sizeof(mCommissionableInstanceName),
                                                     instanceName, maxLength);
}

CHIP_ERROR AdvertiserMinMdns::UpdateCommissionableInstanceName()
{
    uint64_t random_instance_name = chip::Crypto::GetRandU64();
    static_assert(sizeof(mCommissionableInstanceName) == sizeof(random_instance_name), "Not copying the right amount of data");
    memcpy(&mCommissionableInstanceName[0], &random_instance_name, sizeof(mCommissionableInstanceName));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdvertiserMinMdns::Advertise(const CommissionAdvertisingParameters & params)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        mQueryResponderAllocatorCommissionable.Clear();
    }
    else
    {
        mQueryResponderAllocatorCommissioner.Clear();
    }

    // TODO: need to detect colisions here
    char nameBuffer[64] = "";
    ReturnErrorOnFailure(GetCommissionableInstanceName(nameBuffer, sizeof(nameBuffer)));

    QueryResponderAllocator<kMaxCommissionRecords> * allocator =
        params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode ? &mQueryResponderAllocatorCommissionable
                                                                                           : &mQueryResponderAllocatorCommissioner;
    const char * serviceType = params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode
        ? kCommissionableServiceName
        : kCommissionerServiceName;

    FullQName serviceName  = allocator->AllocateQName(serviceType, kCommissionProtocol, kLocalDomain);
    FullQName instanceName = allocator->AllocateQName(nameBuffer, serviceType, kCommissionProtocol, kLocalDomain);

    ReturnErrorOnFailure(MakeHostName(nameBuffer, sizeof(nameBuffer), params.GetMac()));
    FullQName hostName = allocator->AllocateQName(nameBuffer, kLocalDomain);

    if ((serviceName.nameCount == 0) || (instanceName.nameCount == 0) || (hostName.nameCount == 0))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!allocator->AddResponder<PtrResponder>(serviceName, instanceName)
             .SetReportAdditional(instanceName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    SrvResourceRecord srvRecord(instanceName, hostName, params.GetPort());
    srvRecord.SetCacheFlush(true);
    if (!allocator->AddResponder<SrvResponder>(srvRecord).SetReportAdditional(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!allocator->AddResponder<IPv6Responder>(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!allocator->AddResponder<IPv4Responder>(hostName).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (const auto & vendorId = params.GetVendorId(); vendorId.has_value())
    {
        MakeServiceSubtype(nameBuffer, sizeof(nameBuffer), DiscoveryFilter(DiscoveryFilterType::kVendorId, *vendorId));
        FullQName vendorServiceName =
            allocator->AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType, kCommissionProtocol, kLocalDomain);
        ReturnErrorCodeIf(vendorServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

        if (!allocator->AddResponder<PtrResponder>(vendorServiceName, instanceName)
                 .SetReportAdditional(instanceName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add vendor PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (const auto & deviceType = params.GetDeviceType(); deviceType.has_value())
    {
        MakeServiceSubtype(nameBuffer, sizeof(nameBuffer), DiscoveryFilter(DiscoveryFilterType::kDeviceType, *deviceType));
        FullQName vendorServiceName =
            allocator->AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType, kCommissionProtocol, kLocalDomain);
        ReturnErrorCodeIf(vendorServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

        if (!allocator->AddResponder<PtrResponder>(vendorServiceName, instanceName)
                 .SetReportAdditional(instanceName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add device type PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    // the following sub types only apply to commissionable node advertisements
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                               DiscoveryFilter(DiscoveryFilterType::kShortDiscriminator, params.GetShortDiscriminator()));
            FullQName shortServiceName =
                allocator->AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType, kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(shortServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

            if (!allocator->AddResponder<PtrResponder>(shortServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add short discriminator PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                               DiscoveryFilter(DiscoveryFilterType::kLongDiscriminator, params.GetLongDiscriminator()));
            FullQName longServiceName =
                allocator->AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType, kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(longServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);
            if (!allocator->AddResponder<PtrResponder>(longServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add long discriminator PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        if (params.GetCommissioningMode() != CommissioningMode::kDisabled)
        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer), DiscoveryFilter(DiscoveryFilterType::kCommissioningMode));
            FullQName longServiceName =
                allocator->AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType, kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(longServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);
            if (!allocator->AddResponder<PtrResponder>(longServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add commissioning mode PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }
    }

    TxtResourceRecord txtRecord(instanceName, GetCommissioningTxtEntries(params));
    txtRecord.SetCacheFlush(true);
    if (!allocator->AddResponder<TxtResponder>(txtRecord).SetReportAdditional(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Commissionable node device'; instance name: %s.",
                        StringOrNullMarker(instanceName.names[0]));
    }
    else
    {
        ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Commissioner device'; instance name: %s.",
                        StringOrNullMarker(instanceName.names[0]));
    }

    AdvertiseRecords(BroadcastAdvertiseType::kStarted);

    ChipLogProgress(Discovery, "mDNS service published: %s.%s", StringOrNullMarker(instanceName.names[1]),
                    StringOrNullMarker(instanceName.names[2]));

    return CHIP_NO_ERROR;
}

FullQName AdvertiserMinMdns::GetOperationalTxtEntries(OperationalQueryAllocator::Allocator * allocator,
                                                      const OperationalAdvertisingParameters & params)
{
    char * txtFields[OperationalAdvertisingParameters::kTxtMaxNumber];
    size_t numTxtFields = 0;

    struct CommonTxtEntryStorage commonStorage;
    AddCommonTxtEntries<OperationalAdvertisingParameters>(params, commonStorage, txtFields, numTxtFields);
    if (numTxtFields == 0)
    {
        return allocator->AllocateQNameFromArray(mEmptyTextEntries, 1);
    }

    return allocator->AllocateQNameFromArray(txtFields, numTxtFields);
}

FullQName AdvertiserMinMdns::GetCommissioningTxtEntries(const CommissionAdvertisingParameters & params)
{
    char * txtFields[CommissionAdvertisingParameters::kTxtMaxNumber];
    size_t numTxtFields = 0;

    QueryResponderAllocator<kMaxCommissionRecords> * allocator =
        params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode ? &mQueryResponderAllocatorCommissionable
                                                                                           : &mQueryResponderAllocatorCommissioner;

    char txtVidPid[chip::Dnssd::kKeyVendorProductMaxLength + 4];
    {
        const auto & productId = params.GetProductId();
        const auto & vendorId  = params.GetVendorId();
        if (productId.has_value() && vendorId.has_value())
        {
            snprintf(txtVidPid, sizeof(txtVidPid), "VP=%d+%d", *vendorId, *productId);
            txtFields[numTxtFields++] = txtVidPid;
        }
        else if (vendorId.has_value())
        {
            snprintf(txtVidPid, sizeof(txtVidPid), "VP=%d", *vendorId);
            txtFields[numTxtFields++] = txtVidPid;
        }
    }

    char txtDeviceType[chip::Dnssd::kKeyDeviceTypeMaxLength + 4];
    if (const auto & deviceType = params.GetDeviceType(); deviceType.has_value())
    {
        snprintf(txtDeviceType, sizeof(txtDeviceType), "DT=%" PRIu32, *deviceType);
        txtFields[numTxtFields++] = txtDeviceType;
    }

    char txtDeviceName[chip::Dnssd::kKeyDeviceNameMaxLength + 4];
    if (const auto & deviceName = params.GetDeviceName(); deviceName.has_value())
    {
        snprintf(txtDeviceName, sizeof(txtDeviceName), "DN=%s", *deviceName);
        txtFields[numTxtFields++] = txtDeviceName;
    }
    CommonTxtEntryStorage commonStorage;
    AddCommonTxtEntries<CommissionAdvertisingParameters>(params, commonStorage, txtFields, numTxtFields);

    // the following sub types only apply to commissionable node advertisements
    char txtDiscriminator[chip::Dnssd::kKeyLongDiscriminatorMaxLength + 3];
    char txtCommissioningMode[chip::Dnssd::kKeyCommissioningModeMaxLength + 4];
    char txtRotatingDeviceId[chip::Dnssd::kKeyRotatingDeviceIdMaxLength + 4];
    char txtPairingHint[chip::Dnssd::kKeyPairingInstructionMaxLength + 4];
    char txtPairingInstr[chip::Dnssd::kKeyPairingInstructionMaxLength + 4];

    // the following sub types only apply to commissioner discovery advertisements
    char txtCommissionerPasscode[chip::Dnssd::kKeyCommissionerPasscodeMaxLength + 4];

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        // a discriminator always exists
        snprintf(txtDiscriminator, sizeof(txtDiscriminator), "D=%d", params.GetLongDiscriminator());
        txtFields[numTxtFields++] = txtDiscriminator;

        snprintf(txtCommissioningMode, sizeof(txtCommissioningMode), "CM=%d", static_cast<int>(params.GetCommissioningMode()));
        txtFields[numTxtFields++] = txtCommissioningMode;

        if (const auto & rotatingDeviceId = params.GetRotatingDeviceId(); rotatingDeviceId.has_value())
        {
            snprintf(txtRotatingDeviceId, sizeof(txtRotatingDeviceId), "RI=%s", *rotatingDeviceId);
            txtFields[numTxtFields++] = txtRotatingDeviceId;
        }

        if (const auto & pairingHint = params.GetPairingHint(); pairingHint.has_value())
        {
            snprintf(txtPairingHint, sizeof(txtPairingHint), "PH=%d", *pairingHint);
            txtFields[numTxtFields++] = txtPairingHint;
        }

        if (const auto & pairingInstruction = params.GetPairingInstruction(); pairingInstruction.has_value())
        {
            snprintf(txtPairingInstr, sizeof(txtPairingInstr), "PI=%s", *pairingInstruction);
            txtFields[numTxtFields++] = txtPairingInstr;
        }
    }
    else
    {
        if (params.GetCommissionerPasscodeSupported().value_or(false))
        {
            snprintf(txtCommissionerPasscode, sizeof(txtCommissionerPasscode), "CP=%d", static_cast<int>(1));
            txtFields[numTxtFields++] = txtCommissionerPasscode;
        }
    }
    if (numTxtFields == 0)
    {
        return allocator->AllocateQNameFromArray(mEmptyTextEntries, 1);
    }

    return allocator->AllocateQNameFromArray(txtFields, numTxtFields);
}

void AdvertiserMinMdns::AdvertiseRecords(BroadcastAdvertiseType type)
{
    ResponseConfiguration responseConfiguration;
    if (type == BroadcastAdvertiseType::kRemovingAll)
    {
        // make a "remove all records now" broadcast
        responseConfiguration.SetTtlSecondsOverride(0);
    }

    UniquePtr<ListenIterator> allInterfaces = GetAddressPolicy()->GetListenEndpoints();
    VerifyOrDieWithMsg(allInterfaces != nullptr, Discovery, "Failed to allocate memory for endpoints.");

    chip::Inet::InterfaceId interfaceId;
    chip::Inet::IPAddressType addressType;

    while (allInterfaces->Next(&interfaceId, &addressType))
    {
        UniquePtr<IpAddressIterator> allIps = GetAddressPolicy()->GetIpAddressesForEndpoint(interfaceId, addressType);
        VerifyOrDieWithMsg(allIps != nullptr, Discovery, "Failed to allocate memory for ip addresses.");

        chip::Inet::IPPacketInfo packetInfo;

        packetInfo.Clear();

        // advertising on every interface requires a valid IP address
        // since we use "BROADCAST" (unicast is false), we do not actually care about
        // the source IP address value, just that it has the right "type"
        //
        // NOTE: cannot use Broadcast address as the source as they have the type kAny.
        //
        // TODO: ideally we may want to have a destination that is explicit as "unicast/destIp"
        //       vs "multicast/addressType". Such a change requires larger code updates.
        packetInfo.SrcAddress  = chip::Inet::IPAddress::Loopback(addressType);
        packetInfo.DestAddress = BroadcastIpAddresses::Get(addressType);
        packetInfo.SrcPort     = kMdnsPort;
        packetInfo.DestPort    = kMdnsPort;
        packetInfo.Interface   = interfaceId;

        // Advertise all records
        //
        // TODO: Consider advertising delta changes.
        //
        // Current advertisement does not have a concept of "delta" to only
        // advertise changes. Current implementation is to always
        //    1. advertise TTL=0 (clear all caches)
        //    2. advertise available records (with longer TTL)
        //
        // It would be nice if we could selectively advertise what changes, like
        // send TTL=0 for anything removed/about to be removed (and only those),
        // then only advertise new items added.
        //
        // This optimization likely will take more logic and state storage, so
        // for now it is not done.
        QueryData queryData(QType::PTR, QClass::IN, false /* unicast */);
        queryData.SetIsAnnounceBroadcast(true);

        for (auto & it : mOperationalResponders)
        {
            it.GetAllocator()->GetQueryResponder()->ClearBroadcastThrottle();
        }
        mQueryResponderAllocatorCommissionable.GetQueryResponder()->ClearBroadcastThrottle();
        mQueryResponderAllocatorCommissioner.GetQueryResponder()->ClearBroadcastThrottle();

        CHIP_ERROR err = mResponseSender.Respond(0, queryData, &packetInfo, responseConfiguration);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise records: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    // Once all automatic broadcasts are done, allow immediate replies once.
    for (auto & it : mOperationalResponders)
    {
        it.GetAllocator()->GetQueryResponder()->ClearBroadcastThrottle();
    }
    mQueryResponderAllocatorCommissionable.GetQueryResponder()->ClearBroadcastThrottle();
    mQueryResponderAllocatorCommissioner.GetQueryResponder()->ClearBroadcastThrottle();
}

AdvertiserMinMdns gAdvertiser;
} // namespace

#if CHIP_DNSSD_DEFAULT_MINIMAL

ServiceAdvertiser & GetDefaultAdvertiser()
{
    return gAdvertiser;
}

#endif // CHIP_DNSSD_DEFAULT_MINIMAL

} // namespace Dnssd
} // namespace chip
