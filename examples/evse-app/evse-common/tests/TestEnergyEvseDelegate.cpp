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
#include <app/SafeAttributePersistenceProvider.h>
#include <gtest/gtest.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>

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
        mOldProvider = GetSafeAttributePersistenceProvider();
        ASSERT_EQ(mProvider.Init(&mStorage), CHIP_NO_ERROR);
        SetSafeAttributePersistenceProvider(&mProvider);

        mDelegate.SetInstance(&mInstance);
        ASSERT_EQ(mDelegate.HwSetCircuitCapacity(kCircuitCapacity_mA), Status::Success);
    }

    void TearDown() override
    {
        mDelegate.CancelActiveTimers();
        mDelegate.SetInstance(nullptr);
        SetSafeAttributePersistenceProvider(mOldProvider);
    }

protected:
    ConcreteAttributePath Path(AttributeId id) { return ConcreteAttributePath(kEndpointId, EnergyEvse::Id, id); }

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

    TestPersistentStorageDelegate mStorage;
    DefaultSafeAttributePersistenceProvider mProvider;
    SafeAttributePersistenceProvider * mOldProvider = nullptr;
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

    // The disable takes effect once the fault clears
    ASSERT_EQ(mDelegate.HwSetFault(FaultStateEnum::kNoError), Status::Success);
    EXPECT_EQ(mInstance.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(mInstance.GetState(), StateEnum::kPluggedInDemand);
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
