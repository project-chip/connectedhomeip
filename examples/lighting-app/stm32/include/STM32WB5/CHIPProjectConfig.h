/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      CHIP project configuration for stm32wb builds.
 *
 */
#ifndef CHIPPROJECTCONFIG_H
#define CHIPPROJECTCONFIG_H
#include "app_conf.h"

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#endif

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_PAIRING_CODE "CHIPUS"

// For convenience, Chip Security Test Mode can be enabled and the
// requirement for authentication in various protocols can be disabled.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
#define CHIP_CONFIG_SECURITY_TEST_MODE 0

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * 0xFFF1: Test vendor
 */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * 0x8004: example lighting app
 */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8004

/* define Device type based on the application */
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 257 // 0x0101  Dimmable Bulb

/**
 * CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in Chip NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"

/**
 * CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
 *
 * Enable recording UTC timestamps.
 */
#define CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 1

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote Chip Time service
 * using the Chip Time Sync protocol.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

/**
 * CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
 *
 * Enables the use of a hard-coded default Chip device id and credentials if no device id
 * is found in Chip NV storage.
 *
 * This option is for testing only and should be disabled in production releases.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY 34

// For convenience, enable Chip Security Test Mode and disable the requirement for
// authentication in various protocols.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#define CHIP_CONFIG_REQUIRE_AUTH 1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 * CHIP service currently expects the software version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.1"
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
 * CHIP service currently expects the firmware version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING "1.17"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION
 *
 * The hardware version number assigned to device or product by the device vendor.  This
 * number is scoped to the device product id, and typically corresponds to a revision of the
 * physical device, a change to its packaging, and/or a change to its marketing presentation.
 * This value is generally *not* incremented for device software versions.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION 1

/**
 * CHIP_DETAIL_LOGGING
 *
 * Show detail log in terminal
 */
#define CHIP_DETAIL_LOGGING 0

/**
 * CHIP_ERROR_LOGGING
 *
 * Show error log in terminal
 */
#define CHIP_ERROR_LOGGING 1

/**
 * CHIP_PROGRESS_LOGGING
 *
 * Show progress log in terminal
 */
#define CHIP_PROGRESS_LOGGING 1

/**
 * CHIP_AUTOMATION_LOGGING
 *
 * Show  automation log in terminal
 */
#define CHIP_AUTOMATION_LOGGING 1

#define CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES 5
/**
 * HIGHWATERMARK
 *
 * define freertos marker
 */
#define HIGHWATERMARK 0

#endif /* CHIPPROJECTCONFIG_H */
