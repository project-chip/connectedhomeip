/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <cstring>

#include <inet/IPAddress.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/wire/RecordWriter.h>

namespace {

using namespace mdns::Minimal;
using namespace chip::Encoding::BigEndian;

TEST(TestRecordWriter, BasicWriteTest)
{
    const QNamePart kName1[] = { "some", "name" };
    const QNamePart kName2[] = { "abc", "xyz", "here" };

    uint8_t dataBuffer[128];

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.WriteQName(FullQName(kName1));
    writer.WriteQName(FullQName(kName2));

    // clang-format off
    const uint8_t expectedOutput[] = {
        //
        4, 's', 'o', 'm', 'e', // QNAME part: some
        4, 'n', 'a', 'm', 'e', // QNAME part: name
        0,                    // QNAME ends
        3, 'a', 'b', 'c',     // QNAME part: abc
        3, 'x', 'y', 'z',     // QNAME part: xyz
        4, 'h', 'e', 'r', 'e', // QNAME part: here
        0,  // QNAME ends
    };
    // clang-format on

    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, SimpleDedup)
{
    const QNamePart kName1[] = { "some", "name" };
    const QNamePart kName2[] = { "other", "name" };

    uint8_t dataBuffer[128];

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.WriteQName(FullQName(kName1));
    writer.WriteQName(FullQName(kName2));

    // clang-format off
    const uint8_t expectedOutput[] = {
        //
        4, 's', 'o', 'm', 'e',      // QNAME part: some
        4, 'n', 'a', 'm', 'e',      // QNAME part: name
        0,                          // QNAME ends
        5, 'o', 't', 'h', 'e', 'r', // QNAME part: other
        0xC0, 5                     // POINTER: "name" is at offset 5
    };
    // clang-format on

    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, ComplexDedup)
{
    const QNamePart kName1[] = { "some", "name" };
    const QNamePart kName2[] = { "other", "name" };
    const QNamePart kName3[] = { "prefix", "of", "other", "name" };
    const QNamePart kName4[] = { "some", "name", "suffix" };
    const QNamePart kName5[] = { "more", "suffix" };

    uint8_t dataBuffer[128];

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.WriteQName(FullQName(kName1));
    writer.WriteQName(FullQName(kName2));
    writer.WriteQName(FullQName(kName3));
    writer.Writer().Put("xyz"); // inject something that is NOT a qname
    writer.WriteQName(FullQName(kName4));
    writer.WriteQName(FullQName(kName5));

    // clang-format off
    const uint8_t expectedOutput[] = {
        //
        4, 's', 'o', 'm', 'e',      // QNAME part: some
        4, 'n', 'a', 'm', 'e',      // QNAME part: name
        0,                          // QNAME ends
        5, 'o', 't', 'h', 'e', 'r', // QNAME part: other
        0xC0, 5,                    // POINTER: "name" is at offset 5
        6, 'p', 'r', 'e', 'f', 'i', 'x',
        2, 'o', 'f',
        0xC0, 11,                   // POINTER: "other.name" is at offset 11
        'x', 'y', 'z',
        4, 's', 'o', 'm', 'e',            // QNAME part: some
        4, 'n', 'a', 'm', 'e',            // QNAME part: name
        6, 's', 'u', 'f', 'f', 'i', 'x',  // suffix which prevents reuse
        0,
        4, 'm', 'o', 'r', 'e',
        0xC0, 44
    };
    // clang-format on

    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, TonsOfReferences)
{
    const QNamePart kName1[] = { "some", "name" };
    const QNamePart kName2[] = { "different", "name" };

    uint8_t dataBuffer[512];

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    // First name is 11 bytes (2*4 bytes + null terminator)
    // all other entires are 2 bytes (back - references)
    //
    // TOTAL: 211 bytes written
    for (int i = 0; i < 101; i++)
    {
        writer.WriteQName(FullQName(kName1));
    }

    // Extra size: 10 for "different" and 2 for "name" link
    // TOTAL: 211 + 12 =  223
    writer.WriteQName(FullQName(kName2));

    // Another 200 bytes for references
    // TOTAL: 423
    for (int i = 0; i < 100; i++)
    {
        writer.WriteQName(FullQName(kName2));
    }

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), 423u);
}

TEST(TestRecordWriter, PutSrv)
{
    const QNamePart kTarget[] = { "myserver", "local" };

    uint8_t dataBuffer[128];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.PutSrv(1, 2, 3, FullQName(kTarget));

    // clang-format off
    const uint8_t expectedOutput[] = {
        0, 1,                                   // priority
        0, 2,                                   // weight
        0, 3,                                   // port
        8, 'm', 'y', 's', 'e', 'r', 'v', 'e', 'r',
        5, 'l', 'o', 'c', 'a', 'l',
        0,
    };
    // clang-format on

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, PutPtr)
{
    const QNamePart kName[] = { "ptr", "target" };

    uint8_t dataBuffer[128];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.PutPtr(FullQName(kName));

    // clang-format off
    const uint8_t expectedOutput[] = {
        3, 'p', 't', 'r',
        6, 't', 'a', 'r', 'g', 'e', 't',
        0,
    };
    // clang-format on

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, PutIpAddressV4)
{
    chip::Inet::IPAddress addr;
    ASSERT_TRUE(chip::Inet::IPAddress::FromString("10.20.30.40", addr));

    uint8_t dataBuffer[32];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.PutIpAddress(addr);

    const uint8_t expectedOutput[] = { 10, 20, 30, 40 };

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, PutIpAddressV6)
{
    chip::Inet::IPAddress addr;
    ASSERT_TRUE(chip::Inet::IPAddress::FromString("fe80::1", addr));

    uint8_t dataBuffer[32];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.PutIpAddress(addr);

    // clang-format off
    const uint8_t expectedOutput[] = {
        0xfe, 0x80, 0, 0, 0, 0, 0, 0,
        0,    0,    0, 0, 0, 0, 0, 1,
    };
    // clang-format on

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, PutTxt)
{
    const char * kEntries[] = { "a=b", "flag" };

    uint8_t dataBuffer[128];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    EXPECT_TRUE(writer.PutTxt(chip::Span<const char * const>(kEntries)));

    // clang-format off
    const uint8_t expectedOutput[] = {
        3, 'a', '=', 'b',
        4, 'f', 'l', 'a', 'g',
    };
    // clang-format on

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, PutTxtRejectsOverlongEntry)
{
    // Try to write a TXT entry that is one octet over kMaxTxtRecordLength.
    char kLong[RecordWriter::kMaxTxtRecordLength + 2];
    memset(kLong, 'a', RecordWriter::kMaxTxtRecordLength + 1);
    kLong[RecordWriter::kMaxTxtRecordLength + 1] = '\0';
    ASSERT_EQ(strlen(kLong), RecordWriter::kMaxTxtRecordLength + 1);
    const char * kEntries[] = { kLong };

    uint8_t dataBuffer[128];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    EXPECT_FALSE(writer.PutTxt(chip::Span<const char * const>(kEntries)));
}

TEST(TestRecordWriter, ReserveAndFinishRdlength)
{
    uint8_t dataBuffer[64];
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    auto rdlength = writer.ReserveRdlength();
    writer.Put16(0xABCD).Put8(0x42); // 3 bytes of record data
    writer.FinishRdlength(rdlength);

    // clang-format off
    const uint8_t expectedOutput[] = {
        0, 3,               // backpatched RDLENGTH == 3
        0xAB, 0xCD, 0x42,   // record data
    };
    // clang-format on

    EXPECT_TRUE(output.Fit());
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestRecordWriter, BufferTooSmall)
{
    const QNamePart kTarget[] = { "myserver", "local" };

    uint8_t dataBuffer[4]; // far too small for an SRV record
    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    writer.PutSrv(1, 2, 3, FullQName(kTarget));

    EXPECT_FALSE(output.Fit());
}

} // namespace
