/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "CameraAppCommandDelegate.h"
#include <platform/PlatformManager.h>

CameraAppCommandHandler * CameraAppCommandHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified, "Camera App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "Camera App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "Camera App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return chip::Platform::New<CameraAppCommandHandler>(std::move(value));
}

void CameraAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self      = reinterpret_cast<CameraAppCommandHandler *>(context);
    std::string name = self->mJsonValue["Name"].asString();

    VerifyOrExit(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (name == "ZoneTriggered")
    {
        uint16_t zoneId = static_cast<uint16_t>(self->mJsonValue["ZoneId"].asUInt());
        self->OnZoneTriggeredHandler(zoneId);
    }
    else if (name == "SetHardPrivacyModeOn")
    {
        bool value = self->mJsonValue["Value"].asBool();
        self->OnSetHardPrivacyModeOnHandler(value);
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happen");
    }

exit:
    chip::Platform::Delete(self);
}

void CameraAppCommandHandler::SetCameraDevice(Camera::CameraDevice * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

void CameraAppCommandHandler::OnZoneTriggeredHandler(uint16_t zoneId)
{
    mCameraDevice->HandleSimulatedZoneTriggeredEvent(zoneId);
}

void CameraAppCommandHandler::OnSetHardPrivacyModeOnHandler(bool value)
{
    TEMPORARY_RETURN_IGNORED mCameraDevice->GetCameraAVStreamMgmtController().SetHardPrivacyModeOn(value);
}

void CameraAppCommandDelegate::SetCameraDevice(Camera::CameraDevice * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

void CameraAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = CameraAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "Camera App: Unable to instantiate a command handler");
        return;
    }

    handler->SetCameraDevice(mCameraDevice);
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(CameraAppCommandHandler::HandleCommand,
                                                                           reinterpret_cast<intptr_t>(handler));
}
