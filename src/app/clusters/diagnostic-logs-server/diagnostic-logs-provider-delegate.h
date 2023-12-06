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
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

typedef uint16_t LogSessionHandle;

// The value UINT16_MAX will be used as an invalid log session handle and must not be used as a valid value for LogSessionHandle
constexpr LogSessionHandle kInvalidLogSessionHandle = UINT16_MAX;

/** @brief
 *    Defines methods for implementing application-specific logic for getting the log data from the diagnostic logs provider
 * LogProviderDelegate.
 */
class LogProviderDelegate
{
public:
    LogProviderDelegate() = default;

    virtual ~LogProviderDelegate() = default;

    /**
     * Called to start log collection for the log type passed in.
     *
     * @param[in] logType The type of log for which the start of log collection is requested.
     *
     * @return LogSessionHandle  The unique log session handle that identifies the log collection session that has been started.
     *                           Returns kInvalidLogSessionHandle if there are no logs of the logType.
     */
    virtual LogSessionHandle StartLogCollection(IntentEnum logType) = 0;

    /**
     * Called to get the next chunk for the log session identified by logSessionHandle.
     * The outBuffer is resized to the actual size of data that was successfully read from the file.
     * Should return CHIP_NO_ERROR if we were able to read successfully from the file into the buffer, otherwise
     * an appropriate error code is returned.
     *
     * @param[in] logSessionHandle  The unique handle for this log session returned from a call to StartLogCollection.
     * @param[out] outBuffer        The buffer thats passed in by the caller to write to.
     * @param[out] outIsEOF         Set to true if EOF is reached otherwise set to false.
     */
    virtual CHIP_ERROR GetNextChunk(LogSessionHandle logSessionHandle, chip::MutableByteSpan & outBuffer, bool & outIsEOF) = 0;

    /**
     * Called to end log collection for the log session identified by logSessionHandle.
     * This must be called if StartLogCollection happens successfully and a valid logSessionHandle has been
     * returned from StartLogCollection.
     *
     * @param[in] logSessionHandle  The unique handle for this log session returned from a call to StartLogCollection.
     *
     */
    virtual void EndLogCollection(LogSessionHandle logSessionHandle) = 0;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
