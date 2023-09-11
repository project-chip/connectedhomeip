/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on Open IOT SDK platform.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0

// ========== Platform-specific Configuration Overrides =========

#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
