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
#include "pw_sys_io_ameba/init.h"
#include "semphr.h"
#include <lib/support/logging/TextOnlyLogging.h>
#include <pw_hdlc/encoder.h>
#include <pw_stream/sys_io_stream.h>

#include "pw_span/span.h"
#include <assert.h>

namespace PigweedLogger {
namespace {

constexpr uint8_t kLogHdlcAddress = 1;   // Send log messages to HDLC address 1 (other than RPC communication)
constexpr size_t kWriteBufferSize = 128; // Buffer for constructing HDLC frames

SemaphoreHandle_t ameba_log_mutex;

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
    ameba_log_mutex = xSemaphoreCreateMutex();
    assert(ameba_log_mutex != NULL);
    pw_sys_io_Init();
    uartInitialised = true;
}

int putString(const char * buffer, size_t size)
{
    xSemaphoreTake(ameba_log_mutex, portMAX_DELAY);

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

    xSemaphoreGive(ameba_log_mutex);
    return size;
}

SemaphoreHandle_t * getSemaphore()
{
    return &ameba_log_mutex;
}

} // namespace PigweedLogger
