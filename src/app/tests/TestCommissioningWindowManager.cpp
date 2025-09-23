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
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <crypto/RandUtils.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
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

#include <app/tests/CommissioningWindowManagerTestAccess.h>

using namespace chip::Crypto;

using chip::CommissioningWindowAdvertisement;
using chip::CommissioningWindowManager;
using chip::Server;

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

class TestCommissioningWindowManagerDataModelProvider : public chip::app::CodegenDataModelProvider
{
public:
    TestCommissioningWindowManagerDataModelProvider()  = default;
    ~TestCommissioningWindowManagerDataModelProvider() = default;

    void Temporary_ReportAttributeChanged(const chip::app::AttributePathParams & path) override
    {
        using namespace chip::app::Clusters;
        using namespace chip::app::Clusters::AdministratorCommissioning::Attributes;
        if (path.mEndpointId != chip::kRootEndpointId || path.mClusterId != AdministratorCommissioning::Id)
        {
            return;
        }

        switch (path.mAttributeId)
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
};

chip::app::DataModel::Provider * TestDataModelProviderInstance(chip::PersistentStorageDelegate * delegate)
{
    static TestCommissioningWindowManagerDataModelProvider gTestModel;

    if (delegate != nullptr)
    {
        gTestModel.SetPersistentStorageDelegate(delegate);
    }

    return &gTestModel;
}

} // namespace
namespace {

void TearDownTask(intptr_t context)
{
    chip::Server::GetInstance().Shutdown();
}

static void StopEventLoop(intptr_t context)
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

class MockDnssdServer : public chip::IDnssdServer
{
public:
    MockDnssdServer() : mAdvertisingEnabled(false) {}
    CHIP_ERROR AdvertiseOperational() override
    {
        mAdvertisingEnabled = true;
        return CHIP_NO_ERROR;
    }
    bool IsAdvertisingEnabled() override { return mAdvertisingEnabled; }

private:
    bool mAdvertisingEnabled;
};

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
        initParams.dataModelProvider = TestDataModelProviderInstance(initParams.persistentStorageDelegate);
        // Use whatever server port the kernel decides to give us.
        initParams.operationalServicePort = 0;

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

chip::DeviceLayer::ChipDeviceEvent CreateEvent(uint16_t eventType)
{
    chip::DeviceLayer::ChipDeviceEvent event;
    event.Type = eventType;
    return event;
}

// Verify that the commissioning window is closed when commissioning has completed
TEST_F(TestCommissioningWindowManager, TestOnPlatformEventCommissioningComplete)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();

    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                         CommissioningWindowAdvertisement::kAllSupported),
              CHIP_NO_ERROR);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());

    auto event = CreateEvent(chip::DeviceLayer::DeviceEventType::kCommissioningComplete);

    commissionMgr.OnPlatformEvent(&event);
    // When the commissioning has completed (kCommissioningComplete event) OnPlatformEvent cleans up, closes active sessions and the
    // commissioning window is closed The device should no longer be discoverable or accept new commissioners
    EXPECT_FALSE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.GetCommissioningMode(), chip::Dnssd::CommissioningMode::kDisabled);

// When BLE is enabled
#if CONFIG_NETWORK_LAYER_BLE && CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    EXPECT_FALSE(Server::GetInstance().GetBleLayerObject()->IsBleClosing());
#endif
}

// Verify that on normal failsafe timer expiry the commissioning window remains open
// The explicit CloseCommissioningWindow() call is for cleanup and also checks that the commissioning mode changes to kDisabled
TEST_F(TestCommissioningWindowManager, TestOnPlatformEventFailSafeTimerExpired)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();

    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                         CommissioningWindowAdvertisement::kDnssdOnly),
              CHIP_NO_ERROR);

    auto event = CreateEvent(chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired);

    commissionMgr.OnPlatformEvent(&event);
    EXPECT_TRUE(commissionMgr.IsCommissioningWindowOpen());
    EXPECT_EQ(commissionMgr.GetCommissioningMode(), chip::Dnssd::CommissioningMode::kEnabledBasic);
    commissionMgr.CloseCommissioningWindow();
    EXPECT_EQ(commissionMgr.GetCommissioningMode(), chip::Dnssd::CommissioningMode::kDisabled);
}

// Verify that PASE session is properly evicted when failsafe timer is expired
TEST_F(TestCommissioningWindowManager, TestOnPlatformEventFailSafeTimerExpiredPASEEVicted)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    auto access                                = chip::Test::CommissioningWindowManagerTestAccess(&commissionMgr);
    auto & sessionMgr                          = Server::GetInstance().GetSecureSessionManager();

    auto & paseSession            = access.GetPASESession();
    uint16_t localSessionId       = 1;
    chip::NodeId peerNodeId       = chip::kUndefinedNodeId;
    uint16_t peerSessionId        = 2;
    chip::FabricIndex fabricIndex = chip::kUndefinedFabricIndex;
    chip::Transport::PeerAddress peerAddress =
        chip::Transport::PeerAddress::UDP(chip::Inet::IPAddress::Loopback(chip::Inet::IPAddressType::kAny), CHIP_PORT);

    auto role = chip::CryptoContext::SessionRole::kResponder;

    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                         CommissioningWindowAdvertisement::kDnssdOnly),
              CHIP_NO_ERROR);

    // Inject a fake PASE session into SessionManager
    CHIP_ERROR err = sessionMgr.InjectPaseSessionWithTestKey(paseSession, localSessionId, peerNodeId, peerSessionId, fabricIndex,
                                                             peerAddress, role);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(paseSession.Get().HasValue());
    EXPECT_TRUE(paseSession->AsSecureSession()->IsActiveSession());

    auto event = CreateEvent(chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired);

    commissionMgr.OnPlatformEvent(&event);
    // Verify that PASE Session was evicted after failsafe timer expiration
    EXPECT_FALSE(paseSession.Get().HasValue());
    commissionMgr.CloseCommissioningWindow();
    EXPECT_EQ(commissionMgr.GetCommissioningMode(), chip::Dnssd::CommissioningMode::kDisabled);
}

// Verify that operational advertising is started when the operational network is enabled
TEST_F(TestCommissioningWindowManager, TestOnPlatformEventOperationalNetworkEnabled)
{
    MockDnssdServer mockDnssd;
    CommissioningWindowManager commissionMgr = CommissioningWindowManager(&mockDnssd);
    auto event                               = CreateEvent(chip::DeviceLayer::DeviceEventType::kOperationalNetworkEnabled);

    commissionMgr.OnPlatformEvent(&event);
    EXPECT_TRUE(mockDnssd.IsAdvertisingEnabled());
}

// Verify that operational advertising failure is handled gracefully
TEST_F(TestCommissioningWindowManager, TestOnPlatformEventOperationalNetworkEnabledFail)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();
    auto & fabricTable                         = Server::GetInstance().GetFabricTable();

    // Stopping DNS-SD server to trigger AdvertiseOperational() failure
    chip::app::DnssdServer::Instance().StopServer();

    auto event = CreateEvent(chip::DeviceLayer::DeviceEventType::kOperationalNetworkEnabled);

    commissionMgr.OnPlatformEvent(&event);
    // This should attempt to start operational advertising, which will fail
    EXPECT_EQ(chip::app::DnssdServer::Instance().AdvertiseOperational(), CHIP_ERROR_INCORRECT_STATE);
    // StopServer() clears the FabricTable pointer
    // we must restore it with SetFabricTable before calling StartServer()
    // to ensure the server reinitializes correctly and to maintain a clean state for subsequent tests
    chip::app::DnssdServer::Instance().SetFabricTable(&fabricTable);
    chip::app::DnssdServer::Instance().StartServer();
}

// Verify that BLE advertising is stopped when all BLE connections are closed
// BLE advertisement is not supported on Darwin in Matter
// so this test is skipped on Darwin since it requires the device to act as an advertiser
#if CONFIG_NETWORK_LAYER_BLE && !CHIP_DEVICE_LAYER_TARGET_DARWIN
TEST_F(TestCommissioningWindowManager, TestOnPlatformEventCloseAllBleConnections)
{
    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();

    EXPECT_EQ(commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                         CommissioningWindowAdvertisement::kAllSupported),
              CHIP_NO_ERROR);

    // ensure that BLE advertising is active before the event generation
    EXPECT_TRUE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
    auto event = CreateEvent(chip::DeviceLayer::DeviceEventType::kCloseAllBleConnections);

    commissionMgr.OnPlatformEvent(&event);
    commissionMgr.CloseCommissioningWindow();
    EXPECT_FALSE(chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled());
}
#endif

} // namespace
