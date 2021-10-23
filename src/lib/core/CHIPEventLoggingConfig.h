/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
