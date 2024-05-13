/*
 *
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

#pragma once

// Use a default setup PIN code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

// For convenience, enable Chip Security Test Mode and disable the requirement for
// authentication in various protocols.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
#define CHIP_CONFIG_SECURITY_TEST_MODE 0

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * 0xFFF1: Test vendor.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#endif // CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * 0x8009: example persistent-storage
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8009
#endif // CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID

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
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "0.1ALPHA"
#endif
/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Enable support for Chip-over-BLE (CHIPoBLE).
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

/**
 * CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in Chip NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"

/**
 * CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
 *
 * Enable Thread CLI interface at initialisation.
 */
#define CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI 1

/**
 *  @name Interaction Model object pool configuration.
 *
 *  @brief
 *    The following definitions sets the maximum number of corresponding interaction model object pool size.
 *
 *      * #CHIP_IM_MAX_NUM_COMMAND_HANDLER
 *      * #CHIP_IM_MAX_REPORTS_IN_FLIGHT
 *      * #CHIP_IM_MAX_NUM_WRITE_HANDLER
 *      * #CHIP_IM_MAX_NUM_WRITE_CLIENT
 *
 *  @{
 */

/**
 * @def CHIP_IM_MAX_NUM_COMMAND_HANDLER
 *
 * @brief Defines the maximum number of CommandHandler, limits the number of active commands transactions on server.
 */
#define CHIP_IM_MAX_NUM_COMMAND_HANDLER 2

/**
 * @def CHIP_IM_MAX_REPORTS_IN_FLIGHT
 *
 * @brief Defines the maximum number of Reports, limits the traffic of read and subscription transactions.
 */
#define CHIP_IM_MAX_REPORTS_IN_FLIGHT 2

/**
 * @def CHIP_IM_MAX_NUM_WRITE_HANDLER
 *
 * @brief Defines the maximum number of WriteHandler, limits the number of active write transactions on server.
 */
#define CHIP_IM_MAX_NUM_WRITE_HANDLER 2

/**
 * @def CHIP_IM_MAX_NUM_WRITE_CLIENT
 *
 * @brief Defines the maximum number of WriteClient, limits the number of active write transactions on client.
 */
#define CHIP_IM_MAX_NUM_WRITE_CLIENT 2
