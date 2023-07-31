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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

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
    auto * self      = reinterpret_cast<AirQualitySensorAppCommandHandler *>(context);
    std::string name = self->mJsonValue["Name"].asString();

    VerifyOrExit(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (name == "VocChange")
    {
        uint8_t newValue = static_cast<uint8_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnVocChangeHandler(newValue);
    }
    else if (name == "Co2Change")
    {
        uint8_t newValue = static_cast<uint8_t>(self->mJsonValue["NewValue"].asUInt());
        self->OnCo2ChangeHandler(newValue);
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happens");
    }

exit:
    Platform::Delete(self);
}

void AirQualitySensorAppCommandHandler::OnVocChangeHandler(uint8_t newValue)
{
    ChipLogError(NotSpecified, "Failed to set VOC attribute");
    
    // EndpointId endpoint = 0;
    // EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    // VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    // ChipLogDetail(NotSpecified, "The new position when the momentary switch starts to be pressed:%d", newPosition);

    // Clusters::SwitchServer::Instance().OnInitialPress(endpoint, newPosition);
}

void AirQualitySensorAppCommandHandler::OnCo2ChangeHandler(uint8_t newValue)
{
    ChipLogError(NotSpecified, "Failed to set CO2 attribute");

    // EndpointId endpoint = 0;
    // EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    // VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    // ChipLogDetail(NotSpecified, "The the previous value of the CurrentPosition when the momentary switch has been released:%d",
    //               previousPosition);

    // Clusters::SwitchServer::Instance().OnShortRelease(endpoint, previousPosition);
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
