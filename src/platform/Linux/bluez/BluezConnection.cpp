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

static CHIP_ERROR BluezC2Indicate(BluezConnection::ConnectionDataBundle * closure)
{
    BluezConnection * conn = nullptr;
    GAutoPtr<GError> error;
    GIOStatus status;
    const char * buf;
    size_t len, written;

    VerifyOrExit(closure != nullptr, ChipLogError(DeviceLayer, "ConnectionDataBundle is NULL in %s", __func__));

    conn = closure->mpConn;
    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(conn->mpC2 != nullptr, ChipLogError(DeviceLayer, "FAIL: C2 Indicate: %s", "NULL C2"));

    if (bluez_gatt_characteristic1_get_notify_acquired(conn->mpC2) == TRUE)
    {
        buf = (char *) g_variant_get_fixed_array(closure->mpVal, &len, sizeof(uint8_t));
        VerifyOrExit(len <= static_cast<size_t>(std::numeric_limits<gssize>::max()),
                     ChipLogError(DeviceLayer, "FAIL: buffer too large in %s", __func__));
        status = g_io_channel_write_chars(conn->mC2Channel.mpChannel, buf, static_cast<gssize>(len), &written,
                                          &MakeUniquePointerReceiver(error).Get());
        g_variant_unref(closure->mpVal);
        closure->mpVal = nullptr;

        VerifyOrExit(status == G_IO_STATUS_NORMAL, ChipLogError(DeviceLayer, "FAIL: C2 Indicate: %s", error->message));
    }
    else
    {
        bluez_gatt_characteristic1_set_value(conn->mpC2, closure->mpVal);
        closure->mpVal = nullptr;
    }

exit:
    if (closure != nullptr)
    {
        g_free(closure);
    }

    return CHIP_NO_ERROR;
}

static BluezConnection::ConnectionDataBundle * MakeConnectionDataBundle(BLE_CONNECTION_OBJECT apConn,
                                                                        const chip::System::PacketBufferHandle & apBuf)
{
    auto * bundle  = g_new(BluezConnection::ConnectionDataBundle, 1);
    bundle->mpConn = static_cast<BluezConnection *>(apConn);
    bundle->mpVal =
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, apBuf->Start(), apBuf->DataLength() * sizeof(uint8_t), sizeof(uint8_t));
    return bundle;
}

CHIP_ERROR SendBluezIndication(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    return PlatformMgrImpl().GLibMatterContextInvokeSync(BluezC2Indicate, MakeConnectionDataBundle(apConn, apBuf));
}

static CHIP_ERROR BluezDisconnect(BluezConnection * conn)
{
    GAutoPtr<GError> error;
    gboolean success;

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "conn is NULL in %s", __func__));
    VerifyOrExit(conn->mpDevice != nullptr, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", "NULL Device"));

    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, bluez_device1_get_address(conn->mpDevice));

    success = bluez_device1_call_disconnect_sync(conn->mpDevice, nullptr, &MakeUniquePointerReceiver(error).Get());
    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", error->message));

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR CloseBluezConnection(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(BluezDisconnect, static_cast<BluezConnection *>(apConn));
}

// BluezSendWriteRequest callbacks

static void SendWriteRequestDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c1 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c1, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezSendWriteRequest : %s", error->message));
    BLEManagerImpl::HandleWriteComplete(static_cast<BLE_CONNECTION_OBJECT>(apConnection));
}

static CHIP_ERROR SendWriteRequestImpl(BluezConnection::ConnectionDataBundle * data)
{
    GVariant * options = nullptr;
    GVariantBuilder optionsBuilder;

    VerifyOrExit(data != nullptr, ChipLogError(DeviceLayer, "ConnectionDataBundle is NULL in %s", __func__));
    VerifyOrExit(data->mpConn != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(data->mpConn->mpC1 != nullptr, ChipLogError(DeviceLayer, "C1 is NULL in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add(&optionsBuilder, "{sv}", "type", g_variant_new_string("request"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_gatt_characteristic1_call_write_value(data->mpConn->mpC1, data->mpVal, options, nullptr, SendWriteRequestDone,
                                                data->mpConn);

exit:
    g_free(data);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, MakeConnectionDataBundle(apConn, apBuf));
}

// BluezSubscribeCharacteristic callbacks

static void OnCharacteristicChanged(GDBusProxy * aInterface, GVariant * aChangedProperties, const gchar * const * aInvalidatedProps,
                                    gpointer apConnection)
{
    BLE_CONNECTION_OBJECT connection = static_cast<BLE_CONNECTION_OBJECT>(apConnection);
    GAutoPtr<GVariant> dataValue(g_variant_lookup_value(aChangedProperties, "Value", G_VARIANT_TYPE_BYTESTRING));
    VerifyOrReturn(dataValue != nullptr);

    size_t bufferLen;
    auto buffer = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    VerifyOrReturn(buffer != nullptr, ChipLogError(DeviceLayer, "Characteristic value has unexpected type"));

    BLEManagerImpl::HandleTXCharChanged(connection, static_cast<const uint8_t *>(buffer), bufferLen);
}

static void SubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezSubscribeCharacteristic : %s", error->message));

    BLEManagerImpl::HandleSubscribeOpComplete(static_cast<BLE_CONNECTION_OBJECT>(apConnection), true);
}

static CHIP_ERROR SubscribeCharacteristicImpl(BluezConnection * connection)
{
    BluezGattCharacteristic1 * c2 = nullptr;
    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));
    c2 = BLUEZ_GATT_CHARACTERISTIC1(connection->mpC2);

    // Get notifications on the TX characteristic change (e.g. indication is received)
    g_signal_connect(c2, "g-properties-changed", G_CALLBACK(OnCharacteristicChanged), connection);
    bluez_gatt_characteristic1_call_start_notify(connection->mpC2, nullptr, SubscribeCharacteristicDone, connection);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezSubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SubscribeCharacteristicImpl, static_cast<BluezConnection *>(apConn));
}

// BluezUnsubscribeCharacteristic callbacks

static void UnsubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezUnsubscribeCharacteristic : %s", error->message));

    // Stop listening to the TX characteristic changes
    g_signal_handlers_disconnect_by_data(c2, apConnection);
    BLEManagerImpl::HandleSubscribeOpComplete(static_cast<BLE_CONNECTION_OBJECT>(apConnection), false);
}

static CHIP_ERROR UnsubscribeCharacteristicImpl(BluezConnection * connection)
{
    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));

    bluez_gatt_characteristic1_call_stop_notify(connection->mpC2, nullptr, UnsubscribeCharacteristicDone, connection);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(UnsubscribeCharacteristicImpl, static_cast<BluezConnection *>(apConn));
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
