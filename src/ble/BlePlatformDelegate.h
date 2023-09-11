/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2014-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the interface for downcalls from BleLayer
 *      to a platform's BLE framework.
 */

#pragma once

#include <ble/BleConfig.h>

#include <ble/BleUUID.h>

#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Ble {

using ::chip::System::PacketBufferHandle;

// Platform-agnostic BLE interface
class DLL_EXPORT BlePlatformDelegate
{
public:
    virtual ~BlePlatformDelegate() {}

    // Following APIs must be implemented by platform:

    // Subscribe to updates and indications on the specfied characteristic
    virtual bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId) = 0;

    // Unsubscribe from updates and indications on the specified characteristic
    virtual bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                           const ChipBleUUID * charId) = 0;

    // Close the underlying BLE connection.
    virtual bool CloseConnection(BLE_CONNECTION_OBJECT connObj) = 0;

    // Get MTU size negotiated for specified BLE connection. Return value of 0 means MTU size could not be determined.
    virtual uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const = 0;

    // Data path calling convention:
    //   A 'true' return value from a Send* function indicates that the characteristic was written or updated
    //   successfully. A 'false' value indicates failure, and is used to report this failure to the user via the return
    //   value of chipConnection::SendMessage.

    // Send GATT characteristic indication request
    virtual bool SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                PacketBufferHandle pBuf) = 0;

    // Send GATT characteristic write request
    virtual bool SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                  PacketBufferHandle pBuf) = 0;

    // Send GATT characteristic read request
    virtual bool SendReadRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                 PacketBufferHandle pBuf) = 0;

    // Send response to remote host's GATT chacteristic read response
    virtual bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                                  const ChipBleUUID * charId) = 0;
};

} /* namespace Ble */
} /* namespace chip */
