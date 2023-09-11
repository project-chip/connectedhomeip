/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on Tizen platforms.
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

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 32
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS
