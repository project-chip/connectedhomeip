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

#include "AirQualitySensorAppCommandDelegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/att-storage.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <platform/PlatformManager.h>

#include <map>
#include <limits>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

struct MinMaxValues {
    float minValue;
    float maxValue;
};

struct ConcentratorFuncPointers {
    EmberAfStatus (*setMeasuredValue)(chip::EndpointId, float);
    EmberAfStatus (*getMinValue)(chip::EndpointId endpoint, DataModel::Nullable<float> & value);
    EmberAfStatus (*setMinValue)(chip::EndpointId, float);
    EmberAfStatus (*getMaxValue)(chip::EndpointId endpoint, DataModel::Nullable<float> & value);
    EmberAfStatus (*setMaxValue)(chip::EndpointId, float);
};

// Define a map of string keys to function pointers
std::map<std::string, ConcentratorFuncPointers> concentratorFuncMap = {
    {"CarbonMonoxideConcentrationMeasurement",
        {&CarbonMonoxideConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &CarbonMonoxideConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &CarbonMonoxideConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &CarbonMonoxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &CarbonMonoxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"CarbonDioxideConcentrationMeasurement",
        {&CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &CarbonDioxideConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &CarbonDioxideConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &CarbonDioxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &CarbonDioxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"NitrogenDioxideConcentrationMeasurement",
        {&NitrogenDioxideConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &NitrogenDioxideConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &NitrogenDioxideConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &NitrogenDioxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &NitrogenDioxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"OzoneConcentrationMeasurement",
        {&OzoneConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &OzoneConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &OzoneConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &OzoneConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &OzoneConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"Pm25ConcentrationMeasurement",
        {&Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &Pm25ConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &Pm25ConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &Pm25ConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &Pm25ConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"FormaldehydeConcentration",
        {&FormaldehydeConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &FormaldehydeConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &FormaldehydeConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &FormaldehydeConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &FormaldehydeConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"Pm1ConcentrationMeasurement",
        {&Pm1ConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &Pm1ConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &Pm1ConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &Pm1ConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &Pm1ConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"Pm10ConcentrationMeasurement",
        {&Pm10ConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &Pm10ConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &Pm10ConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &Pm10ConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &Pm10ConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"TotalVolatileOrganicCompoundsConcentrationMeasurement",
        {&TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    },
    {"RadonConcentrationMeasurement",
        {&RadonConcentrationMeasurement::Attributes::MeasuredValue::Set,
         &RadonConcentrationMeasurement::Attributes::MinMeasuredValue::Get,
         &RadonConcentrationMeasurement::Attributes::MinMeasuredValue::Set,
         &RadonConcentrationMeasurement::Attributes::MaxMeasuredValue::Get,
         &RadonConcentrationMeasurement::Attributes::MaxMeasuredValue::Set }
    }
};


AirQualitySensorAppCommandHandler * AirQualitySensorAppCommandHandler::FromJSON(const char * json)
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

    return Platform::New<AirQualitySensorAppCommandHandler>(std::move(value));
}

void AirQualitySensorAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self = reinterpret_cast<AirQualitySensorAppCommandHandler *>(context);
    std::string clusterName = self->mJsonValue["Name"].asString();

    VerifyOrReturn(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (clusterName == "AirQuality") {
        uint8_t newValue = static_cast<uint8_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnAirQualityChangeHandler(newValue);
    }
    else if (clusterName == "TemperatureMeasurement") {
        int16_t newValue = static_cast<int16_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnTemperatureChangeHandler(newValue);
    }
    else if (clusterName == "RelativeHumidityMeasurement") {
        uint16_t newValue = static_cast<uint16_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnHumidityChangeHandler(newValue);
    }
    else if (clusterName.find("Concentration") != std::string::npos) {
        float newValue = static_cast<uint16_t>(self->mJsonValue["NewValue"].asFloat());
        self->OnConcetratorChangeHandler(clusterName, newValue);
    }
    else  {
        ChipLogError(NotSpecified, "Invalid cluster name %s", clusterName.c_str());
    }

     ChipLogError(NotSpecified, "done");
}

void AirQualitySensorAppCommandHandler::OnAirQualityChangeHandler(uint8_t newValue)
{
    EndpointId endpoint = 1;
    EmberAfStatus status = AirQuality::Attributes::AirQuality::Set(endpoint, static_cast<AirQuality::AirQualityEnum>(newValue));
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set AirQuality attribute"));
    ChipLogDetail(NotSpecified, "The new AirQuality value: %d", newValue);
}

void AirQualitySensorAppCommandHandler::OnTemperatureChangeHandler(int16_t newValue)
{
    DataModel::Nullable<int16_t> minVal;
    DataModel::Nullable<int16_t> maxVal;   
    EndpointId endpoint = 1; 

    EmberAfStatus status = TemperatureMeasurement::Attributes::MeasuredValue::Set(endpoint, newValue);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to TemperatureMeasurement MeasuredValue attribute"));
    ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);

    TemperatureMeasurement::Attributes::MinMeasuredValue::Get(endpoint, minVal);
    if(minVal.IsNull()) {
        status = TemperatureMeasurement::Attributes::MinMeasuredValue::Set(endpoint,newValue);
    }
    else if(newValue < minVal.Value()) {
        status = TemperatureMeasurement::Attributes::MinMeasuredValue::Set(endpoint,newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));
    }

    TemperatureMeasurement::Attributes::MaxMeasuredValue::Get(endpoint, maxVal);
    if(maxVal.IsNull()) {
        status = TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(endpoint,newValue);
    }    
    else if(newValue > maxVal.Value()) {
        status = TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(endpoint, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));
    }
}

void AirQualitySensorAppCommandHandler::OnHumidityChangeHandler(uint16_t newValue)
{
    DataModel::Nullable<uint16_t> minVal;
    DataModel::Nullable<uint16_t> maxVal;   
    EndpointId endpoint = 1; 

    EmberAfStatus status = RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(endpoint, newValue);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to RelativeHumidityMeasurement MeasuredValue attribute"));
    ChipLogDetail(NotSpecified, "The new RelativeHumidityMeasurement value: %d", newValue);

    RelativeHumidityMeasurement::Attributes::MeasuredValue::Get(endpoint, minVal);
    if(minVal.IsNull()) {
        status = RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Set(endpoint,newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MinMeasuredValue attribute"));
    }
    else if(newValue < minVal.Value()) {
        status = RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Set(endpoint,newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MinMeasuredValue attribute"));
    }

    RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Get(endpoint, maxVal);
    if(maxVal.IsNull()) {
        status = RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Set(endpoint,newValue);
    }    
    else if(newValue > maxVal.Value()) {
        status = RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Set(endpoint, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MinMeasuredValue attribute"));
    }
}

void AirQualitySensorAppCommandHandler::OnConcetratorChangeHandler(std::string concentratorName, float newValue)
{  
    EndpointId endpoint = 1;
    DataModel::Nullable<float> minVal;
    DataModel::Nullable<float> maxVal;
    auto [setMeasuredValue, getMinValue, setMinValue, getMaxValue, setMaxValue] = concentratorFuncMap[concentratorName];

    VerifyOrReturn(setMeasuredValue != NULL, ChipLogError(NotSpecified, "Invalid concentrator %s", concentratorName.c_str()));

    EmberAfStatus status = setMeasuredValue(endpoint, newValue);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to %s set MeasuredValue attribute", concentratorName.c_str()));
    ChipLogDetail(NotSpecified, "The new %s value: %f", concentratorName.c_str(), newValue);
    
    getMinValue(endpoint, minVal);
    if(minVal.IsNull()) {
        ChipLogDetail(NotSpecified, "minVal.IsNull");
        status = setMinValue(endpoint,newValue);
    }
    else if(newValue < minVal.Value()) {
        ChipLogDetail(NotSpecified, "newValue < minVal");
        status = setMinValue(endpoint, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to %s MinMeasuredValue attribute", concentratorName.c_str()));
    }

    getMaxValue(endpoint, maxVal);
    if(maxVal.IsNull()) {
        ChipLogDetail(NotSpecified, "maxVal.IsNull");
        status = setMaxValue(endpoint, newValue);
    }    
    else if(newValue > maxVal.Value()) {
        ChipLogDetail(NotSpecified, "newValue > maxVal");
        status = setMaxValue(endpoint, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to %s MinMeasuredValue attribute", concentratorName.c_str()));
    }
}

void AirQualitySensorAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = AirQualitySensorAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "AllClusters App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(AirQualitySensorAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
}
