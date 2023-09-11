/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Defines the chip Device Layer Event Logging functions.
 *
 */

#pragma once

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Defines the event logging resources and initializes event logging management subsystem.
 */
extern CHIP_ERROR InitChipEventLogging();

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
