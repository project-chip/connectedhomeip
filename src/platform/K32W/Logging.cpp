/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *          Provides implementations for the CHIP and LwIP logging
 *          functions on NXP K32W platforms.
 */

#define K32W_LOG_MODULE_NAME chip
#define EOL_CHARS "\r\n" /* End of Line Characters */
#define EOL_CHARS_LEN 2  /* Length of EOL */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#include <openthread/platform/uart.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

extern "C" void K32WWriteBlocking(const uint8_t * aBuf, uint32_t len);

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::Logging;

namespace {

void GetModuleName(char * buf, uint8_t module)
{
    if (module == ::chip::Logging::kLogModule_DeviceLayer)
    {
        memcpy(buf, "DL", 3);
    }
    else
    {
        ::chip::Logging::GetModuleName(buf, module);
    }
}

void GetMessageString(char * buf, uint8_t chipCategory, uint8_t otLogLevel)
{
    if (chipCategory != kLogCategory_None)
    {
        switch (chipCategory)
        {
        case kLogCategory_Error:
            memcpy(buf, "[Error]", 7);
            break;
        case kLogCategory_Progress:
        case kLogCategory_Retain:
        default:
            memcpy(buf, "[Progress]", 10);
            break;
        case kLogCategory_Detail:
            memcpy(buf, "[Debug]", 7);
            break;
        }
    }

    if (otLogLevel != OT_LOG_LEVEL_NONE)
    {
        switch (otLogLevel)
        {
        case OT_LOG_LEVEL_CRIT:
            memcpy(buf, "[Error]", 7);
            break;
        case OT_LOG_LEVEL_WARN:
            memcpy(buf, "[Warn]", 6);
            break;
        case OT_LOG_LEVEL_NOTE:
        case OT_LOG_LEVEL_INFO:
        default:
            memcpy(buf, "[Info]", 6);
            break;
        case OT_LOG_LEVEL_DEBG:
            memcpy(buf, "[Debug]", 7);
            break;
        }
    }
}

void FillPrefix(char * buf, uint8_t bufLen, uint8_t chipCategory, uint8_t otLogLevel, uint8_t module)
{
    size_t prefixLen;

    /* add the error string */
    assert(bufLen > nlChipLoggingChipPrefixLen);
    ::GetMessageString(buf, chipCategory, otLogLevel);

    /* add the module name string */
    prefixLen = strlen(buf);
    assert(bufLen > (prefixLen + nlChipLoggingModuleNameLen + 3));
    buf[prefixLen++] = '[';
    GetModuleName(buf + prefixLen, module);
    prefixLen        = strlen(buf);
    buf[prefixLen++] = ']';
    buf[prefixLen++] = ' ';
}

} // unnamed namespace

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by CHIP or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace Logging {

/**
 * CHIP log output function.
 */
void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;

#if K32W_LOG_ENABLED

    if (IsCategoryEnabled(category))
    {
        {
            char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE - 1] = { 0 };
            size_t prefixLen, writtenLen;

            /* Prefix is composed of [Debug String][MOdule Name String] */
            FillPrefix(formattedMsg, CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE - 1, category, OT_LOG_LEVEL_NONE, module);
            prefixLen = strlen(formattedMsg);

            // Append the log message.
            writtenLen = vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen - EOL_CHARS_LEN, msg, v);
            assert(writtenLen > 0);
            memcpy(formattedMsg + prefixLen + writtenLen, EOL_CHARS, EOL_CHARS_LEN);

            K32WWriteBlocking((const uint8_t *) formattedMsg, strlen(formattedMsg));
        }
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // NRF_LOG_ENABLED
}

} // namespace Logging
} // namespace chip

#undef K32W_LOG_MODULE_NAME
#define K32W_LOG_MODULE_NAME lwip

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

#if K32W_LOG_ENABLED
    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE - 1];

        // Append the log message.
        size_t len = vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

        while (len > 0 && isspace(formattedMsg[len - 1]))
        {
            len--;
            formattedMsg[len] = 0;
        }

        K32WWriteBlocking((const uint8_t *) formattedMsg, strlen(formattedMsg));
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // NRF_LOG_ENABLED

    va_end(v);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#undef K32W_LOG_MODULE_NAME
#define K32W_LOG_MODULE_NAME thread

extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    va_list v;

    (void) aLogLevel;
    (void) aLogRegion;

    va_start(v, aFormat);

#if K32W_LOG_ENABLED
    char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE - 1] = { 0 };
    size_t prefixLen, writtenLen;

    /* Prefix is composed of [Debug String][MOdule Name String] */
    FillPrefix(formattedMsg, CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE - 1, kLogCategory_None, aLogLevel,
               (uint8_t) kLogModule_NotSpecified);
    prefixLen = strlen(formattedMsg);

    // Append the log message.
    writtenLen = vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen - EOL_CHARS_LEN, aFormat, v);
    assert(writtenLen > 0);
    memcpy(formattedMsg + prefixLen + writtenLen, EOL_CHARS, EOL_CHARS_LEN);

    K32WWriteBlocking((const uint8_t *) formattedMsg, strlen(formattedMsg));

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
#endif // NRF_LOG_ENABLED

    va_end(v);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
