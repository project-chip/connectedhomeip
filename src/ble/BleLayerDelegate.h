/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the interface for downcalls from BleLayer
 *      to a BLE transport.
 */

#pragma once

#include <ble/BLEEndPoint.h>
#include <ble/BleConfig.h>
#include <ble/BleError.h>
#include <ble/BleUUID.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Ble {

class BleLayerDelegate
{
public:
    virtual ~BleLayerDelegate() = default;

    virtual void OnBleConnectionComplete(BLEEndPoint * endpoint) = 0;
    virtual void OnBleConnectionError(CHIP_ERROR err)            = 0;

    virtual void OnEndPointConnectComplete(BLEEndPoint * endPoint, CHIP_ERROR err)                    = 0;
    virtual void OnEndPointMessageReceived(BLEEndPoint * endPoint, System::PacketBufferHandle && msg) = 0;
    virtual void OnEndPointConnectionClosed(BLEEndPoint * endPoint, CHIP_ERROR err)                   = 0;

    virtual CHIP_ERROR SetEndPoint(BLEEndPoint * endPoint) = 0;
};

} // namespace Ble
} // namespace chip
