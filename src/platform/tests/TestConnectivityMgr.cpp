/*
 *
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

/**
 *    @file
 *      This file implements a unit test suite for the Connectivity Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

struct TestConnectivityMgr : public ::testing::Test
{

    static void SetUpTestSuite()
    {
        auto err = chip::Platform::MemoryInit();
        EXPECT_EQ(err, CHIP_NO_ERROR);
        // TODO: Move initialization of the platform manager from Init test to here
    }

    static void TearDownTestSuite()
    {
        chip::Platform::MemoryShutdown();
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }
};

TEST_F(TestConnectivityMgr, Init)
{
    // ConfigurationManager is initialized from PlatformManager indirectly
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestConnectivityMgr, GetNetworkInterfaces)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NetworkInterface * netifs = nullptr;

    err = GetDiagnosticDataProvider().GetNetworkInterfaces(&netifs);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(netifs, nullptr);

    GetDiagnosticDataProvider().ReleaseNetworkInterfaces(netifs);
}
