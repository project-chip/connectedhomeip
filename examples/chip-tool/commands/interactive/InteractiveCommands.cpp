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
constexpr const char * kCategoryError                  = "Error";
constexpr const char * kCategoryProgress               = "Info";
constexpr const char * kCategoryDetail                 = "Debug";

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

struct InteractiveServerResultLog
{
    std::string module;
    std::string message;
    std::string messageType;
};

struct InteractiveServerResult
{
    bool mEnabled       = false;
    bool mIsAsyncReport = false;
    int mStatus         = EXIT_SUCCESS;
    std::vector<std::string> mResults;
    std::vector<InteractiveServerResultLog> mLogs;

    void Setup(bool isAsyncReport)
    {
        mEnabled       = true;
        mIsAsyncReport = isAsyncReport;
    }

    void Reset()
    {
        mEnabled       = false;
        mIsAsyncReport = false;
        mStatus        = EXIT_SUCCESS;
        mResults.clear();
        mLogs.clear();
    }

    bool IsAsyncReport() const { return mIsAsyncReport; }

    void MaybeAddLog(const char * module, uint8_t category, const char * base64Message)
    {
        VerifyOrReturn(mEnabled);

        const char * messageType = nullptr;
        switch (category)
        {
        case chip::Logging::kLogCategory_Error:
            messageType = kCategoryError;
            break;
        case chip::Logging::kLogCategory_Progress:
            messageType = kCategoryProgress;
            break;
        case chip::Logging::kLogCategory_Detail:
            messageType = kCategoryDetail;
            break;
        }

        mLogs.push_back(InteractiveServerResultLog({ module, base64Message, messageType }));
    }

    void MaybeAddResult(const char * result)
    {
        VerifyOrReturn(mEnabled);
        mResults.push_back(result);
    }

    std::string AsJsonString() const
    {
        std::string resultsStr;
        if (mResults.size())
        {
            for (const auto & result : mResults)
            {
                resultsStr = resultsStr + result + ",";
            }

            // Remove last comma.
            resultsStr.pop_back();
        }

        if (mStatus != EXIT_SUCCESS)
        {
            if (resultsStr.size())
            {
                resultsStr = resultsStr + ",";
            }
            resultsStr = resultsStr + "{ \"error\": \"FAILURE\" }";
        }

        std::string logsStr;
        if (mLogs.size())
        {
            for (const auto & log : mLogs)
            {
                logsStr = logsStr + "{";
                logsStr = logsStr + "  \"module\": \"" + log.module + "\",";
                logsStr = logsStr + "  \"category\": \"" + log.messageType + "\",";
                logsStr = logsStr + "  \"message\": \"" + log.message + "\"";
                logsStr = logsStr + "},";
            }

            // Remove last comma.
            logsStr.pop_back();
        }

        std::string jsonLog;
        jsonLog = jsonLog + "{";
        jsonLog = jsonLog + "  \"results\": [" + resultsStr + "],";
        jsonLog = jsonLog + "  \"logs\": [" + logsStr + "]";
        jsonLog = jsonLog + "}";

        return jsonLog;
    }
};

InteractiveServerResult gInteractiveServerResult;

void ENFORCE_FORMAT(3, 0) InteractiveServerLoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    chip::Logging::Platform::LogV(module, category, msg, args);

    char message[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(message, sizeof(message), msg, args_copy);
    va_end(args_copy);

    char base64Message[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE * 2] = {};
    chip::Base64Encode(chip::Uint8::from_char(message), static_cast<uint16_t>(strlen(message)), base64Message);

    gInteractiveServerResult.MaybeAddLog(module, category, base64Message);
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
    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(InteractiveServerLoggingCallback);

    RemoteDataModelLogger::SetDelegate(this);
    ReturnErrorOnFailure(mWebSocketServer.Run(mPort, this));

    gInteractiveServerResult.Reset();
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

void SendOverWebSocket(intptr_t context)
{
    auto server = reinterpret_cast<WebSocketServer *>(context);
    server->Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
}

bool InteractiveServerCommand::OnWebSocketMessageReceived(char * msg)
{
    bool isAsyncReport = strlen(msg) == 0;
    gInteractiveServerResult.Setup(isAsyncReport);
    VerifyOrReturnValue(!isAsyncReport, true);

    auto shouldStop = ParseCommand(msg, &gInteractiveServerResult.mStatus);
    chip::DeviceLayer::PlatformMgr().ScheduleWork(SendOverWebSocket, reinterpret_cast<intptr_t>(&mWebSocketServer));
    return shouldStop;
}

CHIP_ERROR InteractiveServerCommand::LogJSON(const char * json)
{
    gInteractiveServerResult.MaybeAddResult(json);
    if (gInteractiveServerResult.IsAsyncReport())
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(SendOverWebSocket, reinterpret_cast<intptr_t>(&mWebSocketServer));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractiveStartCommand::RunCommand()
{
    read_history(kInteractiveModeHistoryFilePath);

    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(LoggingCallback);

    char * command = nullptr;
    int status;
    while (true)
    {
        command = GetCommand(command);
        if (command != nullptr && !ParseCommand(command, &status))
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

bool InteractiveCommand::ParseCommand(char * command, int * status)
{
    if (strcmp(command, kInteractiveModeStopCommand) == 0)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(ExecuteDeferredCleanups, 0);
        return false;
    }

    ClearLine();

    *status = mHandler->RunInteractive(command);

    return true;
}
