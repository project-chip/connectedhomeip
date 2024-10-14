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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>

namespace {

using namespace chip;
using namespace chip::Encoding;
using namespace mdns::Minimal;

TEST(TestResourceRecordTxt, TestTxt)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    const QNamePart kName[] = { "some", "test", "local" };
    const char * kData[]    = { "a=b", "foo=bar", "flag" };

    HeaderRef header(headerBuffer);

    BigEndian::BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    TxtResourceRecord record(kName, kData);
    record.SetTtl(128);
    EXPECT_EQ(record.GetNumEntries(), 3u);

    header.Clear();

    EXPECT_TRUE(record.Append(header, ResourceType::kAdditional, writer));
    EXPECT_EQ(header.GetAnswerCount(), 0);
    EXPECT_EQ(header.GetAuthorityCount(), 0);
    EXPECT_EQ(header.GetAdditionalCount(), 1);

    const uint8_t expectedOutput[] = {
        4, 's', 'o', 'm', 'e',                // QNAME part: some
        4, 't', 'e', 's', 't',                // QNAME part: test
        5, 'l', 'o', 'c', 'a', 'l',           // QNAME part: local
        0,                                    // QNAME ends
        0, 16,                                // QType TXT
        0, 1,                                 // QClass IN
        0, 0,   0,   128,                     // TTL
        0, 17,                                // data size
        3, 'a', '=', 'b',                     // ENTRY: a=b
        7, 'f', 'o', 'o', '=', 'b', 'a', 'r', // ENTRY: foo=bar
        4, 'f', 'l', 'a', 'g'                 // ENTRY: flag
    };

    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}
} // namespace
