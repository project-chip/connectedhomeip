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

BluezConnection::BluezConnection(BluezEndpoint * apEndpoint, BluezDevice1 * apDevice) :
    mpEndpoint(apEndpoint), mpDevice(BLUEZ_DEVICE1(g_object_ref(apDevice)))
{
    mpPeerAddress = g_strdup(bluez_device1_get_address(apDevice));
    Init();
}

BluezConnection::~BluezConnection()
{
    if (mpDevice)
        g_object_unref(mpDevice);
    if (mpService)
        g_object_unref(mpService);
    if (mpC1)
        g_object_unref(mpC1);
    if (mpC2)
        g_object_unref(mpC2);
    if (mpPeerAddress)
        g_free(mpPeerAddress);
    if (mC1Channel.mWatchSource)
    {
        g_source_destroy(mC1Channel.mWatchSource);
        g_source_unref(mC1Channel.mWatchSource);
    }
    if (mC1Channel.mpChannel)
        g_io_channel_unref(mC1Channel.mpChannel);
    if (mC2Channel.mWatchSource)
    {
        g_source_destroy(mC2Channel.mWatchSource);
        g_source_unref(mC2Channel.mWatchSource);
    }
    if (mC2Channel.mpChannel)
        g_io_channel_unref(mC2Channel.mpChannel);
}

BluezConnection::ConnectionDataBundle::ConnectionDataBundle(BluezConnection * apConn,
                                                            const chip::System::PacketBufferHandle & apBuf)
{
    mpConn = apConn;
    mData.reset(g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, apBuf->Start(), apBuf->DataLength(), sizeof(uint8_t)));
}

CHIP_ERROR BluezConnection::Init()
{
    // populate the service and the characteristics
    GList * objects = nullptr;
    GList * l;

    VerifyOrExit(mpEndpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    if (!mpEndpoint->mIsCentral)
    {
        mpService = BLUEZ_GATT_SERVICE1(g_object_ref(mpEndpoint->mpService));
        mpC1      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(mpEndpoint->mpC1));
        mpC2      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(mpEndpoint->mpC2));
    }
    else
    {
        objects = g_dbus_object_manager_get_objects(mpEndpoint->mpObjMgr);

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
                else if ((BluezIsCharOnService(char1, mpService) == TRUE) &&
                         (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C3_STRING) == 0))
                {
                    mpC3 = char1;
                }
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

CHIP_ERROR BluezConnection::BluezC2Indicate(ConnectionDataBundle * data)
{
    BluezConnection * conn = nullptr;
    GAutoPtr<GError> error;
    GIOStatus status;
    size_t len, written;

    VerifyOrExit((conn = data->mpConn) != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(conn->mpC2 != nullptr, ChipLogError(DeviceLayer, "FAIL: C2 Indicate: %s", "NULL C2"));

    if (bluez_gatt_characteristic1_get_notify_acquired(conn->mpC2) == TRUE)
    {
        auto * buf = static_cast<const char *>(g_variant_get_fixed_array(data->mData.get(), &len, sizeof(uint8_t)));
        VerifyOrExit(len <= static_cast<size_t>(std::numeric_limits<gssize>::max()),
                     ChipLogError(DeviceLayer, "FAIL: buffer too large in %s", __func__));
        status = g_io_channel_write_chars(conn->mC2Channel.mpChannel, buf, static_cast<gssize>(len), &written,
                                          &MakeUniquePointerReceiver(error).Get());
        VerifyOrExit(status == G_IO_STATUS_NORMAL, ChipLogError(DeviceLayer, "FAIL: C2 Indicate: %s", error->message));
    }
    else
    {
        bluez_gatt_characteristic1_set_value(conn->mpC2, data->mData.release());
    }

exit:
    Platform::Delete(data);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::SendBluezIndication(chip::System::PacketBufferHandle apBuf)
{
    ConnectionDataBundle * bundle;
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    VerifyOrReturnError((bundle = Platform::New<ConnectionDataBundle>(this, apBuf)) != nullptr, CHIP_ERROR_NO_MEMORY);
    return PlatformMgrImpl().GLibMatterContextInvokeSync(BluezC2Indicate, bundle);
}

CHIP_ERROR BluezConnection::BluezDisconnect(BluezConnection * conn)
{
    GAutoPtr<GError> error;
    gboolean success;

    VerifyOrExit(conn->mpDevice != nullptr, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", "NULL Device"));

    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, bluez_device1_get_address(conn->mpDevice));

    success = bluez_device1_call_disconnect_sync(conn->mpDevice, nullptr, &MakeUniquePointerReceiver(error).Get());
    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", error->message));

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::CloseBluezConnection()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(BluezDisconnect, this);
}

// BluezSendWriteRequest callbacks

void BluezConnection::SendWriteRequestDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c1 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c1, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezSendWriteRequest : %s", error->message));
    BLEManagerImpl::HandleWriteComplete(static_cast<BluezConnection *>(apConnection));
}

CHIP_ERROR BluezConnection::SendWriteRequestImpl(ConnectionDataBundle * data)
{
    GVariant * options = nullptr;
    GVariantBuilder optionsBuilder;

    VerifyOrExit(data->mpConn->mpC1 != nullptr, ChipLogError(DeviceLayer, "C1 is NULL in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add(&optionsBuilder, "{sv}", "type", g_variant_new_string("request"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_gatt_characteristic1_call_write_value(data->mpConn->mpC1, data->mData.release(), options, nullptr, SendWriteRequestDone,
                                                data->mpConn);

exit:
    Platform::Delete(data);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezConnection::BluezSendWriteRequest(chip::System::PacketBufferHandle apBuf)
{
    ConnectionDataBundle * bundle;
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    VerifyOrReturnError((bundle = Platform::New<ConnectionDataBundle>(this, apBuf)) != nullptr, CHIP_ERROR_NO_MEMORY);
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, bundle);
}

// BluezSubscribeCharacteristic callbacks

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

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezSubscribeCharacteristic : %s", error->message));

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

CHIP_ERROR BluezConnection::BluezSubscribeCharacteristic()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SubscribeCharacteristicImpl, this);
}

// BluezUnsubscribeCharacteristic callbacks

void BluezConnection::UnsubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezUnsubscribeCharacteristic : %s", error->message));

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

CHIP_ERROR BluezConnection::BluezUnsubscribeCharacteristic()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(UnsubscribeCharacteristicImpl, this);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
