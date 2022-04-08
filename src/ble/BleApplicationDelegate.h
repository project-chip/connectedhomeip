/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
