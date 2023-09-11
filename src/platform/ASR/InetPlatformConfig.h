/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

// ==================== Platform Adaptations ====================

#define INET_CONFIG_ERROR_TYPE uint32_t
#define INET_CONFIG_NO_ERROR 0
#define INET_CONFIG_ERROR_MIN 1000000
#define INET_CONFIG_ERROR_MAX 1000999

// ==================== General Configuration Overrides ====================

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 10
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 20
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS
