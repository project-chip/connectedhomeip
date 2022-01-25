/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "app/ConcreteAttributePath.h"
#include <cinttypes>

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <controller/python/chip/interaction_model/Delegate.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app;
using namespace chip::Controller;

namespace chip {

namespace Controller {

PythonInteractionModelDelegate gPythonInteractionModelDelegate;

void PythonInteractionModelDelegate::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath,
                                                const app::StatusIB & aStatus, TLV::TLVReader * aData)
{
    CommandStatus status{
        aStatus.mStatus,
        aStatus.mClusterStatus.HasValue() ? aStatus.mClusterStatus.Value() : chip::python::kUndefinedClusterStatus,
        aPath.mEndpointId,
        aPath.mClusterId,
        aPath.mCommandId,
        1
    }; // This indicates the index of the command if multiple command/status payloads are present in the
       // message. For now, we don't support this in the IM layer, so just always set this to 1.
    if (commandResponseStatusFunct != nullptr)
    {
        commandResponseStatusFunct(reinterpret_cast<uint64_t>(apCommandSender), &status, sizeof(status));
    }

    DeviceControllerInteractionModelDelegate::OnResponse(apCommandSender, aPath, aStatus, aData);

    if (commandResponseErrorFunct != nullptr)
    {
        commandResponseErrorFunct(reinterpret_cast<uint64_t>(apCommandSender), CHIP_NO_ERROR.AsInteger());
    }
}

void PythonInteractionModelDelegate::OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError)
{
    StatusIB serverStatus(aError);
    CommandStatus status{
        serverStatus.mStatus, serverStatus.mClusterStatus.ValueOr(chip::python::kUndefinedClusterStatus), 0, 0, 0, 1
    };

    if (commandResponseStatusFunct != nullptr)
    {
        commandResponseStatusFunct(reinterpret_cast<uint64_t>(apCommandSender), &status, sizeof(status));
    }

    if (commandResponseErrorFunct != nullptr)
    {
        commandResponseErrorFunct(reinterpret_cast<uint64_t>(apCommandSender), aError.AsInteger());
    }
    DeviceControllerInteractionModelDelegate::OnError(apCommandSender, aError);
}

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f)
{
    gPythonInteractionModelDelegate.SetOnCommandResponseStatusCodeReceivedCallback(f);
}

void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f)
{
    gPythonInteractionModelDelegate.SetOnCommandResponseProtocolErrorCallback(f);
}

void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f)
{
    gPythonInteractionModelDelegate.SetOnCommandResponseCallback(f);
}

void pychip_InteractionModelDelegate_SetOnWriteResponseStatusCallback(PythonInteractionModelDelegate_OnWriteResponseStatusFunct f)
{
    gPythonInteractionModelDelegate.SetOnWriteResponseStatusCallback(f);
}

PythonInteractionModelDelegate & PythonInteractionModelDelegate::Instance()
{
    return gPythonInteractionModelDelegate;
}

} // namespace Controller
} // namespace chip

extern "C" {

static_assert(std::is_same<uint32_t, chip::ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

chip::ChipError::StorageType pychip_InteractionModel_GetCommandSenderHandle(uint64_t * commandSender)
{
    chip::app::CommandSender * commandSenderObj = nullptr;
    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_INVALID_ARGUMENT.AsInteger());
    commandSenderObj = new chip::app::CommandSender(nullptr, nullptr);
    VerifyOrReturnError(commandSenderObj != nullptr, (CHIP_ERROR_NO_MEMORY).AsInteger());
    *commandSender = reinterpret_cast<uint64_t>(commandSenderObj);
    return CHIP_NO_ERROR.AsInteger();
}
}
