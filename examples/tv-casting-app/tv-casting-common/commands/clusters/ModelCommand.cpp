/*
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <commands/clusters/ModelCommand.h>

#include <CastingServer.h>
#include <app/InteractionModelEngine.h>
#include <inttypes.h>

using namespace ::chip;

CHIP_ERROR ModelCommand::RunCommand()
{
    FabricIndex fabricIndex = CastingServer::GetInstance()->CurrentFabricIndex();

    if (mDestinationId == 0)
    {
        ChipLogProgress(chipTool, "nodeId set to 0, using default for fabric %d", fabricIndex);
        mDestinationId = CastingServer::GetInstance()->GetVideoPlayerNodeForFabricIndex(fabricIndex);
    }
    else
    {
        // potentially change fabric index if this is not the right one for the given nodeId
        fabricIndex = CastingServer::GetInstance()->GetVideoPlayerFabricIndexForNode(mDestinationId);
    }
    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, mDestinationId);

    if (IsGroupId(mDestinationId))
    {
        ChipLogProgress(chipTool, "Sending command to group 0x%x", GroupIdFromNodeId(mDestinationId));

        return SendGroupCommand(GroupIdFromNodeId(mDestinationId), fabricIndex);
    }

    Server * server = &(chip::Server::GetInstance());
    server->GetCASESessionManager()->FindOrEstablishSession(ScopedNodeId(mDestinationId, fabricIndex), &mOnDeviceConnectedCallback,
                                                            &mOnDeviceConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void ModelCommand::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                       const SessionHandle & sessionHandle)
{
    ChipLogProgress(chipTool, "ModelCommand::OnDeviceConnectedFn");
    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));

    OperationalDeviceProxy device(&exchangeMgr, sessionHandle);
    CHIP_ERROR err = command->SendCommand(&device, command->mEndPointId);
    VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
}

void ModelCommand::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(chipTool, "ModelCommand::OnDeviceConnectionFailureFn");
    LogErrorOnFailure(err);

    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}

void ModelCommand::Shutdown()
{
    ResetArguments();
    mOnDeviceConnectedCallback.Cancel();
    mOnDeviceConnectionFailureCallback.Cancel();
}
