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
namespace python {
static constexpr ClusterStatus kUndefinedClusterStatus = 0xFF;
}
namespace Controller {

// The command status will be used for python script.
// use packed attribute so we can unpack it from python and no need to worry about padding.
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
static_assert(sizeof(CommandStatus) == 2 + 1 + 2 + 4 + 4 + 1, "Size of CommandStatus might contain padding");

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
static_assert(sizeof(AttributeWriteStatus) == 8 + 8 + 2 + 2 + 4 + 4, "Size of AttributeWriteStatus might contain padding");

extern "C" {
typedef void (*PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct)(uint64_t commandSenderPtr,
                                                                                        void * commandStatusBuf,
                                                                                        uint32_t commandStatusBufLen);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct)(uint64_t commandSenderPtr, uint8_t commandIndex);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseFunct)(uint64_t commandSenderPtr, uint32_t error);

typedef void (*PythonInteractionModelDelegate_OnWriteResponseStatusFunct)(void * writeStatusBuf, uint32_t writeStatusBufLen);

typedef void (*PythonInteractionModelDelegate_OnReportDataFunct)(chip::NodeId nodeId, uint64_t readClientAppIdentifier,
                                                                 uint64_t subscriptionId, void * attributePathBuf,
                                                                 size_t attributePathBufLen, uint8_t * readTlvData,
                                                                 size_t readTlvDataLen, uint16_t statusCode);

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f);
void pychip_InteractionModelDelegate_SetOnReportDataCallback(PythonInteractionModelDelegate_OnReportDataFunct f);
void pychip_InteractionModelDelegate_SetOnWriteResponseStatusCallback(PythonInteractionModelDelegate_OnWriteResponseStatusFunct f);
}

class PythonInteractionModelDelegate : public chip::Controller::DeviceControllerInteractionModelDelegate
{
public:
    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath, const app::StatusIB & aStatus,
                    TLV::TLVReader * aData) override;
    void OnError(const app::CommandSender * apCommandSender, const app::StatusIB & aStatus, CHIP_ERROR aError) override;

    void OnAttributeData(const app::ReadClient * apReadClient, const app::ConcreteDataAttributePath & aPath,
                         TLV::TLVReader * apData, const app::StatusIB & status) override;

    void OnEventData(const app::ReadClient * apReadClient, const app::EventHeader & aEventHeader, TLV::TLVReader * apData,
                     const app::StatusIB * apStatus) override
    {}

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

    void SetOnWriteResponseStatusCallback(PythonInteractionModelDelegate_OnWriteResponseStatusFunct f) { onWriteResponseFunct = f; }

    void SetOnReportDataCallback(PythonInteractionModelDelegate_OnReportDataFunct f) { onReportDataFunct = f; }

private:
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct commandResponseStatusFunct   = nullptr;
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct commandResponseProtocolErrorFunct = nullptr;
    PythonInteractionModelDelegate_OnCommandResponseFunct commandResponseErrorFunct                      = nullptr;
    PythonInteractionModelDelegate_OnReportDataFunct onReportDataFunct                                   = nullptr;
    PythonInteractionModelDelegate_OnWriteResponseStatusFunct onWriteResponseFunct                       = nullptr;
};

} // namespace Controller
} // namespace chip
