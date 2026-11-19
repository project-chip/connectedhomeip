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

#include <app/clusters/thermostat-server/ThermostatClusterHeatingSetpoints.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <device/types/water-heater/WaterHeater.h>
#include <lib/support/TimerDelegate.h>

#include <optional>

namespace chip::app {

/**
 * Self-contained simulated Water Heater using the self-delegate pattern.
 *
 * The delegate interfaces are inherited privately so that external code interacts with the device through the
 * clusters (e.g. `ThermostatCluster().SetSystemMode(...)`) instead of calling delegate callbacks directly, which
 * would bypass cluster-side validation and attribute reporting.
 *
 * The delegate bases are listed before `WaterHeater` on purpose: base classes are constructed in declaration order
 * and destroyed in reverse order, so the delegate subobjects outlive the clusters owned by `WaterHeater` that hold
 * references to them.
 *
 * Note: both `SingleEndpoint` and `WaterHeaterManagement::Delegate` declare a protected `mEndpointId`, so an
 * unqualified `mEndpointId` is ambiguous in this class. Use `GetEndpointId()` (from `SingleEndpoint`) instead.
 */
class SimulatedWaterHeater : private Clusters::WaterHeaterManagement::Delegate,
                             private Clusters::Thermostat::Delegate,
                             private Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate,
                             private Clusters::ModeBase::AppDelegate,
                             private TimerContext,
                             public WaterHeater
{
public:
    // Temperatures are in units of 0.01 °C.
    static constexpr Clusters::Thermostat::temperature kMinTemperature         = 0;
    static constexpr Clusters::Thermostat::temperature kAmbientTemperature     = 2000;
    static constexpr Clusters::Thermostat::temperature kDefaultHeatingSetpoint = 3000;
    static constexpr Clusters::Thermostat::temperature kMaxTemperature         = 10000;

    SimulatedWaterHeater(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                         DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider);
    ~SimulatedWaterHeater() override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    // TimerContext
    void TimerFired() override;

    // Clusters::WaterHeaterManagement::Delegate
    Protocols::InteractionModel::Status HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                                    Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                                    Optional<Percent> targetReheat) override;
    Protocols::InteractionModel::Status HandleCancelBoost() override;
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> GetHeaterTypes() override;
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> GetHeatDemand() override;
    uint16_t GetTankVolume() override;
    Energy_mWh GetEstimatedHeatRequired() override;
    Percent GetTankPercentage() override;
    Clusters::WaterHeaterManagement::BoostStateEnum GetBoostState() override;

    // Clusters::Thermostat::Delegate
    FabricTable & GetFabricTable() const override;
    Clusters::Thermostat::SystemModeEnum GetSystemMode() const override;
    Protocols::InteractionModel::Status SetSystemMode(Clusters::Thermostat::SystemModeEnum systemMode, bool & changed) override;
    Protocols::InteractionModel::Status
    GetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum & runningMode) const override;
    Protocols::InteractionModel::Status SetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum runningMode,
                                                       bool & changed) override;
    Protocols::InteractionModel::Status
    GetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap> & runningState) const override;
    Protocols::InteractionModel::Status SetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap> runningState,
                                                        bool & changed) override;
    Clusters::Thermostat::ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(Clusters::Thermostat::ControlSequenceOfOperationEnum seq,
                                                                      bool & changed) override;
    DataModel::Nullable<Clusters::Thermostat::temperature> GetLocalTemperature() const override;
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<Clusters::Thermostat::temperature> temp,
                                                            bool & changed) override;
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<Clusters::Thermostat::RemoteSensingBitmap> sensing,
                                                         bool & changed) override;

    // Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate
    Protocols::InteractionModel::Status
    GetOccupiedHeatingSetpoint(Clusters::Thermostat::temperature & occupiedHeatingSetpoint) const override;
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(Clusters::Thermostat::temperature occupiedHeatingSetpoint,
                                                                   bool & changed) override;
    Protocols::InteractionModel::Status
    GetAbsMinHeatSetpointLimit(Clusters::Thermostat::temperature & absMinHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status
    GetAbsMaxHeatSetpointLimit(Clusters::Thermostat::temperature & absMaxHeatSetpointLimit) const override;

    // Shared override of Clusters::Thermostat::Delegate and Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate.
    // Both base classes declare the same virtual signature, so a single override implements both. ThermostatCluster
    // calls Startup() once per delegate interface (i.e. twice on this object); the implementation is idempotent and
    // only the first call after a Shutdown() performs initialization.
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    // Clusters::ModeBase::AppDelegate
    CHIP_ERROR Init() override;
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    void EndBoost();
    void SetHeatingEnabled(bool enabled);
    void EvaluateHeatingDemand(std::optional<uint8_t> pendingMode = std::nullopt);
    bool IsModeOff(std::optional<uint8_t> mode = std::nullopt);
    bool IsNormalHeatingPermitted(std::optional<uint8_t> mode = std::nullopt);
    Clusters::Thermostat::temperature GetHeatingTargetTemperature() const;
    void NotifyHeatDemandChanged();
    void NotifyBoostStateChanged();

    FabricTable & mFabricTable;
    AttributePersistenceProvider * mAttributeStorage = nullptr;

    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeatDemand;
    Clusters::WaterHeaterManagement::BoostStateEnum mBoostState = Clusters::WaterHeaterManagement::BoostStateEnum::kInactive;
    uint32_t mBoostRemainingTime                                = 0;
    std::optional<Clusters::Thermostat::temperature> mBoostTemporarySetpoint;
    bool mBoostOneShot                 = false;
    bool mHeatingEnabled               = false;
    bool mIsSyncingMode                = false;
    bool mStartupReconciliationPending = false;

    // Thermostat attributes
    Clusters::Thermostat::SystemModeEnum mSystemMode                         = Clusters::Thermostat::SystemModeEnum::kOff;
    DataModel::Nullable<Clusters::Thermostat::temperature> mLocalTemperature = DataModel::MakeNullable(kAmbientTemperature);
    Clusters::Thermostat::temperature mOccupiedHeatingSetpoint               = kDefaultHeatingSetpoint;
};

} // namespace chip::app
