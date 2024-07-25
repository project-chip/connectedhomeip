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

// For convenience, enable Chip Security Test Mode and disable the requirement for
// authentication in various protocols.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
#define CHIP_CONFIG_SECURITY_TEST_MODE 0

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
 *
 * A uint32_t identifying the software version running on the device.
 * First two bytes are reflecting the Matter standard
 * Last two bytes are reflecting the SDK version of which the first nibble of the first byte represents the major
 * version and the second nibble of the first byte has the minor number. The last byte holds the patch number.
 * example for SDK v0.1.5 with Matter v1.2 standard:
 * 0x01020105
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#ifndef OTA_TEST_IMAGE
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x01020105
#else
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x01020106
#endif
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 * CHIP service currently expects the software version to be in the format
 * {MAJOR_VERSION}.0d{MINOR_VERSION}
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#ifndef OTA_TEST_IMAGE
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.2-0.1.5"
#else
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.2-0.1.6"
#endif
#endif

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Enable support for Chip-over-BLE (CHIPoBLE).
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

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

/**
 * @def CHIP_DEVICE_CONFIG_ENABLE_SED
 *
 * @brief Defines if a matter device is acting as a Sleepy End Device(SED)
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_SED
#define CHIP_DEVICE_CONFIG_ENABLE_SED 0
#endif

/**
 * @def CHIP_DEVICE_CONFIG_ENABLE_SSED
 *
 * @brief Defines if a matter device is acting as a Synchronized Sleepy End Device(SSED)
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_SSED
#define CHIP_DEVICE_CONFIG_ENABLE_SSED 0
#endif

/**
 * @def CHIP_DEVICE_CONFIG_THREAD_FTD
 *
 * @brief Defines if a matter device is acting as Full Thread Device (FTD)
 */
#ifndef CHIP_DEVICE_CONFIG_THREAD_FTD
#define CHIP_DEVICE_CONFIG_THREAD_FTD 1
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
