/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <ble/Ble.h>
#include <ble/BleLayer.h>

namespace chip {
namespace Testing {
/**
 * @brief Class acts as an accessor to private methods of the BleLayer class without needing to give friend access to
 *        each individual test.
 */
class BleLayerTestAccess
{
public:
    BleLayerTestAccess() = delete;
    BleLayerTestAccess(Ble::BleLayer * layer) : mLayer(layer) {}

    void SetConnectionDelegate(Ble::BleConnectionDelegate * delegate) { mLayer->mConnectionDelegate = delegate; }

    // Wrapper for OnConnectionComplete private method
    void CallOnConnectionComplete(void * appState, BLE_CONNECTION_OBJECT connObj)
    {
        mLayer->OnConnectionComplete(appState, connObj);
    }

    void CallOnConnectionError(void * appState, CHIP_ERROR err) { mLayer->OnConnectionError(appState, err); }

private:
    Ble::BleLayer * mLayer = nullptr;
};
} // namespace Testing
} // namespace chip
