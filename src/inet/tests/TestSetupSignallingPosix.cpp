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
 *      This file sets up signalling for all POSIX CHIP Inet layer library test
 *      applications and tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#include "TestSetupSignalling.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ExitOnSIGUSR1Handler(int signum)
{
    // exit() allows us a slightly better clean up (gcov data) than SIGINT's exit
    exit(0);
}

// We set a hook to exit when we receive SIGUSR1, SIGTERM or SIGHUP
void SetSIGUSR1Handler()
{
    SetSignalHandler(ExitOnSIGUSR1Handler);
}

void SetSignalHandler(SignalHandler handler)
{
    struct sigaction sa;
    int signals[] = { SIGUSR1 };
    size_t i;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;

    for (i = 0; i < sizeof(signals) / sizeof(signals[0]); i++)
    {
        if (sigaction(signals[i], &sa, nullptr) == -1)
        {
            perror("Can't catch signal");
            exit(1);
        }
    }
}
