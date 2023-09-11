/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "CYW30739 Lighting App"

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
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
