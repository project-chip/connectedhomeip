/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "LockAppCommandDelegate.h"
#include <platform/PlatformManager.h>

#include <LockManager.h>
#include <utility>

using chip::to_underlying;

class LockAppCommandHandler
{
public:
    static LockAppCommandHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    LockAppCommandHandler(std::string && cmd, Json::Value && params) :
        mCommandName(std::move(cmd)), mCommandParameters(std::move(params))
    {}

private:
    std::string mCommandName;
    Json::Value mCommandParameters;
};

LockAppCommandHandler * LockAppCommandHandler::FromJSON(const char * json)
{
    // Command format:
    // { "Cmd": "SetDoorState", "Params": { "EndpointId": 1, "DoorState": 2} }
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified, "Lock App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "Lock App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Cmd") || !value["Cmd"].isString())
    {
        ChipLogError(NotSpecified, "Lock App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    Json::Value params = Json::objectValue;
    if (value.isMember("Params"))
    {
        if (!value["Params"].isObject())
        {
            ChipLogError(NotSpecified, "Lock App: Invalid JSON command received: specified parameters are incorrect");
            return nullptr;
        }
        params = value["Params"];
    }
    auto commandName = value["Cmd"].asString();
    return chip::Platform::New<LockAppCommandHandler>(std::move(commandName), std::move(params));
}

void LockAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self         = reinterpret_cast<LockAppCommandHandler *>(context);
    const auto & params = self->mCommandParameters;
    // Determine the endpoint ID from the parameters JSON. If it is missing, use the default endpoint defined in the
    // door-lock-server.h
    chip::EndpointId endpointId = DOOR_LOCK_SERVER_ENDPOINT;
    if (params.isMember("EndpointId"))
    {
        VerifyOrExit(params["EndpointId"].isUInt(),
                     ChipLogError(NotSpecified, "Lock App: Unable to execute command \"%s\": invalid endpoint Id",
                                  self->mCommandName.c_str()));
        endpointId = static_cast<chip::EndpointId>(params["EndpointId"].asUInt());
    }

    // TODO: Make commands separate objects derived from some base class to clean up this mess.

    // Now we can try to execute a command
    if (self->mCommandName == "SetDoorState")
    {
        VerifyOrExit(params.isMember("DoorState"),
                     ChipLogError(NotSpecified,
                                  "Lock App: Unable to execute command to set the door state: DoorState is missing in command"));

        VerifyOrExit(
            params["DoorState"].isUInt(),
            ChipLogError(NotSpecified, "Lock App: Unable to execute command to set the door state: invalid type for DoorState"));

        auto doorState = params["DoorState"].asUInt();
        VerifyOrExit(doorState < to_underlying(DlDoorState::kUnknownEnumValue),
                     ChipLogError(NotSpecified,
                                  "Lock App: Unable to execute command to set door state: DoorState is out of range [doorState=%u]",
                                  doorState));
        LockManager::Instance().SetDoorState(endpointId, static_cast<DlDoorState>(doorState));
    }
    else if (self->mCommandName == "SendDoorLockAlarm")
    {
        VerifyOrExit(
            params.isMember("AlarmCode"),
            ChipLogError(NotSpecified, "Lock App: Unable to execute command to send lock alarm: AlarmCode is missing in command"));

        VerifyOrExit(
            params["AlarmCode"].isUInt(),
            ChipLogError(NotSpecified, "Lock App: Unable to execute command to send lock alarm: invalid type for AlarmCode"));

        auto alarmCode = params["AlarmCode"].asUInt();
        VerifyOrExit(
            alarmCode < to_underlying(DlAlarmCode::kUnknownEnumValue),
            ChipLogError(NotSpecified,
                         "Lock App: Unable to execute command to send lock alarm: AlarmCode is out of range [alarmCode=%u]",
                         alarmCode));
        LockManager::Instance().SendLockAlarm(endpointId, static_cast<DlAlarmCode>(alarmCode));
    }
    else
    {
        ChipLogError(NotSpecified, "Lock App: Unable to execute command \"%s\": command not supported", self->mCommandName.c_str());
    }

exit:
    chip::Platform::Delete(self);
}

void LockAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = LockAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "Lock App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(LockAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
}
