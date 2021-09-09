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
#include <bits/stdint-uintn.h>
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

void InitializeChip(nlTestSuite * suite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    err = chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
    err = Server::GetInstance().Init();
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
}

void CheckCommissionManagerBasicWindowOpenClose(nlTestSuite * suite, void *)
{
    CommissionManager & commissionMgr = Server::GetInstance().GetCommissionManager();
    CHIP_ERROR err                    = commissionMgr.OpenBasicCommissioningWindow(ResetFabrics::kNo, kNoCommissioningTimeout,
                                                                CommissioningWindowAdvertisement::kMdns);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsPairingWindowOpen());
    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsPairingWindowOpen());
}

void CheckCommissionManagerWindowTimeout(nlTestSuite * suite, void *)
{
    CommissionManager & commissionMgr  = Server::GetInstance().GetCommissionManager();
    constexpr uint16_t kTimeoutSeconds = 1;
    constexpr unsigned kUsPerSecond    = 1000 * 1000;
    constexpr unsigned kSleepPadding   = 1000 * 10;
    CHIP_ERROR err =
        commissionMgr.OpenBasicCommissioningWindow(ResetFabrics::kNo, kTimeoutSeconds, CommissioningWindowAdvertisement::kMdns);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsPairingWindowOpen());
    usleep(kTimeoutSeconds * kUsPerSecond + kSleepPadding);
    NL_TEST_ASSERT(suite, !commissionMgr.IsPairingWindowOpen());
    commissionMgr.CloseCommissioningWindow();
}

void CheckCommissionManagerEnhancedWindow(nlTestSuite * suite, void *)
{
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

const nlTest sTests[] = { NL_TEST_DEF("CheckCommissionManagerBasicWindowOpenClose", CheckCommissionManagerBasicWindowOpenClose),
                          NL_TEST_DEF("CheckCommissionManagerWindowTimeout", CheckCommissionManagerWindowTimeout),
                          NL_TEST_DEF("CheckCommissionManagerEnhancedWindow", CheckCommissionManagerEnhancedWindow),
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
    chip::DeviceLayer::PlatformMgr().Shutdown();

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommissionManager)
