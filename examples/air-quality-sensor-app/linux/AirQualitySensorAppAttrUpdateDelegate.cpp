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
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <platform/PlatformManager.h>

#include <air-quality-sensor-manager.h>

#include <limits>
#include <map>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;
using namespace chip::DeviceLayer;

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

    AirQualitySensorManager * mInstance = AirQualitySensorManager::GetInstance();
    if (clusterName == "AirQuality")
    {
        uint8_t newValue = static_cast<uint8_t>(self->mJsonValue["NewValue"].asUInt());
        mInstance->OnAirQualityChangeHandler(static_cast<AirQualityEnum>(newValue));
    }
    else if (clusterName == "TemperatureMeasurement")
    {
        int16_t newValue = static_cast<int16_t>(self->mJsonValue["NewValue"].asUInt());
        mInstance->OnTemperatureMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "RelativeHumidityMeasurement")
    {
        uint16_t newValue = static_cast<uint16_t>(self->mJsonValue["NewValue"].asUInt());
        mInstance->OnHumidityMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "CarbonDioxideConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnCarbonDioxideMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "CarbonMonoxideConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnCarbonMonoxideMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "NitrogenDioxideConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnNitrogenDioxideMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "OzoneConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnOzoneMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "Pm25ConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnPm25MeasurementChangeHandler(newValue);
    }
    else if (clusterName == "FormaldehydeConcentration")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnFormaldehydeMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "Pm1ConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnPm1MeasurementChangeHandler(newValue);
    }
    else if (clusterName == "Pm10ConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnPm10MeasurementChangeHandler(newValue);
    }
    else if (clusterName == "TotalVolatileOrganicCompoundsConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnTotalVolatileOrganicCompoundsMeasurementChangeHandler(newValue);
    }
    else if (clusterName == "RadonConcentrationMeasurement")
    {
        float newValue = static_cast<float>(self->mJsonValue["NewValue"].asDouble());
        mInstance->OnRadonMeasurementChangeHandler(newValue);
    }
    else
    {
        ChipLogError(NotSpecified, "Invalid cluster name %s", clusterName.c_str());
    }

    // Delete AirQualitySensorAttrUpdateHandler
    Platform::Delete(self);
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
