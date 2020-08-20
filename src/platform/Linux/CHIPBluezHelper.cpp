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
 *          Provides Bluez dbus implementatioon for BLE
 */

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <errno.h>
#include <gio/gunixfdlist.h>
#include <stdarg.h>
#include <strings.h>
#include <unistd.h>

#include "CHIPBluezHelper.h"
#include <support/CodeUtils.h>

using namespace ::nl;

namespace chip {
namespace DeviceLayer {
namespace Internal {

static int sBluezFD[2];
static GMainLoop * sBluezMainLoop = NULL;
static pthread_t sBluezThread;
static BluezConnection * GetBluezConnectionViaDevice(BluezEndpoint * apEndpoint);
static BluezConnection * BluezCharacteristicGetBluezConnection(BluezGattCharacteristic1 * aChar, GVariant * aOptions,
                                                               BluezEndpoint * apEndpoint);

static gboolean BluezAdvertisingRelease(BluezLEAdvertisement1 * aAdv, GDBusMethodInvocation * aInvocation, gpointer apClosure)
{
    bool isSuccess           = false;
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(aAdv != NULL, ChipLogProgress(DeviceLayer, "BluezLEAdvertisement1 is NULL in %s", __func__));
    ChipLogProgress(DeviceLayer, "Release adv object in %s", __func__);

    g_dbus_object_manager_server_unexport(endpoint->mpRoot, endpoint->mpAdvPath);
    endpoint->mIsAdvertising = false;
    isSuccess                = true;
exit:

    return isSuccess ? TRUE : FALSE;
}

static BluezLEAdvertisement1 * BluezAdvertisingCreate(BluezEndpoint * apEndpoint)
{
    BluezLEAdvertisement1 * adv = NULL;
    BluezObjectSkeleton * object;
    GVariant * serviceData;
    gchar * localName;
    GVariantBuilder serviceDataBuilder;
    GVariantBuilder serviceUUIDsBuilder;
    uint16_t offset;
    char * debugStr;
    const gchar * array[1];

    VerifyOrExit(apEndpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    if (apEndpoint->mpAdvPath == NULL)
        apEndpoint->mpAdvPath = g_strdup_printf("%s/advertising", apEndpoint->mpRootPath);

    array[0] = g_strdup_printf("%s", apEndpoint->mpAdvertisingUUID);

    ChipLogProgress(DeviceLayer, "Create adv object at %s", apEndpoint->mpAdvPath);
    object = bluez_object_skeleton_new(apEndpoint->mpAdvPath);

    adv = bluez_leadvertisement1_skeleton_new();

    g_variant_builder_init(&serviceDataBuilder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_init(&serviceUUIDsBuilder, G_VARIANT_TYPE("as"));
    offset = 0;

    g_variant_builder_add(
        &serviceDataBuilder, "{sv}", apEndpoint->mpAdvertisingUUID,
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, apEndpoint->mpChipServiceData, sizeof(CHIPServiceData), sizeof(uint8_t)));
    g_variant_builder_add(&serviceUUIDsBuilder, "s", apEndpoint->mpAdvertisingUUID);

    if (apEndpoint->mpAdapterName != NULL)
        localName = g_strdup_printf("%s", apEndpoint->mpAdapterName);
    else
        localName = g_strdup_printf("%s%04x", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, getpid() & 0xffff);

    serviceData = g_variant_builder_end(&serviceDataBuilder);
    debugStr    = g_variant_print(serviceData, TRUE);
    ChipLogProgress(DeviceLayer, "SET service data to %s", debugStr);
    g_free(debugStr);

    bluez_leadvertisement1_set_type_(adv, (apEndpoint->mType & BLUEZ_ADV_TYPE_CONNECTABLE) ? "peripheral" : "broadcast");
    // empty manufacturer data
    // empty solicit UUIDs
    bluez_leadvertisement1_set_service_data(adv, serviceData);
    // empty data

    bluez_leadvertisement1_set_discoverable(adv, (apEndpoint->mType & BLUEZ_ADV_TYPE_SCANNABLE) ? TRUE : FALSE);

    // advertising name corresponding to the PID and object path, for debug purposes
    bluez_leadvertisement1_set_local_name(adv, localName);
    bluez_leadvertisement1_set_service_uuids(adv, array);

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

    BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(true, NULL);

exit:
    return adv;
}

static void BluezAdvStartDone(GObject * aObject, GAsyncResult * aResult, gpointer apClosure)
{
    BluezLEAdvertisingManager1 * advMgr = BLUEZ_LEADVERTISING_MANAGER1(aObject);
    GError * error                      = NULL;
    BluezEndpoint * endpoint            = static_cast<BluezEndpoint *>(apClosure);
    gboolean success;

    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    success = bluez_leadvertising_manager1_call_register_advertisement_finish(advMgr, aResult, &error);
    if (success == FALSE)
    {
        g_dbus_object_manager_server_unexport(endpoint->mpRoot, endpoint->mpAdvPath);
    }
    VerifyOrExit(success == TRUE, ChipLogProgress(DeviceLayer, "FAIL: RegisterAdvertisement : %s", error->message));

    endpoint->mIsAdvertising = true;

    ChipLogProgress(DeviceLayer, "RegisterAdvertisement complete");

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(success == TRUE ? true : false, NULL);
    if (error != NULL)
        g_error_free(error);
}

static void BluezAdvStopDone(GObject * aObject, GAsyncResult * aResult, gpointer apClosure)
{
    BluezLEAdvertisingManager1 * advMgr = BLUEZ_LEADVERTISING_MANAGER1(aObject);
    BluezEndpoint * endpoint            = static_cast<BluezEndpoint *>(apClosure);
    GError * error                      = NULL;
    gboolean success;

    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    success = bluez_leadvertising_manager1_call_unregister_advertisement_finish(advMgr, aResult, &error);

    if (success == FALSE)
    {
        g_dbus_object_manager_server_unexport(endpoint->mpRoot, endpoint->mpAdvPath);
    }
    else
    {
        endpoint->mIsAdvertising = false;
    }

    VerifyOrExit(success == TRUE, ChipLogProgress(DeviceLayer, "FAIL: UnregisterAdvertisement : %s", error->message));

    ChipLogProgress(DeviceLayer, "UnregisterAdvertisement complete");

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(success == TRUE ? true : false, NULL);
    if (error != NULL)
        g_error_free(error);
}

static gboolean BluezAdvSetup(void * apClosure)
{
    GDBusObject * adapter;
    BluezEndpoint * endpoint            = static_cast<BluezEndpoint *>(apClosure);
    BluezLEAdvertisingManager1 * advMgr = NULL;
    GVariantBuilder optionsBuilder;
    GVariant * options;
    BluezLEAdvertisement1 * adv;

    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(endpoint->mIsAdvertising == FALSE,
                 ChipLogProgress(DeviceLayer, "FAIL: Advertising already enabled in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adv = BluezAdvertisingCreate(endpoint);
    VerifyOrExit(adv != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL adv in %s", __func__));

exit:
    return G_SOURCE_REMOVE;
}

static gboolean BluezAdvStart(void * apClosure)
{
    GDBusObject * adapter;
    BluezLEAdvertisingManager1 * advMgr = NULL;
    GVariantBuilder optionsBuilder;
    GVariant * options;
    BluezLEAdvertisement1 * adv;
    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);

    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(!endpoint->mIsAdvertising,
                 ChipLogProgress(DeviceLayer, "FAIL: Advertising has already been enabled in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(endpoint->mpAdapter));
    VerifyOrExit(adapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    advMgr = bluez_object_get_leadvertising_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(advMgr != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL advMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_leadvertising_manager1_call_register_advertisement(advMgr, endpoint->mpAdvPath, options, NULL, BluezAdvStartDone,
                                                             apClosure);

exit:
    return G_SOURCE_REMOVE;
}

static gboolean BluezAdvStop(void * apClosure)
{
    GDBusObject * adapter;
    BluezEndpoint * endpoint            = static_cast<BluezEndpoint *>(apClosure);
    BluezLEAdvertisingManager1 * advMgr = NULL;
    GVariantBuilder optionsBuilder;
    GVariant * options;
    BluezLEAdvertisement1 * adv;

    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(endpoint->mIsAdvertising,
                 ChipLogProgress(DeviceLayer, "FAIL: Advertising has already been disabled in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(endpoint->mpAdapter));
    VerifyOrExit(adapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    advMgr = bluez_object_get_leadvertising_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(advMgr != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL advMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_leadvertising_manager1_call_unregister_advertisement(advMgr, endpoint->mpAdvPath, NULL, BluezAdvStopDone, apClosure);

exit:
    return G_SOURCE_REMOVE;
}

static gboolean BluezCharacteristicReadValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                             GVariant * aOptions)
{
    GVariant * val;
    ChipLogProgress(DeviceLayer, "Received BluezCharacteristicReadValue");
    val = (GVariant *) bluez_gatt_characteristic1_get_value(aChar);
    bluez_gatt_characteristic1_complete_read_value(aChar, aInvocation, val);
    return TRUE;
}

static gboolean BluezCharacteristicWriteValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                              GVariant * aValue, GVariant * aOptions, gpointer apClosure)
{
    const uint8_t * tmpBuf;
    uint8_t * buf;
    size_t len;
    bool isSuccess         = false;
    BluezConnection * conn = NULL;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    VerifyOrExit(aValue != NULL, ChipLogProgress(DeviceLayer, "aValue is NULL in %s", __func__));

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

static gboolean BluezCharacteristicWriteValueError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                   GVariant * aValue, GVariant * aOptions, gpointer apClosure)
{
    ChipLogProgress(DeviceLayer, "BluezCharacteristicWriteValueError");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "Write for characteristic is unsupported");
exit:
    return TRUE;
}

static gboolean BluezCharacteristicWriteFD(GIOChannel * aChannel, GIOCondition aCond, gpointer apClosure)
{
    GVariant * newVal;
    gchar * buf;
    ssize_t len;
    int fd;
    bool isSuccess = false;

    BluezConnection * conn = static_cast<BluezConnection *>(apClosure);

    VerifyOrExit(conn != NULL, ChipLogProgress(DeviceLayer, "No CHIP Bluez connection in %s", __func__));

    VerifyOrExit(!(aCond & G_IO_HUP), ChipLogProgress(DeviceLayer, "INFO: socket disconnected in %s", __func__));
    VerifyOrExit(!(aCond & (G_IO_ERR | G_IO_NVAL)), ChipLogProgress(DeviceLayer, "INFO: socket error in %s", __func__));
    VerifyOrExit(aCond == G_IO_IN, ChipLogProgress(DeviceLayer, "FAIL: error in %s", __func__));

    ChipLogProgress(DeviceLayer, "c1 %s mtu, %d", __func__, conn->mMtu);

    buf = (gchar *) (g_malloc(conn->mMtu));
    fd  = g_io_channel_unix_get_fd(aChannel);

    len = read(fd, buf, conn->mMtu);

    VerifyOrExit(len > 0, ChipLogProgress(DeviceLayer, "FAIL: short read in %s (%d)", __func__, len));

    newVal = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, buf, len, sizeof(uint8_t));

    bluez_gatt_characteristic1_set_value(conn->mpC1, newVal);
    BLEManagerImpl::HandleRXCharWrite(conn, (uint8_t *) (buf), len);
    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}

static void Bluez_gatt_characteristic1_complete_acquire_write_with_fd(GDBusMethodInvocation * invocation, int fd, guint16 mtu)
{
    GUnixFDList * fd_list = g_unix_fd_list_new();
    int index;

    index = g_unix_fd_list_append(fd_list, fd, NULL);

    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, g_variant_new("(@hq)", g_variant_new_handle(index), mtu),
                                                            fd_list);
}

static gboolean bluezCharacteristicDestroyFD(GIOChannel * aChannel, GIOCondition aCond, gpointer apClosure)
{
    return G_SOURCE_REMOVE;
}

static gboolean BluezCharacteristicAcquireWrite(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                GVariant * aOptions, gpointer apClosure)
{
    int fds[2] = { -1, -1 };
    GIOChannel * channel;
    char * errStr;
    GVariantDict options;
    bool isSuccess         = false;
    BluezConnection * conn = NULL;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != NULL,
                 g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    ChipLogProgress(DeviceLayer, "BluezCharacteristicAcquireWrite is called, conn: %p", conn);

    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) < 0)
    {
        errStr = strerror(errno);
        ChipLogProgress(DeviceLayer, "FAIL: socketpair: %s in %s", errStr, __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "FD creation failed");
        SuccessOrExit(false);
    }

    g_variant_dict_init(&options, aOptions);
    if (g_variant_dict_contains(&options, "mtu") == TRUE)
    {
        GVariant * v = g_variant_dict_lookup_value(&options, "mtu", G_VARIANT_TYPE_UINT16);
        conn->mMtu   = g_variant_get_uint16(v);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: no MTU in options in %s", __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.InvalidArguments", "MTU negotiation failed");
        SuccessOrExit(false);
    }

    channel = g_io_channel_unix_new(fds[0]);
    g_io_channel_set_encoding(channel, NULL, NULL);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_channel_set_buffered(channel, FALSE);

    conn->mC1Channel.mpChannel = channel;
    conn->mC1Channel.mWatch =
        g_io_add_watch(channel, (GIOCondition)(G_IO_HUP | G_IO_IN | G_IO_ERR | G_IO_NVAL), BluezCharacteristicWriteFD, conn);

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
    ChipLogProgress(DeviceLayer, "BluezCharacteristicAcquireWriteError is called");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireWrite for characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicAcquireNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                 GVariant * aOptions, gpointer apClosure)
{
    int fds[2] = { -1, -1 };
    GIOChannel * channel;
    char * errStr;
    GVariantDict options;
    BluezConnection * conn = NULL;
    bool isSuccess         = false;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != NULL,
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
        errStr = strerror(errno);
        ChipLogProgress(DeviceLayer, "FAIL: socketpair: %s in %s", errStr, __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "FD creation failed");
        SuccessOrExit(false);
    }
    channel = g_io_channel_unix_new(fds[0]);
    g_io_channel_set_encoding(channel, NULL, NULL);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_channel_set_buffered(channel, FALSE);
    conn->mC2Channel.mpChannel = channel;
    conn->mC2Channel.mWatch = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT_IDLE, (GIOCondition)(G_IO_HUP | G_IO_ERR | G_IO_NVAL),
                                                  bluezCharacteristicDestroyFD, conn, NULL);

    bluez_gatt_characteristic1_set_notify_acquired(aChar, TRUE);

    // same reply as for AcquireWrite
    Bluez_gatt_characteristic1_complete_acquire_write_with_fd(aInvocation, fds[1], conn->mMtu);
    close(fds[1]);

    conn->mIsNotify = true;
    BLEManagerImpl::HandleTXCharCCCDWrite((void *) conn);
    isSuccess = true;

exit:
    return isSuccess ? TRUE : FALSE;
}

static gboolean BluezCharacteristicAcquireNotifyError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                      GVariant * aOptions)
{
    ChipLogProgress(DeviceLayer, "TRACE: AcquireNotify is called");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireNotify for characteristic is unsupported");
    return TRUE;
}

static gboolean BluezCharacteristicStartNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                               gpointer apClosure)
{
    bool isSuccess         = false;
    BluezConnection * conn = NULL;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != NULL,
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
                                              gpointer apClosure)
{
    bool isSuccess         = false;
    BluezConnection * conn = NULL;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apClosure);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrExit(conn != NULL,
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
    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
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

// both arguments allocated, and non-null
static void BluezStringAddressToCHIPAddress(const char * aAddressString, BluezAddress * apAddress)
{
    size_t i, j = 0;
    for (i = 0; i < BLUEZ_ADDRESS_SIZE; i++)
    {
        apAddress->mAddress[i] = (CHAR_TO_BLUEZ(aAddressString[j]) << 4) + CHAR_TO_BLUEZ(aAddressString[j + 1]);
        j += 2;
        if (aAddressString[j] == ':')
            j++;
    }
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
    ChipLogProgress(DeviceLayer, "Char1 %s", bluez_gatt_characteristic1_get_service(aChar));
    ChipLogProgress(DeviceLayer, "Char1 %s", g_dbus_proxy_get_object_path(G_DBUS_PROXY(aService)));
    return strcmp(bluez_gatt_characteristic1_get_service(aChar), g_dbus_proxy_get_object_path(G_DBUS_PROXY(aService))) == 0 ? TRUE
                                                                                                                            : FALSE;
}

static uint16_t BluezUUIDStringToShortServiceID(const char * aService)
{
    uint32_t shortService = 0;
    size_t i;
    // check that the service is a short UUID

    if (strncasecmp(CHIP_BLE_BASE_SERVICE_UUID_STRING, aService + CHIP_BLE_SERVICE_PREFIX_LENGTH,
                    sizeof(CHIP_BLE_BASE_SERVICE_UUID_STRING)) == 0)
    {
        for (i = 0; i < 4; i++)
        {
            shortService = (shortService << 8) | (CHAR_TO_BLUEZ(aService[2 * i]) << 4) | CHAR_TO_BLUEZ(aService[2 * i + 1]);
        }

        ChipLogProgress(DeviceLayer, "TRACE: full service UUID: %s, short service %08x", aService, shortService);

        if (shortService > UINT16_MAX)
        {
            shortService = 0;
        }
    }
    return (uint16_t) shortService;
}

static void BluezConnectionInit(BluezConnection * apConn)
{
    // populate the service and the characteristics
    GList * objects = NULL;
    GList * l;
    BluezEndpoint * endpoint = NULL;

    VerifyOrExit(apConn != NULL, ChipLogProgress(DeviceLayer, "Bluez connection is NULL in %s", __func__));

    endpoint = apConn->mpEndpoint;
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    if (!endpoint->mIsCentral)
    {
        apConn->mpService = BLUEZ_GATT_SERVICE1(g_object_ref(apConn->mpEndpoint->mpService));
        apConn->mpC1      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(endpoint->mpC1));
        apConn->mpC2      = BLUEZ_GATT_CHARACTERISTIC1(g_object_ref(endpoint->mpC2));
    }
    else
    {
        objects = g_dbus_object_manager_get_objects(endpoint->mpObjMgr);

        for (l = objects; l != NULL; l = l->next)
        {
            BluezObject * object        = BLUEZ_OBJECT(l->data);
            BluezGattService1 * service = bluez_object_get_gatt_service1(object);

            if (service != NULL)
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

        VerifyOrExit(apConn->mpService != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL service in %s", __func__));

        for (l = objects; l != NULL; l = l->next)
        {
            BluezObject * object             = BLUEZ_OBJECT(l->data);
            BluezGattCharacteristic1 * char1 = bluez_object_get_gatt_characteristic1(object);

            if (char1 != NULL)
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
                else
                {
                    g_object_unref(char1);
                }
                if ((apConn->mpC1 != NULL) && (apConn->mpC2 != NULL))
                {
                    break;
                }
            }
        }

        VerifyOrExit(apConn->mpC1 != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL C1 in %s", __func__));
        VerifyOrExit(apConn->mpC2 != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL C2 in %s", __func__));
    }

exit:
    if (objects != NULL)
        g_list_free_full(objects, g_object_unref);
}

static gboolean BluezConnectionInitIdle(gpointer user_data)
{
    BluezConnection * conn = (BluezConnection *) user_data;

    ChipLogProgress(DeviceLayer, "%s", __func__);

    BluezConnectionInit(conn);

    return FALSE;
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

    ChipLogProgress(DeviceLayer, "Create characteristic object at %s", charPath);
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
    GError * error              = NULL;
    BluezGattManager1 * gattMgr = BLUEZ_GATT_MANAGER1(aObject);

    gboolean success = bluez_gatt_manager1_call_register_application_finish(gattMgr, aResult, &error);

    VerifyOrExit(success == TRUE, ChipLogProgress(DeviceLayer, "FAIL: RegisterApplication : %s", error->message));

    BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(true, NULL);
    ChipLogProgress(DeviceLayer, "BluezPeripheralRegisterAppDone done");

exit:
    if (error != NULL)
    {
        BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(false, NULL);
        g_error_free(error);
    }
    return;
}

gboolean BluezPeripheralRegisterApp(void * apClosure)
{
    GDBusObject * adapter;
    BluezGattManager1 * gattMgr;
    GVariantBuilder optionsBuilder;
    GVariant * options;

    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
    VerifyOrExit(endpoint->mpAdapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(endpoint->mpAdapter));
    VerifyOrExit(adapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    gattMgr = bluez_object_get_gatt_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(gattMgr != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL gattMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_gatt_manager1_call_register_application(gattMgr, endpoint->mpRootPath, options, NULL, BluezPeripheralRegisterAppDone,
                                                  NULL);

exit:
    return G_SOURCE_REMOVE;
}

/***********************************************************************
 * GATT Characteristic object
 ***********************************************************************/

static void BluezHandleAdvertisementFromDevice(BluezDevice1 * aDevice)
{
    const char * address   = bluez_device1_get_address(aDevice);
    int16_t rssi           = bluez_device1_get_rssi(aDevice);
    GVariant * serviceData = bluez_device1_get_service_data(aDevice);
    GVariant * entry;
    GVariantIter iter;
    ChipAdvType type;
    BluezAddress * src;
    const uint8_t * tmpBuf;
    uint8_t * buf;
    size_t len, dataLen;
    size_t i, j;
    uint16_t serviceId;
    char * debugStr;

    // service data is optional and may not be present
    SuccessOrExit(serviceData != NULL);

    src = g_new0(BluezAddress, 1);

    BluezStringAddressToCHIPAddress(address, src);
    type     = ChipAdvType::BLUEZ_ADV_TYPE_CONNECTABLE;
    debugStr = g_variant_print(serviceData, TRUE);
    ChipLogProgress(DeviceLayer, "TRACE: Device %s Service data: %s", address, debugStr);
    g_free(debugStr);
    // advertising flags
    ChipLogProgress(DeviceLayer, "TRACE: Device %s Advertising flags: %s", address, bluez_device1_get_advertising_flags(aDevice));

    g_variant_iter_init(&iter, serviceData);

    len = 0;
    while ((entry = g_variant_iter_next_value(&iter)) != NULL)
    {
        GVariant * key    = g_variant_get_child_value(entry, 0);
        GVariant * val    = g_variant_get_child_value(entry, 1);
        GVariant * rawVal = g_variant_get_variant(val);

        serviceId = BluezUUIDStringToShortServiceID(g_variant_get_string(key, &dataLen));

        if (serviceId != 0)
        {
            len += 1 + sizeof(uint8_t) + sizeof(serviceId) + g_variant_n_children(rawVal);
        }
    }

    if (len != 0)
    {
        // we only parsed services, add 3 bytes for flags
        len      = len + 3;
        buf      = (uint8_t *) g_malloc(len);
        i        = 0;
        buf[i++] = 2; // length of flags
        buf[i++] = BLUEZ_ADV_TYPE_FLAGS;
        buf[i++] = BLUEZ_ADV_FLAGS_LE_DISCOVERABLE | BLUEZ_ADV_FLAGS_EDR_UNSUPPORTED;
        g_variant_iter_init(&iter, serviceData);
        while ((entry = g_variant_iter_next_value(&iter)) != NULL)
        {
            GVariant * key    = g_variant_get_child_value(entry, 0);
            GVariant * val    = g_variant_get_child_value(entry, 1);
            GVariant * rawVal = g_variant_get_variant(val);
            serviceId         = BluezUUIDStringToShortServiceID(g_variant_get_string(key, &dataLen));

            if (serviceId != 0)
            {
                buf[i++] = g_variant_n_children(rawVal) + 2 + 1;
                buf[i++] = BLUEZ_ADV_TYPE_SERVICE_DATA;
                buf[i++] = serviceId & 0xff;
                buf[i++] = (serviceId >> 8) & 0xff;
                tmpBuf   = (const uint8_t *) g_variant_get_fixed_array(rawVal, &dataLen, sizeof(uint8_t));
                for (j = 0; j < dataLen; j++)
                {
                    buf[i++] = tmpBuf[j];
                }
            }
        }
    }

exit:
    return;
}

static void BluezSignalInterfacePropertiesChanged(GDBusObjectManagerClient * aManager, GDBusObjectProxy * aObject,
                                                  GDBusProxy * aInterface, GVariant * aChangedProperties,
                                                  const gchar * const * aInvalidatedProps, gpointer apClosure)
{

    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(endpoint->mpAdapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL endpoint->mpAdapter in %s", __func__));

    if (strcmp(g_dbus_proxy_get_interface_name(aInterface), CHARACTERISTIC_INTERFACE) == 0)
    {
        BluezGattCharacteristic1 * chr = BLUEZ_GATT_CHARACTERISTIC1(aInterface);
        BluezConnection * conn         = BluezCharacteristicGetBluezConnection(chr, NULL, endpoint);

        if (conn)
        {
            GVariantIter iter;
            GVariant * value;
            char * key;

            g_variant_iter_init(&iter, aChangedProperties);
            while (g_variant_iter_next(&iter, "{&sv}", &key, &value))
            {
                if (strcmp(key, "Value") == 0)
                {
                    gconstpointer data;
                    uint8_t * buf;
                    size_t len;

                    data = g_variant_get_fixed_array(value, &len, sizeof(uint8_t));

                    /* TODO why is otPlat API limited to 255 bytes? */
                    if (len > 255)
                        len = 255;

                    buf = (uint8_t *) g_memdup(data, len);
                }
                g_variant_unref(value);
            }
        }
    }
    else if (strcmp(g_dbus_proxy_get_interface_name(aInterface), DEVICE_INTERFACE) == 0)
    {
        BluezDevice1 * device = BLUEZ_DEVICE1(aInterface);
        GVariantIter iter;
        GVariant * value;
        char * key;

        if (BluezIsDeviceOnAdapter(device, endpoint->mpAdapter))
        {
            BluezConnection * conn =
                (BluezConnection *) g_hash_table_lookup(endpoint->mpConnMap, g_dbus_proxy_get_object_path(aInterface));
            g_variant_iter_init(&iter, aChangedProperties);
            while (g_variant_iter_next(&iter, "{&sv}", &key, &value))
            {
                if (strcmp(key, "Connected") == 0)
                {
                    gboolean connected;
                    connected = g_variant_get_boolean(value);

                    if (connected)
                    {
                        ChipLogProgress(DeviceLayer, "Bluez coonnected");
                        // for a central, the connection has been already allocated.  For a peripheral, it has not.
                        // todo do we need this ? we could handle all connection the same wa...
                        if (endpoint->mIsCentral)
                            SuccessOrExit(conn != NULL);

                        if (!endpoint->mIsCentral)
                        {
                            VerifyOrExit(conn == NULL,
                                         ChipLogProgress(DeviceLayer, "FAIL: connection already tracked: conn: %x device: %s", conn,
                                                         g_dbus_proxy_get_object_path(aInterface)));
                            conn                = g_new0(BluezConnection, 1);
                            conn->mpPeerAddress = g_strdup(bluez_device1_get_address(device));
                            conn->mpDevice      = (BluezDevice1 *) g_object_ref(device);
                            conn->mpEndpoint    = endpoint;
                            BluezConnectionInit(conn);
                            endpoint->mpPeerDevicePath = g_strdup(g_dbus_proxy_get_object_path(aInterface));
                            ChipLogProgress(DeviceLayer, "coonnected, insert, conn:%p, c1:%p, c2:%p and %s", conn, conn->mpC1,
                                            conn->mpC2, endpoint->mpPeerDevicePath);
                            g_hash_table_insert(endpoint->mpConnMap, endpoint->mpPeerDevicePath, conn);
                        }
                        // for central, we do not call BluezConnectionInit until the services have been resolved

                        BLEManagerImpl::CHIPoBluez_NewConnection(endpoint);
                    }
                    else
                    {
                        ChipLogProgress(DeviceLayer, "Bluez disconnected");
                        BLEManagerImpl::CHIPoBluez_ConnectionClosed(endpoint);
                        BluezOTConnectionDestroy(conn);
                        g_hash_table_remove(endpoint->mpConnMap, g_dbus_proxy_get_object_path(aInterface));
                    }
                }
                else if (strcmp(key, "ServicesResolved") == 0)
                {
                    gboolean resolved;
                    resolved = g_variant_get_boolean(value);

                    if (endpoint->mIsCentral && conn != NULL && resolved == TRUE)
                    {
                        /* delay to idle, this is to workaround race in handling
                         * of interface-added and properites-changed signals
                         * it looks like we cannot specify order of those
                         * handlers and currently implementation assumes
                         * that interfaces-added is called first.
                         *
                         * TODO figure out if we can avoid this
                         */
                        g_idle_add(BluezConnectionInitIdle, conn);
                    }
                }
                else if (strcmp(key, "RSSI") == 0)
                {
                    /* when discovery starts we will get this one device is
                     * found even if device object was already present
                     */
                    if (endpoint->mIsCentral)
                    {
                        BluezHandleAdvertisementFromDevice(device);
                    }
                }

                g_variant_unref(value);
            }
        }
    }
exit:
    return;
}

static void BluezHandleNewDevice(BluezDevice1 * device, BluezEndpoint * apEndpoint)
{
    VerifyOrExit(apEndpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    if (apEndpoint->mIsCentral)
    {
        BluezHandleAdvertisementFromDevice(device);
    }
    else
    {
        BluezConnection * conn;
        SuccessOrExit(bluez_device1_get_connected(device));

        conn = (BluezConnection *) g_hash_table_lookup(apEndpoint->mpConnMap, g_dbus_proxy_get_object_path(G_DBUS_PROXY(device)));
        VerifyOrExit(conn == NULL,
                     ChipLogProgress(DeviceLayer, "FAIL: connection already tracked: conn: %x new device: %s", conn,
                                     g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))));

        conn                = g_new0(BluezConnection, 1);
        conn->mpPeerAddress = g_strdup(bluez_device1_get_address(device));
        conn->mpDevice      = (BluezDevice1 *) g_object_ref(device);
        conn->mpEndpoint    = apEndpoint;
        BluezConnectionInit(conn);

        g_hash_table_insert(apEndpoint->mpConnMap, g_strdup(g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))), conn);
    }

exit:
    return;
}

static void BluezSignalOnObjectAdded(GDBusObjectManager * aManager, GDBusObject * aObject, gpointer apClosure)
{
    // TODO: right now we do not handle addition/removal of adapters
    // Primary focus here is to handle addition of a device

    BluezObject * o          = BLUEZ_OBJECT(aObject);
    BluezDevice1 * device    = bluez_object_get_device1(o);
    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
    if (device != NULL)
    {
        if (BluezIsDeviceOnAdapter(device, endpoint->mpAdapter) == TRUE)
        {
            BluezHandleNewDevice(device, endpoint);
        }

        g_object_unref(device);
    }
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
    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;

    endpoint->mpServicePath = g_strdup_printf("%s/service", endpoint->mpRootPath);
    ChipLogProgress(DeviceLayer, "CREATE service object at %s", endpoint->mpServicePath);
    object = bluez_object_skeleton_new(endpoint->mpServicePath);

    service = bluez_gatt_service1_skeleton_new();
    bluez_gatt_service1_set_uuid(service, "0xFEAF");
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
    GList * objects;
    GList * l;
    char * expectedPath = NULL;

    VerifyOrExit(apEndpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    expectedPath = g_strdup_printf("%s/hci%d", BLUEZ_PATH, apEndpoint->mNodeId);
    objects      = g_dbus_object_manager_get_objects(apEndpoint->mpObjMgr);

    for (l = objects; l != NULL && apEndpoint->mpAdapter == NULL; l = l->next)
    {
        BluezObject * object = BLUEZ_OBJECT(l->data);
        GList * interfaces;
        GList * ll;
        interfaces = g_dbus_object_get_interfaces(G_DBUS_OBJECT(object));

        for (ll = interfaces; ll != NULL; ll = ll->next)
        {
            if (BLUEZ_IS_ADAPTER1(ll->data))
            { // we found the adapter
                BluezAdapter1 * adapter = BLUEZ_ADAPTER1(ll->data);
                char * addr             = (char *) bluez_adapter1_get_address(adapter);
                if (apEndpoint->mpAdapterAddr == NULL) // no adapter address provided, bind to the hci indicated by nodeid
                {
                    if (strcmp(g_dbus_proxy_get_object_path(G_DBUS_PROXY(adapter)), expectedPath) == 0)
                    {
                        apEndpoint->mpAdapter = (BluezAdapter1 *) g_object_ref(adapter);
                    }
                }
                else
                {
                    if (strcmp(apEndpoint->mpAdapterAddr, addr) == 0)
                    {
                        apEndpoint->mpAdapter = (BluezAdapter1 *) g_object_ref(adapter);
                    }
                }
            }
        }
        g_list_free_full(interfaces, g_object_unref);
    }
    VerifyOrExit(apEndpoint->mpAdapter != NULL, ChipLogProgress(DeviceLayer, "FAIL: NULL apEndpoint->mpAdapter in %s", __func__));
    bluez_adapter1_set_powered(apEndpoint->mpAdapter, TRUE);

    // with BLE we are discoverable only when advertising so this can be
    // set once on init
    bluez_adapter1_set_discoverable_timeout(apEndpoint->mpAdapter, 0);
    bluez_adapter1_set_discoverable(apEndpoint->mpAdapter, TRUE);

exit:
    g_list_free_full(objects, g_object_unref);
    g_free(expectedPath);
}

static BluezConnection * GetBluezConnectionViaDevice(BluezEndpoint * apEndpoint)
{
    BluezConnection * retval = (BluezConnection *) g_hash_table_lookup(apEndpoint->mpConnMap, apEndpoint->mpPeerDevicePath);
    // ChipLogProgress(DeviceLayer, "acquire connection object %p in (%s)", retval, __func__);
    return retval;
}

static BluezConnection * BluezCharacteristicGetBluezConnection(BluezGattCharacteristic1 * aChar, GVariant * aOptions,
                                                               BluezEndpoint * apEndpoint)
{
    BluezConnection * retval = NULL;
    const gchar * path       = NULL;
    GVariantDict options;
    GVariant * v;

    VerifyOrExit(apEndpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));
    VerifyOrExit(apEndpoint->mIsCentral, );
    ChipLogProgress(DeviceLayer, "loopup BluezCharacteristicGetBluezConnection");
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

        VerifyOrExit(v != NULL, ChipLogProgress(DeviceLayer, "FAIL: No device option in dictionary (%s)", __func__));

        path = g_variant_get_string(v, NULL);

        retval = (BluezConnection *) g_hash_table_lookup(apEndpoint->mpConnMap, path);
    }

exit:
    return retval;
}

void EndpointCleanup(BluezEndpoint * apEndpoint)
{
    if (apEndpoint != NULL)
    {
        if (apEndpoint->mpOwningName != NULL)
        {
            g_free(apEndpoint->mpOwningName);
            apEndpoint->mpOwningName = NULL;
        }
        if (apEndpoint->mpAdapterName != NULL)
        {
            g_free(apEndpoint->mpAdapterName);
            apEndpoint->mpAdapterName = NULL;
        }
        if (apEndpoint->mpAdapterAddr != NULL)
        {
            g_free(apEndpoint->mpAdapterAddr);
            apEndpoint->mpAdapterAddr = NULL;
        }
        if (apEndpoint->mpRootPath != NULL)
        {
            g_free(apEndpoint->mpRootPath);
            apEndpoint->mpRootPath = NULL;
        }
        if (apEndpoint->mpAdvPath != NULL)
        {
            g_free(apEndpoint->mpAdvPath);
            apEndpoint->mpAdvPath = NULL;
        }
        if (apEndpoint->mpServicePath != NULL)
        {
            g_free(apEndpoint->mpServicePath);
            apEndpoint->mpServicePath = NULL;
        }
        if (apEndpoint->mpConnMap != NULL)
        {
            g_hash_table_destroy(apEndpoint->mpConnMap);
            apEndpoint->mpConnMap = NULL;
        }
        if (apEndpoint->mpAdvertisingUUID != NULL)
        {
            g_free(apEndpoint->mpAdvertisingUUID);
            apEndpoint->mpAdvertisingUUID = NULL;
        }
        if (apEndpoint->mpChipServiceData != NULL)
        {
            g_free(apEndpoint->mpChipServiceData);
            apEndpoint->mpChipServiceData = NULL;
        }
        if (apEndpoint->mpPeerDevicePath != NULL)
        {
            g_free(apEndpoint->mpPeerDevicePath);
            apEndpoint->mpPeerDevicePath = NULL;
        }

        g_free(apEndpoint);
    }
}

void BluezObjectsCleanup(BluezEndpoint * apEndpoint)
{
    g_object_unref(apEndpoint->mpAdapter);
    EndpointCleanup(apEndpoint);
}

static void BluezPeripheralObjectsSetup(gpointer apClosure)
{

    static const char * const c1_flags[] = { "write", NULL };
    static const char * const c2_flags[] = { "read", "indicate", NULL };

    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

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

    ChipLogProgress(DeviceLayer, "CHIP BTP C1 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC1));
    ChipLogProgress(DeviceLayer, "CHIP BTP C2 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC2));

exit:
    return;
}

static void BluezOnBusAcquired(GDBusConnection * aConn, const gchar * aName, gpointer apClosure)
{
    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    ChipLogProgress(DeviceLayer, "TRACE: Bus acquired for name %s", aName);

    endpoint->mpRootPath = g_strdup_printf("/chipoble/%04x", getpid() & 0xffff);
    endpoint->mpRoot     = g_dbus_object_manager_server_new(endpoint->mpRootPath);
    g_dbus_object_manager_server_set_connection(endpoint->mpRoot, aConn);
    BluezPeripheralObjectsSetup(apClosure);

exit:
    return;
}

static void BluezOnNameAcquired(GDBusConnection * aConn, const gchar * aName, gpointer apClosure)
{
    ChipLogProgress(DeviceLayer, "TRACE: Owning name: Acquired %s", aName);
}

static void BluezOnNameLost(GDBusConnection * aConn, const gchar * aName, gpointer apClosure)
{
    ChipLogProgress(DeviceLayer, "TRACE: Owning name: lost %s", aName);
}

static void * BluezMainLoop(void * apClosure)
{
    GDBusObjectManager * manager;
    GError * error = NULL;
    guint id;
    GDBusConnection * conn   = NULL;
    BluezEndpoint * endpoint = (BluezEndpoint *) apClosure;
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    VerifyOrExit(conn != NULL, ChipLogProgress(DeviceLayer, "FAIL: get bus sync in %s, error: %s", __func__, error->message));

    if (endpoint->mpAdapterName != NULL)
        endpoint->mpOwningName = g_strdup_printf("%s", endpoint->mpAdapterName);
    else
        endpoint->mpOwningName = g_strdup_printf("C-%04x", getpid() & 0xffff);

    BluezOnBusAcquired(conn, endpoint->mpOwningName, apClosure);

    manager = g_dbus_object_manager_client_new_sync(
        conn, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/", bluez_object_manager_client_get_proxy_type,
        NULL /* unused user data in the Proxy Type Func */, NULL /*destroy notify */, NULL /* cancellable */, &error);

    VerifyOrExit(manager != NULL, ChipLogProgress(DeviceLayer, "FAIL: Error getting object manager client: %s", error->message));

    endpoint->mpObjMgr = manager;

    bluezObjectsSetup(endpoint);

#if 0
    // reenable if we want to handle the bluetoothd restart
    g_signal_connect (manager,
                      "notify::name-owner",
                      G_CALLBACK (on_notify_name_owner),
                      NULL);
#endif
    g_signal_connect(manager, "object-added", G_CALLBACK(BluezSignalOnObjectAdded), apClosure);
    g_signal_connect(manager, "object-removed", G_CALLBACK(BluezSignalOnObjectRemoved), apClosure);
    g_signal_connect(manager, "interface-proxy-properties-changed", G_CALLBACK(BluezSignalInterfacePropertiesChanged), apClosure);

    ChipLogProgress(DeviceLayer, "TRACE: Bluez mainloop starting %s", __func__);
    g_main_loop_run(sBluezMainLoop);
    ChipLogProgress(DeviceLayer, "TRACE: Bluez mainloop stopping %s", __func__);

    g_bus_unown_name(id);
    BluezObjectsCleanup(endpoint);

exit:
    if (error != NULL)
        g_error_free(error);
    return NULL;
}

bool BluezRunOnBluezThread(int (*aCallback)(void *), void * apClosure)
{
    GMainContext * context = NULL;
    const char * msg       = NULL;

    VerifyOrExit(sBluezMainLoop != NULL, msg = "FAIL: NULL sBluezMainLoop");
    VerifyOrExit(g_main_loop_is_running(sBluezMainLoop), msg = "FAIL: sBluezMainLoop not running");

    context = g_main_loop_get_context(sBluezMainLoop);
    VerifyOrExit(context != NULL, msg = "FAIL: NULL main context");
    g_main_context_invoke(context, aCallback, apClosure);

exit:
    if (msg != NULL)
    {
        ChipLogProgress(DeviceLayer, "%s in %s", msg, __func__);
    }

    return msg == NULL;
}

static gboolean BluezC2Indicate(void * apClosure)
{
    ConnectionDataBundle * closure = NULL;
    BluezConnection * conn         = NULL;
    GError * error                 = NULL;
    GIOStatus status;
    const char * buf;
    size_t len, written;

    closure = static_cast<ConnectionDataBundle *>(apClosure);
    VerifyOrExit(closure != NULL, ChipLogProgress(DeviceLayer, "ConnectionDataBundle is NULL in %s", __func__));

    conn = closure->mpConn;
    VerifyOrExit(conn != NULL, ChipLogProgress(DeviceLayer, "BluezConnection is NULL in %s", __func__));
    VerifyOrExit(conn->mpC2 != NULL, ChipLogProgress(DeviceLayer, "FAIL: C2 Indicate: %s", "NULL C2"));

    if (bluez_gatt_characteristic1_get_notify_acquired(conn->mpC2) == TRUE)
    {
        buf    = (char *) g_variant_get_fixed_array(closure->mpVal, &len, sizeof(uint8_t));
        status = g_io_channel_write_chars(conn->mC2Channel.mpChannel, buf, len, &written, &error);
        g_variant_unref(closure->mpVal);
        closure->mpVal = NULL;

        VerifyOrExit(status == G_IO_STATUS_NORMAL, ChipLogProgress(DeviceLayer, "FAIL: C2 Indicate: %s", error->message));
    }
    else
    {
        bluez_gatt_characteristic1_set_value(conn->mpC2, closure->mpVal);
        closure->mpVal = NULL;
    }

exit:
    if (closure != NULL)
    {
        if (closure->mpVal)
        {
            g_variant_unref(closure->mpVal);
        }
        g_free(closure);
    }

    if (error != NULL)
        g_error_free(error);
    return G_SOURCE_REMOVE;
}

bool SendBluezIndication(void * apConn, chip::System::PacketBuffer * apBuf)
{
    ConnectionDataBundle * closure;
    const char * msg = NULL;
    bool success     = false;
    uint8_t * buffer = NULL;
    size_t len       = 0;

    VerifyOrExit(apBuf != NULL, ChipLogProgress(DeviceLayer, "apBuf is NULL in %s", __func__));
    buffer = apBuf->Start();
    len    = apBuf->DataLength();

    closure         = g_new(ConnectionDataBundle, 1);
    closure->mpConn = static_cast<BluezConnection *>(apConn);

    closure->mpVal = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, buffer, len * sizeof(uint8_t), sizeof(uint8_t));

    success = BluezRunOnBluezThread(BluezC2Indicate, closure);

exit:
    if (NULL != msg)
    {
        ChipLogError(Ble, msg);
    }

    if (NULL != apBuf)
    {
        chip::System::PacketBuffer::Free(apBuf);
    }

    return success;
}

static gboolean BluezDisconnect(void * apClosure)
{
    BluezConnection * conn = static_cast<BluezConnection *>(apClosure);
    GError * error         = NULL;
    gboolean success;

    VerifyOrExit(conn != NULL, ChipLogProgress(DeviceLayer, "conn is NULL in %s", __func__));
    VerifyOrExit(conn->mpDevice != NULL, ChipLogProgress(DeviceLayer, "FAIL: Disconnect: %s", "NULL Device"));

    ChipLogProgress(DeviceLayer, "%s peer=%s", __func__, bluez_device1_get_address(conn->mpDevice));

    success = bluez_device1_call_disconnect_sync(conn->mpDevice, NULL, &error);
    VerifyOrExit(success == TRUE, ChipLogProgress(DeviceLayer, "FAIL: Disconnect: %s", error->message));

exit:
    if (error != NULL)
        g_error_free(error);
    return G_SOURCE_REMOVE;
}

static int CloseBleconnectionCB(void * apAppState)
{
    BluezDisconnect(apAppState);
    return G_SOURCE_REMOVE;
}

bool CloseBluezConnection(void * apAppState)
{
    return BluezRunOnBluezThread(CloseBleconnectionCB, apAppState);
}

CHIP_ERROR StartBluezAdv(void * apAppState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!BluezRunOnBluezThread(BluezAdvStart, apAppState))
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(Ble, "Failed to schedule BluezAdvStart() on CHIPoBluez thread");
    }
    return err;
}

CHIP_ERROR StopBluezAdv(void * apAppState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!BluezRunOnBluezThread(BluezAdvSetup, apAppState))
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(Ble, "Failed to schedule BluezAdvStop() on CHIPoBluez thread");
    }
    return err;
}

CHIP_ERROR BluezAdvertisementSetup(void * apAppState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!BluezRunOnBluezThread(BluezAdvSetup, apAppState))
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(Ble, "Failed to schedule BluezAdvertisementSetup() on CHIPoBluez thread");
    }
    return err;
}

CHIP_ERROR BluezGattsAppRegister(void * apAppState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!BluezRunOnBluezThread(BluezPeripheralRegisterApp, apAppState))
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(Ble, "Failed to schedule BluezPeripheralRegisterApp() on CHIPoBluez thread");
    }
    return err;
}

CHIP_ERROR ConfigureBluezAdv(BLEAdvConfig & aBleAdvConfig, BluezEndpoint * apEndpoint)
{
    const char * msg = NULL;
    CHIP_ERROR err   = CHIP_NO_ERROR;
    VerifyOrExit(aBleAdvConfig.mpBleName != NULL, msg = "FAIL: BLE name is NULL");
    VerifyOrExit(aBleAdvConfig.mpAdvertisingUUID != NULL, msg = "FAIL: BLE mpAdvertisingUUID is NULL in %s");

    apEndpoint->mpAdapterName                             = g_strdup(aBleAdvConfig.mpBleName);
    apEndpoint->mpAdvertisingUUID                         = g_strdup(aBleAdvConfig.mpAdvertisingUUID);
    apEndpoint->mNodeId                                   = aBleAdvConfig.mNodeId;
    apEndpoint->mType                                     = aBleAdvConfig.mType;
    apEndpoint->mDuration                                 = aBleAdvConfig.mDuration;
    apEndpoint->mpChipServiceData                         = static_cast<CHIPServiceData *>(g_malloc(sizeof(CHIPServiceData) + 1));
    apEndpoint->mpChipServiceData->mDataBlock0Len         = sizeof(CHIPIdInfo) + 1;
    apEndpoint->mpChipServiceData->mDataBlock0Type        = 1;
    apEndpoint->mpChipServiceData->mIdInfo.mMajor         = aBleAdvConfig.mMajor;
    apEndpoint->mpChipServiceData->mIdInfo.mMinor         = aBleAdvConfig.mMinor;
    apEndpoint->mpChipServiceData->mIdInfo.mVendorId      = aBleAdvConfig.mVendorId;
    apEndpoint->mpChipServiceData->mIdInfo.mProductId     = aBleAdvConfig.mProductId;
    apEndpoint->mpChipServiceData->mIdInfo.mDeviceId      = aBleAdvConfig.mDeviceId;
    apEndpoint->mpChipServiceData->mIdInfo.mPairingStatus = aBleAdvConfig.mPairingStatus;
    apEndpoint->mDuration                                 = aBleAdvConfig.mDuration;

exit:
    if (NULL != msg)
    {
        ChipLogProgress(DeviceLayer, "%s in %s", msg, __func__);
        err = CHIP_ERROR_INCORRECT_STATE;
        if (apEndpoint->mpChipServiceData != NULL)
        {
            g_free(apEndpoint->mpChipServiceData);
            apEndpoint->mpChipServiceData = NULL;
        }
    }
    return err;
}

CHIP_ERROR InitBluezBleLayer(bool aIsCentral, char * apBleAddr, BLEAdvConfig & aBleAdvConfig, void *& apEndpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool retval    = false;
    int pthreadErr = 0;
    int tmpErrno;
    BluezEndpoint * endpoint = NULL;

    VerifyOrExit(pipe2(sBluezFD, O_DIRECT) == 0, ChipLogProgress(DeviceLayer, "FAIL: open pipe in %s", __func__));

    // initialize server endpoint
    endpoint = g_new0(BluezEndpoint, 1);
    VerifyOrExit(endpoint != NULL, ChipLogProgress(DeviceLayer, "FAIL: memory allocation in %s", __func__));

    if (apBleAddr != NULL)
        endpoint->mpAdapterAddr = g_strdup(apBleAddr);
    else
        endpoint->mpAdapterAddr = NULL;

    endpoint->mpConnMap  = g_hash_table_new(g_str_hash, g_str_equal);
    endpoint->mIsCentral = aIsCentral;

    err = ConfigureBluezAdv(aBleAdvConfig, endpoint);
    SuccessOrExit(err);

    sBluezMainLoop = g_main_loop_new(NULL, FALSE);
    VerifyOrExit(sBluezMainLoop != NULL, ChipLogProgress(DeviceLayer, "FAIL: memory alloc in %s", __func__));

    pthreadErr = pthread_create(&sBluezThread, NULL, BluezMainLoop, endpoint);
    tmpErrno   = errno;
    VerifyOrExit(pthreadErr == 0, ChipLogProgress(DeviceLayer, "FAIL: pthread_create (%s) in %s", strerror(tmpErrno), __func__));
    sleep(1);

    retval = TRUE;

exit:
    if (retval)
    {
        apEndpoint = endpoint;
        ChipLogProgress(DeviceLayer, "PlatformBlueZInit init success");
    }
    else
    {
        EndpointCleanup(endpoint);
    }

    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
