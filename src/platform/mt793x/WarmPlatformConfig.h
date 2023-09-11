/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the Chip
 *          Addressing and Routing Module (WARM) on Genio platforms
 *          using the MediaTek SDK.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define WARM_CONFIG_SUPPORT_THREAD 1
#define WARM_CONFIG_SUPPORT_THREAD_ROUTING 0
#define WARM_CONFIG_SUPPORT_LEGACY6LOWPAN_NETWORK 0
#define WARM_CONFIG_SUPPORT_WIFI 0
#define WARM_CONFIG_SUPPORT_CELLULAR 0

// ========== Platform-specific Configuration Overrides =========

/* none so far */
