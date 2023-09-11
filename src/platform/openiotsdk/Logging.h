/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of logging support
 *          for Open IOT SDK platform.
 */

#pragma once

#include <lib/support/logging/Constants.h>

namespace chip {
namespace Logging {
namespace Platform {

void ois_logging_init(void);

} // namespace Platform
} // namespace Logging
} // namespace chip
