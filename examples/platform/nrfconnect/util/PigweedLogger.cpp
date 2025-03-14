/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/*
 * @file PigweedLogger.cpp
 *
 * This file contains a backend of Zephyr logging system, based on Pigweed HDLC
 * over UART transport. It allows to send log messages even if the application
 * needs to use HDLC/UART for another purpose like the RPC server.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_backend_std.h>
#include <zephyr/logging/log_output.h>

#include <pw_hdlc/encoder.h>
#include <pw_stream/sys_io_stream.h>
#include <pw_sys_io_nrfconnect/init.h>

#include "pw_span/span.h"
#include <cassert>
#include <cstdint>
#include <string_view>

namespace PigweedLogger {
namespace {

#if CONFIG_LOG

#if !CONFIG_LOG_MODE_IMMEDIATE
#error "Backend of Zephyr logger based on Pigweed HDLC requires LOG_MODE_IMMEDIATE=y"
#endif

constexpr uint8_t kLogHdlcAddress = 1;   // Send log messages to HDLC address 1 (other than RPC communication)
constexpr size_t kWriteBufferSize = 128; // Buffer for constructing HDLC frames

// Exclusive access to the backend is needed to make sure that log messages coming
// from different threads are not interwoven.
K_SEM_DEFINE(sLoggerLock, 1, 1);
pw::stream::SysIoWriter sWriter;
size_t sWriteBufferPos;
uint8_t sWriteBuffer[kWriteBufferSize];
bool sIsPanicMode;

void flush()
{
    pw::hdlc::WriteUIFrame(kLogHdlcAddress, pw::as_bytes(pw::span(sWriteBuffer, sWriteBufferPos)), sWriter);
    sWriteBufferPos = 0;
}

int putString(uint8_t * buffer, size_t size, void * /* ctx */)
{
    assert(sWriteBufferPos < kWriteBufferSize);

    for (size_t i = 0; i < size; ++i)
    {
        // Send each line excluding "\r\n" in a separate frame

        if (buffer[i] == '\r')
            continue;

        if (buffer[i] == '\n')
        {
            flush();
            continue;
        }

        sWriteBuffer[sWriteBufferPos++] = buffer[i];

        if (sWriteBufferPos == kWriteBufferSize)
            flush();
    }

    return size;
}

LOG_OUTPUT_DEFINE(pigweedLogOutput, putString, nullptr, 0);

void init(const log_backend *)
{
    pw_sys_io_Init();
}

void processMessage(const struct log_backend * const backend, union log_msg_generic * msg)
{
    if (sIsPanicMode || k_is_in_isr())
    {
        return;
    }

    [[maybe_unused]] int ret = k_sem_take(&sLoggerLock, K_FOREVER);
    assert(ret == 0);

    log_format_func_t outputFunc = log_format_func_t_get(LOG_OUTPUT_TEXT);

    outputFunc(&pigweedLogOutput, &msg->log, log_backend_std_get_flags());

    k_sem_give(&sLoggerLock);
}

void panic(const log_backend *)
{
    sIsPanicMode = true;
}

const log_backend_api pigweedLogApi = {
    .process = processMessage,
    .panic   = panic,
    .init    = init,
};

LOG_BACKEND_DEFINE(pigweedLogBackend, pigweedLogApi, /* autostart */ true);

#endif // CONFIG_LOG

} // namespace

k_sem * GetSemaphore()
{
#if CONFIG_LOG
    return &sLoggerLock;
#else
    return nullptr;
#endif
}

} // namespace PigweedLogger
