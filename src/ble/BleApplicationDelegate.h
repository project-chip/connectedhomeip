/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file defines the interface for upcalls from BleLayer
 *      to a client application.
 */

#ifndef BLEAPPLICATIONDELEGATE_H_
#define BLEAPPLICATIONDELEGATE_H_

#include <Weave/Support/NLDLLUtil.h>

#include "BleConfig.h"

namespace nl {
namespace Ble {

// Platform-agnostic BLE interface
class NL_DLL_EXPORT BleApplicationDelegate
{
public:
    // Weave calls this function once it closes the last BLEEndPoint associated with a BLE given connection object.
    // A call to this function means Weave no longer cares about the state of the given BLE connection.
    // The application can use this callback to e.g. close the underlying BLE conection if it is no longer needed,
    // decrement the connection's refcount if it has one, or perform any other sort of cleanup as desired.
    virtual void NotifyWeaveConnectionClosed(BLE_CONNECTION_OBJECT connObj) = 0;
};

} /* namespace Ble */
} /* namespace nl */

#endif /* BLEAPPLICATIONDELEGATE_H_ */
