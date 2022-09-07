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
#import <Matter/Matter.h>

#include <editline.h>
#include <iomanip>
#include <sstream>

char kInteractiveModeName[] = "";
constexpr const char * kInteractiveModePrompt = ">>> ";
constexpr uint8_t kInteractiveModeArgumentsMaxLength = 32;
constexpr const char * kInteractiveModeHistoryFilePath = "/tmp/darwin_framework_tool_history";
constexpr const char * kInteractiveModeStopCommand = "quit()";

namespace {

class RestartCommand : public CHIPCommandBridge {
public:
    RestartCommand()
        : CHIPCommandBridge("restart")
    {
    }

    CHIP_ERROR RunCommand() override
    {
        RestartCommissioners();
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }
};

class StopCommand : public CHIPCommandBridge {
public:
    StopCommand()
        : CHIPCommandBridge("stop")
    {
    }

    CHIP_ERROR RunCommand() override
    {
        StopCommissioners();
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }
};

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
} // namespace

char * GetCommand(char * command)
{
    if (command != nullptr) {
        free(command);
        command = nullptr;
    }

    command = readline(kInteractiveModePrompt);

    // Do not save empty lines
    if (command != nullptr && *command) {
        add_history(command);
        write_history(kInteractiveModeHistoryFilePath);
    }

    return command;
}

el_status_t RestartFunction()
{
    RestartCommand cmd;
    cmd.RunCommand();
    return CSstay;
}

el_status_t StopFunction()
{
    StopCommand cmd;
    cmd.RunCommand();
    return CSstay;
}

CHIP_ERROR InteractiveStartCommand::RunCommand()
{
    read_history(kInteractiveModeHistoryFilePath);

    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(LoggingCallback);

    el_bind_key(CTL('^'), RestartFunction);
    el_bind_key(CTL('_'), StopFunction);

    char * command = nullptr;
    while (YES) {
        command = GetCommand(command);
        if (command != nullptr && !ParseCommand(command)) {
            break;
        }
    }

    if (command != nullptr) {
        free(command);
        command = nullptr;
    }

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

bool InteractiveStartCommand::ParseCommand(char * command)
{
    if (strcmp(command, kInteractiveModeStopCommand) == 0) {
        ExecuteDeferredCleanups();
        return NO;
    }

    char * args[kInteractiveModeArgumentsMaxLength];
    args[0] = kInteractiveModeName;
    int argsCount = 1;
    std::string arg;

    std::stringstream ss(command);
    while (ss >> std::quoted(arg, '\'')) {
        if (argsCount == kInteractiveModeArgumentsMaxLength) {
            ChipLogError(chipTool, "Too many arguments. Ignoring.");
            return YES;
        }

        char * carg = new char[arg.size() + 1];
        strcpy(carg, arg.c_str());
        args[argsCount++] = carg;
    }

    ClearLine();
    mHandler->RunInteractive(argsCount, args);

    // Do not delete arg[0]
    while (--argsCount)
        delete[] args[argsCount];

    return YES;
}
