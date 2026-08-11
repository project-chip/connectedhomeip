/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/dnssd/wire/Parser.h>

#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

constexpr uint16_t kKeyRecordType = 25;

std::vector<std::string> NameLabels(SerializedQNameIterator it)
{
    std::vector<std::string> labels;
    while (it.Next())
    {
        labels.push_back(it.Value());
    }
    return labels;
}

struct ParsedQuery
{
    uint16_t type;
    QClass klass;
    bool unicastAnswer;
    std::vector<std::string> name;
};

struct ParsedResource
{
    ResourceType section;
    uint16_t type;
    QClass klass;
    uint64_t ttl;
    std::vector<std::string> name;
    std::vector<uint8_t> data;
};

class PacketCapture : public ParserDelegate
{
public:
    void OnHeader(ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }

    void OnQuery(const QueryData & data) override
    {
        mQueries.push_back(ParsedQuery{ static_cast<uint16_t>(data.GetType()), data.GetClass(), data.RequestedUnicastAnswer(),
                                        NameLabels(data.GetName()) });
    }

    void OnResource(ResourceType section, const ResourceData & data) override
    {
        mResources.push_back(ParsedResource{ section, static_cast<uint16_t>(data.GetType()), data.GetClass(), data.GetTtlSeconds(),
                                             NameLabels(data.GetName()),
                                             std::vector<uint8_t>(data.GetData().Start(), data.GetData().End()) });
    }

    uint16_t mMessageId = 0;
    std::vector<ParsedQuery> mQueries;
    std::vector<ParsedResource> mResources;
};

TEST(TestParser, ParsesQuestion)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x00, 0x00, // flags: query, opcode 0, rcode 0
        0x00, 0x01, // one question
        0x00, 0x00, // no answers
        0x00, 0x00, // no authority records
        0x00, 0x00, // no additional records

        4,    'h',  'o', 's', 't',      // QNAME part: host
        5,    'l',  'o', 'c', 'a', 'l', // QNAME part: local
        0,                              // QNAME ends
        0x00, 0x0C,                     // QTYPE PTR
        0x00, 0x01,                     // QCLASS IN
    };

    PacketCapture capture;
    ASSERT_TRUE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));

    EXPECT_EQ(capture.mMessageId, 0x1234);
    EXPECT_TRUE(capture.mResources.empty());
    ASSERT_EQ(capture.mQueries.size(), 1u);

    const ParsedQuery & query = capture.mQueries[0];
    EXPECT_EQ(query.type, static_cast<uint16_t>(QType::PTR));
    EXPECT_EQ(query.klass, QClass::IN);
    EXPECT_FALSE(query.unicastAnswer);
    EXPECT_EQ(query.name, (std::vector<std::string>{ "host", "local" }));
}

TEST(TestParser, QuestionUnicastFlagIsSplitFromClass)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x00, 0x00, // flags: query, opcode 0, rcode 0
        0x00, 0x01, // one question
        0x00, 0x00, // no answers
        0x00, 0x00, // no authority records
        0x00, 0x00, // no additional records

        4,    'h',  'o', 's', 't', // QNAME part: host
        0,                         // QNAME ends
        0x00, 0x01,                // QTYPE A
        0x80, 0x01,                // QCLASS IN with the unicast-response bit set
    };

    PacketCapture capture;
    ASSERT_TRUE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
    ASSERT_EQ(capture.mQueries.size(), 1u);

    // The unicast bit is reported separately and must not leak into the class.
    EXPECT_TRUE(capture.mQueries[0].unicastAnswer);
    EXPECT_EQ(capture.mQueries[0].klass, QClass::IN);
}

TEST(TestParser, ReportsEachSectionSeparately)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x84, 0x00, // flags: response, authoritative, opcode 0, rcode 0
        0x00, 0x00, // no questions
        0x00, 0x01, // one answer
        0x00, 0x01, // one authority record
        0x00, 0x01, // one additional record

        1,    'a',              // QNAME part: a
        0,                      // QNAME ends
        0x00, 0x10,             // TYPE TXT
        0x80, 0x01,             // CLASS IN with the mDNS cache-flush bit set
        0x00, 0x00, 0x00, 0x0A, // TTL 10
        0x00, 0x01,             // RDLENGTH
        0xAA,

        1,    'b',              // QNAME part: b
        0,                      // QNAME ends
        0x00, 0x02,             // TYPE NS
        0x00, 0x01,             // CLASS IN
        0x00, 0x00, 0x00, 0x14, // TTL 20
        0x00, 0x01,             // RDLENGTH
        0xBB,

        1,    'c',              // QNAME part: c
        0,                      // QNAME ends
        0x00, 0x01,             // TYPE A
        0x00, 0x01,             // CLASS IN
        0x00, 0x00, 0x00, 0x1E, // TTL 30
        0x00, 0x01,             // RDLENGTH
        0xCC,
    };

    PacketCapture capture;
    ASSERT_TRUE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
    ASSERT_EQ(capture.mResources.size(), 3u);

    EXPECT_EQ(capture.mResources[0].section, ResourceType::kAnswer);
    EXPECT_EQ(capture.mResources[0].name, (std::vector<std::string>{ "a" }));
    EXPECT_EQ(capture.mResources[0].ttl, 10u);
    EXPECT_EQ(capture.mResources[0].data, (std::vector<uint8_t>{ 0xAA }));
    // Unlike the question unicast bit, the cache-flush bit stays part of the class.
    EXPECT_EQ(capture.mResources[0].klass, QClass::IN_FLUSH);

    EXPECT_EQ(capture.mResources[1].section, ResourceType::kAuthority);
    EXPECT_EQ(capture.mResources[1].name, (std::vector<std::string>{ "b" }));
    EXPECT_EQ(capture.mResources[1].ttl, 20u);
    EXPECT_EQ(capture.mResources[1].data, (std::vector<uint8_t>{ 0xBB }));
    EXPECT_EQ(capture.mResources[1].klass, QClass::IN);

    EXPECT_EQ(capture.mResources[2].section, ResourceType::kAdditional);
    EXPECT_EQ(capture.mResources[2].name, (std::vector<std::string>{ "c" }));
    EXPECT_EQ(capture.mResources[2].ttl, 30u);
    EXPECT_EQ(capture.mResources[2].data, (std::vector<uint8_t>{ 0xCC }));
}

TEST(TestParser, ResolvesCompressedRecordName)
{
    const uint8_t packet[] = {
        0x12,
        0x34, // message id
        0x00,
        0x00, // flags: query, opcode 0, rcode 0
        0x00,
        0x01, // one question
        0x00,
        0x01, // one answer
        0x00,
        0x00, // no authority records
        0x00,
        0x00, // no additional records

        // Question name starts at offset 12.
        4,
        'h',
        'o',
        's',
        't', // QNAME part: host
        5,
        'l',
        'o',
        'c',
        'a',
        'l', // QNAME part: local
        0,   // QNAME ends
        0x00,
        0x01, // QTYPE A
        0x00,
        0x01, // QCLASS IN

        0xC0,
        0x0C, // QNAME: pointer back to offset 12
        0x00,
        0x01, // TYPE A
        0x00,
        0x01, // CLASS IN
        0x00,
        0x00,
        0x00,
        0x3C, // TTL 60
        0x00,
        0x04, // RDLENGTH
        10,
        0,
        0,
        1,
    };

    PacketCapture capture;
    ASSERT_TRUE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
    ASSERT_EQ(capture.mResources.size(), 1u);

    EXPECT_EQ(capture.mResources[0].name, (std::vector<std::string>{ "host", "local" }));
    EXPECT_EQ(capture.mResources[0].data, (std::vector<uint8_t>{ 10, 0, 0, 1 }));
}

TEST(TestParser, UnknownRecordTypeKeepsDataOpaque)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x00, 0x00, // flags: query, opcode 0, rcode 0
        0x00, 0x00, // no questions
        0x00, 0x00, // no answers
        0x00, 0x00, // no authority records
        0x00, 0x01, // one additional record

        4,    'h',  'o',  's',  't',       // QNAME part: host
        5,    'l',  'o',  'c',  'a',  'l', // QNAME part: local
        0,                                 // QNAME ends
        0x00, 0x19,                        // TYPE 25 (KEY)
        0x00, 0x01,                        // CLASS IN
        0x00, 0x00, 0x0E, 0x10,            // TTL 3600
        0x00, 0x06,                        // RDLENGTH
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    };

    PacketCapture capture;
    ASSERT_TRUE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));

    EXPECT_EQ(capture.mMessageId, 0x1234);
    EXPECT_TRUE(capture.mQueries.empty());
    ASSERT_EQ(capture.mResources.size(), 1u);

    const ParsedResource & resource = capture.mResources[0];
    EXPECT_EQ(resource.section, ResourceType::kAdditional);
    EXPECT_EQ(resource.type, kKeyRecordType);
    EXPECT_EQ(resource.klass, QClass::IN);
    EXPECT_EQ(resource.ttl, 3600u);
    EXPECT_EQ(resource.data, (std::vector<uint8_t>{ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 }));
}

TEST(TestParser, AcceptsHeaderOnlyPacket)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x00, 0x00, // flags: query, opcode 0, rcode 0
        0x00, 0x00, // no questions
        0x00, 0x00, // no answers
        0x00, 0x00, // no authority records
        0x00, 0x00, // no additional records
    };

    PacketCapture capture;
    EXPECT_TRUE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
    EXPECT_EQ(capture.mMessageId, 0x1234);
    EXPECT_TRUE(capture.mQueries.empty());
    EXPECT_TRUE(capture.mResources.empty());
}

TEST(TestParser, RejectsNonZeroOpcode)
{
    // RFC 6762 section 18.3 requires mDNS receivers to ignore anything with a non-zero opcode.
    // receivers to ignore anything with a non-zero opcode, so this entry point
    // refuses to parse it.

    // TODO: DNS Update carries an OpCode (5) and often non-zero return code.(RFC 2136)
    // Update this test when we add ULD/SRP support.
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x28, 0x00, // flags: opcode 5
        0x00, 0x00, // no questions
        0x00, 0x00, // no answers
        0x00, 0x00, // no authority records
        0x00, 0x00, // no additional records
    };

    PacketCapture capture;
    EXPECT_FALSE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
    EXPECT_TRUE(capture.mResources.empty());
}

TEST(TestParser, RejectsTruncatedHeader)
{
    const uint8_t packet[] = { 0x12, 0x34, 0x00, 0x00, 0x00, 0x00 };

    PacketCapture capture;
    EXPECT_FALSE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
}

TEST(TestParser, RejectsRecordDataPastEndOfPacket)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0x00, 0x00, // flags: query, opcode 0, rcode 0
        0x00, 0x00, // no questions
        0x00, 0x01, // one answer
        0x00, 0x00, // no authority records
        0x00, 0x00, // no additional records

        4,    'h',  'o',  's',  't', // QNAME part: host
        0,                           // QNAME ends
        0x00, 0x19,                  // TYPE 25 (KEY)
        0x00, 0x01,                  // CLASS IN
        0x00, 0x00, 0x0E, 0x10,      // TTL 3600
        0x00, 0x20,                  // RDLENGTH claims 32 bytes ...
        0x01, 0x02,                  // ... but only 2 are present
    };

    PacketCapture capture;
    EXPECT_FALSE(ParsePacket(BytesRange::BufferWithSize(packet, sizeof(packet)), &capture));
}

} // namespace
