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

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/tests/echo/common.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>
#include <protocols/secure_channel/PASESession.h>

#include <nlunit-test.h>

using chip::CommissioningWindowAdvertisement;
using chip::CommissioningWindowManager;
using chip::kNoCommissioningTimeout;
using chip::Server;

// Mock function for linking
void InitDataModelHandler(chip::Messaging::ExchangeManager * exchangeMgr) {}

namespace {

static constexpr int kTestTaskWaitSeconds = 2;

void InitializeChip(nlTestSuite * suite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    err = Server::GetInstance().Init();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
}

void CheckCommissioningWindowManagerBasicWindowOpenCloseTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    CHIP_ERROR err =
        commissionMgr.OpenBasicCommissioningWindow(kNoCommissioningTimeout, CommissioningWindowAdvertisement::kDnssdOnly);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
}

void CheckCommissioningWindowManagerBasicWindowOpenClose(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerBasicWindowOpenCloseTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissioningWindowManagerWindowClosedTask(chip::System::Layer *, void * context)
{
    nlTestSuite * suite                        = static_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
}

void CheckCommissioningWindowManagerWindowTimeoutTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr uint16_t kTimeoutSeconds         = 1;
    constexpr uint16_t kTimeoutMs              = 1000;
    constexpr unsigned kSleepPadding           = 100;
    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, CommissioningWindowAdvertisement::kDnssdOnly);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs + kSleepPadding),
                                                CheckCommissioningWindowManagerWindowClosedTask, suite);
}

void CheckCommissioningWindowManagerWindowTimeout(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerWindowTimeoutTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissioningWindowManagerEnhancedWindowTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    uint16_t originDiscriminator;
    CHIP_ERROR err = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(originDiscriminator);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    uint16_t newDiscriminator = static_cast<uint16_t>(originDiscriminator + 1);
    chip::PASEVerifier verifier;
    constexpr uint32_t kIterations = chip::kPBKDFMinimumIterations;
    uint8_t salt[chip::kPBKDFMinimumSaltLen];
    chip::ByteSpan saltData(salt);
    constexpr uint16_t kPasscodeID = 1;
    uint16_t currentDiscriminator;

    err = commissionMgr.OpenEnhancedCommissioningWindow(kNoCommissioningTimeout, newDiscriminator, verifier, kIterations, saltData,
                                                        kPasscodeID);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    err = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(currentDiscriminator);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, currentDiscriminator == newDiscriminator);

    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
    err = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(currentDiscriminator);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, currentDiscriminator == originDiscriminator);
}

void CheckCommissioningWindowManagerEnhancedWindow(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerEnhancedWindowTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void TearDownTask(intptr_t context)
{
    chip::Server::GetInstance().Shutdown();
}

const nlTest sTests[] = {
    NL_TEST_DEF("CheckCommissioningWindowManagerEnhancedWindow", CheckCommissioningWindowManagerEnhancedWindow),
    NL_TEST_DEF("CheckCommissioningWindowManagerBasicWindowOpenClose", CheckCommissioningWindowManagerBasicWindowOpenClose),
    NL_TEST_DEF("CheckCommissioningWindowManagerWindowTimeout", CheckCommissioningWindowManagerWindowTimeout), NL_TEST_SENTINEL()
};

} // namespace

int TestCommissioningWindowManager()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "CommissioningWindowManager",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);
    nlTestRunner(&theSuite, nullptr);

    // TODO: The platform memory was intentionally left not deinitialized so that minimal mdns can destruct
    chip::DeviceLayer::PlatformMgr().ScheduleWork(TearDownTask, 0);
    sleep(kTestTaskWaitSeconds);
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    chip::DeviceLayer::PlatformMgr().Shutdown();

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommissioningWindowManager)
