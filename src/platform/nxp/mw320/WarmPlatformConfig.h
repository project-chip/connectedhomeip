/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the Chip
 *          Addressing and Routing Module (WARM) on rt1050 platforms
 *          using the NXP SDK.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define WARM_CONFIG_SUPPORT_THREAD 0
#define WARM_CONFIG_SUPPORT_THREAD_ROUTING 0
#define WARM_CONFIG_SUPPORT_LEGACY6LOWPAN_NETWORK 0
#define WARM_CONFIG_SUPPORT_WIFI 0
#define WARM_CONFIG_SUPPORT_CELLULAR 0

// ========== Platform-specific Configuration Overrides =========

/* none so far */
