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

#pragma once

#include <stdint.h>

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct BluezEndpoint;

struct BluezConnection
{

    struct IOChannel
    {
        GIOChannel * mpChannel;
        GSource * mWatchSource;
    };

    struct ConnectionDataBundle
    {
        BluezConnection * mpConn;
        GVariant * mpVal;
    };

    char * mpPeerAddress;
    BluezDevice1 * mpDevice;
    BluezGattService1 * mpService;
    BluezGattCharacteristic1 * mpC1;
    BluezGattCharacteristic1 * mpC2;
    // additional data characteristics
    BluezGattCharacteristic1 * mpC3;

    bool mIsNotify;
    uint16_t mMtu;
    struct IOChannel mC1Channel;
    struct IOChannel mC2Channel;
    BluezEndpoint * mpEndpoint;
};

CHIP_ERROR SendBluezIndication(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf);
CHIP_ERROR CloseBluezConnection(BLE_CONNECTION_OBJECT apConn);

/// Write to the CHIP RX characteristic on the remote peripheral device
CHIP_ERROR BluezSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf);
/// Subscribe to the CHIP TX characteristic on the remote peripheral device
CHIP_ERROR BluezSubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn);
/// Unsubscribe from the CHIP TX characteristic on the remote peripheral device
CHIP_ERROR BluezUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
