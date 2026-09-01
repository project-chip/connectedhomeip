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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/uld/DeleteRecord.h>
#include <lib/dnssd/wire/records/IP.h>

#include <cstring>

namespace {

using namespace chip;
using namespace chip::Encoding::BigEndian;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Uld;

const QNamePart kNames[] = { "host", "local" };

TEST(TestDeleteRecord, DeleteRrsetWritesEmptyRdata)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    DeleteRrsetRecord record(kNames, QType::ANY);

    const uint8_t expectedOutput[] = {
        4, 'h', 'o', 's', 't',      //
        5, 'l', 'o', 'c', 'a', 'l', //
        0,                          //
        0, 255,                     // QType ANY
        0, 255,                     // QClass ANY
        0, 0,   0,   0,             // TTL 0
        0, 0,                       // RDLENGTH 0
    };

    EXPECT_TRUE(record.Append(header, ResourceType::kAuthority, writer));
    EXPECT_EQ(header.GetAuthorityCount(), 1);
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

TEST(TestDeleteRecord, DeleteSpecificUsesClassNoneWithMatchingRdata)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    Inet::IPAddress addr;
    ASSERT_TRUE(Inet::IPAddress::FromString("fd00::1", addr));

    IPResourceRecord record(kNames, addr);
    record.SetClass(QClass::NONE).SetTtl(0);

    EXPECT_TRUE(record.Append(header, ResourceType::kAuthority, writer));
    EXPECT_EQ(header.GetAuthorityCount(), 1);

    // TYPE AAAA, CLASS NONE, TTL 0, RDLENGTH 16 + 16 address bytes after the name.
    // Validate class / ttl / rdlength fields rather than the full address encoding.
    const size_t nameBytes = 1 + 4 + 1 + 5 + 1; // host.local.
    EXPECT_EQ(dataBuffer[nameBytes], 0);        // type hi
    EXPECT_EQ(dataBuffer[nameBytes + 1], static_cast<uint8_t>(QType::AAAA));
    EXPECT_EQ(dataBuffer[nameBytes + 2], 0); // class hi
    EXPECT_EQ(dataBuffer[nameBytes + 3], static_cast<uint8_t>(QClass::NONE));
    EXPECT_EQ(dataBuffer[nameBytes + 4], 0); // ttl
    EXPECT_EQ(dataBuffer[nameBytes + 5], 0);
    EXPECT_EQ(dataBuffer[nameBytes + 6], 0);
    EXPECT_EQ(dataBuffer[nameBytes + 7], 0);
    EXPECT_EQ(dataBuffer[nameBytes + 8], 0); // rdlength hi
    EXPECT_EQ(dataBuffer[nameBytes + 9], 16);
}

} // namespace
