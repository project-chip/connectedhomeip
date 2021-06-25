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

#include "TestCommand.h"

CHIP_ERROR TestCommand::Run()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * ctx = GetExecContext();

    err = ctx->commissioner->GetConnectedDevice(ctx->remoteId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    ReturnErrorOnFailure(err);

    return CHIP_NO_ERROR;
}

void TestCommand::OnDeviceConnectedFn(void * context, chip::Controller::Device * device)
{
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Device connected, but cannot run the test, as the context is null"));
    command->mDevice = device;
    command->NextTest();
}

void TestCommand::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(chipTool, "Failed in connecting to the device %" PRIu64 ". Error %d", deviceId, error);
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Test command context is null"));
    command->SetCommandExitStatus(error);
}
