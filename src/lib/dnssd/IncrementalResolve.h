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
#pragma once

#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Variant.h>

namespace chip {
namespace Dnssd {

/// Allows storing and retrieving of a SerializedQName for later recall.
///
/// This is a convenience storage as MDNS generally provides data as QNames
/// and comparisons between QNames is more convenient than replacing them with
/// null-terminated character strings.
class StoredServerName
{
public:
    StoredServerName() {}

    void Clear() { memset(mNameBuffer, 0, sizeof(mNameBuffer)); }

    /// Set the underlying value. Will return CHIP_ERROR_NO_MEMORY
    /// on insufficient storage space.
    ///
    /// If insufficient space, buffer will be cleared.
    CHIP_ERROR Set(mdns::Minimal::SerializedQNameIterator value);

    /// Return the underlying value in this object.
    ///
    /// Value valid as long as this object is valid and underlying Set() is
    /// not called.
    mdns::Minimal::SerializedQNameIterator Get() const;

private:
    // Try to have space for at least:
    //  L1234._sub._matterc._udp.local      => 30 chars
    //  <fabric>-<node>._matter._tcp.local  => 52 chars
    //  <hostname>.local (where hostname is kHostNameMaxLength == 16)
    //
    // This does not try to optimize out ".local" suffix which is always expected
    // since comparisons are easier when suffix is still present.
    static constexpr size_t kMaxStoredNameLength = 64;

    uint8_t mNameBuffer[kMaxStoredNameLength] = {};
};

/// Incrementally accumulates data from DNSSD packets. It is geared twoards
/// resource-constrained dns-sd querier implementations.
///
/// It all starts with processing SRV records which define the type of record
/// (could be operational, commissionable or commissioner), after which the
/// additional data is accumulated, specifically TXT information and A/AAAA
///
/// Class can also be used to determine what additional data is missing from a
/// record so that additional DNSSD queries can be made recursively (e.g. if
/// only a SRV/TXT records are available, ask for AAAA records).
class IncrementalResolver
{
public:
    // Elements that the incremental resolve still needs
    enum class RequiredInformationBitFlags : uint8_t
    {
        kSrvInitialization = (1 << 0), // server being initialized
        kIpAddress         = (1 << 1), // IP address missing
    };
    using RequiredInformationFlags = BitFlags<RequiredInformationBitFlags>;

    IncrementalResolver() {}

    /// Checks if object has been initialized using the `InitializeParsing`
    /// method.
    bool IsActive() const { return mSpecificResolutionData.Valid(); }

    bool IsActiveCommissionParse() const { return mSpecificResolutionData.Is<CommissionNodeData>(); }
    bool IsActiveOperationalParse() const { return mSpecificResolutionData.Is<OperationalNodeData>(); }

    /// Start parsing a new record. SRV records are the records we are mainly
    /// interested on, after which TXT and A/AAAA are looked for.
    ///
    /// If this function returns with error, the object will be in an inactive state.
    CHIP_ERROR InitializeParsing(mdns::Minimal::SerializedQNameIterator name, const mdns::Minimal::SrvRecord & srv);

    /// Notify that a new record is being processed.
    /// Will handle filtering and processing of data to determine if the entry is relevant for
    /// the current resolver.
    ///
    /// Providing a data that is not relevant to the current parser is not considered and error,
    /// however if the resource fails parsing completely an error will be returned.
    ///
    ///
    /// [data] represents the record received via [interface] and [packetRange] represents the range
    /// of valid bytes within the packet for the purpose of QName parsing
    CHIP_ERROR OnRecord(Inet::InterfaceId interface, const mdns::Minimal::ResourceData & data,
                        mdns::Minimal::BytesRange packetRange);

    /// Return what additional data is required until the object can be extracted
    ///
    /// If `!GetREquiredInformation().HasAny()` the parsed information is ready
    /// to be processed.
    RequiredInformationFlags GetMissingRequiredInformation() const;

    /// Fetch the target host name set by `InitializeParsing`
    ///
    /// VALIDITY: Data references internal storage of this object and is valid as long
    ///           as this object is valid and InitializeParsing is not called again.
    mdns::Minimal::SerializedQNameIterator GetTargetHostName() const { return mTargetHostName.Get(); }

    /// Fetch the record name set by `InitializeParsing`.
    ///
    /// VALIDITY: Data references internal storage of this object and is valid as long
    ///           as this object is valid and InitializeParsing is not called again.
    mdns::Minimal::SerializedQNameIterator GetRecordName() const { return mRecordName.Get(); }

    /// Take the current value of the object and clear it once returned.
    ///
    /// Object must be in `IsActiveCommissionParse()` for this to succeed.
    /// Data will be returned (and cleared) even if not yet complete based
    /// on `GetMissingRequiredInformation()`. This method takes as much data as
    /// it was parsed so far.
    CHIP_ERROR Take(DiscoveredNodeData & outputData);

    /// Take the current value of the object and clear it once returned.
    ///
    /// Object must be in `IsActiveOperationalParse()` for this to succeed.
    /// Data will be returned (and cleared) even if not yet complete based
    /// on `GetMissingRequiredInformation()`. This method takes as much data as
    /// it was parsed so far.
    CHIP_ERROR Take(ResolvedNodeData & outputData);

    /// Clears current state, setting as inactive
    void ResetToInactive()
    {
        mCommonResolutionData.Reset();
        mSpecificResolutionData = ParsedRecordSpecificData();
    }

private:
    /// Notify that a PTR record can be parsed.
    ///
    /// Input data MUST have GetType() == QType::TXT
    CHIP_ERROR OnTxtRecord(const mdns::Minimal::ResourceData & data, mdns::Minimal::BytesRange packetRange);

    /// Notify that a new IP addres has been found.
    ///
    /// This is to be called on both A (if IPv4 support is enabled) and AAAA
    /// addresses.
    ///
    /// Prerequisite: IP address belongs to the right nost name
    CHIP_ERROR OnIpAddress(Inet::InterfaceId interface, const Inet::IPAddress & addr);

    using ParsedRecordSpecificData = Variant<OperationalNodeData, CommissionNodeData>;

    StoredServerName mRecordName;     // Record name for what is parsed (SRV/PTR/TXT)
    StoredServerName mTargetHostName; // `Target` for the SRV record
    CommonResolutionData mCommonResolutionData;
    ParsedRecordSpecificData mSpecificResolutionData;
};

} // namespace Dnssd
} // namespace chip
