/*
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#ifndef CHIP_PROJECT_CONFIG_H
#define CHIP_PROJECT_CONFIG_H

#if BUILD_RELEASE // release build

// Security and Authentication enabled for release build.
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#define CHIP_CONFIG_REQUIRE_AUTH 1

#else // development build

// Security and Authentication disabled for development build.
// For convenience, enable CHIP Security Test Mode and disable the requirement for
// authentication in various protocols.
// WARNING: These options make it possible to circumvent basic CHIP security functionality,
// including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
#define CHIP_CONFIG_SECURITY_TEST_MODE 1
#define CHIP_CONFIG_REQUIRE_AUTH 0

/**
 * CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
 *
 * Enables the use of a hard-coded default CHIP device id and credentials if no device id
 * is found in CHIP NV storage.
 *
 * This option is for testing only and should be disabled in production releases.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY 34

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_PAIRING_CODE "CHIPUS"

/**
 * CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in CHIP NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"

#endif // BUILD_RELEASE

/**
 * CHIP_DEVICE_CONFIG_THREAD_FTD
 *
 * Disable Full Thread Device features
 */
#define CHIP_DEVICE_CONFIG_THREAD_FTD 1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * 0x235A: Chip's Vendor Id.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0x235A

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * 0x434B: cc13x2x7_26x2x7 lock-app
 */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x434B

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION
 *
 * The product revision number assigned to device or product by the device vendor.  This
 * number is scoped to the device product id, and typically corresponds to a revision of the
 * physical device, a change to its packaging, and/or a change to its marketing presentation.
 * This value is generally *not* incremented for device software revisions.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION 1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING
 *
 * A string identifying the firmware revision running on the device.
 * CHIP currently expects the firmware version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING "1.0d1"
#endif
/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Enable support for CHIP-over-BLE (CHIPOBLE).
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote CHIP Time service
 * using the CHIP Time Sync protocol.
 */
//#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 1

/**
 * CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
 *
 * Enable recording UTC timestamps.
 */
#define CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 1

/**
 * CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
 *
 * A size, in bytes, of the individual debug event logging buffer.
 */
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)

/**
 * CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
 *
 * Enable the OpenThread SRP client to allow for CHIP device discovery.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1

/**
 * CHIP_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE
 *
 * For a development build, set the default importance of events to be logged as Debug.
 * Since debug is the lowest importance level, this means all standard, critical, info and
 * debug importance level vi events get logged.
 */
#if BUILD_RELEASE
#define CHIP_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE chip::Profiles::DataManagement::Production
#else
#define CHIP_CONFIG_EVENT_LOGGING_DEFAULT_IMPORTANCE chip::Profiles::DataManagement::Debug
#endif // BUILD_RELEASE

#endif // CHIP_PROJECT_CONFIG_H
