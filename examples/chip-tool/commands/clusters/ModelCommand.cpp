/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "ModelCommand.h"

#include <app/InteractionModelEngine.h>
#include <app/icd/client/DefaultICDClientStorage.h>
#include <inttypes.h>

using namespace ::chip;

CHIP_ERROR ModelCommand::RunCommand()
{

    if (IsGroupId(mDestinationId))
    {
        FabricIndex fabricIndex = CurrentCommissioner().GetFabricIndex();
        ChipLogProgress(chipTool, "Sending command to group 0x%x", GroupIdFromNodeId(mDestinationId));

        return SendGroupCommand(GroupIdFromNodeId(mDestinationId), fabricIndex);
    }

    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, mDestinationId);
    CheckPeerICDType();

    CommissioneeDeviceProxy * commissioneeDeviceProxy = nullptr;
    if (CHIP_NO_ERROR == CurrentCommissioner().GetDeviceBeingCommissioned(mDestinationId, &commissioneeDeviceProxy))
    {
        return SendCommand(commissioneeDeviceProxy, mEndPointId);
    }

    // Check whether the session needs to allow large payload support.
    TransportPayloadCapability transportPayloadCapability =
        AllowLargePayload() ? TransportPayloadCapability::kLargePayload : TransportPayloadCapability::kMRPPayload;
    return CurrentCommissioner().GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback,
                                                    &mOnDeviceConnectionFailureCallback, transportPayloadCapability);
}

void ModelCommand::OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                       const chip::SessionHandle & sessionHandle)
{
    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));

    chip::OperationalDeviceProxy device(&exchangeMgr, sessionHandle);
    CHIP_ERROR err = command->SendCommand(&device, command->mEndPointId);
    VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
}

void ModelCommand::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}

void ModelCommand::Shutdown()
{
    mOnDeviceConnectedCallback.Cancel();
    mOnDeviceConnectionFailureCallback.Cancel();

    CHIPCommand::Shutdown();
}

void ModelCommand::ClearICDEntry(const ScopedNodeId & nodeId)
{
    CHIP_ERROR deleteEntryError = CHIPCommand::sICDClientStorage.DeleteEntry(nodeId);
    if (deleteEntryError != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Failed to delete ICD entry: %" CHIP_ERROR_FORMAT, deleteEntryError.Format());
    }
}

void ModelCommand::StoreICDEntryWithKey(app::ICDClientInfo & clientInfo, ByteSpan key)
{
    CHIP_ERROR err = CHIPCommand::sICDClientStorage.SetKey(clientInfo, key);
    if (err == CHIP_NO_ERROR)
    {
        err = CHIPCommand::sICDClientStorage.StoreEntry(clientInfo);
    }

    if (err != CHIP_NO_ERROR)
    {
        CHIPCommand::sICDClientStorage.RemoveKey(clientInfo);
        ChipLogError(chipTool, "Failed to persist symmetric key with error: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }
}

void ModelCommand::CheckPeerICDType()
{
    if (mIsPeerLIT.HasValue())
    {
        ChipLogProgress(chipTool, "Peer ICD type is set to %s", mIsPeerLIT.Value() == 1 ? "LIT-ICD" : "non LIT-ICD");
        return;
    }

    app::ICDClientInfo info;
    auto destinationPeerId = chip::ScopedNodeId(mDestinationId, CurrentCommissioner().GetFabricIndex());
    auto iter              = CHIPCommand::sICDClientStorage.IterateICDClientInfo();
    if (iter == nullptr)
    {
        return;
    }
    app::DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iter);

    while (iter->Next(info))
    {
        if (ScopedNodeId(info.peer_node.GetNodeId(), info.peer_node.GetFabricIndex()) == destinationPeerId)
        {
            ChipLogProgress(chipTool, "Peer is a registered LIT ICD.");
            mIsPeerLIT.SetValue(true);
            return;
        }
    }
}

bool ModelCommand::IsPeerLIT()
{
    CheckPeerICDType();
    return mIsPeerLIT.ValueOr(false);
}

bool ModelCommand::AllowLargePayload()
{
    return mAllowLargePayload.ValueOr(false);
}
