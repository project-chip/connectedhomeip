/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file defines constants, globals and interfaces common to
 *      and used by all CHP Inet layer library test applications and
 *      tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#pragma once

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <ctype.h>
#include <time.h>

#include <sys/time.h>

#include <inet/InetLayer.h>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>

#define CHIP_TOOL_COPYRIGHT "Copyright (c) 2020 Project CHIP Authors\nAll rights reserved.\n"

#define INET_FAIL_ERROR(ERR, MSG)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        InetFailError((ERR), (MSG));                                                                                               \
    } while (0)

extern chip::System::Layer gSystemLayer;

extern chip::Inet::InetLayer gInet;

extern bool gDone;

void InitTestInetCommon();
void InitSystemLayer();
void ShutdownSystemLayer();
void InetFailError(int32_t err, const char * msg);

void InitNetwork();
void ServiceEvents(struct ::timeval & aSleepTime);
void ShutdownNetwork();
void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix, uint32_t rowWidth);
void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix);

inline static void ServiceNetwork(struct ::timeval & aSleepTime)
{
    ServiceEvents(aSleepTime);
}
