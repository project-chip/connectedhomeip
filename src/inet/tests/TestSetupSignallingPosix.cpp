/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
