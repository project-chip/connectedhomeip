/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines default compile-time configuration constants
 *      for the CHIP Time Services profile.
 *
 */

#pragma once

/**
 *  @def CHIP_CONFIG_TIME
 *
 *  @brief
 *    1 (Default) If chip Time Services is needed/enabled.
 *    0 If chip Time Services is not needed. Content of implementation files
 *      would be compiled out.
 *
 */
#ifndef CHIP_CONFIG_TIME
#define CHIP_CONFIG_TIME 1
#endif // CHIP_CONFIG_TIME

/**
 *  @def CHIP_CONFIG_TIME_PROGRESS_LOGGING
 *
 *  @brief
 *    Disabled: 0, Enabled: 1. If enabled, the messages at PROGRESS
 *    level would be available in the log.
 *
 */
#ifndef CHIP_CONFIG_TIME_PROGRESS_LOGGING
#define CHIP_CONFIG_TIME_PROGRESS_LOGGING 1
#endif // CHIP_CONFIG_TIME_PROGRESS_LOGGING

/**
 *  @def CHIP_CONFIG_TIME_ENABLE_CLIENT
 *
 *  @brief
 *    1 (Default) Enable Client role in TimeSyncNode. Note that
 *      Coordinator also requires Client.
 *    0 If not needed, Client-specific part would be compiled out.
 *
 */
#ifndef CHIP_CONFIG_TIME_ENABLE_CLIENT
#define CHIP_CONFIG_TIME_ENABLE_CLIENT (CHIP_CONFIG_TIME)
#endif // CHIP_CONFIG_TIME_ENABLE_CLIENT

/**
 *  @def CHIP_CONFIG_TIME_ENABLE_SERVER
 *
 *  @brief
 *    1 (Default) Enable Server role in TimeSyncNode. Note that
 *      Coordinator also requires Server.
 *    0 If not needed, Server-specific part would be compiled out.
 *
 */
#ifndef CHIP_CONFIG_TIME_ENABLE_SERVER
#define CHIP_CONFIG_TIME_ENABLE_SERVER (CHIP_CONFIG_TIME)
#endif // CHIP_CONFIG_TIME_ENABLE_SERVER

/**
 *  @def CHIP_CONFIG_TIME_ENABLE_COORDINATOR
 *
 *  @brief
 *    defined as combination of CHIP_CONFIG_TIME_ENABLE_SERVER and CHIP_CONFIG_TIME_ENABLE_CLIENT.
 *    if explicitly defined to be 1, set both CHIP_CONFIG_TIME_ENABLE_SERVER and
 *    CHIP_CONFIG_TIME_ENABLE_CLIENT to 1.
 *
 */
#ifndef CHIP_CONFIG_TIME_ENABLE_COORDINATOR
#define CHIP_CONFIG_TIME_ENABLE_COORDINATOR ((CHIP_CONFIG_TIME_ENABLE_CLIENT) && (CHIP_CONFIG_TIME_ENABLE_SERVER))
#endif // CHIP_CONFIG_TIME_ENABLE_COORDINATOR

#if (CHIP_CONFIG_TIME_ENABLE_COORDINATOR && !((CHIP_CONFIG_TIME_ENABLE_CLIENT) && (CHIP_CONFIG_TIME_ENABLE_SERVER)))
#error Time Sync Coordinator requires both Server and Client to be enabled
#endif // CHIP_CONFIG_TIME_ENABLE_COORDINATOR

/**
 *  @def CHIP_CONFIG_TIME_NUM_UTC_OFFSET_RECORD
 *
 *  @brief
 *    Number of UTC offset records can be used for Time Zone calculation
 *
 */
#ifndef CHIP_CONFIG_TIME_NUM_UTC_OFFSET_RECORD
#define CHIP_CONFIG_TIME_NUM_UTC_OFFSET_RECORD 4
#endif // CHIP_CONFIG_TIME_NUM_UTC_OFFSET_RECORD

/**
 *  @def CHIP_CONFIG_TIME_SERVER_TIMER_UNRELIABLE_AFTER_BOOT_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Server/Coordinator roles. This is
 *    'timer_unreliable_after_boot' in design spec. Number of msec for
 *    the Server/Coordinator to consider its own system time as unreliable
 *    after program starts. Default is 30 seconds.
 *
 */
#ifndef CHIP_CONFIG_TIME_SERVER_TIMER_UNRELIABLE_AFTER_BOOT_MSEC
#define CHIP_CONFIG_TIME_SERVER_TIMER_UNRELIABLE_AFTER_BOOT_MSEC (30 * 1000)
#endif // CHIP_CONFIG_TIME_SERVER_TIMER_UNRELIABLE_AFTER_BOOT_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_SYNC_PERIOD_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the variable 'timer_sync_period' in design spec. Default number of msec for
 *    the Client/Coordinator to perform another unicast time sync operation,
 *    if not specified in the init code. The default is 30 minutes.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_SYNC_PERIOD_MSEC
#define CHIP_CONFIG_TIME_CLIENT_SYNC_PERIOD_MSEC (30 * 60 * 1000)
#endif // CHIP_CONFIG_TIME_CLIENT_SYNC_PERIOD_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_NOMINAL_DISCOVERY_PERIOD_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the variable 'timer_nominal_discovery_period' in design spec. Default
 *    number of msec for the Client/Coordinator to perform another multicast
 *    discover operation, when there is no communication error,
 *    if not specified in the init code. Default is 12 hours.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_NOMINAL_DISCOVERY_PERIOD_MSEC
#define CHIP_CONFIG_TIME_CLIENT_NOMINAL_DISCOVERY_PERIOD_MSEC (12 * 3600 * 1000)
#endif // CHIP_CONFIG_TIME_CLIENT_NOMINAL_DISCOVERY_PERIOD_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_MINIMUM_DISCOVERY_PERIOD_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the variable 'timer_minimum_discovery_period' in design spec. Default
 *    number of msec for the Client/Coordinator to perform another multicast
 *    discover operation, when there is some communication error,
 *    if not specified in the init call. It is not recommended to perform
 *    discover operation faster than the default value. Default is 30 minutes.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_MINIMUM_DISCOVERY_PERIOD_MSEC
#define CHIP_CONFIG_TIME_CLIENT_MINIMUM_DISCOVERY_PERIOD_MSEC (30 * 60 * 1000)
#endif // CHIP_CONFIG_TIME_CLIENT_MINIMUM_DISCOVERY_PERIOD_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_TIMER_MULTICAST_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the variable 'timer_wait_for_multicast_response' in design spec.
 *    Number of msec before the Client/Coordinator quits waiting for
 *    responses to the multicasted sync request. Default is 2 seconds.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_TIMER_MULTICAST_MSEC
#define CHIP_CONFIG_TIME_CLIENT_TIMER_MULTICAST_MSEC (2 * 1000)
#endif // CHIP_CONFIG_TIME_CLIENT_TIMER_MULTICAST_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_TIMER_UNICAST_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the variable 'timer_wait_for_unicast_response' in design spec.
 *    Number of msec before the Client/Coordinator quits waiting for
 *    response to the unicasted sync request. Default is 2 seconds.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_TIMER_UNICAST_MSEC
#define CHIP_CONFIG_TIME_CLIENT_TIMER_UNICAST_MSEC (2 * 1000)
#endif // CHIP_CONFIG_TIME_CLIENT_TIMER_UNICAST_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_TIMER_CLOUD_SERVICE_MSEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the variable 'timer_wait_for_service_response' in design spec.
 *    Number of msec before the Client/Coordinator quits waiting for
 *    response to the unicasted sync request going through TCP.
 *    Default is 5 seconds.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_TIMER_CLOUD_SERVICE_MSEC
#define CHIP_CONFIG_TIME_CLIENT_TIMER_CLOUD_SERVICE_MSEC (5 * 1000)
#endif // CHIP_CONFIG_TIME_CLIENT_TIMER_CLOUD_SERVICE_MSEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_MAX_RTT_USEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the 'max round trip time for usable response' in design spec.
 *    Number of usec for the longest round trip time a Client/Coordinator
 *    would consider a response as valid/useful. Default is 10 seconds.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_MAX_RTT_USEC
#define CHIP_CONFIG_TIME_CLIENT_MAX_RTT_USEC (10 * 1000000LL)
#endif // CHIP_CONFIG_TIME_CLIENT_MAX_RTT_USEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_MAX_NUM_CONTACTS
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the 'list of known Time Coordinators and Servers' in design spec.
 *    Number of unicast contacts a Client/Coordinator can remember.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_MAX_NUM_CONTACTS
#define CHIP_CONFIG_TIME_CLIENT_MAX_NUM_CONTACTS 4
#endif // CHIP_CONFIG_TIME_CLIENT_MAX_NUM_CONTACTS

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_FABRIC_LOCAL_DISCOVERY
 *
 *  @brief
 *    Disabled: 0, Enabled: 1. If enabled, the client would use
 *    UDP multicast to automatically discover available time servers
 *    and coordinators in the fabric. If disabled, contact information
 *    must be fed to the client through chip::Profiles::Time::TimeSyncNode::SyncWithNodes
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_FABRIC_LOCAL_DISCOVERY
#define CHIP_CONFIG_TIME_CLIENT_FABRIC_LOCAL_DISCOVERY 1
#endif // CHIP_CONFIG_TIME_CLIENT_FABRIC_LOCAL_DISCOVERY

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_CONNECTION_FOR_SERVICE
 *
 *  @brief
 *    Disabled: 0, Enabled: 1. If enabled, the client provides an API
 *    to go through a chip connection for time sync. If disabled,
 *    contact to cloud service must be fed to the client through chip::Profiles::Time::TimeSyncNode::SyncWithNodes
 *    and the transport would be UDP.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_CONNECTION_FOR_SERVICE
#define CHIP_CONFIG_TIME_CLIENT_CONNECTION_FOR_SERVICE 1
#endif // CHIP_CONFIG_TIME_CLIENT_CONNECTION_FOR_SERVICE

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_MIN_OFFSET_FROM_SERVER_USEC
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    the 'min_time_offset_from_service' in design spec. If the absolute value
 *    of calculated correction from a Time Sync Server is smaller than this
 *    number of usec, the correction would be ignored. Default is 2 seconds.
 *
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_MIN_OFFSET_FROM_SERVER_USEC
#define CHIP_CONFIG_TIME_CLIENT_MIN_OFFSET_FROM_SERVER_USEC (2 * 1000000LL)
#endif // CHIP_CONFIG_TIME_CLIENT_MIN_OFFSET_FROM_SERVER_USEC

/**
 *  @def CHIP_CONFIG_TIME_CLIENT_REASONABLE_TIME_SINCE_LAST_SYNC_MIN
 *
 *  @brief
 *    This only applies to Time Sync Client/Coordinator roles. This is
 *    'time since last sync for reliable response' in design spec.
 *    If a Server/Coordinator indicates that its system time has only been
 *    synced to some other reliable source longer than this number minutes
 *    ago, it would be counted as a stale/unreliable source.
 *    Default is 2 hours.
 */
#ifndef CHIP_CONFIG_TIME_CLIENT_REASONABLE_TIME_SINCE_LAST_SYNC_MIN
#define CHIP_CONFIG_TIME_CLIENT_REASONABLE_TIME_SINCE_LAST_SYNC_MIN 120
#endif // CHIP_CONFIG_TIME_CLIENT_REASONABLE_TIME_SINCE_LAST_SYNC_MIN

/**
 *  @def CHIP_CONFIG_TIME_COORDINATOR_THRESHOLD_TO_SEND_NOTIFICATION_USEC
 *
 *  @brief
 *    This only applies to Time Sync Coordinator role. This specifies
 *    threshold to trigger sending out of time change notification.
 *    If the system time is corrected on a Coordinator for more than this
 *    number of usec, it would multicast a Time Change Notification.
 *    Default is 2 seconds.
 */
#ifndef CHIP_CONFIG_TIME_COORDINATOR_THRESHOLD_TO_SEND_NOTIFICATION_USEC
#define CHIP_CONFIG_TIME_COORDINATOR_THRESHOLD_TO_SEND_NOTIFICATION_USEC (2 * 1000000LL)
#endif // CHIP_CONFIG_TIME_COORDINATOR_THRESHOLD_TO_SEND_NOTIFICATION_USEC
