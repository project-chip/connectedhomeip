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
#include <lib/mdns/minimal/records/IP.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;
using namespace chip;
using namespace chip::Inet;
using namespace chip::Encoding::BigEndian;

const QNamePart kNames[] = { "some", "test", "local" };

#if INET_CONFIG_ENABLE_IPV4
void WriteIPv4(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;

    NL_TEST_ASSERT(inSuite, IPAddress::FromString("10.20.30.40", ipAddress));

    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);

    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));
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

        NL_TEST_ASSERT(inSuite, ipResourceRecord.Append(header, ResourceType::kAnswer, output));
        NL_TEST_ASSERT(inSuite, header.GetAnswerCount() == 1);
        NL_TEST_ASSERT(inSuite, header.GetAuthorityCount() == 0);
        NL_TEST_ASSERT(inSuite, header.GetAdditionalCount() == 0);
        NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
        NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
    }

    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));

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

        NL_TEST_ASSERT(inSuite, ipResourceRecord.Append(header, ResourceType::kAuthority, output));
        NL_TEST_ASSERT(inSuite, header.GetAnswerCount() == 0);
        NL_TEST_ASSERT(inSuite, header.GetAuthorityCount() == 1);
        NL_TEST_ASSERT(inSuite, header.GetAdditionalCount() == 0);
        NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
        NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
    }

    {
        BufferWriter output(dataBuffer, sizeof(dataBuffer));

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

        NL_TEST_ASSERT(inSuite, ipResourceRecord.Append(header, ResourceType::kAdditional, output));
        NL_TEST_ASSERT(inSuite, header.GetAnswerCount() == 0);
        NL_TEST_ASSERT(inSuite, header.GetAuthorityCount() == 0);
        NL_TEST_ASSERT(inSuite, header.GetAdditionalCount() == 1);
        NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
        NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
    }
}
#endif // INET_CONFIG_ENABLE_IPV4

void WriteIPv6(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;

    NL_TEST_ASSERT(inSuite, IPAddress::FromString("fe80::224:32ff:fe19:359b", ipAddress));

    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
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

    NL_TEST_ASSERT(inSuite, ipResourceRecord.Append(header, ResourceType::kAnswer, output));
    NL_TEST_ASSERT(inSuite, header.GetAnswerCount() == 1);
    NL_TEST_ASSERT(inSuite, header.GetAuthorityCount() == 0);
    NL_TEST_ASSERT(inSuite, header.GetAdditionalCount() == 0);
    NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
    NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
}

const nlTest sTests[] = {
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("IPV4", WriteIPv4), //
#endif                              // INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("IPV6", WriteIPv6), //
    NL_TEST_SENTINEL()              //
};

} // namespace

int TestIPResourceRecord(void)
{

    nlTestSuite theSuite = { "IPResourceRecord", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestIPResourceRecord)
