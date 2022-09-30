/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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

#pragma once

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
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#define CHIP_CONFIG_REQUIRE_AUTH 1

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
#define CHIP_DEVICE_CONFIG_USE_TEST_PAIRING_CODE "CHIPUS"

/**
 * CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in CHIP NV storage.
 */
#define CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER "DUMMY_SN"

#endif // BUILD_RELEASE

#if (defined(CONFIG_CHIP_MW320_REAL_FACTORY_DATA) && (CONFIG_CHIP_MW320_REAL_FACTORY_DATA == 1))

// VID/PID for product => will be used by Basic Information Cluster
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0x1037
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0xA220

// set it to 0 for the moment
#define CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER 0

#ifndef CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION
//-> format_version = 1
//-> vendor_id = 0x1037
//-> product_id_array = [ 0xA220 ]
//-> device_type_id = 0x0015
//-> certificate_id = "ZIG20142ZB330003-24"
//-> security_level = 0
//-> security_information = 0
//-> version_number = 0x2694
//-> certification_type = 1
//-> dac_origin_vendor_id is not present
//-> dac_origin_product_id is not present
#define CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION                                                                               \
    {                                                                                                                              \
        0x30, 0x81, 0xe7, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xd9, 0x30, 0x81, 0xd6,    \
            0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30,      \
            0x43, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x36, 0x04, 0x34, 0x15, 0x24, 0x00,      \
            0x01, 0x25, 0x01, 0x37, 0x10, 0x36, 0x02, 0x05, 0x20, 0xa2, 0x18, 0x24, 0x03, 0x15, 0x2c, 0x04, 0x13, 0x5a, 0x49,      \
            0x47, 0x32, 0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05,      \
            0x00, 0x24, 0x06, 0x00, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01, 0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01, 0x03, 0x80,      \
            0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3,      \
            0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08,      \
            0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x20, 0x11, 0x8c, 0x13, 0x63, 0x9a,      \
            0xe7, 0x77, 0xaa, 0xc1, 0x52, 0x8c, 0x46, 0x40, 0x82, 0xcc, 0xdb, 0x6c, 0x43, 0x4d, 0xfc, 0xff, 0xe2, 0x5e, 0x1f,      \
            0xda, 0xef, 0xdf, 0x0d, 0xf1, 0x7c, 0x2c, 0xcd, 0x02, 0x21, 0x00, 0xbf, 0xad, 0x91, 0xc3, 0x3b, 0xf5, 0xb9, 0x89,      \
            0x2e, 0x5e, 0x15, 0x3c, 0x52, 0x61, 0xad, 0xb8, 0x53, 0x42, 0x46, 0xc6, 0x7d, 0xec, 0xc0, 0x93, 0x10, 0x1f, 0xc4,      \
            0xd2, 0xc3, 0x8a, 0xd5, 0x16,                                                                                          \
    }

// All remaining data will be pulled from the provisioning region of flash.
#endif

#else // CONFIG_CHIP_MW320_REAL_FACTORY_DATA

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * 0x235A: Chip's Vendor Id.
 * 0xFFF1: Test vendor.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * 0xFE00: SDK Sample Lock Resource
 */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8001

#endif // CONFIG_CHIP_MW320_REAL_FACTORY_DATA

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION
 *
 * The product revision number assigned to device or product by the device vendor.  This
 * number is scoped to the device product id, and typically corresponds to a revision of the
 * physical device, a change to its packaging, and/or a change to its marketing presentation.
 * This value is generally *not* incremented for device software revisions.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION 1
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION 1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
 *
 * A string identifying the firmware revision running on the device.
 * CHIP currently expects the firmware version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
#define CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION 1
#endif
/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Enable support for CHIP-over-BLE (CHIPOBLE).
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote CHIP Time service
 * using the CHIP Time Sync protocol.
 */
//#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 1

/**
 * CHIP_CONFIG_MAX_BINDINGS
 *
 * Maximum number of simultaneously active bindings per ChipExchangeManager
 * 1 (Time Sync) + 2 (Two 1-way subscriptions) + 1 (Software Update) = 4
 * in the worst case. Keeping another 4 as buffer.
 */
#define CHIP_CONFIG_MAX_BINDINGS 6

/**
 * CHIP_CONFIG_EVENT_LOGGING_WDM_OFFLOAD
 *
 * Select the ability to offload event logs to any interested subscribers using WDM.
 */
#define CHIP_CONFIG_EVENT_LOGGING_WDM_OFFLOAD 1

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

#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1
