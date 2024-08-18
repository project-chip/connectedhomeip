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
#include <lib/dnssd/minimal_mdns/records/IP.h>

namespace {

using namespace mdns::Minimal;
using namespace chip;
using namespace chip::Inet;
using namespace chip::Encoding::BigEndian;

const QNamePart kNames[] = { "some", "test", "local" };

#if INET_CONFIG_ENABLE_IPV4
TEST(TestResourceRecordIP, WriteIPv4)
{
    IPAddress ipAddress;

    EXPECT_TRUE(IPAddress::FromString("10.20.30.40", ipAddress));

    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);

    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
        RecordWriter writer(&output);

        IPResourceRecord ipResourceRecord(kNames, ipAddress);

        ipResourceRecord.SetTtl(123);

        header.Clear();

        const uint8_t expectedOutput[] = {
            4,  's', 'o', 'm', 'e',      // QNAME part: some
            4,  't', 'e', 's', 't',      // QNAME part: test
            5,  'l', 'o', 'c', 'a', 'l', // QNAME part: local
            0,                           // QNAME ends
            0,  1,                       // QClass IN
            0,  1,                       // QType A
            0,  0,   0,   123,           // TTL
            0,  4,                       // data size - size for IPv4
            10, 20,  30,  40             // IP Address
        };

        EXPECT_TRUE(ipResourceRecord.Append(header, ResourceType::kAnswer, writer));
        EXPECT_EQ(header.GetAnswerCount(), 1);
        EXPECT_EQ(header.GetAuthorityCount(), 0);
        EXPECT_EQ(header.GetAdditionalCount(), 0);
        EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
        EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
    }

    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
        RecordWriter writer(&output);

        IPResourceRecord ipResourceRecord(kNames, ipAddress);

        ipResourceRecord.SetTtl(234);

        header.Clear();

        const uint8_t expectedOutput[] = {
            4,  's', 'o', 'm', 'e',      // QNAME part: some
            4,  't', 'e', 's', 't',      // QNAME part: test
            5,  'l', 'o', 'c', 'a', 'l', // QNAME part: local
            0,                           // QNAME ends
            0,  1,                       // QClass IN
            0,  1,                       // QType A
            0,  0,   0,   234,           // TTL
            0,  4,                       // data size - size for IPv4
            10, 20,  30,  40             // IP Address
        };

        EXPECT_TRUE(ipResourceRecord.Append(header, ResourceType::kAuthority, writer));
        EXPECT_EQ(header.GetAnswerCount(), 0);
        EXPECT_EQ(header.GetAuthorityCount(), 1);
        EXPECT_EQ(header.GetAdditionalCount(), 0);
        EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
        EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
    }

    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
        RecordWriter writer(&output);

        IPResourceRecord ipResourceRecord(kNames, ipAddress);

        ipResourceRecord.SetTtl(0x1234);

        header.Clear();

        const uint8_t expectedOutput[] = {
            4,  's', 'o',  'm',  'e',      // QNAME part: some
            4,  't', 'e',  's',  't',      // QNAME part: test
            5,  'l', 'o',  'c',  'a', 'l', // QNAME part: local
            0,                             // QNAME ends
            0,  1,                         // QClass IN
            0,  1,                         // QType A
            0,  0,   0x12, 0x34,           // TTL
            0,  4,                         // data size - size for IPv4
            10, 20,  30,   40              // IP Address
        };

        EXPECT_TRUE(ipResourceRecord.Append(header, ResourceType::kAdditional, writer));
        EXPECT_EQ(header.GetAnswerCount(), 0);
        EXPECT_EQ(header.GetAuthorityCount(), 0);
        EXPECT_EQ(header.GetAdditionalCount(), 1);
        EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
        EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
    }
}
#endif // INET_CONFIG_ENABLE_IPV4

TEST(TestResourceRecordIP, WriteIPv6)
{
    IPAddress ipAddress;

    EXPECT_TRUE(IPAddress::FromString("fe80::224:32ff:fe19:359b", ipAddress));

    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);
    IPResourceRecord ipResourceRecord(kNames, ipAddress);

    ipResourceRecord.SetTtl(0x12345678);

    header.Clear();

    const uint8_t expectedOutput[] = {                                   //
                                       4,    's',  'o',  'm',  'e',      // QNAME part: some
                                       4,    't',  'e',  's',  't',      // QNAME part: test
                                       5,    'l',  'o',  'c',  'a', 'l', // QNAME part: local
                                       0,                                // QNAME ends
                                       0,    28,                         // QType AAAA
                                       0,    1,                          // QClass IN
                                       0x12, 0x34, 0x56, 0x78,           // TTL
                                       0,    16,                         // data size - size for IPv4
                                       0xfe, 0x80, 0x00, 0x00,           // IPv6
                                       0x00, 0x00, 0x00, 0x00,           //
                                       0x02, 0x24, 0x32, 0xff,           //
                                       0xfe, 0x19, 0x35, 0x9b
    };

    EXPECT_TRUE(ipResourceRecord.Append(header, ResourceType::kAnswer, writer));
    EXPECT_EQ(header.GetAnswerCount(), 1);
    EXPECT_EQ(header.GetAuthorityCount(), 0);
    EXPECT_EQ(header.GetAdditionalCount(), 0);
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}
} // namespace
