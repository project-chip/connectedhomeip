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

#include "DiagnosticLogsProviderDelegateImpl.h"

#include <src/app/server/Server.h>
#include <src/lib/support/SafeInt.h>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

namespace {
bool IsValidIntent(IntentEnum intent)
{
    return intent != IntentEnum::kUnknownEnumValue;
}
} // namespace

LogProvider LogProvider::sInstance;

LogProvider::~LogProvider() {}

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    auto key = GetKeyForIntent(intent);
    VerifyOrReturnValue(key.IsInitialized(), CHIP_ERROR_NOT_FOUND);

    uint16_t diagSize = GetSizeForIntent(intent);
    VerifyOrReturnError(diagSize, CHIP_ERROR_NOT_FOUND);

    uint8_t * diagData = (uint8_t *) calloc(1, diagSize);
    VerifyOrReturnError(diagData, CHIP_ERROR_NO_MEMORY);

    err = chip::Server::GetInstance().GetPersistentStorage().SyncGetKeyValue(key.KeyName(), diagData, diagSize);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err);

    MutableByteSpan * mutableSpan = reinterpret_cast<MutableByteSpan *>(calloc(1, sizeof(MutableByteSpan)));
    VerifyOrReturnValue(mutableSpan, CHIP_ERROR_NO_MEMORY, free(diagData));

    *mutableSpan = MutableByteSpan(diagData, diagSize);

    mLogSessionHandle++;
    // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
    VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);

    outHandle                          = mLogSessionHandle;
    mSessionSpanMap[mLogSessionHandle] = mutableSpan;
    mSessionDiagMap[mLogSessionHandle] = diagData;

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mSessionSpanMap.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    free(mSessionDiagMap[sessionHandle]);
    free(mSessionSpanMap[sessionHandle]);

    mSessionSpanMap.erase(sessionHandle);
    mSessionDiagMap.erase(sessionHandle);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mSessionSpanMap.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    MutableByteSpan * mutableSpan = mSessionSpanMap[sessionHandle];
    auto diagSize                 = mutableSpan->size();
    auto count                    = std::min(diagSize, outBuffer.size());

    VerifyOrReturnError(CanCastTo<off_t>(count), CHIP_ERROR_INVALID_ARGUMENT, outBuffer.reduce_size(0));

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(mutableSpan->data(), count), outBuffer));

    outIsEndOfLog = diagSize == count;

    if (!outIsEndOfLog)
    {
        // reduce the span after reading count bytes
        *mutableSpan = mutableSpan->SubSpan(count);
    }

    return CHIP_NO_ERROR;
}

size_t LogProvider::GetSizeForIntent(IntentEnum intent)
{
    uint16_t sizeForIntent = 0;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    auto key = GetKeyForIntent(intent);
    VerifyOrReturnValue(key.IsInitialized(), 0);

    uint16_t bufferLen = CHIP_DEVICE_CONFIG_MAX_DIAG_LOG_SIZE;
    Platform::ScopedMemoryBuffer<uint8_t> buffer;

    buffer.Calloc(bufferLen);
    VerifyOrReturnValue(buffer.Get() != nullptr, 0);

    err = Server::GetInstance().GetPersistentStorage().SyncGetKeyValue(key.KeyName(), buffer.Get(), sizeForIntent);
    VerifyOrReturnValue(err == CHIP_ERROR_BUFFER_TOO_SMALL, 0);

    return sizeForIntent;
}

StorageKeyName LogProvider::GetKeyForIntent(IntentEnum intent) const
{
    StorageKeyName key = StorageKeyName::Uninitialized();

    switch (intent)
    {
    case IntentEnum::kEndUserSupport:
        return GetKeyDiagUserSupport();
    case IntentEnum::kNetworkDiag:
        return GetKeyDiagNetwork();
    case IntentEnum::kCrashLogs:
        return GetKeyDiagCrashLog();
    case IntentEnum::kUnknownEnumValue:
        // It should never happen.
        chipDie();
    }

    return key;
}
