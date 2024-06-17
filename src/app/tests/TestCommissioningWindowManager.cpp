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

#include <app/TestEventTriggerDelegate.h>
#include <app/TimerDelegates.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <crypto/RandUtils.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/Span.h>
#include <messaging/tests/echo/common.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>
#include <protocols/secure_channel/PASESession.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip::Crypto;

using chip::CommissioningWindowAdvertisement;
using chip::CommissioningWindowManager;
using chip::Server;

// Mock function for linking
void InitDataModelHandler() {}

namespace {
bool sAdminFabricIndexDirty = false;
bool sAdminVendorIdDirty    = false;
bool sWindowStatusDirty     = false;

void ResetDirtyFlags()
{
    sAdminFabricIndexDirty = false;
    sAdminVendorIdDirty    = false;
    sWindowStatusDirty     = false;
}

} // namespace

void MatterReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::AdministratorCommissioning::Attributes;
    if (endpoint != chip::kRootEndpointId || clusterId != AdministratorCommissioning::Id)
    {
        return;
    }

    switch (attributeId)
    {
    case WindowStatus::Id:
        sWindowStatusDirty = true;
        break;
    case AdminVendorId::Id:
        sAdminVendorIdDirty = true;
        break;
    case AdminFabricIndex::Id:
        sAdminFabricIndexDirty = true;
        break;
    default:
        break;
    }
}

namespace {

void TearDownTask(intptr_t context)
{
    chip::Server::GetInstance().Shutdown();
}

static void StopEventLoop(intptr_t context)
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

class TestCommissioningWindowManager : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

        static chip::DeviceLayer::TestOnlyCommissionableDataProvider commissionableDataProvider;
        chip::DeviceLayer::SetCommissionableDataProvider(&commissionableDataProvider);

        static chip::CommonCaseDeviceServerInitParams initParams;
        // Report scheduler and timer delegate instance
        static chip::app::DefaultTimerDelegate sTimerDelegate;
        static chip::app::reporting::ReportSchedulerImpl sReportScheduler(&sTimerDelegate);
        initParams.reportScheduler = &sReportScheduler;
        static chip::SimpleTestEventTriggerDelegate sSimpleTestEventTriggerDelegate;
        initParams.testEventTriggerDelegate = &sSimpleTestEventTriggerDelegate;
        (void) initParams.InitializeStaticResourcesBeforeServerInit();
        // Set a randomized server port(slightly shifted from CHIP_PORT) for testing
        initParams.operationalServicePort =
            static_cast<uint16_t>(initParams.operationalServicePort + chip::Crypto::GetRandU16() % 20);

        ASSERT_EQ(chip::Server::GetInstance().Init(initParams), CHIP_NO_ERROR);

        Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    }
    static void TearDownTestSuite()
    {

        // TODO: The platform memory was intentionally left not deinitialized so that minimal mdns can destruct
        chip::DeviceLayer::PlatformMgr().ScheduleWork(TearDownTask, 0);
        chip::DeviceLayer::PlatformMgr().ScheduleWork(StopEventLoop);
        chip::DeviceLayer::PlatformMgr().RunEventLoop();

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
};

void CheckCommissioningWindowManagerBasicWindowOpenCloseTask(intptr_t context)
{
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();

    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                         CommissioningWindowAdvertisement::kDnssdOnly),
              CHIP_NO_ERROR);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    EXPECT_TRUE(commissionMgr.GetOpenerFabricIndex().IsNull());
    EXPECT_TRUE(commissionMgr.GetOpenerVendorId().IsNull());
    EXPECT_FALSE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    commissionMgr.CloseCommissioningWindow();
    EXPECT_FALSE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);
}

TEST_F(TestCommissioningWindowManager, TestCheckCommissioningWindowManagerBasicWindowOpenClose)
{

    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerBasicWindowOpenCloseTask);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(StopEventLoop);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

void CheckCommissioningWindowManagerBasicWindowOpenCloseFromClusterTask(intptr_t context)
{
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr auto fabricIndex                 = static_cast<chip::FabricIndex>(1);
    constexpr auto vendorId                    = static_cast<chip::VendorId>(0xFFF3);
    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
                  commissionMgr.MaxCommissioningTimeout(), fabricIndex, vendorId),
              CHIP_NO_ERROR);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kBasicWindowOpen);
    EXPECT_FALSE(commissionMgr.GetOpenerFabricIndex().IsNull());
    EXPECT_EQ(commissionMgr.GetOpenerFabricIndex().Value(), fabricIndex);
    EXPECT_FALSE(commissionMgr.GetOpenerVendorId().IsNull());
    EXPECT_EQ(commissionMgr.GetOpenerVendorId().Value(), vendorId);
    EXPECT_FALSE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    EXPECT_TRUE(sWindowStatusDirty);
    EXPECT_TRUE(sAdminFabricIndexDirty);
    EXPECT_TRUE(sAdminVendorIdDirty);

    ResetDirtyFlags();
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    commissionMgr.CloseCommissioningWindow();
    EXPECT_FALSE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_TRUE(commissionMgr.GetOpenerFabricIndex().IsNull());
    EXPECT_TRUE(commissionMgr.GetOpenerVendorId().IsNull());
    EXPECT_TRUE(sWindowStatusDirty);
    EXPECT_TRUE(sAdminFabricIndexDirty);
    EXPECT_TRUE(sAdminVendorIdDirty);

    ResetDirtyFlags();
}

TEST_F(TestCommissioningWindowManager, TestCheckCommissioningWindowManagerBasicWindowOpenCloseFromCluster)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerBasicWindowOpenCloseFromClusterTask);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(StopEventLoop);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

void CheckCommissioningWindowManagerWindowClosedTask(chip::System::Layer *, void *)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    EXPECT_FALSE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);
}

void CheckCommissioningWindowManagerWindowTimeoutTask(intptr_t context)
{

    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr auto kTimeoutSeconds             = chip::System::Clock::Seconds32(1);
    constexpr uint16_t kTimeoutMs              = 1000;
    constexpr unsigned kSleepPadding           = 100;
    commissionMgr.OverrideMinCommissioningTimeout(kTimeoutSeconds);
    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, CommissioningWindowAdvertisement::kDnssdOnly),
              CHIP_NO_ERROR);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    EXPECT_FALSE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs + kSleepPadding),
                                                CheckCommissioningWindowManagerWindowClosedTask, nullptr);
}

TEST_F(TestCommissioningWindowManager, TestCheckCommissioningWindowManagerWindowTimeout)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerWindowTimeoutTask);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(StopEventLoop);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

void SimulateFailedSessionEstablishmentTask(chip::System::Layer *, void *)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    commissionMgr.OnSessionEstablishmentStarted();
    commissionMgr.OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);
}

void CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrorsTask(intptr_t context)
{
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    constexpr auto kTimeoutSeconds             = chip::System::Clock::Seconds16(1);
    constexpr uint16_t kTimeoutMs              = 1000;
    constexpr unsigned kSleepPadding           = 100;

    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, CommissioningWindowAdvertisement::kDnssdOnly),
              CHIP_NO_ERROR);

    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);

    EXPECT_FALSE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs + kSleepPadding),
                                                CheckCommissioningWindowManagerWindowClosedTask, nullptr);
    // Simulate a session establishment error during that window, such that the
    // delay for the error plus the window size exceeds our "timeout + padding" above.
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(kTimeoutMs / 4 * 3),
                                                SimulateFailedSessionEstablishmentTask, nullptr);
}

TEST_F(TestCommissioningWindowManager, CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrors)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerWindowTimeoutWithSessionEstablishmentErrorsTask);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(StopEventLoop);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

void CheckCommissioningWindowManagerEnhancedWindowTask(intptr_t context)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    uint16_t originDiscriminator;
    EXPECT_EQ(chip::DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(originDiscriminator), CHIP_NO_ERROR);
    uint16_t newDiscriminator = static_cast<uint16_t>(originDiscriminator + 1);
    Spake2pVerifier verifier;
    constexpr uint32_t kIterations = kSpake2p_Min_PBKDF_Iterations;
    uint8_t salt[kSpake2p_Min_PBKDF_Salt_Length];
    chip::ByteSpan saltData(salt);

    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    constexpr auto fabricIndex = static_cast<chip::FabricIndex>(1);
    constexpr auto vendorId    = static_cast<chip::VendorId>(0xFFF3);
    EXPECT_EQ(commissionMgr.OpenEnhancedCommissioningWindow(commissionMgr.MaxCommissioningTimeout(), newDiscriminator, verifier,
                                                            kIterations, saltData, fabricIndex, vendorId),
              CHIP_NO_ERROR);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kEnhancedWindowOpen);
    EXPECT_FALSE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    EXPECT_FALSE(commissionMgr.GetOpenerFabricIndex().IsNull());
    EXPECT_EQ(commissionMgr.GetOpenerFabricIndex().Value(), fabricIndex);
    EXPECT_FALSE(commissionMgr.GetOpenerVendorId().IsNull());
    EXPECT_EQ(commissionMgr.GetOpenerVendorId().Value(), vendorId);
    EXPECT_TRUE(sWindowStatusDirty);
    EXPECT_TRUE(sAdminFabricIndexDirty);
    EXPECT_TRUE(sAdminVendorIdDirty);

    ResetDirtyFlags();
    EXPECT_FALSE(sWindowStatusDirty);
    EXPECT_FALSE(sAdminFabricIndexDirty);
    EXPECT_FALSE(sAdminVendorIdDirty);

    commissionMgr.CloseCommissioningWindow();
    EXPECT_FALSE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.CommissioningWindowStatusForCluster(),
              chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    EXPECT_TRUE(commissionMgr.GetOpenerFabricIndex().IsNull());
    EXPECT_TRUE(commissionMgr.GetOpenerVendorId().IsNull());
    EXPECT_TRUE(sWindowStatusDirty);
    EXPECT_TRUE(sAdminFabricIndexDirty);
    EXPECT_TRUE(sAdminVendorIdDirty);

    ResetDirtyFlags();
}

TEST_F(TestCommissioningWindowManager, TestCheckCommissioningWindowManagerEnhancedWindow)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(CheckCommissioningWindowManagerEnhancedWindowTask);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(StopEventLoop);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

} // namespace
