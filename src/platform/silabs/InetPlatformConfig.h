/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on Silabs platforms using the Silicon Labs SDK.
 *
 */

#pragma once

#include <stdint.h>

// ==================== Platform Adaptations ====================

#define INET_CONFIG_ERROR_TYPE int32_t
#define INET_CONFIG_NO_ERROR 0
#define INET_CONFIG_ERROR_MIN 1000000
#define INET_CONFIG_ERROR_MAX 1000999

#ifndef INET_CONFIG_ENABLE_IPV4
#ifdef SL_WIFI
#define INET_CONFIG_ENABLE_IPV4 1
#else /* For Thread only devices */
#define INET_CONFIG_ENABLE_IPV4 0
#endif
#endif

// ========== Platform-specific Configuration Overrides =========

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 4
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 20
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS
