/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#if !__has_feature(objc_arc)
#error This file must be compiled in ObjC++ mode with ARC.
#endif

#include <ble/Ble.h>

#import <CoreBluetooth/CoreBluetooth.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Holds a CBPeripheral and the associated CBCentralManager
struct BlePeripheral
{
    CBPeripheral * const peripheral;
    CBCentralManager * const centralManager;

    BlePeripheral(CBPeripheral * aPeripheral, CBCentralManager * aCentralManager) :
        peripheral(aPeripheral), centralManager(aCentralManager)
    {}
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
