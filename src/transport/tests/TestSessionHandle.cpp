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

/**
 *    @file
 *      This file implements unit tests for the SecureSession implementation.
 */

#include <errno.h>
#include <nlunit-test.h>

#include <lib/core/CHIPCore.h>

#include <protocols/secure_channel/PASESession.h>
#include <transport/FabricTable.h>
#include <transport/SecureSession.h>
#include <transport/SessionHandle.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <stdarg.h>

using namespace chip;

void TestMatchSession(nlTestSuite * inSuite, void * inContext)
{
    SessionHandle session3(chip::kTestDeviceNodeId, 1, 1, 0);
    SessionHandle session4(chip::kTestDeviceNodeId, 1, 2, 0);
    NL_TEST_ASSERT(inSuite, !(session3 == session4));
    NL_TEST_ASSERT(inSuite, session3.MatchIncomingSession(session4));
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("MatchSession",    TestMatchSession),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SessionHandle",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

/**
 *  Main
 */
int TestSessionHandle()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSessionHandle)
