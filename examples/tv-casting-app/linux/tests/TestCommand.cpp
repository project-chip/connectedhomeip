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

CHIP_ERROR TestCommand::RunCommand()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR TestCommand::WaitForCommissionee(const char * identity,
                                            const chip::app::Clusters::DelayCommands::Commands::WaitForCommissionee::Type & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void TestCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device) {}

void TestCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error) {}

void TestCommand::ExitAsync(intptr_t context) {}

void TestCommand::Exit(std::string message, CHIP_ERROR err) {}

CHIP_ERROR TestCommand::ContinueOnChipMainThread(CHIP_ERROR err)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
