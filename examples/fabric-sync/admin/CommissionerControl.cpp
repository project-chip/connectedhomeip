/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "CommissionerControl.h"
#include "DeviceManager.h"

using namespace ::chip;

namespace admin {

void CommissionerControl::Init(Controller::DeviceCommissioner & commissioner, NodeId nodeId, EndpointId endpointId)
{
    // Ensure that mCommissioner is not already initialized
    VerifyOrDie(mCommissioner == nullptr);

    ChipLogProgress(NotSpecified, "Initilize CommissionerControl");
    mCommissioner  = &commissioner;
    mDestinationId = nodeId;
    mEndpointId    = endpointId;
}

CHIP_ERROR CommissionerControl::RequestCommissioningApproval(uint64_t requestId, uint16_t vendorId, uint16_t productId,
                                                             Optional<CharSpan> label)
{
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(NotSpecified, "Sending RequestCommissioningApproval to node " ChipLogFormatX64,
                    ChipLogValueX64(mDestinationId));

    mRequestCommissioningApproval.requestID = requestId;
    mRequestCommissioningApproval.vendorID  = static_cast<VendorId>(vendorId);
    mRequestCommissioningApproval.productID = productId;

    if (label.HasValue())
    {
        VerifyOrReturnError(label.Value().size() <= kMaxDeviceLabelLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(mLabelBuffer, label.Value().data(), label.Value().size());
        mRequestCommissioningApproval.label = Optional<Span<const char>>(CharSpan(mLabelBuffer, label.Value().size()));
    }

    mCommandType = CommandType::kRequestCommissioningApproval;
    return mCommissioner->GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

CHIP_ERROR CommissionerControl::CommissionNode(uint64_t requestId, uint16_t responseTimeoutSeconds)
{
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(NotSpecified, "Sending CommissionNode to node " ChipLogFormatX64, ChipLogValueX64(mDestinationId));

    mCommissionNode.requestID              = requestId;
    mCommissionNode.responseTimeoutSeconds = responseTimeoutSeconds;

    mCommandType = CommandType::kCommissionNode;
    return mCommissioner->GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void CommissionerControl::OnResponse(app::CommandSender * client, const app::ConcreteCommandPath & path,
                                     const app::StatusIB & status, TLV::TLVReader * data)
{
    ChipLogProgress(NotSpecified, "CommissionerControl: OnResponse.");

    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(NotSpecified, "Response Failure: %s", ErrorStr(error));
        return;
    }

    if (data != nullptr)
    {
        DeviceManager::Instance().HandleCommandResponse(path, *data);
    }
}

void CommissionerControl::OnError(const app::CommandSender * client, CHIP_ERROR error)
{
    // Handle the error, then reset mCommandSender
    ChipLogProgress(NotSpecified, "CommissionerControl: OnError: Error: %s", ErrorStr(error));
}

void CommissionerControl::OnDone(app::CommandSender * client)
{
    ChipLogProgress(NotSpecified, "CommissionerControl: OnDone.");

    switch (mCommandType)
    {
    case CommandType::kRequestCommissioningApproval:
        ChipLogProgress(NotSpecified, "CommissionerControl: Command RequestCommissioningApproval has been successfully processed.");
        break;

    case CommandType::kCommissionNode:
        ChipLogProgress(NotSpecified, "CommissionerControl: Command CommissionNode has been successfully processed.");
        break;

    default:
        ChipLogError(NotSpecified, "CommissionerControl: Unknown or unhandled command type in OnDone.");
        break;
    }

    // Reset command type to undefined after processing is done
    mCommandType = CommandType::kUndefined;

    // Ensure that mCommandSender is cleaned up after it is done
    mCommandSender.reset();
}

CHIP_ERROR CommissionerControl::SendCommandForType(CommandType commandType, DeviceProxy * device)
{
    ChipLogProgress(AppServer, "Sending command with Endpoint ID: %d, Command Type: %d", mEndpointId,
                    static_cast<int>(commandType));

    switch (commandType)
    {
    case CommandType::kRequestCommissioningApproval:
        return SendCommand(device, mEndpointId, app::Clusters::CommissionerControl::Id,
                           app::Clusters::CommissionerControl::Commands::RequestCommissioningApproval::Id,
                           mRequestCommissioningApproval);
    case CommandType::kCommissionNode:
        return SendCommand(device, mEndpointId, app::Clusters::CommissionerControl::Id,
                           app::Clusters::CommissionerControl::Commands::CommissionNode::Id, mCommissionNode);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void CommissionerControl::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                              const SessionHandle & sessionHandle)
{
    CommissionerControl * self = reinterpret_cast<CommissionerControl *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));

    OperationalDeviceProxy device(&exchangeMgr, sessionHandle);

    CHIP_ERROR err = self->SendCommandForType(self->mCommandType, &device);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send CommissionerControl command.");
        self->OnDone(nullptr);
    }
}

void CommissionerControl::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    CommissionerControl * self = reinterpret_cast<CommissionerControl *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    self->OnDone(nullptr);
}

} // namespace admin
