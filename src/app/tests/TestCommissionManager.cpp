/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <lib/dnssd/Advertiser.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/tests/echo/common.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>
#include <protocols/secure_channel/PASESession.h>

#include <nlunit-test.h>

using chip::CommissioningWindowAdvertisement;
using chip::CommissioningWindowManager;
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

    static chip::DeviceLayer::TestOnlyCommissionableDataProvider commissionableDataProvider;
    chip::DeviceLayer::SetCommissionableDataProvider(&commissionableDataProvider);

    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    err = chip::Server::GetInstance().Init(initParams);

    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);

    Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
}

void ShutdownChipTest()
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    chip::DeviceLayer::PlatformMgr().Shutdown();

    auto & mdnsAdvertiser = chip::Dnssd::ServiceAdvertiser::Instance();
    mdnsAdvertiser.RemoveServices();
    mdnsAdvertiser.Shutdown();

    // Server shudown will be called in TearDownTask

    // TODO: At this point UDP endpoits still seem leaked and the sanitizer
    // builds will attempt a memory free. As a result, we keep Memory initialized
    // so that the global UDPManager can still be destructed without a coredump.
    //
    // This is likely either a missing shutdown or an actual UDP endpoint leak
    // which I have not been able to track down yet.
    //
    // chip::Platform::MemoryShutdown();
}

void CheckCommissioningWindowManagerBasicWindowOpenCloseTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    CHIP_ERROR err                             = commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                                CommissioningWindowAdvertisement::kDnssdOnly);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerFabricIndex().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerVendorId().IsNull());
    NL_TEST_ASSERT(suite, !chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
}

void CheckCommissioningWindowManagerBasicWindowOpenClose(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerBasicWindowOpenCloseTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissioningWindowManagerBasicWindowOpenCloseFromClusterTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr auto fabricIndex                 = static_cast<chip::FabricIndex>(1);
    constexpr auto vendorId                    = static_cast<chip::VendorId>(0xFFF3);
    CHIP_ERROR err                             = commissionMgr.OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
        commissionMgr.MaxCommissioningTimeout(), fabricIndex, vendorId);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kBasicWindowOpen);
    NL_TEST_ASSERT(suite, !commissionMgr.GetOpenerFabricIndex().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerFabricIndex().Value() == fabricIndex);
    NL_TEST_ASSERT(suite, !commissionMgr.GetOpenerVendorId().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerVendorId().Value() == vendorId);
    NL_TEST_ASSERT(suite, !chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerFabricIndex().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerVendorId().IsNull());
}

void CheckCommissioningWindowManagerBasicWindowOpenCloseFromCluster(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerBasicWindowOpenCloseFromClusterTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissioningWindowManagerWindowClosedTask(chip::System::Layer *, void * context)
{
    nlTestSuite * suite                        = static_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
}

void CheckCommissioningWindowManagerWindowTimeoutTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr auto kTimeoutSeconds             = chip::System::Clock::Seconds16(1);
    constexpr uint16_t kTimeoutMs              = 1000;
    constexpr unsigned kSleepPadding           = 100;
    commissionMgr.OverrideMinCommissioningTimeout(kTimeoutSeconds);
    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, CommissioningWindowAdvertisement::kDnssdOnly);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
    NL_TEST_ASSERT(suite, !chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs + kSleepPadding),
                                                CheckCommissioningWindowManagerWindowClosedTask, suite);
}

void CheckCommissioningWindowManagerWindowTimeout(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerWindowTimeoutTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void SimulateFailedSessionEstablishmentTask(chip::System::Layer *, void * context)
{
    nlTestSuite * suite                        = static_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
    commissionMgr.OnSessionEstablishmentStarted();
    commissionMgr.OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
}

void CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrorsTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr auto kTimeoutSeconds             = chip::System::Clock::Seconds16(1);
    constexpr uint16_t kTimeoutMs              = 1000;
    constexpr unsigned kSleepPadding           = 100;
    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, CommissioningWindowAdvertisement::kDnssdOnly);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
    NL_TEST_ASSERT(suite, !chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs + kSleepPadding),
                                                CheckCommissioningWindowManagerWindowClosedTask, suite);
    // Simulate a session establishment error during that window, such that the
    // delay for the error plust hte window size exceeds our "timeout + padding" above.
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs / 4 * 3),
                                                SimulateFailedSessionEstablishmentTask, suite);
}

void CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrors(nlTestSuite * suite, void *)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrorsTask,
                                                  reinterpret_cast<intptr_t>(suite));
    sleep(kTestTaskWaitSeconds);
}

void CheckCommissioningWindowManagerEnhancedWindowTask(intptr_t context)
{
    nlTestSuite * suite                        = reinterpret_cast<nlTestSuite *>(context);
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    uint16_t originDiscriminator;
    CHIP_ERROR err = chip::DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(originDiscriminator);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    uint16_t newDiscriminator = static_cast<uint16_t>(originDiscriminator + 1);
    chip::Spake2pVerifier verifier;
    constexpr uint32_t kIterations = chip::kSpake2p_Min_PBKDF_Iterations;
    uint8_t salt[chip::kSpake2p_Min_PBKDF_Salt_Length];
    chip::ByteSpan saltData(salt);

    constexpr auto fabricIndex = static_cast<chip::FabricIndex>(1);
    constexpr auto vendorId    = static_cast<chip::VendorId>(0xFFF3);
    err = commissionMgr.OpenEnhancedCommissioningWindow(commissionMgr.MaxCommissioningTimeout(), newDiscriminator, verifier,
                                                        kIterations, saltData, fabricIndex, vendorId);
    NL_TEST_ASSERT(suite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(suite, commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kEnhancedWindowOpen);
    NL_TEST_ASSERT(suite, !chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    NL_TEST_ASSERT(suite, !commissionMgr.GetOpenerFabricIndex().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerFabricIndex().Value() == fabricIndex);
    NL_TEST_ASSERT(suite, !commissionMgr.GetOpenerVendorId().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerVendorId().Value() == vendorId);

    commissionMgr.CloseCommissioningWindow();
    NL_TEST_ASSERT(suite, !commissionMgr.IsCommissioningWindowOpen());
    NL_TEST_ASSERT(suite,
                   commissionMgr.CommissioningWindowStatusForCluster() ==
                       chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen);
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerFabricIndex().IsNull());
    NL_TEST_ASSERT(suite, commissionMgr.GetOpenerVendorId().IsNull());
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
    NL_TEST_DEF("CheckCommissioningWindowManagerBasicWindowOpenCloseFromCluster",
                CheckCommissioningWindowManagerBasicWindowOpenCloseFromCluster),
    NL_TEST_DEF("CheckCommissioningWindowManagerWindowTimeout", CheckCommissioningWindowManagerWindowTimeout),
    NL_TEST_DEF("CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrors",
                CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrors),
    NL_TEST_SENTINEL(),
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
    ShutdownChipTest();

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommissioningWindowManager)
