/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      This file implements constants, globals and interfaces common to
 *      and used by all CHP Inet layer library test applications and
 *      tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "TestInetCommon.h"

#include <vector>

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include <support/CHIPMem.h>
#include <support/ErrorStr.h>
#include <support/ScopedBuffer.h>
#include <system/SystemTimer.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <arpa/inet.h>
#include <sys/select.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

using namespace chip;
using namespace chip::Inet;

System::Layer gSystemLayer;

Inet::InetLayer gInet;

char gDefaultTapDeviceName[32];
bool gDone = false;

void InetFailError(int32_t err, const char * msg)
{
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("%s: %s", msg, ErrorStr(err));
        exit(-1);
    }
}

void InitTestInetCommon()
{
    chip::Platform::MemoryInit();
}

void InitSystemLayer()
{
    gSystemLayer.Init(nullptr);
}

void ShutdownSystemLayer()
{
    gSystemLayer.Shutdown();
}

void InitNetwork()
{
    void * lContext = nullptr;

    gInet.Init(gSystemLayer, lContext);
}

void ServiceEvents(struct ::timeval & aSleepTime)
{
    static bool printed = false;

    if (!printed)
    {
        {
            LOG_INF("CHIP node ready to service events");
            printed = true;
        }
    }
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (gSystemLayer.State() == System::kLayerState_Initialized)
        gSystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (gInet.State == InetLayer::kState_Initialized)
        gInet.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        LOG_INF("select failed: %s", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (gSystemLayer.State() == System::kLayerState_Initialized)
    {

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

        gSystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
    }

    if (gInet.State == InetLayer::kState_Initialized)
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

        gInet.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
    }
}

void ShutdownNetwork()
{
    gInet.Shutdown();
}

#define DUMP_BUF_LEN 80

void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix, uint32_t rowWidth)
{
    int indexWidth = snprintf(nullptr, 0, "%X", len);

    if (indexWidth < 4)
        indexWidth = 4;

    for (uint32_t i = 0; i < len; i += rowWidth)
    {
        char buf[DUMP_BUF_LEN];
        char * ptr           = buf;
        const char * buf_end = buf + DUMP_BUF_LEN;
        uint32_t rowEnd;
        uint32_t j;
        int result = snprintf(ptr, DUMP_BUF_LEN, "%s%0*X: ", prefix, indexWidth, i);
        if (result < 0 || result >= DUMP_BUF_LEN)
            goto print_line;

        ptr += result;
        rowEnd = i + rowWidth;

        j = i;
        for (; j < rowEnd && j < len; j++)
        {
            result = snprintf(ptr, buf_end - ptr, "%02X ", mem[j]);
            if (result < 0 || result >= buf_end - ptr)
                goto print_line;
            ptr += result;
        }

        for (; j < rowEnd; j++)
        {
            result = snprintf(ptr, buf_end - ptr, "   ");
            if (result < 0 || result >= buf_end - ptr)
                goto print_line;
            ptr += result;
        }

        for (j = i; j < rowEnd && j < len; j++)
        {
            if (isprint(static_cast<char>(mem[j])))
                result = snprintf(ptr, buf_end - ptr, "%c", mem[j]);
            else
                result = snprintf(ptr, buf_end - ptr, ".");
            if (result < 0 || result >= buf_end - ptr)
                goto print_line;
            ptr += result;
        }

    print_line:
        if (result < 0 || result >= buf_end - ptr)
        {
            LOG_ERR("Dump buffer overflow");
        }
        if (ptr > buf && ptr < buf + DUMP_BUF_LEN)
        {
            *ptr = '\0';
            LOG_INF(buf);
        }
    }
}

void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix)
{
    const uint32_t kRowWidth = 16;

    DumpMemory(mem, len, prefix, kRowWidth);
}
