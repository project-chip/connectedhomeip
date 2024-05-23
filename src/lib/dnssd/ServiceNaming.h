/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <lib/dnssd/Constants.h>
#include <lib/dnssd/Types.h>
#include <lib/support/Span.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace Dnssd {
inline constexpr char kSubtypeServiceNamePart[]    = "_sub";
inline constexpr char kCommissionableServiceName[] = "_matterc";
inline constexpr char kOperationalServiceName[]    = "_matter";
inline constexpr char kCommissionerServiceName[]   = "_matterd";
inline constexpr char kOperationalProtocol[]       = "_tcp";
inline constexpr char kCommissionProtocol[]        = "_udp";
inline constexpr char kLocalDomain[]               = "local";

// each includes space for a null terminator, which becomes a . when the names are appended.
constexpr size_t kMaxCommissionableServiceNameSize =
    Common::kSubTypeMaxLength + 1 + sizeof(kSubtypeServiceNamePart) + sizeof(kCommissionableServiceName);

// each includes space for a null terminator, which becomes a . when the names are appended.
constexpr size_t kMaxCommissionerServiceNameSize =
    Common::kSubTypeMaxLength + 1 + sizeof(kSubtypeServiceNamePart) + sizeof(kCommissionerServiceName);

// + 1 for nullchar on prefix.
constexpr size_t kMaxOperationalServiceNameSize =
    Operational::kInstanceNameMaxLength + 1 + sizeof(kOperationalServiceName) + sizeof(kOperationalProtocol) + sizeof(kLocalDomain);

/// builds the MDNS advertising name for a given fabric + nodeid pair
CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, const PeerId & peerId);

/// Inverse of MakeInstanceName.  Will return errors on non-spec-compliant ids,
/// _except_ for allowing lowercase hex, not just the spec-defined uppercase
/// hex.  The part of "name" up to the first '.' (or end of string, whichever
/// comes first) is parsed as a FABRICID-NODEID.
CHIP_ERROR ExtractIdFromInstanceName(const char * name, PeerId * peerId);

/// Generates the host name that a CHIP device is to use for a given unique
/// identifier (MAC address or EUI64)
CHIP_ERROR MakeHostName(char * buffer, size_t bufferLen, const chip::ByteSpan & macOrEui64);

CHIP_ERROR MakeServiceSubtype(char * buffer, size_t bufferLen, DiscoveryFilter subtype);

CHIP_ERROR MakeServiceTypeName(char * buffer, size_t bufferLen, DiscoveryFilter nameDesc, DiscoveryType type);

} // namespace Dnssd
} // namespace chip
