/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "OpenCommissioningWindowCommand.h"

using namespace ::chip;

CHIP_ERROR OpenCommissioningWindowCommand::RunCommand()
{
    return CurrentCommissioner().GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void OpenCommissioningWindowCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device)
{
    OpenCommissioningWindowCommand * command = reinterpret_cast<OpenCommissioningWindowCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));
    command->OpenCommissioningWindow();
}
void OpenCommissioningWindowCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    OpenCommissioningWindowCommand * command = reinterpret_cast<OpenCommissioningWindowCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}

void OpenCommissioningWindowCommand::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err,
                                                                       chip::SetupPayload payload)
{
    LogErrorOnFailure(err);

    OpenCommissioningWindowCommand * command = reinterpret_cast<OpenCommissioningWindowCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnOpenCommissioningWindowCommand: context is null"));
    command->SetCommandExitStatus(err);
}

CHIP_ERROR OpenCommissioningWindowCommand::OpenCommissioningWindow()
{
    return CurrentCommissioner().OpenCommissioningWindowWithCallback(
        mNodeId, mTimeout, mIteration, mDiscriminator, mCommissioningWindowOption, &mOnOpenCommissioningWindowCallback,
        /* readVIDPIDAttributes */ true);
}
