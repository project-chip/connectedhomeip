/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements a test for  CHIP core library reference counted object.
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <lib/core/DataModelTypes.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

static void TestMEI(nlTestSuite * inSuite, void * inContext)
{
    {
        ClusterId clusterId = 0x1234'5678;
        NL_TEST_ASSERT(inSuite, MEI::GetPrefix(clusterId) == 0x1234);
        NL_TEST_ASSERT(inSuite, MEI::GetSuffix(clusterId) == 0x5678);
    }
}
/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestMEI", TestMEI),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestMEI_Setup(void * inContext)
{
    return SUCCESS;
}

int TestMEI_Teardown(void * inContext)
{
    return SUCCESS;
}

int TestMEI(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "MEI",
        &sTests[0],
        TestMEI_Setup,
        TestMEI_Teardown
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestMEI)
