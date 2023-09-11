/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          the ASR platform.
 */

#pragma once

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID "enc-msg-counter"

#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 15

#define CHIP_CONFIG_ERROR_TYPE uint32_t
#define CHIP_CONFIG_NO_ERROR 0
#define CHIP_CONFIG_ERROR_MIN 4000000
#define CHIP_CONFIG_ERROR_MAX 4000999
#define CHIP_CONFIG_ERROR_FORMAT PRId32

#define ASN1_CONFIG_ERROR_TYPE uint32_t
#define ASN1_CONFIG_NO_ERROR 0
#define ASN1_CONFIG_ERROR_MIN 5000000
#define ASN1_CONFIG_ERROR_MAX 5000999

#define CHIP_CONFIG_ABORT() abort()

// ==================== Security Adaptations ====================

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif // CHIP_CONFIG_MAX_FABRICS

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 16
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 20
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING
