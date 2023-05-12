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

class ScopedLock
{
public:
    ScopedLock(std::mutex & mutex) : mMutex(mutex) { mMutex.lock(); }

    ~ScopedLock() { mMutex.unlock(); }

private:
    std::mutex & mMutex;
};

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

    // The InteractiveServerResult instance (gInteractiveServerResult) is initially
    // accessed on the main thread in InteractiveServerCommand::RunCommand, which is
    // when chip-tool starts in 'interactive server' mode.
    //
    // Then command results are normally sent over the wire onto the main thread too
    // when a command is received over WebSocket in InteractiveServerCommand::OnWebSocketMessageReceived
    // which for most cases runs a command onto the chip thread and block until
    // it is resolved (or until it timeouts).
    //
    // But in the meantime, when some parts of the command result happens, it is appended
    // to the mResults vector onto the chip thread.
    //
    // For empty commands, which means that the test suite is *waiting* for some events
    // (e.g a subscription report), the command results are sent over the chip thread
    // (this is the isAsyncReport use case).
    //
    // Finally, logs can be appended from either the chip thread or the main thread.
    //
    // This class should be refactored to abstract that properly and reduce the scope of
    // of the mutex, but in the meantime, the access to the members of this class are
    // protected by a mutex.
    std::mutex mMutex;

    void Setup(bool isAsyncReport)
    {
        auto lock      = ScopedLock(mMutex);
        mEnabled       = true;
        mIsAsyncReport = isAsyncReport;
    }

    void Reset()
    {
        auto lock      = ScopedLock(mMutex);
        mEnabled       = false;
        mIsAsyncReport = false;
        mStatus        = EXIT_SUCCESS;
        mResults.clear();
        mLogs.clear();
    }

    bool IsAsyncReport()
    {
        auto lock = ScopedLock(mMutex);
        return mIsAsyncReport;
    }

    void MaybeAddLog(const char * module, uint8_t category, const char * base64Message)
    {
        auto lock = ScopedLock(mMutex);
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
        default:
            // This should not happen.
            chipDie();
            break;
        }

        mLogs.push_back(InteractiveServerResultLog({ module, base64Message, messageType }));
    }

    void MaybeAddResult(const char * result)
    {
        auto lock = ScopedLock(mMutex);
        VerifyOrReturn(mEnabled);

        mResults.push_back(result);
    }

    std::string AsJsonString()
    {
        auto lock = ScopedLock(mMutex);

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

bool InteractiveServerCommand::OnWebSocketMessageReceived(char * msg)
{
    bool isAsyncReport = strlen(msg) == 0;
    gInteractiveServerResult.Setup(isAsyncReport);
    VerifyOrReturnValue(!isAsyncReport, true);

    auto shouldStop = ParseCommand(msg, &gInteractiveServerResult.mStatus);
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
    return shouldStop;
}

CHIP_ERROR InteractiveServerCommand::LogJSON(const char * json)
{
    gInteractiveServerResult.MaybeAddResult(json);
    if (gInteractiveServerResult.IsAsyncReport())
    {
        mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
        gInteractiveServerResult.Reset();
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
        // If scheduling the cleanup fails, there is not much we can do.
        // But if something went wrong while the application is leaving it could be because things have
        // not been cleaned up properly, so it is still useful to log the failure.
        LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(ExecuteDeferredCleanups, 0));
        return false;
    }

    ClearLine();

    *status = mHandler->RunInteractive(command, GetStorageDirectory());

    return true;
}

bool InteractiveCommand::NeedsOperationalAdvertising()
{
    return mAdvertiseOperational.ValueOr(true);
}
