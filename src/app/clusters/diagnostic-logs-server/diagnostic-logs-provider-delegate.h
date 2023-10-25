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
#include <app-common/zap-generated/enums.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

typedef uint16_t LogSessionHandle;

// The value 0xFF will be used as an invalid log session handle and must not be used as a valid value for LogSessionHandle
constexpr uint8_t kInvalidLogSessionHandle = 0xFF;

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
     */
    virtual LogSessionHandle StartLogCollection(IntentEnum logType) = 0;

    /**
     * Called to get the next chunk for the log session identified by logSessionHandle.
     * Should return the number of bytes read and indicate if EOF has been reached.
     *
     * @param[in] logSessionHandle  The unique handle for this log session returned from a call to StartLogCollection.
     * @param[out] outBuffer        The buffer thats passed in by the caller to write to.
     * @param[in] bufferLen         The size of the buffer passed in.
     * @param[out] outIsEOF         Set to true if EOF is reached otherwise set to false.
     */
    virtual uint64_t GetNextChunk(LogSessionHandle logSessionHandle, chip::MutableByteSpan & outBuffer, bool & outIsEOF) = 0;

    /**
     * Called to end log collection for the log session identified by logSessionHandle
     *
     * @param[in] logSessionHandle  The unique handle for this log session returned from a call to StartLogCollection.
     *
     */
    virtual void EndLogCollection(LogSessionHandle logSessionHandle) = 0;

    /**
     * Called to get the total number of bytes consumed from the log session identified by logSessionHandle
     *
     * @param[in] logSessionHandle  The unique handle for this log session returned from a call to StartLogCollection.
     *
     */
    virtual uint64_t GetTotalNumberOfBytesConsumed(LogSessionHandle logSessionHandle) = 0;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
