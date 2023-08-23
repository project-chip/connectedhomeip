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

#include "AirQualitySensorAppAttrUpdateDelegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/att-storage.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <platform/PlatformManager.h>

#include <limits>
#include <map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

struct ConcentrationFuncPointers
{
    EmberAfStatus (*setMeasuredValue)(chip::EndpointId, float);
    /* Add required function maps */
};

// Define a map of string keys to function pointers
std::map<std::string, ConcentrationFuncPointers> ConcentrationFuncMap = {
    { "CarbonMonoxideConcentrationMeasurement",
      { &CarbonMonoxideConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "CarbonDioxideConcentrationMeasurement",
      { &CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "NitrogenDioxideConcentrationMeasurement",
      { &NitrogenDioxideConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "OzoneConcentrationMeasurement",
      { &OzoneConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "Pm25ConcentrationMeasurement",
      { &Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "FormaldehydeConcentration",
      { &FormaldehydeConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "Pm1ConcentrationMeasurement",
      { &Pm1ConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "Pm10ConcentrationMeasurement",
      { &Pm10ConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "TotalVolatileOrganicCompoundsConcentrationMeasurement",
      { &TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } },
    { "RadonConcentrationMeasurement",
      { &RadonConcentrationMeasurement::Attributes::MeasuredValue::Set
        /* Add required function maps */ } }
};

AirQualitySensorAttrUpdateHandler * AirQualitySensorAttrUpdateHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified,
                     "AllClusters App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "AllClusters App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "AllClusters App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return Platform::New<AirQualitySensorAttrUpdateHandler>(std::move(value));
}

void AirQualitySensorAttrUpdateHandler::HandleCommand(intptr_t context)
{
    auto * self             = reinterpret_cast<AirQualitySensorAttrUpdateHandler *>(context);
    std::string clusterName = self->mJsonValue["Name"].asString();

    VerifyOrReturn(!self->mJsonValue.empty(), {
        ChipLogError(NotSpecified, "Invalid JSON event command received");
        Platform::Delete(self);
    });

    if (clusterName == "AirQuality")
    {
        uint8_t newValue = static_cast<uint8_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnAirQualityChangeHandler(newValue);
    }
    else if (clusterName == "TemperatureMeasurement")
    {
        int16_t newValue = static_cast<int16_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnTemperatureChangeHandler(newValue);
    }
    else if (clusterName == "RelativeHumidityMeasurement")
    {
        uint16_t newValue = static_cast<uint16_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnHumidityChangeHandler(newValue);
    }
    else if (clusterName.find("Concentration") != std::string::npos)
    {
        float newValue = static_cast<uint16_t>(self->mJsonValue["NewValue"].asFloat());
        self->OnConcetratorChangeHandler(clusterName, newValue);
    }
    else
    {
        ChipLogError(NotSpecified, "Invalid cluster name %s", clusterName.c_str());
    }

    // Delete AirQualitySensorAttrUpdateHandler
    Platform::Delete(self);
}

void AirQualitySensorAttrUpdateHandler::OnAirQualityChangeHandler(uint8_t newValue)
{
    EndpointId endpoint  = 1;
    EmberAfStatus status = AirQuality::Attributes::AirQuality::Set(endpoint, static_cast<AirQuality::AirQualityEnum>(newValue));
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set AirQuality attribute"));
    ChipLogDetail(NotSpecified, "The new AirQuality value: %d", newValue);
}

void AirQualitySensorAttrUpdateHandler::OnTemperatureChangeHandler(int16_t newValue)
{
    EndpointId endpoint  = 1;
    EmberAfStatus status = TemperatureMeasurement::Attributes::MeasuredValue::Set(endpoint, newValue);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                   ChipLogError(NotSpecified, "Failed to TemperatureMeasurement MeasuredValue attribute"));
    ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);
}

void AirQualitySensorAttrUpdateHandler::OnHumidityChangeHandler(uint16_t newValue)
{
    EndpointId endpoint  = 1;
    EmberAfStatus status = RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(endpoint, newValue);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                   ChipLogError(NotSpecified, "Failed to RelativeHumidityMeasurement MeasuredValue attribute"));
    ChipLogDetail(NotSpecified, "The new RelativeHumidityMeasurement value: %d", newValue);
}

void AirQualitySensorAttrUpdateHandler::OnConcetratorChangeHandler(std::string ConcentrationName, float newValue)
{
    auto [setMeasuredValue] = ConcentrationFuncMap[ConcentrationName];

    VerifyOrReturn(setMeasuredValue != NULL, ChipLogError(NotSpecified, "Invalid Concentration %s", ConcentrationName.c_str()));

    EndpointId endpoint  = 1;
    EmberAfStatus status = setMeasuredValue(endpoint, newValue);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                   ChipLogError(NotSpecified, "Failed to %s set MeasuredValue attribute", ConcentrationName.c_str()));
    ChipLogDetail(NotSpecified, "The new %s value: %f", ConcentrationName.c_str(), newValue);
}

void AirQualitySensorAppAttrUpdateDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = AirQualitySensorAttrUpdateHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "AllClusters App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(AirQualitySensorAttrUpdateHandler::HandleCommand,
                                                  reinterpret_cast<intptr_t>(handler));
}
