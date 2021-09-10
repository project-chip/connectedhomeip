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

#include <app/server/CommissionManager.h>
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
using chip::CommissionManager;
using chip::kNoCommissioningTimeout;
using chip::ResetFabrics;
using chip::Server;

// Mock function for linking
void InitDataModelHandler(chip::Messaging::ExchangeManager * exchangeMgr) {}
void HandleDataModelMessage(chip::Messaging::ExchangeContext * exchange, chip::System::PacketBufferHandle && buffer) {}

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

void CheckCommissionManagerBasicWindowOpenCloseTask(intptr_t context)
{
    nlTestSuite * suite               = reinterpret_cast<nlTestSuite *>(context);
    CommissionManager & commissionMgr = Server::GetInstance().GetCommissionManager();
    CHIP_ERROR err                    = commissionMgr.OpenBasicCommissioningWindow(ResetFabrics::kNo, kNoCommissioningTimeout,
                                                                CommissioningWindowAdvertisement::kMdns);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsPairingWindowOpen());
    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsPairingWindowOpen());
}

void CheckCommissionManagerBasicWindowOpenClose(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissionManagerBasicWindowOpenCloseTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissionManagerWindowClosedTask(chip::System::Layer *, void * context)
{
    nlTestSuite * suite               = static_cast<nlTestSuite *>(context);
    CommissionManager & commissionMgr = Server::GetInstance().GetCommissionManager();
    NL_TEST_ASSERT(suite, !commissionMgr.IsPairingWindowOpen());
}

void CheckCommissionManagerWindowTimeoutTask(intptr_t context)
{
    nlTestSuite * suite                = reinterpret_cast<nlTestSuite *>(context);
    CommissionManager & commissionMgr  = Server::GetInstance().GetCommissionManager();
    constexpr uint16_t kTimeoutSeconds = 1;
    constexpr uint16_t kTimeoutMs      = 1000;
    constexpr unsigned kSleepPadding   = 100;
    CHIP_ERROR err =
        commissionMgr.OpenBasicCommissioningWindow(ResetFabrics::kNo, kTimeoutSeconds, CommissioningWindowAdvertisement::kMdns);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsPairingWindowOpen());
    chip::DeviceLayer::SystemLayer().StartTimer(kTimeoutMs + kSleepPadding, CheckCommissionManagerWindowClosedTask, suite);
}

void CheckCommissionManagerWindowTimeout(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissionManagerWindowTimeoutTask, reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissionManagerEnhancedWindowTask(intptr_t context)
{
    nlTestSuite * suite               = reinterpret_cast<nlTestSuite *>(context);
    CommissionManager & commissionMgr = Server::GetInstance().GetCommissionManager();
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
    NL_TEST_ASSERT(suite, commissionMgr.IsPairingWindowOpen());
    err = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(currentDiscriminator);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, currentDiscriminator == newDiscriminator);

    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsPairingWindowOpen());
    err = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(currentDiscriminator);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, currentDiscriminator == originDiscriminator);
}

void CheckCommissionManagerEnhancedWindow(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissionManagerEnhancedWindowTask, reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void TearDownTask(intptr_t context)
{
    chip::Server::GetInstance().Shutdown();
    chip::DeviceLayer::PlatformMgr().Shutdown();
}

const nlTest sTests[] = { NL_TEST_DEF("CheckCommissionManagerEnhancedWindow", CheckCommissionManagerEnhancedWindow),
                          NL_TEST_DEF("CheckCommissionManagerBasicWindowOpenClose", CheckCommissionManagerBasicWindowOpenClose),
                          NL_TEST_DEF("CheckCommissionManagerWindowTimeout", CheckCommissionManagerWindowTimeout),
                          NL_TEST_SENTINEL() };

} // namespace

int TestCommissionManager()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "CommissionManager",
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

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommissionManager)
