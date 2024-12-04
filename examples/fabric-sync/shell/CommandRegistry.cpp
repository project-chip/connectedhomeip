/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include "CommandRegistry.h"

#include <system/SystemLayer.h>

using namespace ::chip;

namespace commands {

void CommandRegistry::SetActiveCommand(std::unique_ptr<Command> command, uint32_t timeoutSeconds)
{
    mActiveCommand = std::move(command);

    // Cancel any previous timer to avoid multiple timers running simultaneously
    DeviceLayer::SystemLayer().CancelTimer(OnTimeout, this);

    // Start a new timer for the specified timeout
    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(timeoutSeconds * 1000), OnTimeout, this);
    if (err != CHIP_NO_ERROR)
    {
        ResetActiveCommand();
    }
}

void CommandRegistry::ResetActiveCommand()
{
    DeviceLayer::SystemLayer().CancelTimer(OnTimeout, this);
    mActiveCommand.reset();
}

} // namespace commands
