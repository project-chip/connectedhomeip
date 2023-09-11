/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
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

// include the CHIPProjectConfig from config/standalone

#ifndef CHIP_CONFIG_KVS_PATH
#define CHIP_CONFIG_KVS_PATH "/tmp/chip_casting_kvs"
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 0

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 41 // 0x0029 = 41 = Video Client

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021

#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Test TV casting app"

#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0

// Enable some test-only interaction model APIs.
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1

#define CHIP_ENABLE_ROTATING_DEVICE_ID 1

#define CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH 128

// Disable this since it should not be enabled for production setups
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 0

#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 4

// Include the CHIPProjectConfig from config/standalone
// Add this at the end so that we can hit our #defines first
#include <CHIPProjectConfig.h>
