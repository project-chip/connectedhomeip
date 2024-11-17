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

#pragma once

#include <admin/PairingManager.h>

namespace commands {

class Command
{
public:
    virtual ~Command()              = default;
    virtual CHIP_ERROR RunCommand() = 0;
};

class CommandRegistry
{
public:
    static CommandRegistry & Instance()
    {
        static CommandRegistry instance;
        return instance;
    }

    void SetActiveCommand(std::unique_ptr<Command> command, uint32_t timeoutSeconds = 30);

    Command * GetActiveCommand() { return mActiveCommand.get(); }

    void ResetActiveCommand();

    bool IsCommandActive() const { return mActiveCommand != nullptr; }

private:
    CommandRegistry() = default;

    static void OnTimeout(chip::System::Layer * layer, void * appState)
    {
        // Callback function to reset the command when the timer expires
        static_cast<CommandRegistry *>(appState)->ResetActiveCommand();
    }

    std::unique_ptr<Command> mActiveCommand;
};

} // namespace commands
