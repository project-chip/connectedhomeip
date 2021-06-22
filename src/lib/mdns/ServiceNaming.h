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

#include <core/CHIPError.h>
#include <core/PeerId.h>
#include <mdns/Resolver.h>
#include <support/Span.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace Mdns {
constexpr size_t kMaxSubtypeDescSize        = 8; // max 5 decimal digits + _X prefix. + null character
constexpr char kSubtypeServiceNamePart[]    = "_sub";
constexpr char kCommissionableServiceName[] = "_chipc";
constexpr char kOperationalServiceName[]    = "_chip";
constexpr char kCommissionerServiceName[]   = "_chipd";
constexpr char kOperationalProtocol[]       = "_tcp";
constexpr char kCommissionProtocol[]        = "_udp";
constexpr char kLocalDomain[]               = "local";

// each includes space for a null terminator, which becomes a . when the names are appended.
constexpr size_t kMaxCommisisonableServiceNameSize =
    kMaxSubtypeDescSize + sizeof(kSubtypeServiceNamePart) + sizeof(kCommissionableServiceName);

/// builds the MDNS advertising name for a given fabric + nodeid pair
CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, const PeerId & peerId);

/// Inverse of MakeInstanceName
CHIP_ERROR ExtractIdFromInstanceName(const char * name, PeerId * peerId);

/// Generates the host name that a CHIP device is to use for a given unique
/// identifier (MAC address or EUI64)
CHIP_ERROR MakeHostName(char * buffer, size_t bufferLen, const chip::ByteSpan & macOrEui64);

CHIP_ERROR MakeServiceSubtype(char * buffer, size_t bufferLen, DiscoveryFilter subtype);

CHIP_ERROR MakeCommissionableNodeServiceTypeName(char * buffer, size_t bufferLen, DiscoveryFilter nameDesc);

} // namespace Mdns
} // namespace chip
