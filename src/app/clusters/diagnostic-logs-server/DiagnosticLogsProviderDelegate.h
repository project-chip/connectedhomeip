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

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

typedef uint16_t LogSessionHandle;

// The value UINT16_MAX will be used as an invalid log session handle and must not be used as a valid value for LogSessionHandle
constexpr LogSessionHandle kInvalidLogSessionHandle = UINT16_MAX;

/** @brief
 *  Defines methods for implementing application-specific logic for getting the log data from the diagnostic logs provider
 *  DiagnosticLogsProviderDelegate.
 */
class DiagnosticLogsProviderDelegate
{
public:
    DiagnosticLogsProviderDelegate() = default;

    virtual ~DiagnosticLogsProviderDelegate() = default;

    /**
     * Called to start log collection for the log type passed in.
     *
     * @param[in] intent            The type of log for which the start of log collection is requested.
     * @param[out] outHandle        The unique log session handle that identifies the log collection session that has been started.
     * @param[out] outTimeStamp     An optional value with the timestamp of the oldest log entry
     * @param[out] outTimeSinceBoot An optional value with the time since boot of the oldest log entry
     *                        Returns kInvalidLogSessionHandle if there are no logs of the intent.
     */
    virtual CHIP_ERROR StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                          Optional<uint64_t> & outTimeSinceBoot) = 0;

    /**
     * Called to end log collection for the log session identified by sessionHandle.
     * This must be called if StartLogCollection happens successfully and a valid sessionHandle has been
     * returned from StartLogCollection.
     *
     * @param[in] sessionHandle  The unique handle for this log session returned from a call to StartLogCollection.
     *
     */
    virtual CHIP_ERROR EndLogCollection(LogSessionHandle sessionHandle) = 0;

    /**
     * Called to get the next chunk for the log session identified by sessionHandle.
     * The outBuffer is resized to the actual size of data that was successfully read from the file.
     * Should return CHIP_NO_ERROR if we were able to read successfully from the file into the buffer, otherwise
     * an appropriate error code is returned.
     *
     * @param[in] sessionHandle   The unique handle for this log session returned from a call to StartLogCollection.
     * @param[out] outBuffer      The buffer thats passed in by the caller to write to.
     * @param[out] outIsEndOfLog  Set to true if there is no more log data otherwise set to false.
     */
    virtual CHIP_ERROR CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog) = 0;

    /**
     * Called to get the file size for the log type passed in.
     *
     * @param[in] intent   The type of log for which the file size is requested.
     *
     * Returns the size of the logs for the target intent. Retuns 0 if there are no logs for the given intent, otherwise returns the
     * size in bytes.
     */
    virtual size_t GetSizeForIntent(IntentEnum intent) = 0;

    /**
     * Called to get the newest diagnostic log entries for the target intent.
     *
     * @param[in] intent            The intent for which the log size is requested.
     * @param[out] outBuffer        The buffer thats passed in by the caller to write to.
     * @param[out] outTimeStamp     An optional value with the timestamp of the oldest log entry
     * @param[out] outTimeSinceBoot An optional value with the time since boot of the oldest log entry
     *
     */
    virtual CHIP_ERROR GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                                       Optional<uint64_t> & outTimeSinceBoot) = 0;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
