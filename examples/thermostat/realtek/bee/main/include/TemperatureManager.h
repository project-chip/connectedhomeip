/*
 *
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

#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/CHIPError.h>
#include <stdbool.h>
#include <stdint.h>

using namespace chip;

class TemperatureManager
{
public:
    static TemperatureManager & Instance()
    {
        static TemperatureManager sTemperatureManager;
        return sTemperatureManager;
    };

    CHIP_ERROR Init();
    void AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    app::DataModel::Nullable<int16_t> GetLocalTemp();

    void LogThermostatStatus();

private:
    app::DataModel::Nullable<int16_t> mLocalTempCelsius;
    int16_t mCoolingCelsiusSetPoint;
    int16_t mHeatingCelsiusSetPoint;
    app::Clusters::Thermostat::SystemModeEnum mThermMode;
};
