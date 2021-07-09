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

#include <core/CHIPConfig.h>

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
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 1
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
 * CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
 *
 * Enable the device factory provisioning feature.
 *
 * The factory provisioning feature allows factory or developer-supplied provisioning information
 * to be injected into a device at boot time and automatically stored in persistent storage.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
// We don't have platform/internal/FactoryProvisioning.ipp for now, so set it to 0 by default.
#define CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING 0
#endif

/**
 * CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH
 *
 * Compute and log a hash of the device's provisioning data on boot.
 *
 * The generated hash value confirms to the form described in the CHIP: Factory
 * Provisioning Specification.
 */
#ifndef CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH
#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 1
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
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 9050
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
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 65279
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION_STRING
 *
 * Human readable string identifying version of the product assigned by the device vendor.
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION_STRING
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION_STRING "TEST_VERSION"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION
 *
 * The default product revision number assigned to the device or product by the device vendor.
 *
 * Product revisions are specific to a particular device vendor and product id, and typically
 * correspond to a revision of the physical device, a change to its packaging, and/or a change
 * to its marketing presentation. This value is generally *not* incremented for device software
 * revisions.
 *
 * This is a default value which is used when a product revision has not been stored in device
 * persistent storage (e.g. by a factory provisioning process).
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION 1
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING
 *
 * A string identifying the firmware revision running on the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING "prerelease"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
 *
 * A monothonic number identifying the firmware revision running on the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
#define CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION 1
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

// -------------------- WiFi AP Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
 *
 * Enable support for a WiFi AP interface.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 1
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
 * CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
 *
 * Automatically disable CHIPoBLE advertising when the device transitions to a fully
 * provisioned state.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
#define CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED 0
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
 * consists of the final two bytes of the device's chip node id in hex.
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
 * CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT
 *
 * The amount of time in miliseconds after which BLE advertisement should be disabled, counting
 * from the moment of advertisement commencement.
 *
 * Defaults to 9000000 (15 minutes).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT
#define CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT (15 * 60 * 1000)
#endif

// -------------------- Time Sync Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote chip Time service
 * using the chip Time Sync protocol.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0
#endif

/**
 * CHIP_DEVICE_CONFIG_CHIP_TIME_SERVICE_ENDPOINT_ID
 *
 * Specifies the service endpoint id of the chip Time Sync service to be used to synchronize time.
 *
 * This value is only meaningful if CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC has
 * been enabled.
 */
#ifndef CHIP_DEVICE_CONFIG_CHIP_TIME_SERVICE_ENDPOINT_ID
#define CHIP_DEVICE_CONFIG_CHIP_TIME_SERVICE_ENDPOINT_ID 0x18B4300200000005ULL
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_TIME_SYNC_INTERVAL
 *
 * Specifies the minimum interval (in seconds) at which the device should synchronize its real time
 * clock with the configured chip Time Sync server.
 *
 * This value is only meaningful if CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC has
 * been enabled.
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_TIME_SYNC_INTERVAL
#define CHIP_DEVICE_CONFIG_DEFAULT_TIME_SYNC_INTERVAL 180
#endif

/**
 * CHIP_DEVICE_CONFIG_TIME_SYNC_TIMEOUT
 *
 * Specifies the maximum amount of time (in milliseconds) to wait for a response from a
 * chip Time Sync server.
 *
 * This value is only meaningful if CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC has
 * been enabled.
 */
#ifndef CHIP_DEVICE_CONFIG_TIME_SYNC_TIMEOUT
#define CHIP_DEVICE_CONFIG_TIME_SYNC_TIMEOUT 10000
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

// -------------------- Just-In-Time Provisioning Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
 *
 * Enable just-in-time provisioning functionality in the chip Device Layer.
 *
 * When enabled, device creates and uses its ephemeral operational credentials:
 *   - operational device id
 *   - operational device self-signed certificate
 *   - operational device private key
 * When enabled, device also implements certificate provisioning protocol and uses it to obtain
 * service assigned certificate from the Certification Authority Service.
 *
 * Then, device uses these credentials to authenticate and communicate to other chip nodes.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
#define CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING 0
#endif

// -------------------- Service Discovery Configuration -----------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_MDNS
 *
 * Enable support to use MDNS for service advertising and discovery in CHIP.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_MDNS
#define CHIP_DEVICE_CONFIG_ENABLE_MDNS 0
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_UNPROVISIONED_MDNS
 *
 * Enable MDNS commissionable node advertising when not yet provisioned.
 *
 * This should be 1 for WiFi SoftAP devices, ethernet devices, and (probably) bridge devices
 *
 * This should be 0 for Thread/BLE devices and WiFi/BLE devices
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_UNPROVISIONED_MDNS
#define CHIP_DEVICE_CONFIG_ENABLE_UNPROVISIONED_MDNS 0
#endif

/**
 * CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES
 *
 * Maximum number of CHIP Commissioners or Commissionable Nodes that can be discovered
 */
#ifndef CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES
#define CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES 10
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
#define CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES 3
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
#define CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY 2
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

// -------------------- Trait Manager Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
 *
 * Enable or disable the chip Trait Manager.
 *
 * NOTE: The Trait Manager is an experimental feature of the chip Device Layer.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
#define CHIP_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER 0
#endif

// -------------------- Test Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
 *
 * Enables the use of a hard-coded default chip device id and credentials if no device id
 * is found in chip NV storage.  The value specifies which of 10 identities, numbered 1 through 10,
 * is to be used.  A value of 0 disables use of a default identity.
 *
 * This option is for testing only and should be disabled in production releases.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY 0
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

// -------------------- Event Logging Configuration --------------------

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the individual critical event logging buffer.
 *   Note: the critical event buffer must exist.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE (1024)
#endif

#if (CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE <= 0)
#error "The Prod critical event buffer must exist (CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE > 0)"
#endif

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_PROD_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the individual production event logging buffer.
 *   Note: the production event buffer must exist.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_PROD_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_PROD_BUFFER_SIZE (512)
#endif

#if (CHIP_DEVICE_CONFIG_EVENT_LOGGING_PROD_BUFFER_SIZE <= 0)
#error "The Prod event buffer must exist (CHIP_DEVICE_CONFIG_EVENT_LOGGING_PROD_BUFFER_SIZE > 0)"
#endif

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the individual info event logging buffer.
 *   Note: set to 0 to disable info event buffer and all support
 *   for the info level events.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE (512)
#endif

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the individual debug event logging buffer.
 *   Note: set to 0 to disable debug event buffer and all support
 *   for the debug level events.
 */
#ifndef CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (256)
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
 *  @def CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY
 *
 *  @brief
 *    The critical event id counter (eidc) persisted storage key.
 */
#ifndef CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY "crit-eidc"
#endif

/**
 *  @def CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY
 *
 *  @brief
 *    The production event id counter (eidc) persisted storage key.
 */
#ifndef CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY "prod-eidc"
#endif

/**
 *  @def CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY
 *
 *  @brief
 *    The info event id counter (eidc) persisted storage key.
 */
#ifndef CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY "info-eidc"
#endif

/**
 *  @def CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY
 *
 *  @brief
 *    The debug event id counter (eidc) persisted storage key.
 */
#ifndef CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY "debug-eidc"
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
#ifndef CHIP_DEVICE_CONFIG_FIRWMARE_BUILD_DATE
#define CHIP_DEVICE_CONFIG_FIRWMARE_BUILD_DATE __DATE__
#endif

/**
 * CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME
 *
 * Specifies the time of the build. Useful for deterministic builds.
 */
#ifndef CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME
#define CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME __TIME__
#endif

// -------------------- Device DNS-SD Advertising Configuration --------------------

/**
 * CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
 *
 * Enable or disable whether this device advertises as a commissioner.
 *
 * For Video Players, this value will be 1
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0
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
