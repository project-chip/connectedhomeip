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

#include <pw_hdlc/encoder.h>
#include <pw_stream/sys_io_stream.h>
#include <pw_sys_io_mbed/init.h>

#include <assert.h>
#include <rtos/Mutex.h>

namespace PigweedLogger {
namespace {

constexpr uint8_t kLogHdlcAddress = 1;   // Send log messages to HDLC address 1 (other than RPC communication)
constexpr size_t kWriteBufferSize = 128; // Buffer for constructing HDLC frames

rtos::Mutex mbed_log_mutex;

pw::stream::SysIoWriter sWriter;
size_t sWriteBufferPos;
char sWriteBuffer[kWriteBufferSize];

void send()
{
    pw::hdlc::WriteUIFrame(kLogHdlcAddress, pw::as_bytes(pw::span(sWriteBuffer, sWriteBufferPos)), sWriter);
    sWriteBufferPos = 0;
}

} // namespace

void init()
{
    pw_sys_io_Init();
}

int putString(const char * buffer, size_t size)
{
    mbed_log_mutex.lock();

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

    mbed_log_mutex.unlock();
    return size;
}

rtos::Mutex * GetSemaphore()
{
    return &mbed_log_mutex;
}

} // namespace PigweedLogger
