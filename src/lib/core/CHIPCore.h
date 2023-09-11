/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file is the master "umbrella" include file for the core
 *      chip library.
 *
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#include <system/SystemLayer.h>

#include <ble/BleConfig.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <inet/TCPEndPoint.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
#include <inet/UDPEndPoint.h>
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#define CHIP_CORE_IDENTITY "chip-core"
#define CHIP_CORE_PREFIX CHIP_CORE_IDENTITY ": "

#include <lib/core/CHIPError.h>
