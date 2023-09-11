/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
