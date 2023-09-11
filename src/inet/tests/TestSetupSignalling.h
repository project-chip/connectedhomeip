/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file sets up signalling used in Linux CHIP Inet layer library test
 *      applications and tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#pragma once

void SetSIGUSR1Handler();
typedef void (*SignalHandler)(int signum);
void SetSignalHandler(SignalHandler handler);
