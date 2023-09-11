/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on the PSoC6 platform.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define INET_CONFIG_ERROR_TYPE cy_rslt_t
#define INET_CONFIG_NO_ERROR CY_RSLT_SUCCESS
#define INET_CONFIG_ERROR_MIN 1000000
#define INET_CONFIG_ERROR_MAX 1000999
