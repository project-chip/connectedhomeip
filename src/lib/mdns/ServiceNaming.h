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
#include <support/Span.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace Mdns {

/// builds the MDNS advertising name for a given fabric + nodeid pair
CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, const PeerId & peerId);

/// Inverse of MakeInstanceName
CHIP_ERROR ExtractIdFromInstanceName(const char * name, PeerId * peerId);

/// Generates the host name that a CHIP device is to use for a given unique
/// identifier (MAC address or EUI64)
CHIP_ERROR MakeHostName(char * buffer, size_t bufferLen, const chip::ByteSpan & macOrEui64);

} // namespace Mdns
} // namespace chip
