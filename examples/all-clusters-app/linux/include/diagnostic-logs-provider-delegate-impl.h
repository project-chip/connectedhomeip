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
    LogSessionHandle StartLogCollection(IntentEnum logType);

    CHIP_ERROR GetNextChunk(LogSessionHandle logSessionHandle, MutableByteSpan & outBuffer, bool & outIsEOF);

    void EndLogCollection(LogSessionHandle logSessionHandle);

    uint64_t GetTotalNumberOfBytesConsumed(LogSessionHandle logSessionHandle);

    void SetEndUserSupportLogFilePath(Optional<std::string> logFilePath);

    void SetNetworkDiagnosticsLogFilePath(Optional<std::string> logFilePath);

    void SetCrashLogFilePath(Optional<std::string> logFilePath);

    LogProvider(){};

    ~LogProvider(){};

    static inline LogProvider & getLogProvider() { return sInstance; }

private:
    Optional<std::string> GetLogFilePath(IntentEnum logType);

    Optional<std::string> mEndUserSupportLogFilePath;
    Optional<std::string> mNetworkDiagnosticsLogFilePath;
    Optional<std::string> mCrashLogFilePath;

    std::ifstream mFileStream;

    LogSessionHandle mLogSessionHandle;

    bool mIsInALogCollectionSession;

    uint64_t mTotalNumberOfBytesConsumed;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
