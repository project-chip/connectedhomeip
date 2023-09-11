/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
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
#include <CHIPProjectConfig.h>

// All clusters app has 3 group endpoints. This needs to defined here so that
// CHIP_CONFIG_MAX_GROUPS_PER_FABRIC is properly configured.
#define CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC 3

// Allows app options (ports) to be configured on launch of app
#define CHIP_DEVICE_ENABLE_PORT_PARAMS 1

// Expose the device type in the advertisement for CI testing.
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

// Marks that a ModeBase Derived cluster is being used.
#define EMBER_AF_PLUGIN_MODE_BASE
