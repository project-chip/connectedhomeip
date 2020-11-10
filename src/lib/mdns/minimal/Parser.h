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

#pragma once

#include "DnsHeader.h"
#include "QName.h"

namespace mdns {
namespace Minimal {

class QueryData
{
public:
    QueryData() {}
};

class ResourceData
{
public:
    ResourceData() {}
};

class ParserDelegate
{
public:
    enum class ResourceType
    {
        kAnswer,
        kAuthority,
        kAdditional,
    };
    virtual ~ParserDelegate() {}

    virtual void Header(const HeaderRef & header) = 0;

    virtual void Query(const QueryData & data) = 0;

    virtual void Resource(ResourceType type, const ResourceData & data) = 0;
};

/// Parses a mMDNS packet.
///
/// Calls appropriate delegate callbacks while parsing
///
/// returns true if packet was succesfully parsed, false otherwise
bool ParsePacket(const uint8_t * packet, size_t length, ParserDelegate * delegate);

// FIXME: implement

} // namespace Minimal
} // namespace mdns