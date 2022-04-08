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

#include <controller/CHIPDeviceController.h>

namespace chip {
namespace python {
static constexpr ClusterStatus kUndefinedClusterStatus = 0xFF;
}
namespace Controller {

// The command status will be used for python script.
// use packed attribute so we can unpack it from python and no need to worry about padding.
// This struct needs to match the IMCommandStatus definition in delegate.py
struct __attribute__((packed)) CommandStatus
{
    Protocols::InteractionModel::Status status;
    chip::ClusterStatus clusterStatus;
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    uint8_t commandIndex;
};

static_assert(std::is_same<chip::EndpointId, uint16_t>::value && std::is_same<chip::ClusterId, uint32_t>::value &&
                  std::is_same<chip::CommandId, uint32_t>::value,
              "Members in CommandStatus does not match interaction_model/delegate.py");
static_assert(sizeof(CommandStatus) == 1 + 1 + 2 + 4 + 4 + 1, "Size of CommandStatus might contain padding");

struct __attribute__((packed)) AttributePath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::FieldId fieldId;
};

static_assert(std::is_same<chip::EndpointId, uint16_t>::value && std::is_same<chip::ClusterId, uint32_t>::value &&
                  std::is_same<chip::FieldId, uint32_t>::value,
              "Members in AttributePath does not match interaction_model/delegate.py");
static_assert(sizeof(AttributePath) == 2 + 4 + 4, "AttributePath might contain padding");

// This struct needs to match the IMWriteStatus definition in delegate.py
struct __attribute__((packed)) AttributeWriteStatus
{
    chip::NodeId nodeId;
    uint64_t appIdentifier;
    Protocols::InteractionModel::Status status;
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::FieldId fieldId;
};
static_assert(std::is_same<chip::EndpointId, uint16_t>::value && std::is_same<chip::ClusterId, uint32_t>::value &&
                  std::is_same<chip::FieldId, uint32_t>::value,
              "Members in AttributeWriteStatus does not match interaction_model/delegate.py");
static_assert(sizeof(AttributeWriteStatus) == 8 + 8 + 1 + 2 + 4 + 4, "Size of AttributeWriteStatus might contain padding");

extern "C" {
typedef void (*PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct)(uint64_t commandSenderPtr,
                                                                                        void * commandStatusBuf,
                                                                                        uint32_t commandStatusBufLen);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct)(uint64_t commandSenderPtr, uint8_t commandIndex);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseFunct)(uint64_t commandSenderPtr, uint32_t error);

typedef void (*PythonInteractionModelDelegate_OnWriteResponseStatusFunct)(void * writeStatusBuf, uint32_t writeStatusBufLen);

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f);
void pychip_InteractionModelDelegate_SetOnWriteResponseStatusCallback(PythonInteractionModelDelegate_OnWriteResponseStatusFunct f);
}

} // namespace Controller
} // namespace chip
