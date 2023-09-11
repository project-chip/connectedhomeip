/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Project configuration for Darwin Framework Tool.
 *
 */
#ifndef CHIPPROJECTCONFIG_H
#define CHIPPROJECTCONFIG_H

// Enable some test-only interaction model APIs.
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1

// Allow us, for test purposes, to encode invalid enum values.
#define CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES 1

#endif /* CHIPPROJECTCONFIG_H */
