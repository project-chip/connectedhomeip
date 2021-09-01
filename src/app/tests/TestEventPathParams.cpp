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
    EventPathParams eventPathParams1(1, 2, 3, 4, false);
    EventPathParams eventPathParams2(1, 2, 3, 4, false);
    NL_TEST_ASSERT(apSuite, eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentNodeId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(1, 2, 3, 4, false);
    EventPathParams eventPathParams2(6, 2, 3, 4, false);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(1, 2, 3, 4, false);
    EventPathParams eventPathParams2(1, 6, 3, 4, false);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(1, 2, 3, 4, false);
    EventPathParams eventPathParams2(1, 2, 6, 4, false);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}

void TestDifferentEventId(nlTestSuite * apSuite, void * apContext)
{
    EventPathParams eventPathParams1(1, 2, 3, 4, false);
    EventPathParams eventPathParams2(1, 2, 3, 6, false);
    NL_TEST_ASSERT(apSuite, !eventPathParams1.IsSamePath(eventPathParams2));
}
} // namespace TestEventPathParams
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestSamePath", chip::app::TestEventPathParams::TestSamePath),
                          NL_TEST_DEF("TestDifferentNodeId", chip::app::TestEventPathParams::TestDifferentNodeId),
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
