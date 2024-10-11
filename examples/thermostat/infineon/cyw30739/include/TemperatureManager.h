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

#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>

#include <lib/core/CHIPError.h>

using namespace chip;
using namespace ::chip::app::Clusters::Thermostat;

class TemperatureManager
{
public:
    CHIP_ERROR Init();
    void SensorInterruptHandler();
    void AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    SystemModeEnum GetMode();
    int16_t GetCurrentTemp();
    uint8_t GetCurrentHumidity();
    int8_t GetHeatingSetPoint();
    int8_t GetCoolingSetPoint();

private:
    friend TemperatureManager & TempMgr();

    int8_t mCurrentTempCelsius;
    uint8_t mCurrentHumidity;
    int8_t mCoolingCelsiusSetPoint;
    int8_t mHeatingCelsiusSetPoint;
    SystemModeEnum mThermMode;

    int8_t ConvertToPrintableTemp(int16_t temperature);
    static TemperatureManager sTempMgr;
};

inline TemperatureManager & TempMgr()
{
    return TemperatureManager::sTempMgr;
}
