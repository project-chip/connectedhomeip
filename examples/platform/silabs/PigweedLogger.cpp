/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * @file PigweedLogger.cpp
 *
 * This file contains basic string writting function, based on Pigweed HDLC
 * over UART transport. It allows to send log messages even if the application
 * needs to use HDLC/UART for another purpose like the RPC server.
 */

#include <FreeRTOS.h>

#include "semphr.h"
#include <pw_hdlc/encoder.h>
#include <pw_stream/sys_io_stream.h>
#include <pw_sys_io_efr32/init.h>

#include "pw_span/span.h"
#include <cassert>
#include <cstdint>
#include <string_view>

namespace PigweedLogger {
namespace {

constexpr uint8_t kLogHdlcAddress = 1;   // Send log messages to HDLC address 1 (other than RPC communication)
constexpr size_t kWriteBufferSize = 128; // Buffer for constructing HDLC frames

// Exclusive access to the backend is needed to make sure that log messages coming
// from different threads are not interwoven.
SemaphoreHandle_t sLoggerLock;

static pw::stream::SysIoWriter sWriter;
static size_t sWriteBufferPos;
static char sWriteBuffer[kWriteBufferSize];

static void send(void)
{
    pw::hdlc::WriteUIFrame(kLogHdlcAddress, pw::as_bytes(pw::span(sWriteBuffer, sWriteBufferPos)), sWriter);
    sWriteBufferPos = 0;
}

} // namespace

void init(void)
{
    sLoggerLock = xSemaphoreCreateMutex();
    assert(sLoggerLock != NULL);

    pw_sys_io_Init();
}

int putString(const char * buffer, size_t size)
{
    xSemaphoreTake(sLoggerLock, portMAX_DELAY);
    assert(sWriteBufferPos < kWriteBufferSize);

    for (size_t i = 0; i < size; ++i)
    {
        // Send each line excluding "\r\n" in a separate frame

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

    xSemaphoreGive(sLoggerLock);
    return size;
}

SemaphoreHandle_t * GetSemaphore()
{
    return &sLoggerLock;
}

} // namespace PigweedLogger
