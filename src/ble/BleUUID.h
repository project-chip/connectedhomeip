/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2014-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>

namespace chip {
namespace Ble {

// Type to represent 128-bit BLE UUIDs. 16-bit short UUIDs may be combined with
// the Bluetooth Base UUID to form full 128-bit UUIDs as described in the
// Service Discovery Protocol (SDP) definition, part of the Bluetooth Core
// Specification.
struct ChipBleUUID
{
    uint8_t bytes[16];
};

// UUID of CHIP BLE service. Exposed for use in scan filter.
extern const ChipBleUUID CHIP_BLE_SVC_ID;

bool UUIDsMatch(const ChipBleUUID * idOne, const ChipBleUUID * idTwo);
bool StringToUUID(const char * str, ChipBleUUID & uuid);

} /* namespace Ble */
} /* namespace chip */
