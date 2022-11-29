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

#include <lib/dnssd/minimal_mdns/core/RecordWriter.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;
using namespace chip::Encoding::BigEndian;

void BasicWriteTest(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
    NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
}

void SimpleDedup(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
    NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
}

void ComplexDedup(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, output.Needed() == sizeof(expectedOutput));
    NL_TEST_ASSERT(inSuite, memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)) == 0);
}

void TonsOfReferences(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, output.Fit());
    NL_TEST_ASSERT(inSuite, output.Needed() == 423);
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicWriteTest", BasicWriteTest),
    NL_TEST_DEF("SimpleDedup", SimpleDedup),
    NL_TEST_DEF("ComplexDedup", ComplexDedup),
    NL_TEST_DEF("TonsOfReferences", TonsOfReferences),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestRecordWriter()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "RecordWriter",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestRecordWriter)
