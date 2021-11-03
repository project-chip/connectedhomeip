/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <system/SystemConfig.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>
#include <system/SystemLayerImpl.h>

#include <errno.h>
#include <stdint.h>
#include <string.h>

using chip::ErrorStr;
using namespace chip::System;

static void ServiceEvents(Layer & aLayer)
{
    chip::DeviceLayer::PlatformMgr().ProcessDeviceEvents();
}

class TestContext
{
public:
    Layer * mLayer;
};

// Test input data.

static void CheckScheduleLambda(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    Layer & lSys           = *lContext.mLayer;
    bool * called          = new bool(false);
    lSys.ScheduleLambda([called] { *called = true; });
    while (!*called)
    {
        ServiceEvents(lSys);
    }
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("System::TestScheduleLambda", CheckScheduleLambda),
    NL_TEST_SENTINEL()
};
// clang-format on

static int TestSetup(void * aContext);
static int TestTeardown(void * aContext);

// clang-format off
static nlTestSuite kTheSuite =
{
    "chip-system-schedule-lambda",
    &sTests[0],
    TestSetup,
    TestTeardown
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    static LayerImpl sLayer;
    chip::DeviceLayer::SetSystemLayerForTesting(&sLayer);
    sLayer.Init();

    lContext.mLayer = &sLayer;

    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 *  Free memory reserved at TestSetup.
 */
static int TestTeardown(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);
    lContext.mLayer->Shutdown();
    return (SUCCESS);
}

int TestSystemScheduleLambda(void)
{
    TestContext context;

    // Run test suit againt one lContext.
    nlTestRunner(&kTheSuite, &context);

    return nlTestRunnerStats(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemScheduleLambda)
