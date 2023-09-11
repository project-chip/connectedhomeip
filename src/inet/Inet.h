/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file is an umbrella header for the Inet library, a
 *      portable Internet Protocol (IP) network interface.
 *
 */

#pragma once

#include <inet/InetConfig.h>

#include <inet/IPAddress.h>
#include <inet/IPPrefix.h>
#include <inet/InetError.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <inet/TCPEndPoint.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
#include <inet/UDPEndPoint.h>
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
