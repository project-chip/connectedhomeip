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

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseStatus(const CommandSender * apCommandSender,
                                                                 const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                                                 const uint32_t aProtocolId, const uint16_t aProtocolCode,
                                                                 chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
                                                                 chip::CommandId aCommandId, uint8_t aCommandIndex)
{
    CommandStatus status{ aProtocolId, aProtocolCode, aEndpointId, aClusterId, aCommandId, aCommandIndex };
    if (commandResponseStatusFunct != nullptr)
    {
        commandResponseStatusFunct(reinterpret_cast<uint64_t>(apCommandSender), &status, sizeof(status));
    }
    // For OpCred callbacks.
    DeviceControllerInteractionModelDelegate::CommandResponseStatus(apCommandSender, aGeneralCode, aProtocolId, aProtocolCode,
                                                                    aEndpointId, aClusterId, aCommandId, aCommandIndex);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseProtocolError(const CommandSender * apCommandSender,
                                                                        uint8_t aCommandIndex)
{
    if (commandResponseProtocolErrorFunct != nullptr)
    {
        commandResponseProtocolErrorFunct(reinterpret_cast<uint64_t>(apCommandSender), aCommandIndex);
    }
    DeviceControllerInteractionModelDelegate::CommandResponseProtocolError(apCommandSender, aCommandIndex);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    if (commandResponseErrorFunct != nullptr)
    {
        commandResponseErrorFunct(reinterpret_cast<uint64_t>(apCommandSender), aError.AsInteger());
    }
    if (aError != CHIP_NO_ERROR)
    {
        DeviceControllerInteractionModelDelegate::CommandResponseError(apCommandSender, aError);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseProcessed(const app::CommandSender * apCommandSender)
{
    this->CommandResponseError(apCommandSender, CHIP_NO_ERROR);
    DeviceControllerInteractionModelDelegate::CommandResponseProcessed(apCommandSender);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::WriteResponseStatus(const app::WriteClient * apWriteClient,
                                                               const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                                               const uint32_t aProtocolId, const uint16_t aProtocolCode,
                                                               app::AttributePathParams & aAttributePathParams,
                                                               uint8_t aCommandIndex)
{
    if (onWriteResponseFunct != nullptr)
    {
        AttributeWriteStatus status{ apWriteClient->GetSourceNodeId(),
                                     apWriteClient->GetAppIdentifier(),
                                     aProtocolId,
                                     aProtocolCode,
                                     aAttributePathParams.mEndpointId,
                                     aAttributePathParams.mClusterId,
                                     aAttributePathParams.mFieldId };
        onWriteResponseFunct(&status, sizeof(status));
    }
    DeviceControllerInteractionModelDelegate::WriteResponseStatus(apWriteClient, aGeneralCode, aProtocolId, aProtocolCode,
                                                                  aAttributePathParams, aCommandIndex);
    return CHIP_NO_ERROR;
}

void PythonInteractionModelDelegate::OnReportData(const app::ReadClient * apReadClient, const app::ClusterInfo & aPath,
                                                  TLV::TLVReader * apData, Protocols::InteractionModel::ProtocolCode status)
{
    if (onReportDataFunct != nullptr)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TLV::TLVWriter writer;
        uint8_t writerBuffer[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
        writer.Init(writerBuffer);
        // When the apData is nullptr, means we did not receive a valid attribute data from server, status will be some error
        // status.
        if (apData != nullptr)
        {
            TLV::TLVReader tmpReader;
            tmpReader.Init(*apData);
            // The Copy operation should succeed since:
            // - We used a buffer that is large enough
            // - The writer is in a clean state.
            err = writer.CopyElement(TLV::AnonymousTag, tmpReader);
        }
        if (CHIP_NO_ERROR == err)
        {
            AttributePath path{ .endpointId = aPath.mEndpointId, .clusterId = aPath.mClusterId, .fieldId = aPath.mFieldId };
            onReportDataFunct(apReadClient->GetExchangeContext()->GetSecureSession().GetPeerNodeId(),
                              apReadClient->GetAppIdentifier(), &path, sizeof(path), writerBuffer, writer.GetLengthWritten(),
                              to_underlying(status));
        }
        else
        {
            // We failed to dump the TLV data to buffer, so we cannot pass valid data to the Python side, this should be a internal
            // error of the binding.
            ChipLogError(Controller, "Cannot pass TLV data to python: failed to copy TLV: %s", ErrorStr(err));
        }
    }
    DeviceControllerInteractionModelDelegate::OnReportData(apReadClient, aPath, apData, status);
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

void pychip_InteractionModelDelegate_SetOnReportDataCallback(PythonInteractionModelDelegate_OnReportDataFunct f)
{
    gPythonInteractionModelDelegate.SetOnReportDataCallback(f);
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
    CHIP_ERROR err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&commandSenderObj);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());
    *commandSender = reinterpret_cast<uint64_t>(commandSenderObj);
    return CHIP_NO_ERROR.AsInteger();
}
}
