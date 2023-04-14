/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements macros, constants, and interfaces for a
 *      platform-independent logging interface for the chip SDK.
 *
 */

#include "CHIPLogging.h"

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <platform/logging/LogV.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <atomic>

#if CHIP_PW_TOKENIZER_LOGGING
#include "pw_tokenizer/encode_args.h"
#endif

namespace chip {
namespace Logging {

#if _CHIP_USE_LOGGING

#if CHIP_PW_TOKENIZER_LOGGING

void HandleTokenizedLog(uint32_t levels, pw_tokenizer_Token token, pw_tokenizer_ArgTypes types, ...)
{
    uint8_t encoded_message[PW_TOKENIZER_CFG_ENCODING_BUFFER_SIZE_BYTES];

    va_list args;
    va_start(args, types);
    // Use the C argument encoding API, since the C++ API requires C++17.
    const size_t encoded_size = pw_tokenizer_EncodeArgs(types, args, encoded_message, sizeof(encoded_message));
    va_end(args);

    uint8_t log_category = levels >> 8 & 0xFF;
    uint8_t log_module   = levels & 0xFF;
    char * buffer        = (char *) chip::Platform::MemoryAlloc(2 * encoded_size + 1);

    if (buffer)
    {
        for (int i = 0; i < encoded_size; i++)
        {
            sprintf(buffer + 2 * i, "%02x", encoded_message[i]);
        }
        buffer[2 * encoded_size] = '\0';
        Log(log_module, log_category, "%s", buffer);
        chip::Platform::MemoryFree(buffer);
    }
}

#endif

namespace {

std::atomic<LogRedirectCallback_t> sLogRedirectCallback{ nullptr };

/*
 * Array of strings containing the names for each of the chip log modules.
 *
 * NOTE: The names must be in the order defined in the LogModule enumeration.
 */
static const char ModuleNames[kLogModule_Max][kMaxModuleNameLen + 1] = {
    "-",   // None
    "IN",  // Inet
    "BLE", // BLE
    "ML",  // MessageLayer
    "SM",  // SecurityManager
    "EM",  // ExchangeManager
    "TLV", // TLV
    "ASN", // ASN1
    "CR",  // Crypto
    "CTL", // Controller
    "AL",  // Alarm
    "SC",  // SecureChannel
    "BDX", // BulkDataTransfer
    "DMG", // DataManagement
    "DC",  // DeviceControl
    "DD",  // DeviceDescription
    "ECH", // Echo
    "FP",  // FabricProvisioning
    "NP",  // NetworkProvisioning
    "SD",  // ServiceDirectory
    "SP",  // ServiceProvisioning
    "SWU", // SoftwareUpdate
    "FS",  // FailSafe
    "TS",  // TimeService
    "HB",  // Heartbeat
    "CSL", // chipSystemLayer
    "EVL", // Event Logging
    "SPT", // Support
    "TOO", // chipTool
    "ZCL", // Zcl
    "SH",  // Shell
    "DL",  // DeviceLayer
    "SPL", // SetupPayload
    "SVR", // AppServer
    "DIS", // Discovery
    "IM",  // InteractionModel
    "TST", // Test
    "OSS", // OperationalSessionSetup
    "ATM", // Automation
    "CSM", // CASESessionManager
};

static char const * GetModuleName(LogModule module)
{
    return ModuleNames[(module < kLogModule_Max) ? module : kLogModule_NotSpecified];
}

} // namespace

void SetLogRedirectCallback(LogRedirectCallback_t callback)
{
    sLogRedirectCallback.store(callback);
}

/**
 * Log, to the platform-specified mechanism, the specified log
 * message, @a msg, for the specified module, @a module, in the
 * provided category, @a category.
 *
 * @param[in] module    A LogModule enumeration indicating the
 *                      source of the chip package module that
 *                      generated the log message. This must be
 *                      translated within the function to a module
 *                      name for inclusion in the log message.
 * @param[in] category  A LogCategory enumeration indicating the
 *                      category of the log message. The category
 *                      may be filtered in or out if
 *                      CHIP_LOG_FILTERING was asserted.
 * @param[in] msg       A pointer to a NULL-terminated C string with
 *                      C Standard Library-style format specifiers
 *                      containing the log message to be formatted and
 *                      logged.
 * @param[in] ...       A variadic argument list whose elements should
 *                      correspond to the format specifiers in @a msg.
 *
 */
void Log(uint8_t module, uint8_t category, const char * msg, ...)
{

    va_list v;
    va_start(v, msg);
    LogV(module, category, msg, v);
    va_end(v);
}

void LogByteSpan(uint8_t module, uint8_t category, const chip::ByteSpan & span)
{
    // Maximum number of characters needed to print 8 byte buffer including formatting (0x)
    // 8 bytes * (2 nibbles per byte + 4 character for ", 0x") + null termination.
    // Rounding up to 50 bytes.
    char output[50];
    size_t offset = 0;
    for (unsigned int i = 0; i < span.size(); i++)
    {
        if (i % 8 == 0 && offset != 0)
        {
            Log(module, category, "%s", output);
            offset = 0;
        }
        int result = snprintf(&output[offset], sizeof(output) - offset, "0x%02x, ", (unsigned char) span.data()[i]);
        if (result > 0)
        {
            offset += static_cast<size_t>(result);
        }
        else
        {
            Log(module, chip::Logging::kLogCategory_Error, "Failed to print ByteSpan buffer");
            return;
        }
    }
    if (offset != 0)
    {
        Log(module, category, "%s", output);
    }
}

void LogV(uint8_t module, uint8_t category, const char * msg, va_list args)
{
    const char * moduleName        = GetModuleName(static_cast<LogModule>(module));
    LogRedirectCallback_t redirect = sLogRedirectCallback.load();
    if (redirect != nullptr)
    {
        redirect(moduleName, category, msg, args);
    }
    else
    {
        Platform::LogV(moduleName, category, msg, args);
    }
}

#if CHIP_LOG_FILTERING
uint8_t gLogFilter = kLogCategory_Max;

uint8_t GetLogFilter()
{
    return gLogFilter;
}

void SetLogFilter(uint8_t category)
{
    gLogFilter = category;
}

#else  // CHIP_LOG_FILTERING

uint8_t GetLogFilter()
{
    return kLogCategory_Max;
}

void SetLogFilter(uint8_t category)
{
    IgnoreUnusedVariable(category);
}
#endif // CHIP_LOG_FILTERING

#if CHIP_LOG_FILTERING
bool IsCategoryEnabled(uint8_t category)
{
    return (category <= gLogFilter);
}
#endif // CHIP_LOG_FILTERING

#endif // _CHIP_USE_LOGGING

} // namespace Logging
} // namespace chip
