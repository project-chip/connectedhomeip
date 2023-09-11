/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the Openchip Inet
 *          Layer on CC32XX platforms using the Texas Instruments SDK.
 *
 */

#ifndef INET_PLATFORM_CONFIG_H
#define INET_PLATFORM_CONFIG_H

// XXX: Seth update config?

// ==================== Platform Adaptations ====================

// XXX: Seth is there a proper platform error type??
#define INET_CONFIG_ERROR_TYPE uint32_t
#define INET_CONFIG_NO_ERROR (0)

#define INET_CONFIG_ERROR_MIN 1000000
#define INET_CONFIG_ERROR_MAX 1000999

#define INET_CONFIG_ENABLE_IPV4 1

// ========== Platform-specific Configuration Overrides =========

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 4
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 8
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

#endif // INET_PLATFORM_CONFIG_H
