/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "WaitForCommissioneeCommand.h"

using namespace chip;

CHIP_ERROR WaitForCommissioneeCommand::RunCommand()
{
    chip::FabricIndex fabricIndex = CurrentCommissioner().GetFabricIndex();
    ReturnErrorCodeIf(fabricIndex == chip::kUndefinedFabricIndex, CHIP_ERROR_INCORRECT_STATE);

    if (mExpireExistingSession.ValueOr(true))
    {
        CurrentCommissioner().SessionMgr()->ExpireAllSessions(chip::ScopedNodeId(mNodeId, fabricIndex));
    }

    return CurrentCommissioner().GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void WaitForCommissioneeCommand::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                     const SessionHandle & sessionHandle)
{
    auto * command = reinterpret_cast<WaitForCommissioneeCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));
    command->SetCommandExitStatus(CHIP_NO_ERROR);
}

void WaitForCommissioneeCommand::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    auto * command = reinterpret_cast<WaitForCommissioneeCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}
