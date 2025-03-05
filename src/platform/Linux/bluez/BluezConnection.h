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
#include <platform/Linux/dbus/bluez/DBusBluez.h>
#include <system/SystemPacketBuffer.h>

#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BluezEndpoint;

class BluezConnection
{
public:
    BluezConnection(BluezDevice1 & aDevice) : mDevice(reinterpret_cast<BluezDevice1 *>(g_object_ref(&aDevice))) {}
    ~BluezConnection() = default;

    CHIP_ERROR Init(const BluezEndpoint & aEndpoint);

    const char * GetPeerAddress() const;

    uint16_t GetMTU() const { return mMtu; }
    void SetMTU(uint16_t aMtu) { mMtu = aMtu; }

    bool IsNotifyAcquired() const { return mNotifyAcquired; }
    void SetNotifyAcquired(bool aNotifyAcquired) { mNotifyAcquired = aNotifyAcquired; }

    /**
     * @brief Setup callback for receiving data from the CHIP TX characteristic on
     *        the remote peripheral device.
     *
     * @note This function takes the ownership of the passed file descriptor and
     *       will close it when the connection is closed.
     */
    void SetupWriteHandler(int aSocketFd);

    /**
     * @brief Setup callback for receiving HUP event on the notification channel.
     *
     * @note This function takes the ownership of the passed file descriptor and
     *       will close it when the connection is closed.
     */
    void SetupNotifyHandler(int aSocketFd, bool aAdditionalAdvertising = false);

    /// Send indication to the CHIP RX characteristic on the remote peripheral device
    CHIP_ERROR SendIndication(chip::System::PacketBufferHandle apBuf);
    /// Write to the CHIP RX characteristic on the remote peripheral device
    CHIP_ERROR SendWriteRequest(chip::System::PacketBufferHandle apBuf);
    /// Subscribe to the CHIP TX characteristic on the remote peripheral device
    CHIP_ERROR SubscribeCharacteristic();
    /// Unsubscribe from the CHIP TX characteristic on the remote peripheral device
    CHIP_ERROR UnsubscribeCharacteristic();

    CHIP_ERROR CloseConnection();

private:
    struct IOChannel
    {
        IOChannel() = default;
        ~IOChannel();

        GAutoPtr<GIOChannel> mChannel;
        GAutoPtr<GSource> mWatchSource;
    };

    struct ConnectionDataBundle
    {
        ConnectionDataBundle(const BluezConnection &, const chip::System::PacketBufferHandle &);
        ~ConnectionDataBundle() = default;

        const BluezConnection & mConn;
        GAutoPtr<GVariant> mData;
    };

    static CHIP_ERROR CloseConnectionImpl(BluezConnection * apConn);

    static gboolean WriteHandlerCallback(GIOChannel * aChannel, GIOCondition aCond, BluezConnection * apConn);
    static gboolean NotifyHandlerCallback(GIOChannel * aChannel, GIOCondition aCond, BluezConnection * apConn);

    static CHIP_ERROR SendIndicationImpl(ConnectionDataBundle * data);

    static void SendWriteRequestDone(GObject * aObject, GAsyncResult * aResult, gpointer apConn);
    static CHIP_ERROR SendWriteRequestImpl(ConnectionDataBundle * data);

    static void OnCharacteristicChanged(GDBusProxy * aInterface, GVariant * aChangedProperties,
                                        const gchar * const * aInvalidatedProps, gpointer apConn);
    static void SubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConn);
    static CHIP_ERROR SubscribeCharacteristicImpl(BluezConnection * apConn);

    static void UnsubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConn);
    static CHIP_ERROR UnsubscribeCharacteristicImpl(BluezConnection * apConn);

    GAutoPtr<BluezDevice1> mDevice;
    GAutoPtr<BluezGattService1> mService;

    bool mNotifyAcquired = false;
    uint16_t mMtu        = 0;

    GAutoPtr<BluezGattCharacteristic1> mC1;
    IOChannel mC1Channel = {};
    GAutoPtr<BluezGattCharacteristic1> mC2;
    IOChannel mC2Channel = {};
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    GAutoPtr<BluezGattCharacteristic1> mC3;
    IOChannel mC3Channel = {};
#endif
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
