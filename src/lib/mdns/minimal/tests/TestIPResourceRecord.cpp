
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

#include "TestMinimalMdns.h"

#include <mdns/minimal/IPResourceRecord.h>
#include <support/TestUtils.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;
using namespace chip;
using namespace chip::Inet;

constexpr uint16_t kTestQnameCount        = 3;
const char * kTestQnames[kTestQnameCount] = { "some", "test", "local" };

void WriteIPv4(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;

    NL_TEST_ASSERT(inSuite, IPAddress::FromString("10.20.30.40", ipAddress));

    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    {
        HeaderRef header(headerBuffer);
        BufBound output(dataBuffer, sizeof(dataBuffer));

        IPResourceRecord ipResourceRecord(kTestQnames, kTestQnameCount, ipAddress);

        ipResourceRecord.SetTtl(123);

        header.Clear();

        const uint8_t expectedOutput[] = {
            //
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
        NL_TEST_ASSERT(inSuite, output.Written() == sizeof(expectedOutput));
        NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
    }

    // FIXME: implement
}

void WriteIPv6(nlTestSuite * inSuite, void * inContext)
{
    // FIXME: implement
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("IPV4", WriteIPv4),
    NL_TEST_DEF("IPV6", WriteIPv6),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestIPResourceRecord(void)
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "QName",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestIPResourceRecord)
