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

#include "diagnostic-logs-provider-delegate-impl.h"

#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cerrno>
#include <cstdio>
#include <limits>
#include <string>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

LogProvider LogProvider::sInstance;

namespace {
bool IsValidIntent(IntentEnum intent)
{
    return intent != IntentEnum::kUnknownEnumValue;
}

size_t GetFileSize(FILE * fp)
{
    VerifyOrReturnValue(nullptr != fp, 0);

    auto offset = ftell(fp);
    VerifyOrReturnValue(offset != -1, 0);

    auto rv = fseek(fp, 0, SEEK_END);
    VerifyOrReturnValue(rv == 0, 0);

    auto fileSize = ftell(fp);
    VerifyOrReturnValue(fileSize != -1, 0);
    VerifyOrReturnValue(CanCastTo<size_t>(fileSize), 0);

    rv = fseek(fp, offset, SEEK_SET);
    VerifyOrReturnValue(rv == 0, 0);

    return static_cast<size_t>(fileSize);
}
} // namespace

LogProvider::~LogProvider()
{
    for (auto f : mFiles)
    {
        auto rv = fclose(f.second);
        if (rv != 0)
        {
            ChipLogError(NotSpecified, "Error when closing file pointer: %d", errno);
        }
    }
    mFiles.clear();
}

CHIP_ERROR LogProvider::GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                                        Optional<uint64_t> & outTimeSinceBoot)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    LogSessionHandle sessionHandle = kInvalidLogSessionHandle;

    err = StartLogCollection(intent, sessionHandle, outTimeStamp, outTimeSinceBoot);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    bool unusedOutIsEndOfLog;
    err = CollectLog(sessionHandle, outBuffer, unusedOutIsEndOfLog);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    err = EndLogCollection(sessionHandle);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                           Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    auto filePath = GetFilePathForIntent(intent);
    VerifyOrReturnValue(filePath.HasValue(), CHIP_ERROR_NOT_FOUND);

    // Guard against infinite loop if all handles are exhausted
    VerifyOrReturnError(mFiles.size() < std::numeric_limits<LogSessionHandle>::max(), CHIP_ERROR_NO_MEMORY);
    auto fp = fopen(filePath.Value().c_str(), "rb");
    VerifyOrReturnValue(!(nullptr == fp && errno == ENOENT), CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnValue(nullptr != fp, CHIP_ERROR_INTERNAL);

    // Select the next unused session handle, skipping the invalid handle.
    do
    {
        mLogSessionHandle++;
        // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
        VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);
    } while (mFiles.count(mLogSessionHandle) != 0);

    outHandle                 = mLogSessionHandle;
    mFiles[mLogSessionHandle] = fp;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mFiles.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    auto fp = mFiles[sessionHandle];
    mFiles.erase(sessionHandle);

    errno   = 0;
    auto rv = fclose(fp);
    if (rv != 0)
    {
        int savedErrno = errno;
        if (savedErrno == 0)
        {
            savedErrno = EIO;
        }
        return CHIP_ERROR_POSIX(savedErrno);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mFiles.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    auto fp                     = mFiles[sessionHandle];
    const size_t bytesRequested = outBuffer.size();

    clearerr(fp);
    errno            = 0;
    size_t bytesRead = fread(outBuffer.data(), 1, bytesRequested, fp);
    if (ferror(fp) != 0)
    {
        int savedErrno = errno;
        if (savedErrno == 0)
        {
            savedErrno = EIO;
        }
        outBuffer.reduce_size(0);
        return CHIP_ERROR_POSIX(savedErrno);
    }

    outBuffer.reduce_size(bytesRead);

    // Prefer determining end-of-log by comparing the current offset to the total file size,
    // so that the last non-empty chunk is marked as EOF even when the file size is an exact
    // multiple of bytesRequested. Fall back to the short-read/feof heuristic if needed.
    size_t fileSize = GetFileSize(fp);
    long currentOffset = ftell(fp);
    if ((fileSize != 0) && (currentOffset != -1) && CanCastTo<size_t>(currentOffset))
    {
        outIsEndOfLog = (static_cast<size_t>(currentOffset) >= fileSize);
    }
    else
    {
        // Fallback: Treat short reads (including 0 bytes) as end-of-log, and also respect EOF.
        outIsEndOfLog = (bytesRead < bytesRequested) || (feof(fp) != 0);
    }

    return CHIP_NO_ERROR;
}

size_t LogProvider::GetSizeForIntent(IntentEnum intent)
{
    VerifyOrReturnValue(IsValidIntent(intent), 0);

    auto filePath = GetFilePathForIntent(intent);
    VerifyOrReturnValue(filePath.HasValue(), 0);

    auto fp = fopen(filePath.Value().c_str(), "rb");
    VerifyOrReturnValue(nullptr != fp, 0);

    auto fileSize = GetFileSize(fp);

    auto rv = fclose(fp);
    if (rv != 0)
    {
        ChipLogError(NotSpecified, "Error when closing file pointer (%d)", errno);
    }

    return fileSize;
}

Optional<std::string> LogProvider::GetFilePathForIntent(IntentEnum intent) const
{
    switch (intent)
    {
    case IntentEnum::kEndUserSupport:
        return mEndUserSupportLogFilePath;
    case IntentEnum::kNetworkDiag:
        return mNetworkDiagnosticsLogFilePath;
    case IntentEnum::kCrashLogs:
        return mCrashLogFilePath;
    case IntentEnum::kUnknownEnumValue:
        // It should never happen.
        chipDie();
        return NullOptional; // Unreachable.
    default:
        return NullOptional;
    }
}
