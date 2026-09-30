/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on Linux platforms.
 */

#pragma once

// ==================== Platform Adaptations ====================

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#else
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#endif

#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD CHIP_ENABLE_OPENTHREAD
#endif

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#endif

// Start GLib main event loop if BLE, Thread or WiFi is enabled. This is needed
// to handle D-Bus communication with BlueZ or wpa_supplicant.
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE || CHIP_DEVICE_CONFIG_ENABLE_WIFI ||                                                        \
    (CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT)
#define CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP 1
#else
#define CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP 0
#endif

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to Linux platforms.
// These can be overridden by the application as needed.

/**
 * CHIP_DEVICE_CONFIG_WIFIPAF_NAN_RECOVERY_TIMEOUT_MS
 *
 * Only relevant when CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF is enabled.
 *
 * On a device that shares one radio between Wi-Fi PAF and the station link, NAN does not
 * resume the instant wpa_supplicant reports the station link connected.  A PAF frame sent
 * in that window is accepted by the supplicant and then silently dropped, and PAFTP has no
 * retransmission to recover it.
 *
 * After the link comes up the PAF transport is therefore held until the NAN layer is seen
 * carrying traffic again, or until this timeout expires, whichever happens first.  It is an
 * upper bound on how long to wait for that evidence. Real world measurements show NAN
 * activity within 0.01-1.05s.
 *
 * The default is roughly twice the slowest recovery observed.  Tune it to suit the radio; a
 * value of 0 releases the transport after the link comes up.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFIPAF_NAN_RECOVERY_TIMEOUT_MS
#define CHIP_DEVICE_CONFIG_WIFIPAF_NAN_RECOVERY_TIMEOUT_MS 1500
#endif // CHIP_DEVICE_CONFIG_WIFIPAF_NAN_RECOVERY_TIMEOUT_MS

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 1
#endif // CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT (CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT)
