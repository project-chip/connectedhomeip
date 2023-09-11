/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines Defines error filtering and selection methods.
 *
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace Messaging {

CHIP_ERROR FilterUDPSendError(CHIP_ERROR err, bool isMulticast);
bool IsIgnoredMulticastSendError(CHIP_ERROR err);
bool IsSendErrorNonCritical(CHIP_ERROR err);

} // namespace Messaging
} // namespace chip
