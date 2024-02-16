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
#include <platform/PlatformManager.h>

#include "rvc-device.h"
#include <utility>

using namespace chip;
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
