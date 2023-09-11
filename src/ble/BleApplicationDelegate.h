/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2014-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the interface for upcalls from BleLayer
 *      to a client application.
 */

#pragma once

#include <ble/BleConfig.h>

#include <lib/support/DLLUtil.h>

namespace chip {
namespace Ble {

// Platform-agnostic BLE interface
class DLL_EXPORT BleApplicationDelegate
{
public:
    virtual ~BleApplicationDelegate() {}

    // CHIP calls this function once it closes the last BLEEndPoint associated with a BLE given connection object.
    // A call to this function means CHIP no longer cares about the state of the given BLE connection.
    // The application can use this callback to e.g. close the underlying BLE connection if it is no longer needed,
    // decrement the connection's refcount if it has one, or perform any other sort of cleanup as desired.
    virtual void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj) = 0;
};

} /* namespace Ble */
} /* namespace chip */
