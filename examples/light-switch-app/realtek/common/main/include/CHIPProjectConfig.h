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

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
 *
 * @brief
 *   Test Spake2p passcode to use.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#endif

/**
 * @def CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
 *
 * @brief
 *   Setup discriminator to use.
 */
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#endif

// For convenience, Chip Security Test Mode can be enabled and the
// requirement for authentication in various protocols can be disabled.
//
//    WARNING: These options make it possible to circumvent basic Chip security functionality,
//    including message encryption. Because of this they MUST NEVER BE ENABLED IN PRODUCTION BUILDS.
//
// #define CHIP_CONFIG_SECURITY_TEST_MODE 0

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * The CHIP-assigned vendor id for the organization responsible for producing the device.
 *
 * Default is the Test VendorID of 0xFFF1.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
 *
 * The unique id assigned by the device vendor to identify the product or device type.  This
 * number is scoped to the device vendor id.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8001
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
 *
 * Human readable vendor name for the organization responsible for producing the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "TEST_VENDOR"
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
 * CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number.
 */
#ifndef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"
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
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION 1
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
 *
 * Human readable string identifying version of the product assigned by the device vendor.
 */
#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING "1.0"
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
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 */
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.0"
#endif

/**
 * CHIP_DEVICE_CONFIG_THREAD_SSED
 *
 * Enable support for Thread Synchronized Sleepy End Device behavior.
 *
 */
#if CHIP_ENABLE_OPENTHREAD
#ifndef CHIP_DEVICE_CONFIG_THREAD_SSED
#define CHIP_DEVICE_CONFIG_THREAD_SSED 0
#endif
#if CHIP_DEVICE_CONFIG_THREAD_SSED
#define CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL System::Clock::Milliseconds32(2000)
#endif
#endif

#ifndef CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
#define CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE 0
#endif

#ifndef CONFIG_DEFAULT_ZAP
#define CONFIG_DEFAULT_ZAP 1
#endif

// You must modidy chip_main.cmake : ZAP_FILE ${matter_example_path}/../data_model/light-switch-app-1_to_2.zap
#ifndef CONFIG_1_TO_2_ZAP
#define CONFIG_1_TO_2_ZAP 0
#endif

// You must modidy chip_main.cmake : ZAP_FILE ${matter_example_path}/../data_model/light-switch-app-1_to_8.zap
#ifndef CONFIG_1_TO_8_ZAP
#define CONFIG_1_TO_8_ZAP 0
#endif

// You must modidy chip_main.cmake : ZAP_FILE ${matter_example_path}/../data_model/light-switch-app-1_to_11.zap
#ifndef CONFIG_1_TO_11_ZAP
#define CONFIG_1_TO_11_ZAP 0
#define MATTER_BINDING_TABLE_SIZE 20
#endif

// Set the following define to use the Certification Declaration from CHIPDevicePlatformConfig.h and not use it stored in factory
// data section
#ifndef CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION
#define CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION 0
#endif
