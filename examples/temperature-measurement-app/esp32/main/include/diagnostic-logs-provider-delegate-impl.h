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
#include <map>

#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
#include <esp_core_dump.h>
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)

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

private:
    static LogProvider sInstance;
    LogProvider() = default;
    ~LogProvider();

    LogProvider(const LogProvider &)             = delete;
    LogProvider & operator=(const LogProvider &) = delete;

    struct CrashLogContext
    {
#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
        uint32_t crashSize                = 0;
        uint32_t readOffset               = 0;
        esp_core_dump_summary_t * summary = nullptr;

        void Reset()
        {
            this->crashSize  = 0;
            this->readOffset = 0;

            if (this->summary)
            {
                Platform::MemoryFree(this->summary);
                this->summary = nullptr;
            }
        }
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
    };

    static CrashLogContext sCrashLogContext;

    struct LogContext
    {
        IntentEnum intent;
        union
        {
            struct
            {
                ByteSpan span;
            } EndUserSupport;
            struct
            {
                ByteSpan span;
            } NetworkDiag;
            struct
            {
                // TODO: This be a ref counted, so that we can serve parallel queries for crash logs
                CrashLogContext * logContext;
            } Crash;
        };
    };

    // This tracks the ByteSpan for each session, need to change this to void *
    std::map<LogSessionHandle, LogContext *> mSessionContextMap;

    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;

    // Helpers for Retrieving Core Dump from flash
    size_t GetCrashSize();
    CHIP_ERROR MapCrashPartition(CrashLogContext * crashLogContext);

    CHIP_ERROR PrepareLogContextForIntent(LogContext * context, IntentEnum intent);
    void CleanupLogContextForIntent(LogContext * contex);
    CHIP_ERROR GetDataForIntent(LogContext * context, MutableByteSpan & outBuffer, bool & outIsEndOfLog);
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
