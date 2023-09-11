/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          Open IoT SDK platform.
 */

#pragma once

#include <stdint.h>

#include <ois/OpenIoTSDKConfig.h>

// ==================== General Platform Adaptations ====================

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif

// ==================== Security Adaptations ====================

// ==================== General Configuration Overrides ====================
