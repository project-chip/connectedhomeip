/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on Open IoT SDK platform.
 *
 */

#pragma once

#include <stdint.h>

// ==================== Platform Adaptations ====================
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#define CHIP_SYSTEM_CONFIG_MBED_LOCKING 0
#define CHIP_SYSTEM_CONFIG_NO_LOCKING 0
#define CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING 1
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 1

#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#ifndef CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
#define CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME 0
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

#define CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT 1

// ========== Platform-specific Configuration Overrides =========
