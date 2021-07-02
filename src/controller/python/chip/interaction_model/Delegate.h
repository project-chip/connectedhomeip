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

#pragma once

#include <type_traits>

#include <app/InteractionModelDelegate.h>
#include <controller/CHIPDeviceController.h>

namespace chip {
namespace Controller {

// The command status will be used for python script.
// use packed attribute so we can unpack it from python and no need to worry about padding.
struct __attribute__((packed)) CommandStatus
{
    uint32_t ProtocolId;
    uint16_t ProtocolCode;
    chip::EndpointId EndpointId;
    chip::ClusterId ClusterId;
    chip::CommandId CommandId;
    uint8_t CommandIndex;
};

static_assert(std::is_same<chip::EndpointId, uint16_t>::value && std::is_same<chip::ClusterId, uint32_t>::value &&
                  std::is_same<chip::CommandId, uint32_t>::value,
              "Members in CommandStatus does not match interaction_model/delegate.py");
static_assert(sizeof(CommandStatus) == 4 + 2 + 2 + 4 + 4 + 1, "Size of CommandStatus might contain padding");

extern "C" {
typedef void (*PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct)(uint64_t commandSenderPtr,
                                                                                        void * commandStatusBuf,
                                                                                        uint32_t commandStatusBufLen);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct)(uint64_t commandSenderPtr, uint8_t commandIndex);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseFunct)(uint64_t commandSenderPtr, uint32_t error);

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f);
}

class PythonInteractionModelDelegate : public chip::Controller::DeviceControllerInteractionModelDelegate
{
public:
    CHIP_ERROR CommandResponseStatus(const app::CommandSender * apCommandSender,
                                     const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, const uint32_t aProtocolId,
                                     const uint16_t aProtocolCode, chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
                                     chip::CommandId aCommandId, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseProtocolError(const app::CommandSender * apCommandSender, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override;

    CHIP_ERROR CommandResponseProcessed(const app::CommandSender * apCommandSender) override;

    static PythonInteractionModelDelegate & Instance();

    void SetOnCommandResponseStatusCodeReceivedCallback(PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f)
    {
        commandResponseStatusFunct = f;
    }

    void SetOnCommandResponseProtocolErrorCallback(PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f)
    {
        commandResponseProtocolErrorFunct = f;
    }

    void SetOnCommandResponseCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f) { commandResponseErrorFunct = f; }

private:
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct commandResponseStatusFunct   = nullptr;
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct commandResponseProtocolErrorFunct = nullptr;
    PythonInteractionModelDelegate_OnCommandResponseFunct commandResponseErrorFunct                      = nullptr;
};

} // namespace Controller
} // namespace chip
