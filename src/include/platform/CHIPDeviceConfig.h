/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Defines compile-time configuration values for the chip Device Layer.
 */

#pragma once

#if CHIP_HAVE_CONFIG_H
#include <platform/CHIPDeviceBuildConfig.h>
#endif

#include <lib/core/CHIPConfig.h>

/* Include a project-specific configuration file, if defined.
 *
 * An application or module that incorporates the chip Device Layer can define a project
 * configuration file to override standard chip configuration with application-specific
 * values.  The project config file is typically located outside the Openchip source
 * tree, alongside the source code for the application.
 */
#ifdef CHIP_DEVICE_PROJECT_CONFIG_INCLUDE
#include CHIP_DEVICE_PROJECT_CONFIG_INCLUDE
#endif

/* Include a platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard chip Device Layer
 * configuration that are specific to the platform or OS on which chip is running.
 * It is typically provided as apart of an adaptation layer that adapts Openchip
 * to the target environment.  This adaptation layer may be included in the Openchip
 * source tree itself or implemented externally.
 */
#ifdef CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE
#include CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE
#endif

// -------------------- General Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_CHIP_TASK_NAME
 *
 * The name of the chip task.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_NAME
#define CHIP_DEVICE_CONFIG_CHIP_TASK_NAME "CHIP"
#endif

/**
 * CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
 *
 * The size (in bytes) of the chip task stack.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 4096
#endif

/**
 * CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
 *
 * The priority of the chip task.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 2
#endif

/**
 * CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE
 *
 * The maximum number of events that can be held in the chip Platform event queue.
 */
#ifndef CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 100
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
 *
 * Enable support for background event processing.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
#define CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING 0
#endif

/**
 * CHIP_DEVICE_CONFIG_BG_TASK_NAME
 *
 * The name of the background task.
 */
#ifndef CHIP_DEVICE_CONFIG_BG_TASK_NAME
#define CHIP_DEVICE_CONFIG_BG_TASK_NAME "BG"
#endif

/**
 * CHIP_DEVICE_CONFIG_BG_TASK_STACK_SIZE
 *
 * The size (in bytes) of the background task stack.
 */
#ifndef CHIP_DEVICE_CONFIG_BG_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_BG_TASK_STACK_SIZE (6 * 1024)
#endif

/**
 * CHIP_DEVICE_CONFIG_BG_TASK_PRIORITY
 *
 * The priority of the background task.
 */
#ifndef CHIP_DEVICE_CONFIG_BG_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_BG_TASK_PRIORITY 1
#endif

/**
 * CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE
 *
 * The maximum number of events that can be held in the chip background event queue.
 */
#ifndef CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE
#define CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE 1
#endif

/**
 * CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL
 *
 * The default amount of time in milliseconds that the sleepy end device will use as an idle interval.
 * This interval is used by the device to periodically wake up and poll the data in the idle mode.
 */
#ifndef CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL
#define CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL System::Clock::Milliseconds32(5000)
#endif

/**
 * CHIP_DEVICE_CONFIG_ICD_SIT_SLOW_POLL_LIMIT
 *
 * The maximum value of time in milliseconds that the sleepy end device can use as an idle interval in the SIT mode.
 * The Matter spec does not allow this value to exceed 15s (spec 9.16.1.5).
 * For the SIT device, the usability of this value is arguable, as slow poll interval can be configured using
 * CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL. This value can be used for the LIT device, to limit the slow poll interval used while
 * temporarily working in the SIT mode.
 */
#ifndef CHIP_DEVICE_CONFIG_ICD_SIT_SLOW_POLL_LIMIT
#define CHIP_DEVICE_CONFIG_ICD_SIT_SLOW_POLL_LIMIT System::Clock::Milliseconds32(15000)
#endif

/**
 * CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL
 *
 * The default amount of time in milliseconds that the sleepy end device will use as an active interval.
 * This interval is used by the device to periodically wake up and poll the data in the active mode.
 */
#ifndef CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL
#define CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL System::Clock::Milliseconds32(200)
#endif

/**
 * CHIP_DEVICE_CONFIG_SED_ACTIVE_THRESHOLD
 *
 *  Minimum amount the node SHOULD stay awake after network activity.
 * Spec section 2.12.5
 */
#ifndef CHIP_DEVICE_CONFIG_SED_ACTIVE_THRESHOLD
#define CHIP_DEVICE_CONFIG_SED_ACTIVE_THRESHOLD System::Clock::Milliseconds32(4000)
#endif

// -------------------- Device Identification Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
 *
 * Human readable vendor name for the organization responsible for producing the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "TEST_VENDOR"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * The CHIP-assigned vendor id for the organization responsible for producing the device.
 *
 * Default is the Test VendorID of 0xFFF1.
 *
 * Un-overridden default must match the default test DAC
 * (see src/credentials/examples/DeviceAttestationCredsExample.cpp).
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME
 *
 * Human readable name of the device model.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "TEST_PRODUCT"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * The unique id assigned by the device vendor to identify the product or device type.  This
 * number is scoped to the device vendor id.
 *
 * Un-overridden default must either match one of the given development certs
 * or have a DeviceAttestationCredentialsProvider implemented.
 * (see src/credentials/examples/DeviceAttestationCredsExample.cpp)
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8001
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
 *
 * Human readable string identifying version of the product assigned by the device vendor.
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING "TEST_VERSION"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
 *
 * The default hardware version number assigned to the device or product by the device vendor.
 *
 * Hardware versions are specific to a particular device vendor and product id, and typically
 * correspond to a revision of the physical device, a change to its packaging, and/or a change
 * to its marketing presentation. This value is generally *not* incremented for device software
 * revisions.
 *
 * This is a default value which is used when a hardware version has not been stored in device
 * persistent storage (e.g. by a factory provisioning process).
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION 0
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.0"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
 *
 * A monothonic number identifying the software version running on the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1
#endif

/**
 * CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in Chip NV storage.
 */
#ifndef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"
#endif

/**
 * CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC
 *
 * The default conservative initial duration (in seconds) to set in the FailSafe for the commissioning
 * flow to complete successfully. This may vary depending on the speed or sleepiness of the Commissionee.
 */
#ifndef CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC
#define CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC 60
#endif // CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC

/**
 * CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC
 *
 * The default conservative value in seconds denoting the maximum total duration for which a fail safe
 * timer can be re-armed.
 */
#ifndef CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC
#define CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC 900
#endif // CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC

/**
 * CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
 *
 * Whether a device supports "concurrent connection commissioning mode" (1) or
 * or "non-concurrenct connection commissioning mode" (0).
 *
 * See section "5.5. Commissioning Flows" in spec for definition.
 *
 * The default value is to support concurrent connection.
 */
#ifndef CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
#define CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION 1
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

/**
 * The device shall check every WIFI_START_CHECK_TIME_USEC whether Wi-Fi management
 * has been fully initialized. If after WIFI_START_CHECK_ATTEMPTS Wi-Fi management
 * still hasn't been initialized, the device configuration is reset, and device
 * needs to be paired again.
 */
#ifndef WIFI_START_CHECK_TIME_USEC
#define WIFI_START_CHECK_TIME_USEC 100000 // 100ms
#endif
#ifndef WIFI_START_CHECK_ATTEMPTS
#define WIFI_START_CHECK_ATTEMPTS 5
#endif

/**
 * CHIP_DEVICE_CONFIG_USER_SELECTED_MODE_TIMEOUT_SEC
 *
 * The default amount of time (in whole seconds) that the device will remain in "user selected"
 * mode.  User selected mode is typically initiated by a button press, or other direct interaction
 * by a user.  While in user selected mode, the device will respond to Device Identify Requests
 * that have the UserSelectedMode flag set.
 */
#ifndef CHIP_DEVICE_CONFIG_USER_SELECTED_MODE_TIMEOUT_SEC
#define CHIP_DEVICE_CONFIG_USER_SELECTED_MODE_TIMEOUT_SEC 30
#endif // CHIP_DEVICE_CONFIG_USER_SELECTED_MODE_TIMEOUT_SEC

/**
 * CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
 *
 * Enables the use of a hard-coded default unique ID utilized for the rotating device ID calculation.
 */
#ifndef CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
#define CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID                                                                            \
    {                                                                                                                              \
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff                             \
    }
#endif

/**
 * CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH
 *
 * Unique ID length in bytes. The value should be 16-bytes or longer.
 */
#ifndef CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH
#define CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH 16
#endif

// -------------------- WiFi Station Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
 *
 * Enable support for a WiFi station interface.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
 *
 * Enable support for WiFi Per-Device Credentials
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC 0
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL
 *
 * The interval at which the chip platform will attempt to reconnect to the configured WiFi
 * network (in milliseconds).
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL
#define CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL 5000
#endif

/**
 * CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS
 *
 * The maximum number of networks to return as a result of a NetworkProvisioning:ScanNetworks request.
 */
#ifndef CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS
#define CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS 10
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT
 *
 * The amount of time (in milliseconds) after which the chip platform will timeout a WiFi scan
 * operation that hasn't completed.  A value of 0 will disable the timeout logic.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT
#define CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT 10000
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_CONNECTIVITY_TIMEOUT
 *
 * The amount of time (in milliseconds) to wait for Internet connectivity to be established on
 * the device's WiFi station interface during a Network Provisioning TestConnectivity operation.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_CONNECTIVITY_TIMEOUT
#define CHIP_DEVICE_CONFIG_WIFI_CONNECTIVITY_TIMEOUT 30000
#endif

/**
 * CHIP_DEVICE_CONFIG_LWIP_WIFI_STATION_IF_NAME
 *
 * Name of the WiFi station interface on LwIP-based platforms.
 */
#ifndef CHIP_DEVICE_CONFIG_LWIP_WIFI_STATION_IF_NAME
#define CHIP_DEVICE_CONFIG_LWIP_WIFI_STATION_IF_NAME "wl"
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME
 *
 * Name of the WiFi station interface
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME
#define CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME "wlan0"
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
 *
 * Enable support for commissioning using Wi-Fi Public Action Frame as the transport.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#define CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF 0
#endif

// -------------------- WiFi AP Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
 *
 * Enable support for a WiFi AP interface.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX
 *
 * A prefix string used in forming the WiFi soft-AP SSID.  The remainder of the SSID
 * consists of the final two bytes of the device's primary WiFi MAC address in hex.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX
#define CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX "MATTER-"
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL
 *
 * The WiFi channel number to be used by the soft-AP.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL
#define CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL 1
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_AP_MAX_STATIONS
 *
 * The maximum number of stations allowed to connect to the soft-AP.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_AP_MAX_STATIONS
#define CHIP_DEVICE_CONFIG_WIFI_AP_MAX_STATIONS 4
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_AP_BEACON_INTERVAL
 *
 * The beacon interval (in milliseconds) for the WiFi soft-AP.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_AP_BEACON_INTERVAL
#define CHIP_DEVICE_CONFIG_WIFI_AP_BEACON_INTERVAL 100
#endif

/**
 * CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT
 *
 * The amount of time (in milliseconds) after which the chip platform will deactivate the soft-AP
 * if it has been idle.
 */
#ifndef CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT
#define CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT 120000
#endif

/**
 * CHIP_DEVICE_CONFIG_LWIP_WIFI_AP_IF_NAME
 *
 * Name of the WiFi AP interface on LwIP-based platforms.
 */
#ifndef CHIP_DEVICE_CONFIG_LWIP_WIFI_AP_IF_NAME
#define CHIP_DEVICE_CONFIG_LWIP_WIFI_AP_IF_NAME "ap"
#endif

// -------------------- BLE/CHIPoBLE Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Enable support for chip-over-BLE (CHIPoBLE).
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#endif

/**
 * CHIP_DEVICE_CONFIG_SINGLE_CHIPOBLE_CONNECTION
 *
 * Limit support for chip-over-BLE (CHIPoBLE) to a single connection.
 * When set, CHIPoBLE advertisements will stop while a CHIPoBLE connection is active.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
#define CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION 1
#endif

/**
 * CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART
 *
 * Enable CHIPoBLE advertising start automatically after device power-up.
 *
 * CHIP's device may start advertising automatically only if its all primary device
 * functions are within a CHIP network. Device providing unrelated to CHIP functionalities
 * should not start advertising automatically after power-up.
 *
 * TODO: Default value should be changed to 0, after all platforms will implement enabling
 *       advertisement in their own way.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART
#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 1
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
 *
 * Enable opening pairing window automatically after device power-up.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 1
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX
 *
 * A prefix string used in forming the BLE device name.  The remainder of the name
 * typically contains the setup discriminator as a 4-digit decimal number.
 *
 * NOTE: The device layer limits the total length of a device name to 16 characters.
 * However, due to other data sent in CHIPoBLE advertise packets, the device name
 * may need to be shorter.
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX
#define CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX "MATTER-"
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
 *
 * The minimum interval (in units of 0.625ms) at which the device will send BLE advertisements while
 * in fast advertising mode. The minimum interval should be less and not equal to the
 * CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX.
 *
 * Defaults to 32 (20 ms).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
#define CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN 32
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
 *
 * The maximum interval (in units of 0.625ms) at which the device will send BLE advertisements while
 * in fast advertising mode. The maximum interval should be greater and not equal to the
 * CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN.
 *
 * Defaults to 96 (60 ms).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
#define CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX 96
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN
 *
 * The minimum interval (in units of 0.625ms) at which the device will send BLE advertisements while
 * in slow advertising mode. The minimum interval should be greater and not equal to the
 * CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX.
 *
 * Defaults to 240 (150 ms).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN
#define CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN 240
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX
 *
 * The maximum interval (in units of 0.625ms) at which the device will send BLE advertisements while
 * in slow advertising mode. The maximum interval should be greater and not equal to the
 * CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN.
 *
 * Defaults to 1920 (1200 ms).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX
#define CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX 1920
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME
 *
 * The amount of time in miliseconds after which BLE advertisement should be switched from the fast
 * advertising to the slow advertising, counting from the moment of advertisement commencement.
 *
 * Defaults to 30000 (30 seconds).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME
#define CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME 30000
#endif

/**
 * CHIP_DEVICE_CONFIG_EXT_ADVERTISING
 *
 * Optional configuration to enable Extended Announcement Duration up to 48h.
 * Should be used together with extending CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS past 15 minutes.
 * Disabled by default.
 */

#ifndef CHIP_DEVICE_CONFIG_EXT_ADVERTISING
#define CHIP_DEVICE_CONFIG_EXT_ADVERTISING 0
#endif

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING

/**
 * CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS
 *
 * The amount of time in miliseconds after which BLE advertisement should be switched from the slow
 * advertising to the extended advertising, counting from the moment of advertisement commencement.
 *
 *  Defaults to 900000 ms.
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS
#define CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS (15 * 60 * 1000)
#endif

/**
 * CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN
 *
 * The minimum interval (in units of 0.625ms) at which the device will send BLE advertisements while
 * in the extended advertising mode. The minimum interval shall not be smaller than the default value.
 *
 * Defaults to 2056 (1285 ms).
 */
#define CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN 2056

/**
 * CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX
 *
 * The maximum interval (in units of 0.625ms) at which the device will send BLE advertisements while
 * in the extended advertising mode. The maximum interval should be greater.
 *
 * Defaults to 2056 (1285 ms).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX
#define CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX 2056
#endif

static_assert(CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN <= CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX,
              "Max Extended Advertising Interval cannot be larger to the Min Extended Advertising Interval");

#endif

// -------------------- Service Provisioning Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_ENDPOINT_ID
 *
 * Specifies the service endpoint id of the chip Service Provisioning service.  When a device
 * undergoes service provisioning, this is the endpoint to which it will send its Pair Device
 * to Account request.
 */
#ifndef CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_ENDPOINT_ID
#define CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_ENDPOINT_ID 0x18B4300200000010ULL
#endif

/**
 * CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_CONNECTIVITY_TIMEOUT
 *
 * The maximum amount of time (in milliseconds) to wait for service connectivity during the device
 * service provisioning step.  More specifically, this is the maximum amount of time the device will
 * wait for connectivity to be established with the service at the point where the device waiting
 * to send a Pair Device to Account request to the Service Provisioning service.
 */
#ifndef CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_CONNECTIVITY_TIMEOUT
#define CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_CONNECTIVITY_TIMEOUT 10000
#endif

/**
 * CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_REQUEST_TIMEOUT
 *
 * Specifies the maximum amount of time (in milliseconds) to wait for a response from the Service
 * Provisioning service.
 */
#ifndef CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_REQUEST_TIMEOUT
#define CHIP_DEVICE_CONFIG_SERVICE_PROVISIONING_REQUEST_TIMEOUT 10000
#endif

// -------------------- Device DNS-SD Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS
 *
 * Time in seconds that a factory new device will advertise commissionable node discovery.
 */
#ifndef CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
/**
 * By default, the extended announcement, when enabled, starts its extended advertising 15 mins
 * after the standard slow advertisement. Time at which the default discovery time would close the
 * commissioning window and stop the BLE.
 * Therefore, when CHIP_DEVICE_CONFIG_EXT_ADVERTISING is enabled bump the default Discovery timeout
 * to the maximum allowed by the spec. 48h.
 */
#define CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS (60 * 60 * 48)
#else
#define CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS (15 * 60)
#endif // CHIP_DEVICE_CONFIG_EXT_ADVERTISING
#endif // CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS

/**
 * CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES
 *
 * Maximum number of CHIP Commissioners or Commissionable Nodes that can be discovered
 */
#ifndef CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES
#define CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES 10
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
 *
 * Enable or disable whether this device advertises as a commissioner.
 *
 * Depends upon CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE set to 1
 *
 * For Video Players, this value will be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0
#endif

/**
 * CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS
 *
 * The number of target apps that a client can include in a UDC message.
 *
 * Depends upon CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY set to 1
 *
 * For Video Players, this value should be set to 10
 */
#ifndef CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS
#define CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS 3
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
 *
 * Enable including commissioner code (CHIPDeviceController.cpp) in the commissionee (Server.cpp) code.
 *
 * For Video Players, this value will be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE 0
#endif

/**
 * CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
 *
 * See issue 23625.
 *
 * Needs to be 1 when the following is 1:
 *  CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
 */
#ifndef CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
#define CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
 *
 * Enable or disable whether this device will attempt to
 * discover commissioners and send Uder Directed Commissioning
 * messages to them.
 *
 * For Video Player Clients, this value will be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
 *
 * Enable or disable whether this device advertises when not in commissioning mode.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 0
#endif

/**
 * CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS
 *
 * Default time in seconds that a device will advertise commissionable node discovery
 * after commissioning mode ends. This value can be overridden by the user.
 *
 * Only valid when CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY==1
 */
#define CHIP_DEVICE_CONFIG_DISCOVERY_DISABLED 0
#define CHIP_DEVICE_CONFIG_DISCOVERY_NO_TIMEOUT -1
#ifndef CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS
#define CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS (15 * 60)
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE
 *
 * Enable or disable including device type in commissionable node discovery.
 *
 * For Video Players, this value will often be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 0
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_TYPE
 *
 * Type of device using the CHIP Device Type Identifier.
 *
 * Examples:
 * 0xFFFF = 65535 = Invalid Device Type
 * 0x0051 = 81 = Smart Plug
 * 0x0022 = 34 = Speaker
 * 0x0023 = 35 = Video Player
 *
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 65535 // 65535 = Invalid Device Type
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME
 *
 * Enable or disable including device name in commissionable node discovery.
 *
 * For Video Players, this value will often be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 0
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_NAME
 *
 * Name of device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Test Kitchen"
#endif

/**
 * CHIP_DEVICE_CONFIG_PAIRING_INITIAL_HINT
 *
 * Pairing Hint, bitmap value of methods to put device into pairing mode
 * when it has not yet been commissioned.
 *
 * Bits:
 * 0 - Power Cycle
 * 5 - See Device Manual
 */
#ifndef CHIP_DEVICE_CONFIG_PAIRING_INITIAL_HINT
#define CHIP_DEVICE_CONFIG_PAIRING_INITIAL_HINT                                                                                    \
    (1 << CHIP_COMMISSIONING_HINT_INDEX_POWER_CYCLE | 1 << CHIP_COMMISSIONING_HINT_INDEX_SEE_MANUAL)
#endif

/**
 * CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION
 *
 * Pairing Instruction, when device has not yet been commissioned
 *
 * Meaning is depedent upon pairing hint value.
 */
#ifndef CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION
#define CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION ""
#endif

/**
 * CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_HINT
 *
 * Pairing Hint, bitmap value of methods to put device into pairing mode
 * when it has already been commissioned.
 *
 * Bits:
 * 2 - Visit Administrator UX (always true for secondary)
 * 5 - See Device Manual
 */
#ifndef CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_HINT
#define CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_HINT                                                                                  \
    (1 << CHIP_COMMISSIONING_HINT_INDEX_SEE_ADMINISTRATOR_UX | 1 << CHIP_COMMISSIONING_HINT_INDEX_SEE_MANUAL)
#endif

/**
 * CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION
 *
 * Pairing Instruction, when device has not yet been commissioned
 *
 * Meaning is depedent upon pairing hint value.
 */
#ifndef CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION
#define CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION ""
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_PASSCODE
 *
 * Enable or disable commissioner passcode feature.
 * With this feature enabled, the commissioner can generate a commissioning passcode
 * and display it to the user so that the user can enter it into a commissionable
 * node, such as a phone app, during user directed commissioning.
 *
 * For Video Players, this value will often be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_PASSCODE
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_PASSCODE 0
#endif

// -------------------- Thread Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_THREAD
 *
 * Enable support for Thread in the chip Device Layer.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_FTD
 *
 * Enable Full Thread Device features
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_FTD
#define CHIP_DEVICE_CONFIG_THREAD_FTD 1
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_SSED
 *
 * Enable support for Thread Synchronized Sleepy End Device behavior.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_SSED
#define CHIP_DEVICE_CONFIG_THREAD_SSED 0
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_BORDER_ROUTER
 *
 * Enable Thread Border Router service.
 * Users should ensure OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE is set accordingly within their thread stack
 *
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_BORDER_ROUTER
#define CHIP_DEVICE_CONFIG_THREAD_BORDER_ROUTER 0
#endif

/**
 * CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK
 *
 * Indicate if the matter device thread stack is implemented using the ot-br-posix dbus API
 * Rather than the standard openthread stack api
 *
 */
#ifndef CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK
#define CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK 0
#endif
/**
 * CHIP_DEVICE_CONFIG_THREAD_TASK_NAME
 *
 * The name of the Thread task.
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_NAME
#define CHIP_DEVICE_CONFIG_THREAD_TASK_NAME "THREAD"
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
 *
 * The size (in bytes) of the OpenThread task stack.
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 8192
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY
 *
 * The priority of the OpenThread task.
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY 3
#endif

/**
 * CHIP_DEVICE_CONFIG_LWIP_THREAD_IF_NAME
 *
 * Name of the Thread interface on LwIP-based platforms.
 */
#ifndef CHIP_DEVICE_CONFIG_LWIP_THREAD_IF_NAME
#define CHIP_DEVICE_CONFIG_LWIP_THREAD_IF_NAME "th"
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_IF_MTU
 *
 * Default MTU for Thread interface
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_IF_MTU
#define CHIP_DEVICE_CONFIG_THREAD_IF_MTU 1280
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_THREAD_NETWORK_NAME_PREFIX
 *
 * A prefix string to be used when forming a default Thread network name.
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_THREAD_NETWORK_NAME_PREFIX
#define CHIP_DEVICE_CONFIG_DEFAULT_THREAD_NETWORK_NAME_PREFIX "CHIP-PAN-"
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_CONNECTIVITY_TIMEOUT
 *
 * The amount of time (in milliseconds) to wait for connectivity with a Thread mesh
 * to be established on during a Network Provisioning TestConnectivity operation.
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_CONNECTIVITY_TIMEOUT
#define CHIP_DEVICE_CONFIG_THREAD_CONNECTIVITY_TIMEOUT 30000
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
 *
 * Enable Thread CLI interface at initialisation.
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#define CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
 *
 * Enable support to DNS-SD SRP client usage for service advertising and discovery in CHIP.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 0
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES
 *
 * Amount of services available for advertising using SRP.
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#define CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES (CHIP_CONFIG_MAX_FABRICS + 3)
#elif CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY || CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#define CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES (CHIP_CONFIG_MAX_FABRICS + 2)
#else
#define CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES (CHIP_CONFIG_MAX_FABRICS + 1)
#endif
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY
 *
 * Enable support to Commissionable Discovery for Thread devices.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
 *
 * Enable support to DNS client usage for resolving and browsing services in CHIP.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_THREAD_AUTOSTART
 *
 * Enable starting provisioned Thread network automatically after device power-up.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_AUTOSTART
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_AUTOSTART 1
#endif

// -------------------- Network Telemetry Configuration --------------------

/**
 * @def CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY
 *
 * @brief
 *   Enable automatically uploading Wi-Fi telemetry via trait on an interval.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY (0)
#endif

/**
 * @def CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY
 *
 * @brief
 *   Enable automatically uploading minimal Thread telemetry and topology via trait on an interval.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY (0)
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY && !CHIP_DEVICE_CONFIG_ENABLE_THREAD
#error "If CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY set, then CHIP_DEVICE_CONFIG_ENABLE_THREAD must also be set."
#endif

/**
 * @def CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL
 *
 * @brief
 *   Enable automatically uploading all Thread telemetry and topology via trait on an interval.
 *   This is suitable for products that have router capability.
 *
 * @note
 *   If set, CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY must also be set.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL (0)
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL && !CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY
#error "If CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL set, then CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY must also be set."
#endif

// Enable Network Telemetry feature if it is enabled for at lease one network.
#define CHIP_DEVICE_CONFIG_ENABLE_NETWORK_TELEMETRY                                                                                \
    (CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY || CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY)

/**
 *  @def CHIP_DEVICE_CONFIG_DEFAULT_TELEMETRY_INTERVAL_MS
 *
 *  @brief
 *    This sets the default interval at which network telemetry events
 *    will be logged to chip buffers. This can be overwritten at runtime
 *    with a trait.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_TELEMETRY_INTERVAL_MS
#define CHIP_DEVICE_CONFIG_DEFAULT_TELEMETRY_INTERVAL_MS 90000
#endif

// -------------------- Test Setup (PASE) Configuration --------------------

/**
 * @def CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS
 *
 * @brief
 *   Enable use of test setup parameters for testing purposes only.
 *
 *  @note
 *    WARNING: This option makes it possible to circumvent basic chip security functionality.
 *    Because of this it SHOULD NEVER BE ENABLED IN PRODUCTION BUILDS.
 */
// TODO: When the SDK code is production ready this should be set to 0 and each platform
// will need to enable it indivually when needed.
#ifndef CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 1
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
 *
 * @brief
 *   Test Spake2p passcode to use if actual passcode value is not provisioned in the device memory.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#endif

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
 *
 * @brief
 *   Test setup discriminator to use if actual discriminator value is not provisioned in the device memory.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#endif

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
 *
 * @brief
 *   Test Spake2p iteration count to use if actual iteration count value is not provisioned in the device memory.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT 1000
#endif

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT
 *
 * @brief
 *   Test Spake2p Salt to use if actual salt value is not provisioned in the device memory.
 * @note
 *   The value is base-64 encoded string.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT "U1BBS0UyUCBLZXkgU2FsdA=="
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT_DEFAULT
#endif

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER
 *
 * @brief
 *   Test Spake2p Verifier to use if actual verifier value is not provisioned in the device memory.
 * @note
 *   The value is base-64 encoded string.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER

#if CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE != 20202021
#error "Non-default Spake2+ passcode configured but verifier left unchanged"
#endif

#if CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT != 1000
#error "Non-default Spake2+ iteration count configured but verifier left unchanged"
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT_DEFAULT
#error "Non-default Spake2+ salt configured but verifier left unchanged"
#endif

// Generated with: spake2p gen-verifier -o - -i 1000 -s "U1BBS0UyUCBLZXkgU2FsdA==" -p 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER                                                                               \
    "uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ65CJzbeUB49s31EH+NEkg0JVI5MGCQGMMT/SRPFNRODm3wH/MBiehuFc6FJ/"  \
    "NH6Rmzw=="
#endif

#else

#undef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#undef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#undef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
#undef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT
#undef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER

#endif

// -------------------- Event Logging Configuration --------------------

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the buffer reserved for storing CRITICAL events and no
 *   other events.  CRITICAL events will never be evicted until this buffer is
 *   full, so its size and the sizes of events determine how many of the last N
 *   CRITICAL events are guaranteed to be available.
 *
 *   Note: this number must be nonzero.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE (1024)
#endif

#if (CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE <= 0)
#error "The Prod critical event buffer must exist (CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE > 0)"
#endif

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the buffer reserved for storing events at INFO
 *   priority and higher.  INFO-priority events will not be evicted until this
 *   buffer is full (with INFO and CRITICAL events in it) and the oldest event
 *   in the buffer is an INFO-priority event (which cannot be evicted into the
 *   CRITICAL event buffer).
 *
 *   Note: set to 0 to treat INFO events as effectively equivalent to DEBUG events.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE (512)
#endif

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the buffer used for storing newly generated events,
 *   and the only buffer in which DEBUG-priority events are allowed.
 *   DEBUG-priority events will start getting evicted when this buffer is full
 *   (with DEBUG, INFO, and CRITICAL events in it) and the oldest event in the
 *   buffer is a DEBUG-priority event, which cannot be evicted into the INFO
 *   event buffer.
 *
 *   Note: set to 0 to disable storing DEBUG events.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)
#endif

/**
 *  @def CHIP_DEVICE_CONFIG_EVENT_ID_COUNTER_EPOCH
 *
 *  @brief
 *    The event id counter persisted storage epoch.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_ID_COUNTER_EPOCH
#define CHIP_DEVICE_CONFIG_EVENT_ID_COUNTER_EPOCH (0x10000)
#endif

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
 *
 * @brief
 *   By default, don't record UTC timestamps.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 0
#endif

// -------------------- Software Update Manager Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
 *
 * Enable or disable the chip Software Update Manager.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#define CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER 1
#endif

/**
 * CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN
 *
 * Specifies the size in bytes of the buffer that stores the
 * URI
 *
 */
#ifndef CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN
#define CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN 128
#endif

/**
 * CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_ENDPOINT_ID
 *
 * Specifies the service endpoint id of the chip Software Update service.  When a device
 * attempts software update, this is the endpoint to which it will send its Software Update
 * Query request.
 */
#ifndef CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_ENDPOINT_ID
#define CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_ENDPOINT_ID 0x18B4300200000002ULL
#endif

/**
 * CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_ENDPOINT_ID
 *
 * Specifies the service endpoint id of the chip File Download service.  When a device
 * attempts file download over BDX, this is the endpoint to which it will send its BDX request.
 */
#ifndef CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_ENDPOINT_ID
#define CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_ENDPOINT_ID 0x18B4300200000013ULL
#endif

/**
 * CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_RESPONSE_TIMEOUT
 *
 * Specifies the maximum amount of time (in milliseconds) to wait for a response from a
 * chip Software Update service.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_RESPONSE_TIMEOUT
#define CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_RESPONSE_TIMEOUT 10000
#endif

/**
 * CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_RESPONSE_TIMEOUT
 *
 * Specifies the maximum amount of time (in milliseconds) to wait for a response from a
 * chip File Download service.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_RESPONSE_TIMEOUT
#define CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_RESPONSE_TIMEOUT 10000
#endif

/**
 * CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_RETRIES
 *
 * Specifies the maximum number of times a failed software is retried.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_RETRIES
#define CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_RETRIES 5
#endif

/**
 * CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_WAIT_TIME_INTERVAL_MS
 *
 * If scheduled software update check is disabled & default retry policy is used,
 * specify the max wait time interval to be used (in milliseconds)
 */
#ifndef CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_WAIT_TIME_INTERVAL_MS
#define CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_WAIT_TIME_INTERVAL_MS 1 * 60 * 60 * 1000 // 1 hour
#endif

/**
 *  @def CHIP_DEVICE_CONFIG_SWU_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP
 *
 *  @brief
 *    If default software update retry policy is used,
 *    specify the minimum wait
 *    time as a percentage of the max wait interval for that step.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_SWU_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP
#define CHIP_DEVICE_CONFIG_SWU_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP 50
#endif

/**
 *  @def CHIP_DEVICE_CONFIG_SWU_WAIT_TIME_MULTIPLIER_MS
 *
 *  @brief
 *    If default software update retry policy is used,
 *    specify the multiplier that multiplies the result of a Fibonacci computation
 *    based on a specific index to provide a max wait time for
 *    a step.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_SWU_WAIT_TIME_MULTIPLIER_MS
#define CHIP_DEVICE_CONFIG_SWU_WAIT_TIME_MULTIPLIER_MS 1 * 60 * 1000 // 1 minute
#endif

/**
 * CHIP_DEVICE_CONFIG_SWU_BDX_BLOCK_SIZE
 *
 * Specifies the block size to be used during software download over BDX.
 */
#define CHIP_DEVICE_CONFIG_SWU_BDX_BLOCK_SIZE 1024

/**
 * CHIP_DEVICE_CONFIG_FIRWMARE_BUILD_DATE
 *
 * Specifies the date of the build. Useful for deterministic builds.
 */
#ifndef CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE
#define CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE __DATE__
#endif

/**
 * CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME
 *
 * Specifies the time of the build. Useful for deterministic builds.
 */
#ifndef CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME
#define CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME __TIME__
#endif

// -------------------- App Platform Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
 *
 * Does this device support an app platform 1=Yes, 0=No
 */
#ifndef CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#define CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED 0
#endif

/**
 * CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
 *
 * When app platform is enabled, max number of endpoints
 */
#ifndef CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 0
#endif

/**
 * CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS
 *
 * Time threshold for events dispatching
 * Set 0 to disable event dispatching time measurement
 */
#ifndef CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS
#define CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS 100
#endif

// -------------------- Miscellaneous --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
 *
 * If 1, enable support for automatic CASE establishment retries.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
#define CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES 1
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

/**
 * CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS
 *
 * The initial retry delay, in seconds, for our automatic CASE retries.
 */
#ifndef CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS
#define CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS 1
#endif

/**
 * CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF
 *
 * The maximum number of times we back off, by a factor of 2 each time, from our
 * initial CASE retry interval before we plateau.
 */
#ifndef CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF
#define CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF 5
#endif

#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

/**
 * CHIP_DEVICE_LAYER_NONE aims to turn off the device layer, for platforms that
 * implement that in some alternate way.
 */
#ifndef CHIP_DEVICE_LAYER_NONE
#define CHIP_DEVICE_LAYER_NONE 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_NFC enables NFC communication for commissioning.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_NFC
#define CHIP_DEVICE_CONFIG_ENABLE_NFC 0
#endif

/**
 * CHIP_DEVICE_ENABLE_PORT_PARAMS enables command-line parameters to set the
 * port to use for POSIX example applications.
 */
#ifndef CHIP_DEVICE_ENABLE_PORT_PARAMS
#define CHIP_DEVICE_ENABLE_PORT_PARAMS 0
#endif // CHIP_DEVICE_ENABLE_PORT_PARAMS
