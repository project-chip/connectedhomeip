/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "ModelCommand.h"

#include <app/InteractionModelEngine.h>
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

    CommissioneeDeviceProxy * commissioneeDeviceProxy = nullptr;
    if (CHIP_NO_ERROR == CurrentCommissioner().GetDeviceBeingCommissioned(mDestinationId, &commissioneeDeviceProxy))
    {
        return SendCommand(commissioneeDeviceProxy, mEndPointId);
    }

    return CurrentCommissioner().GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback,
                                                    &mOnDeviceConnectionFailureCallback);
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
