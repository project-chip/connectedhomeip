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

 #include <device/types/water-heater/ThermostatDelegate.h>
 #include <device/types/water-heater/ThermostatSetpointsDelegate.h>
 #include <device/types/water-heater/WaterHeater.h>
 #include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
 #include <lib/support/TimerDelegate.h>

 namespace chip::app {

 constexpr Clusters::Thermostat::temperature kInitialTemperature = 2000;
 constexpr Clusters::Thermostat::temperature kFinalTemperature   = 3000;

 struct SimulatedWaterHeaterDelegates
 {
     explicit SimulatedWaterHeaterDelegates(FabricTable & fabricTable) : thermostatDelegate(fabricTable) {}

     ThermostatDelegate thermostatDelegate;
     ThermostatSetpointsDelegate thermostatSetpointsDelegate;
 };

 class SimulatedWaterHeater : private SimulatedWaterHeaterDelegates,
        public WaterHeater<ThermostatDelegate, ThermostatSetpointsDelegate>,
        public Clusters::WaterHeaterManagement::Delegate, public TimerContext, public Clusters::ModeBase::AppDelegate
 {
 public:

    explicit SimulatedWaterHeater(const Config & config);
    ~SimulatedWaterHeater();

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

    // Clusters::ModeBase::AppDelegate
    CHIP_ERROR Init() override;
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;


private:

    void EndBoost();
    void NotifyHeatDemandAndBoostStateChanged();
    
    template <typename DelegateType>
    DelegateType & GetDelegate()
    {
        return std::get<std::add_lvalue_reference_t<DelegateType>>(mThermostatDelegates);
    }

    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeaterTypes{
        Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap::kImmersionElement1
    };
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeatDemand;
    Clusters::WaterHeaterManagement::BoostStateEnum mBoostState = Clusters::WaterHeaterManagement::BoostStateEnum::kInactive;
    uint32_t mBoostRemainingTime = 0;
    Clusters::Thermostat::temperature mTemperature = kInitialTemperature;
    bool mHeatingEnabled = false;
 };


} // namespace chip::app