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
 *      This file defines the interface for downcalls from BleLayer
 *      to a platform's BLE framework.
 */

#ifndef BLEPLATFORMDELEGATE_H_
#define BLEPLATFORMDELEGATE_H_

#include <BleLayer/BleConfig.h>
#include <BleLayer/BleUUID.h>

#include <SystemLayer/SystemPacketBuffer.h>
#include <Weave/Support/NLDLLUtil.h>

namespace nl {
namespace Ble {

using ::nl::Weave::System::PacketBuffer;

// Platform-agnostic BLE interface
class NL_DLL_EXPORT BlePlatformDelegate
{
public:
    // Following APIs must be implemented by platform:

    // Subscribe to updates and indications on the specfied characteristic
    virtual bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId,
                                         const WeaveBleUUID * charId) = 0;

    // Unsubscribe from updates and indications on the specified characteristic
    virtual bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId,
                                           const WeaveBleUUID * charId) = 0;

    // Close the underlying BLE connection.
    virtual bool CloseConnection(BLE_CONNECTION_OBJECT connObj) = 0;

    // Get MTU size negotiated for specified BLE connection. Return value of 0 means MTU size could not be determined.
    virtual uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const = 0;

    // Data path calling convention:
    //   The Weave stack retains partial ownership of pBufs sent via the below functions. These buffers are freed by
    //   Weave after either they're acknowledged by the peer's BLE controller, or Weave shuts down the pBuf's
    //   associated BLEEndPoint.
    //
    //   For its part, the platform MUST call PacketBuffer::Free on each pBuf it receives via a Send* function once it no
    //   longer requires a reference to this buffer, e.g. when a NL_CLIENT_EVENT_BLE_PBUF_CLEAR event is received on
    //   platforms with the Nest BLE SDK.
    //
    //   On platforms such as iOS or Android where the contents of the pBuf PacketBuffer are copied into a separate
    //   buffer for transmission, pBuf may be freed on the downcall to the platform delegate once the copy completes.
    //
    //   A 'true' return value from a Send* function indicates that the characteristic was written or updated
    //   successfully. A 'false' value indicates failure, and is used to report this failure to the user via the return
    //   value of WeaveConnection::SendMessage.
    //
    //   If a Send* function returns false, it must release its reference to pBuf prior to return.

    // Send GATT characteristic indication request
    virtual bool SendIndication(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                                PacketBuffer * pBuf) = 0;

    // Send GATT characteristic write request
    virtual bool SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                                  PacketBuffer * pBuf) = 0;

    // Send GATT characteristic read request
    virtual bool SendReadRequest(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                                 PacketBuffer * pBuf) = 0;

    // Send response to remote host's GATT chacteristic read response
    virtual bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
                                  const WeaveBleUUID * svcId, const WeaveBleUUID * charId) = 0;
};

} /* namespace Ble */
} /* namespace nl */

#endif /* BLEPLATFORMDELEGATE_H_ */
