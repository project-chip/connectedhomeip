/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>

namespace {

using namespace chip;
using namespace chip::Encoding::BigEndian;
using namespace mdns::Minimal;

const QNamePart kNames[] = { "foo", "bar" };

class FakeResourceRecord : public ResourceRecord
{
public:
    FakeResourceRecord(const char * data) : ResourceRecord(QType::ANY, kNames), mData(data) {}

protected:
    bool WriteData(RecordWriter & out) const override { return out.PutString(mData).Fit(); }

private:
    const char * mData;
};

TEST(TestResourceRecord, CanWriteSimpleRecord)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    FakeResourceRecord record("somedata");

    record.SetTtl(0x11223344);

    const uint8_t expectedOutput[] = {
        //
        3,    'f',  'o',  'o',  // QNAME part: foo
        3,    'b',  'a',  'r',  // QNAME part: bar
        0,                      // QNAME ends
        0,    255,              // QType ANY (totally fake)
        0,    1,                // QClass IN
        0x11, 0x22, 0x33, 0x44, // TTL
        0,    8,                // data size
        's',  'o',  'm',  'e',  'd', 'a', 't', 'a',
    };

    EXPECT_TRUE(record.Append(header, ResourceType::kAnswer, writer));
    EXPECT_EQ(header.GetAnswerCount(), 1);
    EXPECT_EQ(header.GetAuthorityCount(), 0);
    EXPECT_EQ(header.GetAdditionalCount(), 0);
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestResourceRecord, CanWriteMultipleRecords)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    FakeResourceRecord record1("somedata");
    FakeResourceRecord record2("moredata");
    FakeResourceRecord record3("xyz");

    record1.SetTtl(0x11223344);
    record2.SetTtl(0);
    record3.SetTtl(0xFF);

    const uint8_t expectedOutput[] = {
        3,    'f',  'o',  'o',                      // QNAME part: foo
        3,    'b',  'a',  'r',                      // QNAME part: bar
        0,                                          // QNAME ends
        0,    255,                                  // QType ANY (totally fake)
        0,    1,                                    // QClass IN
        0x11, 0x22, 0x33, 0x44,                     // TTL
        0,    8,                                    // data size
        's',  'o',  'm',  'e',  'd', 'a', 't', 'a', //
        0xC0, 0x00,                                 // PTR: foo.bar
        0,    255,                                  // QType ANY (totally fake)
        0,    1,                                    // QClass IN
        0,    0,    0,    0,                        // TTL
        0,    8,                                    // data size
        'm',  'o',  'r',  'e',  'd', 'a', 't', 'a', //
        0xC0, 0x00,                                 // PTR: foo.bar
        0,    255,                                  // QType ANY (totally fake)
        0,    1,                                    // QClass IN
        0,    0,    0,    0xFF,                     // TTL
        0,    3,                                    // data size
        'x',  'y',  'z',
    };

    EXPECT_TRUE(record1.Append(header, ResourceType::kAnswer, writer));
    EXPECT_EQ(header.GetAnswerCount(), 1);
    EXPECT_EQ(header.GetAuthorityCount(), 0);
    EXPECT_EQ(header.GetAdditionalCount(), 0);

    EXPECT_TRUE(record2.Append(header, ResourceType::kAuthority, writer));
    EXPECT_EQ(header.GetAnswerCount(), 1);
    EXPECT_EQ(header.GetAuthorityCount(), 1);
    EXPECT_EQ(header.GetAdditionalCount(), 0);

    EXPECT_TRUE(record3.Append(header, ResourceType::kAdditional, writer));
    EXPECT_EQ(header.GetAnswerCount(), 1);
    EXPECT_EQ(header.GetAuthorityCount(), 1);
    EXPECT_EQ(header.GetAdditionalCount(), 1);

    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestResourceRecord, RecordOrderIsEnforced)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    FakeResourceRecord record("somedata");

    header.Clear();
    header.SetAuthorityCount(1);
    EXPECT_EQ(record.Append(header, ResourceType::kAnswer, writer), false);

    header.Clear();
    header.SetAdditionalCount(1);
    EXPECT_EQ(record.Append(header, ResourceType::kAnswer, writer), false);
    EXPECT_EQ(record.Append(header, ResourceType::kAuthority, writer), false);
}

TEST(TestResourceRecord, ErrorsOutOnSmallBuffers)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[123];

    HeaderRef header(headerBuffer);

    FakeResourceRecord record("somedata");

    const uint8_t expectedOutput[] = {
        //
        3,   'f', 'o', 'o', // QNAME part: foo
        3,   'b', 'a', 'r', // QNAME part: bar
        0,                  // QNAME ends
        0,   255,           // QType ANY (totally fake)
        0,   1,             // QClass IN
        0,   0,   0,   120, // TTL
        0,   8,             // data size
        's', 'o', 'm', 'e', 'd', 'a', 't', 'a',
    };

    header.Clear();
    uint8_t clearHeader[HeaderRef::kSizeBytes];

    memcpy(clearHeader, headerBuffer, HeaderRef::kSizeBytes);

    for (size_t i = 0; i < sizeof(expectedOutput); i++)
    {
        memset(dataBuffer, 0, sizeof(dataBuffer));
        BufferWriter output(dataBuffer, i);
        RecordWriter writer(&output);

        EXPECT_EQ(record.Append(header, ResourceType::kAnswer, writer), false);

        // header untouched
        EXPECT_EQ(memcmp(headerBuffer, clearHeader, HeaderRef::kSizeBytes), 0);
    }

    memset(dataBuffer, 0, sizeof(dataBuffer));
    BufferWriter output(dataBuffer, sizeof(expectedOutput));
    RecordWriter writer(&output);

    EXPECT_TRUE(record.Append(header, ResourceType::kAnswer, writer));
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
    EXPECT_NE(memcmp(headerBuffer, clearHeader, HeaderRef::kSizeBytes), 0);
}

TEST(TestResourceRecord, RecordCount)
{
    constexpr int kAppendCount = 10;
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[123];

    HeaderRef header(headerBuffer);
    header.Clear();

    FakeResourceRecord record("somedata");

    for (int i = 0; i < kAppendCount; i++)
    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
        RecordWriter writer(&output);

        EXPECT_TRUE(record.Append(header, ResourceType::kAnswer, writer));
        EXPECT_EQ(header.GetAnswerCount(), i + 1);
        EXPECT_EQ(header.GetAuthorityCount(), 0);
        EXPECT_EQ(header.GetAdditionalCount(), 0);
    }

    for (int i = 0; i < kAppendCount; i++)
    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
        RecordWriter writer(&output);

        EXPECT_TRUE(record.Append(header, ResourceType::kAuthority, writer));
        EXPECT_EQ(header.GetAnswerCount(), kAppendCount);
        EXPECT_EQ(header.GetAuthorityCount(), i + 1);
        EXPECT_EQ(header.GetAdditionalCount(), 0);
    }

    for (int i = 0; i < kAppendCount; i++)
    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
        RecordWriter writer(&output);

        EXPECT_TRUE(record.Append(header, ResourceType::kAdditional, writer));
        EXPECT_EQ(header.GetAnswerCount(), kAppendCount);
        EXPECT_EQ(header.GetAuthorityCount(), kAppendCount);
        EXPECT_EQ(header.GetAdditionalCount(), i + 1);
    }
}
TEST(TestResourceRecord, CacheFlushBit)
{
    FakeResourceRecord record("somedata");
    // No cache flush bit by default.
    EXPECT_EQ(record.GetClass(), QClass::IN);
    EXPECT_EQ(record.GetCacheFlush(), false);

    // Check we can set flush bit and the class marker reflects that.
    record.SetCacheFlush(true);
    EXPECT_EQ(record.GetClass(), QClass::IN_FLUSH);
    EXPECT_EQ(record.GetCacheFlush(), true);

    // Check we can unset.
    record.SetCacheFlush(false);
    EXPECT_EQ(record.GetClass(), QClass::IN);
    EXPECT_EQ(record.GetCacheFlush(), false);
}
} // namespace
