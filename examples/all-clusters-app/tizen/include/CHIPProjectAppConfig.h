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
#include <CHIPProjectConfig.h>

// All clusters app has 3 group endpoints. This needs to defined here so that
// CHIP_CONFIG_MAX_GROUPS_PER_FABRIC is properly configured.
#define CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC 3
