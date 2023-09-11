/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ble/BleApplicationDelegate.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BleApplicationDelegateImpl : public Ble::BleApplicationDelegate
{
public:
    virtual void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
