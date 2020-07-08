/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file
 *    This is a unit test suite for <tt>chip::App::AttributeStorage</tt>.
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <datamodel/CHIPClusterServer.h>
#include <nlassert.h>
#include <nlunit-test.h>
#include <support/TestUtils.h>

namespace {

void TestClusterServer(nlTestSuite * inSuite, void * inContext)
{
    chip::DataModel::ClusterServer server(0, 0, 0, 0);
}

} // namespace

int TestClusterServer(void)
{
    /**
     *   Test Suite. It lists all the test functions.
     */
    static const nlTest sTests[] = { NL_TEST_DEF("TestClusterServer", TestClusterServer), NL_TEST_SENTINEL() };

    nlTestSuite theSuite = {
        "TestClusterServer", &sTests[0], NULL /* setup */, NULL /* teardown */
    };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL /* context */);

    return (nlTestRunnerStats(&theSuite));
}

static void __attribute__((constructor)) Ctor(void)
{
    nlABORT(chip::RegisterUnitTests(&TestClusterServer) == CHIP_NO_ERROR);
}
