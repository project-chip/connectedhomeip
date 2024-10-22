/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

/**
 * The LogProvider class serves as the sole instance delegate for handling diagnostic logs.
 *
 * It implements the DiagnosticLogsProviderDelegate interface
 */

class LogProvider : public DiagnosticLogsProviderDelegate
{
public:
    static inline LogProvider & GetInstance() { return sInstance; }

    /////////// DiagnosticLogsProviderDelegate Interface /////////
    CHIP_ERROR StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                  Optional<uint64_t> & outTimeSinceBoot) override;
    CHIP_ERROR EndLogCollection(LogSessionHandle sessionHandle) override;
    CHIP_ERROR CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog) override;
    size_t GetSizeForIntent(IntentEnum intent) override;
    CHIP_ERROR GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                               Optional<uint64_t> & outTimeSinceBoot) override;

    static inline StorageKeyName GetKeyDiagUserSupport() { return StorageKeyName::FromConst("nxp/diag/usr"); }

    static inline StorageKeyName GetKeyDiagNetwork() { return StorageKeyName::FromConst("nxp/diag/nwk"); }

    static inline StorageKeyName GetKeyDiagCrashLog() { return StorageKeyName::FromConst("nxp/diag/crash"); }

private:
    static LogProvider sInstance;
    LogProvider() = default;
    ~LogProvider();

    LogProvider(const LogProvider &)             = delete;
    LogProvider & operator=(const LogProvider &) = delete;

    // This tracks the ByteSpan for each session
    std::map<LogSessionHandle, MutableByteSpan *> mSessionSpanMap;
    std::map<LogSessionHandle, uint8_t *> mSessionDiagMap;
    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;

    StorageKeyName GetKeyForIntent(IntentEnum intent) const;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
