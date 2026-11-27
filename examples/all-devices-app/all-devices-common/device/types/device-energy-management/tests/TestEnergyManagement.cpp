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

#include <device/types/device-energy-management/EnergyManagement.h>
#include <pw_unit_test/framework.h>

#include <app/TestEventTriggerDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/DeviceEnergyManagement/Attributes.h>
#include <clusters/DeviceEnergyManagement/ClusterId.h>
#include <clusters/DeviceEnergyManagement/Commands.h>
#include <clusters/DeviceEnergyManagement/Events.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kEndpoint = 1;

// Values match the send_test_event_trigger_* helpers in src/python_testing/TC_DEMTestBase.py.
constexpr uint64_t kTriggerPowerAdjustment      = 0x0098'0000'0000'0000;
constexpr uint64_t kTriggerPowerAdjustmentClear = 0x0098'0000'0000'0001;
constexpr uint64_t kTriggerOptOutLocal          = 0x0098'0000'0000'0002;
constexpr uint64_t kTriggerOptOutGrid           = 0x0098'0000'0000'0003;
constexpr uint64_t kTriggerOptOutClearAll       = 0x0098'0000'0000'0004;
constexpr uint64_t kTriggerStartTimeAdjustment  = 0x0098'0000'0000'0005;

uint64_t TriggerForEndpoint(uint64_t eventTrigger, uint64_t endpoint)
{
    return eventTrigger | (endpoint << 32);
}

class EnergyManagementTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_SUCCESS(Platform::MemoryInit()); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        ASSERT_SUCCESS(mDevice.Register(kEndpoint, mProvider));
        ASSERT_SUCCESS(mProvider.Startup(mContext.ImContext()));
    }

    void TearDown() override
    {
        mDevice.Unregister(mProvider);
        EXPECT_SUCCESS(mProvider.Shutdown());
    }

    Status PowerAdjust(int64_t power, uint32_t duration, AdjustmentCauseEnum cause)
    {
        Commands::PowerAdjustRequest::Type request;
        request.power    = power;
        request.duration = duration;
        request.cause    = cause;
        return StatusOf(
            Testing::ClusterTester(mDevice.EnergyManagementCluster()).Invoke(Commands::PowerAdjustRequest::Id, request));
    }

    Status CancelPowerAdjust()
    {
        Commands::CancelPowerAdjustRequest::Type request;
        return StatusOf(
            Testing::ClusterTester(mDevice.EnergyManagementCluster()).Invoke(Commands::CancelPowerAdjustRequest::Id, request));
    }

    void ExpectPowerAdjustStartEvent()
    {
        auto event = mContext.EventsGenerator().GetNextEvent();
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ(event->eventOptions.mPath, ConcreteEventPath(kEndpoint, Id, Events::PowerAdjustStart::Id));
    }

    void ExpectPowerAdjustEndEvent(CauseEnum cause, uint32_t duration, int64_t energyUse)
    {
        auto event = mContext.EventsGenerator().GetNextEvent();
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ(event->eventOptions.mPath, ConcreteEventPath(kEndpoint, Id, Events::PowerAdjustEnd::Id));

        Events::PowerAdjustEnd::DecodableType decoded;
        ASSERT_SUCCESS(event->GetEventData(decoded));
        EXPECT_EQ(decoded.cause, cause);
        EXPECT_EQ(decoded.duration, duration);
        EXPECT_EQ(decoded.energyUse, energyUse);
    }

    void ExpectNoEvent() { EXPECT_FALSE(mContext.EventsGenerator().GetNextEvent().has_value()); }

    void ExpectIdle()
    {
        EXPECT_EQ(mDevice.GetESAState(), ESAStateEnum::kOnline);
        EXPECT_EQ(mDevice.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kNoAdjustment);
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    Testing::ScopedAttributeChangeListenerRegistration mChangeListenerRegistration{ mProvider, mContext };
    TimerDelegateMock mTimerDelegate;
    SimpleTestEventTriggerDelegate mTestEventTriggerDelegate;
    EnergyManagement mDevice{ { .timerDelegate = mTimerDelegate, .testEventTriggerDelegate = mTestEventTriggerDelegate },
                              { .featureFlags = BitMask<Feature>(Feature::kPowerAdjustment) } };

private:
    static Status StatusOf(const Testing::ClusterTester::InvokeResult<DataModel::NullObjectType> & result)
    {
        EXPECT_TRUE(result.GetStatusCode().has_value());
        return result.GetStatusCode().has_value() ? result.GetStatusCode()->GetStatus() : Status::Failure;
    }
};

TEST_F(EnergyManagementTestFixture, PowerAdjustmentEndsWhenDurationExpires)
{
    EXPECT_EQ(PowerAdjust(600, 60, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();
    EXPECT_EQ(mDevice.GetESAState(), ESAStateEnum::kPowerAdjustActive);
    EXPECT_EQ(mDevice.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kLocalOptimizationAdjustment);

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(59));
    EXPECT_EQ(mDevice.GetESAState(), ESAStateEnum::kPowerAdjustActive);
    ExpectNoEvent();

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(1));
    // 600 mW for 60 s = 10 mWh
    ExpectPowerAdjustEndEvent(CauseEnum::kNormalCompletion, 60, 10);
    ExpectIdle();

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(120));
    ExpectNoEvent();
}

TEST_F(EnergyManagementTestFixture, CancelEndsPowerAdjustment)
{
    EXPECT_EQ(CancelPowerAdjust(), Status::InvalidInState);
    ExpectNoEvent();

    EXPECT_EQ(PowerAdjust(720, 60, AdjustmentCauseEnum::kGridOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();
    EXPECT_EQ(mDevice.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kGridOptimizationAdjustment);

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(10));
    EXPECT_EQ(CancelPowerAdjust(), Status::Success);
    // 720 mW for 10 s = 2 mWh
    ExpectPowerAdjustEndEvent(CauseEnum::kCancelled, 10, 2);
    ExpectIdle();

    // The duration timer of the cancelled adjustment does not end anything.
    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(60));
    ExpectNoEvent();
}

TEST_F(EnergyManagementTestFixture, RequestDuringAdjustmentRestartsDurationWithoutStartEvent)
{
    EXPECT_EQ(PowerAdjust(360, 60, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(30));
    EXPECT_EQ(PowerAdjust(720, 60, AdjustmentCauseEnum::kGridOptimization), Status::Success);
    ExpectNoEvent();
    EXPECT_EQ(mDevice.GetESAState(), ESAStateEnum::kPowerAdjustActive);
    EXPECT_EQ(mDevice.GetPowerAdjustmentCapability().Value().cause, PowerAdjustReasonEnum::kGridOptimizationAdjustment);

    // 60 s after the first request: the second request restarted the duration.
    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(30));
    ExpectNoEvent();

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(30));
    // 360 mW for 30 s = 3 mWh, then 720 mW for 60 s = 12 mWh
    ExpectPowerAdjustEndEvent(CauseEnum::kNormalCompletion, 90, 15);
    ExpectIdle();
}

TEST_F(EnergyManagementTestFixture, EnergyUseIsNotRoundedDownPerRequest)
{
    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerPowerAdjustment));

    EXPECT_EQ(PowerAdjust(5'000'000, 20, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();
    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(10));
    EXPECT_EQ(PowerAdjust(5'000'000, 10, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectNoEvent();

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(10));
    // 5 kW for 20 s = 27777.7 mWh. Rounding the two 10 s intervals down separately would give 27776 mWh.
    ExpectPowerAdjustEndEvent(CauseEnum::kNormalCompletion, 20, 27'777);
    ExpectIdle();
}

TEST_F(EnergyManagementTestFixture, PowerAdjustmentTriggerChangesCapability)
{
    EXPECT_EQ(mDevice.GetAbsMinPower(), 0);
    EXPECT_EQ(mDevice.GetAbsMaxPower(), 1000);
    EXPECT_EQ(PowerAdjust(5'000'000, 10, AdjustmentCauseEnum::kLocalOptimization), Status::ConstraintError);

    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerPowerAdjustment));
    EXPECT_EQ(mDevice.GetAbsMinPower(), 5'000'000);
    EXPECT_EQ(mDevice.GetAbsMaxPower(), 30'000'000);
    {
        const auto & capability = mDevice.GetPowerAdjustmentCapability();
        ASSERT_FALSE(capability.IsNull());
        ASSERT_FALSE(capability.Value().powerAdjustCapability.IsNull());
        ASSERT_EQ(capability.Value().powerAdjustCapability.Value().size(), 1u);
        const auto & entry = capability.Value().powerAdjustCapability.Value()[0];
        EXPECT_EQ(entry.minPower, 5'000'000);
        EXPECT_EQ(entry.maxPower, 30'000'000);
        EXPECT_EQ(entry.minDuration, 10u);
        EXPECT_EQ(entry.maxDuration, 60u);
    }
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(ConcreteAttributePath(kEndpoint, Id, Attributes::PowerAdjustmentCapability::Id)));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(ConcreteAttributePath(kEndpoint, Id, Attributes::AbsMinPower::Id)));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(ConcreteAttributePath(kEndpoint, Id, Attributes::AbsMaxPower::Id)));

    EXPECT_EQ(PowerAdjust(4'999'999, 10, AdjustmentCauseEnum::kLocalOptimization), Status::ConstraintError);
    EXPECT_EQ(PowerAdjust(5'000'000, 9, AdjustmentCauseEnum::kLocalOptimization), Status::ConstraintError);
    EXPECT_EQ(PowerAdjust(30'000'000, 10, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();

    // Clearing the trigger restores limits that the active adjustment does not satisfy.
    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerPowerAdjustmentClear));
    ExpectPowerAdjustEndEvent(CauseEnum::kCancelled, 0, 0);
    ExpectIdle();
    EXPECT_EQ(mDevice.GetAbsMaxPower(), 1000);
}

TEST_F(EnergyManagementTestFixture, UserOptOutEndsMatchingPowerAdjustment)
{
    EXPECT_EQ(PowerAdjust(360, 60, AdjustmentCauseEnum::kGridOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();

    // Opting out of local optimization does not affect a grid adjustment.
    const ConcreteClusterPath clusterPath(kEndpoint, Id);
    const DataVersion dataVersion = mDevice.EnergyManagementCluster().GetDataVersion(clusterPath);
    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerOptOutLocal));
    EXPECT_NE(mDevice.EnergyManagementCluster().GetDataVersion(clusterPath), dataVersion);
    EXPECT_EQ(mDevice.GetOptOutState(), OptOutStateEnum::kLocalOptOut);
    EXPECT_EQ(mDevice.GetESAState(), ESAStateEnum::kPowerAdjustActive);
    ExpectNoEvent();
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(ConcreteAttributePath(kEndpoint, Id, Attributes::OptOutState::Id)));
    EXPECT_EQ(PowerAdjust(360, 60, AdjustmentCauseEnum::kLocalOptimization), Status::ConstraintError);

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(20));
    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerOptOutGrid));
    EXPECT_EQ(mDevice.GetOptOutState(), OptOutStateEnum::kOptOut);
    // 360 mW for 20 s = 2 mWh
    ExpectPowerAdjustEndEvent(CauseEnum::kUserOptOut, 20, 2);
    ExpectIdle();

    EXPECT_EQ(PowerAdjust(360, 60, AdjustmentCauseEnum::kGridOptimization), Status::ConstraintError);
    EXPECT_EQ(PowerAdjust(360, 60, AdjustmentCauseEnum::kLocalOptimization), Status::ConstraintError);

    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerOptOutClearAll));
    EXPECT_EQ(mDevice.GetOptOutState(), OptOutStateEnum::kNoOptOut);
    EXPECT_EQ(PowerAdjust(360, 60, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();
}

TEST_F(EnergyManagementTestFixture, HandlesTriggersOnlyForOwnEndpointWhileRegistered)
{
    EXPECT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(TriggerForEndpoint(kTriggerOptOutLocal, kEndpoint)));
    EXPECT_EQ(mDevice.GetOptOutState(), OptOutStateEnum::kLocalOptOut);

    EXPECT_EQ(mTestEventTriggerDelegate.HandleEventTriggers(TriggerForEndpoint(kTriggerOptOutGrid, kEndpoint + 1u)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mDevice.GetOptOutState(), OptOutStateEnum::kLocalOptOut);

    // Start Time Adjustment is not simulated.
    EXPECT_EQ(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerStartTimeAdjustment), CHIP_ERROR_INVALID_ARGUMENT);

    mDevice.Unregister(mProvider);
    EXPECT_EQ(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerOptOutClearAll), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mDevice.GetOptOutState(), OptOutStateEnum::kNoOptOut);

    ASSERT_SUCCESS(mDevice.Register(kEndpoint, mProvider));
}

TEST_F(EnergyManagementTestFixture, UnregisterDropsActivePowerAdjustment)
{
    ASSERT_SUCCESS(mTestEventTriggerDelegate.HandleEventTriggers(kTriggerPowerAdjustment));
    EXPECT_EQ(PowerAdjust(5'000'000, 10, AdjustmentCauseEnum::kLocalOptimization), Status::Success);
    ExpectPowerAdjustStartEvent();

    mDevice.Unregister(mProvider);
    ExpectNoEvent();
    EXPECT_EQ(mDevice.GetESAState(), ESAStateEnum::kOnline);
    EXPECT_EQ(mDevice.GetAbsMaxPower(), 1000);

    mTimerDelegate.AdvanceClock(System::Clock::Seconds32(10));
    ExpectNoEvent();

    ASSERT_SUCCESS(mDevice.Register(kEndpoint, mProvider));
}

TEST(EnergyManagementWithoutPowerAdjustment, RejectsPowerAdjustmentTriggers)
{
    ASSERT_SUCCESS(Platform::MemoryInit());
    {
        Testing::TestServerClusterContext context;
        CodeDrivenDataModelProvider provider{ context.StorageDelegate(), context.AttributePersistenceProvider() };
        TimerDelegateMock timerDelegate;
        SimpleTestEventTriggerDelegate testEventTriggerDelegate;
        EnergyManagement device({ .timerDelegate = timerDelegate, .testEventTriggerDelegate = testEventTriggerDelegate });

        ASSERT_SUCCESS(device.Register(kEndpoint, provider));
        ASSERT_SUCCESS(provider.Startup(context.ImContext()));

        EXPECT_EQ(testEventTriggerDelegate.HandleEventTriggers(kTriggerPowerAdjustment), CHIP_ERROR_INVALID_ARGUMENT);
        EXPECT_EQ(testEventTriggerDelegate.HandleEventTriggers(kTriggerPowerAdjustmentClear), CHIP_ERROR_INVALID_ARGUMENT);
        EXPECT_TRUE(device.GetPowerAdjustmentCapability().IsNull());

        device.Unregister(provider);
        EXPECT_SUCCESS(provider.Shutdown());
    }
    Platform::MemoryShutdown();
}

} // namespace
