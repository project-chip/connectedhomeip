/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef BLE_CONNECTION_DELEGATE_H
#define BLE_CONNECTION_DELEGATE_H

#include <ble/BleConnectionDelegate.h>

using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BleConnectionDelegateImpl : public BleConnectionDelegate
{
public:
    virtual void NewConnection(Ble::BleLayer * bleLayer, void * appState, const uint16_t connDiscriminator);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // BLE_CONNECTION_DELEGATE_H
