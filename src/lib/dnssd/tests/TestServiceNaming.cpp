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

#include <lib/dnssd/ServiceNaming.h>

#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

using namespace chip;
using namespace chip::Dnssd;

namespace {

TEST(TestServiceNaming, TestMakeInstanceName)
{
    char buffer[128];

    EXPECT_EQ(MakeInstanceName(buffer, sizeof(buffer), PeerId().SetCompressedFabricId(0x1234).SetNodeId(0x5678)), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "0000000000001234-0000000000005678");

    EXPECT_EQ(MakeInstanceName(buffer, sizeof(buffer),
                               PeerId().SetCompressedFabricId(0x1122334455667788ULL).SetNodeId(0x123456789abcdefULL)),
              CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "1122334455667788-0123456789ABCDEF");

    // insufficient buffer size:
    //  buffer needs at least space for hex encoding + separator + 0 terminator
    constexpr size_t kMinBufferSize = 2 * 16 + 1 + 1;
    for (size_t shortSize = 0; shortSize < kMinBufferSize; shortSize++)
    {
        EXPECT_NE(MakeInstanceName(buffer, shortSize, PeerId()), CHIP_NO_ERROR);
    }
    EXPECT_EQ(MakeInstanceName(buffer, kMinBufferSize, PeerId()), CHIP_NO_ERROR);
}

TEST(TestServiceNaming, TestExtractIdFromInstanceName)
{
    PeerId peerId;

    EXPECT_EQ(ExtractIdFromInstanceName(nullptr, nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ExtractIdFromInstanceName("ACDEF1234567890-1234567890ABCDEF", nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ExtractIdFromInstanceName(nullptr, &peerId), CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(ExtractIdFromInstanceName("ABCDEF1234567890-1234567890ABCDEF", &peerId), CHIP_NO_ERROR);
    EXPECT_EQ(peerId, PeerId().SetCompressedFabricId(0xABCDEF1234567890ULL).SetNodeId(0x1234567890ABCDEFULL));

    // ending in period (partial name) is acceptable
    EXPECT_EQ(ExtractIdFromInstanceName("1122334455667788-AABBCCDDEEFF1122.some.suffix.here", &peerId), CHIP_NO_ERROR);
    EXPECT_EQ(peerId, PeerId().SetCompressedFabricId(0x1122334455667788ULL).SetNodeId(0xaabbccddeeff1122ULL));

    // Invalid: non hex character
    EXPECT_NE(ExtractIdFromInstanceName("1x22334455667788-AABBCCDDEEDD1122", &peerId), CHIP_NO_ERROR);

    // Invalid: missing node id part (no - separator)
    EXPECT_NE(ExtractIdFromInstanceName("1122334455667788x2233445566778899", &peerId), CHIP_NO_ERROR);
    EXPECT_NE(ExtractIdFromInstanceName("1122334455667788x2233445566778899.12-33.4455", &peerId), CHIP_NO_ERROR);
    EXPECT_NE(ExtractIdFromInstanceName("1122334455667788x2233445566778899.4455", &peerId), CHIP_NO_ERROR);

    // Invalid: missing part
    EXPECT_NE(ExtractIdFromInstanceName("-1234567890ABCDEF", &peerId), CHIP_NO_ERROR);
    EXPECT_NE(ExtractIdFromInstanceName("1234567890ABCDEF-", &peerId), CHIP_NO_ERROR);

    // Invalid: separator in wrong place
    EXPECT_NE(ExtractIdFromInstanceName("112233445566778-8AABBCCDDEEFF1122", &peerId), CHIP_NO_ERROR);
    EXPECT_NE(ExtractIdFromInstanceName("1122334455667788A-ABBCCDDEEFF1122", &peerId), CHIP_NO_ERROR);

    // Invalid: fabric part too short
    EXPECT_NE(ExtractIdFromInstanceName("11223344556677-AABBCCDDEEFF1122", &peerId), CHIP_NO_ERROR);
    // Invalid: fabric part too long
    EXPECT_NE(ExtractIdFromInstanceName("112233445566778899-AABBCCDDEEFF1122", &peerId), CHIP_NO_ERROR);

    // Invalid: node part too short
    EXPECT_NE(ExtractIdFromInstanceName("1122334455667788-AABBCCDDEEFF11", &peerId), CHIP_NO_ERROR);
    // Invalid: node part too long
    EXPECT_NE(ExtractIdFromInstanceName("1122334455667788-AABBCCDDEEFF112233", &peerId), CHIP_NO_ERROR);
}

TEST(TestServiceNaming, TestMakeServiceNameSubtype)
{
    constexpr size_t kSize = 19;
    char buffer[kSize];
    DiscoveryFilter filter;

    // Long tests
    filter.type = DiscoveryFilterType::kLongDiscriminator;
    filter.code = 3;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_L3");

    filter.code = (1 << 12) - 1;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_L4095");

    filter.code = 1 << 12;
    EXPECT_NE(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);

    // Short tests
    filter.type = DiscoveryFilterType::kShortDiscriminator;
    filter.code = 3;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_S3");

    filter.code = (1 << 4) - 1;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_S15");

    filter.code = 1 << 4;
    EXPECT_NE(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);

    // Vendor tests
    filter.type = DiscoveryFilterType::kVendorId;
    filter.code = 3;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_V3");
    filter.code = 0xFFFF;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_V65535");
    filter.code = 1 << 16;
    EXPECT_NE(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);

    // Device Type tests
    filter.type = DiscoveryFilterType::kDeviceType;
    filter.code = 3;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_T3");
    // TODO: Add tests for longer device types once spec issue #3226 is closed.

    // Commissioning mode tests
    filter.type = DiscoveryFilterType::kCommissioningMode;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_CM");

    // Compressed fabric ID tests.
    filter.type = DiscoveryFilterType::kCompressedFabricId;
    filter.code = 0xABCD12341111BBBB;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_IABCD12341111BBBB");

    // None tests.
    filter.type = DiscoveryFilterType::kNone;
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "");

    // instance name - "1234567890123456._matterc"
    filter.type         = DiscoveryFilterType::kInstanceName;
    filter.instanceName = (char *) "1234567890123456";
    EXPECT_EQ(MakeServiceSubtype(buffer, sizeof(buffer), filter), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "1234567890123456");
}

TEST(TestServiceNaming, TestMakeServiceTypeName)
{
    // TODO(cecille): These need to be changed to remove leading zeros
    constexpr size_t kSize = 128;
    char buffer[kSize];
    DiscoveryFilter filter;

    // Long tests
    filter.type = DiscoveryFilterType::kLongDiscriminator;
    filter.code = 3;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_L3._sub._matterc");

    filter.code = (1 << 12) - 1;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_L4095._sub._matterc");

    filter.code = 1 << 12;
    EXPECT_NE(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);

    // Short tests
    filter.type = DiscoveryFilterType::kShortDiscriminator;
    filter.code = 3;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_S3._sub._matterc");

    filter.code = (1 << 4) - 1;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_S15._sub._matterc");

    filter.code = 1 << 4;
    EXPECT_NE(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);

    // Vendor tests
    filter.type = DiscoveryFilterType::kVendorId;
    filter.code = 3;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_V3._sub._matterc");

    filter.code = (1 << 16) - 1;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_V65535._sub._matterc");

    filter.code = 1 << 16;
    EXPECT_NE(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);

    // Device Type tests
    filter.type = DiscoveryFilterType::kDeviceType;
    filter.code = 3;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_T3._sub._matterc");

    // Commissioning mode tests
    filter.type = DiscoveryFilterType::kCommissioningMode;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_CM._sub._matterc");

    // Compressed fabric ID tests
    filter.type = DiscoveryFilterType::kCompressedFabricId;
    filter.code = 0x1234ABCD0000AAAA;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kOperational), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_I1234ABCD0000AAAA._sub._matter");

    // None tests
    filter.type = DiscoveryFilterType::kNone;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_matterc");

    filter.type = DiscoveryFilterType::kNone;
    EXPECT_EQ(MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionerNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_matterd");

    // Test buffer just under the right size - "_matterc" = 8 + nullchar = 9
    filter.type = DiscoveryFilterType::kNone;
    EXPECT_EQ(MakeServiceTypeName(buffer, 8, filter, DiscoveryType::kCommissionableNode), CHIP_ERROR_NO_MEMORY);

    // Test buffer exactly the right size - "_matterc" = 8 + nullchar = 9
    filter.type = DiscoveryFilterType::kNone;
    EXPECT_EQ(MakeServiceTypeName(buffer, 9, filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_matterc");

    // Test buffer exactly the right size for subtype - "_CM._sub._matterc" = 17 + nullchar = 18
    filter.type = DiscoveryFilterType::kCommissioningMode;
    EXPECT_EQ(MakeServiceTypeName(buffer, 18, filter, DiscoveryType::kCommissionableNode), CHIP_NO_ERROR);
    EXPECT_STREQ(buffer, "_CM._sub._matterc");
}
} // namespace
