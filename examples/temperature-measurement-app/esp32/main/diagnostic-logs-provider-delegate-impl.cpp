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

#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
#include <esp_core_dump.h>
#include <esp_flash_encrypt.h>
// Its a bit hackish but we need this in order to pull in the sizeof(core_dump_header_t)
// we can even use the static 20 but, what if that gets chagned?
#include "../include_core_dump/esp_core_dump_types.h"
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

LogProvider LogProvider::sInstance;
LogProvider::CrashLogContext LogProvider::sCrashLogContext;

namespace {
bool IsValidIntent(IntentEnum intent)
{
    return intent != IntentEnum::kUnknownEnumValue;
}

// end_user_support.log and network_diag.log files are embedded in the firmware
extern const uint8_t endUserSupportLogStart[] asm("_binary_end_user_support_log_start");
extern const uint8_t endUserSupportLogEnd[] asm("_binary_end_user_support_log_end");

extern const uint8_t networkDiagnosticLogStart[] asm("_binary_network_diag_log_start");
extern const uint8_t networkDiagnosticLogEnd[] asm("_binary_network_diag_log_end");
} // namespace

LogProvider::~LogProvider()
{
    for (auto sessionSpan : mSessionContextMap)
    {
        Platform::MemoryFree(sessionSpan.second);
    }
    mSessionContextMap.clear();
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

size_t LogProvider::GetSizeForIntent(IntentEnum intent)
{
    switch (intent)
    {
    case IntentEnum::kEndUserSupport:
        return static_cast<size_t>(endUserSupportLogEnd - endUserSupportLogStart);
    case IntentEnum::kNetworkDiag:
        return static_cast<size_t>(networkDiagnosticLogEnd - networkDiagnosticLogStart);
    case IntentEnum::kCrashLogs:
        return GetCrashSize();
    default:
        return 0;
    }
}

size_t LogProvider::GetCrashSize()
{
    size_t outSize = 0;

#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
    size_t unusedOutAddr;
    esp_err_t esp_err = esp_core_dump_image_get(&unusedOutAddr, &outSize);
    VerifyOrReturnValue(esp_err == ESP_OK, 0, ChipLogError(DeviceLayer, "Failed to get core dump image, esp_err:%d", esp_err));
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)

    return outSize;
}

CHIP_ERROR LogProvider::MapCrashPartition(CrashLogContext * context)
{
#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
    size_t outAddr, outSize;
    esp_err_t esp_err = esp_core_dump_image_get(&outAddr, &outSize);
    VerifyOrReturnError(esp_err == ESP_OK, CHIP_ERROR(ChipError::Range::kPlatform, esp_err),
                        ChipLogError(DeviceLayer, "Failed to get core dump image, esp_err:%d", esp_err));

    /* map the full core dump parition, including the checksum. */
    esp_err = spi_flash_mmap(outAddr, outSize, SPI_FLASH_MMAP_DATA, &context->mappedAddress, &context->mappedHandle);
    VerifyOrReturnError(esp_err == ESP_OK, CHIP_ERROR(ChipError::Range::kPlatform, esp_err),
                        ChipLogError(DeviceLayer, "Failed to mmap the crash partition, esp_err:%d", esp_err));

    context->crashSize = static_cast<uint32_t>(outSize);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_FOUND;
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
}

CHIP_ERROR LogProvider::PrepareLogContextForIntent(LogContext * context, IntentEnum intent)
{
    context->intent = intent;

    switch (intent)
    {
    case IntentEnum::kEndUserSupport: {
        context->EndUserSupport.span =
            ByteSpan(&endUserSupportLogStart[0], static_cast<size_t>(endUserSupportLogEnd - endUserSupportLogStart));
    }
    break;

    case IntentEnum::kNetworkDiag: {
        context->NetworkDiag.span =
            ByteSpan(&networkDiagnosticLogStart[0], static_cast<size_t>(networkDiagnosticLogEnd - networkDiagnosticLogStart));
    }
    break;

    case IntentEnum::kCrashLogs: {
#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
        sCrashLogContext.Reset();
        context->Crash.logContext = &sCrashLogContext;

        CHIP_ERROR err = MapCrashPartition(context->Crash.logContext);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err, context->Crash.logContext = nullptr);

        context->Crash.logContext->readOffset = sizeof(core_dump_header_t);
        context->Crash.logContext->isMapped   = true;
#else
        return CHIP_ERROR_NOT_FOUND;
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
    }
    break;

    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void LogProvider::CleanupLogContextForIntent(LogContext * context)
{
    switch (context->intent)
    {
    case IntentEnum::kEndUserSupport:
        break;

    case IntentEnum::kNetworkDiag:
        break;

    case IntentEnum::kCrashLogs: {
#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
        CrashLogContext * logContext = context->Crash.logContext;
        spi_flash_munmap(logContext->mappedHandle);
        logContext->Reset();
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
    }
    break;

    default:
        break;
    }
}

CHIP_ERROR LogProvider::GetDataForIntent(LogContext * context, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    switch (context->intent)
    {
    case IntentEnum::kEndUserSupport: {
        auto dataSize = context->EndUserSupport.span.size();
        auto count    = std::min(dataSize, outBuffer.size());

        VerifyOrReturnError(CanCastTo<off_t>(count), CHIP_ERROR_INVALID_ARGUMENT, outBuffer.reduce_size(0));
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(context->EndUserSupport.span.data(), count), outBuffer));

        outIsEndOfLog = dataSize == count;
        if (!outIsEndOfLog)
        {
            // reduce the span after reading count bytes
            context->EndUserSupport.span = context->EndUserSupport.span.SubSpan(count);
        }
    }
    break;

    case IntentEnum::kNetworkDiag: {
        auto dataSize = context->NetworkDiag.span.size();
        auto count    = std::min(dataSize, outBuffer.size());

        VerifyOrReturnError(CanCastTo<off_t>(count), CHIP_ERROR_INVALID_ARGUMENT, outBuffer.reduce_size(0));
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(context->NetworkDiag.span.data(), count), outBuffer));

        outIsEndOfLog = dataSize == count;
        if (!outIsEndOfLog)
        {
            // reduce the span after reading count bytes
            context->NetworkDiag.span = context->NetworkDiag.span.SubSpan(count);
        }
    }
    break;

    case IntentEnum::kCrashLogs: {
#if defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
        CrashLogContext * logContext = context->Crash.logContext;
        size_t dataSize              = logContext->crashSize - logContext->readOffset;
        auto count                   = std::min(dataSize, outBuffer.size());

        VerifyOrReturnError(CanCastTo<off_t>(count), CHIP_ERROR_INVALID_ARGUMENT, outBuffer.reduce_size(0));

        const uint8_t * readAddr = reinterpret_cast<const uint8_t *>(logContext->mappedAddress) + logContext->readOffset;
        memcpy(outBuffer.data(), readAddr, count);
        outBuffer.reduce_size(count);

        logContext->readOffset += count;
        outIsEndOfLog = dataSize == count;
#else
        outBuffer.reduce_size(0);
        return CHIP_ERROR_NOT_FOUND;
#endif // defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && defined(CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF)
    }
    break;

    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                           Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    // In case of crash logs we can only mmap at max once, so check before doing anything
    if (intent == IntentEnum::kCrashLogs)
    {
        VerifyOrReturnError(sCrashLogContext.isMapped == false, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(DeviceLayer, "Crash partition already mapped"));
    }

    LogContext * context = reinterpret_cast<LogContext *>(Platform::MemoryCalloc(1, sizeof(LogContext)));
    VerifyOrReturnValue(context != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = PrepareLogContextForIntent(context, intent);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, Platform::MemoryFree(context));

    mLogSessionHandle++;
    // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
    VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);

    outHandle                             = mLogSessionHandle;
    mSessionContextMap[mLogSessionHandle] = context;

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mSessionContextMap.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    LogContext * context = mSessionContextMap[sessionHandle];
    VerifyOrReturnError(context, CHIP_ERROR_INCORRECT_STATE);

    CleanupLogContextForIntent(context);
    Platform::MemoryFree(context);
    mSessionContextMap.erase(sessionHandle);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mSessionContextMap.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    LogContext * context = mSessionContextMap[sessionHandle];
    VerifyOrReturnError(context, CHIP_ERROR_INCORRECT_STATE);

    return GetDataForIntent(context, outBuffer, outIsEndOfLog);
}
