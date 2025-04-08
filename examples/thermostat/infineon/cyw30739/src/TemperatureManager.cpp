/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "AppTask.h"
#include "hdc2010.h"
#include "platform/CHIPDeviceLayer.h"
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

    ThermAttr::LocalTemperature::Get(kThermostatEndpoint, temp);
    ThermAttr::OccupiedCoolingSetpoint::Get(kThermostatEndpoint, &coolingSetpoint);
    ThermAttr::OccupiedHeatingSetpoint::Get(kThermostatEndpoint, &heatingSetpoint);
    ThermAttr::SystemMode::Get(kThermostatEndpoint, &mThermMode);

    mCurrentTempCelsius     = 0;
    mCurrentHumidity        = 0;
    mHeatingCelsiusSetPoint = ConvertToPrintableTemp(coolingSetpoint);
    mCoolingCelsiusSetPoint = ConvertToPrintableTemp(heatingSetpoint);

    return CHIP_NO_ERROR;
}

void TemperatureManager::SensorInterruptHandler()
{
    ThermAttr::LocalTemperature::Set(kThermostatEndpoint, wiced_hdc2010_read_temp() * 100);

    mCurrentHumidity = wiced_hdc2010_read_humidity();
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
        ChipLogDetail(Zcl, "Local temp %d", Temp);
        mCurrentTempCelsius = Temp;
    }
    break;

    case ThermAttr::OccupiedCoolingSetpoint::Id: {
        int8_t coolingTemp = ConvertToPrintableTemp(*((int16_t *) value));
        ChipLogDetail(Zcl, "CoolingSetpoint %d", coolingTemp);
        mCoolingCelsiusSetPoint = coolingTemp;
    }
    break;

    case ThermAttr::OccupiedHeatingSetpoint::Id: {
        int8_t heatingTemp = ConvertToPrintableTemp(*((int16_t *) value));
        ChipLogDetail(Zcl, "HeatingSetpoint %d", heatingTemp);
        mHeatingCelsiusSetPoint = heatingTemp;
    }
    break;

    case ThermAttr::SystemMode::Id: {
        ChipLogDetail(Zcl, "SystemMode %d", static_cast<uint8_t>(*value));
        SystemModeEnum mode = static_cast<SystemModeEnum>(*value);
        if (mThermMode != mode)
        {
            mThermMode = mode;
        }
    }
    break;

    default: {
        ChipLogDetail(Zcl, "Unhandled thermostat attribute %lx", attributeId);
        return;
    }
    break;
    }
}

SystemModeEnum TemperatureManager::GetMode()
{
    return mThermMode;
}

int16_t TemperatureManager::GetCurrentTemp()
{
    return mCurrentTempCelsius;
}

uint8_t TemperatureManager::GetCurrentHumidity()
{
    return mCurrentHumidity;
}

int8_t TemperatureManager::GetHeatingSetPoint()
{
    return mHeatingCelsiusSetPoint;
}

int8_t TemperatureManager::GetCoolingSetPoint()
{
    return mCoolingCelsiusSetPoint;
}
