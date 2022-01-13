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
#include <lib/dnssd/minimal_mdns/records/Ptr.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;
using namespace chip::Encoding;

void TestPtrResourceRecord(nlTestSuite * inSuite, void * inContext)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    const QNamePart kName[] = { "some", "test", "local" };
    const QNamePart kPtr[]  = { "ptr", "addr" };

    HeaderRef header(headerBuffer);

    BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    PtrResourceRecord record(kName, kPtr);

    record.SetTtl(123);

    header.Clear();

    const uint8_t expectedOutput[] = {
        4, 's', 'o', 'm', 'e',      // QNAME part: some
        4, 't', 'e', 's', 't',      // QNAME part: test
        5, 'l', 'o', 'c', 'a', 'l', // QNAME part: local
        0,                          // QNAME ends
        0, 12,                      // QType PTR
        0, 1,                       // QClass IN
        0, 0,   0,   123,           // TTL
        0, 10,                      // data size - size data
        3, 'p', 't', 'r',           // QNAME part: ptr
        4, 'a', 'd', 'd', 'r',      // QNAME part: addr
        0                           // QNAME ends
    };

    NL_TEST_ASSERT(inSuite, record.Append(header, ResourceType::kAnswer, writer));
    NL_TEST_ASSERT(inSuite, header.GetAnswerCount() == 1);
    NL_TEST_ASSERT(inSuite, header.GetAuthorityCount() == 0);
    NL_TEST_ASSERT(inSuite, header.GetAdditionalCount() == 0);
    NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
    NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestPtrResourceRecord", TestPtrResourceRecord), //
    NL_TEST_SENTINEL()                                           //
};

} // namespace

int TestPtrResourceRecord(void)
{
    nlTestSuite theSuite = { "PtrResourceRecord", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPtrResourceRecord)
