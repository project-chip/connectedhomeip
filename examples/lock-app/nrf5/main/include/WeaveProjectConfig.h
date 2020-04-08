/*
 *
 *    Copyright (c) 2019 Google LLC.
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
 *          Example project configuration file for OpenWeave.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general OpenWeave features.
 *
 */

#ifndef WEAVE_PROJECT_CONFIG_H
#define WEAVE_PROJECT_CONFIG_H

#if BUILD_RELEASE  // release build

// Security and Authentication enabled for release build.
#define WEAVE_CONFIG_SECURITY_TEST_MODE 0
#define WEAVE_CONFIG_REQUIRE_AUTH 1

#else  // development build

// Security and Authentication disabled for development build.
// For convenience, enable Weave Security Test Mode and disable the requirement for
// authentication in various protocols.
// WARNING: These options make it possible to circumvent basic Weave security functionality,
// including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
#define WEAVE_CONFIG_SECURITY_TEST_MODE 1
#define WEAVE_CONFIG_REQUIRE_AUTH 0

/**
 * WEAVE_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
 *
 * Enables the use of a hard-coded default Weave device id and credentials if no device id
 * is found in Weave NV storage.
 *
 * This option is for testing only and should be disabled in production releases.
 */
#define WEAVE_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY 34

// Use a default pairing code if one hasn't been provisioned in flash.
#define WEAVE_DEVICE_CONFIG_USE_TEST_PAIRING_CODE "NESTUS"

/**
 * WEAVE_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in Weave NV storage.
 */
#define WEAVE_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER "DUMMY_SN"

# endif // BUILD_RELEASE

/**
 * WEAVE_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * 0xE100: Google's Vendor Id.
 */
#define WEAVE_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xE100

/**
 * WEAVE_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * 0xFE00: SDK Sample Lock Resource
 */
#define WEAVE_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0xFE00

/**
 * WEAVE_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION
 *
 * The product revision number assigned to device or product by the device vendor.  This
 * number is scoped to the device product id, and typically corresponds to a revision of the
 * physical device, a change to its packaging, and/or a change to its marketing presentation.
 * This value is generally *not* incremented for device software revisions.
 */
#define WEAVE_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION 1

/**
 * WEAVE_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
 *
 * A string identifying the firmware revision running on the device.
 * Nest service currently expects the firmware version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef WEAVE_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
#define WEAVE_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION "1.0d1"
#endif
/**
 * WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
 *
 * Enable support for Weave-over-BLE (WoBLE).
 */
#define WEAVE_DEVICE_CONFIG_ENABLE_WOBLE 1

/**
 * WEAVE_DEVICE_CONFIG_ENABLE_WEAVE_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote Weave Time service
 * using the Weave Time Sync protocol.
 */
#define WEAVE_DEVICE_CONFIG_ENABLE_WEAVE_TIME_SERVICE_TIME_SYNC 1

/**
 * WEAVE_CONFIG_MAX_BINDINGS
 *
 * Maximum number of simultaneously active bindings per WeaveExchangeManager
 * 1 (Time Sync) + 2 (Two 1-way subscriptions) + 1 (Software Update) = 4
 * in the worst case. Keeping another 4 as buffer.
 */
#define WEAVE_CONFIG_MAX_BINDINGS 8

/**
 * WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD
 *
 * Select the ability to offload event logs to any interested subscribers using WDM.
 */
#define WEAVE_CONFIG_EVENT_LOGGING_WDM_OFFLOAD 1

/**
 * WEAVE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
 *
 * Enable recording UTC timestamps.
 */
#define WEAVE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 1

/**
 * WEAVE_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
 *
 * A size, in bytes, of the individual debug event logging buffer.
 */
#define WEAVE_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)

/**
 * WEAVE_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE
 *
 * For a development build, set the default importance of events to be logged as Debug.
 * Since debug is the lowest importance level, this means all standard, critical, info and
 * debug importance level vi events get logged.
 */
#if BUILD_RELEASE
#define WEAVE_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE nl::Weave::Profiles::DataManagement::Production
#else
#define WEAVE_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE nl::Weave::Profiles::DataManagement::Debug
#endif // BUILD_RELEASE

#endif // WEAVE_PROJECT_CONFIG_H
