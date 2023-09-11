/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file declares test entry points for CHIP Internet (inet)
 *      layer library unit tests.
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int TestInetAddress(void);
int TestInetBuffer(void);
int TestInetErrorStr(void);
int TestInetTimer(void);
int TestInetEndPoint(void);
int TestInetLayerDNS(void);
#ifdef __cplusplus
}
#endif
