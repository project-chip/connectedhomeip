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

const char * ToProtocolName(uint16_t vendorId, uint16_t protocolId);

const char * ToProtocolMessageTypeName(uint16_t vendorId, uint16_t protocolId, uint8_t protocolCode);

CHIP_ERROR LogAsProtocolMessage(uint16_t vendorId, uint16_t protocolId, uint8_t protocolCode, const char * payload, size_t len,
                                bool interactionModelResponse);

} // namespace trace
} // namespace chip
