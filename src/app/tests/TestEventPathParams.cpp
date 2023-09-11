/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements unit tests for EventPathParams
 *
 */

#include <app/EventPathParams.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

namespace chip {
namespace app {
namespace TestEventPathParams {
void TestSamePath(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(2, 3, 4);
    NL_TEST_ASSERT(apSuite, eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(6, 3, 4);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(2, 6, 4);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentEventId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(2, 3, 6);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}
} // namespace TestEventPathParams
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestSamePath", chip::app::TestEventPathParams::TestSamePath),
                          NL_TEST_DEF("TestDifferentEndpointId", chip::app::TestEventPathParams::TestDifferentEndpointId),
                          NL_TEST_DEF("TestDifferentClusterId", chip::app::TestEventPathParams::TestDifferentClusterId),
                          NL_TEST_DEF("TestDifferentEventId", chip::app::TestEventPathParams::TestDifferentEventId),
                          NL_TEST_SENTINEL() };
}

int TestEventPathParams()
{
    nlTestSuite theSuite = { "EventPathParams", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestEventPathParams)
