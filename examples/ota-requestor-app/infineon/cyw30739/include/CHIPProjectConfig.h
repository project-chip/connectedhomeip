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

// -------------------- Device Identification Configuration --------------------

/* The VendorName attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "Infineon"

/* The VendorID attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF2

/* The ProductName attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "CYW30739 OTA Requestor App"

/* The ProductID attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8002

/* The HardwareVersionString attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING "30739"

/* The HardwareVersion attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION 30739

/* The SoftwareVersionString attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "TE7.5"

/* The SoftwareVersion attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x0705

/* The SerialNumber attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"

// -------------------- Test Configuration --------------------
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
