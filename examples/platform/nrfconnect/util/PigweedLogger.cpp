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

#include <logging/log.h>
#include <logging/log_backend.h>
#include <logging/log_backend_std.h>
#include <logging/log_output.h>
#include <zephyr.h>

#include <pw_hdlc/encoder.h>
#include <pw_stream/sys_io_stream.h>
#include <pw_sys_io_nrfconnect/init.h>

#include <cassert>
#include <cstdint>
#include <span>
#include <string_view>

namespace PigweedLogger {
namespace {

#if CONFIG_LOG

#if !CONFIG_LOG_IMMEDIATE
#error "Backend of Zephyr logger based on Pigweed HDLC requires LOG_IMMEDIATE=y"
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
    pw::hdlc::WriteUIFrame(kLogHdlcAddress, std::as_bytes(std::span(sWriteBuffer, sWriteBufferPos)), sWriter);
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

void init()
{
    pw_sys_io_Init();
}

void putMessageSync(const log_backend *, log_msg_ids srcLevel, uint32_t timestamp, const char * fmt, va_list args)
{
    int ret = k_sem_take(&sLoggerLock, K_FOREVER);
    assert(ret == 0);

    if (!sIsPanicMode)
        log_backend_std_sync_string(&pigweedLogOutput, 0, srcLevel, timestamp, fmt, args);

    k_sem_give(&sLoggerLock);
}

void putHexdumpSync(const log_backend *, log_msg_ids srcLevel, uint32_t timestamp, const char * metadata, const uint8_t * data,
                    uint32_t length)
{
    int ret = k_sem_take(&sLoggerLock, K_FOREVER);
    assert(ret == 0);

    if (!sIsPanicMode)
        log_backend_std_sync_hexdump(&pigweedLogOutput, 0, srcLevel, timestamp, metadata, data, length);

    k_sem_give(&sLoggerLock);
}

void panic(const log_backend *)
{
    int ret = k_sem_take(&sLoggerLock, K_FOREVER);
    assert(ret == 0);

    log_backend_std_panic(&pigweedLogOutput);
    flush();
    sIsPanicMode = true;

    k_sem_give(&sLoggerLock);
}

const log_backend_api pigweedLogApi = {
    .put              = nullptr,
    .put_sync_string  = putMessageSync,
    .put_sync_hexdump = putHexdumpSync,
    .panic            = panic,
    .init             = init,
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
