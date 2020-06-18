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

/**
 *    @file
 *      This file implements a process to effect a functional test for
 *      the Security Header class within the transport layer
 *
 */
#include "TestTransportLayer.h"

#include <support/ErrorStr.h>
#include <transport/SecurityHeader.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestSerializeDeserialize(nlTestSuite * inSuite, void * inContext)
{
    SecurityHeader header;

    header.SetIV(1234).SetTag(4321).SetSessionID(6789).SetType(9876);
    NL_TEST_ASSERT(inSuite, header.IV() == 1234);
    NL_TEST_ASSERT(inSuite, header.Tag() == 4321);
    NL_TEST_ASSERT(inSuite, header.SessionID() == 6789);
    NL_TEST_ASSERT(inSuite, header.Type() == 9876);

    unsigned char serialized[128];
    size_t header_len = header.RawHeaderLen() - 1;
    NL_TEST_ASSERT(inSuite, header.Serialize(serialized, header_len) != CHIP_NO_ERROR);

    header_len = header.RawHeaderLen();
    NL_TEST_ASSERT(inSuite, header.Serialize(serialized, header_len) == CHIP_NO_ERROR);

    header_len = sizeof(serialized);
    NL_TEST_ASSERT(inSuite, header.Serialize(serialized, header_len) == CHIP_NO_ERROR);

    SecurityHeader header2;
    header_len = header.RawHeaderLen() - 1;
    NL_TEST_ASSERT(inSuite, header2.Deserialize(serialized, header_len) != CHIP_NO_ERROR);

    header_len = header.RawHeaderLen();
    NL_TEST_ASSERT(inSuite, header2.Deserialize(serialized, header_len) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(header2.RawHeader(), header.RawHeader(), header.RawHeaderLen()) == 0);

    header_len = sizeof(serialized);
    NL_TEST_ASSERT(inSuite, header2.Deserialize(serialized, header_len) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(header2.RawHeader(), header.RawHeader(), header.RawHeaderLen()) == 0);

    NL_TEST_ASSERT(inSuite, header2.IV() == 1234);
    NL_TEST_ASSERT(inSuite, header2.Tag() == 4321);
    NL_TEST_ASSERT(inSuite, header2.SessionID() == 6789);
    NL_TEST_ASSERT(inSuite, header2.Type() == 9876);
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("SerializeDeserialize", TestSerializeDeserialize),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestSecurityHeader(void)
{
    nlTestSuite theSuite = { "Transport-MessageHeader", &sTests[0], NULL, NULL };
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
