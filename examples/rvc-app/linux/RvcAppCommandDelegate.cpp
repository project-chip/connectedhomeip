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

#include "RvcAppCommandDelegate.h"
#include <app/data-model/Nullable.h>
#include <platform/PlatformManager.h>

#include "rvc-device.h"
#include <string>
#include <utility>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

RvcAppCommandHandler * RvcAppCommandHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified, "RVC App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "RVC App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "RVC App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return Platform::New<RvcAppCommandHandler>(std::move(value));
}

void RvcAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self      = reinterpret_cast<RvcAppCommandHandler *>(context);
    std::string name = self->mJsonValue["Name"].asString();

    VerifyOrExit(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (name == "Charged")
    {
        self->OnChargedHandler();
    }
    else if (name == "Charging")
    {
        self->OnChargingHandler();
    }
    else if (name == "Docked")
    {
        self->OnDockedHandler();
    }
    else if (name == "ChargerFound")
    {
        self->OnChargerFoundHandler();
    }
    else if (name == "LowCharge")
    {
        self->OnLowChargeHandler();
    }
    else if (name == "ActivityComplete")
    {
        self->OnActivityCompleteHandler();
    }
    else if (name == "AreaComplete")
    {
        self->OnAreaCompleteHandler();
    }
    else if (name == "AddMap")
    {
        self->OnAddServiceAreaMap(self->mJsonValue);
    }
    else if (name == "AddArea")
    {
        VerifyOrExit(self->mJsonValue.isMember("AreaId"), ChipLogError(NotSpecified, "RVC App: AreaId key is missing"));
        self->OnAddServiceAreaArea(self->mJsonValue);
    }
    else if (name == "RemoveMap")
    {
        VerifyOrExit(self->mJsonValue.isMember("MapId"), ChipLogError(NotSpecified, "RVC App: MapId key is missing"));
        self->OnRemoveServiceAreaMap(self->mJsonValue["MapId"].asUInt());
    }
    else if (name == "RemoveArea")
    {
        VerifyOrExit(self->mJsonValue.isMember("AreaId"), ChipLogError(NotSpecified, "RVC App: AreaId key is missing"));
        self->OnRemoveServiceAreaArea(self->mJsonValue["AreaId"].asUInt());
    }
    else if (name == "ErrorEvent")
    {
        std::string error = self->mJsonValue["Error"].asString();
        self->OnErrorEventHandler(error);
    }
    else if (name == "ClearError")
    {
        self->OnClearErrorHandler();
    }
    else if (name == "Reset")
    {
        self->OnResetHandler();
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happens");
    }

exit:
    Platform::Delete(self);
}

void RvcAppCommandHandler::SetRvcDevice(chip::app::Clusters::RvcDevice * aRvcDevice)
{
    mRvcDevice = aRvcDevice;
}

void RvcAppCommandHandler::OnChargedHandler()
{
    mRvcDevice->HandleChargedMessage();
}

void RvcAppCommandHandler::OnChargingHandler()
{
    mRvcDevice->HandleChargingMessage();
}

void RvcAppCommandHandler::OnDockedHandler()
{
    mRvcDevice->HandleDockedMessage();
}

void RvcAppCommandHandler::OnChargerFoundHandler()
{
    mRvcDevice->HandleChargerFoundMessage();
}

void RvcAppCommandHandler::OnLowChargeHandler()
{
    mRvcDevice->HandleLowChargeMessage();
}

void RvcAppCommandHandler::OnActivityCompleteHandler()
{
    mRvcDevice->HandleActivityCompleteEvent();
}

void RvcAppCommandHandler::OnAreaCompleteHandler()
{
    mRvcDevice->HandleAreaCompletedEvent();
}

void RvcAppCommandHandler::OnAddServiceAreaMap(Json::Value jsonValue)
{
    // Find if self->mJsonValue has the MapId and MapName Keys
    if (jsonValue.isMember("MapId") && jsonValue.isMember("MapName"))
    {
        uint32_t mapId      = jsonValue["MapId"].asUInt();
        std::string mapName = jsonValue["MapName"].asString();
        mRvcDevice->HandleAddServiceAreaMap(mapId, CharSpan(mapName.data(), mapName.size()));
    }
    else
    {
        ChipLogError(NotSpecified, "RVC App: MapId and MapName keys are missing");
    }
}

void RvcAppCommandHandler::OnAddServiceAreaArea(Json::Value jsonValue)
{
    ServiceArea::AreaStructureWrapper area;
    area.SetAreaId(jsonValue["AreaId"].asUInt());
    if (jsonValue.isMember("MapId"))
    {
        area.SetMapId(jsonValue["MapId"].asUInt());
    }

    // Set the location info
    if (jsonValue.isMember("LocationName") || jsonValue.isMember("FloorNumber") || jsonValue.isMember("AreaType"))
    {
        DataModel::Nullable<int16_t> floorNumber = DataModel::NullNullable;
        if (jsonValue.isMember("FloorNumber"))
        {
            floorNumber = jsonValue["FloorNumber"].asInt();
        }
        DataModel::Nullable<Globals::AreaTypeTag> areaType = DataModel::NullNullable;
        if (jsonValue.isMember("AreaType"))
        {
            areaType = Globals::AreaTypeTag(jsonValue["AreaType"].asUInt());
        }
        auto locationName = jsonValue["LocationName"].asString();

        area.SetLocationInfo(CharSpan(locationName.data(), locationName.size()), floorNumber, areaType);
    }

    // Set landmark info
    if (jsonValue.isMember("LandmarkTag"))
    {
        DataModel::Nullable<Globals::RelativePositionTag> relativePositionTag = DataModel::NullNullable;
        if (jsonValue.isMember("RelativePositionTag"))
        {
            relativePositionTag = Globals::RelativePositionTag(jsonValue["RelativePositionTag"].asUInt());
        }

        area.SetLandmarkInfo(Globals::LandmarkTag(jsonValue["LandmarkTag"].asUInt()), relativePositionTag);
    }

    mRvcDevice->HandleAddServiceAreaArea(area);
}

void RvcAppCommandHandler::OnRemoveServiceAreaMap(uint32_t mapId)
{
    mRvcDevice->HandleRemoveServiceAreaMap(mapId);
}

void RvcAppCommandHandler::OnRemoveServiceAreaArea(uint32_t areaId)
{
    mRvcDevice->HandleRemoveServiceAreaArea(areaId);
}

void RvcAppCommandHandler::OnErrorEventHandler(const std::string & error)
{
    mRvcDevice->HandleErrorEvent(error);
}

void RvcAppCommandHandler::OnClearErrorHandler()
{
    mRvcDevice->HandleClearErrorMessage();
}

void RvcAppCommandHandler::OnResetHandler()
{
    mRvcDevice->HandleResetMessage();
}

void RvcAppCommandDelegate::SetRvcDevice(chip::app::Clusters::RvcDevice * aRvcDevice)
{
    mRvcDevice = aRvcDevice;
}

void RvcAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = RvcAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "RVC App: Unable to instantiate a command handler");
        return;
    }

    handler->SetRvcDevice(mRvcDevice);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(RvcAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
}
