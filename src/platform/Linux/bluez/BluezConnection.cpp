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

#include "BluezConnection.h"

#include <cstring>
#include <limits>
#include <memory>

#include <gio/gio.h>
#include <glib.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/ConnectivityManager.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <platform/PlatformManager.h>
#include <platform/internal/BLEManager.h>
#include <system/SystemPacketBuffer.h>

#include "BluezEndpoint.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

gboolean BluezIsServiceOnDevice(BluezGattService1 * aService, BluezDevice1 * aDevice)
{
    const auto * servicePath = bluez_gatt_service1_get_device(aService);
    const auto * devicePath  = g_dbus_proxy_get_object_path(G_DBUS_PROXY(aDevice));
    return strcmp(servicePath, devicePath) == 0 ? TRUE : FALSE;
}

gboolean BluezIsCharOnService(BluezGattCharacteristic1 * aChar, BluezGattService1 * aService)
{
    const auto * charPath    = bluez_gatt_characteristic1_get_service(aChar);
    const auto * servicePath = g_dbus_proxy_get_object_path(G_DBUS_PROXY(aService));
    ChipLogDetail(DeviceLayer, "Char %s on service %s", charPath, servicePath);
    return strcmp(charPath, servicePath) == 0 ? TRUE : FALSE;
}

} // namespace

BluezConnection::BluezConnection(const BluezEndpoint & aEndpoint, BluezDevice1 * apDevice) :
    mpDevice(BLUEZ_DEVICE1(g_object_ref(apDevice)))
{
    Init(aEndpoint);
}

BluezConnection::~BluezConnection()
{
    g_object_unref(mpDevice);
    if (mpService)
        g_object_unref(mpService);
    if (mpC1)
        g_object_unref(mpC1);
    if (mpC2)
        g_object_unref(mpC2);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    if (mpC3)
        g_object_unref(mpC2);
#endif
}

BluezConnection::IOChannel::~IOChannel()
{
    if (mWatchSource != nullptr)
        // Make sure the source is detached before destroying the channel.
        g_source_destroy(mWatchSource.get());
}

BluezConnection::ConnectionDataBundle::ConnectionDataBundle(const BluezConnection & aConn,
                                                            const chip::System::PacketBufferHandle & aBuf) :
    mConn(aConn),
    mData(g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, aBuf->Start(), aBuf->DataLength(), sizeof(uint8_t)))
{}

CHIP_ERROR BluezConnection::Init(const BluezEndpoint & aEndpoint)
{
    // populate the service and the characteristics
    GList * objects = nullptr;
    GList * l;

    if (!aEndpoint.mIsCentral)
    {
        mpService = BLUEZ_GATT_SERVICE1(g_object_ref(aEndpoint.mpService));
        mpC1      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(aEndpoint.mpC1));
        mpC2      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(aEndpoint.mpC2));
    }
    else
    {
        objects = g_dbus_object_manager_get_objects(aEndpoint.mpObjMgr);

        for (l = objects; l != nullptr; l = l->next)
        {
            BluezObject * object        = BLUEZ_OBJECT(l->data);
            BluezGattService1 * service = bluez_object_get_gatt_service1(object);

            if (service != nullptr)
            {
                if ((BluezIsServiceOnDevice(service, mpDevice)) == TRUE &&
                    (strcmp(bluez_gatt_service1_get_uuid(service), CHIP_BLE_UUID_SERVICE_STRING) == 0))
                {
                    mpService = service;
                    break;
                }
                g_object_unref(service);
            }
        }

        VerifyOrExit(mpService != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL service in %s", __func__));

        for (l = objects; l != nullptr; l = l->next)
        {
            BluezObject * object             = BLUEZ_OBJECT(l->data);
            BluezGattCharacteristic1 * char1 = bluez_object_get_gatt_characteristic1(object);

            if (char1 != nullptr)
            {
                if ((BluezIsCharOnService(char1, mpService) == TRUE) &&
                    (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C1_STRING) == 0))
                {
                    mpC1 = char1;
                }
                else if ((BluezIsCharOnService(char1, mpService) == TRUE) &&
                         (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C2_STRING) == 0))
                {
                    mpC2 = char1;
                }
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
                else if ((BluezIsCharOnService(char1, mpService) == TRUE) &&
                         (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C3_STRING) == 0))
                {
                    mpC3 = char1;
                }
#endif
                else
                {
                    g_object_unref(char1);
                }
                if ((mpC1 != nullptr) && (mpC2 != nullptr))
                {
                    break;
                }
            }
        }

        VerifyOrExit(mpC1 != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL C1 in %s", __func__));
        VerifyOrExit(mpC2 != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL C2 in %s", __func__));
    }

exit:
    if (objects != nullptr)
        g_list_free_full(objects, g_object_unref);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::BluezDisconnect(BluezConnection * conn)
{
    GAutoPtr<GError> error;
    gboolean success;

    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, conn->GetPeerAddress());

    success = bluez_device1_call_disconnect_sync(conn->mpDevice, nullptr, &MakeUniquePointerReceiver(error).Get());
    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", error->message));

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::CloseConnection()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(BluezDisconnect, this);
}

const char * BluezConnection::GetPeerAddress() const
{
    return bluez_device1_get_address(mpDevice);
}

gboolean BluezConnection::WriteHandlerCallback(GIOChannel * aChannel, GIOCondition aCond, BluezConnection * apConn)
{
    uint8_t buf[512 /* characteristic max size per BLE specification */];
    bool isSuccess = false;
    GVariant * newVal;
    ssize_t len;

    VerifyOrExit(!(aCond & G_IO_HUP), ChipLogError(DeviceLayer, "INFO: socket disconnected in %s", __func__));
    VerifyOrExit(!(aCond & (G_IO_ERR | G_IO_NVAL)), ChipLogError(DeviceLayer, "INFO: socket error in %s", __func__));
    VerifyOrExit(aCond == G_IO_IN, ChipLogError(DeviceLayer, "FAIL: error in %s", __func__));

    ChipLogDetail(DeviceLayer, "C1 %s MTU: %d", __func__, apConn->GetMTU());

    len = read(g_io_channel_unix_get_fd(aChannel), buf, sizeof(buf));
    VerifyOrExit(len > 0, ChipLogError(DeviceLayer, "FAIL: short read in %s (%zd)", __func__, len));

    // Casting len to size_t is safe, since we ensured that it's not negative.
    newVal = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, buf, static_cast<size_t>(len), sizeof(uint8_t));

    bluez_gatt_characteristic1_set_value(apConn->mpC1, newVal);
    BLEManagerImpl::HandleRXCharWrite(apConn, buf, static_cast<size_t>(len));
    isSuccess = true;

exit:
    return isSuccess ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

void BluezConnection::SetupWriteHandler(int aSocketFd)
{
    auto channel = g_io_channel_unix_new(aSocketFd);
    g_io_channel_set_encoding(channel, nullptr, nullptr);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_channel_set_buffered(channel, FALSE);

    auto watchSource = g_io_create_watch(channel, static_cast<GIOCondition>(G_IO_HUP | G_IO_IN | G_IO_ERR | G_IO_NVAL));
    g_source_set_callback(watchSource, G_SOURCE_FUNC(WriteHandlerCallback), this, nullptr);

    mC1Channel.mChannel.reset(channel);
    mC1Channel.mWatchSource.reset(watchSource);

    PlatformMgrImpl().GLibMatterContextAttachSource(watchSource);
}

gboolean BluezConnection::NotifyHandlerCallback(GIOChannel *, GIOCondition, BluezConnection *)
{
    return G_SOURCE_REMOVE;
}

void BluezConnection::SetupNotifyHandler(int aSocketFd, bool aAdditionalAdvertising)
{
    auto channel = g_io_channel_unix_new(aSocketFd);
    g_io_channel_set_encoding(channel, nullptr, nullptr);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_channel_set_buffered(channel, FALSE);

    auto watchSource = g_io_create_watch(channel, static_cast<GIOCondition>(G_IO_HUP | G_IO_ERR | G_IO_NVAL));
    g_source_set_callback(watchSource, G_SOURCE_FUNC(NotifyHandlerCallback), this, nullptr);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    if (aAdditionalAdvertising)
    {
        mC3Channel.mChannel.reset(channel);
        mC3Channel.mWatchSource.reset(watchSource);
    }
    else
#endif
    {
        mC2Channel.mChannel.reset(channel);
        mC2Channel.mWatchSource.reset(watchSource);
    }

    PlatformMgrImpl().GLibMatterContextAttachSource(watchSource);
}

// SendIndication callbacks

CHIP_ERROR BluezConnection::SendIndicationImpl(ConnectionDataBundle * data)
{
    GAutoPtr<GError> error;
    size_t len, written;

    if (bluez_gatt_characteristic1_get_notify_acquired(data->mConn.mpC2) == TRUE)
    {
        auto * buf = static_cast<const char *>(g_variant_get_fixed_array(data->mData.get(), &len, sizeof(uint8_t)));
        VerifyOrExit(len <= static_cast<size_t>(std::numeric_limits<gssize>::max()),
                     ChipLogError(DeviceLayer, "FAIL: buffer too large in %s", __func__));
        auto status = g_io_channel_write_chars(data->mConn.mC2Channel.mChannel.get(), buf, static_cast<gssize>(len), &written,
                                               &MakeUniquePointerReceiver(error).Get());
        VerifyOrExit(status == G_IO_STATUS_NORMAL, ChipLogError(DeviceLayer, "FAIL: C2 Indicate: %s", error->message));
    }
    else
    {
        bluez_gatt_characteristic1_set_value(data->mConn.mpC2, data->mData.release());
    }

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::SendIndication(chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    VerifyOrReturnError(mpC2 != nullptr, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));

    ConnectionDataBundle bundle(*this, apBuf);
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendIndicationImpl, &bundle);
}

// SendWriteRequest callbacks

void BluezConnection::SendWriteRequestDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c1 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c1, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: SendWriteRequest : %s", error->message));
    BLEManagerImpl::HandleWriteComplete(static_cast<BluezConnection *>(apConnection));
}

CHIP_ERROR BluezConnection::SendWriteRequestImpl(ConnectionDataBundle * data)
{
    GVariantBuilder optionsBuilder;

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add(&optionsBuilder, "{sv}", "type", g_variant_new_string("request"));
    auto options = g_variant_builder_end(&optionsBuilder);

    bluez_gatt_characteristic1_call_write_value(data->mConn.mpC1, data->mData.release(), options, nullptr, SendWriteRequestDone,
                                                const_cast<BluezConnection *>(&data->mConn));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::SendWriteRequest(chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    VerifyOrReturnError(mpC1 != nullptr, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "C1 is NULL in %s", __func__));

    ConnectionDataBundle bundle(*this, apBuf);
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, &bundle);
}

// SubscribeCharacteristic callbacks

void BluezConnection::OnCharacteristicChanged(GDBusProxy * aInterface, GVariant * aChangedProperties,
                                              const gchar * const * aInvalidatedProps, gpointer apConnection)
{
    GAutoPtr<GVariant> dataValue(g_variant_lookup_value(aChangedProperties, "Value", G_VARIANT_TYPE_BYTESTRING));
    VerifyOrReturn(dataValue != nullptr);

    size_t bufferLen;
    auto buffer = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    VerifyOrReturn(buffer != nullptr, ChipLogError(DeviceLayer, "Characteristic value has unexpected type"));

    BLEManagerImpl::HandleTXCharChanged(static_cast<BluezConnection *>(apConnection), static_cast<const uint8_t *>(buffer),
                                        bufferLen);
}

void BluezConnection::SubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: SubscribeCharacteristic : %s", error->message));

    BLEManagerImpl::HandleSubscribeOpComplete(static_cast<BluezConnection *>(apConnection), true);
}

CHIP_ERROR BluezConnection::SubscribeCharacteristicImpl(BluezConnection * connection)
{
    BluezGattCharacteristic1 * c2 = nullptr;
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));
    c2 = BLUEZ_GATT_CHARACTERISTIC1(connection->mpC2);

    // Get notifications on the TX characteristic change (e.g. indication is received)
    g_signal_connect(c2, "g-properties-changed", G_CALLBACK(OnCharacteristicChanged), connection);
    bluez_gatt_characteristic1_call_start_notify(connection->mpC2, nullptr, SubscribeCharacteristicDone, connection);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::SubscribeCharacteristic()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SubscribeCharacteristicImpl, this);
}

// UnsubscribeCharacteristic callbacks

void BluezConnection::UnsubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: UnsubscribeCharacteristic : %s", error->message));

    // Stop listening to the TX characteristic changes
    g_signal_handlers_disconnect_by_data(c2, apConnection);
    BLEManagerImpl::HandleSubscribeOpComplete(static_cast<BluezConnection *>(apConnection), false);
}

CHIP_ERROR BluezConnection::UnsubscribeCharacteristicImpl(BluezConnection * connection)
{
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));

    bluez_gatt_characteristic1_call_stop_notify(connection->mpC2, nullptr, UnsubscribeCharacteristicDone, connection);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::UnsubscribeCharacteristic()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(UnsubscribeCharacteristicImpl, this);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
