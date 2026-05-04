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

#include "chef-water-heater-management.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;

#if MATTER_DM_WATER_HEATER_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {
namespace Chef {

namespace {
static void OnTimerTick(System::Layer * systemLayer, void * data)
{
    ChipLogProgress(Zcl, "Inside chef-water-heater-management.cpp::OnTimerTick");
    ChefDelegate * delegate = reinterpret_cast<ChefDelegate *>(data);
    delegate->TimerTick();
}
constexpr uint32_t kMinBoostDurationSeconds             = 60;
constexpr int16_t kDefaultTemperatureHundredthsCelsius  = 2000;
constexpr double kWaterHeatCapacityJoulesPerKgPerKelvin = 4182.0;
} // namespace

Protocols::InteractionModel::Status ChefDelegate::HandleBoost(uint32_t durationSeconds, Optional<bool> oneShot,
                                                              Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint,
                                                              Optional<Percent> targetPercentage, Optional<Percent> targetReheat)
{
    ChipLogProgress(Zcl, "Inside ChefDelegate::HandleBoost");

    if (durationSeconds < kMinBoostDurationSeconds)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    DeviceLayer::SystemLayer().CancelTimer(OnTimerTick, this);
    mBoostInfo.ClearValue();

    mBoostState = BoostStateEnum::kActive;
    MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::BoostState::Id);

    Structs::WaterHeaterBoostInfoStruct::Type boostInfo;
    boostInfo.duration          = durationSeconds;
    boostInfo.oneShot           = oneShot;
    boostInfo.emergencyBoost    = emergencyBoost;
    boostInfo.temporarySetpoint = temporarySetpoint;
    boostInfo.targetPercentage  = targetPercentage;
    boostInfo.targetReheat      = targetReheat;

    mBoostInfo.SetValue(boostInfo);
    mBoostInfo.Value().duration--;

    CHIP_ERROR err =
        GenerateBoostStartedEvent(durationSeconds, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleBoost: Failed to generate BoostStarted event: %" CHIP_ERROR_FORMAT, err.Format());
        return Protocols::InteractionModel::Status::Failure;
    }

    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), OnTimerTick, this);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status ChefDelegate::HandleCancelBoost()
{
    DeviceLayer::SystemLayer().CancelTimer(OnTimerTick, this);
    mBoostInfo.ClearValue();

    mBoostState = BoostStateEnum::kInactive;
    MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::BoostState::Id);

    CHIP_ERROR err = GenerateBoostEndedEvent();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleCancelBoost: Failed to generate BoostEnded event: %" CHIP_ERROR_FORMAT, err.Format());
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

void ChefDelegate::TimerTick()
{
    if (mBoostState != BoostStateEnum::kActive)
    {
        return;
    }

    if (!mBoostInfo.HasValue() || mBoostInfo.Value().duration == 0)
    {
        HandleCancelBoost();
        return;
    }

    mBoostInfo.Value().duration--;
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), OnTimerTick, this);
}

Energy_mWh ChefDelegate::GetEstimatedHeatRequired()
{
    int16_t occupiedHeatingSetpoint;
    int16_t localTemperature;

    if (Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Get(mEndpointId, &occupiedHeatingSetpoint) !=
        Protocols::InteractionModel::Status::Success)
    {
        occupiedHeatingSetpoint = kDefaultTemperatureHundredthsCelsius;
    }

    DataModel::Nullable<int16_t> temp;
    if (Clusters::Thermostat::Attributes::LocalTemperature::Get(mEndpointId, temp) !=
            Protocols::InteractionModel::Status::Success ||
        temp.IsNull())
    {
        localTemperature = kDefaultTemperatureHundredthsCelsius;
    }
    else
    {
        localTemperature = temp.Value();
    }

    if (localTemperature >= occupiedHeatingSetpoint)
    {
        return 0;
    }

    // Formula: Energy (mWh) = (4182 * (OccupiedHeatingSetpoint - LocalTemperature) * Volume) / 3600 * (1 - TankPercentage / 100)
    // Note: Use milliwatt-hours as per the energy_mwh type.
    double tempDiffHundreths   = static_cast<double>(occupiedHeatingSetpoint) - static_cast<double>(localTemperature);
    double energyHundreths     = kWaterHeatCapacityJoulesPerKgPerKelvin * tempDiffHundreths * static_cast<double>(mTankVolume);
    double coldWaterPercentage = 100.0 - static_cast<double>(mTankPercentage);
    energyHundreths            = energyHundreths * coldWaterPercentage / 100.0;
    double energymWh           = energyHundreths / 360.0;
    return static_cast<Energy_mWh>(energymWh);
}

} // namespace Chef
} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_WATER_HEATER_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT
