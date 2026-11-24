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
#include <lib/dnssd/uld/OptRecord.h>

#include <cstring>

namespace {

using namespace chip;
using namespace chip::Encoding::BigEndian;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Uld;

TEST(TestOptRecord, WritesRootNameLeaseOption)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    OptLeaseRecord record(/*udpPayloadSize=*/1232, /*leaseSeconds=*/0x11111111, /*keyLeaseSeconds=*/0x22222222);

    const uint8_t expectedOutput[] = {
        0,                      // root name
        0,    41,               // TYPE OPT
        0x04, 0xD0,             // CLASS = UDP payload 1232
        0x00, 0x00, 0x00, 0x00, // TTL = default
        0,    12,               // RDLENGTH = 12
        0,    2,                // option code Update Lease
        0,    8,                // option length
        0x11, 0x11, 0x11, 0x11, // lease
        0x22, 0x22, 0x22, 0x22, // key lease
    };

    EXPECT_TRUE(record.Append(header, ResourceType::kAdditional, writer));
    EXPECT_EQ(header.GetAdditionalCount(), 1);
    EXPECT_EQ(output.Needed(), sizeof(expectedOutput));
    EXPECT_EQ(memcmp(dataBuffer, expectedOutput, sizeof(expectedOutput)), 0);
}

} // namespace
