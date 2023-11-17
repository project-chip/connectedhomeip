/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "NamedPipeCommands.h"

#include <json/json.h>
#include <platform/DiagnosticDataProvider.h>

class AirQualitySensorAttrUpdateHandler
{
public:
    static AirQualitySensorAttrUpdateHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    AirQualitySensorAttrUpdateHandler(Json::Value && jasonValue) : mJsonValue(std::move(jasonValue)) {}

private:
    Json::Value mJsonValue;

    /**
     * Should be called to set new value of AirQuality.
     */
    void OnAirQualityChangeHandler(uint8_t newValue);

    /**
     * Should be called to set new value of Temperature.
     */
    void OnTemperatureChangeHandler(int16_t newValue);

    /**
     * Should be called to set new value of Humidity.
     */
    void OnHumidityChangeHandler(uint16_t newValue);

    /**
     * Should be called to set new value for a Concentration.
     */
    void OnConcetratorChangeHandler(std::string ConcentrationName, float newValue);
};

class AirQualitySensorAppAttrUpdateDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;
};
