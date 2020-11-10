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
#include "Query.h"

namespace mdns {
namespace Minimal {

class QueryData
{
public:
    QueryData() {}
    QueryData(const QueryData &) = default;
    QueryData & operator=(const QueryData &) = default;

    QueryData(QType type, QClass klass, bool unicast, const uint8_t * nameStart, const uint8_t * dataStart,
              const uint8_t * dataEnd) :
        mType(type),
        mClass(klass), mAnswerViaUnicast(unicast), mNameIterator(dataStart, dataEnd, nameStart)
    {}

    QType GetType() const { return mType; }
    QClass GetClass() const { return mClass; }
    bool GetUnicastAnswer() const { return mAnswerViaUnicast; }

    SerializedQNameIterator GetName() const { return mNameIterator; }

    /// Parses a query structure
    ///
    /// Valid data packet is assumed between [dataStart] and [dataEnd]
    /// Parses the query at [start] and updates start to the end of the structure.
    ///
    /// returns true on parse success, false on failure.
    bool Parse(const uint8_t * dataStart, const uint8_t * dataEnd, const uint8_t ** start);

private:
    QType mType            = QType::ANY;
    QClass mClass          = QClass::ANY;
    bool mAnswerViaUnicast = false;
    SerializedQNameIterator mNameIterator; // const since we reuse it
};

class ResourceData
{
public:
    ResourceData() {}

    ResourceData(const ResourceData &) = default;
    ResourceData & operator=(const ResourceData &) = default;

    SerializedQNameIterator GetName() const { return mNameIterator; }

    /// Parses a resource data structure
    ///
    /// Valid data packet is assumed between [dataStart] and [dataEnd]
    /// Parses the daata at [start] and updates start to the end of the structure.
    /// Updates [out] with the parsed data on success.
    ///
    /// returns true on parse success, false on failure.
    bool Parse(const uint8_t * dataStart, const uint8_t * dataEnd, const uint8_t ** start);

private:
    SerializedQNameIterator mNameIterator;
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