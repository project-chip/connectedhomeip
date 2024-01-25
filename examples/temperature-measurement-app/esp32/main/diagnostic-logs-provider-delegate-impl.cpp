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

#include <diagnostic-logs-provider-delegate-impl.h>
#include <lib/support/SafeInt.h>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

LogProvider LogProvider::sInstance;

namespace {
bool IsValidIntent(IntentEnum intent)
{
    return intent != IntentEnum::kUnknownEnumValue;
}

// end_user_support.log, network_diag.log, and crash.log files are embedded in the firmware
extern const uint8_t endUserSupportLogStart[] asm("_binary_end_user_support_log_start");
extern const uint8_t endUserSupportLogEnd[] asm("_binary_end_user_support_log_end");

extern const uint8_t networkDiagnosticLogStart[] asm("_binary_network_diag_log_start");
extern const uint8_t networkDiagnosticLogEnd[] asm("_binary_network_diag_log_end");

extern const uint8_t crashLogStart[] asm("_binary_crash_log_start");
extern const uint8_t crashLogEnd[] asm("_binary_crash_log_end");
} // namespace

LogProvider::~LogProvider()
{
    for (auto sessionSpan : mSessionSpanMap)
    {
        Platform::MemoryFree(sessionSpan.second);
    }
    mSessionSpanMap.clear();
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

const uint8_t * LogProvider::GetDataStartForIntent(IntentEnum intent)
{
    switch (intent)
    {
    case IntentEnum::kEndUserSupport:
        return &endUserSupportLogStart[0];
    case IntentEnum::kNetworkDiag:
        return &networkDiagnosticLogStart[0];
    case IntentEnum::kCrashLogs:
        return &crashLogStart[0];
    default:
        return nullptr;
    }
}

size_t LogProvider::GetSizeForIntent(IntentEnum intent)
{
    switch (intent)
    {
    case IntentEnum::kEndUserSupport:
        return static_cast<size_t>(endUserSupportLogEnd - endUserSupportLogStart);
    case IntentEnum::kNetworkDiag:
        return static_cast<size_t>(networkDiagnosticLogEnd - networkDiagnosticLogStart);
    case IntentEnum::kCrashLogs:
        return static_cast<size_t>(crashLogEnd - crashLogStart);
    default:
        return 0;
    }
}

CHIP_ERROR LogProvider::StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                           Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    const uint8_t * dataStart = GetDataStartForIntent(intent);
    VerifyOrReturnError(dataStart, CHIP_ERROR_NOT_FOUND);

    size_t dataSize = GetSizeForIntent(intent);
    VerifyOrReturnError(dataSize, CHIP_ERROR_NOT_FOUND);

    ByteSpan * span = reinterpret_cast<ByteSpan *>(Platform::MemoryCalloc(1, sizeof(ByteSpan)));
    VerifyOrReturnValue(span, CHIP_ERROR_NO_MEMORY);

    *span = ByteSpan(dataStart, dataSize);

    mLogSessionHandle++;
    // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
    VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);

    outHandle                          = mLogSessionHandle;
    mSessionSpanMap[mLogSessionHandle] = span;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mSessionSpanMap.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan * span = mSessionSpanMap[sessionHandle];
    mSessionSpanMap.erase(sessionHandle);

    Platform::MemoryFree(span);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mSessionSpanMap.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan * span = mSessionSpanMap[sessionHandle];
    auto dataSize   = span->size();
    auto count      = std::min(dataSize, outBuffer.size());

    VerifyOrReturnError(CanCastTo<off_t>(count), CHIP_ERROR_INVALID_ARGUMENT, outBuffer.reduce_size(0));

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(span->data(), count), outBuffer));

    outIsEndOfLog = dataSize == count;

    if (!outIsEndOfLog)
    {
        // reduce the span after reading count bytes
        *span = span->SubSpan(count);
    }

    return CHIP_NO_ERROR;
}
