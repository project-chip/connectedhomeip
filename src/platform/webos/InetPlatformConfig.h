/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on webOS platforms.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#ifndef INET_CONFIG_ENABLE_IPV4
#error Inet IPv4 configuration should be configured at build generation time
#endif

// ========== Platform-specific Configuration Overrides =========

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 32
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef IPV6_MULTICAST_IMPLEMENTED
#define IPV6_MULTICAST_IMPLEMENTED
#endif

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 32
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

// On linux platform, we have sys/socket.h, so HAVE_SO_BINDTODEVICE should be set to 1
#define HAVE_SO_BINDTODEVICE 1
