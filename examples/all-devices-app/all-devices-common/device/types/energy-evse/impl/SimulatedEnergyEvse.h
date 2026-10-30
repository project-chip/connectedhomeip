/*
 *
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

#pragma once

#include <app/clusters/energy-evse-server/Constants.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/clusters/energy-evse-server/EnergyEvseDelegate.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <device/types/energy-evse/EnergyEvse.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class SimulatedEnergyEvse : private Clusters::EnergyEvse::Delegate,
                            private Clusters::ModeBase::AppDelegate,
                            private Clusters::IdentifyDelegate,
                            public EnergyEvse
{
public:
    using EnergyEvse::GetEndpointId;

    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider();
    };

    explicit SimulatedEnergyEvse(const Context & context);
    ~SimulatedEnergyEvse() override = default;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    // Clusters::EnergyEvse::Delegate
    Protocols::InteractionModel::Status Disable() override;
    Protocols::InteractionModel::Status EnableCharging(const DataModel::Nullable<uint32_t> & enableChargeTime,
                                                       const int64_t & minimumChargeCurrent,
                                                       const int64_t & maximumChargeCurrent) override;
    Protocols::InteractionModel::Status EnableDischarging(const DataModel::Nullable<uint32_t> & enableDischargeTime,
                                                          const int64_t & maximumDischargeCurrent) override;
    Protocols::InteractionModel::Status StartDiagnostics() override;
    Protocols::InteractionModel::Status
    SetTargets(const DataModel::DecodableList<Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::DecodableType> &
                   chargingTargetSchedules) override;
    Protocols::InteractionModel::Status LoadTargets() override;
    Protocols::InteractionModel::Status
    GetTargets(DataModel::List<const Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> &
                   chargingTargetSchedules) override;
    Protocols::InteractionModel::Status ClearTargets() override;

    void OnStateChanged(Clusters::EnergyEvse::StateEnum newValue) override;
    void OnSupplyStateChanged(Clusters::EnergyEvse::SupplyStateEnum newValue) override;
    void OnFaultStateChanged(Clusters::EnergyEvse::FaultStateEnum newValue) override;
    void OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnCircuitCapacityChanged(int64_t newValue) override;
    void OnMinimumChargeCurrentChanged(int64_t newValue) override;
    void OnMaximumChargeCurrentChanged(int64_t newValue) override;
    void OnMaximumDischargeCurrentChanged(int64_t newValue) override;
    void OnUserMaximumChargeCurrentChanged(int64_t newValue) override;
    void OnRandomizationDelayWindowChanged(uint32_t newValue) override;
    void OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t> newValue) override;
    void OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent> newValue) override;
    void OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t> newValue) override;
    void OnStateOfChargeChanged(DataModel::Nullable<Percent> newValue) override;
    void OnBatteryCapacityChanged(DataModel::Nullable<int64_t> newValue) override;
    void OnVehicleIDChanged(DataModel::Nullable<CharSpan> newValue) override;
    void OnSessionIDChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnSessionDurationChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t> newValue) override;
    void OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t> newValue) override;

    // Clusters::ModeBase::AppDelegate
    CHIP_ERROR Init() override;
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & tags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    // Clusters::IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;

    // Targets storage
    static constexpr size_t kMaxTargetsPerDay = 10;
    size_t mTotalTargetsCount                 = 0;
    uint8_t mDaysWithTargets                  = 0;
    Clusters::EnergyEvse::Structs::ChargingTargetStruct::Type mTargets[7][kMaxTargetsPerDay];
    size_t mTargetsPerDay[7]                  = { 0 };
    Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::Type mSchedules[7];
};

} // namespace chip::app
