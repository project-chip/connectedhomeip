/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <ble/BleLayer.h>
#include <controller/java/AndroidBleConnectionDelegate.h>

void AndroidBleConnectionDelegate::NewConnection(chip::Ble::BleLayer * bleLayer, void * appState, const uint16_t connDiscriminator)
{
    if (newConnectionCb)
    {
        newConnectionCb(bleLayer->mAppState, connDiscriminator);
    }
}

BLE_ERROR AndroidBleConnectionDelegate::CancelConnection()
{
    return BLE_ERROR_NOT_IMPLEMENTED;
}
