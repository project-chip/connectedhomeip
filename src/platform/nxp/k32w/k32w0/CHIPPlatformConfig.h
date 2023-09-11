/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          NXP K32W platforms.
 */

#pragma once

#include <stdint.h>

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE uint16_t
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID 1
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY 0x01

#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE (512)
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE (512)

// ==================== Security Adaptations ====================

// FIXME: K32W currently set to CHIP (Does this use Entropy.cpp ?)

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS

// ==================== WDM Configuration Overrides ====================

#ifndef WDM_MAX_NUM_SUBSCRIPTION_CLIENTS
#define WDM_MAX_NUM_SUBSCRIPTION_CLIENTS 2
#endif // WDM_MAX_NUM_SUBSCRIPTION_CLIENTS

#ifndef WDM_MAX_NUM_SUBSCRIPTION_HANDLERS
#define WDM_MAX_NUM_SUBSCRIPTION_HANDLERS 2
#endif // WDM_MAX_NUM_SUBSCRIPTION_HANDLERS

#ifndef WDM_PUBLISHER_MAX_NOTIFIES_IN_FLIGHT
#define WDM_PUBLISHER_MAX_NOTIFIES_IN_FLIGHT 2
#endif // WDM_PUBLISHER_MAX_NOTIFIES_IN_FLIGHT

#include "platform/nxp/common/CHIPNXPPlatformDefaultConfig.h"
