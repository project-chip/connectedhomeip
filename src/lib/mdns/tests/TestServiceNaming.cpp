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

#include <mdns/ServiceNaming.h>

#include <string.h>

#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Mdns;

namespace {

void TestMakeInstanceName(nlTestSuite * inSuite, void * inContext)
{
    char buffer[128];

    NL_TEST_ASSERT(inSuite,
                   MakeInstanceName(buffer, sizeof(buffer), PeerId().SetFabricId(0x1234).SetNodeId(0x5678)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(buffer, "0000000000001234-0000000000005678") == 0);

    NL_TEST_ASSERT(inSuite,
                   MakeInstanceName(buffer, sizeof(buffer),
                                    PeerId().SetFabricId(0x1122334455667788ULL).SetNodeId(0x123456789abcdefULL)) == CHIP_NO_ERROR);
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
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("01234-5678", nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName(nullptr, &peerId) == CHIP_ERROR_INVALID_ARGUMENT);

    // Short format is acceptable
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("ABCD-1234", &peerId) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, peerId == PeerId().SetFabricId(0xABCD).SetNodeId(0x1234));

    // ending in period (partial name) is acceptable
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1122-aabb.some.suffix.here", &peerId) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, peerId == PeerId().SetFabricId(0x1122).SetNodeId(0xaabb));

    // Invalid: non hex character
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1x22-aabc", &peerId) != CHIP_NO_ERROR);

    // Invalid: missing node id part (no - separator)
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("11x22", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("11x22.12-33.4455", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("11x22.4455", &peerId) != CHIP_NO_ERROR);

    // Invalid: missing part
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("-1234", &peerId) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ExtractIdFromInstanceName("1234-", &peerId) != CHIP_NO_ERROR);
}

const nlTest sTests[] = {
    NL_TEST_DEF("MakeInstanceName", TestMakeInstanceName),                    //
    NL_TEST_DEF("ExtractIdFromInstandceName", TestExtractIdFromInstanceName), //
    NL_TEST_SENTINEL()                                                        //
};

} // namespace

int TestCHIPServiceNaming(void)
{
    nlTestSuite theSuite = { "ServiceNaming", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPServiceNaming)
