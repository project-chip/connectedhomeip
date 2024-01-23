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

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>

#include <fstream>
#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

/**
 * The LogProvider class serves as the sole instance delegate for handling diagnostic logs.
 *
 * It implements the DiagnosticLogsProviderDelegate interface and provides mechanisms to set file paths for different types of logs
 * (end-user support, network diagnostics, and crash logs).
 */

class LogProvider : public DiagnosticLogsProviderDelegate
{
public:
    static inline LogProvider & GetInstance() { return sInstance; }

    void SetEndUserSupportLogFilePath(Optional<std::string> logFilePath) { mEndUserSupportLogFilePath = logFilePath; }
    void SetNetworkDiagnosticsLogFilePath(Optional<std::string> logFilePath) { mNetworkDiagnosticsLogFilePath = logFilePath; }
    void SetCrashLogFilePath(Optional<std::string> logFilePath) { mCrashLogFilePath = logFilePath; }

    /////////// DiagnosticLogsProviderDelegate Interface /////////
    CHIP_ERROR StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                  Optional<uint64_t> & outTimeSinceBoot) override;
    CHIP_ERROR EndLogCollection(LogSessionHandle sessionHandle) override;
    CHIP_ERROR CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog) override;
    size_t GetSizeForIntent(IntentEnum intent) override;
    CHIP_ERROR GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                               Optional<uint64_t> & outTimeSinceBoot) override;

private:
    static LogProvider sInstance;
    LogProvider() = default;
    ~LogProvider();

    LogProvider(const LogProvider &)             = delete;
    LogProvider & operator=(const LogProvider &) = delete;

    Optional<std::string> GetFilePathForIntent(IntentEnum intent) const;

    Optional<std::string> mEndUserSupportLogFilePath;
    Optional<std::string> mNetworkDiagnosticsLogFilePath;
    Optional<std::string> mCrashLogFilePath;

    std::map<LogSessionHandle, FILE *> mFiles;

    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
