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

/**********************************************************
 * Includes
 *********************************************************/

#include "TemperatureManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include <app-common/zap-generated/cluster-objects.h>

/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace ::chip::DeviceLayer;

constexpr EndpointId kThermostatEndpoint = 1;

using namespace ::chip::app::Clusters::Thermostat;
namespace ThermAttr = chip::app::Clusters::Thermostat::Attributes;
/**********************************************************
 * Variable declarations
 *********************************************************/

TemperatureManager TemperatureManager::sTempMgr;

CHIP_ERROR TemperatureManager::Init()
{
    app::DataModel::Nullable<int16_t> temp;
    int16_t heatingSetpoint, coolingSetpoint;
    SystemModeEnum systemMode;

    PlatformMgr().LockChipStack();
    ThermAttr::LocalTemperature::Get(kThermostatEndpoint, temp);
    ThermAttr::OccupiedCoolingSetpoint::Get(kThermostatEndpoint, &coolingSetpoint);
    ThermAttr::OccupiedHeatingSetpoint::Get(kThermostatEndpoint, &heatingSetpoint);
    ThermAttr::SystemMode::Get(kThermostatEndpoint, &systemMode);
    PlatformMgr().UnlockChipStack();

    mCurrentTempCelsius     = ConvertToPrintableTemp((temp.IsNull()) ? static_cast<int16_t>(0.0) : temp.Value());
    mHeatingCelsiusSetPoint = ConvertToPrintableTemp(coolingSetpoint);
    mCoolingCelsiusSetPoint = ConvertToPrintableTemp(heatingSetpoint);

    switch (systemMode)
    {
    case SystemModeEnum::kOff:
        mThermMode = 0;
        break;
    case SystemModeEnum::kAuto:
        mThermMode = 1;
        break;
    case SystemModeEnum::kCool:
        mThermMode = 3;
        break;
    case SystemModeEnum::kHeat:
        mThermMode = 4;
        break;
    case SystemModeEnum::kEmergencyHeat:
        mThermMode = 5;
        break;
    case SystemModeEnum::kPrecooling:
        mThermMode = 6;
        break;
    case SystemModeEnum::kFanOnly:
        mThermMode = 7;
        break;
    case SystemModeEnum::kDry:
        mThermMode = 8;
        break;
    case SystemModeEnum::kSleep:
        mThermMode = 9;
        break;
    default:
        mThermMode = 2;
        break; // unknown value;
    }

    AppTask::GetAppTask().UpdateThermoStatUI();

    return CHIP_NO_ERROR;
}

int8_t TemperatureManager::ConvertToPrintableTemp(int16_t temperature)
{
    constexpr uint8_t kRoundUpValue = 50;

    // Round up the temperature as we won't print decimals on LCD
    // Is it a negative temperature
    if (temperature < 0)
    {
        temperature -= kRoundUpValue;
    }
    else
    {
        temperature += kRoundUpValue;
    }

    return static_cast<int8_t>(temperature / 100);
}

void TemperatureManager::AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
    case ThermAttr::LocalTemperature::Id: {
        int8_t Temp = ConvertToPrintableTemp(*((int16_t *) value));
        SILABS_LOG("Local temp %d", Temp);
        mCurrentTempCelsius = Temp;
    }
    break;

    case ThermAttr::OccupiedCoolingSetpoint::Id: {
        int8_t coolingTemp = ConvertToPrintableTemp(*((int16_t *) value));
        SILABS_LOG("CoolingSetpoint %d", coolingTemp);
        mCoolingCelsiusSetPoint = coolingTemp;
    }
    break;

    case ThermAttr::OccupiedHeatingSetpoint::Id: {
        int8_t heatingTemp = ConvertToPrintableTemp(*((int16_t *) value));
        SILABS_LOG("HeatingSetpoint %d", heatingTemp);
        mHeatingCelsiusSetPoint = heatingTemp;
    }
    break;

    case ThermAttr::SystemMode::Id: {
        SILABS_LOG("SystemMode %d", static_cast<uint8_t>(*value));
        uint8_t mode = static_cast<uint8_t>(*value);
        if (mThermMode != mode)
        {
            mThermMode = mode;
        }
    }
    break;

    default: {
        SILABS_LOG("Unhandled thermostat attribute %x", attributeId);
        return;
    }
    break;
    }

    AppTask::GetAppTask().UpdateThermoStatUI();
}

uint8_t TemperatureManager::GetMode()
{
    return mThermMode;
}

int8_t TemperatureManager::GetCurrentTemp()
{
    return mCurrentTempCelsius;
}
int8_t TemperatureManager::GetHeatingSetPoint()
{
    return mHeatingCelsiusSetPoint;
}

int8_t TemperatureManager::GetCoolingSetPoint()
{
    return mCoolingCelsiusSetPoint;
}
