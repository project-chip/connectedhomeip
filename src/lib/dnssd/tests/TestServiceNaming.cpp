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

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Dnssd;

namespace {

void TestMakeInstanceName(nlTestSuite * inSuite, void * inContext)
{
    char buffer[128];

    NL_TEST_ASSERT(inSuite,
                   MakeInstanceName(buffer, sizeof(buffer), PeerId().SetCompressedFabricId(0x1234).SetNodeId(0x5678)) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "0000000000001234-0000000000005678") == 0);

    NL_TEST_ASSERT(inSuite,
                   MakeInstanceName(buffer, sizeof(buffer),
                                    PeerId().SetCompressedFabricId(0x1122334455667788ULL).SetNodeId(0x123456789abcdefULL)) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "1122334455667788-0123456789ABCDEF") == 0);

    // insufficient buffer size:
    //  buffer needs at least space for hex encoding + separator + 0 terminator
    constexpr size_t kMinBufferSize = 2 * 16 + 1 + 1;
    for (size_t shortSize = 0; shortSize < kMinBufferSize; shortSize++)
    {
        NL_TEST_ASSERT(inSuite, MakeInstanceName(buffer, shortSize, PeerId()) != CHIP_NO_ERROR);
    }
    NL_TEST_ASSERT(inSuite, MakeInstanceName(buffer, kMinBufferSize, PeerId()) == CHIP_NO_ERROR);
}

void TestExtractIdFromInstanceName(nlTestSuite * inSuite, void * inContext)
{
    PeerId peerId;

    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName(nullptr, nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("ACDEF1234567890-1234567890ABCDEF", nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName(nullptr, &peerId) == CHIP_ERROR_INVALID_ARGUMENT);

    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("ABCDEF1234567890-1234567890ABCDEF", &peerId) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, peerId == PeerId().SetCompressedFabricId(0xABCDEF1234567890ULL).SetNodeId(0x1234567890ABCDEFULL));

    // ending in period (partial name) is acceptable
    NL_TEST_ASSERT(inSuite,
                   ExtractIdFromInstanceName("1122334455667788-AABBCCDDEEFF1122.some.suffix.here", &peerId) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, peerId == PeerId().SetCompressedFabricId(0x1122334455667788ULL).SetNodeId(0xaabbccddeeff1122ULL));

    // Invalid: non hex character
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1x22334455667788-AABBCCDDEEDD1122", &peerId) != CHIP_NO_ERROR);

    // Invalid: missing node id part (no - separator)
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122334455667788x2233445566778899", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122334455667788x2233445566778899.12-33.4455", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122334455667788x2233445566778899.4455", &peerId) != CHIP_NO_ERROR);

    // Invalid: missing part
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("-1234567890ABCDEF", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1234567890ABCDEF-", &peerId) != CHIP_NO_ERROR);

    // Invalid: separator in wrong place
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("112233445566778-8AABBCCDDEEFF1122", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122334455667788A-ABBCCDDEEFF1122", &peerId) != CHIP_NO_ERROR);

    // Invalid: fabric part too short
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("11223344556677-AABBCCDDEEFF1122", &peerId) != CHIP_NO_ERROR);
    // Invalid: fabric part too long
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("112233445566778899-AABBCCDDEEFF1122", &peerId) != CHIP_NO_ERROR);

    // Invalid: node part too short
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122334455667788-AABBCCDDEEFF11", &peerId) != CHIP_NO_ERROR);
    // Invalid: node part too long
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122334455667788-AABBCCDDEEFF112233", &peerId) != CHIP_NO_ERROR);
}

void TestMakeServiceNameSubtype(nlTestSuite * inSuite, void * inContext)
{
    constexpr size_t kSize = 19;
    char buffer[kSize];
    DiscoveryFilter filter;

    // Long tests
    filter.type = DiscoveryFilterType::kLongDiscriminator;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_L3") == 0);

    filter.code = (1 << 12) - 1;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_L4095") == 0);

    filter.code = 1 << 12;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) != CHIP_NO_ERROR);

    // Short tests
    filter.type = DiscoveryFilterType::kShortDiscriminator;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_S3") == 0);

    filter.code = (1 << 4) - 1;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_S15") == 0);

    filter.code = 1 << 4;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) != CHIP_NO_ERROR);

    // Vendor tests
    filter.type = DiscoveryFilterType::kVendorId;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_V3") == 0);
    filter.code = 0xFFFF;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_V65535") == 0);
    filter.code = 1 << 16;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) != CHIP_NO_ERROR);

    // Device Type tests
    filter.type = DiscoveryFilterType::kDeviceType;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_T3") == 0);
    // TODO: Add tests for longer device types once spec issue #3226 is closed.

    // Commissioning mode tests
    filter.type = DiscoveryFilterType::kCommissioningMode;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_CM") == 0);

    // Compressed fabric ID tests.
    filter.type = DiscoveryFilterType::kCompressedFabricId;
    filter.code = 0xABCD12341111BBBB;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_IABCD12341111BBBB") == 0);

    // None tests.
    filter.type = DiscoveryFilterType::kNone;
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "") == 0);

    // instance name - "1234567890123456._matterc"
    filter.type         = DiscoveryFilterType::kInstanceName;
    filter.instanceName = (char *) "1234567890123456";
    NL_TEST_ASSERT(inSuite, MakeServiceSubtype(buffer, sizeof(buffer), filter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "1234567890123456") == 0);
}

void TestMakeServiceTypeName(nlTestSuite * inSuite, void * inContext)
{
    // TODO(cecille): These need to be changed to remove leading zeros
    constexpr size_t kSize = 128;
    char buffer[kSize];
    DiscoveryFilter filter;

    // Long tests
    filter.type = DiscoveryFilterType::kLongDiscriminator;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_L3._sub._matterc") == 0);

    filter.code = (1 << 12) - 1;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_L4095._sub._matterc") == 0);

    filter.code = 1 << 12;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) != CHIP_NO_ERROR);

    // Short tests
    filter.type = DiscoveryFilterType::kShortDiscriminator;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_S3._sub._matterc") == 0);

    filter.code = (1 << 4) - 1;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_S15._sub._matterc") == 0);

    filter.code = 1 << 4;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) != CHIP_NO_ERROR);

    // Vendor tests
    filter.type = DiscoveryFilterType::kVendorId;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_V3._sub._matterc") == 0);

    filter.code = (1 << 16) - 1;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_V65535._sub._matterc") == 0);

    filter.code = 1 << 16;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) != CHIP_NO_ERROR);

    // Device Type tests
    filter.type = DiscoveryFilterType::kDeviceType;
    filter.code = 3;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_T3._sub._matterc") == 0);

    // Commissioning mode tests
    filter.type = DiscoveryFilterType::kCommissioningMode;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_CM._sub._matterc") == 0);

    // Compressed fabric ID tests
    filter.type = DiscoveryFilterType::kCompressedFabricId;
    filter.code = 0x1234ABCD0000AAAA;
    NL_TEST_ASSERT(inSuite, MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kOperational) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_I1234ABCD0000AAAA._sub._matter") == 0);

    // None tests
    filter.type = DiscoveryFilterType::kNone;
    NL_TEST_ASSERT(inSuite,
                   MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_matterc") == 0);

    filter.type = DiscoveryFilterType::kNone;
    NL_TEST_ASSERT(inSuite, MakeServiceTypeName(buffer, sizeof(buffer), filter, DiscoveryType::kCommissionerNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_matterd") == 0);

    // Test buffer just under the right size - "_matterc" = 8 + nullchar = 9
    filter.type = DiscoveryFilterType::kNone;
    NL_TEST_ASSERT(inSuite, MakeServiceTypeName(buffer, 8, filter, DiscoveryType::kCommissionableNode) == CHIP_ERROR_NO_MEMORY);

    // Test buffer exactly the right size - "_matterc" = 8 + nullchar = 9
    filter.type = DiscoveryFilterType::kNone;
    NL_TEST_ASSERT(inSuite, MakeServiceTypeName(buffer, 9, filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_matterc") == 0);

    // Test buffer exactly the right size for subtype - "_CM._sub._matterc" = 17 + nullchar = 18
    filter.type = DiscoveryFilterType::kCommissioningMode;
    NL_TEST_ASSERT(inSuite, MakeServiceTypeName(buffer, 18, filter, DiscoveryType::kCommissionableNode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "_CM._sub._matterc") == 0);
}

const nlTest sTests[] = {
    NL_TEST_DEF("MakeInstanceName", TestMakeInstanceName),                             //
    NL_TEST_DEF("ExtractIdFromInstandceName", TestExtractIdFromInstanceName),          //
    NL_TEST_DEF("TestMakeServiceNameSubtype", TestMakeServiceNameSubtype),             //
    NL_TEST_DEF("TestMakeCommisisonableNodeServiceTypeName", TestMakeServiceTypeName), //
    NL_TEST_SENTINEL()                                                                 //
};

} // namespace

int TestCHIPServiceNaming()
{
    nlTestSuite theSuite = { "ServiceNaming", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPServiceNaming)
