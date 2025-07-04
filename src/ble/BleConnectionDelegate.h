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

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <lib/support/DLLUtil.h>
#include <lib/support/SetupDiscriminator.h>
#include <lib/support/Span.h>

#include "BleConfig.h"
#include "BleError.h"

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

    // A callback indicating that a connection was established to a device with (long) discriminator
    // matchedDiscriminator.
    typedef void (*OnConnectionByDiscriminatorsCompleteFunct)(void * appState, uint16_t matchedLongDiscriminator,
                                                              BLE_CONNECTION_OBJECT connObj);

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

    // Call this function to delegate the connection steps required to get a BLE_CONNECTION_OBJECT
    // out of a peripheral that matches any of the given discriminators.
    //
    // The provided onConnectionComplete callback may be called multiple times, if multiple
    // connections are created.
    //
    // If the onConnectionError callback is called, that indicates that there will be no more
    // onConnectionComplete callbacks until the next NewConnection call.
    //
    // Calling CancelConnection will ensure no more calls to onConnectionComplete or
    // onConnectionError until the next NewConnection call.
    //
    // The implementation must not assume that the memory backing the "discriminators" argument will
    // outlive this call returning.
    //
    virtual CHIP_ERROR NewConnection(BleLayer * bleLayer, void * appState, const Span<const SetupDiscriminator> & discriminators,
                                     OnConnectionByDiscriminatorsCompleteFunct onConnectionComplete,
                                     OnConnectionErrorFunct onConnectionError)
    {
        // Should this handle the case when "discriminators" has length 1 automatically by
        // delegating to the NewConnection overload that takes a single SetupDiscriminator?  It adds
        // some unavoidable codesize and storage for the discriminator to do that.  Probably better
        // to have the API consumers handle that.
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

} /* namespace Ble */
} /* namespace chip */
