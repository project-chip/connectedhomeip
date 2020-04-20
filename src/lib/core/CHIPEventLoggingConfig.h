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

#ifndef CHIPEVENTLOGGINGCONFIG_H
#define CHIPEVENTLOGGINGCONFIG_H

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
 *  @def CHIP_CONFIG_EVENT_LOGGING_BDX_OFFLOAD
 *
 *  @brief
 *    Control whether the logs are offloaded using the BDX profile
 *
 *   Select the ability to offload logs to the BDX endpoint.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_BDX_OFFLOAD
#define CHIP_CONFIG_EVENT_LOGGING_BDX_OFFLOAD 0
#endif /* CHIP_CONFIG_EVENT_LOGGING_BDX_OFFLOAD */

/**
 *  @def CHIP_CONFIG_EVENT_LOGGING_WDM_OFFLOAD
 *
 *  @brief
 *    Control whether the logs are offloaded using the WDM profile
 *
 *   Select the ability to offload logs to any interested subscribers using WDM.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_WDM_OFFLOAD
#define CHIP_CONFIG_EVENT_LOGGING_WDM_OFFLOAD 0
#endif /* CHIP_CONFIG_EVENT_LOGGING_WDM_OFFLOAD */

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_XFER_THRESHOLD
 *
 * @brief
 *   The log size threshold in bytes beyond which the system will
 *   trigger the automatic upload
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_XFER_THRESHOLD
#define CHIP_CONFIG_EVENT_LOGGING_XFER_THRESHOLD 512
#endif /* CHIP_CONFIG_EVENT_LOGGING_XFER_THRESHOLD */

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_EVENT_THRESHOLD
 *
 * @brief
 *  The number of locally stored events beyond which the system will
 *  trigger the automatic upload. Uses XFER_THRESHOLD and EVENT_SIZE_RESERVE
 *  to approximate the size of an average WDM notification
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_EVENT_THRESHOLD
#define CHIP_CONFIG_EVENT_LOGGING_EVENT_THRESHOLD (CHIP_CONFIG_EVENT_LOGGING_XFER_THRESHOLD / CHIP_CONFIG_EVENT_SIZE_RESERVE)
#endif

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD
 *
 * @brief The number of bytes written to the event logging system that
 *   will trigger WDM Notification.
 *
 * The configuration captures the number of bytes written to the event
 * logging subsystem needed to trigger a WDM notification. Generally,
 * the number does not account for events supplied via the external
 * event subsystem. The number should be set both in terms of the
 * notify size and in terms of the desired quality of service.  For
 * example, if an application wants to offload all DEBUG events
 * reliably, the threshold should be set to less than the size of the
 * DEBUG buffer (plus a slop factor to account for events generated
 * during the scheduling and event offload).  Similarly, if the
 * application does not want to drop INFO events, the threshold should
 * be set to the sum of DEBUG and INFO buffers (with the same
 * correction).
 *
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD
#define CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD 512
#endif /* CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD */

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_MINIMUM_UPLOAD_SECONDS
 *
 * @brief
 *   Default minimum time between log uploads (in seconds). If
 *   event generation rate requires more frequent upload, events may
 *   be dropped.  The value is used to initialize chip
 *   LoggingConfiguration. This is only relevant for BDX uploads.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_MINIMUM_UPLOAD_SECONDS
#define CHIP_CONFIG_EVENT_LOGGING_MINIMUM_UPLOAD_SECONDS 1
#endif

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
 * @def CHIP_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE
 *
 * @brief
 *   The default importance of events to be logged.  It is used
 *   to initialize the chip LoggingConfiguration.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE
#define CHIP_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE chip::Profiles::DataManagement::Production
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

#endif /* CHIPEVENTLOGGINGCONFIG_H */
