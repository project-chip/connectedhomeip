/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines compile time configuration of the Event Logging profile
 */

#pragma once

/**
 * @def CHIP_CONFIG_EVENT_SIZE_RESERVE
 *
 * @brief
 *   Initial size to reserve for event. The logging subsystem ensures
 *   this space is available in the buffer before attempting to store
 *   the event.
 */
#ifndef CHIP_CONFIG_EVENT_SIZE_RESERVE
#define CHIP_CONFIG_EVENT_SIZE_RESERVE 196
#endif /* CHIP_CONFIG_EVENT_SIZE_RESERVE */

/**
 * @def CHIP_CONFIG_EVENT_SIZE_INCREMENT
 *
 * @brief
 *   When the initial reserved size proves insufficient, clean out the
 *   buffer in following increments.
 */
#ifndef CHIP_CONFIG_EVENT_SIZE_INCREMENT
#define CHIP_CONFIG_EVENT_SIZE_INCREMENT 8
#endif /* CHIP_CONFIG_EVENT_SIZE_INCREMENT */

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_MAXIMUM_UPLOAD_SECONDS
 *
 * @brief
 *   The maximum time (in seconds) between log uploads.  The
 *   system is guaranteed to attempt to offload the log buffer at
 *   least this often.  The value is used to initialize chip
 *   LoggingConfiguration. This is only relevant for BDX uploads.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_MAXIMUM_UPLOAD_SECONDS
#define CHIP_CONFIG_EVENT_LOGGING_MAXIMUM_UPLOAD_SECONDS 86400
#endif

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
 *
 * @brief
 *   By default, don't record UTC timestamps.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
#define CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 0
#endif

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT
 *
 * @brief
 *   Enable or disable support for externally managed event storage
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT
#define CHIP_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT 0
#endif
