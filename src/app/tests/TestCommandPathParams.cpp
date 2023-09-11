/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements unit tests for AttributePathParams
 *
 */

#include <app/CommandPathParams.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

namespace chip {
namespace app {
namespace TestCommandPathParams {
void TestSamePath(nlTestSuite * apSuite, void * apContext)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    NL_TEST_ASSERT(apSuite, commandPathParams1.IsSamePath(commandPathParams2));
}

void TestDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(6, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    NL_TEST_ASSERT(apSuite, !commandPathParams1.IsSamePath(commandPathParams2));
}

void TestDifferentGroupId(nlTestSuite * apSuite, void * apContext)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kGroupIdValid);
    CommandPathParams commandPathParams2(1, 6, 3, 4, CommandPathFlags::kGroupIdValid);
    NL_TEST_ASSERT(apSuite, !commandPathParams1.IsSamePath(commandPathParams2));
}

void TestDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 6, 4, CommandPathFlags::kEndpointIdValid);
    NL_TEST_ASSERT(apSuite, !commandPathParams1.IsSamePath(commandPathParams2));
}

void TestDifferentCommandId(nlTestSuite * apSuite, void * apContext)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 3, 6, CommandPathFlags::kEndpointIdValid);
    NL_TEST_ASSERT(apSuite, !commandPathParams1.IsSamePath(commandPathParams2));
}

void TestDifferentPathFlag(nlTestSuite * apSuite, void * apContext)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 3, 4, CommandPathFlags::kGroupIdValid);
    NL_TEST_ASSERT(apSuite, !commandPathParams1.IsSamePath(commandPathParams2));
}
} // namespace TestCommandPathParams
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestSamePath", chip::app::TestCommandPathParams::TestSamePath),
                          NL_TEST_DEF("TestDifferentEndpointId", chip::app::TestCommandPathParams::TestDifferentEndpointId),
                          NL_TEST_DEF("TestDifferentGroupId", chip::app::TestCommandPathParams::TestDifferentGroupId),
                          NL_TEST_DEF("TestDifferentClusterId", chip::app::TestCommandPathParams::TestDifferentClusterId),
                          NL_TEST_DEF("TestDifferentCommandId", chip::app::TestCommandPathParams::TestDifferentCommandId),
                          NL_TEST_DEF("TestDifferentPathFlag", chip::app::TestCommandPathParams::TestDifferentPathFlag),
                          NL_TEST_SENTINEL() };
}

int TestCommandPathParams()
{
    nlTestSuite theSuite = { "CommandPathParams", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommandPathParams)
