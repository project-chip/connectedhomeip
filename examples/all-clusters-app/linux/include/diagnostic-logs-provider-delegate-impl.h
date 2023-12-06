/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-provider-delegate.h>
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>

#include <filesystem>
#include <fstream>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

/**
 * The application delegate to statically define the options.
 */

class LogProvider : public LogProviderDelegate
{
    static LogSessionHandle sLogSessionHandle;
    static LogProvider sInstance;

public:
    LogSessionHandle StartLogCollection(IntentEnum logType) override;

    CHIP_ERROR GetNextChunk(LogSessionHandle logSessionHandle, MutableByteSpan & outBuffer, bool & outIsEOF) override;

    void EndLogCollection(LogSessionHandle logSessionHandle) override;

    void SetEndUserSupportLogFilePath(std::optional<std::string> logFilePath);

    void SetNetworkDiagnosticsLogFilePath(std::optional<std::string> logFilePath);

    void SetCrashLogFilePath(std::optional<std::string> logFilePath);

    LogProvider(){};

    ~LogProvider(){};

    static inline LogProvider & GetInstance() { return sInstance; }

private:
    std::optional<std::string> GetLogFilePath(IntentEnum logType);

    std::optional<std::string> mEndUserSupportLogFilePath;
    std::optional<std::string> mNetworkDiagnosticsLogFilePath;
    std::optional<std::string> mCrashLogFilePath;

    std::ifstream mFileStream;

    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;

    bool mIsInALogCollectionSession = false;

    uint64_t mTotalNumberOfBytesConsumed;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
