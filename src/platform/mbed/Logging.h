/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Logging implementation for Mbed platform
 */

#pragma once

#include <lib/support/logging/Constants.h>

namespace chip {
namespace Logging {
namespace Platform {

void mbed_logging_init(void);

} // namespace Platform
} // namespace Logging
} // namespace chip
