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

#include "include/diagnostic-logs-provider-delegate-impl.h"

#include <lib/support/SafeInt.h>

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
            ChipLogError(NotSpecified, "Error when closing file pointer: %p (%d)", f.second, errno);
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

    auto fp = fopen(filePath.Value().c_str(), "rb");
    VerifyOrReturnValue(!(nullptr == fp && errno == ENOENT), CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnValue(nullptr != fp, CHIP_ERROR_INTERNAL);

    mLogSessionHandle++;
    // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
    VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);

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

    auto rv = fclose(fp);
    VerifyOrReturnError(rv == 0, CHIP_ERROR_POSIX(errno));

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mFiles.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    auto fp       = mFiles[sessionHandle];
    auto fileSize = GetFileSize(fp);
    auto count    = std::min(fileSize, outBuffer.size());

    VerifyOrReturnError(CanCastTo<off_t>(count), CHIP_ERROR_INVALID_ARGUMENT, outBuffer.reduce_size(0));

    auto bytesRead = fread(outBuffer.data(), 1, count, fp);
    VerifyOrReturnError(CanCastTo<size_t>(bytesRead), CHIP_ERROR_INTERNAL, outBuffer.reduce_size(0));

    outBuffer.reduce_size(static_cast<size_t>(bytesRead));
    outIsEndOfLog = fileSize == static_cast<size_t>(ftell(fp));
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
    }

    return NullOptional;
}
