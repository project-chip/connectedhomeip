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

#include "InteractiveCommands.h"

#include <platform/logging/LogV.h>

#include <editline.h>

constexpr const char * kInteractiveModePrompt          = ">>> ";
constexpr const char * kInteractiveModeHistoryFilePath = "/tmp/chip_tool_history";
constexpr const char * kInteractiveModeStopCommand     = "quit()";

namespace {

void ClearLine()
{
    printf("\r\x1B[0J"); // Move cursor to the beginning of the line and clear from cursor to end of the screen
}

void ENFORCE_FORMAT(3, 0) LoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    ClearLine();
    chip::Logging::Platform::LogV(module, category, msg, args);
    ClearLine();
}

char * GetCommand(char * command)
{
    if (command != nullptr)
    {
        free(command);
        command = nullptr;
    }

    command = readline(kInteractiveModePrompt);

    // Do not save empty lines
    if (command != nullptr && *command)
    {
        add_history(command);
        write_history(kInteractiveModeHistoryFilePath);
    }

    return command;
}
} // namespace

CHIP_ERROR InteractiveServerCommand::RunCommand()
{
    ReturnErrorOnFailure(mWebSocketServer.Run(mPort, this));

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

bool InteractiveServerCommand::OnWebSocketMessageReceived(char * msg)
{
    return ParseCommand(msg);
}

CHIP_ERROR InteractiveStartCommand::RunCommand()
{
    read_history(kInteractiveModeHistoryFilePath);

    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(LoggingCallback);

    char * command = nullptr;
    while (true)
    {
        command = GetCommand(command);
        if (command != nullptr && !ParseCommand(command))
        {
            break;
        }
    }

    if (command != nullptr)
    {
        free(command);
        command = nullptr;
    }

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

bool InteractiveCommand::ParseCommand(char * command)
{
    if (strcmp(command, kInteractiveModeStopCommand) == 0)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(ExecuteDeferredCleanups, 0);
        return false;
    }

    ClearLine();
    mHandler->RunInteractive(command);
    return true;
}
