/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief The include file for all the types for the data model that are not
 *        dependent on an individual application configuration.
 */

#pragma once

#include <cstdint>

// Pull in core types
#include <lib/core/DataModelTypes.h>

namespace chip {

typedef uint8_t Percent;
typedef uint16_t Percent100ths;

} // namespace chip
