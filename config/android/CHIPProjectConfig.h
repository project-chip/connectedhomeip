/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-FileCopyrightText: 2019-2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Project-specific configuration file for Android builds.
 *
 */
#ifndef CHIPPROJECTCONFIG_H
#define CHIPPROJECTCONFIG_H

// Enable UDP listening on demand in the CHIPDeviceController
#define CHIP_CONFIG_DEVICE_CONTROLLER_DEMAND_ENABLE_UDP 1

#define INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT 0

// Uncomment this for a large Tunnel MTU.
//#define CHIP_CONFIG_TUNNEL_INTERFACE_MTU                           (9000)

#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 300

#define CHIP_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL 1

#define CHIP_CONFIG_MAX_SOFTWARE_VERSION_LENGTH 128

#endif /* CHIPPROJECTCONFIG_H */
