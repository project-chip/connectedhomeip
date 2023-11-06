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
#include <lib/support/BytesToHex.h>
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
    // aCommand should be "lock" or "unlock".
    static CHIP_ERROR ExtractPINFromParams(const char * aCommand, const Json::Value & aParams, Optional<chip::ByteSpan> & aPIN,
                                           chip::Platform::ScopedMemoryBuffer<uint8_t> & aPINBuffer);

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
    CHIP_ERROR err              = CHIP_NO_ERROR;
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
        VerifyOrExit(doorState < to_underlying(DoorStateEnum::kUnknownEnumValue),
                     ChipLogError(NotSpecified,
                                  "Lock App: Unable to execute command to set door state: DoorState is out of range [doorState=%u]",
                                  doorState));
        LockManager::Instance().SetDoorState(endpointId, static_cast<DoorStateEnum>(doorState));
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
            alarmCode < to_underlying(AlarmCodeEnum::kUnknownEnumValue),
            ChipLogError(NotSpecified,
                         "Lock App: Unable to execute command to send lock alarm: AlarmCode is out of range [alarmCode=%u]",
                         alarmCode));
        LockManager::Instance().SendLockAlarm(endpointId, static_cast<AlarmCodeEnum>(alarmCode));
    }
    else if (self->mCommandName == "Lock")
    {

        VerifyOrExit(params["OperationSource"].isUInt(),
                     ChipLogError(NotSpecified, "Lock App: Unable to execute command to lock: invalid type for OperationSource"));

        auto operationSource = params["OperationSource"].asUInt();

        Optional<chip::ByteSpan> pin;
        chip::Platform::ScopedMemoryBuffer<uint8_t> pinBuffer;
        SuccessOrExit(err = ExtractPINFromParams("lock", params, pin, pinBuffer));

        OperationErrorEnum error = OperationErrorEnum::kUnspecified;
        LockManager::Instance().Lock(endpointId, NullNullable, NullNullable, pin, error, OperationSourceEnum(operationSource));
        VerifyOrExit(error == OperationErrorEnum::kUnspecified,
                     ChipLogError(NotSpecified, "Lock App: Lock error received: %u", to_underlying(error)));
    }
    else if (self->mCommandName == "Unlock")
    {
        VerifyOrExit(params["OperationSource"].isUInt(),
                     ChipLogError(NotSpecified, "Lock App: Unable to execute command to unlock: invalid type for OperationSource"));

        auto operationSource = params["OperationSource"].asUInt();

        Optional<chip::ByteSpan> pin;
        chip::Platform::ScopedMemoryBuffer<uint8_t> pinBuffer;
        SuccessOrExit(err = ExtractPINFromParams("unlock", params, pin, pinBuffer));

        OperationErrorEnum error = OperationErrorEnum::kUnspecified;
        LockManager::Instance().Unlock(endpointId, NullNullable, NullNullable, pin, error, OperationSourceEnum(operationSource));
        VerifyOrExit(error == OperationErrorEnum::kUnspecified,
                     ChipLogError(NotSpecified, "Lock App: Unlock error received: %u", to_underlying(error)));
    }
    else
    {
        ChipLogError(NotSpecified, "Lock App: Unable to execute command \"%s\": command not supported", self->mCommandName.c_str());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Lock App: Failed executing command \"%s\": %" CHIP_ERROR_FORMAT, self->mCommandName.c_str(),
                     err.Format());
    }
    chip::Platform::Delete(self);
}

CHIP_ERROR LockAppCommandHandler::ExtractPINFromParams(const char * aCommand, const Json::Value & aParams,
                                                       Optional<chip::ByteSpan> & aPIN,
                                                       chip::Platform::ScopedMemoryBuffer<uint8_t> & aPINBuffer)
{
    if (aParams.isMember("PINAsHex"))
    {
        // Hex-encoded PIN bytes.  So a PIN consisting of the numbers 123 gets encoded as the string "313233"
        VerifyOrReturnError(
            aParams["PINAsHex"].isString(), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(NotSpecified, "Lock App: Unable to execute command to %s: invalid type for PIN", aCommand));

        auto pinAsHex = aParams["PINAsHex"].asString();
        size_t size   = pinAsHex.length();
        VerifyOrReturnError(size % 2 == 0, CHIP_ERROR_INVALID_STRING_LENGTH);

        size_t bufferSize = size / 2;

        VerifyOrReturnError(aPINBuffer.Calloc(bufferSize), CHIP_ERROR_NO_MEMORY);
        size_t octetCount = chip::Encoding::HexToBytes(pinAsHex.c_str(), size, aPINBuffer.Get(), bufferSize);
        VerifyOrReturnError(
            octetCount != 0 || size == 0, CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(NotSpecified, "Lock app: Unable to execute command to %s: invalid hex value for PIN", aCommand));

        aPIN.Emplace(aPINBuffer.Get(), octetCount);
        ChipLogProgress(NotSpecified, "Lock App: Received command to %s with hex PIN: %s", aCommand, pinAsHex.c_str());
    }
    else if (aParams.isMember("PINAsString"))
    {
        // ASCII-encoded PIN bytes.  So a PIN consisting of the numbers 123 gets encoded as the string "123"
        VerifyOrReturnError(
            aParams["PINAsString"].isString(), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(NotSpecified, "Lock App: Unable to execute command to %s: invalid type for PIN", aCommand));

        auto pinAsString  = aParams["PINAsString"].asString();
        size_t bufferSize = pinAsString.length();

        VerifyOrReturnError(aPINBuffer.Calloc(bufferSize), CHIP_ERROR_NO_MEMORY);
        memcpy(aPINBuffer.Get(), pinAsString.c_str(), bufferSize);
        aPIN.Emplace(aPINBuffer.Get(), bufferSize);

        ChipLogProgress(NotSpecified, "Lock App: Received command to %s with string PIN: %s", aCommand, pinAsString.c_str());
    }
    else
    {
        aPIN.ClearValue();
    }

    return CHIP_NO_ERROR;
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
