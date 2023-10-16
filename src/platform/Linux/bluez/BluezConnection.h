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

class BluezConnection
{
public:
    struct IOChannel
    {
        IOChannel() = default;
        ~IOChannel();

        GIOChannel * mpChannel;
        GSource * mWatchSource;
    };

    BluezConnection(BluezEndpoint * apEndpoint, BluezDevice1 * apDevice);
    ~BluezConnection();

    CHIP_ERROR SendBluezIndication(chip::System::PacketBufferHandle apBuf);
    CHIP_ERROR CloseBluezConnection();

    const char * GetPeerAddress() const;

    uint16_t GetMTU() const { return mMtu; }
    void SetMTU(uint16_t aMtu) { mMtu = aMtu; }

    bool IsNotifyAcquired() const { return mNotifyAcquired; }
    void SetNotifyAcquired(bool aNotifyAcquired) { mNotifyAcquired = aNotifyAcquired; }

    /// Write to the CHIP RX characteristic on the remote peripheral device
    CHIP_ERROR BluezSendWriteRequest(chip::System::PacketBufferHandle apBuf);
    /// Subscribe to the CHIP TX characteristic on the remote peripheral device
    CHIP_ERROR BluezSubscribeCharacteristic();
    /// Unsubscribe from the CHIP TX characteristic on the remote peripheral device
    CHIP_ERROR BluezUnsubscribeCharacteristic();

private:
    struct ConnectionDataBundle
    {
        ConnectionDataBundle(const BluezConnection &, const chip::System::PacketBufferHandle &);
        ~ConnectionDataBundle() = default;

        const BluezConnection & mConn;
        GAutoPtr<GVariant> mData;
    };

    CHIP_ERROR Init();

    static CHIP_ERROR BluezC2Indicate(ConnectionDataBundle * data);
    static CHIP_ERROR BluezDisconnect(BluezConnection * apConn);

    static void SendWriteRequestDone(GObject * aObject, GAsyncResult * aResult, gpointer apConn);
    static CHIP_ERROR SendWriteRequestImpl(ConnectionDataBundle * data);

    static void OnCharacteristicChanged(GDBusProxy * aInterface, GVariant * aChangedProperties,
                                        const gchar * const * aInvalidatedProps, gpointer apConn);
    static void SubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConn);
    static CHIP_ERROR SubscribeCharacteristicImpl(BluezConnection * apConn);

    static void UnsubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConn);
    static CHIP_ERROR UnsubscribeCharacteristicImpl(BluezConnection * apConn);

    BluezEndpoint * mpEndpoint;
    BluezDevice1 * mpDevice;

    bool mNotifyAcquired = false;
    uint16_t mMtu        = 0;

public:
    BluezGattService1 * mpService   = nullptr;
    BluezGattCharacteristic1 * mpC1 = nullptr;
    BluezGattCharacteristic1 * mpC2 = nullptr;
    // additional data characteristics
    BluezGattCharacteristic1 * mpC3 = nullptr;

    IOChannel mC1Channel = { 0 };
    IOChannel mC2Channel = { 0 };
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
