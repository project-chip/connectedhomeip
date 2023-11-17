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

/**
 *    @file
 *      This file defines the interface for application to delegate Ble connection to
 *      to BleLayer.
 */

#pragma once

#include <ble/BleConfig.h>
#include <ble/BleError.h>

#include <lib/support/DLLUtil.h>
#include <lib/support/SetupDiscriminator.h>

namespace chip {
namespace Ble {
class BleLayer;
} // namespace Ble
} // namespace chip

namespace chip {
namespace Ble {

// Platform-agnostic BLE interface
class DLL_EXPORT BleConnectionDelegate
{
public:
    virtual ~BleConnectionDelegate() {}

    // Public function pointers:
    typedef void (*OnConnectionCompleteFunct)(void * appState, BLE_CONNECTION_OBJECT connObj);
    OnConnectionCompleteFunct OnConnectionComplete;

    typedef void (*OnConnectionErrorFunct)(void * appState, CHIP_ERROR err);
    OnConnectionErrorFunct OnConnectionError;

    // Call this function to delegate the connection steps required to get a BLE_CONNECTION_OBJECT
    // out of a peripheral that matches the given discriminator.
    virtual void NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator) = 0;

    // Call this function to delegate the connection steps required to get a connected BLE_CONNECTION_OBJECT
    // out of a disconnected BLE_CONNECTION_OBJECT.
    virtual void NewConnection(BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj) = 0;

    // Call this function to stop the connection
    virtual CHIP_ERROR CancelConnection() = 0;
};

} /* namespace Ble */
} /* namespace chip */
