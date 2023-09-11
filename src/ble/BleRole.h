/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2014-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {
namespace Ble {

/// Role of end points' associated BLE connections. Determines means used by end points to send and receive data.
typedef enum
{
    kBleRole_Central    = 0,
    kBleRole_Peripheral = 1
} BleRole;

} // namespace Ble
} // namespace chip
