/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/ConcreteAttributePath.h"
#include <cinttypes>

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <controller/python/chip/interaction_model/Delegate.h>
#include <controller/python/chip/native/PyChipError.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app;
using namespace chip::Controller;

namespace chip {

namespace Controller {

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f)
{}

void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f)
{}

void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f) {}

void pychip_InteractionModelDelegate_SetOnWriteResponseStatusCallback(PythonInteractionModelDelegate_OnWriteResponseStatusFunct f)
{}

} // namespace Controller
} // namespace chip

extern "C" {

PyChipError pychip_InteractionModel_GetCommandSenderHandle(uint64_t * commandSender)
{
    chip::app::CommandSender * commandSenderObj = nullptr;
    VerifyOrReturnError(commandSender != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
    commandSenderObj = new (std::nothrow) chip::app::CommandSender(nullptr, nullptr);
    VerifyOrReturnError(commandSenderObj != nullptr, ToPyChipError((CHIP_ERROR_NO_MEMORY)));
    *commandSender = reinterpret_cast<uint64_t>(commandSenderObj);
    return ToPyChipError(CHIP_NO_ERROR);
}
}
