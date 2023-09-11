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

const char * ToProtocolName();

const char * ToProtocolMessageTypeName(uint8_t protocolCode);

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len, bool decodeResponse);

} // namespace interaction_model
} // namespace trace
} // namespace chip
