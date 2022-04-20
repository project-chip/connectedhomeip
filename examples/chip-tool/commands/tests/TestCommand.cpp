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

CHIP_ERROR TestCommand::RunCommand()
{
    if (mPICSFilePath.HasValue())
    {
        PICS.SetValue(PICSBooleanReader::Read(mPICSFilePath.Value()));
    }

    NextTest();

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestCommand::WaitForCommissionee(chip::NodeId nodeId)
{
    chip::FabricIndex fabricIndex;

    ReturnErrorOnFailure(CurrentCommissioner().GetFabricIndex(&fabricIndex));

    //
    // There's a chance the commissionee may have rebooted before this call here as part of a test flow
    // or is just starting out fresh outright. Let's make sure we're not re-using any cached CASE sessions
    // that will now be stale and mismatched with the peer, causing subsequent interactions to fail.
    //
    CurrentCommissioner().SessionMgr()->ExpireAllPairings(chip::ScopedNodeId(nodeId, fabricIndex));

    return CurrentCommissioner().GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void TestCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device)
{
    ChipLogProgress(chipTool, " **** Test Setup: Device Connected\n");
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Device connected, but cannot run the test, as the context is null"));
    command->mDevices[command->GetIdentity()] = device;

    LogErrorOnFailure(command->ContinueOnChipMainThread(CHIP_NO_ERROR));
}

void TestCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error)
{
    ChipLogProgress(chipTool, " **** Test Setup: Device Connection Failure [deviceId=%" PRIu64 ". Error %" CHIP_ERROR_FORMAT "\n]",
                    peerId.GetNodeId(), error.Format());
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Test command context is null"));

    LogErrorOnFailure(command->ContinueOnChipMainThread(error));
}

void TestCommand::Exit(std::string message)
{
    ChipLogError(chipTool, " ***** Test Failure: %s\n", message.c_str());
    SetCommandExitStatus(CHIP_ERROR_INTERNAL);
}

void TestCommand::ThrowFailureResponse(CHIP_ERROR error)
{
    Exit(std::string("Expecting success response but got a failure response: ") + chip::ErrorStr(error));
}

void TestCommand::ThrowSuccessResponse()
{
    Exit("Expecting failure response but got a success response");
}
