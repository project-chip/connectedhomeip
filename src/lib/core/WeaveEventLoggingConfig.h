/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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

#ifndef WEAVEEVENTLOGGINGCONFIG_H
#define WEAVEEVENTLOGGINGCONFIG_H

/**
 * @def WEAVE_CONFIG_EVENT_SIZE_RESERVE
 *
 * @brief
 *   Initial size to reserve for event. The logging subsystem ensures
 *   this space is available in the buffer before attempting to store
 *   the event.
 */
#ifndef WEAVE_CONFIG_EVENT_SIZE_RESERVE
#define WEAVE_CONFIG_EVENT_SIZE_RESERVE 196
#endif /* WEAVE_CONFIG_EVENT_SIZE_RESERVE */


/**
 * @def WEAVE_CONFIG_EVENT_SIZE_INCREMENT
 *
 * @brief
 *   When the initial reserved size proves insufficient, clean out the
 *   buffer in following increments.
 */
#ifndef WEAVE_CONFIG_EVENT_SIZE_INCREMENT
#define WEAVE_CONFIG_EVENT_SIZE_INCREMENT 8
#endif /* WEAVE_CONFIG_EVENT_SIZE_INCREMENT */

/**
 *  @def WEAVE_CONFIG_EVENT_LOGGING_BDX_OFFLOAD
 *
 *  @brief
 *    Control whether the logs are offloaded using the BDX profile
 *
 *   Select the ability to offload logs to the BDX endpoint.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_BDX_OFFLOAD
#define WEAVE_CONFIG_EVENT_LOGGING_BDX_OFFLOAD 0
#endif /* WEAVE_CONFIG_EVENT_LOGGING_BDX_OFFLOAD */

/**
 *  @def WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD
 *
 *  @brief
 *    Control whether the logs are offloaded using the WDM profile
 *
 *   Select the ability to offload logs to any interested subscribers using WDM.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD
#define WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD 0
#endif /* WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD */

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_XFER_THRESHOLD
 *
 * @brief
 *   The log size threshold in bytes beyond which the system will
 *   trigger the automatic upload
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_XFER_THRESHOLD
#define WEAVE_CONFIG_EVENT_LOGGING_XFER_THRESHOLD 512
#endif /* WEAVE_CONFIG_EVENT_LOGGING_XFER_THRESHOLD */

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_EVENT_THRESHOLD
 *
 * @brief
 *  The number of locally stored events beyond which the system will
 *  trigger the automatic upload. Uses XFER_THRESHOLD and EVENT_SIZE_RESERVE
 *  to approximate the size of an average WDM notification
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_EVENT_THRESHOLD
#define WEAVE_CONFIG_EVENT_LOGGING_EVENT_THRESHOLD (WEAVE_CONFIG_EVENT_LOGGING_XFER_THRESHOLD/WEAVE_CONFIG_EVENT_SIZE_RESERVE)
#endif

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD
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
#ifndef WEAVE_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD
#define WEAVE_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD 512
#endif /* WEAVE_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD */

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_MINIMUM_UPLOAD_SECONDS
 *
 * @brief
 *   Default minimum time between log uploads (in seconds). If
 *   event generation rate requires more frequent upload, events may
 *   be dropped.  The value is used to initialize Weave
 *   LoggingConfiguration. This is only relevant for BDX uploads.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_MINIMUM_UPLOAD_SECONDS
#define WEAVE_CONFIG_EVENT_LOGGING_MINIMUM_UPLOAD_SECONDS 1
#endif

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_MAXIMUM_UPLOAD_SECONDS
 *
 * @brief
 *   The maximum time (in seconds) between log uploads.  The
 *   system is guaranteed to attempt to offload the log buffer at
 *   least this often.  The value is used to initialize Weave
 *   LoggingConfiguration. This is only relevant for BDX uploads.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_MAXIMUM_UPLOAD_SECONDS
#define WEAVE_CONFIG_EVENT_LOGGING_MAXIMUM_UPLOAD_SECONDS 86400
#endif

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE
 *
 * @brief
 *   The default importance of events to be logged.  It is used
 *   to initialize the Weave LoggingConfiguration.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE
#define WEAVE_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE nl::Weave::Profiles::DataManagement::Production
#endif

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
 *
 * @brief
 *   By default, don't record UTC timestamps.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
#define WEAVE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 0
#endif

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT
 *
 * @brief
 *   Enable or disable support for externally managed event storage
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT
#define WEAVE_CONFIG_EVENT_LOGGING_EXTERNAL_EVENT_SUPPORT 0
#endif

#endif /* WEAVEEVENTLOGGINGCONFIG_H */
