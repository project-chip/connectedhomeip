/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
