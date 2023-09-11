/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#if !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

// include generated configuration information from AppBuilder.
// ZA_GENERATED_HEADER is defined in the project file
#ifdef ZA_GENERATED_HEADER
#include ZA_GENERATED_HEADER
#else
#include <zap-generated/gen_config.h>
#endif

#ifdef ATTRIBUTE_STORAGE_CONFIGURATION
#include ATTRIBUTE_STORAGE_CONFIGURATION
#else
#include <zap-generated/endpoint_config.h>
#endif

#endif // !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

// User options for plugin Binding Table Library
// TODO: Make this a CHIP_CONFIG value.
#ifndef EMBER_BINDING_TABLE_SIZE
#define EMBER_BINDING_TABLE_SIZE 10
#endif // EMBER_BINDING_TABLE_SIZE

/**
 * @brief CHIP uses millisecond ticks
 */
#define MILLISECOND_TICKS_PER_SECOND 1000
