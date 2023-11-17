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
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-provider-delegate.h>
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

LogProvider LogProvider::sInstance;

LogSessionHandle LogProvider::sLogSessionHandle;

// TODO: Fix #30477 - Add support for multiple log collection sessions

LogSessionHandle LogProvider::StartLogCollection(IntentEnum logType)
{

    // We can handle only one log collection session. Return kInvalidLogSessionHandle
    // if we are in the middle of another log collection session.
    if (mIsInALogCollectionSession)
    {
        return kInvalidLogSessionHandle;
    }

    mIsInALogCollectionSession  = true;
    mTotalNumberOfBytesConsumed = 0;

    // Open the log file for reading.
    Optional<std::string> filePath = GetLogFilePath(logType);
    if (filePath.HasValue())
    {
        mFileStream.open(filePath.Value().c_str(), std::ios_base::binary | std::ios_base::in);
        if (!mFileStream.good())
        {
            ChipLogError(BDX, "Failed to open the log file");
            return kInvalidLogSessionHandle;
        }
        sLogSessionHandle++;

        // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
        if (sLogSessionHandle == kInvalidLogSessionHandle)
        {
            sLogSessionHandle = 0;
        }
        mLogSessionHandle = sLogSessionHandle;
    }
    else
    {
        mLogSessionHandle = kInvalidLogSessionHandle;
    }
    return mLogSessionHandle;
}

CHIP_ERROR LogProvider::GetNextChunk(LogSessionHandle logSessionHandle, MutableByteSpan & outBuffer, bool & outIsEOF)
{
    if (!mIsInALogCollectionSession)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (!mFileStream.is_open())
    {
        ChipLogError(BDX, "File is not open");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mFileStream.seekg(mFileStream.end);
    long long fileSize = mFileStream.tellg();

    long long remainingBytesToBeRead = fileSize - static_cast<long long>(mTotalNumberOfBytesConsumed);

    if ((remainingBytesToBeRead >= kMaxLogContentSize && outBuffer.size() < kMaxLogContentSize) ||
        (remainingBytesToBeRead < kMaxLogContentSize && static_cast<long long>(outBuffer.size()) < remainingBytesToBeRead))
    {
        ChipLogError(BDX, "Buffer is too small to read the next chunk from file");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mFileStream.seekg(static_cast<long long>(mTotalNumberOfBytesConsumed));
    mFileStream.read(reinterpret_cast<char *>(outBuffer.data()), kMaxLogContentSize);

    if (!(mFileStream.good() || mFileStream.eof()))
    {
        ChipLogError(BDX, "Failed to read the log file");
        mFileStream.close();
        return CHIP_ERROR_INCORRECT_STATE;
    }

    uint64_t bytesRead = static_cast<uint64_t>(mFileStream.gcount());
    outBuffer.reduce_size(bytesRead);
    outIsEOF = (mFileStream.peek() == EOF);

    mTotalNumberOfBytesConsumed += bytesRead;
    return CHIP_NO_ERROR;
}

void LogProvider::EndLogCollection(LogSessionHandle logSessionHandle)
{
    if (mFileStream.is_open())
    {
        mFileStream.close();
    }
    mIsInALogCollectionSession = false;
}

uint64_t LogProvider::GetTotalNumberOfBytesConsumed(LogSessionHandle logSessionHandle)
{
    return mTotalNumberOfBytesConsumed;
}

Optional<std::string> LogProvider::GetLogFilePath(IntentEnum logType)
{
    switch (logType)
    {
    case IntentEnum::kEndUserSupport:
        return mEndUserSupportLogFilePath;
    case IntentEnum::kNetworkDiag:
        return mNetworkDiagnosticsLogFilePath;
    case IntentEnum::kCrashLogs:
        return mCrashLogFilePath;
    default:
        return NullOptional;
    }
}

void LogProvider::SetEndUserSupportLogFilePath(Optional<std::string> logFilePath)
{
    mEndUserSupportLogFilePath = logFilePath;
}

void LogProvider::SetNetworkDiagnosticsLogFilePath(Optional<std::string> logFilePath)
{
    mNetworkDiagnosticsLogFilePath = logFilePath;
}

void LogProvider::SetCrashLogFilePath(Optional<std::string> logFilePath)
{
    mCrashLogFilePath = logFilePath;
}
