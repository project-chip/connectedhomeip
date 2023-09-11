/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      chip::System project configuration for standalone builds on Linux and OS X.
 *
 */
#ifndef SYSTEMPROJECTCONFIG_H
#define SYSTEMPROJECTCONFIG_H

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
// Uncomment this for larger buffers (e.g. to support a bigger CHIP_CONFIG_TUNNEL_INTERFACE_MTU).
//#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX 9050
#endif

#endif /* SYSTEMPROJECTCONFIG_H */
