/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include <commands/tests/TestCommand.h>

#include <CastingServer.h>

using namespace ::chip;

CHIP_ERROR TestCommand::RunCommand()
{
    NextTest();

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestCommand::WaitForCommissionee(const char * identity,
                                            const chip::app::Clusters::DelayCommands::Commands::WaitForCommissionee::Type & value)
{
    FabricIndex fabricIndex = CastingServer::GetInstance()->CurrentFabricIndex();

    auto destinationId = CastingServer::GetInstance()->GetVideoPlayerNodeForFabricIndex(fabricIndex);
    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, destinationId);

    Server * server           = &(chip::Server::GetInstance());
    const FabricInfo * fabric = server->GetFabricTable().FindFabricWithIndex(fabricIndex);
    if (fabric == nullptr)
    {
        ChipLogError(AppServer, "Did not find fabric for index %d", fabricIndex);
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    PeerId peerID = fabric->GetPeerIdForNode(destinationId);
    server->GetCASESessionManager()->FindOrEstablishSession(peerID, &mOnDeviceConnectedCallback,
                                                            &mOnDeviceConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void TestCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device)
{
    ChipLogProgress(chipTool, " **** Test Setup: Device Connected\n");
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Device connected, but cannot run the test, as the context is null"));

    // The current code assumes the identity is alpha. That does not means much in the context
    // of the tv-casting-app since there is a single identity.
    command->mDevices["alpha"] = device;

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

void TestCommand::ExitAsync(intptr_t context)
{
    auto testCommand = reinterpret_cast<TestCommand *>(context);
    testCommand->InteractionModel::Shutdown();
    testCommand->SetCommandExitStatus(CHIP_ERROR_INTERNAL);
}

void TestCommand::Exit(std::string message, CHIP_ERROR err)
{
    mContinueProcessing = false;

    LogEnd(message, err);

    if (CHIP_NO_ERROR == err)
    {
        InteractionModel::Shutdown();
        SetCommandExitStatus(err);
    }
    else
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(ExitAsync, reinterpret_cast<intptr_t>(this));
    }
}

CHIP_ERROR TestCommand::ContinueOnChipMainThread(CHIP_ERROR err)
{
    if (mContinueProcessing == false)
    {
        return CHIP_NO_ERROR;
    }

    if (CHIP_NO_ERROR == err)
    {
        chip::app::Clusters::DelayCommands::Commands::WaitForMs::Type value;
        value.ms = 0;
        return WaitForMs("", value);
    }

    Exit(chip::ErrorStr(err), err);
    return CHIP_NO_ERROR;
}
