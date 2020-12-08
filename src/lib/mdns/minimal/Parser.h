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

#include <mdns/minimal/core/Constants.h>
#include <mdns/minimal/core/DnsHeader.h>
#include <mdns/minimal/core/QName.h>

namespace mdns {
namespace Minimal {

class QueryData
{
public:
    QueryData() {}
    QueryData(const QueryData &) = default;
    QueryData & operator=(const QueryData &) = default;

    QueryData(QType type, QClass klass, bool unicast, const uint8_t * nameStart, const BytesRange & validData) :
        mType(type), mClass(klass), mAnswerViaUnicast(unicast), mNameIterator(validData, nameStart)
    {}

    QType GetType() const { return mType; }
    QClass GetClass() const { return mClass; }
    bool RequestedUnicastAnswer() const { return mAnswerViaUnicast; }

    SerializedQNameIterator GetName() const { return mNameIterator; }

    /// Parses a query structure
    ///
    /// Parses the query at [start] and updates start to the end of the structure.
    ///
    /// returns true on parse success, false on failure.
    bool Parse(const BytesRange & validData, const uint8_t ** start);

    /// Write out this query data back into an output buffer.
    bool Append(HeaderRef & hdr, chip::Encoding::BigEndian::BufferWriter & out) const;

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

    QType GetType() const { return mType; }
    QClass GetClass() const { return mClass; }
    uint64_t GetTtlSeconds() const { return mTtl; }
    SerializedQNameIterator GetName() const { return mNameIterator; }
    const BytesRange & GetData() const { return mData; }

    /// Parses a resource data structure
    ///
    /// Parses the daata at [start] and updates start to the end of the structure.
    /// Updates [out] with the parsed data on success.
    ///
    /// returns true on parse success, false on failure.
    bool Parse(const BytesRange & validData, const uint8_t ** start);

private:
    SerializedQNameIterator mNameIterator;
    QType mType   = QType::ANY;
    QClass mClass = QClass::ANY;
    uint64_t mTtl = 0;
    BytesRange mData;
};

class ParserDelegate
{
public:
    virtual ~ParserDelegate() {}

    virtual void OnHeader(ConstHeaderRef & header) = 0;

    virtual void OnQuery(const QueryData & data) = 0;

    virtual void OnResource(ResourceType type, const ResourceData & data) = 0;
};

/// Parses a mMDNS packet.
///
/// Calls appropriate delegate callbacks while parsing
///
/// returns true if packet was succesfully parsed, false otherwise
bool ParsePacket(const BytesRange & packetData, ParserDelegate * delegate);

} // namespace Minimal
} // namespace mdns
