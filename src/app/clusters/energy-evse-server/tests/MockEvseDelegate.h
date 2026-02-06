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

#pragma once

#include <app/clusters/energy-evse-server/EnergyEvseDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class MockEvseDelegate : public Delegate
{
public:
    // Mock test values - used for verification in tests
    static constexpr StateEnum kMockState                   = StateEnum::kPluggedInNoDemand;
    static constexpr SupplyStateEnum kMockSupplyState       = SupplyStateEnum::kChargingEnabled;
    static constexpr FaultStateEnum kMockFaultState         = FaultStateEnum::kNoError;
    static constexpr uint32_t kMockChargingEnabledUntil     = 1000;
    static constexpr uint32_t kMockDischargingEnabledUntil  = 2000;
    static constexpr int64_t kMockCircuitCapacity           = 32000;    // 32A in mA
    static constexpr int64_t kMockMinimumChargeCurrent      = 6000;     // 6A in mA
    static constexpr int64_t kMockMaximumChargeCurrent      = 32000;    // 32A in mA
    static constexpr int64_t kMockMaximumDischargeCurrent   = 16000;    // 16A in mA (V2x)
    static constexpr int64_t kMockUserMaximumChargeCurrent  = 24000;    // 24A in mA
    static constexpr uint32_t kMockRandomizationDelayWindow = 600;      // 10 minutes
    static constexpr uint32_t kMockNextChargeStartTime      = 3600;     // 1 hour from now
    static constexpr uint32_t kMockNextChargeTargetTime     = 7200;     // 2 hours from now
    static constexpr int64_t kMockNextChargeRequiredEnergy  = 20000000; // 20 kWh in mWh
    static constexpr Percent kMockNextChargeTargetSoC       = 80;
    static constexpr uint16_t kMockApproximateEVEfficiency  = 150; // 150 Wh/km
    static constexpr Percent kMockStateOfCharge             = 45;
    static constexpr int64_t kMockBatteryCapacity           = 60000000; // 60 kWh in mWh
    static constexpr uint32_t kMockSessionID                = 12345;
    static constexpr uint32_t kMockSessionDuration          = 1800;    // 30 minutes
    static constexpr int64_t kMockSessionEnergyCharged      = 5000000; // 5 kWh in mWh
    static constexpr int64_t kMockSessionEnergyDischarged   = 1000000; // 1 kWh in mWh

    MockEvseDelegate()  = default;
    ~MockEvseDelegate() = default;

    // ------------------------------------------------------------------
    // Command handlers (implementations in MockEvseDelegate.cpp)
    Protocols::InteractionModel::Status Disable() override;
    Protocols::InteractionModel::Status EnableCharging(const DataModel::Nullable<uint32_t> & enableChargeTime,
                                                       const int64_t & minimumChargeCurrent,
                                                       const int64_t & maximumChargeCurrent) override;
    Protocols::InteractionModel::Status EnableDischarging(const DataModel::Nullable<uint32_t> & enableDischargeTime,
                                                          const int64_t & maximumDischargeCurrent) override;
    Protocols::InteractionModel::Status StartDiagnostics() override;
    Protocols::InteractionModel::Status SetTargets(
        const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules) override;
    Protocols::InteractionModel::Status LoadTargets() override;
    Protocols::InteractionModel::Status
    GetTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules) override;
    Protocols::InteractionModel::Status ClearTargets() override;

    // ------------------------------------------------------------------
    // Attribute change callbacks (from Delegate interface)
    void OnStateChanged(StateEnum newValue) override { mState = newValue; }
    void OnSupplyStateChanged(SupplyStateEnum newValue) override { mSupplyState = newValue; }
    void OnFaultStateChanged(FaultStateEnum newValue) override { mFaultState = newValue; }
    void OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) override { mChargingEnabledUntil = newValue; }
    void OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) override { mDischargingEnabledUntil = newValue; }
    void OnCircuitCapacityChanged(int64_t newValue) override { mCircuitCapacity = newValue; }
    void OnMinimumChargeCurrentChanged(int64_t newValue) override { mMinimumChargeCurrent = newValue; }
    void OnMaximumChargeCurrentChanged(int64_t newValue) override { mMaximumChargeCurrent = newValue; }
    void OnMaximumDischargeCurrentChanged(int64_t newValue) override { mMaximumDischargeCurrent = newValue; }
    void OnUserMaximumChargeCurrentChanged(int64_t newValue) override { mUserMaximumChargeCurrent = newValue; }
    void OnRandomizationDelayWindowChanged(uint32_t newValue) override { mRandomizationDelayWindow = newValue; }
    void OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t> newValue) override { mNextChargeStartTime = newValue; }
    void OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t> newValue) override { mNextChargeTargetTime = newValue; }
    void OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t> newValue) override { mNextChargeRequiredEnergy = newValue; }
    void OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent> newValue) override { mNextChargeTargetSoC = newValue; }
    void OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t> newValue) override { mApproximateEVEfficiency = newValue; }
    void OnStateOfChargeChanged(DataModel::Nullable<Percent> newValue) override { mStateOfCharge = newValue; }
    void OnBatteryCapacityChanged(DataModel::Nullable<int64_t> newValue) override { mBatteryCapacity = newValue; }
    void OnVehicleIDChanged(DataModel::Nullable<CharSpan> newValue) override { mVehicleID = newValue; }
    void OnSessionIDChanged(DataModel::Nullable<uint32_t> newValue) override { mSessionID = newValue; }
    void OnSessionDurationChanged(DataModel::Nullable<uint32_t> newValue) override { mSessionDuration = newValue; }
    void OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t> newValue) override { mSessionEnergyCharged = newValue; }
    void OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t> newValue) override { mSessionEnergyDischarged = newValue; }

    // ------------------------------------------------------------------
    // Test helper getters (for accessing mock values - not for override)
    StateEnum GetState() const { return mState; }
    SupplyStateEnum GetSupplyState() const { return mSupplyState; }
    FaultStateEnum GetFaultState() const { return mFaultState; }
    DataModel::Nullable<uint32_t> GetChargingEnabledUntil() const { return mChargingEnabledUntil; }
    DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() const { return mDischargingEnabledUntil; }
    int64_t GetCircuitCapacity() const { return mCircuitCapacity; }
    int64_t GetMinimumChargeCurrent() const { return mMinimumChargeCurrent; }
    int64_t GetMaximumChargeCurrent() const { return mMaximumChargeCurrent; }
    int64_t GetMaximumDischargeCurrent() const { return mMaximumDischargeCurrent; }
    int64_t GetUserMaximumChargeCurrent() const { return mUserMaximumChargeCurrent; }
    uint32_t GetRandomizationDelayWindow() const { return mRandomizationDelayWindow; }
    DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() const { return mApproximateEVEfficiency; }
    DataModel::Nullable<Percent> GetStateOfCharge() const { return mStateOfCharge; }
    DataModel::Nullable<int64_t> GetBatteryCapacity() const { return mBatteryCapacity; }

    // ------------------------------------------------------------------
    // Test helper setters (for setting up test state - not for override)
    void SetFaultState(FaultStateEnum value) { mFaultState = value; }
    void SetSupplyState(SupplyStateEnum value) { mSupplyState = value; }

    // ------------------------------------------------------------------
    // Test helper getters (for verifying targets state)
    size_t GetTotalTargetsCount() const { return mTotalTargetsCount; }
    uint8_t GetDaysWithTargets() const { return mDaysWithTargets; }

    // Maximum targets per day and total (for mock purposes)
    static constexpr size_t kMaxTargetsPerDay = 10;
    static constexpr size_t kMaxTotalTargets  = 70; // 7 days * 10 targets

private:
    // Member variables with default mock values
    StateEnum mState                                       = kMockState;
    SupplyStateEnum mSupplyState                           = kMockSupplyState;
    FaultStateEnum mFaultState                             = kMockFaultState;
    DataModel::Nullable<uint32_t> mChargingEnabledUntil    = DataModel::Nullable<uint32_t>(kMockChargingEnabledUntil);
    DataModel::Nullable<uint32_t> mDischargingEnabledUntil = DataModel::Nullable<uint32_t>(kMockDischargingEnabledUntil);
    int64_t mCircuitCapacity                               = kMockCircuitCapacity;
    int64_t mMinimumChargeCurrent                          = kMockMinimumChargeCurrent;
    int64_t mMaximumChargeCurrent                          = kMockMaximumChargeCurrent;
    int64_t mMaximumDischargeCurrent                       = kMockMaximumDischargeCurrent;
    int64_t mUserMaximumChargeCurrent                      = kMockUserMaximumChargeCurrent;
    uint32_t mRandomizationDelayWindow                     = kMockRandomizationDelayWindow;
    DataModel::Nullable<uint32_t> mNextChargeStartTime     = DataModel::Nullable<uint32_t>(kMockNextChargeStartTime);
    DataModel::Nullable<uint32_t> mNextChargeTargetTime    = DataModel::Nullable<uint32_t>(kMockNextChargeTargetTime);
    DataModel::Nullable<int64_t> mNextChargeRequiredEnergy = DataModel::Nullable<int64_t>(kMockNextChargeRequiredEnergy);
    DataModel::Nullable<Percent> mNextChargeTargetSoC      = DataModel::Nullable<Percent>(kMockNextChargeTargetSoC);
    DataModel::Nullable<uint16_t> mApproximateEVEfficiency = DataModel::Nullable<uint16_t>(kMockApproximateEVEfficiency);
    DataModel::Nullable<Percent> mStateOfCharge            = DataModel::Nullable<Percent>(kMockStateOfCharge);
    DataModel::Nullable<int64_t> mBatteryCapacity          = DataModel::Nullable<int64_t>(kMockBatteryCapacity);
    static constexpr size_t kVehicleIdBufferSize           = 32;
    char mVehicleIdBuffer[kVehicleIdBufferSize]            = "MOCK-VIN-12345";
    DataModel::Nullable<CharSpan> mVehicleID           = DataModel::Nullable<CharSpan>(CharSpan::fromCharString(mVehicleIdBuffer));
    DataModel::Nullable<uint32_t> mSessionID           = DataModel::Nullable<uint32_t>(kMockSessionID);
    DataModel::Nullable<uint32_t> mSessionDuration     = DataModel::Nullable<uint32_t>(kMockSessionDuration);
    DataModel::Nullable<int64_t> mSessionEnergyCharged = DataModel::Nullable<int64_t>(kMockSessionEnergyCharged);
    DataModel::Nullable<int64_t> mSessionEnergyDischarged = DataModel::Nullable<int64_t>(kMockSessionEnergyDischarged);

    // Targets storage
    size_t mTotalTargetsCount = 0;
    uint8_t mDaysWithTargets  = 0;                                      // Bitmask of days that have targets
    Structs::ChargingTargetStruct::Type mTargets[7][kMaxTargetsPerDay]; // 7 days, up to kMaxTargetsPerDay each
    size_t mTargetsPerDay[7] = { 0 };
    Structs::ChargingTargetScheduleStruct::Type mSchedules[7]; // For GetTargets response
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
