/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on Android platforms.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================
#ifndef INET_CONFIG_ENABLE_IPV4
#define INET_CONFIG_ENABLE_IPV4 1
#endif // INET_CONFIG_ENABLE_IPV4

// ========== Platform-specific Configuration Overrides =========

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 32
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 32
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

// On linux platform, we have sys/socket.h, so HAVE_SO_BINDTODEVICE should be set to 1
#define HAVE_SO_BINDTODEVICE 1
