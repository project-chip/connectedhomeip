/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ble/CHIPBleServiceData.h>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace DeviceLayer {

class DLL_EXPORT BleScannerDelegate
{
public:
    virtual ~BleScannerDelegate() {}

    // Called when a scan result is available.
    virtual void OnBleScanAdd(BLE_CONNECTION_OBJECT connObj, const Ble::ChipBLEDeviceIdentificationInfo & info) = 0;

    // Called when a scan result is not available anymore.
    virtual void OnBleScanRemove(BLE_CONNECTION_OBJECT connObj) = 0;
};

} // namespace DeviceLayer
} // namespace chip
