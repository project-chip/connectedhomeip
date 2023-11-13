/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include "BluezEndpoint.h"

#include <cassert>
#include <cstring>
#include <errno.h>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib-object.h>
#include <glib.h>

#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/PlatformManager.h>
#include <platform/internal/BLEManager.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <system/SystemPacketBuffer.h>

#include "BluezConnection.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

constexpr uint16_t kMaxConnectRetries = 4;

static BluezConnection * GetBluezConnectionViaDevice(BluezEndpoint * apEndpoint);

static gboolean BluezCharacteristicReadValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                             GVariant * aOptions)
{
    GVariant * val;
    ChipLogDetail(DeviceLayer, "Received BluezCharacteristicReadValue");
    val = bluez_gatt_characteristic1_get_value(aChar);
    bluez_gatt_characteristic1_complete_read_value(aChar, aInvocation, val);
    return TRUE;
}

static void Bluez_gatt_characteristic1_complete_acquire_write_with_fd(GDBusMethodInvocation * invocation, int fd, guint16 mtu)
{
    GUnixFDList * fd_list = g_unix_fd_list_new();
    int index             = g_unix_fd_list_append(fd_list, fd, nullptr);
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, g_variant_new("(@hq)", g_variant_new_handle(index), mtu),
                                                            fd_list);
    g_object_unref(fd_list);
}

static gboolean BluezCharacteristicAcquireWrite(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                GVariant * aOptions, gpointer apEndpoint)
{
    int fds[2] = { -1, -1 };
#if CHIP_ERROR_LOGGING
    char * errStr;
#endif // CHIP_ERROR_LOGGING
    BluezConnection * conn = nullptr;
    GAutoPtr<GVariant> option_mtu;
    uint16_t mtu;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);

    VerifyOrReturnValue(endpoint != nullptr, FALSE, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrReturnValue(
        conn != nullptr, FALSE,
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    ChipLogDetail(DeviceLayer, "BluezCharacteristicAcquireWrite is called, conn: %p", conn);

    VerifyOrReturnValue(
        g_variant_lookup(aOptions, "mtu", "q", &mtu), FALSE, ChipLogError(DeviceLayer, "FAIL: No MTU in options in %s", __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.InvalidArguments", "MTU negotiation failed"));
    conn->SetMTU(mtu);

    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) < 0)
    {
#if CHIP_ERROR_LOGGING
        errStr = strerror(errno);
#endif // CHIP_ERROR_LOGGING
        ChipLogError(DeviceLayer, "FAIL: socketpair: %s in %s", StringOrNullMarker(errStr), __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "FD creation failed");
        return FALSE;
    }

    conn->SetupWriteHandler(fds[0]);
    bluez_gatt_characteristic1_set_write_acquired(aChar, TRUE);

    Bluez_gatt_characteristic1_complete_acquire_write_with_fd(aInvocation, fds[1], conn->GetMTU());
    close(fds[1]);

    return TRUE;
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
#if CHIP_ERROR_LOGGING
    char * errStr;
#endif // CHIP_ERROR_LOGGING
    BluezConnection * conn       = nullptr;
    bool isAdditionalAdvertising = false;
    GAutoPtr<GVariant> option_mtu;
    uint16_t mtu;

    BluezEndpoint * endpoint = static_cast<BluezEndpoint *>(apEndpoint);
    VerifyOrReturnValue(endpoint != nullptr, FALSE, ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    isAdditionalAdvertising = (aChar == endpoint->mpC3);
#endif

    if (bluez_gatt_characteristic1_get_notifying(aChar))
    {
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotPermitted", "Already notifying");
        return FALSE;
    }

    conn = GetBluezConnectionViaDevice(endpoint);
    VerifyOrReturnValue(
        conn != nullptr, FALSE,
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No Chipoble connection"));

    VerifyOrReturnValue(
        g_variant_lookup(aOptions, "mtu", "q", &mtu), FALSE, ChipLogError(DeviceLayer, "FAIL: No MTU in options in %s", __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.InvalidArguments", "MTU negotiation failed"););
    conn->SetMTU(mtu);

    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) < 0)
    {
#if CHIP_ERROR_LOGGING
        errStr = strerror(errno);
#endif // CHIP_ERROR_LOGGING
        ChipLogError(DeviceLayer, "FAIL: socketpair: %s in %s", StringOrNullMarker(errStr), __func__);
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "FD creation failed");
        return FALSE;
    }

    conn->SetupNotifyHandler(fds[0], isAdditionalAdvertising);
    bluez_gatt_characteristic1_set_notify_acquired(aChar, TRUE);
    conn->SetNotifyAcquired(true);

    // same reply as for AcquireWrite
    Bluez_gatt_characteristic1_complete_acquire_write_with_fd(aInvocation, fds[1], conn->GetMTU());
    close(fds[1]);

    BLEManagerImpl::HandleTXCharCCCDWrite(conn);

    return TRUE;
}

static gboolean BluezCharacteristicAcquireNotifyError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                      GVariant * aOptions)
{
    ChipLogDetail(DeviceLayer, "TRACE: AcquireNotify is called");
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireNotify for characteristic is unsupported");
    return TRUE;
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

static gboolean BluezCharacteristicConfirmError(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation)
{
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "Confirm from characteristic is unsupported");
    return TRUE;
}

static gboolean BluezIsDeviceOnAdapter(BluezDevice1 * aDevice, BluezAdapter1 * aAdapter)
{
    return strcmp(bluez_device1_get_adapter(aDevice), g_dbus_proxy_get_object_path(G_DBUS_PROXY(aAdapter))) == 0 ? TRUE : FALSE;
}

static BluezGattCharacteristic1 * BluezCharacteristicCreate(BluezGattService1 * aService, const char * aCharName,
                                                            const char * aUUID, GDBusObjectManagerServer * aRoot)
{
    const char * servicePath = g_dbus_object_get_object_path(g_dbus_interface_get_object(G_DBUS_INTERFACE(aService)));
    char * charPath          = g_strdup_printf("%s/%s", servicePath, aCharName);
    BluezObjectSkeleton * object;
    BluezGattCharacteristic1 * characteristic;

    ChipLogDetail(DeviceLayer, "Create characteristic object at %s", charPath);
    object = bluez_object_skeleton_new(charPath);

    characteristic = bluez_gatt_characteristic1_skeleton_new();
    bluez_gatt_characteristic1_set_uuid(characteristic, aUUID);
    bluez_gatt_characteristic1_set_service(characteristic, servicePath);

    bluez_object_skeleton_set_gatt_characteristic1(object, characteristic);
    g_dbus_object_manager_server_export(aRoot, G_DBUS_OBJECT_SKELETON(object));

    g_free(charPath);
    g_object_unref(object);

    return characteristic;
}

static void BluezPeripheralRegisterAppDone(GObject * aObject, GAsyncResult * aResult, gpointer apClosure)
{
    GAutoPtr<GError> error;
    BluezGattManager1 * gattMgr = BLUEZ_GATT_MANAGER1(aObject);

    gboolean success =
        bluez_gatt_manager1_call_register_application_finish(gattMgr, aResult, &MakeUniquePointerReceiver(error).Get());

    VerifyOrReturn(success == TRUE, {
        ChipLogError(DeviceLayer, "FAIL: RegisterApplication : %s", error->message);
        BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(false, nullptr);
    });

    BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(true, nullptr);
    ChipLogDetail(DeviceLayer, "BluezPeripheralRegisterAppDone done");
}

static CHIP_ERROR BluezPeripheralRegisterApp(BluezEndpoint * endpoint)
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
    return CHIP_NO_ERROR;
}

/// Update the table of open BLE connections whenever a new device is spotted or its attributes have changed.
static void UpdateConnectionTable(BluezDevice1 * apDevice, BluezEndpoint & aEndpoint)
{
    const char * objectPath      = g_dbus_proxy_get_object_path(G_DBUS_PROXY(apDevice));
    BluezConnection * connection = static_cast<BluezConnection *>(g_hash_table_lookup(aEndpoint.mpConnMap, objectPath));

    if (connection != nullptr && !bluez_device1_get_connected(apDevice))
    {
        ChipLogDetail(DeviceLayer, "Bluez disconnected");
        BLEManagerImpl::CHIPoBluez_ConnectionClosed(connection);
        // TODO: the connection object should be released after BLEManagerImpl finishes cleaning up its resources
        // after the disconnection. Releasing it here doesn't cause any issues, but it's error-prone.
        chip::Platform::Delete(connection);
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
        connection                 = chip::Platform::New<BluezConnection>(&aEndpoint, apDevice);
        aEndpoint.mpPeerDevicePath = g_strdup(objectPath);
        g_hash_table_insert(aEndpoint.mpConnMap, aEndpoint.mpPeerDevicePath, connection);

        ChipLogDetail(DeviceLayer, "New BLE connection: conn %p, device %s, path %s", connection, connection->GetPeerAddress(),
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
    // When a device is connected for first time, this function will be triggered.
    // The future connections for the same device will trigger ``Connect'' property change.
    // TODO: Factor common code in the two function.
    BluezConnection * conn;
    VerifyOrExit(bluez_device1_get_connected(device), ChipLogError(DeviceLayer, "FAIL: device is not connected"));

    conn = static_cast<BluezConnection *>(
        g_hash_table_lookup(apEndpoint->mpConnMap, g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))));
    VerifyOrExit(conn == nullptr,
                 ChipLogError(DeviceLayer, "FAIL: connection already tracked: conn: %p new device: %s", conn,
                              g_dbus_proxy_get_object_path(G_DBUS_PROXY(device))));

    conn                         = chip::Platform::New<BluezConnection>(apEndpoint, device);
    apEndpoint->mpPeerDevicePath = g_strdup(g_dbus_proxy_get_object_path(G_DBUS_PROXY(device)));
    g_hash_table_insert(apEndpoint->mpConnMap, apEndpoint->mpPeerDevicePath, conn);

    ChipLogDetail(DeviceLayer, "BLE device connected: conn %p, device %s, path %s", conn, conn->GetPeerAddress(),
                  apEndpoint->mpPeerDevicePath);

exit:
    return;
}

static void BluezSignalOnObjectAdded(GDBusObjectManager * aManager, GDBusObject * aObject, BluezEndpoint * endpoint)
{
    // TODO: right now we do not handle addition/removal of adapters
    // Primary focus here is to handle addition of a device
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(BLUEZ_OBJECT(aObject)));

    VerifyOrReturn(device.get() != nullptr);

    if (BluezIsDeviceOnAdapter(device.get(), endpoint->mpAdapter) == TRUE)
    {
        BluezHandleNewDevice(device.get(), endpoint);
    }
}

static void BluezSignalOnObjectRemoved(GDBusObjectManager * aManager, GDBusObject * aObject, gpointer apClosure)
{
    // TODO: for Device1, lookup connection, and call otPlatTobleHandleDisconnected
    // for Adapter1: unclear, crash if this pertains to our adapter? at least null out the endpoint->mpAdapter.
    // for Characteristic1, or GattService -- handle here via calling otPlatTobleHandleDisconnected, or ignore.
}

static BluezGattService1 * BluezServiceCreate(BluezEndpoint * endpoint)
{
    BluezObjectSkeleton * object;
    BluezGattService1 * service;

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

static void EndpointCleanup(BluezEndpoint * apEndpoint)
{
    if (apEndpoint != nullptr)
    {
        g_free(apEndpoint->mpOwningName);
        g_free(apEndpoint->mpAdapterName);
        g_free(apEndpoint->mpAdapterAddr);
        g_free(apEndpoint->mpRootPath);
        g_free(apEndpoint->mpServicePath);
        if (apEndpoint->mpObjMgr != nullptr)
            g_object_unref(apEndpoint->mpObjMgr);
        if (apEndpoint->mpAdapter != nullptr)
            g_object_unref(apEndpoint->mpAdapter);
        if (apEndpoint->mpDevice != nullptr)
            g_object_unref(apEndpoint->mpDevice);
        if (apEndpoint->mpRoot != nullptr)
            g_object_unref(apEndpoint->mpRoot);
        if (apEndpoint->mpService != nullptr)
            g_object_unref(apEndpoint->mpService);
        if (apEndpoint->mpC1 != nullptr)
            g_object_unref(apEndpoint->mpC1);
        if (apEndpoint->mpC2 != nullptr)
            g_object_unref(apEndpoint->mpC2);
        if (apEndpoint->mpC3 != nullptr)
            g_object_unref(apEndpoint->mpC3);
        if (apEndpoint->mpConnMap != nullptr)
            g_hash_table_destroy(apEndpoint->mpConnMap);
        g_free(apEndpoint->mpPeerDevicePath);
        if (apEndpoint->mpConnectCancellable != nullptr)
            g_object_unref(apEndpoint->mpConnectCancellable);
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

static void BluezPeripheralObjectsSetup(BluezEndpoint * endpoint)
{

    static const char * const c1_flags[] = { "write", nullptr };
    static const char * const c2_flags[] = { "read", "indicate", nullptr };
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    static const char * const c3_flags[] = { "read", nullptr };
#endif

    endpoint->mpService = BluezServiceCreate(endpoint);
    // C1 characteristic
    endpoint->mpC1 = BluezCharacteristicCreate(endpoint->mpService, "c1", CHIP_PLAT_BLE_UUID_C1_STRING, endpoint->mpRoot);
    bluez_gatt_characteristic1_set_flags(endpoint->mpC1, c1_flags);

    g_signal_connect(endpoint->mpC1, "handle-read-value", G_CALLBACK(BluezCharacteristicReadValue), endpoint);
    g_signal_connect(endpoint->mpC1, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWrite), endpoint);
    g_signal_connect(endpoint->mpC1, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotifyError), nullptr);
    g_signal_connect(endpoint->mpC1, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirmError), nullptr);

    endpoint->mpC2 = BluezCharacteristicCreate(endpoint->mpService, "c2", CHIP_PLAT_BLE_UUID_C2_STRING, endpoint->mpRoot);
    bluez_gatt_characteristic1_set_flags(endpoint->mpC2, c2_flags);
    g_signal_connect(endpoint->mpC2, "handle-read-value", G_CALLBACK(BluezCharacteristicReadValue), endpoint);
    g_signal_connect(endpoint->mpC2, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWriteError), nullptr);
    g_signal_connect(endpoint->mpC2, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotify), endpoint);
    g_signal_connect(endpoint->mpC2, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirm), endpoint);

    ChipLogDetail(DeviceLayer, "CHIP BTP C1 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC1));
    ChipLogDetail(DeviceLayer, "CHIP BTP C2 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC2));

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ChipLogDetail(DeviceLayer, "CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING is TRUE");
    // Additional data characteristics
    endpoint->mpC3 = BluezCharacteristicCreate(endpoint->mpService, "c3", CHIP_PLAT_BLE_UUID_C3_STRING, endpoint->mpRoot);
    bluez_gatt_characteristic1_set_flags(endpoint->mpC3, c3_flags);
    g_signal_connect(endpoint->mpC3, "handle-read-value", G_CALLBACK(BluezCharacteristicReadValue), endpoint);
    g_signal_connect(endpoint->mpC3, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWriteError), nullptr);
    g_signal_connect(endpoint->mpC3, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotify), endpoint);
    g_signal_connect(endpoint->mpC3, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirm), endpoint);
    // update the characteristic value
    UpdateAdditionalDataCharacteristic(endpoint->mpC3);
    ChipLogDetail(DeviceLayer, "CHIP BTP C3 %s", bluez_gatt_characteristic1_get_service(endpoint->mpC3));
#else
    ChipLogDetail(DeviceLayer, "CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING is FALSE");
#endif
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

        BluezPeripheralObjectsSetup(endpoint);
    }

exit:
    return;
}

static CHIP_ERROR StartupEndpointBindings(BluezEndpoint * endpoint)
{
    VerifyOrReturnError(endpoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(DeviceLayer, "endpoint is NULL in %s", __func__));

    GAutoPtr<GError> err;
    GAutoPtr<GDBusConnection> conn(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &MakeUniquePointerReceiver(err).Get()));
    VerifyOrReturnError(conn != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: get bus sync in %s, error: %s", __func__, err->message));

    if (endpoint->mpAdapterName != nullptr)
        endpoint->mpOwningName = g_strdup_printf("%s", endpoint->mpAdapterName);
    else
        endpoint->mpOwningName = g_strdup_printf("C-%04x", getpid() & 0xffff);

    BluezOnBusAcquired(conn.get(), endpoint->mpOwningName, endpoint);

    GDBusObjectManager * manager = g_dbus_object_manager_client_new_sync(
        conn.get(), G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/", bluez_object_manager_client_get_proxy_type,
        nullptr /* unused user data in the Proxy Type Func */, nullptr /*destroy notify */, nullptr /* cancellable */,
        &MakeUniquePointerReceiver(err).Get());
    VerifyOrReturnError(manager != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Error getting object manager client: %s", err->message));

    endpoint->mpObjMgr = manager;
    bluezObjectsSetup(endpoint);

    g_signal_connect(manager, "object-added", G_CALLBACK(BluezSignalOnObjectAdded), endpoint);
    g_signal_connect(manager, "object-removed", G_CALLBACK(BluezSignalOnObjectRemoved), endpoint);
    g_signal_connect(manager, "interface-proxy-properties-changed", G_CALLBACK(BluezSignalInterfacePropertiesChanged), endpoint);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezGattsAppRegister(BluezEndpoint * apEndpoint)
{
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(BluezPeripheralRegisterApp, apEndpoint);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Ble, "Failed to schedule BluezPeripheralRegisterApp() on CHIPoBluez thread"));
    return err;
}

CHIP_ERROR InitBluezBleLayer(uint32_t aAdapterId, bool aIsCentral, const char * apBleAddr, const char * apBleName,
                             BluezEndpoint *& apEndpoint)
{
    BluezEndpoint * endpoint = g_new0(BluezEndpoint, 1);
    CHIP_ERROR err           = CHIP_NO_ERROR;

    if (apBleAddr != nullptr)
        endpoint->mpAdapterAddr = g_strdup(apBleAddr);

    endpoint->mpConnMap  = g_hash_table_new(g_str_hash, g_str_equal);
    endpoint->mAdapterId = aAdapterId;
    endpoint->mIsCentral = aIsCentral;

    if (!aIsCentral)
    {
        endpoint->mpAdapterName = g_strdup(apBleName);
    }
    else
    {
        endpoint->mpConnectCancellable = g_cancellable_new();
    }

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(StartupEndpointBindings, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to schedule endpoint initialization"));

exit:
    if (err == CHIP_NO_ERROR)
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
    // Run endpoint cleanup on the CHIPoBluez thread. This is necessary because the
    // cleanup function releases the D-Bus manager client object, which handles D-Bus
    // signals. Otherwise, we will face race condition when the D-Bus signal is in
    // the middle of being processed when the cleanup function is called.
    return PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezEndpoint * endpoint) {
            EndpointCleanup(endpoint);
            return CHIP_NO_ERROR;
        },
        apEndpoint);
}

// ConnectDevice callbacks

struct ConnectParams
{
    ConnectParams(BluezDevice1 * device, BluezEndpoint * endpoint) :
        mDevice(reinterpret_cast<BluezDevice1 *>(g_object_ref(device))), mEndpoint(endpoint), mNumRetries(0)
    {}
    ~ConnectParams() { g_object_unref(mDevice); }
    BluezDevice1 * mDevice;
    BluezEndpoint * mEndpoint;
    uint16_t mNumRetries;
};

static void ConnectDeviceDone(GObject * aObject, GAsyncResult * aResult, gpointer apParams)
{
    BluezDevice1 * device = BLUEZ_DEVICE1(aObject);
    GAutoPtr<GError> error;
    gboolean success       = bluez_device1_call_connect_finish(device, aResult, &MakeUniquePointerReceiver(error).Get());
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
        if (g_error_matches(error.get(), G_IO_ERROR, G_IO_ERROR_DBUS_ERROR) && params->mNumRetries++ < kMaxConnectRetries)
        {
            // Clear the error before usage in subsequent call.
            g_clear_error(&MakeUniquePointerReceiver(error).Get());

            bluez_device1_call_disconnect_sync(device, nullptr, &MakeUniquePointerReceiver(error).Get());
            bluez_device1_call_connect(device, params->mEndpoint->mpConnectCancellable, ConnectDeviceDone, params);
            return;
        }

        BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "ConnectDevice complete");
    }

    chip::Platform::Delete(params);
}

static CHIP_ERROR ConnectDeviceImpl(ConnectParams * apParams)
{
    BluezDevice1 * device    = apParams->mDevice;
    BluezEndpoint * endpoint = apParams->mEndpoint;

    assert(device != nullptr);
    assert(endpoint != nullptr);

    g_cancellable_reset(endpoint->mpConnectCancellable);
    bluez_device1_call_connect(device, endpoint->mpConnectCancellable, ConnectDeviceDone, apParams);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectDevice(BluezDevice1 & aDevice, BluezEndpoint * apEndpoint)
{
    auto params = chip::Platform::New<ConnectParams>(&aDevice, apEndpoint);
    VerifyOrReturnError(params != nullptr, CHIP_ERROR_NO_MEMORY);

    if (PlatformMgrImpl().GLibMatterContextInvokeSync(ConnectDeviceImpl, params) != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule ConnectDeviceImpl() on CHIPoBluez thread");
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
