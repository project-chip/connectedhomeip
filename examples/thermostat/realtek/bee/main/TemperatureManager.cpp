/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "TemperatureManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include <app-common/zap-generated/cluster-objects.h>

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::app::Clusters;

constexpr EndpointId kThermostatEndpoint = 1;

/* Convert and return only complete part of value to printable type */
uint8_t ReturnCompleteValue(int16_t Value)
{
    return static_cast<uint8_t>(Value / 100);
}

/* Converts and returns only reminder part of value to printable type.
 * This formula rounds reminder value to one significant figure
 */

uint8_t ReturnRemainderValue(int16_t Value)
{
    return static_cast<uint8_t>((Value % 100 + 5) / 10);
}

CHIP_ERROR TemperatureManager::Init()
{
    PlatformMgr().LockChipStack();
    Thermostat::Attributes::LocalTemperature::Get(kThermostatEndpoint, mLocalTempCelsius);
    Thermostat::Attributes::OccupiedCoolingSetpoint::Get(kThermostatEndpoint, &mCoolingCelsiusSetPoint);
    Thermostat::Attributes::OccupiedHeatingSetpoint::Get(kThermostatEndpoint, &mHeatingCelsiusSetPoint);
    Thermostat::Attributes::SystemMode::Get(kThermostatEndpoint, &mThermMode);
    PlatformMgr().UnlockChipStack();

    return CHIP_NO_ERROR;
}

void TemperatureManager::LogThermostatStatus()
{
    ChipLogProgress(NotSpecified, "Thermostat:");
    ChipLogProgress(NotSpecified, "  Mode - %d", static_cast<uint8_t>(mThermMode));
    if (!(GetLocalTemp().IsNull()))
    {
        int16_t tempValue = GetLocalTemp().Value();
        ChipLogProgress(NotSpecified, "  LocalTemperature - %d,%d'C", ReturnCompleteValue(tempValue),
                        ReturnRemainderValue(tempValue));
    }
    else
    {
        ChipLogProgress(NotSpecified, "  LocalTemperature - No Value");
    }
    ChipLogProgress(NotSpecified, "  HeatingSetpoint - %d,%d'C", ReturnCompleteValue(mHeatingCelsiusSetPoint),
                    ReturnRemainderValue(mHeatingCelsiusSetPoint));
    ChipLogProgress(NotSpecified, "  CoolingSetpoint - %d,%d'C \n", ReturnCompleteValue(mCoolingCelsiusSetPoint),
                    ReturnRemainderValue(mCoolingCelsiusSetPoint));
}

void TemperatureManager::AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
    case Thermostat::Attributes::LocalTemperature::Id: {
        Thermostat::Attributes::LocalTemperature::Get(kThermostatEndpoint, mLocalTempCelsius);
    }
    break;

    case Thermostat::Attributes::OccupiedCoolingSetpoint::Id: {
        mCoolingCelsiusSetPoint = *reinterpret_cast<int16_t *>(value);
        ChipLogProgress(NotSpecified, "Cooling TEMP: %d", mCoolingCelsiusSetPoint);
    }
    break;

    case Thermostat::Attributes::OccupiedHeatingSetpoint::Id: {
        mHeatingCelsiusSetPoint = *reinterpret_cast<int16_t *>(value);
        ChipLogProgress(NotSpecified, "Heating TEMP %d", mHeatingCelsiusSetPoint);
    }
    break;

    case Thermostat::Attributes::SystemMode::Id: {
        mThermMode = static_cast<app::Clusters::Thermostat::SystemModeEnum>(*value);
    }
    break;

    default: {
        ChipLogProgress(NotSpecified, "Unhandled thermostat attribute %x", attributeId);
        return;
    }
    break;
    }
}

app::DataModel::Nullable<int16_t> TemperatureManager::GetLocalTemp()
{
    return mLocalTempCelsius;
}
