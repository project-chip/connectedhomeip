/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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

    Server * server           = &(chip::Server::GetInstance());
    const FabricInfo * fabric = server->GetFabricTable().FindFabricWithIndex(fabricIndex);
    if (fabric == nullptr)
    {
        ChipLogError(AppServer, "Did not find fabric for index %d", fabricIndex);
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    PeerId peerID = fabric->GetPeerIdForNode(mDestinationId);
    server->GetCASESessionManager()->FindOrEstablishSession(peerID, &mOnDeviceConnectedCallback,
                                                            &mOnDeviceConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void ModelCommand::OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device)
{
    ChipLogProgress(chipTool, "ModelCommand::OnDeviceConnectedFn");
    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));

    CHIP_ERROR err = command->SendCommand(device, command->mEndPointId);
    VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
}

void ModelCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR err)
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
