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

namespace chip::app {

constexpr Clusters::Thermostat::temperature kInitialTemperature = 2000;
constexpr Clusters::Thermostat::temperature kFinalTemperature   = 3000;

class SimulatedWaterHeater
    : public WaterHeater<Clusters::Thermostat::Delegate, Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate>,
      public Clusters::WaterHeaterManagement::Delegate,
      public Clusters::Thermostat::Delegate,
      public Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate,
      public TimerContext,
      public Clusters::ModeBase::AppDelegate
{
public:
    explicit SimulatedWaterHeater(const Config & config);
    ~SimulatedWaterHeater() override;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

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

    // Clusters::ModeBase::AppDelegate
    CHIP_ERROR Init() override;
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    void EndBoost();
    void NotifyHeatDemandAndBoostStateChanged();

    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeaterTypes{
        Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap::kImmersionElement1
    };
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeatDemand;
    Clusters::WaterHeaterManagement::BoostStateEnum mBoostState = Clusters::WaterHeaterManagement::BoostStateEnum::kInactive;
    uint32_t mBoostRemainingTime                                = 0;
    Clusters::Thermostat::temperature mTemperature              = kInitialTemperature;
    bool mHeatingEnabled                                        = false;

    // Thermostat attributes
    Clusters::Thermostat::ControlSequenceOfOperationEnum mControlSequenceOfOperation =
        Clusters::Thermostat::ControlSequenceOfOperationEnum::kCoolingAndHeating;
    Clusters::Thermostat::SystemModeEnum mSystemMode                         = Clusters::Thermostat::SystemModeEnum::kOff;
    DataModel::Nullable<Clusters::Thermostat::temperature> mLocalTemperature = DataModel::Nullable<int16_t>();
    Clusters::Thermostat::temperature mOccupiedHeatingSetpoint               = 2000;
};

} // namespace chip::app
