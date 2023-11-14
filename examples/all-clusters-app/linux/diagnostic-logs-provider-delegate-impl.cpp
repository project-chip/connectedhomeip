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

constexpr uint16_t kChunkSizeZero = 0;

LogProvider LogProvider::sInstance;

LogSessionHandle LogProvider::sLogSessionHandle;

LogSessionHandle LogProvider::StartLogCollection(IntentEnum logType)
{

    mTotalNumberOfBytesConsumed = 0;

    // Open the file of type
    const char * fileName = GetLogFilePath(logType);
    if (fileName != nullptr)
    {
        mFileStream.open(fileName, std::ios_base::binary | std::ios_base::in);
        if (!mFileStream.good())
        {
            ChipLogError(BDX, "Failed to open the log file");
            return kInvalidLogSessionHandle;
        }
        sLogSessionHandle++;
        mLogSessionHandle = sLogSessionHandle;
    }
    else
    {
        mLogSessionHandle = kInvalidLogSessionHandle;
    }
    return mLogSessionHandle;
}

uint64_t LogProvider::GetNextChunk(LogSessionHandle logSessionHandle, chip::MutableByteSpan & outBuffer, bool & outIsEOF)
{
    if (logSessionHandle != mLogSessionHandle && outBuffer.size() == 0)
    {
        return kChunkSizeZero;
    }

    if (!mFileStream.is_open())
    {
        ChipLogError(BDX, "File is not open");
        return kChunkSizeZero;
    }

    mFileStream.seekg(static_cast<long long>(mTotalNumberOfBytesConsumed));
    mFileStream.read(reinterpret_cast<char *>(outBuffer.data()), kLogContentMaxSize);

    if (!(mFileStream.good() || mFileStream.eof()))
    {
        ChipLogError(BDX, "Failed to read the log file");
        mFileStream.close();
        return kChunkSizeZero;
    }

    uint64_t bytesRead = static_cast<uint64_t>(mFileStream.gcount());
    outIsEOF           = (mFileStream.peek() == EOF);

    mTotalNumberOfBytesConsumed += bytesRead;
    return bytesRead;
}

void LogProvider::EndLogCollection(LogSessionHandle logSessionHandle)
{
    if (logSessionHandle == mLogSessionHandle && mFileStream.is_open())
    {
        mFileStream.close();
    }
}

uint64_t LogProvider::GetTotalNumberOfBytesConsumed(LogSessionHandle logSessionHandle)
{
    if (logSessionHandle == mLogSessionHandle)
    {
        return mTotalNumberOfBytesConsumed;
    }
    return kChunkSizeZero;
}

const char * LogProvider::GetLogFilePath(IntentEnum logType)
{
    switch (logType)
    {
    case IntentEnum::kEndUserSupport:
        return mEndUserSupportLogFileDesignator;
    case IntentEnum::kNetworkDiag:
        return mNetworkDiagnosticsLogFileDesignator;
    case IntentEnum::kCrashLogs:
        return mCrashLogFileDesignator;
    default:
        return nullptr;
    }
}

void LogProvider::SetEndUserSupportLogFileDesignator(const char * logFileName)
{
    strncpy(mEndUserSupportLogFileDesignator, logFileName, strlen(logFileName));
}

void LogProvider::SetNetworkDiagnosticsLogFileDesignator(const char * logFileName)
{
    strncpy(mNetworkDiagnosticsLogFileDesignator, logFileName, strlen(logFileName));
}

void LogProvider::SetCrashLogFileDesignator(const char * logFileName)
{
    strncpy(mCrashLogFileDesignator, logFileName, strlen(logFileName));
}
