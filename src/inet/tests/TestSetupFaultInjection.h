/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file sets up fault injection for all CHIP Inet layer library test applications
 *      and tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#pragma once

#include <ctype.h>
#include <nlfaultinjection.hpp>

extern bool gSigusr1Received;

void SetupFaultInjectionContext(int argc, char * argv[]);
void SetupFaultInjectionContext(int argc, char * argv[], int32_t (*aNumEventsAvailable)(),
                                void (*aInjectAsyncEvents)(int32_t index));
