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

#pragma once

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementCluster.h>
#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <app/clusters/on-off-server/OnOffLightingCluster.h>
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/// A plug-in unit that reports its own power use: one endpoint with both the
/// On/Off Plug-in Unit (0x010A) and Electrical Sensor (0x0510) device types.
///
/// Readings follow the OnOff state: 0 W when off, kOnPower_mW when on.
/// CumulativeEnergyImported grows with on-time and is published every
/// kEnergySnapshotInterval and on every OnOff change.
class OnOffPlugWithSensors : private Clusters::OnOffDelegate,
                             private Clusters::OnOffEffectDelegate,
                             private Clusters::ElectricalEnergyMeasurement::Delegate,
                             private Clusters::ElectricalPowerMeasurement::Delegate,
                             private Clusters::PowerTopology::Delegate,
                             private TimerContext,
                             public SingleEndpoint
{
public:
    using EnergyMeasurementClusterT = Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster;
    using PowerMeasurementClusterT  = Clusters::ElectricalPowerMeasurement::ElectricalPowerMeasurementCluster;
    using PowerTopologyClusterT     = Clusters::PowerTopology::PowerTopologyCluster;

    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
        Clusters::IdentifyDelegate & identifyDelegate;
        Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> tagList = {};
    };

    static constexpr int64_t kOnPower_mW   = 60'000;
    static constexpr int64_t kVoltage_mV   = 230'000;
    static constexpr int64_t kOnCurrent_mA = kOnPower_mW * 1000 / kVoltage_mV;

    static constexpr System::Clock::Seconds32 kEnergySnapshotInterval{ 10 };

    explicit OnOffPlugWithSensors(const Context & context);
    ~OnOffPlugWithSensors() override { mContext.timerDelegate.CancelTimer(this); }

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::GroupsCluster & GroupsCluster() { return mGroupsCluster.Cluster(); }
    Clusters::ScenesManagementCluster & ScenesManagementCluster() { return mScenesManagementCluster.Cluster(); }
    Clusters::OnOffLightingCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    EnergyMeasurementClusterT & ElectricalEnergyMeasurementCluster() { return mEnergyMeasurementCluster.Cluster(); }
    PowerMeasurementClusterT & ElectricalPowerMeasurementCluster() { return mPowerMeasurementCluster.Cluster(); }
    PowerTopologyClusterT & PowerTopologyCluster() { return mPowerTopologyCluster.Cluster(); }

private:
    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // OnOffEffectDelegate
    DataModel::ActionReturnStatus TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum variant) override;
    DataModel::ActionReturnStatus TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum variant) override;

    // ElectricalEnergyMeasurement::Delegate
    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override;
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override { return DataModel::NullNullable; }

    // ElectricalPowerMeasurement::Delegate
    Clusters::ElectricalPowerMeasurement::PowerModeEnum GetPowerMode() override
    {
        return Clusters::ElectricalPowerMeasurement::PowerModeEnum::kAc;
    }
    uint8_t GetNumberOfMeasurementTypes() override;
    CHIP_ERROR StartAccuracyRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetAccuracyByIndex(uint8_t index,
                                  Clusters::ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & value) override;
    CHIP_ERROR EndAccuracyRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StartRangesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetRangeByIndex(uint8_t, Clusters::ElectricalPowerMeasurement::Structs::MeasurementRangeStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndRangesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StartHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t,
                                          Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StartHarmonicPhasesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t,
                                        Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndHarmonicPhasesRead() override { return CHIP_NO_ERROR; }
    DataModel::Nullable<int64_t> GetVoltage() override { return DataModel::MakeNullable(kVoltage_mV); }
    DataModel::Nullable<int64_t> GetActiveCurrent() override { return DataModel::MakeNullable(mIsOn ? kOnCurrent_mA : 0); }
    DataModel::Nullable<int64_t> GetActivePower() override { return DataModel::MakeNullable(mIsOn ? kOnPower_mW : 0); }
    DataModel::Nullable<int64_t> GetReactiveCurrent() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetApparentCurrent() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetReactivePower() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetApparentPower() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetRMSVoltage() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetRMSCurrent() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetRMSPower() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetFrequency() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetPowerFactor() override { return DataModel::NullNullable; }
    DataModel::Nullable<int64_t> GetNeutralCurrent() override { return DataModel::NullNullable; }

    // PowerTopology::Delegate (NodeTopology feature: no endpoint lists)
    CHIP_ERROR GetAvailableEndpointAtIndex(size_t, EndpointId &) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }
    CHIP_ERROR GetActiveEndpointAtIndex(size_t, EndpointId &) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }

    // TimerContext
    void TimerFired() override;

    /// Adds the energy used since the last call to mEnergyImported_mWms.
    void AccumulateEnergy();

    class DefaultScenesManagementTableProvider : public Clusters::ScenesManagementTableProvider
    {
    public:
        Clusters::ScenesManagementSceneTable * Take() override
        {
            return scenes::GetSceneTableImpl(mEndpointId, scenes::kMaxScenesPerEndpoint);
        }
        void Release(Clusters::ScenesManagementSceneTable *) override {}

        void SetEndpoint(EndpointId endpoint) { mEndpointId = endpoint; }

    private:
        EndpointId mEndpointId = kInvalidEndpointId;
    };

    const Context mContext;

    bool mIsOn = false;
    System::Clock::Timestamp mLastEnergyUpdate{ 0 };
    // Milliwatt-milliseconds, so short on-periods are not lost to rounding before conversion to mWh.
    int64_t mEnergyImported_mWms = 0;

    DefaultScenesManagementTableProvider mScenesTableProvider;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::OnOffLightingCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
    LazyRegisteredServerCluster<EnergyMeasurementClusterT> mEnergyMeasurementCluster;
    LazyRegisteredServerCluster<PowerMeasurementClusterT> mPowerMeasurementCluster;
    LazyRegisteredServerCluster<PowerTopologyClusterT> mPowerTopologyCluster;
};

} // namespace chip::app
