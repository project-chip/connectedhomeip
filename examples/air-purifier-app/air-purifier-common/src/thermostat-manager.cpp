/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "thermostat-manager.h"
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void ThermostatManager::Init()
{
    BitMask<Thermostat::Feature> FeatureMap;
    FeatureMap.Set(Thermostat::Feature::kHeating);
    Status status = Thermostat::Attributes::FeatureMap::Set(mEndpointId, FeatureMap.Raw());

    status = Thermostat::Attributes::ControlSequenceOfOperation::Set(mEndpointId,
                                                                     Thermostat::ControlSequenceOfOperationEnum::kHeatingOnly);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set Thermostat ControlSequenceOfOperation attribute"));

    status = Thermostat::Attributes::AbsMinHeatSetpointLimit::Set(mEndpointId, 500);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set Thermostat MinHeatSetpointLimit attribute"));

    status = Thermostat::Attributes::AbsMaxHeatSetpointLimit::Set(mEndpointId, 3000);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set Thermostat MaxHeatSetpointLimit attribute"));
}

void ThermostatManager::HeatingSetpointWriteCallback(int16_t newValue)
{
    ChipLogDetail(NotSpecified, "ThermostatManager::HeatingSetpointWriteCallback: %d", newValue);
    Thermostat::SystemModeEnum systemMode;
    Status status = Thermostat::Attributes::SystemMode::Get(mEndpointId, &systemMode);
    VerifyOrReturn(Status::Success == status, ChipLogError(NotSpecified, "Failed to get Thermostat SystemMode attribute"));

    // A new setpoint has been set, so we shall infer that the we want to be in Heating mode
    if (systemMode == Thermostat::SystemModeEnum::kOff)
    {
        SetHeatMode(true);
    }

    // Check the current temperature and turn on the heater if needed
    DataModel::Nullable<int16_t> localTemperature;
    status = Thermostat::Attributes::LocalTemperature::Get(mEndpointId, localTemperature);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to get TemperatureMeasurement MeasuredValue attribute"));

    if (localTemperature.Value() < newValue)
    {
        SetHeating(true);
    }
    else
    {
        SetHeating(false);
    }
}

void ThermostatManager::SystemModeWriteCallback(uint8_t newValue)
{
    ChipLogDetail(NotSpecified, "ThermostatManager::SystemModeWriteCallback: %d", newValue);
    if ((Thermostat::SystemModeEnum) newValue == Thermostat::SystemModeEnum::kOff)
    {
        SetHeating(false);
    }
    else if ((Thermostat::SystemModeEnum) newValue == Thermostat::SystemModeEnum::kHeat)
    {
        DataModel::Nullable<int16_t> localTemperature;
        Status status = Thermostat::Attributes::LocalTemperature::Get(mEndpointId, localTemperature);
        VerifyOrReturn(Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to get TemperatureMeasurement MeasuredValue attribute"));

        int16_t heatingSetpoint;
        status = Thermostat::Attributes::OccupiedHeatingSetpoint::Get(mEndpointId, &heatingSetpoint);
        VerifyOrReturn(Status::Success == status, ChipLogError(NotSpecified, "Failed to get Thermostat HeatingSetpoint attribute"));

        if (localTemperature.Value() < heatingSetpoint)
        {
            SetHeating(true);
        }
    }
}

void ThermostatManager::OnLocalTemperatureChangeCallback(int16_t temperature)
{
    Status status = Thermostat::Attributes::LocalTemperature::Set(mEndpointId, temperature);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValue attribute"));
}

void ThermostatManager::SetHeating(bool isHeating)
{
    BitMask<Thermostat::RelayStateBitmap> runningState;

    if (isHeating)
    {
        runningState.Set(Thermostat::RelayStateBitmap::kHeat);

        if (heatingCallback)
        {
            heatingCallback();
        }
    }
    else
    {
        runningState.Clear(Thermostat::RelayStateBitmap::kHeat);
    }

    Status status = Thermostat::Attributes::ThermostatRunningState::Set(mEndpointId, runningState);
    VerifyOrReturn(Status::Success == status, ChipLogError(NotSpecified, "Failed to set Thermostat RunningState attribute"));
}

void ThermostatManager::SetHeatMode(bool heat)
{
    Status status = Thermostat::Attributes::SystemMode::Set(
        mEndpointId, heat ? Thermostat::SystemModeEnum::kHeat : Thermostat::SystemModeEnum::kOff);
    VerifyOrReturn(Status::Success == status, ChipLogError(NotSpecified, "Failed to set Thermostat SystemMode attribute"));
}
