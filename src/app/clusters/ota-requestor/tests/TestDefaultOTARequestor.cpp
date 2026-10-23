/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>

#include <lib/support/CHIPMem.h>
#include <optional>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include <pw_unit_test/framework.h>
#include <vector>

using namespace chip;
using chip::app::Clusters::OtaSoftwareUpdateRequestor::AnnouncementReasonEnum;
using chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;
using OTAUpdateStateEnum = OTARequestorInterface::OTAUpdateStateEnum;

namespace chip {
namespace DeviceLayer {
void StartDelayTimerHandler(System::Layer * systemLayer, void * appState);
} // namespace DeviceLayer
} // namespace chip

namespace {

class FakeDriver : public OTARequestorDriver
{
public:
    bool CanConsent() override { return false; }
    void SetMaxDownloadBlockSize(uint16_t) override {}
    void HandleIdleStateExit() override {}
    void HandleIdleStateEnter(IdleStateReason reason) override { mIdleReasons.push_back(reason); }
    void UpdateAvailable(const UpdateDescription &, System::Clock::Seconds32) override {}
    CHIP_ERROR UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32) override
    {
        mNotFoundReasons.push_back(reason);
        return CHIP_NO_ERROR;
    }
    void UpdateDownloaded() override {}
    void UpdateConfirmed(System::Clock::Seconds32) override {}
    void UpdateSuspended(System::Clock::Seconds32) override {}
    void UpdateDiscontinued() override {}
    void UpdateCancelled() override {}
    void OTACommissioningCallback() override {}
    void ProcessAnnounceOTAProviders(const ProviderLocationType &, AnnouncementReasonEnum) override {}
    void SendQueryImage() override {}
    bool GetNextProviderLocation(ProviderLocationType &, bool &) override { return false; }

    std::vector<IdleStateReason> mIdleReasons;
    std::vector<UpdateNotFoundReason> mNotFoundReasons;
};

class FakeStorage : public OTARequestorStorage
{
public:
    CHIP_ERROR StoreDefaultProviders(const ProviderLocationList &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadDefaultProviders(ProviderLocationList &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR StoreCurrentProviderLocation(const ProviderLocationType &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClearCurrentProviderLocation() override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadCurrentProviderLocation(ProviderLocationType &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR StoreUpdateToken(ByteSpan) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadUpdateToken(MutableByteSpan &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR ClearUpdateToken() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StoreCurrentUpdateState(OTAUpdateStateEnum) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadCurrentUpdateState(OTAUpdateStateEnum &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR ClearCurrentUpdateState() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StoreTargetVersion(uint32_t) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadTargetVersion(uint32_t &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR ClearTargetVersion() override { return CHIP_NO_ERROR; }
};

class FakeEventGenerator : public DefaultOTARequestorEventGenerator
{
public:
    CHIP_ERROR GenerateVersionAppliedEvent(const VersionAppliedEvent &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GenerateDownloadErrorEvent(const DownloadErrorEvent &) override
    {
        mDownloadErrors++;
        return CHIP_NO_ERROR;
    }

    uint32_t mDownloadErrors = 0;
};

class FakeImageProcessor : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Finalize() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Apply() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Abort() override { return CHIP_NO_ERROR; }
    CHIP_ERROR ProcessBlock(ByteSpan &) override { return CHIP_NO_ERROR; }
    bool IsFirstImageRun() override { return false; }
    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }
};

// Static storage: Init() registers a platform event handler that is never removed.
FakeDriver gDriver;
FakeStorage gStorage;
FakeEventGenerator gEvents;
FakeImageProcessor gProcessor;
BDXDownloader gDownloader;
OTARequestorAttributes gAttributes;
DefaultOTARequestor gRequestor;

class TimedDriver : public DeviceLayer::DefaultOTARequestorDriver
{
public:
    // Emulated clock: fires the driver timer due first and returns how far the clock moved.
    std::optional<System::Clock::Timeout> FireNextTimer()
    {
        const System::TimerCompleteCallback timers[] = { DeviceLayer::StartDelayTimerHandler, WatchdogTimerHandler,
                                                         PeriodicQueryTimerHandler };
        System::TimerCompleteCallback next           = nullptr;
        System::Clock::Timeout due;
        for (auto timer : timers)
        {
            if (DeviceLayer::SystemLayer().IsTimerActive(timer, this) &&
                (next == nullptr || DeviceLayer::SystemLayer().GetRemainingTime(timer, this) < due))
            {
                next = timer;
                due  = DeviceLayer::SystemLayer().GetRemainingTime(timer, this);
            }
        }
        if (next == nullptr)
        {
            return std::nullopt;
        }
        DeviceLayer::SystemLayer().CancelTimer(next, this);
        mWatchdogFired += (next == WatchdogTimerHandler) ? 1 : 0;
        mPeriodicFired += (next == PeriodicQueryTimerHandler) ? 1 : 0;
        next(&DeviceLayer::SystemLayer(), this);
        return due;
    }
    void CancelTimers()
    {
        UpdateCancelled();
        StopWatchdogTimer();
        StopPeriodicQueryTimer();
    }

    uint32_t mWatchdogFired = 0;
    uint32_t mPeriodicFired = 0;
};

OTARequestorAttributes gBusyAttributes;
DefaultOTARequestor gBusyRequestor;
TimedDriver gTimedDriver;

// Answers every query with an update whose download the provider refuses as busy.
class BusyProvider : public OTARequestorInterface
{
public:
    void TriggerImmediateQueryInternal() override
    {
        mQueries++;
        gTimedDriver.HandleIdleStateExit();
        gBusyAttributes.SetUpdateState(OTAUpdateStateEnum::kDownloading, OTAChangeReasonEnum::kSuccess,
                                       app::DataModel::NullNullable);
        gBusyRequestor.OnDownloadStateChanged(OTADownloader::State::kIdle, OTAChangeReasonEnum::kDelayByProvider);
    }
    OTAUpdateStateEnum GetCurrentUpdateState() override { return gBusyRequestor.GetCurrentUpdateState(); }
    void CancelImageUpdate() override { mCancels++; }
    void SetCurrentProviderLocation(ProviderLocationType location) override { gBusyRequestor.SetCurrentProviderLocation(location); }
    void GetProviderLocation(Optional<ProviderLocationType> & location) override { gBusyRequestor.GetProviderLocation(location); }
    ProviderLocationList::Iterator GetDefaultOTAProviderListIterator() override
    {
        return gBusyRequestor.GetDefaultOTAProviderListIterator();
    }
    void HandleAnnounceOTAProvider(
        app::CommandHandler *, const app::ConcreteCommandPath &,
        const app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType &) override
    {}
    void Reset() override {}
    CHIP_ERROR TriggerImmediateQuery(FabricIndex) override { return CHIP_NO_ERROR; }
    void DownloadUpdate() override {}
    void DownloadUpdateDelayedOnUserConsent() override {}
    void ApplyUpdate() override {}
    void NotifyUpdateApplied() override {}
    CHIP_ERROR GetUpdateStateProgressAttribute(EndpointId, app::DataModel::Nullable<uint8_t> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetUpdateStateAttribute(EndpointId, OTAUpdateStateEnum &) override { return CHIP_NO_ERROR; }
    uint32_t GetTargetVersion() override { return 0; }
    CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex) override { return CHIP_NO_ERROR; }
    void SetMetadataForProvider(ByteSpan) override {}
    CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType &) override { return CHIP_NO_ERROR; }

    uint32_t mQueries = 0;
    uint32_t mCancels = 0;
};

BusyProvider gBusyProvider;

void RunScheduledWork()
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t) { TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask(); });
    DeviceLayer::PlatformMgr().RunEventLoop();
}

class TestDefaultOTARequestor : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        gDownloader.SetImageProcessorDelegate(&gProcessor);
        ASSERT_EQ(gRequestor.Init(Server::GetInstance(), gStorage, gDriver, gDownloader, gAttributes, gEvents), CHIP_NO_ERROR);
        ASSERT_EQ(gBusyRequestor.Init(Server::GetInstance(), gStorage, gTimedDriver, gDownloader, gBusyAttributes, gEvents),
                  CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        gAttributes.SetUpdateState(OTAUpdateStateEnum::kDownloading, OTAChangeReasonEnum::kSuccess, app::DataModel::NullNullable);
        gDriver.mIdleReasons.clear();
        gDriver.mNotFoundReasons.clear();
        gEvents.mDownloadErrors = 0;
    }
};

TEST_F(TestDefaultOTARequestor, ResponderBusyDuringDownloadSchedulesABusyRetry)
{
    gRequestor.OnDownloadStateChanged(OTADownloader::State::kIdle, OTAChangeReasonEnum::kDelayByProvider);

    ASSERT_EQ(gDriver.mNotFoundReasons.size(), 1u);
    EXPECT_EQ(gDriver.mNotFoundReasons[0], UpdateNotFoundReason::kBusy);
    EXPECT_EQ(gRequestor.GetCurrentUpdateState(), OTAUpdateStateEnum::kIdle);
    ASSERT_EQ(gDriver.mIdleReasons.size(), 1u);
    EXPECT_EQ(gDriver.mIdleReasons[0], IdleStateReason::kUnknown);
    EXPECT_EQ(gEvents.mDownloadErrors, 1u);
}

TEST_F(TestDefaultOTARequestor, OtherDownloadFailureSchedulesNoBusyRetry)
{
    gRequestor.OnDownloadStateChanged(OTADownloader::State::kIdle, OTAChangeReasonEnum::kFailure);

    EXPECT_TRUE(gDriver.mNotFoundReasons.empty());
    EXPECT_EQ(gRequestor.GetCurrentUpdateState(), OTAUpdateStateEnum::kIdle);
    ASSERT_EQ(gDriver.mIdleReasons.size(), 1u);
    EXPECT_EQ(gDriver.mIdleReasons[0], IdleStateReason::kUnknown);
    EXPECT_EQ(gEvents.mDownloadErrors, 1u);
}

TEST_F(TestDefaultOTARequestor, BusyRetriesOutlastingTheWatchdogAreNotCutShort)
{
    constexpr uint32_t kWatchdogSec = 100;
    gBusyAttributes.SetUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess, app::DataModel::NullNullable);
    gBusyRequestor.SetCurrentProviderLocation({ .providerNodeID = 0x1234, .endpoint = 0, .fabricIndex = 1 });
    gTimedDriver.SetWatchdogTimeout(kWatchdogSec);
    gTimedDriver.Init(&gBusyProvider, &gProcessor);

    gTimedDriver.SendQueryImage();
    RunScheduledWork();
    ASSERT_EQ(gBusyProvider.mQueries, 1u);

    System::Clock::Milliseconds64 elapsed{ 0 };
    System::Clock::Milliseconds64 lastQueryAt{ 0 };
    while (gTimedDriver.mPeriodicFired == 0)
    {
        std::optional<System::Clock::Timeout> due = gTimedDriver.FireNextTimer();
        ASSERT_TRUE(due.has_value());
        elapsed += due.value_or(System::Clock::kZero);
        uint32_t queries = gBusyProvider.mQueries;
        RunScheduledWork();
        lastQueryAt = (gBusyProvider.mQueries > queries) ? elapsed : lastQueryAt;
        ASSERT_LT(gBusyProvider.mQueries, 10u);
    }

    EXPECT_EQ(gBusyProvider.mQueries, 4u);
    EXPECT_GT(lastQueryAt, System::Clock::Seconds32(3 * kWatchdogSec));
    EXPECT_EQ(gTimedDriver.mWatchdogFired, 0u);
    EXPECT_EQ(gBusyProvider.mCancels, 0u);
    gTimedDriver.CancelTimers();
}

TEST_F(TestDefaultOTARequestor, AnnounceWhileBusyRetryIsPendingReplacesTheRetry)
{
    gBusyAttributes.SetUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess, app::DataModel::NullNullable);
    gBusyRequestor.SetCurrentProviderLocation({ .providerNodeID = 0x1234, .endpoint = 0, .fabricIndex = 1 });
    gTimedDriver.Init(&gBusyProvider, &gProcessor);

    gBusyAttributes.SetUpdateState(OTAUpdateStateEnum::kDownloading, OTAChangeReasonEnum::kSuccess, app::DataModel::NullNullable);
    gBusyRequestor.OnDownloadStateChanged(OTADownloader::State::kIdle, OTAChangeReasonEnum::kDelayByProvider);
    ASSERT_TRUE(DeviceLayer::SystemLayer().IsTimerActive(DeviceLayer::StartDelayTimerHandler, &gTimedDriver));
    EXPECT_GT(DeviceLayer::SystemLayer().GetRemainingTime(DeviceLayer::StartDelayTimerHandler, &gTimedDriver),
              System::Clock::Seconds32(1));

    gTimedDriver.ProcessAnnounceOTAProviders({ .providerNodeID = 0x5678, .endpoint = 0, .fabricIndex = 1 },
                                             AnnouncementReasonEnum::kUrgentUpdateAvailable);
    ASSERT_TRUE(DeviceLayer::SystemLayer().IsTimerActive(DeviceLayer::StartDelayTimerHandler, &gTimedDriver));
    EXPECT_LE(DeviceLayer::SystemLayer().GetRemainingTime(DeviceLayer::StartDelayTimerHandler, &gTimedDriver),
              System::Clock::Seconds32(1));
    Optional<OTARequestorInterface::ProviderLocationType> provider;
    gBusyRequestor.GetProviderLocation(provider);
    ASSERT_TRUE(provider.HasValue());
    EXPECT_EQ(provider.Value().providerNodeID, 0x5678u);
    gTimedDriver.CancelTimers();
}

} // namespace
