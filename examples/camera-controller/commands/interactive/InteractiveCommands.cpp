/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include <device-manager/DeviceManager.h>
#include <platform/logging/LogV.h>
#include <system/SystemClock.h>

#include <editline.h>

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>

using namespace chip;

namespace {

constexpr char kInteractiveModePrompt[]          = ">>> ";
constexpr char kInteractiveModeHistoryFileName[] = "chip_tool_history";
constexpr char kInteractiveModeStopCommand[]     = "quit()";
constexpr char kCategoryError[]                  = "Error";
constexpr char kCategoryProgress[]               = "Info";
constexpr char kCategoryDetail[]                 = "Debug";
constexpr char kCategoryAutomation[]             = "Automation";

// File pointer for the log file
FILE * sLogFile = nullptr;

// Global flag to signal shutdown
std::atomic<bool> sShutdownRequested(false);

std::queue<std::string> sCommandQueue;
std::mutex sQueueMutex;
std::condition_variable sQueueCondition;

void ReadCommandThread()
{
    for (;;)
    {
        // `readline()` allocates with `malloc()`.  Wrap it in a smart
        // pointer so we cannot leak it on every early–return/throw path.
        std::unique_ptr<char, decltype(&std::free)> rawLine{ readline(kInteractiveModePrompt), &std::free };

        if (!rawLine) // EOF or fatal error → shut down cleanly
        {
            std::lock_guard<std::mutex> lk{ sQueueMutex };
            sCommandQueue.emplace(kInteractiveModeStopCommand);
            sQueueCondition.notify_one();
            break;
        }

        // Ignore empty lines produced by just hitting <Enter>.
        if (*rawLine == '\0')
            continue;

        // Copy into an owning `std::string` before the pointer vanishes.
        std::string line{ rawLine.get() };

        {
            std::lock_guard<std::mutex> lk{ sQueueMutex };
            sCommandQueue.push(line);
        }
        sQueueCondition.notify_one();

        // Bail out when the user asks to quit.
        if (line == kInteractiveModeStopCommand)
        {
            ChipLogProgress(NotSpecified, "ReadCommandThread exit on quit");
            break;
        }
    }
}

void OpenLogFile(const char * filePath)
{
    sLogFile = fopen(filePath, "a");
    if (sLogFile == nullptr)
    {
        perror("Failed to open log file");
    }
}

void CloseLogFile()
{
    if (sLogFile != nullptr)
    {
        fclose(sLogFile);
        sLogFile = nullptr;
    }
}

void ClearLine()
{
    printf("\r\x1B[0J"); // Move cursor to the beginning of the line and clear from cursor to end of the screen
}

void ENFORCE_FORMAT(3, 0) LoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    if (sLogFile == nullptr)
    {
        return;
    }

    uint64_t timeMs       = System::SystemClock().GetMonotonicMilliseconds64().count();
    uint64_t seconds      = timeMs / 1000;
    uint64_t milliseconds = timeMs % 1000;

    flockfile(sLogFile);

    fprintf(sLogFile, "[%llu.%06llu] CHIP:%s: ", static_cast<unsigned long long>(seconds),
            static_cast<unsigned long long>(milliseconds), module);
    vfprintf(sLogFile, msg, args);
    fprintf(sLogFile, "\n");
    fflush(sLogFile);

    funlockfile(sLogFile);
}

} // namespace

std::string InteractiveStartCommand::GetCommand() const
{
    std::unique_lock<std::mutex> lock(sQueueMutex);

    // Wait until queue is not empty OR shutdown is requested
    sQueueCondition.wait(lock, [&] { return !sCommandQueue.empty() || sShutdownRequested.load(); });

    if (sShutdownRequested.load())
    {
        return {}; // empty string signals caller to exit
    }

    std::string command = sCommandQueue.front();
    sCommandQueue.pop();

    if (!command.empty())
    {
        add_history(command.c_str());
        write_history(GetHistoryFilePath().c_str());
    }

    return command;
}

std::string InteractiveStartCommand::GetHistoryFilePath() const
{
    std::string storageDir;
    if (GetStorageDirectory().HasValue())
    {
        storageDir = GetStorageDirectory().Value();
    }
    else
    {
        // Match what GetFilename in ExamplePersistentStorage.cpp does.
        const char * dir = getenv("TMPDIR");
        if (dir == nullptr)
        {
            dir = "/tmp";
        }
        storageDir = dir;
    }

    return storageDir + "/" + kInteractiveModeHistoryFileName;
}

CHIP_ERROR InteractiveStartCommand::RunCommand()
{
    read_history(GetHistoryFilePath().c_str());

    if (mLogFilePath.HasValue())
    {
        OpenLogFile(mLogFilePath.Value());

        // Redirect logs to the custom logging callback
        Logging::SetLogRedirectCallback(LoggingCallback);
    }

    std::thread readCommands(ReadCommandThread);
    readCommands.detach();

    int status;
    while (true)
    {
        std::string command = GetCommand();
        if (command.empty())
        {
            break;
        }

        if (!ParseCommand(command, &status))
        {
            break;
        }
    }

    camera::DeviceManager::Instance().Shutdown();
    SetCommandExitStatus(CHIP_NO_ERROR);
    CloseLogFile();

    return CHIP_NO_ERROR;
}

bool InteractiveCommand::ParseCommand(const std::string & command, int * status)
{
    if (command == kInteractiveModeStopCommand)
    {
        // If scheduling the cleanup fails, there is not much we can do.
        // But if something went wrong while the application is leaving it could be because things have
        // not been cleaned up properly, so it is still useful to log the failure.
        LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(ExecuteDeferredCleanups, 0));
        return false;
    }

    ClearLine();

    *status = mHandler->RunInteractive(command.c_str(), GetStorageDirectory(), NeedsOperationalAdvertising());

    return true;
}

bool InteractiveCommand::NeedsOperationalAdvertising()
{
    return mAdvertiseOperational.ValueOr(true);
}

void PushCommand(const std::string & command)
{
    std::unique_lock<std::mutex> lock(sQueueMutex);

    ChipLogProgress(NotSpecified, "PushCommand: %s", command.c_str());
    sCommandQueue.push(command);
    sQueueCondition.notify_one();
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
    uint16_t mTimeout   = 0;
    int mStatus         = EXIT_SUCCESS;
    std::vector<std::string> mResults;
    std::vector<InteractiveServerResultLog> mLogs;

    std::mutex mMutex;

    void Setup(bool isAsyncReport, uint16_t timeout)
    {
        auto lock      = ScopedLock(mMutex);
        mEnabled       = true;
        mIsAsyncReport = isAsyncReport;
        mTimeout       = timeout;

        if (mIsAsyncReport && mTimeout)
        {
            chip::DeviceLayer::PlatformMgr().ScheduleWork(StartAsyncTimeout, reinterpret_cast<intptr_t>(this));
        }
    }

    void Reset()
    {
        auto lock = ScopedLock(mMutex);

        if (mIsAsyncReport && mTimeout)
        {
            chip::DeviceLayer::PlatformMgr().ScheduleWork(StopAsyncTimeout, reinterpret_cast<intptr_t>(this));
        }

        mEnabled       = false;
        mIsAsyncReport = false;
        mTimeout       = 0;
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
        case chip::Logging::kLogCategory_Automation:
            messageType = kCategoryAutomation;
            return;
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

        std::stringstream content;
        content << "{";

        content << "  \"results\": [";
        if (mResults.size())
        {
            for (const auto & result : mResults)
            {
                content << result << ",";
            }

            // Remove last comma.
            content.seekp(-1, std::ios_base::end);
        }

        if (mStatus != EXIT_SUCCESS)
        {
            if (mResults.size())
            {
                content << ",";
            }
            content << "{ \"error\": \"FAILURE\" }";
        }
        content << "],";

        content << "\"logs\": [";
        if (mLogs.size())
        {
            for (const auto & log : mLogs)
            {
                content << "{"
                           "  \"module\": \"" +
                        log.module +
                        "\","
                        "  \"category\": \"" +
                        log.messageType +
                        "\","
                        "  \"message\": \"" +
                        log.message +
                        "\""
                        "},";
            }

            // Remove last comma.
            content.seekp(-1, std::ios_base::end);
        }
        content << "]";

        content << "}";
        return content.str();
    }

    static void StartAsyncTimeout(intptr_t arg)
    {
        auto self    = reinterpret_cast<InteractiveServerResult *>(arg);
        auto timeout = chip::System::Clock::Seconds16(self->mTimeout);
        chip::DeviceLayer::SystemLayer().StartTimer(timeout, OnAsyncTimeout, self);
    }

    static void StopAsyncTimeout(intptr_t arg)
    {
        auto self = reinterpret_cast<InteractiveServerResult *>(arg);
        chip::DeviceLayer::SystemLayer().CancelTimer(OnAsyncTimeout, self);
    }

    static void OnAsyncTimeout(chip::System::Layer *, void * appState)
    {
        RemoteDataModelLogger::LogErrorAsJSON(CHIP_ERROR_TIMEOUT);
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

CHIP_ERROR InteractiveServerCommand::RunCommand()
{
    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(InteractiveServerLoggingCallback);

    RemoteDataModelLogger::SetDelegate(this);
    ReturnErrorOnFailure(mWebSocketServer.Run(mPort, this));

    gInteractiveServerResult.Reset();
    SetCommandExitStatus(CHIP_NO_ERROR);
    CloseLogFile();

    return CHIP_NO_ERROR;
}

bool InteractiveServerCommand::OnWebSocketMessageReceived(char * msg)
{
    bool isAsyncReport = strlen(msg) == 0;
    uint16_t timeout   = 0;
    if (!isAsyncReport && strlen(msg) <= 5 /* Only look for numeric values <= 65535 */)
    {
        std::stringstream ss;
        ss << msg;
        ss >> timeout;
        if (!ss.fail())
        {
            isAsyncReport = true;
        }
    }

    gInteractiveServerResult.Setup(isAsyncReport, timeout);
    VerifyOrReturnValue(!isAsyncReport, true);

    ChipLogProgress(NotSpecified, "OnWebSocketMessageReceived: %s", msg);
    auto shouldStop = ParseCommand(msg, &gInteractiveServerResult.mStatus);
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
    return shouldStop;
}

void InteractiveServerCommand::StopCommand()
{
    mWebSocketServer.Stop();
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

void StopInteractiveEventLoop()
{
    ChipLogProgress(NotSpecified, "Stop Interactive EventLoop, exiting...");

    sShutdownRequested.store(true);
    sQueueCondition.notify_one();

    InteractiveServerCommand * command =
        static_cast<InteractiveServerCommand *>(CommandMgr().GetCommandByName("interactive", "server"));
    if (command)
    {
        command->StopCommand();
    }
}
