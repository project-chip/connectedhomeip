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

#include "ThermostatDelegate.h"
#include "ThermostatSetpointsDelegate.h"

#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class WaterHeater : public SingleEndpoint, public Clusters::WaterHeaterManagement::Delegate, public TimerContext
{
public:

using ThermostatClusterType = Clusters::Thermostat::ThermostatCluster<
    Clusters::Thermostat::ThermostatDelegate, Clusters::Thermostat::ThermostatSetpointsDelegate>;

    explicit WaterHeater(TimerDelegate & timerDelegate);
    ~WaterHeater() override;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::WaterHeaterManagement::WaterHeaterManagementCluster & WaterHeaterManagementCluster();

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

private:
    void EndBoost();
    void NotifyHeatDemandAndBoostStateChanged();

    TimerDelegate & mTimerDelegate;
    CodeDrivenDataModelProvider * mProvider = nullptr;
    std::unique_ptr<Clusters::Thermostat::ThermostatDelegate> mThermostatDelegate;
    std::unique_ptr<Clusters::Thermostat::ThermostatSetpointsDelegate> mThermostatSetpointsDelegate;

    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeaterTypes{
        Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap::kImmersionElement1
    };
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> mHeatDemand;
    Clusters::WaterHeaterManagement::BoostStateEnum mBoostState = Clusters::WaterHeaterManagement::BoostStateEnum::kInactive;

    LazyRegisteredServerCluster<Clusters::WaterHeaterManagement::WaterHeaterManagementCluster> mWaterHeaterManagementCluster;
    LazyRegisteredServerCluster<ThermostatClusterType> mThermostatCluster;
};

} // namespace chip::app
