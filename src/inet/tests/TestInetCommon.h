/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
* SPDX-License-Identifier: Apache-2.0
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

#include <inet/TCPEndPointImpl.h>
#include <inet/UDPEndPointImpl.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemLayerImpl.h>

#define CHIP_TOOL_COPYRIGHT "SPDX-FileCopyrightText: 2020 Project CHIP Authors\n"

#define INET_FAIL_ERROR(ERR, MSG)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        InetFailError((ERR), (MSG));                                                                                               \
    } while (0)

extern chip::System::LayerImpl gSystemLayer;

extern chip::Inet::TCPEndPointManagerImpl gTCP;
extern chip::Inet::UDPEndPointManagerImpl gUDP;

extern bool gDone;

void InitTestInetCommon();
void InitSystemLayer();
void ShutdownSystemLayer();
void ShutdownTestInetCommon();
void InetFailError(CHIP_ERROR err, const char * msg);

void InitNetwork();
void ServiceEvents(uint32_t aSleepTimeMilliseconds);
void ShutdownNetwork();
void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix, uint32_t rowWidth);
void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix);

inline static void ServiceNetwork(uint32_t aSleepTimeMilliseconds)
{
    ServiceEvents(aSleepTimeMilliseconds);
}
