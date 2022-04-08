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
#include <lib/dnssd/minimal_mdns/records/Srv.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Encoding;
using namespace mdns::Minimal;

void TestSrv(nlTestSuite * inSuite, void * inContext)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    const QNamePart kName[]       = { "some", "test", "local" };
    const QNamePart kServerName[] = { "my", "server", "name" };
    constexpr uint16_t kPort      = 0x1234;

    HeaderRef header(headerBuffer);

    BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    SrvResourceRecord record(kName, kServerName, kPort);
    record.SetTtl(128);

    header.Clear();

    NL_TEST_ASSERT(inSuite, record.Append(header, ResourceType::kAdditional, writer));
    NL_TEST_ASSERT(inSuite, header.GetAnswerCount() == 0);
    NL_TEST_ASSERT(inSuite, header.GetAuthorityCount() == 0);
    NL_TEST_ASSERT(inSuite, header.GetAdditionalCount() == 1);

    const uint8_t expectedOutput[] = {
        4,    's',  'o', 'm', 'e',           // QNAME part: some
        4,    't',  'e', 's', 't',           // QNAME part: test
        5,    'l',  'o', 'c', 'a', 'l',      // QNAME part: local
        0,                                   // QNAME ends
        0,    33,                            // QType SRV
        0,    1,                             // QClass IN
        0,    0,    0,   128,                // TTL
        0,    22,                            // data size
        0,    0,                             // Priority
        0,    0,                             // Weight
        0x12, 0x34,                          // port
        2,    'm',  'y',                     // QNAME part: my
        6,    's',  'e', 'r', 'v', 'e', 'r', // QNAME part: server
        4,    'n',  'a', 'm', 'e',           // QNAME part: name
        0                                    // QNAME ends
    };

    NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
    NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestSrv", TestSrv), //
    NL_TEST_SENTINEL()               //
};

} // namespace

int TestSrv(void)
{
    nlTestSuite theSuite = { "Srv", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSrv)
