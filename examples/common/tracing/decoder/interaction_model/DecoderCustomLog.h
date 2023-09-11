/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <lib/core/CHIPError.h>

#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace trace {
namespace interaction_model {

CHIP_ERROR MaybeDecodeNestedReadResponse(const uint8_t * data, size_t dataLen);

CHIP_ERROR MaybeDecodeNestedCommandResponse(const uint8_t * data, size_t dataLen);

CHIP_ERROR MaybeDecodeNestedCommandRequest(const uint8_t * data, size_t dataLen);

} // namespace interaction_model
} // namespace trace
} // namespace chip
