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
#include <inttypes.h>

using namespace ::chip;

CHIP_ERROR ModelCommand::RunCommand()
{
    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, mNodeId);

    CHIP_ERROR err = mController.GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(chipTool, "Failed in initiating connection to the device: %" PRIu64 ", error %" CHIP_ERROR_FORMAT,
                              mNodeId, err.Format()));

exit:
    return err;
}

void ModelCommand::OnDeviceConnectedFn(void * context, chip::Controller::Device * device)
{
    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    VerifyOrReturn(command != nullptr,
                   ChipLogError(chipTool, "Device connected, but cannot send the command, as the context is null"));
    command->SendCommand(device, command->mEndPointId);
}

void ModelCommand::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ModelCommand * command = reinterpret_cast<ModelCommand *>(context);
    ChipLogError(chipTool, "Failed in connecting to the device %" PRIu64 ". Error %" CHIP_ERROR_FORMAT, deviceId, error.Format());
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "ModelCommand context is null"));
    command->SetCommandExitStatus(error);
}
