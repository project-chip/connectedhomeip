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
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#include <map>

class LogProvider : public chip::app::Clusters::DiagnosticLogs::DiagnosticLogsProviderDelegate
{
public:
    static LogProvider & GetInstance() { return sInstance; }

    ~LogProvider();

    /////////// DiagnosticLogsProviderDelegate Interface /////////
    CHIP_ERROR StartLogCollection(chip::app::Clusters::DiagnosticLogs::IntentEnum intent,
                                  chip::app::Clusters::DiagnosticLogs::LogSessionHandle & outHandle,
                                  chip::Optional<uint64_t> & outTimeStamp,
                                  chip::Optional<uint64_t> & outTimeSinceBoot) override;

    CHIP_ERROR EndLogCollection(chip::app::Clusters::DiagnosticLogs::LogSessionHandle sessionHandle) override;

    CHIP_ERROR CollectLog(chip::app::Clusters::DiagnosticLogs::LogSessionHandle sessionHandle,
                          chip::MutableByteSpan & outBuffer, bool & outIsEndOfLog) override;

    size_t GetSizeForIntent(chip::app::Clusters::DiagnosticLogs::IntentEnum intent) override;

    CHIP_ERROR GetLogForIntent(chip::app::Clusters::DiagnosticLogs::IntentEnum intent,
                               chip::MutableByteSpan & outBuffer,
                               chip::Optional<uint64_t> & outTimeStamp,
                               chip::Optional<uint64_t> & outTimeSinceBoot) override;
    /////////// DiagnosticLogsProviderDelegate Interface /////////

    void SetEndUserSupportLogFilePath(chip::Optional<std::string> filePath) { mEndUserSupportLogFilePath = filePath; }
    void SetNetworkDiagnosticsLogFilePath(chip::Optional<std::string> filePath) { mNetworkDiagnosticsLogFilePath = filePath; }
    void SetCrashLogFilePath(chip::Optional<std::string> filePath) { mCrashLogFilePath = filePath; }

private:
    LogProvider()          = default;
    LogProvider(LogProvider &)  = delete;
    void operator=(const LogProvider &) = delete;

    chip::Optional<std::string> GetFilePathForIntent(chip::app::Clusters::DiagnosticLogs::IntentEnum intent) const;

    static LogProvider sInstance;

    chip::Optional<std::string> mEndUserSupportLogFilePath;
    chip::Optional<std::string> mNetworkDiagnosticsLogFilePath;
    chip::Optional<std::string> mCrashLogFilePath;

    chip::app::Clusters::DiagnosticLogs::LogSessionHandle mLogSessionHandle = chip::app::Clusters::DiagnosticLogs::kInvalidLogSessionHandle;
    std::map<chip::app::Clusters::DiagnosticLogs::LogSessionHandle, FILE *> mFiles;
};
