/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "FreeRTOS.h"
#include "esp_log.h"
#include "pw_sys_io_esp32/init.h"
#include "semphr.h"
#include <lib/support/logging/CHIPLogging.h>
#include <pw_hdlc/encoder.h>
#include <pw_stream/sys_io_stream.h>

#if CONFIG_ENABLE_ESP_INSIGHTS_TRACE && CONFIG_DIAG_USE_EXTERNAL_LOG_WRAP
#include <esp_diagnostics.h>
#endif

namespace PigweedLogger {
namespace {

constexpr uint8_t kLogHdlcAddress = 1;   // Send log messages to HDLC address 1 (other than RPC communication)
constexpr size_t kWriteBufferSize = 128; // Buffer for constructing HDLC frames

SemaphoreHandle_t esp_log_mutex;

pw::stream::SysIoWriter sWriter;
size_t sWriteBufferPos;
char sWriteBuffer[kWriteBufferSize];

bool uartInitialised;

void send()
{
    pw::hdlc::WriteUIFrame(kLogHdlcAddress, pw::as_bytes(pw::span(sWriteBuffer, sWriteBufferPos)), sWriter);
    sWriteBufferPos = 0;
}

} // namespace

void init()
{
    esp_log_mutex = xSemaphoreCreateMutex();
    assert(esp_log_mutex != NULL);
    pw_sys_io_Init();
    uartInitialised = true;
}

int putString(const char * buffer, size_t size)
{
    xSemaphoreTake(esp_log_mutex, portMAX_DELAY);

    assert(sWriteBufferPos < kWriteBufferSize);

    for (size_t i = 0; i < size; ++i)
    {

        if (buffer[i] == '\r')
            continue;

        if (buffer[i] == '\n')
        {
            send();
            continue;
        }

        sWriteBuffer[sWriteBufferPos++] = buffer[i];

        if (sWriteBufferPos == kWriteBufferSize)
            send();
    }

    xSemaphoreGive(esp_log_mutex);
    return size;
}

SemaphoreHandle_t * getSemaphore()
{
    return &esp_log_mutex;
}

static const char * getLogColorForLevel(esp_log_level_t level)
{
    switch (level)
    {
    case ESP_LOG_ERROR:
        return LOG_COLOR_E "E";

    case ESP_LOG_INFO:
        return LOG_COLOR_I "I";

    default:
        // default is kept as ESP_LOG_DEBUG
        return LOG_COLOR_D "D";
    }
}

// ESP_LOGx(...) logs are funneled to esp_log_write() and it has the specific format. It contains color codes,
// log level character, timestamp, tag, and actual log message. Everything here is part of format and variadic argument.
//
// ChipLogx(...) logs are funneled to esp_log_writev() will only have actual log message without color codes, log level
// character, timestamp, and tag. So, to match up __wrap_esp_log_writev() adds those details when sending log to pigweed
// logger.
extern "C" void __wrap_esp_log_write(esp_log_level_t level, const char * tag, const char * format, ...)
{
    va_list v;
    va_start(v, format);

#ifndef CONFIG_LOG_DEFAULT_LEVEL_NONE
    if (uartInitialised && level <= CONFIG_LOG_MAXIMUM_LEVEL)
    {
        char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t len = vsnprintf(formattedMsg, sizeof formattedMsg, format, v);
        if (len >= sizeof formattedMsg)
        {
            len = sizeof formattedMsg - 1;
        }
        PigweedLogger::putString(formattedMsg, len);
    }
#endif

#if CONFIG_ENABLE_ESP_INSIGHTS_TRACE && CONFIG_DIAG_USE_EXTERNAL_LOG_WRAP
    esp_diag_log_writev(level, tag, format, v);
#endif

    va_end(v);
}

extern "C" void __wrap_esp_log_writev(esp_log_level_t level, const char * tag, const char * format, va_list v)
{
#ifndef CONFIG_LOG_DEFAULT_LEVEL_NONE
    if (uartInitialised && level <= CONFIG_LOG_MAXIMUM_LEVEL)
    {
        const char * logColor = getLogColorForLevel(level);
        PigweedLogger::putString(logColor, strlen(logColor));

        char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t len = snprintf(formattedMsg, sizeof formattedMsg, " (%" PRIu32 ") %s: ", esp_log_timestamp(), tag);
        PigweedLogger::putString(formattedMsg, len);

        memset(formattedMsg, 0, sizeof formattedMsg);
        len = vsnprintf(formattedMsg, sizeof formattedMsg, format, v);
        if (len >= sizeof formattedMsg)
        {
            len = sizeof formattedMsg - 1;
        }
        PigweedLogger::putString(formattedMsg, len);

        const char * logResetColor = LOG_RESET_COLOR "\n";
        PigweedLogger::putString(logResetColor, strlen(logResetColor));
    }
#endif

#if CONFIG_ENABLE_ESP_INSIGHTS_TRACE && CONFIG_DIAG_USE_EXTERNAL_LOG_WRAP
    esp_diag_log_write(level, tag, format, v);
#endif
}

} // namespace PigweedLogger
