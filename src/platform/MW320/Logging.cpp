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
#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>
#include <platform/CHIPDeviceConfig.h>
#include <src/lib/support/CodeUtils.h>

#include <cstring>

#define MW320_LOG_MODULE_NAME chip
#define EOL_CHARS "\r\n" /* End of Line Characters */
#define EOL_CHARS_LEN 2  /* Length of EOL */

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#include <utils/uart.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

static bool isLogInitialized;
extern uint8_t gOtLogUartInstance;

namespace chip {
namespace Logging {
namespace Platform {

void GetMessageString(char * buf, uint8_t chipCategory, uint8_t otLevelLog)
{
    if (chipCategory != kLogCategory_None)
    {
        switch (chipCategory)
        {
        case kLogCategory_Error:
            memcpy(buf, "[Error]", 7);
            break;
        case kLogCategory_Progress:
        default:
            memcpy(buf, "[Progress]", 10);
            break;
        case kLogCategory_Detail:
            memcpy(buf, "[Debug]", 7);
            break;
        }
    }

    /*if (otLevelLog != OT_LOG_LEVEL_NONE)
    {
        switch (otLevelLog)
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
    }*/
}

} // namespace Platform
} // namespace Logging
} // namespace chip

void FillPrefix(char * buf, uint8_t bufLen, uint8_t chipCategory, uint8_t otLevelLog)
{
    /* add the error string */
    chip::Logging::Platform::GetMessageString(buf, chipCategory, otLevelLog);
}
// unnamed namespace

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

void GenericLog(const char * format, va_list arg)
{

#if MW320_LOG_ENABLED

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - 1] = { 0 };
    size_t prefixLen, writtenLen;

    if (!isLogInitialized)
    {
        isLogInitialized = true;
        // gOtLogUartInstance = 0;
        // otPlatUartEnable();
    }

    /* Prefix is composed of [Debug String][MOdule Name String] */
    FillPrefix(formattedMsg, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - 1, chip::Logging::kLogCategory_None,
               chip::Logging::kLogCategory_Detail);
    prefixLen = strlen(formattedMsg);

    // Append the log message.
    writtenLen = vsnprintf(formattedMsg + prefixLen, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - prefixLen - EOL_CHARS_LEN, format, arg);
    VerifyOrDie(writtenLen > 0);
    memcpy(formattedMsg + prefixLen + writtenLen, EOL_CHARS, EOL_CHARS_LEN);

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();

#endif // MW320_LOG_ENABLED
}

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output function.
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;
    char msgbuf[256];

    PRINTF("%s: ", module);
    vsprintf(msgbuf, msg, v);
    PRINTF("%s", msgbuf);
    PRINTF("\r\n");

#if MW320_LOG_ENABLED
    GenericLog(msg, v);
    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // MW320_LOG_ENABLED
}

} // namespace Platform
} // namespace Logging
} // namespace chip

#undef MW320_LOG_MODULE_NAME
#define MW320_LOG_MODULE_NAME lwip

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);
    GenericLog(msg, v);
    va_end(v);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#undef MW320_LOG_MODULE_NAME
#define MW320_LOG_MODULE_NAME thread

extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    va_list v;

    (void) aLogLevel;
    (void) aLogRegion;

    va_start(v, aFormat);
    GenericLog(aFormat, v);
    va_end(v);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
