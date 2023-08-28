/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

// Use a default pairing code if one hasn't been provisioned in flash.
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#endif

// define Device type based on the application
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 257 // 0x0101  Dimmable Bulb

// For convenience, Chip Security Test Mode can be enabled and the
// requirement for authentication in various protocols can be disabled.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
#define CHIP_CONFIG_SECURITY_TEST_MODE 0

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
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 * CHIP service currently expects the software version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.0"
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
 *
 * A uint32_t identifying the software version running on the device.
 */
/* The SoftwareVersion attribute of the Basic cluster. */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1
#endif

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
 * CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
 *
 * A size, in bytes, of the individual debug event logging buffer.
 */
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)

/**
 * CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
 *
 * A size, max count of the sub device for bridge app.
 */
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT (16)
