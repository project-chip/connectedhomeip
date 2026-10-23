/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <EnergyEvseDelegateImpl.h>
#include <EnergyEvseManager.h>
#include <app/ConcreteAttributePath.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/EventManagement.h>
#include <app/EventReporter.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <gtest/gtest.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kEndpointId              = 1;
constexpr int64_t kCircuitCapacity_mA         = 32000;
constexpr int64_t kMinimumCurrent_mA          = 6000;
constexpr int64_t kMaximumChargeCurrent_mA    = 20000;
constexpr int64_t kMaximumDischargeCurrent_mA = 16000;

// Records every State reported to the application, to check for transient states
void RecordStateChange(const EVSECbInfo * cb, intptr_t arg)
{
    if (cb->type == EVSECallbackType::StateChanged)
    {
        reinterpret_cast<std::vector<StateEnum> *>(arg)->push_back(cb->StateChange.state);
    }
}

// Records the ID of every event logged, to check which events the delegate generates
class RecordingEventReporter : public EventReporter
{
public:
    CHIP_ERROR NewEventGenerated(ConcreteEventPath & aPath, uint32_t aBytesConsumed) override
    {
        mEventIds.push_back(aPath.mEventId);
        return CHIP_NO_ERROR;
    }
    void ScheduleUrgentEventDeliverySync(Optional<FabricIndex> fabricIndex) override {}

    std::vector<EventId> mEventIds;
};

constexpr EventId kStarted = Events::EnergyTransferStarted::Id;
constexpr EventId kStopped = Events::EnergyTransferStopped::Id;

class TestEnergyEvseDelegate : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestEnergyEvseDelegate() :
        mDelegate(mTargetsDelegate),
        mInstance(kEndpointId, mDelegate, BitMask<Feature, uint32_t>(Feature::kChargingPreferences, Feature::kV2x),
                  BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent),
                  EnergyEvseCluster::OptionalCommandSet())
    {}

    void SetUp() override
    {
        const LogStorageResources logStorageResources[] = {
            { mDebugEventBuffer, sizeof(mDebugEventBuffer), PriorityLevel::Debug },
            { mInfoEventBuffer, sizeof(mInfoEventBuffer), PriorityLevel::Info },
            { mCritEventBuffer, sizeof(mCritEventBuffer), PriorityLevel::Critical },
        };
        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);
        ASSERT_EQ(EventManagement::GetInstance().Init(nullptr, MATTER_ARRAY_SIZE(logStorageResources), mCircularEventBuffer,
                                                      logStorageResources, &mEventCounter,
                                                      System::SystemClock().GetMonotonicMilliseconds64(), &mEventReporter),
                  CHIP_NO_ERROR);

        ASSERT_EQ(mProvider.Init(&mStorage), CHIP_NO_ERROR);
        SetSafeAttributePersistenceProvider(&mProvider);

        mDelegate.SetInstance(&mInstance);
        ASSERT_EQ(mDelegate.HwSetCircuitCapacity(kCircuitCapacity_mA), Status::Success);
    }

    void TearDown() override
    {
        mDelegate.CancelActiveTimers();
        mDelegate.SetInstance(nullptr);
        EventManagement::DestroyEventManagement();
    }

protected:
    // The EnergyTransferStarted/Stopped events logged so far, in order
    std::vector<EventId> TransferEvents()
    {
        std::vector<EventId> events;
        for (EventId id : mEventReporter.mEventIds)
        {
            if (id == kStarted || id == kStopped)
            {
                events.push_back(id);
            }
        }
        return events;
    }

    ConcreteAttributePath Path(AttributeId id) { return ConcreteAttributePath(kEndpointId, EnergyEvse::Id, id); }

    std::string PoisonKey(AttributeId id)
    {
        return DefaultStorageKeyAllocator::SafeAttributeValue(kEndpointId, EnergyEvse::Id, id).KeyName();
    }

    template <typename T>
    CHIP_ERROR ReadPersisted(AttributeId id, T & value)
    {
        return mProvider.ReadScalarValue(Path(id), value);
    }

    template <typename T>
    void WritePersisted(AttributeId id, T value)
    {
        ASSERT_EQ(mProvider.WriteScalarValue(Path(id), value), CHIP_NO_ERROR);
    }

    // Plugs in an EV with demand and enables charging indefinitely, leaving the EVSE in kPluggedInCharging
    void StartCharging()
    {
        ASSERT_EQ(mDelegate.HwSetState(StateEnum::kPluggedInDemand), Status::Success);
        ASSERT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, kMaximumChargeCurrent_mA), Status::Success);
        ASSERT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);
    }

    uint8_t mDebugEventBuffer[64];
    uint8_t mInfoEventBuffer[1024];
    uint8_t mCritEventBuffer[256];
    CircularEventBuffer mCircularEventBuffer[3];
    MonotonicallyIncreasingCounter<EventNumber> mEventCounter;
    RecordingEventReporter mEventReporter;

    TestPersistentStorageDelegate mStorage;
    DefaultSafeAttributePersistenceProvider mProvider;
    std::vector<StateEnum> mStateChanges; // declared before mDelegate so it outlives the registered callback
    EvseTargetsDelegate mTargetsDelegate;
    EnergyEvseDelegate mDelegate;
    Instance mInstance;
};

TEST_F(TestEnergyEvseDelegate, EnableDischargingIndefinitelyPersistsEnabledUntil)
{
    DataModel::Nullable<uint32_t> enabledUntil(123u);
    // Nothing has been persisted yet
    EXPECT_NE(ReadPersisted(Attributes::DischargingEnabledUntil::Id, enabledUntil), CHIP_NO_ERROR);

    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);

    // The attribute is already null, so only an explicit write stores the indefinite enable
    enabledUntil = DataModel::Nullable<uint32_t>(123u);
    ASSERT_EQ(ReadPersisted(Attributes::DischargingEnabledUntil::Id, enabledUntil), CHIP_NO_ERROR);
    EXPECT_TRUE(enabledUntil.IsNull());

    int64_t limit = 0;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumDischargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, kMaximumDischargeCurrent_mA);
}

TEST_F(TestEnergyEvseDelegate, DisableClearsPersistedValues)
{
    ASSERT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, kMaximumChargeCurrent_mA), Status::Success);
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);

    ASSERT_EQ(mDelegate.Disable(), Status::Success);

    int64_t limit = -1;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumChargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, 0);
    limit = -1;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumDischargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, 0);

    DataModel::Nullable<uint32_t> enabledUntil;
    ASSERT_EQ(ReadPersisted(Attributes::ChargingEnabledUntil::Id, enabledUntil), CHIP_NO_ERROR);
    ASSERT_FALSE(enabledUntil.IsNull());
    EXPECT_EQ(enabledUntil.Value(), 0u);
    enabledUntil.SetNull();
    ASSERT_EQ(ReadPersisted(Attributes::DischargingEnabledUntil::Id, enabledUntil), CHIP_NO_ERROR);
    ASSERT_FALSE(enabledUntil.IsNull());
    EXPECT_EQ(enabledUntil.Value(), 0u);
}

TEST_F(TestEnergyEvseDelegate, DisableWhenAlreadyDisabledStillClearsRestoredValues)
{
    ASSERT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);

    // Mimic values restored from storage after a reboot past the enabled deadline
    WritePersisted(Attributes::MaximumChargeCurrent::Id, kMaximumChargeCurrent_mA);
    WritePersisted(Attributes::MaximumDischargeCurrent::Id, kMaximumDischargeCurrent_mA);

    ASSERT_EQ(mDelegate.Disable(), Status::Success);

    int64_t limit = -1;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumChargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, 0);
    limit = -1;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumDischargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, 0);
}

TEST_F(TestEnergyEvseDelegate, DisableDuringFaultCompletesAndRecoversDisabled)
{
    StartCharging();
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);
    ASSERT_EQ(mInstance.GetState(), StateEnum::kFault);

    ASSERT_EQ(mDelegate.Disable(), Status::Success);

    // The disable ends the transfer the fault paused
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped }));

    // The disable takes effect once the fault clears
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped }));
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryResumesCharging)
{
    StartCharging();

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kFault);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabledError);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kChargingEnabled);

    // The fault paused the transfer, which resumes without new events
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted }));
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryDoesNotResumeChargingAfterDeadlineExpiredDuringFault)
{
    StartCharging();
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);

    // The deadline passes while the fault is active (no check is scheduled during a fault)
    ASSERT_EQ(mInstance.SetChargingEnabledUntil(DataModel::Nullable<uint32_t>(1u)), CHIP_NO_ERROR);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);

    // The paused transfer does not resume, so it is reported as stopped
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped }));
}

TEST_F(TestEnergyEvseDelegate, EVUnpluggedDuringFaultEndsPausedTransfer)
{
    StartCharging();
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);

    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kNotPluggedIn), Status::Success);
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped }));

    // Nothing is left to stop when the fault clears
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kNotPluggedIn);
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped }));
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryDoesNotStartDischargeThatWasNeverRequested)
{
    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kPluggedInDemand), Status::Success);
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    // Discharging is enabled, but no transfer has been requested so the EVSE stays at demand
    ASSERT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);

    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryForNewSessionStartsNewTransfer)
{
    StartCharging();
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);

    // The EV leaves and another one connects and asks for demand while the fault is still active
    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kNotPluggedIn), Status::Success);
    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kPluggedInDemand), Status::Success);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);
}

TEST_F(TestEnergyEvseDelegate, EnableChargingFailsWhenDeadlineCannotBePersisted)
{
    ASSERT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, kMaximumChargeCurrent_mA), Status::Success);

    mStorage.AddPoisonKey(PoisonKey(Attributes::ChargingEnabledUntil::Id));
    EXPECT_EQ(mDelegate.EnableCharging(DataModel::Nullable<uint32_t>(1000u), kMinimumCurrent_mA, kCircuitCapacity_mA),
              Status::Failure);
    mStorage.ClearPoisonKeys();

    // The new limit must not be left paired with the previous (indefinite) deadline
    int64_t limit = 0;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumChargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, kMaximumChargeCurrent_mA);
    EXPECT_TRUE(mInstance.GetChargingEnabledUntil().IsNull());
}

TEST_F(TestEnergyEvseDelegate, EnableDischargingFailsWhenDeadlineCannotBePersisted)
{
    mStorage.AddPoisonKey(PoisonKey(Attributes::DischargingEnabledUntil::Id));
    EXPECT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Failure);
    mStorage.ClearPoisonKeys();

    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);
}

TEST_F(TestEnergyEvseDelegate, DisableReportsFailureWhenDeadlineCannotBePersisted)
{
    StartCharging();

    mStorage.AddPoisonKey(PoisonKey(Attributes::ChargingEnabledUntil::Id));
    EXPECT_EQ(mDelegate.Disable(), Status::Failure);
    mStorage.ClearPoisonKeys();

    // The disable still takes effect in memory so the EVSE stops charging
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);
    EXPECT_EQ(mInstance.GetMaximumChargeCurrent(), 0);
}

TEST_F(TestEnergyEvseDelegate, ExpiryOfOneDeadlineKeepsTheOtherModeEnabled)
{
    ASSERT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, kMaximumChargeCurrent_mA), Status::Success);
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    ASSERT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kEnabled);

    // The charging deadline is in the past
    ASSERT_EQ(mInstance.SetChargingEnabledUntil(DataModel::Nullable<uint32_t>(1u)), CHIP_NO_ERROR);

    EXPECT_EQ(mDelegate.ScheduleCheckOnEnabledTimeout(), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    ASSERT_FALSE(mInstance.GetChargingEnabledUntil().IsNull());
    EXPECT_EQ(mInstance.GetChargingEnabledUntil().Value(), 0u);
}

TEST_F(TestEnergyEvseDelegate, DisableDuringFaultDoesNotAbortDiagnostics)
{
    ASSERT_EQ(mDelegate.StartDiagnostics(), Status::Success);
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);

    EXPECT_EQ(mDelegate.Disable(), Status::Failure);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryAfterDemandRestartedStartsNewTransfer)
{
    ASSERT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, kMaximumChargeCurrent_mA), Status::Success);
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kPluggedInDemand), Status::Success);
    // DEM has requested a discharge transfer
    ASSERT_EQ(mInstance.SetState(StateEnum::kPluggedInDischarging), CHIP_NO_ERROR);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);
    // The EV stops and then asks for demand again while the fault is active, ending the discharge transfer
    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kPluggedInNoDemand), Status::Success);
    ASSERT_EQ(mDelegate.HwSetState(StateEnum::kPluggedInDemand), Status::Success);

    // The new demand is handled as for any new demand when Enabled: charging starts
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);

    // The EV ended the paused transfer during the fault, and a new transfer starts on recovery
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped, kStarted }));
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryWithExpiredDeadlineHasNoTransientChargingState)
{
    StartCharging();
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);
    ASSERT_EQ(mInstance.SetChargingEnabledUntil(DataModel::Nullable<uint32_t>(1u)), CHIP_NO_ERROR);

    ASSERT_EQ(mDelegate.HwRegisterEvseCallbackHandler(RecordStateChange, reinterpret_cast<intptr_t>(&mStateChanges)),
              Status::Success);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);
    for (StateEnum state : mStateChanges)
    {
        EXPECT_NE(state, StateEnum::kPluggedInCharging);
    }
}

TEST_F(TestEnergyEvseDelegate, ChargingExpiryStopsActiveChargingTransfer)
{
    StartCharging();
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    ASSERT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kEnabled);
    ASSERT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);

    // The charging deadline is in the past
    ASSERT_EQ(mInstance.SetChargingEnabledUntil(DataModel::Nullable<uint32_t>(1u)), CHIP_NO_ERROR);

    EXPECT_EQ(mDelegate.ScheduleCheckOnEnabledTimeout(), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped }));
}

TEST_F(TestEnergyEvseDelegate, DischargingExpiryWithDemandStartsCharging)
{
    StartCharging();
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    // DEM has requested a discharge transfer
    ASSERT_EQ(mInstance.SetState(StateEnum::kPluggedInDischarging), CHIP_NO_ERROR);

    // The discharging deadline is in the past
    ASSERT_EQ(mInstance.SetDischargingEnabledUntil(DataModel::Nullable<uint32_t>(1u)), CHIP_NO_ERROR);

    // The EV is still demanding, so charging starts as for any new demand when ChargingEnabled
    EXPECT_EQ(mDelegate.ScheduleCheckOnEnabledTimeout(), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);

    // The switch to discharging was DEM-controlled (no events); the expiry stops it and starts charging
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped, kStarted }));
}

TEST_F(TestEnergyEvseDelegate, FaultRecoveryAfterDischargingExpiredStartsCharging)
{
    StartCharging();
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    // DEM has requested a discharge transfer
    ASSERT_EQ(mInstance.SetState(StateEnum::kPluggedInDischarging), CHIP_NO_ERROR);

    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kOther), Status::Success);
    // The discharging deadline passes while the fault is active
    ASSERT_EQ(mInstance.SetDischargingEnabledUntil(DataModel::Nullable<uint32_t>(1u)), CHIP_NO_ERROR);

    // Matches the expiry outside a fault: the discharge is not resumed and charging starts instead
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);
    EXPECT_EQ(TransferEvents(), (std::vector<EventId>{ kStarted, kStopped, kStarted }));
}

TEST_F(TestEnergyEvseDelegate, CableAssemblyLimitAppliesToDischarging)
{
    ASSERT_EQ(mDelegate.HwSetMaxHardwareDischargeCurrentLimit(kCircuitCapacity_mA), Status::Success);
    ASSERT_EQ(mDelegate.HwSetCableAssemblyLimit(kCircuitCapacity_mA), Status::Success);
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Success);
    ASSERT_EQ(mInstance.GetMaximumDischargeCurrent(), kMaximumDischargeCurrent_mA);

    ASSERT_EQ(mDelegate.HwSetCableAssemblyLimit(10000), Status::Success);
    EXPECT_EQ(mInstance.GetMaximumDischargeCurrent(), 10000);
}

TEST_F(TestEnergyEvseDelegate, InvalidFaultIsRejectedWithoutChangingState)
{
    StartCharging();

    EXPECT_EQ(mDelegate.HwSetFault(static_cast<FaultStateEnum>(0x20)), Status::ConstraintError);
    EXPECT_EQ(mInstance.GetFaultState(), FaultStateEnum::kNoError);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInCharging);
}

TEST_F(TestEnergyEvseDelegate, UserMaximumChargeCurrentDefaultsImmediatelyWhenCircuitCapacityKnown)
{
    // SetUp already reported the CircuitCapacity, so the default is applied immediately
    ASSERT_EQ(mDelegate.InitializeUserMaximumChargeCurrent(), CHIP_NO_ERROR);
    EXPECT_EQ(mInstance.GetUserMaximumChargeCurrent(), kCircuitCapacity_mA);
}

TEST_F(TestEnergyEvseDelegate, UserMaximumChargeCurrentWriteCancelsPendingDefault)
{
    // Use a fresh delegate so CircuitCapacity has not been reported yet
    mDelegate.SetInstance(nullptr);
    EvseTargetsDelegate targets;
    EnergyEvseDelegate delegate(targets);
    Instance instance(kEndpointId, delegate, BitMask<Feature, uint32_t>(Feature::kChargingPreferences),
                      BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent),
                      EnergyEvseCluster::OptionalCommandSet());
    delegate.SetInstance(&instance);

    ASSERT_EQ(delegate.InitializeUserMaximumChargeCurrent(), CHIP_NO_ERROR);
    ASSERT_EQ(instance.SetUserMaximumChargeCurrent(16000), CHIP_NO_ERROR);

    ASSERT_EQ(delegate.HwSetCircuitCapacity(kCircuitCapacity_mA), Status::Success);
    EXPECT_EQ(instance.GetUserMaximumChargeCurrent(), 16000);

    delegate.CancelActiveTimers();
    delegate.SetInstance(nullptr);
}

TEST_F(TestEnergyEvseDelegate, PendingDefaultAppliedWhenCircuitCapacityArrives)
{
    mDelegate.SetInstance(nullptr);
    EvseTargetsDelegate targets;
    EnergyEvseDelegate delegate(targets);
    Instance instance(kEndpointId, delegate, BitMask<Feature, uint32_t>(Feature::kChargingPreferences),
                      BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent),
                      EnergyEvseCluster::OptionalCommandSet());
    delegate.SetInstance(&instance);

    ASSERT_EQ(delegate.InitializeUserMaximumChargeCurrent(), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.HwSetCircuitCapacity(kCircuitCapacity_mA), Status::Success);
    EXPECT_EQ(instance.GetUserMaximumChargeCurrent(), kCircuitCapacity_mA);

    delegate.CancelActiveTimers();
    delegate.SetInstance(nullptr);
}

class TestEnergyEvseManagerLoad : public TestEnergyEvseDelegate
{
public:
    TestEnergyEvseManagerLoad() :
        mManager(kEndpointId, mManagerDelegate, BitMask<Feature, uint32_t>(Feature::kChargingPreferences, Feature::kV2x),
                 BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent),
                 EnergyEvseCluster::OptionalCommandSet())
    {}

    void SetUp() override
    {
        TestEnergyEvseDelegate::SetUp();
        mManagerDelegate.SetInstance(&mManager);
    }

    void TearDown() override
    {
        mManagerDelegate.CancelActiveTimers();
        mManagerDelegate.SetInstance(nullptr);
        TestEnergyEvseDelegate::TearDown();
    }

protected:
    // Enables charging indefinitely at one limit, then fails the next EnableCharging at the poisoned write
    // (as a power loss there would) and checks that a reboot restores charging as disabled
    void ExpectInterruptedEnableChargingRestoresDisabled(AttributeId poisonedId, int writePattern)
    {
        ASSERT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, 10000), Status::Success);

        mStorage.AddPoisonKey(PoisonKey(poisonedId), -1, writePattern);
        EXPECT_EQ(mDelegate.EnableCharging(DataModel::NullNullable, kMaximumChargeCurrent_mA, kMaximumChargeCurrent_mA),
                  Status::Failure);
        mStorage.ClearPoisonKeys();

        ASSERT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);
        EXPECT_EQ(mManager.GetSupplyState(), SupplyStateEnum::kDisabled);
        EXPECT_EQ(mManager.GetMinimumChargeCurrent(), 0);

        int64_t limit = -1;
        ASSERT_EQ(ReadPersisted(Attributes::MaximumChargeCurrent::Id, limit), CHIP_NO_ERROR);
        EXPECT_EQ(limit, 0);
    }

    EvseTargetsDelegate mManagerTargets;
    EnergyEvseDelegate mManagerDelegate{ mManagerTargets };
    EnergyEvseManager mManager;
};

TEST_F(TestEnergyEvseManagerLoad, LoadsZeroCommandLimitsWrittenByDisable)
{
    // Disable() persists zero for both limits; the next boot must accept them
    WritePersisted(Attributes::MaximumChargeCurrent::Id, int64_t(0));
    WritePersisted(Attributes::MaximumDischargeCurrent::Id, int64_t(0));

    EXPECT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);
}

TEST_F(TestEnergyEvseManagerLoad, LoadsValidCommandLimits)
{
    WritePersisted(Attributes::MaximumChargeCurrent::Id, kMaximumChargeCurrent_mA);
    WritePersisted(Attributes::MaximumDischargeCurrent::Id, kMaximumDischargeCurrent_mA);

    EXPECT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);
}

TEST_F(TestEnergyEvseManagerLoad, RestoredDischargingLimitIsApplied)
{
    // Hardware limits are known before the persisted values are loaded
    ASSERT_EQ(mManagerDelegate.HwSetMaxHardwareDischargeCurrentLimit(kCircuitCapacity_mA), Status::Success);
    ASSERT_EQ(mManagerDelegate.HwSetCircuitCapacity(kCircuitCapacity_mA), Status::Success);
    ASSERT_EQ(mManagerDelegate.HwSetCableAssemblyLimit(kCircuitCapacity_mA), Status::Success);

    WritePersisted(Attributes::DischargingEnabledUntil::Id, DataModel::Nullable<uint32_t>());
    WritePersisted(Attributes::MaximumDischargeCurrent::Id, kMaximumDischargeCurrent_mA);

    ASSERT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);
    EXPECT_EQ(mManager.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(mManager.GetMaximumDischargeCurrent(), kMaximumDischargeCurrent_mA);
}

TEST_F(TestEnergyEvseManagerLoad, EnableChargingInterruptedAtMinimumCurrentRestoresDisabled)
{
    ExpectInterruptedEnableChargingRestoresDisabled(Attributes::MinimumChargeCurrent::Id, 0);
}

TEST_F(TestEnergyEvseManagerLoad, EnableChargingInterruptedAtMaximumCurrentRestoresDisabled)
{
    ExpectInterruptedEnableChargingRestoresDisabled(Attributes::MaximumChargeCurrent::Id, 0);
}

TEST_F(TestEnergyEvseManagerLoad, EnableChargingInterruptedAtDeadlineRestoresDisabled)
{
    // The first write (the disabled marker) succeeds and the final deadline write fails
    ExpectInterruptedEnableChargingRestoresDisabled(Attributes::ChargingEnabledUntil::Id, 0b1);
}

TEST_F(TestEnergyEvseManagerLoad, EnableDischargingInterruptedAtDeadlineRestoresDisabled)
{
    ASSERT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, 10000), Status::Success);

    mStorage.AddPoisonKey(PoisonKey(Attributes::DischargingEnabledUntil::Id), -1, 0b1);
    EXPECT_EQ(mDelegate.EnableDischarging(DataModel::NullNullable, kMaximumDischargeCurrent_mA), Status::Failure);
    mStorage.ClearPoisonKeys();

    ASSERT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);
    EXPECT_EQ(mManager.GetSupplyState(), SupplyStateEnum::kDisabled);

    int64_t limit = -1;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumDischargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, 0);
}

TEST_F(TestEnergyEvseManagerLoad, FactoryResetDefaultsUserMaximumChargeCurrentAndStoresIt)
{
    // Factory reset: nothing stored, and (as in the example app) CircuitCapacity is reported after loading
    ASSERT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);
    EXPECT_EQ(mManager.GetUserMaximumChargeCurrent(), 0);
    int64_t stored = -1;
    EXPECT_NE(ReadPersisted(Attributes::UserMaximumChargeCurrent::Id, stored), CHIP_NO_ERROR);

    // The pending default is applied once CircuitCapacity is known, and written to storage
    ASSERT_EQ(mManagerDelegate.HwSetCircuitCapacity(kCircuitCapacity_mA), Status::Success);
    EXPECT_EQ(mManager.GetUserMaximumChargeCurrent(), kCircuitCapacity_mA);
    ASSERT_EQ(ReadPersisted(Attributes::UserMaximumChargeCurrent::Id, stored), CHIP_NO_ERROR);
    EXPECT_EQ(stored, kCircuitCapacity_mA);

    // A later CircuitCapacity change does not re-apply the default
    ASSERT_EQ(mManagerDelegate.HwSetCircuitCapacity(16000), Status::Success);
    EXPECT_EQ(mManager.GetUserMaximumChargeCurrent(), kCircuitCapacity_mA);
}

TEST_F(TestEnergyEvseManagerLoad, ExpiredChargingDeadlineClearsRestoredChargingValuesOnly)
{
    // Deadline 1 is long past (and 0 is an explicit disable); discharging stays enabled indefinitely
    WritePersisted(Attributes::ChargingEnabledUntil::Id, DataModel::Nullable<uint32_t>(1u));
    WritePersisted(Attributes::MinimumChargeCurrent::Id, kMinimumCurrent_mA);
    WritePersisted(Attributes::MaximumChargeCurrent::Id, kMaximumChargeCurrent_mA);
    WritePersisted(Attributes::DischargingEnabledUntil::Id, DataModel::Nullable<uint32_t>());
    WritePersisted(Attributes::MaximumDischargeCurrent::Id, kMaximumDischargeCurrent_mA);

    ASSERT_EQ(mManager.LoadPersistentValues(), CHIP_NO_ERROR);

    EXPECT_EQ(mManager.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(mManager.GetMinimumChargeCurrent(), 0);

    DataModel::Nullable<uint32_t> enabledUntil;
    ASSERT_EQ(ReadPersisted(Attributes::ChargingEnabledUntil::Id, enabledUntil), CHIP_NO_ERROR);
    ASSERT_FALSE(enabledUntil.IsNull());
    EXPECT_EQ(enabledUntil.Value(), 0u);

    int64_t limit = -1;
    ASSERT_EQ(ReadPersisted(Attributes::MaximumChargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, 0);
    ASSERT_EQ(ReadPersisted(Attributes::MaximumDischargeCurrent::Id, limit), CHIP_NO_ERROR);
    EXPECT_EQ(limit, kMaximumDischargeCurrent_mA);
}

TEST_F(TestEnergyEvseManagerLoad, IndefiniteChargingEnableSurvivesReload)
{
    ASSERT_EQ(mManagerDelegate.EnableCharging(DataModel::NullNullable, kMinimumCurrent_mA, kMaximumChargeCurrent_mA),
              Status::Success);

    // Simulate a reboot with a new delegate and manager reading the same storage
    EvseTargetsDelegate targets;
    EnergyEvseDelegate delegate(targets);
    EnergyEvseManager manager(kEndpointId, delegate, BitMask<Feature, uint32_t>(Feature::kChargingPreferences, Feature::kV2x),
                              BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent),
                              EnergyEvseCluster::OptionalCommandSet());
    delegate.SetInstance(&manager);

    ASSERT_EQ(manager.LoadPersistentValues(), CHIP_NO_ERROR);
    EXPECT_EQ(manager.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_TRUE(manager.GetChargingEnabledUntil().IsNull());

    delegate.CancelActiveTimers();
    delegate.SetInstance(nullptr);
}

TEST_F(TestEnergyEvseManagerLoad, RejectsNegativeDischargingLimit)
{
    WritePersisted(Attributes::MaximumDischargeCurrent::Id, int64_t(-1));

    EXPECT_EQ(mManager.LoadPersistentValues(), CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
