/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <DEMConfig.h>
#include <DeviceEnergyManagementDelegateImpl.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::DeviceEnergyManagement;

namespace {

DeviceEnergyManagementDelegate * sDEMDelegate = nullptr;

class TestDEMTestEventTriggers : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        mDelegate.SetDeviceEnergyManagementInstance(mInstance);
        sDEMDelegate = &mDelegate;
    }

    void TearDown() override { sDEMDelegate = nullptr; }

protected:
    CHIP_ERROR Trigger(DeviceEnergyManagementTrigger trigger) { return mHandler.HandleEventTrigger(to_underlying(trigger)); }

    void SetForecast(uint16_t activeSlot, size_t numSlots = 2)
    {
        Structs::ForecastStruct::Type forecast;
        forecast.activeSlotNumber.SetNonNull(activeSlot);
        forecast.slots = DataModel::List<const Structs::SlotStruct::Type>(mSlots, numSlots);
        ASSERT_EQ(mDelegate.SetForecast(DataModel::MakeNullable(forecast)), CHIP_NO_ERROR);
    }

    void ExpectNextSlotEventsRejected()
    {
        EXPECT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), CHIP_ERROR_INVALID_ARGUMENT);
        EXPECT_EQ(Trigger(DeviceEnergyManagementTrigger::kPausableNextSlot), CHIP_ERROR_INVALID_ARGUMENT);
    }

    DeviceLayer::StackLock mStackLock;
    DeviceEnergyManagementDelegate mDelegate;
    Instance mInstance{ 1, mDelegate, Feature::kPowerForecastReporting };
    Structs::SlotStruct::Type mSlots[3];
    DeviceEnergyManagementTestEventTriggerHandler mHandler;
};

TEST_F(TestDEMTestEventTriggers, NextSlotRejectsNullForecast)
{
    ASSERT_TRUE(mDelegate.GetForecast().IsNull());
    ExpectNextSlotEventsRejected();
    EXPECT_TRUE(mDelegate.GetForecast().IsNull());
}

TEST_F(TestDEMTestEventTriggers, ForecastClearThenNextSlot)
{
    ASSERT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustment), CHIP_NO_ERROR);
    ASSERT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastClear), CHIP_NO_ERROR);
    ASSERT_FALSE(mDelegate.GetForecast().IsNull());
    ASSERT_TRUE(mDelegate.GetForecast().Value().activeSlotNumber.IsNull());

    ExpectNextSlotEventsRejected();
    EXPECT_TRUE(mDelegate.GetForecast().Value().activeSlotNumber.IsNull());
    EXPECT_EQ(mDelegate.GetForecast().Value().slots.size(), 0u);
}

TEST_F(TestDEMTestEventTriggers, NextSlotRejectsNullActiveSlot)
{
    SetForecast(0);
    auto forecast = mDelegate.GetForecast().Value();
    forecast.activeSlotNumber.SetNull();
    ASSERT_EQ(mDelegate.SetForecast(DataModel::MakeNullable(forecast)), CHIP_NO_ERROR);

    ExpectNextSlotEventsRejected();
    EXPECT_TRUE(mDelegate.GetForecast().Value().activeSlotNumber.IsNull());
    EXPECT_EQ(mDelegate.GetForecast().Value().slots.size(), 2u);
}

TEST_F(TestDEMTestEventTriggers, NextSlotRejectsEmptySlotList)
{
    SetForecast(0, 0);
    ExpectNextSlotEventsRejected();
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 0u);
}

TEST_F(TestDEMTestEventTriggers, NextSlotRejectsSingleSlotForecast)
{
    SetForecast(0, 1);
    ExpectNextSlotEventsRejected();
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 0u);
}

TEST_F(TestDEMTestEventTriggers, NextSlotRejectsOutOfRangeActiveSlot)
{
    SetForecast(2);
    ExpectNextSlotEventsRejected();
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 2u);
}

TEST_F(TestDEMTestEventTriggers, NextSlotRejectsMaximumActiveSlot)
{
    SetForecast(UINT16_MAX);
    ExpectNextSlotEventsRejected();
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), UINT16_MAX);
}

TEST_F(TestDEMTestEventTriggers, ForecastAdjustmentRejectsLastSlot)
{
    SetForecast(1);
    EXPECT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 1u);
}

TEST_F(TestDEMTestEventTriggers, ForecastAdjustmentAdvancesActiveSlot)
{
    SetForecast(1, 3);
    EXPECT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), CHIP_NO_ERROR);
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 2u);
    EXPECT_EQ(mDelegate.GetForecast().Value().slots.size(), 3u);
}

TEST_F(TestDEMTestEventTriggers, RepeatedForecastAdjustmentStopsAtLastSlot)
{
    ASSERT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustment), CHIP_NO_ERROR);
    ASSERT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 1u);

    EXPECT_EQ(Trigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 1u);
}

TEST_F(TestDEMTestEventTriggers, PausableSelectsSecondSlot)
{
    ASSERT_EQ(Trigger(DeviceEnergyManagementTrigger::kPausable), CHIP_NO_ERROR);
    ASSERT_EQ(Trigger(DeviceEnergyManagementTrigger::kPausableNextSlot), CHIP_NO_ERROR);
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 1u);

    // The pausable event selects slot 1 rather than incrementing the active slot.
    EXPECT_EQ(Trigger(DeviceEnergyManagementTrigger::kPausableNextSlot), CHIP_NO_ERROR);
    EXPECT_EQ(mDelegate.GetForecast().Value().activeSlotNumber.Value(), 1u);
}

TEST_F(TestDEMTestEventTriggers, GeneralDiagnosticsReportsInvalidNextSlot)
{
    namespace GeneralDiagnostics = chip::app::Clusters::GeneralDiagnostics;
    using chip::app::Clusters::GeneralDiagnosticsCluster;
    using chip::Protocols::InteractionModel::ClusterStatusCode;
    using chip::Protocols::InteractionModel::Status;

    constexpr uint8_t enableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 1 };
    SimpleTestEventTriggerDelegate delegate;
    DeviceEnergyManagementTestEventTriggerHandler handler;
    ASSERT_EQ(delegate.Init(ByteSpan(enableKey)), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.AddHandler(&handler), CHIP_NO_ERROR);

    GeneralDiagnosticsCluster cluster({}, {},
                                      GeneralDiagnosticsCluster::Context{
                                          .deviceLoadStatusProvider = *InteractionModelEngine::GetInstance(),
                                          .diagnosticDataProvider   = DeviceLayer::GetDiagnosticDataProvider(),
                                          .testEventTriggerDelegate = &delegate,
                                      });
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    GeneralDiagnostics::Commands::TestEventTrigger::Type request;
    request.enableKey  = ByteSpan(enableKey);
    auto invokeTrigger = [&](DeviceEnergyManagementTrigger trigger) {
        request.eventTrigger = to_underlying(trigger);
        return tester.Invoke(GeneralDiagnostics::Commands::TestEventTrigger::Id, request).GetStatusCode();
    };

    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), ClusterStatusCode(Status::InvalidCommand));
    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kPausableNextSlot), ClusterStatusCode(Status::InvalidCommand));
    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kForecastClear), ClusterStatusCode(Status::Success));
    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), ClusterStatusCode(Status::InvalidCommand));
    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kForecastAdjustment), ClusterStatusCode(Status::Success));
    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), ClusterStatusCode(Status::Success));
    EXPECT_EQ(invokeTrigger(DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot), ClusterStatusCode(Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace

DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    return sDEMDelegate;
}
