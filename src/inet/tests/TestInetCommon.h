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
 *      and used by all Weave test applications and tools.
 *
 *      NOTE: These do not comprise a public part of the Weave API and
 *            are subject to change without notice.
 *
 */

#ifndef CHIP_TEST_INET_COMMON_H_
#define CHIP_TEST_INET_COMMON_H_

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/time.h>

#include <nlfaultinjection.hpp>

#include <inet/InetLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <system/SystemLayer.h>

#include "TestInetCommonOptions.h"

#define CHIP_TOOL_COPYRIGHT "Copyright (c) 2020 Project CHIP Authors\nAll rights reserved.\n"

extern chip::System::Layer gSystemLayer;

extern chip::Inet::InetLayer gInet;

extern void SetSIGUSR1Handler(void);
extern void InitSystemLayer(void);
extern void ShutdownSystemLayer(void);

typedef void (*SignalHandler)(int signum);

extern void SetSignalHandler(SignalHandler handler);

extern void InitNetwork(void);
extern void ServiceEvents(struct ::timeval & aSleepTime);
extern void ShutdownNetwork(void);

inline static void ServiceNetwork(struct ::timeval &aSleepTime)
{
    ServiceEvents(aSleepTime);
}

#endif /* CHIP_TEST_INET_COMMON_H_ */
