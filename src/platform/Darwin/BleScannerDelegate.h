/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
