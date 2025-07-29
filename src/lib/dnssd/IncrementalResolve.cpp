/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <lib/dnssd/IncrementalResolve.h>

#include <lib/dnssd/IPAddressSorter.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/dnssd/minimal_mdns/Logging.h>
#include <lib/dnssd/minimal_mdns/core/RecordWriter.h>
#include <lib/support/CHIPMemString.h>
#include <tracing/macros.h>

namespace chip {
namespace Dnssd {

using namespace mdns::Minimal;
using namespace mdns::Minimal::Logging;

namespace {

ByteSpan GetSpan(const mdns::Minimal::BytesRange & range)
{
    return ByteSpan(range.Start(), range.Size());
}

/// Handles filling record data from TXT records.
///
/// Supported records are whatever `FillNodeDataFromTxt` supports.
template <class DataType>
class TxtParser : public mdns::Minimal::TxtRecordDelegate
{
public:
    explicit TxtParser(DataType & data) : mData(data) {}
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value) override
    {
        FillNodeDataFromTxt(GetSpan(name), GetSpan(value), mData);
    }

private:
    DataType & mData;
};

// Common prefix to check for all operational/commissioner/commissionable name parts
constexpr QNamePart kOperationalSuffix[]    = { kOperationalServiceName, kOperationalProtocol, kLocalDomain };
constexpr QNamePart kCommissionableSuffix[] = { kCommissionableServiceName, kCommissionProtocol, kLocalDomain };
constexpr QNamePart kCommissionerSuffix[]   = { kCommissionerServiceName, kCommissionProtocol, kLocalDomain };

IncrementalResolver::ServiceNameType ComputeServiceNameType(SerializedQNameIterator name)
{
    // SRV record names look like:
    //   <compressed-fabric-id>-<node-id>._matter._tcp.local  (operational)
    //   <instance>._matterc._udp.local  (commissionable)
    //   <instance>._matterd._udp.local  (commissioner)

    // Starts with compressed-fabric-node or instance, skip over it.
    if (!name.Next() || !name.IsValid())
    {
        // missing required components - empty service name
        return IncrementalResolver::ServiceNameType::kInvalid;
    }

    if (name == kOperationalSuffix)
    {
        return IncrementalResolver::ServiceNameType::kOperational;
    }

    if (name == kCommissionableSuffix)
    {
        return IncrementalResolver::ServiceNameType::kCommissionable;
    }

    if (name == kCommissionerSuffix)
    {
        return IncrementalResolver::ServiceNameType::kCommissioner;
    }

    return IncrementalResolver::ServiceNameType::kInvalid;
}

/// Automatically resets a IncrementalResolver to inactive in its destructor
/// unless disarmed.
///
/// Used for RAII for inactive reset
class AutoInactiveResetter
{
public:
    explicit AutoInactiveResetter(IncrementalResolver & resolver) : mResolver(resolver) {}
    ~AutoInactiveResetter()
    {
        if (mArmed)
        {
            mResolver.ResetToInactive();
        }
    }

    void Disarm() { mArmed = false; }

private:
    bool mArmed = true;
    IncrementalResolver & mResolver;
};

} // namespace

CHIP_ERROR StoredServerName::Set(SerializedQNameIterator value)
{
    chip::Encoding::BigEndian::BufferWriter output(mNameBuffer, sizeof(mNameBuffer));
    RecordWriter writer(&output);

    writer.WriteQName(value);

    if (!writer.Fit())
    {
        Clear();
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

SerializedQNameIterator StoredServerName::Get() const
{
    return SerializedQNameIterator(BytesRange(mNameBuffer, mNameBuffer + sizeof(mNameBuffer)), mNameBuffer);
}

CHIP_ERROR IncrementalResolver::InitializeParsing(mdns::Minimal::SerializedQNameIterator name, const uint64_t ttl,
                                                  const mdns::Minimal::SrvRecord & srv)
{
    AutoInactiveResetter inactiveReset(*this);

    ReturnErrorOnFailure(mRecordName.Set(name));
    ReturnErrorOnFailure(mTargetHostName.Set(srv.GetName()));
    mCommonResolutionData.port = srv.GetPort();

    {
        // TODO: Chip code historically seems to assume that the host name is of the
        // form "<MAC or 802.15.4 Extended Address in hex>.local" and only saves the first part.
        //
        // This should not be needed as server name should not be relevant once parsed.
        // The Resolver keeps track of this name for the purpose of address resolution
        // in mTargetHostName.
        //
        // Keeping track of this in resolution data does not seem useful and should be
        // removed.
        SerializedQNameIterator serverName = srv.GetName();

        VerifyOrReturnError(serverName.Next() && serverName.IsValid(), CHIP_ERROR_INVALID_ARGUMENT);

        // only save the first part: the MAC or 802.15.4 Extended Address in hex
        Platform::CopyString(mCommonResolutionData.hostName, serverName.Value());
    }

    mServiceNameType = ComputeServiceNameType(name);

    switch (mServiceNameType)
    {
    case ServiceNameType::kOperational:
        mSpecificResolutionData.Set<OperationalNodeData>();
        {
            // Operational addresses start with peer node information
            SerializedQNameIterator nameCopy = name;
            if (!nameCopy.Next() || !nameCopy.IsValid())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            CHIP_ERROR err =
                ExtractIdFromInstanceName(nameCopy.Value(), &mSpecificResolutionData.Get<OperationalNodeData>().peerId);
            if (err != CHIP_NO_ERROR)
            {
                return err;
            }
            mSpecificResolutionData.Get<OperationalNodeData>().hasZeroTTL = (ttl == 0);
        }

        LogFoundOperationalSrvRecord(mSpecificResolutionData.Get<OperationalNodeData>().peerId, mTargetHostName.Get());
        break;
    case ServiceNameType::kCommissioner:
    case ServiceNameType::kCommissionable:
        mSpecificResolutionData.Set<CommissionNodeData>();

        {
            // Commission addresses start with instance name
            SerializedQNameIterator nameCopy = name;
            if (!nameCopy.Next() || !nameCopy.IsValid())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            Platform::CopyString(mSpecificResolutionData.Get<CommissionNodeData>().instanceName, nameCopy.Value());
        }

        LogFoundCommissionSrvRecord(mSpecificResolutionData.Get<CommissionNodeData>().instanceName, mTargetHostName.Get());
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    inactiveReset.Disarm();
    return CHIP_NO_ERROR;
}

IncrementalResolver::RequiredInformationFlags IncrementalResolver::GetMissingRequiredInformation() const
{
    RequiredInformationFlags flags;

    if (!mSpecificResolutionData.Valid())
    {
        flags.Set(RequiredInformationBitFlags::kSrvInitialization);
    }
    else
    {
        if (mCommonResolutionData.numIPs == 0)
        {
            flags.Set(RequiredInformationBitFlags::kIpAddress);
        }
    }

    return flags;
}

CHIP_ERROR IncrementalResolver::OnRecord(Inet::InterfaceId interface, const ResourceData & data, BytesRange packetRange)
{
    if (!IsActive())
    {
        return CHIP_NO_ERROR; // nothing to parse
    }

    switch (data.GetType())
    {
    case QType::TXT:
        if (data.GetName() != mRecordName.Get())
        {
            MATTER_TRACE_INSTANT("TXT not applicable", "Resolver");
            return CHIP_NO_ERROR;
        }
        return OnTxtRecord(data, packetRange);
    case QType::A: {
        if (data.GetName() != mTargetHostName.Get())
        {
            MATTER_TRACE_INSTANT("IPv4 not applicable", "Resolver");
            return CHIP_NO_ERROR;
        }

#if INET_CONFIG_ENABLE_IPV4
        Inet::IPAddress addr;
        if (!ParseARecord(data.GetData(), &addr))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        return OnIpAddress(interface, addr);
#else
#if CHIP_MINMDNS_HIGH_VERBOSITY
        ChipLogProgress(Discovery, "Ignoring A record: IPv4 not supported");
#endif
        // skipping IPv4 addresses
        return CHIP_NO_ERROR;
#endif
    }
    case QType::AAAA: {
        if (data.GetName() != mTargetHostName.Get())
        {
            MATTER_TRACE_INSTANT("IPv6 not applicable", "Resolver");
            return CHIP_NO_ERROR;
        }

        Inet::IPAddress addr;
        if (!ParseAAAARecord(data.GetData(), &addr))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        return OnIpAddress(interface, addr);
    }
    case QType::SRV: // SRV handled on creation, ignored for 'additional data'
    default:
        // Other types not interesting during parsing
        return CHIP_NO_ERROR;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR IncrementalResolver::OnTxtRecord(const ResourceData & data, BytesRange packetRange)
{
    {
        TxtParser<CommonResolutionData> delegate(mCommonResolutionData);
        if (!ParseTxtRecord(data.GetData(), &delegate))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (IsActiveCommissionParse())
    {
        TxtParser<CommissionNodeData> delegate(mSpecificResolutionData.Get<CommissionNodeData>());
        if (!ParseTxtRecord(data.GetData(), &delegate))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR IncrementalResolver::OnIpAddress(Inet::InterfaceId interface, const Inet::IPAddress & addr)
{
    if (mCommonResolutionData.numIPs >= MATTER_ARRAY_SIZE(mCommonResolutionData.ipAddress))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mCommonResolutionData.interfaceId.IsPresent())
    {
        mCommonResolutionData.interfaceId = interface;
    }
    else if (mCommonResolutionData.interfaceId != interface)
    {
        // IP addresses received from multiple packets over different interfaces.
        // Processing is assumed per single interface.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mCommonResolutionData.ipAddress[mCommonResolutionData.numIPs++] = addr;

    LogFoundIPAddress(mTargetHostName.Get(), addr);

    return CHIP_NO_ERROR;
}

CHIP_ERROR IncrementalResolver::Take(DiscoveredNodeData & outputData)
{
    VerifyOrReturnError(IsActiveCommissionParse(), CHIP_ERROR_INCORRECT_STATE);

    IPAddressSorter::Sort(mCommonResolutionData.ipAddress, mCommonResolutionData.numIPs, mCommonResolutionData.interfaceId);

    // Set the DiscoveredNodeData with CommissionNodeData info specific to commissionable/commisssioner
    // node available in mSpecificResolutionData.
    outputData.Set<CommissionNodeData>(mSpecificResolutionData.Get<CommissionNodeData>());

    // IncrementalResolver stored CommonResolutionData separately in mCommonResolutionData hence copy the
    //  CommonResolutionData info from mCommonResolutionData, to CommissionNodeData within DiscoveredNodeData
    CommonResolutionData & resolutionData = outputData.Get<CommissionNodeData>();
    resolutionData                        = mCommonResolutionData;

    ResetToInactive();

    return CHIP_NO_ERROR;
}

CHIP_ERROR IncrementalResolver::Take(ResolvedNodeData & outputData)
{
    VerifyOrReturnError(IsActiveOperationalParse(), CHIP_ERROR_INCORRECT_STATE);

    outputData.resolutionData  = mCommonResolutionData;
    outputData.operationalData = mSpecificResolutionData.Get<OperationalNodeData>();

    ResetToInactive();

    return CHIP_NO_ERROR;
}

} // namespace Dnssd
} // namespace chip
