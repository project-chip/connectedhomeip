/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/*
 *  Copyright (c) 2016-2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *    @file
 *          Provides Bluez dbus implementation for BLE
 */
#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/Protocols.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <cassert>
#include <errno.h>
#include <gio/gunixfdlist.h>
#include <limits>
#include <stdarg.h>
#include <strings.h>
#include <unistd.h>
#include <utility>

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/Linux/BLEManagerImpl.h>
#include <system/TLVPacketBufferBackingStore.h>

#include "BluezObjectIterator.h"
#include "BluezObjectList.h"
#include "Helper.h"

using namespace ::nl;
using namespace chip::Protocols;
using chip::Platform::CopyString;

namespace chip {
namespace DeviceLayer {
namespace Internal {

constexpr uint16_t kMaxConnectRetries = 4;

static BluezConnection * GetBluezConnectionViaDevice(BluezEndpoint * apEndpoint);

namespace {

class BluezEndpointObjectList : public BluezObjectList
{
public:
    explicit BluezEndpointObjectList(BluezEndpoint * apEndpoint)
    {
        VerifyOrReturn(apEndpoint != nullptr, ChipLogError(DeviceLayer, "apEndpoint is NULL in %s", __func__));
        Initialize(apEndpoint->mpObjMgr);
    }
};

} // namespace

static gboolean BluezAdvertisingRelease(BluezLEAdvertisement1 * aAdv, GDBusMethodInvocation * aInvocation, gpointer apClosure)
{
    bool isSuccess           = false;
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(aAdv != nullptr, ChipLogError(DeviceLayer, "BluezLEAdvertisement1 is NULL in %s", __func__));
    ChipLogDetail(DeviceLayer, "Release adv object in %s", __func__);

    g_dbus_object_manager_server_unexport(endpoint->mpRoot, endpoint->mpAdvPath);
    endpoint->mIsAdvertising = false;
    isSuccess                = true;
exit:

    return isSuccess ? TRUE : FALSE;
}

static BluezLEAdvertisement1 * BluezAdvertisingCreate(BluezEndpoint * apEndpoint)
{
    BluezLEAdvertisement1 * adv = nullptr;
    BluezObjectSkeleton * object;
    GVariant * serviceData;
    GVariant * serviceUUID;
    gchar * localName;
    GVariantBuilder serviceDataBuilder;
    GVariantBuilder serviceUUIDsBuilder;
    char * debugStr;

    VerifyOrExit(apEndpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    if (apEndpoint->mpAdvPath == nullptr)
        apEndpoint->mpAdvPath = g_strdup_printf("%s/advertising", apEndpoint->mpRootPath);

    ChipLogDetail(DeviceLayer, "Create adv object at %s", apEndpoint->mpAdvPath);
    object = bluez_object_skeleton_new(apEndpoint->mpAdvPath);

    adv = bluez_leadvertisement1_skeleton_new();

    g_variant_builder_init(&serviceDataBuilder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_init(&serviceUUIDsBuilder, G_VARIANT_TYPE("as"));

    g_variant_builder_add(&serviceDataBuilder, "{sv}", apEndpoint->mpAdvertisingUUID,
                          g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, &apEndpoint->mDeviceIdInfo,
                                                    sizeof(apEndpoint->mDeviceIdInfo), sizeof(uint8_t)));
    g_variant_builder_add(&serviceUUIDsBuilder, "s", apEndpoint->mpAdvertisingUUID);

    if (apEndpoint->mpAdapterName != nullptr)
        localName = g_strdup_printf("%s", apEndpoint->mpAdapterName);
    else
        localName = g_strdup_printf("%s%04x", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, getpid() & 0xffff);

    serviceData = g_variant_builder_end(&serviceDataBuilder);
    serviceUUID = g_variant_builder_end(&serviceUUIDsBuilder);

    debugStr = g_variant_print(serviceData, TRUE);
    ChipLogDetail(DeviceLayer, "SET service data to %s", debugStr);
    g_free(debugStr);

    bluez_leadvertisement1_set_type_(adv, (apEndpoint->mType & BLUEZ_ADV_TYPE_CONNECTABLE) ? "peripheral" : "broadcast");
    // empty manufacturer data
    // empty solicit UUIDs
    bluez_leadvertisement1_set_service_data(adv, serviceData);
    // empty data

    // Setting "Discoverable" to False on the adapter and to True on the advertisement convinces
    // Bluez to set "BR/EDR Not Supported" flag. Bluez doesn't provide API to do that explicitly
    // and the flag is necessary to force using LE transport.
    bluez_leadvertisement1_set_discoverable(adv, (apEndpoint->mType & BLUEZ_ADV_TYPE_SCANNABLE) ? TRUE : FALSE);
    if (apEndpoint->mType & BLUEZ_ADV_TYPE_SCANNABLE)
        bluez_leadvertisement1_set_discoverable_timeout(adv, UINT16_MAX);

    // advertising name corresponding to the PID and object path, for debug purposes
    bluez_leadvertisement1_set_local_name(adv, localName);
    bluez_leadvertisement1_set_service_uuids(adv, serviceUUID);

    // 0xffff means no appearance
    bluez_leadvertisement1_set_appearance(adv, 0xffff);

    bluez_leadvertisement1_set_duration(adv, apEndpoint->mDuration);
    // empty duration, we don't have a clear notion what it would mean to timeslice between toble and anyone else
    bluez_leadvertisement1_set_timeout(adv, 0);
    // empty secondary channel for now

    bluez_object_skeleton_set_leadvertisement1(object, adv);
    g_signal_connect(adv, "handle-release", G_CALLBACK(BluezAdvertisingRelease), apEndpoint);

    g_dbus_object_manager_server_export(apEndpoint->mpRoot, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(true, nullptr);

exit:
    return adv;
}

static void BluezAdvStartDone(GObject * aObject, GAsyncResult * aResult, gpointer apClosure)
{
    BluezLEAdvertisingManager1 * advMgr = BLUEZ_LEADVERTISING_MANAGER1(aObject);
    GError * error                      = nullptr;
    BluezEndpoint * endpoint            = static_cast<BluezEndpoint *>(apClosure);
    gboolean success                    = FALSE;

    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    success = bluez_leadvertising_manager1_call_register_advertisement_finish(advMgr, aResult, &error);
    if (success == FALSE)
    {
        g_dbus_object_manager_server_unexport(endpoint->mpRoot, endpoint->mpAdvPath);
    }
    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: RegisterAdvertisement : %s", error->message));

    endpoint->mIsAdvertising = true;

    ChipLogDetail(DeviceLayer, "RegisterAdvertisement complete");

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(success == TRUE, nullptr);
    if (error != nullptr)
        g_error_free(error);
}

static void BluezAdvStopDone(GObject * aObject, GAsyncResult * aResult, gpointer apClosure)
{
    BluezLEAdvertisingManager1 * advMgr = BLUEZ_LEADVERTISING_MANAGER1(aObject);
    BluezEndpoint * endpoint            = static_cast<BluezEndpoint *>(apClosure);
    GError * error                      = nullptr;
    gboolean success                    = FALSE;

    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    success = bluez_leadvertising_manager1_call_unregister_advertisement_finish(advMgr, aResult, &error);

    if (success == FALSE)
    {
        g_dbus_object_manager_server_unexport(endpoint->mpRoot, endpoint->mpAdvPath);
    }
    else
    {
        endpoint->mIsAdvertising = false;
    }

    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: UnregisterAdvertisement : %s", error->message));

    ChipLogDetail(DeviceLayer, "UnregisterAdvertisement complete");

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(success == TRUE, nullptr);
    if (error != nullptr)
        g_error_free(error);
}

static gboolean BluezAdvSetup(BluezEndpoint * endpoint)
{
    BluezLEAdvertisement1 * adv;

    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(endpoint->mIsAdvertising == FALSE, ChipLogError(DeviceLayer, "FAIL: Advertising already enabled in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adv = BluezAdvertisingCreate(endpoint);
    VerifyOrExit(adv != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adv in %s", __func__));

exit:
    return G_SOURCE_REMOVE;
}

static gboolean BluezAdvStart(BluezEndpoint * endpoint)
{
    GDBusObject * adapter;
    BluezLEAdvertisingManager1 * advMgr = nullptr;
    GVariantBuilder optionsBuilder;
    GVariant * options;

    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(!endpoint->mIsAdvertising,
                 ChipLogError(DeviceLayer, "FAIL: Advertising has already been enabled in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(endpoint->mpAdapter));
    VerifyOrExit(adapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    advMgr = bluez_object_get_leadvertising_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(advMgr != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL advMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_leadvertising_manager1_call_register_advertisement(advMgr, endpoint->mpAdvPath, options, nullptr, BluezAdvStartDone,
                                                             endpoint);

exit:
    return G_SOURCE_REMOVE;
}

static gboolean BluezAdvStop(BluezEndpoint * endpoint)
{
    GDBusObject * adapter;
    BluezLEAdvertisingManager1 * advMgr = nullptr;

    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(endpoint->mIsAdvertising,
                 ChipLogError(DeviceLayer, "FAIL: Advertising has already been disabled in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(endpoint->mpAdapter));
    VerifyOrExit(adapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    advMgr = bluez_object_get_leadvertising_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(advMgr != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL advMgr in %s", __func__));

    bluez_leadvertising_manager1_call_unregister_advertisement(advMgr, endpoint->mpAdvPath, nullptr, BluezAdvStopDone, endpoint);

exit:
    return G_SOURCE_REMOVE;
}

static gboolean BluezCharacteristicReadValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                             GVariant * aOptions)
{
    GVariant * val;
    ChipLogDetail(DeviceLayer, "Received BluezCharacteristicReadValue");
    val = bluez_gatt_characteristic1_get_value(aChar);
    bluez_gatt_characteristic1_complete_read_value(aChar, aInvocation, val);
    return TRUE;
}

#if CHIP_BLUEZ_CHAR_WRITE_VALUE
static gboolean BluezCharacteristicWriteValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                              GVariant * aValue, GVariant * aOptions, gpointer apEndpoint)
{
    const uint8_t * tmpBuf;
    uint8_t * buf;
    size_t len;
    bool isSuccess         = false;
    BluezConnection * conn = NULL;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);
    VerifyOrExit(endpoint != NULL, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    VerifyOrExit(aValue != NULL, ChipLogError(DeviceLayer, "aValue is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != NULL,
                 g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No CHIP Bluez connection"));

    bluez_gatt_characteristic1_set_value(aChar, g_variant_ref(aValue));

    tmpBuf = (uint8_t *) (g_variant_get_fixed_array(aValue, &len, sizeof(uint8_t)));
    buf    = (uint8_t *) (g_memdup(tmpBuf, len));

    BLEManagerImpl::HandleRXCharWrite(conn, buf, len);
    bluez_gatt_characteristic1_complete_write_value(aChar, aInvocation);
    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}
#endif

static gboolean BluezCharacteristicWriteValueError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                   GVariant * aValue, GVariant * aOptions, gpointer apClosure)
{
    ChipLogDetail(DeviceLayer, "BluezCharacteristicWriteValueError");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "Write for characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicWriteFD(GIOChannel * aChannel, GIOCondition aCond, gpointer apEndpoint)
{
    GVariant * newVal;
    gchar * buf;
    ssize_t len;
    int fd;
    bool isSuccess = false;

    BluezConnection * conn = static_cast<BluezConnection *>(apEndpoint);

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "No CHIP Bluez connection in %s", __func__));

    VerifyOrExit(!(aCond & G_IO_HUP), ChipLogError(DeviceLayer, "INFO: socket disconnected in %s", __func__));
    VerifyOrExit(!(aCond & (G_IO_ERR | G_IO_NVAL)), ChipLogError(DeviceLayer, "INFO: socket error in %s", __func__));
    VerifyOrExit(aCond == G_IO_IN, ChipLogError(DeviceLayer, "FAIL: error in %s", __func__));

    ChipLogDetail(DeviceLayer, "c1 %s mtu, %d", __func__, conn->mMtu);

    buf = static_cast<gchar *>(g_malloc(conn->mMtu));
    fd  = g_io_channel_unix_get_fd(aChannel);

    len = read(fd, buf, conn->mMtu);

    VerifyOrExit(len > 0, ChipLogError(DeviceLayer, "FAIL: short read in %s (%zd)", __func__, len));

    // Casting len to size_t is safe, since we ensured that it's not negative.
    newVal = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, buf, static_cast<size_t>(len), sizeof(uint8_t));

    bluez_gatt_characteristic1_set_value(conn->mpC1, newVal);
    BLEManagerImpl::HandleRXCharWrite(conn, reinterpret_cast<uint8_t *>(buf), static_cast<size_t>(len));
    isSuccess = true;

exit:
    if (!isSuccess && (conn != nullptr))
    {
        // Returning G_SOURCE_REMOVE from the source callback removes the source object
        // from the context. Unset self source ID tag, so we will not call g_source_remove()
        // in BluezOTConnectionDestroy() on already removed source.
        //
        // TODO: Investigate whether there is a batter way to handle this.
        conn->mC1Channel.mWatch = 0;
    }

    return isSuccess ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

static void Bluez_gatt_characteristic1_complete_acquire_write_with_fd(GDBusMethodInvocation * invocation, int fd, guint16 mtu)
{
    GUnixFDList * fd_list = g_unix_fd_list_new();
    int index;

    index = g_unix_fd_list_append(fd_list, fd, nullptr);

    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, g_variant_new("(@hq)", g_variant_new_handle(index), mtu),
                                                            fd_list);
}

static gboolean bluezCharacteristicDestroyFD(GIOChannel * aChannel, GIOCondition aCond, gpointer apClosure)
{
    BluezConnection * conn = static_cast<BluezConnection *>(apClosure);
    // Returning G_SOURCE_REMOVE from the source callback removes the source object
    // from the context. Unset self source ID tag, so we will not call g_source_remove()
    // in BluezOTConnectionDestroy() on already removed source.
    //
    // TODO: Investigate whether there is a batter way to handle this.
    conn->mC2Channel.mWatch = 0;
    return G_SOURCE_REMOVE;
}

static gboolean BluezCharacteristicAcquireWrite(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                GVariant * aOptions, gpointer apEndpoint)
{
    int fds[2] = { -1, -1 };
    GIOChannel * channel;
#if CHIP_ERROR_LOGGING
    char * errStr;
#endif // CHIP_ERROR_LOGGING
    GVariantDict options;
    bool isSuccess         = false;
    BluezConnection * conn = nullptr;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != nullptr,
                 g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    ChipLogDetail(DeviceLayer, "BluezCharacteristicAcquireWrite is called, conn: %p", conn);

    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) < 0)
    {
#if CHIP_ERROR_LOGGING
        errStr = strerror(errno);
#endif // CHIP_ERROR_LOGGING
        ChipLogError(DeviceLayer, "FAIL: socketpair: %s in %s", errStr, __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "FD creation failed");
        goto exit;
    }

    g_variant_dict_init(&options, aOptions);
    if (g_variant_dict_contains(&options, "mtu") == TRUE)
    {
        GVariant * v = g_variant_dict_lookup_value(&options, "mtu", G_VARIANT_TYPE_UINT16);
        conn->mMtu   = g_variant_get_uint16(v);
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: no MTU in options in %s", __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.InvalidArguments", "MTU negotiation failed");
        goto exit;
    }

    channel = g_io_channel_unix_new(fds[0]);
    g_io_channel_set_encoding(channel, nullptr, nullptr);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_channel_set_buffered(channel, FALSE);

    conn->mC1Channel.mpChannel = channel;
    conn->mC1Channel.mWatch    = g_io_add_watch(channel, static_cast<GIOCondition>(G_IO_HUP | G_IO_IN | G_IO_ERR | G_IO_NVAL),
                                             BluezCharacteristicWriteFD, conn);

    bluez_gatt_characteristic1_set_write_acquired(aChar, TRUE);

    Bluez_gatt_characteristic1_complete_acquire_write_with_fd(aInvocation, fds[1], conn->mMtu);
    close(fds[1]);
    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}

static gboolean BluezCharacteristicAcquireWriteError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                     GVariant * aOptions)
{
    ChipLogDetail(DeviceLayer, "BluezCharacteristicAcquireWriteError is called");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireWrite for characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicAcquireNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                 GVariant * aOptions, gpointer apEndpoint)
{
    int fds[2] = { -1, -1 };
    GIOChannel * channel;
#if CHIP_ERROR_LOGGING
    char * errStr;
#endif // CHIP_ERROR_LOGGING
    GVariantDict options;
    BluezConnection * conn = nullptr;
    bool isSuccess         = false;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != nullptr,
                 g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    g_variant_dict_init(&options, aOptions);
    if ((g_variant_dict_contains(&options, "mtu") == TRUE))
    {
        GVariant * v = g_variant_dict_lookup_value(&options, "mtu", G_VARIANT_TYPE_UINT16);
        conn->mMtu   = g_variant_get_uint16(v);
    }

    if (bluez_gatt_characteristic1_get_notifying(aChar))
    {
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotPermitted", "Already notifying");
    }
    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) < 0)
    {
#if CHIP_ERROR_LOGGING
        errStr = strerror(errno);
#endif // CHIP_ERROR_LOGGING
        ChipLogError(DeviceLayer, "FAIL: socketpair: %s in %s", errStr, __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "FD creation failed");
        goto exit;
    }
    channel = g_io_channel_unix_new(fds[0]);
    g_io_channel_set_encoding(channel, nullptr, nullptr);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_channel_set_buffered(channel, FALSE);
    conn->mC2Channel.mpChannel = channel;
    conn->mC2Channel.mWatch =
        g_io_add_watch_full(channel, G_PRIORITY_DEFAULT_IDLE, static_cast<GIOCondition>(G_IO_HUP | G_IO_ERR | G_IO_NVAL),
                            bluezCharacteristicDestroyFD, conn, nullptr);

    bluez_gatt_characteristic1_set_notify_acquired(aChar, TRUE);

    // same reply as for AcquireWrite
    Bluez_gatt_characteristic1_complete_acquire_write_with_fd(aInvocation, fds[1], conn->mMtu);
    close(fds[1]);

    conn->mIsNotify = true;
    BLEManagerImpl::HandleTXCharCCCDWrite(conn);
    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}

static gboolean BluezCharacteristicAcquireNotifyError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                      GVariant * aOptions)
{
    ChipLogDetail(DeviceLayer, "TRACE: AcquireNotify is called");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireNotify for characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicStartNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                               gpointer apEndpoint)
{
    bool isSuccess         = false;
    BluezConnection * conn = nullptr;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != nullptr,
                 g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    if (bluez_gatt_characteristic1_get_notifying(aChar) == TRUE)
    {
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "Characteristic is already subscribed");
    }
    else
    {
        bluez_gatt_characteristic1_complete_start_notify(aChar, aInvocation);
        bluez_gatt_characteristic1_set_notifying(aChar, TRUE);
        conn->mIsNotify = true;
        BLEManagerImpl::HandleTXCharCCCDWrite(conn);
    }
    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}

static gboolean BluezCharacteristicStartNotifyError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation)
{
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "Subscribing to characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicStopNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                              gpointer apEndpoint)
{
    bool isSuccess         = false;
    BluezConnection * conn = nullptr;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != nullptr,
                 g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    if (bluez_gatt_characteristic1_get_notifying(aChar) == FALSE)
    {
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "Characteristic is already unsubscribed");
    }
    else
    {
        bluez_gatt_characteristic1_complete_start_notify(aChar, aInvocation);
        bluez_gatt_characteristic1_set_notifying(aChar, FALSE);
    }
    conn->mIsNotify = false;

    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}

static gboolean BluezCharacteristicConfirm(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                           gpointer apClosure)
{
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    BluezConnection * conn   = GetBluezConnectionViaDevice(endpoint);

    ChipLogDetail(Ble, "Indication confirmation, %p", conn);
    BLEManagerImpl::HandleTXComplete(conn);

    return TRUE;
}

static gboolean BluezCharacteristicStopNotifyError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation)
{
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed",
                                               "Unsubscribing from characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicConfirmError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation)
{
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "Confirm from characteristic is unsupported");
    return TRUE;
}

static gboolean BluezIsDeviceOnAdapter(BluezDevice1 * aDevice, BluezAdapter1 * aAdapter)
{
    return strcmp(bluez_device1_get_adapter(aDevice), g_dbus_proxy_get_object_path(G_DBUS_PROXY(aAdapter))) == 0 ? TRUE : FALSE;
}

static gboolean BluezIsServiceOnDevice(BluezGattService1 * aService, BluezDevice1 * aDevice)
{
    return strcmp(bluez_gatt_service1_get_device(aService), g_dbus_proxy_get_object_path(G_DBUS_PROXY(aDevice))) == 0 ? TRUE
                                                                                                                      : FALSE;
}

static gboolean BluezIsCharOnService(BluezGattCharacteristic1 * aChar, BluezGattService1 * aService)
{
    ChipLogDetail(DeviceLayer, "Char1 %s", bluez_gatt_characteristic1_get_service(aChar));
    ChipLogDetail(DeviceLayer, "Char1 %s", g_dbus_proxy_get_object_path(G_DBUS_PROXY(aService)));
    return strcmp(bluez_gatt_characteristic1_get_service(aChar), g_dbus_proxy_get_object_path(G_DBUS_PROXY(aService))) == 0 ? TRUE
                                                                                                                            : FALSE;
}

static void BluezConnectionInit(BluezConnection * apConn)
{
    // populate the service and the characteristics
    GList * objects = nullptr;
    GList * l;
    BluezEndpoint * endpoint = nullptr;

    VerifyOrExit(apConn != nullptr, ChipLogError(DeviceLayer, "Bluez connection is NULL in %s", __func__));

    endpoint = apConn->mpEndpoint;
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    if (!endpoint->mIsCentral)
    {
        apConn->mpService = BLUEZ_GATT_SERVICE1(g_object_ref(apConn->mpEndpoint->mpService));
        apConn->mpC1      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(endpoint->mpC1));
        apConn->mpC2      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(endpoint->mpC2));
    }
    else
    {
        objects = g_dbus_object_manager_get_objects(endpoint->mpObjMgr);

        for (l = objects; l != nullptr; l = l->next)
        {
            BluezObject * object        = BLUEZ_OBJECT(l->data);
            BluezGattService1 * service = bluez_object_get_gatt_service1(object);

            if (service != nullptr)
            {
                if ((BluezIsServiceOnDevice(service, apConn->mpDevice)) == TRUE &&
                    (strcmp(bluez_gatt_service1_get_uuid(service), CHIP_BLE_UUID_SERVICE_STRING) == 0))
                {
                    apConn->mpService = service;
                    break;
                }
                g_object_unref(service);
            }
        }

        VerifyOrExit(apConn->mpService != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL service in %s", __func__));

        for (l = objects; l != nullptr; l = l->next)
        {
            BluezObject * object             = BLUEZ_OBJECT(l->data);
            BluezGattCharacteristic1 * char1 = bluez_object_get_gatt_characteristic1(object);

            if (char1 != nullptr)
            {
                if ((BluezIsCharOnService(char1, apConn->mpService) == TRUE) &&
                    (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C1_STRING) == 0))
                {
                    apConn->mpC1 = char1;
                }
                else if ((BluezIsCharOnService(char1, apConn->mpService) == TRUE) &&
                         (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C2_STRING) == 0))
                {
                    apConn->mpC2 = char1;
                }
                else if ((BluezIsCharOnService(char1, apConn->mpService) == TRUE) &&
                         (strcmp(bluez_gatt_characteristic1_get_uuid(char1), CHIP_PLAT_BLE_UUID_C3_STRING) == 0))
                {
                    apConn->mpC3 = char1;
                }
                else
                {
                    g_object_unref(char1);
                }
                if ((apConn->mpC1 != nullptr) && (apConn->mpC2 != nullptr))
                {
                    break;
                }
            }
        }

        VerifyOrExit(apConn->mpC1 != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL C1 in %s", __func__));
        VerifyOrExit(apConn->mpC2 != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL C2 in %s", __func__));
    }

exit:
    if (objects != nullptr)
        g_list_free_full(objects, g_object_unref);
}

static void BluezOTConnectionDestroy(BluezConnection * aConn)
{
    if (aConn)
    {
        if (aConn->mpDevice)
            g_object_unref(aConn->mpDevice);
        if (aConn->mpService)
            g_object_unref(aConn->mpService);
        if (aConn->mpC1)
            g_object_unref(aConn->mpC1);
        if (aConn->mpC2)
            g_object_unref(aConn->mpC2);
        if (aConn->mpPeerAddress)
            g_free(aConn->mpPeerAddress);
        if (aConn->mC1Channel.mWatch > 0)
            g_source_remove(aConn->mC1Channel.mWatch);
        if (aConn->mC1Channel.mpChannel)
            g_io_channel_unref(aConn->mC1Channel.mpChannel);
        if (aConn->mC2Channel.mWatch > 0)
            g_source_remove(aConn->mC2Channel.mWatch);
        if (aConn->mC2Channel.mpChannel)
            g_io_channel_unref(aConn->mC2Channel.mpChannel);

        g_free(aConn);
    }
}

static BluezGattCharacteristic1 * BluezCharacteristicCreate(BluezGattService1 * aService, const char * aCharName,
                                                            const char * aUUID, GDBusObjectManagerServer * aRoot)
{
    char * servicePath = g_strdup(g_dbus_object_get_object_path(g_dbus_interface_get_object(G_DBUS_INTERFACE(aService))));
    char * charPath    = g_strdup_printf("%s/%s", servicePath, aCharName);
    BluezObjectSkeleton * object;
    BluezGattCharacteristic1 * characteristic;

    ChipLogDetail(DeviceLayer, "Create characteristic object at %s", charPath);
    object = bluez_object_skeleton_new(charPath);

    characteristic = bluez_gatt_characteristic1_skeleton_new();
    bluez_gatt_characteristic1_set_uuid(characteristic, aUUID);
    bluez_gatt_characteristic1_set_service(characteristic, servicePath);

    bluez_object_skeleton_set_gatt_characteristic1(object, characteristic);
    g_dbus_object_manager_server_export(aRoot, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    return characteristic;
}

static void BluezPeripheralRegisterAppDone(GObject * aObject, GAsyncResult * aResult, gpointer apClosure)
{
    GError * error              = nullptr;
    BluezGattManager1 * gattMgr = BLUEZ_GATT_MANAGER1(aObject);

    gboolean success = bluez_gatt_manager1_call_register_application_finish(gattMgr, aResult, &error);

    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: RegisterApplication : %s", error->message));

    BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(true, nullptr);
    ChipLogDetail(DeviceLayer, "BluezPeripheralRegisterAppDone done");

exit:
    if (error != nullptr)
    {
        BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(false, nullptr);
        g_error_free(error);
    }
}

gboolean BluezPeripheralRegisterApp(BluezEndpoint * endpoint)
{
    GDBusObject * adapter;
    BluezGattManager1 * gattMgr;
    GVariantBuilder optionsBuilder;
    GVariant * options;

    VerifyOrExit(endpoint->mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(endpoint->mpAdapter));
    VerifyOrExit(adapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    gattMgr = bluez_object_get_gatt_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(gattMgr != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL gattMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_gatt_manager1_call_register_application(gattMgr, endpoint->mpRootPath, options, nullptr, BluezPeripheralRegisterAppDone,
                                                  nullptr);

exit:
    return G_SOURCE_REMOVE;
}

/// Update the table of open BLE connections whevener a new device is spotted or its attributes have changed.
static void UpdateConnectionTable(BluezDevice1 * apDevice, BluezEndpoint & aEndpoint)
{
    const gchar * objectPath     = g_dbus_proxy_get_object_path(G_DBUS_PROXY(apDevice));
    BluezConnection * connection = static_cast<BluezConnection *>(g_hash_table_lookup(aEndpoint.mpConnMap, objectPath));

    if (connection != nullptr && !bluez_device1_get_connected(apDevice))
    {
        ChipLogDetail(DeviceLayer, "Bluez disconnected");
        BLEManagerImpl::CHIPoBluez_ConnectionClosed(connection);
        // TODO: the connection object should be released after BLEManagerImpl finishes cleaning up its resources
        // after the disconnection. Releasing it here doesn't cause any issues, but it's error-prone.
        BluezOTConnectionDestroy(connection);
        g_hash_table_remove(aEndpoint.mpConnMap, objectPath);
        return;
    }

    if (connection == nullptr && !bluez_device1_get_connected(apDevice) && aEndpoint.mIsCentral)
    {
        return;
    }

    if (connection == nullptr && bluez_device1_get_connected(apDevice) &&
        (!aEndpoint.mIsCentral || bluez_device1_get_services_resolved(apDevice)))
    {
        connection                = g_new0(BluezConnection, 1);
        connection->mpPeerAddress = g_strdup(bluez_device1_get_address(apDevice));
        connection->mpDevice      = static_cast<BluezDevice1 *>(g_object_ref(apDevice));
        connection->mpEndpoint    = &aEndpoint;
        BluezConnectionInit(connection);
        aEndpoint.mpPeerDevicePath = g_strdup(objectPath);
        g_hash_table_insert(aEndpoint.mpConnMap, aEndpoint.mpPeerDevicePath, connection);

        ChipLogDetail(DeviceLayer, "New BLE connection %p, device %s, path %s", connection, connection->mpPeerAddress,
                      aEndpoint.mpPeerDevicePath);

        BLEManagerImpl::HandleNewConnection(connection);
    }
}

static void BluezSignalInterfacePropertiesChanged(GDBusObjectManagerClient * aManager, GDBusObjectProxy * aObject,
                                                  GDBusProxy * aInterface, GVariant * aChangedProperties,
                                                  const gchar * const * aInvalidatedProps, gpointer apClosure)
{
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrReturn(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrReturn(endpoint->mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));
    VerifyOrReturn(strcmp(g_dbus_proxy_get_interface_name(aInterface), DEVICE_INTERFACE) == 0, );

    BluezDevice1 * device = BLUEZ_DEVICE1(aInterface);
    VerifyOrReturn(BluezIsDeviceOnAdapter(device, endpoint->mpAdapter));

    UpdateConnectionTable(device, *endpoint);
}

static void BluezHandleNewDevice(BluezDevice1 * device, BluezEndpoint * apEndpoint)
{
    VerifyOrExit(apEndpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    if (apEndpoint->mIsCentral)
    {
        return;
    }

    // We need to handle device connection both this function and BluezSignalInterfacePropertiesChanged
    // When a device is connected for first time, this function will be triggerred.
    // The future connections for the same device will trigger ``Connect'' property change.
    // TODO: Factor common code in the two function.
    BluezConnection * conn;
    VerifyOrExit(bluez_device1_get_connected(device), ChipLogError(DeviceLayer, "FAIL: device is not connected"));

    conn = static_cast<BluezConnection *>(
        g_hash_table_lookup(apEndpoint->mpConnMap, g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))));
    VerifyOrExit(conn == nullptr,
                 ChipLogError(DeviceLayer, "FAIL: connection already tracked: conn: %p new device: %s", conn,
                              g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))));

    conn                = g_new0(BluezConnection, 1);
    conn->mpPeerAddress = g_strdup(bluez_device1_get_address(device));
    conn->mpDevice      = static_cast<BluezDevice1 *>(g_object_ref(device));
    conn->mpEndpoint    = apEndpoint;
    BluezConnectionInit(conn);
    apEndpoint->mpPeerDevicePath = g_strdup(g_dbus_proxy_get_object_path(G_DBUS_PROXY(device)));
    ChipLogDetail(DeviceLayer, "Device %s (Path: %s) Connected", conn->mpPeerAddress, apEndpoint->mpPeerDevicePath);
    g_hash_table_insert(apEndpoint->mpConnMap, g_strdup(g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))), conn);

exit:
    return;
}

static void BluezSignalOnObjectAdded(GDBusObjectManager * aManager, GDBusObject * aObject, BluezEndpoint * endpoint)
{
    // TODO: right now we do not handle addition/removal of adapters
    // Primary focus here is to handle addition of a device
    BluezDevice1 * device = bluez_object_get_device1(BLUEZ_OBJECT(aObject));
    if (device == nullptr)
    {
        return;
    }

    if (BluezIsDeviceOnAdapter(device, endpoint->mpAdapter) == TRUE)
    {
        BluezHandleNewDevice(device, endpoint);
    }

    g_object_unref(device);
}

static void BluezSignalOnObjectRemoved(GDBusObjectManager * aManager, GDBusObject * aObject, gpointer apClosure)
{
    // TODO: for Device1, lookup connection, and call otPlatTobleHandleDisconnected
    // for Adapter1: unclear, crash if this pertains to our adapter? at least null out the endpoint->mpAdapter.
    // for Characteristic1, or GattService -- handle here via calling otPlatTobleHandleDisconnected, or ignore.
}

static BluezGattService1 * BluezServiceCreate(gpointer apClosure)
{
    BluezObjectSkeleton * object;
    BluezGattService1 * service;
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);

    endpoint->mpServicePath = g_strdup_printf("%s/service", endpoint->mpRootPath);
    ChipLogDetail(DeviceLayer, "CREATE service object at %s", endpoint->mpServicePath);
    object = bluez_object_skeleton_new(endpoint->mpServicePath);

    service = bluez_gatt_service1_skeleton_new();
    bluez_gatt_service1_set_uuid(service, "0xFFF6");
    // device is only valid for remote services
    bluez_gatt_service1_set_primary(service, TRUE);

    // includes -- unclear whether required.  Might be filled in later
    bluez_object_skeleton_set_gatt_service1(object, service);
    g_dbus_object_manager_server_export(endpoint->mpRoot, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    return service;
}

static void bluezObjectsSetup(BluezEndpoint * apEndpoint)
{
    GList * objects = nullptr;
    GList * l;
    char * expectedPath = nullptr;

    VerifyOrExit(apEndpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    expectedPath = g_strdup_printf("%s/hci%d", BLUEZ_PATH, apEndpoint->mAdapterId);
    objects      = g_dbus_object_manager_get_objects(apEndpoint->mpObjMgr);

    for (l = objects; l != nullptr && apEndpoint->mpAdapter == nullptr; l = l->next)
    {
        BluezObject * object = BLUEZ_OBJECT(l->data);
        GList * interfaces;
        GList * ll;
        interfaces = g_dbus_object_get_interfaces(G_DBUS_OBJECT(object));

        for (ll = interfaces; ll != nullptr; ll = ll->next)
        {
            if (BLUEZ_IS_ADAPTER1(ll->data))
            { // we found the adapter
                BluezAdapter1 * adapter = BLUEZ_ADAPTER1(ll->data);
                char * addr             = const_cast<char *>(bluez_adapter1_get_address(adapter));
                if (apEndpoint->mpAdapterAddr == nullptr) // no adapter address provided, bind to the hci indicated by nodeid
                {
                    if (strcmp(g_dbus_proxy_get_object_path(G_DBUS_PROXY(adapter)), expectedPath) == 0)
                    {
                        apEndpoint->mpAdapter = static_cast<BluezAdapter1 *>(g_object_ref(adapter));
                    }
                }
                else
                {
                    if (strcmp(apEndpoint->mpAdapterAddr, addr) == 0)
                    {
                        apEndpoint->mpAdapter = static_cast<BluezAdapter1 *>(g_object_ref(adapter));
                    }
                }
            }
        }
        g_list_free_full(interfaces, g_object_unref);
    }
    VerifyOrExit(apEndpoint->mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL apEndpoint->mpAdapter in %s", __func__));
    bluez_adapter1_set_powered(apEndpoint->mpAdapter, TRUE);

    // Setting "Discoverable" to False on the adapter and to True on the advertisement convinces
    // Bluez to set "BR/EDR Not Supported" flag. Bluez doesn't provide API to do that explicitly
    // and the flag is necessary to force using LE transport.
    bluez_adapter1_set_discoverable(apEndpoint->mpAdapter, FALSE);

exit:
    g_list_free_full(objects, g_object_unref);
    g_free(expectedPath);
}

static BluezConnection * GetBluezConnectionViaDevice(BluezEndpoint * apEndpoint)
{
    BluezConnection * retval =
        static_cast<BluezConnection *>(g_hash_table_lookup(apEndpoint->mpConnMap, apEndpoint->mpPeerDevicePath));
    // ChipLogError(DeviceLayer, "acquire connection object %p in (%s)", retval, __func__);
    return retval;
}

#if CHIP_BLUEZ_CENTRAL_SUPPORT
static BluezConnection * BluezCharacteristicGetBluezConnection(BluezGattCharacteristic1 * aChar, GVariant * aOptions,
                                                               BluezEndpoint * apEndpoint)
{
    BluezConnection * retval = NULL;
    const gchar * path       = NULL;
    GVariantDict options;
    GVariant * v;

    VerifyOrExit(apEndpoint != NULL, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(apEndpoint->mIsCentral, );

    /* TODO Unfortunately StartNotify/StopNotify doesn't provide info about
     * peer device in call params so we need look this up ourselves.
     */
    if (aOptions == NULL)
    {
        GList * objects;
        GList * l;
        GList * ll;

        objects = g_dbus_object_manager_get_objects(apEndpoint->mpObjMgr);
        for (l = objects; l != NULL; l = l->next)
        {
            BluezDevice1 * device = bluez_object_get_device1(BLUEZ_OBJECT(l->data));
            if (device != NULL)
            {
                if (BluezIsDeviceOnAdapter(device, apEndpoint->mpAdapter))
                {
                    for (ll = objects; ll != NULL; ll = ll->next)
                    {
                        BluezGattService1 * service = bluez_object_get_gatt_service1(BLUEZ_OBJECT(ll->data));
                        if (service != NULL)
                        {
                            if (BluezIsServiceOnDevice(service, device))
                            {
                                if (BluezIsCharOnService(aChar, service))
                                {
                                    retval = (BluezConnection *) g_hash_table_lookup(
                                        apEndpoint->mpConnMap, g_dbus_proxy_get_object_path(G_DBUS_PROXY(device)));
                                }
                            }
                            g_object_unref(service);
                            if (retval != NULL)
                                break;
                        }
                    }
                }
                g_object_unref(device);
                if (retval != NULL)
                    break;
            }
        }

        g_list_free_full(objects, g_object_unref);
    }
    else
    {
        g_variant_dict_init(&options, aOptions);

        v = g_variant_dict_lookup_value(&options, "device", G_VARIANT_TYPE_OBJECT_PATH);

        VerifyOrExit(v != NULL, ChipLogError(DeviceLayer, "FAIL: No device option in dictionary (%s)", __func__));

        path = g_variant_get_string(v, NULL);

        retval = (BluezConnection *) g_hash_table_lookup(apEndpoint->mpConnMap, path);
    }

exit:
    return retval;
}
#endif // CHIP_BLUEZ_CENTRAL_SUPPORT

void EndpointCleanup(BluezEndpoint * apEndpoint)
{
    if (apEndpoint != nullptr)
    {
        if (apEndpoint->mpOwningName != nullptr)
        {
            g_free(apEndpoint->mpOwningName);
            apEndpoint->mpOwningName = nullptr;
        }
        if (apEndpoint->mpAdapterName != nullptr)
        {
            g_free(apEndpoint->mpAdapterName);
            apEndpoint->mpAdapterName = nullptr;
        }
        if (apEndpoint->mpAdapterAddr != nullptr)
        {
            g_free(apEndpoint->mpAdapterAddr);
            apEndpoint->mpAdapterAddr = nullptr;
        }
        if (apEndpoint->mpRootPath != nullptr)
        {
            g_free(apEndpoint->mpRootPath);
            apEndpoint->mpRootPath = nullptr;
        }
        if (apEndpoint->mpAdvPath != nullptr)
        {
            g_free(apEndpoint->mpAdvPath);
            apEndpoint->mpAdvPath = nullptr;
        }
        if (apEndpoint->mpServicePath != nullptr)
        {
            g_free(apEndpoint->mpServicePath);
            apEndpoint->mpServicePath = nullptr;
        }
        if (apEndpoint->mpConnMap != nullptr)
        {
            g_hash_table_destroy(apEndpoint->mpConnMap);
            apEndpoint->mpConnMap = nullptr;
        }
        if (apEndpoint->mpAdvertisingUUID != nullptr)
        {
            g_free(apEndpoint->mpAdvertisingUUID);
            apEndpoint->mpAdvertisingUUID = nullptr;
        }
        if (apEndpoint->mpPeerDevicePath != nullptr)
        {
            g_free(apEndpoint->mpPeerDevicePath);
            apEndpoint->mpPeerDevicePath = nullptr;
        }
        if (apEndpoint->mpConnectCancellable != nullptr)
        {
            g_object_unref(apEndpoint->mpConnectCancellable);
            apEndpoint->mpConnectCancellable = nullptr;
        }
        g_free(apEndpoint);
    }
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
static void UpdateAdditionalDataCharacteristic(BluezGattCharacteristic1 * characteristic)
{
    if (characteristic == nullptr)
    {
        return;
    }

    // Construct the TLV for the additional data
    GVariant * cValue = nullptr;
    gpointer data;
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;
    BitFlags<AdditionalDataFields> additionalDataFields;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    uint8_t rotatingDeviceIdUniqueId[ConfigurationManager::kRotatingDeviceIDUniqueIDLength] = {};
    MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);

    err = GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan);
    SuccessOrExit(err);
    err = ConfigurationMgr().GetLifetimeCounter(additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter);
    SuccessOrExit(err);
    additionalDataPayloadParams.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
#endif /* CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID) */

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, bufferHandle,
                                                                         additionalDataFields);
    SuccessOrExit(err);

    data = g_memdup(bufferHandle->Start(), bufferHandle->DataLength());

    cValue = g_variant_new_from_data(G_VARIANT_TYPE("ay"), data, bufferHandle->DataLength(), TRUE, g_free, data);
    bluez_gatt_characteristic1_set_value(characteristic, cValue);

    return;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data (%s)", __func__);
    }
    return;
}
#endif

static void BluezPeripheralObjectsSetup(gpointer apClosure)
{

    static const char * const c1_flags[] = { "write", nullptr };
    static const char * const c2_flags[] = { "read", "indicate", nullptr };
    static const char * const c3_flags[] = { "read", nullptr };

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    endpoint->mpService = BluezServiceCreate(apClosure);
    // C1 characteristic
    endpoint->mpC1 =
        BluezCharacteristicCreate(endpoint->mpService, g_strdup("c1"), g_strdup(CHIP_PLAT_BLE_UUID_C1_STRING), endpoint->mpRoot);
    bluez_gatt_characteristic1_set_flags(endpoint->mpC1, c1_flags);

    g_signal_connect(endpoint->mpC1, "handle-read-value", G_CALLBACK(BluezCharacteristicReadValue), apClosure);
    g_signal_connect(endpoint->mpC1, "handle-write-value", G_CALLBACK(BluezCharacteristicWriteValueError), NULL);
    g_signal_connect(endpoint->mpC1, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWrite), apClosure);
    g_signal_connect(endpoint->mpC1, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotifyError), NULL);
    g_signal_connect(endpoint->mpC1, "handle-start-notify", G_CALLBACK(BluezCharacteristicStartNotifyError), NULL);
    g_signal_connect(endpoint->mpC1, "handle-stop-notify", G_CALLBACK(BluezCharacteristicStopNotifyError), NULL);
    g_signal_connect(endpoint->mpC1, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirmError), NULL);

    endpoint->mpC2 =
        BluezCharacteristicCreate(endpoint->mpService, g_strdup("c2"), g_strdup(CHIP_PLAT_BLE_UUID_C2_STRING), endpoint->mpRoot);
    bluez_gatt_characteristic1_set_flags(endpoint->mpC2, c2_flags);
    g_signal_connect(endpoint->mpC2, "handle-read-value", G_CALLBACK(BluezCharacteristicReadValue), apClosure);
    g_signal_connect(endpoint->mpC2, "handle-write-value", G_CALLBACK(BluezCharacteristicWriteValueError), NULL);
    g_signal_connect(endpoint->mpC2, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWriteError), NULL);
    g_signal_connect(endpoint->mpC2, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotify), apClosure);
    g_signal_connect(endpoint->mpC2, "handle-start-notify", G_CALLBACK(BluezCharacteristicStartNotify), apClosure);
    g_signal_connect(endpoint->mpC2, "handle-stop-notify", G_CALLBACK(BluezCharacteristicStopNotify), apClosure);
    g_signal_connect(endpoint->mpC2, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirm), apClosure);

    ChipLogDetail(DeviceLayer, "CHIP BTP C1 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC1));
    ChipLogDetail(DeviceLayer, "CHIP BTP C2 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC2));

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ChipLogDetail(DeviceLayer, "CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING is TRUE");
    // Additional data characteristics
    endpoint->mpC3 =
        BluezCharacteristicCreate(endpoint->mpService, g_strdup("c3"), g_strdup(CHIP_PLAT_BLE_UUID_C3_STRING), endpoint->mpRoot);
    bluez_gatt_characteristic1_set_flags(endpoint->mpC3, c3_flags);
    g_signal_connect(endpoint->mpC3, "handle-read-value", G_CALLBACK(BluezCharacteristicReadValue), apClosure);
    g_signal_connect(endpoint->mpC3, "handle-write-value", G_CALLBACK(BluezCharacteristicWriteValueError), NULL);
    g_signal_connect(endpoint->mpC3, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWriteError), NULL);
    g_signal_connect(endpoint->mpC3, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotify), apClosure);
    g_signal_connect(endpoint->mpC3, "handle-start-notify", G_CALLBACK(BluezCharacteristicStartNotify), apClosure);
    g_signal_connect(endpoint->mpC3, "handle-stop-notify", G_CALLBACK(BluezCharacteristicStopNotify), apClosure);
    g_signal_connect(endpoint->mpC3, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirm), apClosure);
    // update the characteristic value
    UpdateAdditionalDataCharacteristic(endpoint->mpC3);
    ChipLogDetail(DeviceLayer, "CHIP BTP C3 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC3));
#else
    ChipLogDetail(DeviceLayer, "CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING is FALSE");
    (void) c3_flags;
#endif

exit:
    return;
}

static void BluezOnBusAcquired(GDBusConnection * aConn, const gchar * aName, gpointer apClosure)
{
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    ChipLogDetail(DeviceLayer, "TRACE: Bus acquired for name %s", aName);

    if (!endpoint->mIsCentral)
    {
        endpoint->mpRootPath = g_strdup_printf("/chipoble/%04x", getpid() & 0xffff);
        endpoint->mpRoot     = g_dbus_object_manager_server_new(endpoint->mpRootPath);
        g_dbus_object_manager_server_set_connection(endpoint->mpRoot, aConn);

        BluezPeripheralObjectsSetup(apClosure);
    }

exit:
    return;
}

#if CHIP_BLUEZ_NAME_MONITOR
static void BluezOnNameAcquired(GDBusConnection * aConn, const gchar * aName, gpointer apClosure)
{
    ChipLogDetail(DeviceLayer, "TRACE: Owning name: Acquired %s", aName);
}

static void BluezOnNameLost(GDBusConnection * aConn, const gchar * aName, gpointer apClosure)
{
    ChipLogDetail(DeviceLayer, "TRACE: Owning name: lost %s", aName);
}
#endif

static int StartupEndpointBindings(BluezEndpoint * endpoint)
{
    GDBusObjectManager * manager;
    GError * error         = nullptr;
    GDBusConnection * conn = nullptr;
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "FAIL: get bus sync in %s, error: %s", __func__, error->message));

    if (endpoint->mpAdapterName != nullptr)
        endpoint->mpOwningName = g_strdup_printf("%s", endpoint->mpAdapterName);
    else
        endpoint->mpOwningName = g_strdup_printf("C-%04x", getpid() & 0xffff);

    BluezOnBusAcquired(conn, endpoint->mpOwningName, endpoint);

    manager = g_dbus_object_manager_client_new_sync(
        conn, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/", bluez_object_manager_client_get_proxy_type,
        nullptr /* unused user data in the Proxy Type Func */, nullptr /*destroy notify */, nullptr /* cancellable */, &error);

    VerifyOrExit(manager != nullptr, ChipLogError(DeviceLayer, "FAIL: Error getting object manager client: %s", error->message));

    endpoint->mpObjMgr = manager;

    bluezObjectsSetup(endpoint);

    g_signal_connect(manager, "object-added", G_CALLBACK(BluezSignalOnObjectAdded), endpoint);
    g_signal_connect(manager, "object-removed", G_CALLBACK(BluezSignalOnObjectRemoved), endpoint);
    g_signal_connect(manager, "interface-proxy-properties-changed", G_CALLBACK(BluezSignalInterfacePropertiesChanged), endpoint);

exit:
    if (error != nullptr)
        g_error_free(error);

    return 0;
}

static gboolean BluezC2Indicate(ConnectionDataBundle * closure)
{
    BluezConnection * conn = nullptr;
    GError * error         = nullptr;
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
        status = g_io_channel_write_chars(conn->mC2Channel.mpChannel, buf, static_cast<gssize>(len), &written, &error);
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
        if (closure->mpVal)
        {
            g_variant_unref(closure->mpVal);
        }
        g_free(closure);
    }

    if (error != nullptr)
        g_error_free(error);
    return G_SOURCE_REMOVE;
}

static ConnectionDataBundle * MakeConnectionDataBundle(BLE_CONNECTION_OBJECT apConn, const chip::System::PacketBufferHandle & apBuf)
{
    ConnectionDataBundle * bundle = g_new(ConnectionDataBundle, 1);
    bundle->mpConn                = static_cast<BluezConnection *>(apConn);
    bundle->mpVal =
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, apBuf->Start(), apBuf->DataLength() * sizeof(uint8_t), sizeof(uint8_t));
    return bundle;
}

CHIP_ERROR SendBluezIndication(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), (ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__), CHIP_ERROR_INVALID_ARGUMENT));
    return PlatformMgrImpl().ScheduleOnGLibMainLoopThread(BluezC2Indicate, MakeConnectionDataBundle(apConn, apBuf));
}

static gboolean BluezDisconnect(void * apClosure)
{
    BluezConnection * conn = static_cast<BluezConnection *>(apClosure);
    GError * error         = nullptr;
    gboolean success;

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "conn is NULL in %s", __func__));
    VerifyOrExit(conn->mpDevice != nullptr, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", "NULL Device"));

    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, bluez_device1_get_address(conn->mpDevice));

    success = bluez_device1_call_disconnect_sync(conn->mpDevice, nullptr, &error);
    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: Disconnect: %s", error->message));

exit:
    if (error != nullptr)
        g_error_free(error);
    return G_SOURCE_REMOVE;
}

static int CloseBleconnectionCB(void * apAppState)
{
    BluezDisconnect(apAppState);
    return G_SOURCE_REMOVE;
}

CHIP_ERROR CloseBluezConnection(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().ScheduleOnGLibMainLoopThread(CloseBleconnectionCB, apConn);
}

CHIP_ERROR StartBluezAdv(BluezEndpoint * apEndpoint)
{
    CHIP_ERROR err = PlatformMgrImpl().ScheduleOnGLibMainLoopThread(BluezAdvStart, apEndpoint);
    VerifyOrReturnError(err == CHIP_NO_ERROR,
                        (ChipLogError(Ble, "Failed to schedule BluezAdvStart() on CHIPoBluez thread"), CHIP_ERROR_INCORRECT_STATE));
    return err;
}

CHIP_ERROR StopBluezAdv(BluezEndpoint * apEndpoint)
{
    CHIP_ERROR err = PlatformMgrImpl().ScheduleOnGLibMainLoopThread(BluezAdvStop, apEndpoint);
    VerifyOrReturnError(err == CHIP_NO_ERROR,
                        (ChipLogError(Ble, "Failed to schedule BluezAdvStop() on CHIPoBluez thread"), CHIP_ERROR_INCORRECT_STATE));
    return err;
}

CHIP_ERROR BluezAdvertisementSetup(BluezEndpoint * apEndpoint)
{
    CHIP_ERROR err = PlatformMgrImpl().ScheduleOnGLibMainLoopThread(BluezAdvSetup, apEndpoint);
    VerifyOrReturnError(err == CHIP_NO_ERROR,
                        (ChipLogError(Ble, "Failed to schedule BluezAdvSetup() on CHIPoBluez thread"), CHIP_ERROR_INCORRECT_STATE));
    return err;
}

CHIP_ERROR BluezGattsAppRegister(BluezEndpoint * apEndpoint)
{
    CHIP_ERROR err = PlatformMgrImpl().ScheduleOnGLibMainLoopThread(BluezPeripheralRegisterApp, apEndpoint);
    VerifyOrReturnError(
        err == CHIP_NO_ERROR,
        (ChipLogError(Ble, "Failed to schedule BluezPeripheralRegisterApp() on CHIPoBluez thread"), CHIP_ERROR_INCORRECT_STATE));
    return err;
}

CHIP_ERROR ConfigureBluezAdv(BLEAdvConfig & aBleAdvConfig, BluezEndpoint * apEndpoint)
{
    const char * msg = nullptr;
    CHIP_ERROR err   = CHIP_NO_ERROR;
    VerifyOrExit(aBleAdvConfig.mpBleName != nullptr, msg = "FAIL: BLE name is NULL");
    VerifyOrExit(aBleAdvConfig.mpAdvertisingUUID != nullptr, msg = "FAIL: BLE mpAdvertisingUUID is NULL in %s");

    apEndpoint->mpAdapterName     = g_strdup(aBleAdvConfig.mpBleName);
    apEndpoint->mpAdvertisingUUID = g_strdup(aBleAdvConfig.mpAdvertisingUUID);
    apEndpoint->mAdapterId        = aBleAdvConfig.mAdapterId;
    apEndpoint->mType             = aBleAdvConfig.mType;
    apEndpoint->mDuration         = aBleAdvConfig.mDuration;
    apEndpoint->mDuration         = aBleAdvConfig.mDuration;

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(apEndpoint->mDeviceIdInfo);
    SuccessOrExit(err);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    apEndpoint->mDeviceIdInfo.SetAdditionalDataFlag(true);
#endif

exit:
    if (nullptr != msg)
    {
        ChipLogDetail(DeviceLayer, "%s in %s", msg, __func__);
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    return err;
}

CHIP_ERROR InitBluezBleLayer(bool aIsCentral, char * apBleAddr, BLEAdvConfig & aBleAdvConfig, BluezEndpoint *& apEndpoint)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool retval              = false;
    BluezEndpoint * endpoint = nullptr;

    // initialize server endpoint
    endpoint = g_new0(BluezEndpoint, 1);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "FAIL: memory allocation in %s", __func__));

    if (apBleAddr != nullptr)
        endpoint->mpAdapterAddr = g_strdup(apBleAddr);
    else
        endpoint->mpAdapterAddr = nullptr;

    endpoint->mpConnMap  = g_hash_table_new(g_str_hash, g_str_equal);
    endpoint->mIsCentral = aIsCentral;

    if (!aIsCentral)
    {
        err = ConfigureBluezAdv(aBleAdvConfig, endpoint);
        SuccessOrExit(err);
    }
    else
    {
        endpoint->mAdapterId           = aBleAdvConfig.mAdapterId;
        endpoint->mpConnectCancellable = g_cancellable_new();
    }

    err = PlatformMgrImpl().ScheduleOnGLibMainLoopThread(StartupEndpointBindings, endpoint, true);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to schedule endpoint initialization"));

    retval = TRUE;

exit:
    if (retval)
    {
        apEndpoint = endpoint;
        ChipLogDetail(DeviceLayer, "PlatformBlueZInit init success");
    }
    else
    {
        EndpointCleanup(endpoint);
    }

    return err;
}

CHIP_ERROR ShutdownBluezBleLayer(BluezEndpoint * apEndpoint)
{
    VerifyOrReturnError(apEndpoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    g_object_unref(apEndpoint->mpAdapter);
    EndpointCleanup(apEndpoint);
    return CHIP_NO_ERROR;
}

// BluezSendWriteRequest callbacks

static void SendWriteRequestDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c1 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GError * error                = nullptr;
    gboolean success              = bluez_gatt_characteristic1_call_write_value_finish(c1, aResult, &error);

    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezSendWriteRequest : %s", error->message));
    BLEManagerImpl::HandleWriteComplete(static_cast<BLE_CONNECTION_OBJECT>(apConnection));

exit:
    if (error != nullptr)
        g_error_free(error);
}

static gboolean SendWriteRequestImpl(ConnectionDataBundle * data)
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
    return G_SOURCE_REMOVE;
}

CHIP_ERROR BluezSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), (ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__), CHIP_ERROR_INVALID_ARGUMENT));
    return PlatformMgrImpl().ScheduleOnGLibMainLoopThread(SendWriteRequestImpl, MakeConnectionDataBundle(apConn, apBuf));
}

// BluezSubscribeCharacteristic callbacks

static void OnCharacteristicChanged(GDBusProxy * aInterface, GVariant * aChangedProperties, const gchar * const * aInvalidatedProps,
                                    gpointer apConnection)
{
    BLE_CONNECTION_OBJECT connection = static_cast<BLE_CONNECTION_OBJECT>(apConnection);
    GVariant * value                 = g_variant_lookup_value(aChangedProperties, "Value", G_VARIANT_TYPE_BYTESTRING);
    VerifyOrReturn(value != nullptr);

    size_t bufferLen;
    auto buffer = g_variant_get_fixed_array(value, &bufferLen, sizeof(uint8_t));
    VerifyOrReturn(value != nullptr, ChipLogError(DeviceLayer, "Characteristic value has unexpected type"));

    BLEManagerImpl::HandleTXCharChanged(connection, static_cast<const uint8_t *>(buffer), bufferLen);
}

static void SubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GError * error                = nullptr;
    gboolean success              = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &error);

    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezSubscribeCharacteristic : %s", error->message));

    BLEManagerImpl::HandleSubscribeOpComplete(static_cast<BLE_CONNECTION_OBJECT>(apConnection), true);

exit:
    if (error != nullptr)
        g_error_free(error);
}

static gboolean SubscribeCharacteristicImpl(BluezConnection * connection)
{
    BluezGattCharacteristic1 * c2 = nullptr;
    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));
    c2 = BLUEZ_GATT_CHARACTERISTIC1(connection->mpC2);

    // Get notifications on the TX characteristic change (e.g. indication is received)
    g_signal_connect(c2, "g-properties-changed", G_CALLBACK(OnCharacteristicChanged), connection);
    bluez_gatt_characteristic1_call_start_notify(connection->mpC2, nullptr, SubscribeCharacteristicDone, connection);

exit:
    return G_SOURCE_REMOVE;
}

CHIP_ERROR BluezSubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().ScheduleOnGLibMainLoopThread(SubscribeCharacteristicImpl, static_cast<BluezConnection *>(apConn));
}

// BluezUnsubscribeCharacteristic callbacks

static void UnsubscribeCharacteristicDone(GObject * aObject, GAsyncResult * aResult, gpointer apConnection)
{
    BluezGattCharacteristic1 * c2 = BLUEZ_GATT_CHARACTERISTIC1(aObject);
    GError * error                = nullptr;
    gboolean success              = bluez_gatt_characteristic1_call_write_value_finish(c2, aResult, &error);

    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: BluezUnsubscribeCharacteristic : %s", error->message));

    // Stop listening to the TX characteristic changes
    g_signal_handlers_disconnect_by_data(c2, apConnection);
    BLEManagerImpl::HandleSubscribeOpComplete(static_cast<BLE_CONNECTION_OBJECT>(apConnection), false);

exit:
    if (error != nullptr)
        g_error_free(error);
}

static gboolean UnsubscribeCharacteristicImpl(BluezConnection * connection)
{
    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in %s", __func__));

    bluez_gatt_characteristic1_call_stop_notify(connection->mpC2, nullptr, UnsubscribeCharacteristicDone, connection);

exit:
    return G_SOURCE_REMOVE;
}

CHIP_ERROR BluezUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().ScheduleOnGLibMainLoopThread(UnsubscribeCharacteristicImpl, static_cast<BluezConnection *>(apConn));
}

// ConnectDevice callbacks

struct ConnectParams
{
    ConnectParams(BluezDevice1 * device, BluezEndpoint * endpoint) : mDevice(device), mEndpoint(endpoint), mNumRetries(0) {}
    BluezDevice1 * mDevice;
    BluezEndpoint * mEndpoint;
    uint16_t mNumRetries;
};

static void ConnectDeviceDone(GObject * aObject, GAsyncResult * aResult, gpointer apParams)
{
    BluezDevice1 * device  = BLUEZ_DEVICE1(aObject);
    GError * error         = nullptr;
    gboolean success       = bluez_device1_call_connect_finish(device, aResult, &error);
    ConnectParams * params = static_cast<ConnectParams *>(apParams);

    assert(params != nullptr);

    if (!success)
    {
        ChipLogError(DeviceLayer, "FAIL: ConnectDevice : %s (%d)", error->message, error->code);

        // Due to radio interferences or Wi-Fi coexistence, sometimes the BLE connection may not be
        // established (e.g. Connection Indication Packet is missed by BLE peripheral). In such case,
        // BlueZ returns "Software caused connection abort error", and we should make a connection retry.
        // It's important to make sure that the connection is correctly ceased, by calling `Disconnect()`
        // D-Bus method, or else `Connect()` returns immediately without any effect.
        if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_DBUS_ERROR) && params->mNumRetries++ < kMaxConnectRetries)
        {
            // Clear the error before usage in subsequent call.
            g_clear_error(&error);

            bluez_device1_call_disconnect_sync(device, nullptr, &error);
            bluez_device1_call_connect(device, params->mEndpoint->mpConnectCancellable, ConnectDeviceDone, params);
            ExitNow();
        }

        BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "ConnectDevice complete");
    }

    chip::Platform::Delete(params);

exit:
    if (error != nullptr)
        g_error_free(error);
}

static gboolean ConnectDeviceImpl(ConnectParams * apParams)
{
    BluezDevice1 * device    = apParams->mDevice;
    BluezEndpoint * endpoint = apParams->mEndpoint;

    assert(device != nullptr);
    assert(endpoint != nullptr);

    g_cancellable_reset(endpoint->mpConnectCancellable);
    bluez_device1_call_connect(device, endpoint->mpConnectCancellable, ConnectDeviceDone, apParams);
    g_object_unref(device);

    return G_SOURCE_REMOVE;
}

CHIP_ERROR ConnectDevice(BluezDevice1 * apDevice, BluezEndpoint * apEndpoint)
{
    auto params = chip::Platform::New<ConnectParams>(apDevice, apEndpoint);
    g_object_ref(apDevice);

    if (PlatformMgrImpl().ScheduleOnGLibMainLoopThread(ConnectDeviceImpl, params) != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule ConnectDeviceImpl() on CHIPoBluez thread");
        g_object_unref(apDevice);
        chip::Platform::Delete(params);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

void CancelConnect(BluezEndpoint * apEndpoint)
{
    assert(apEndpoint->mpConnectCancellable != nullptr);
    g_cancellable_cancel(apEndpoint->mpConnectCancellable);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
