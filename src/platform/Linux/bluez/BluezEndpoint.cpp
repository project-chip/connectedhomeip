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

#include <cstring>
#include <errno.h>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

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
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <platform/PlatformManager.h>
#include <platform/internal/BLEManager.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <system/SystemPacketBuffer.h>

#include "BluezConnection.h"
#include "BluezObjectList.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

constexpr uint16_t kMaxConnectRetries = 4;

gboolean BluezEndpoint::BluezCharacteristicReadValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                     GVariant * aOptions)
{
    ChipLogDetail(DeviceLayer, "Received %s", __func__);
    GVariant * val = bluez_gatt_characteristic1_get_value(aChar);
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

gboolean BluezEndpoint::BluezCharacteristicAcquireWrite(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                        GVariant * aOptions)
{
    int fds[2] = { -1, -1 };
#if CHIP_ERROR_LOGGING
    char * errStr;
#endif // CHIP_ERROR_LOGGING
    BluezConnection * conn = nullptr;
    GAutoPtr<GVariant> option_mtu;
    uint16_t mtu;

    conn = GetBluezConnectionViaDevice();
    VerifyOrReturnValue(
        conn != nullptr, FALSE,
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No CHIPoBLE connection"));

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
    ChipLogDetail(DeviceLayer, "Received %s", __func__);
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireWrite for characteristic is unsupported");
    return TRUE;
}

gboolean BluezEndpoint::BluezCharacteristicAcquireNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation,
                                                         GVariant * aOptions)
{
    int fds[2] = { -1, -1 };
#if CHIP_ERROR_LOGGING
    char * errStr;
#endif // CHIP_ERROR_LOGGING
    BluezConnection * conn       = nullptr;
    bool isAdditionalAdvertising = false;
    GAutoPtr<GVariant> option_mtu;
    uint16_t mtu;

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    isAdditionalAdvertising = (aChar == mpC3);
#endif

    if (bluez_gatt_characteristic1_get_notifying(aChar))
    {
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotPermitted", "Already notifying");
        return FALSE;
    }

    conn = GetBluezConnectionViaDevice();
    VerifyOrReturnValue(
        conn != nullptr, FALSE,
        g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.Failed", "No CHIPoBLE connection"));

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
    ChipLogDetail(DeviceLayer, "Received %s", __func__);
    g_dbus_method_invocation_return_dbus_error(aInvocation, "org.bluez.Error.NotSupported",
                                               "AcquireNotify for characteristic is unsupported");
    return TRUE;
}

gboolean BluezEndpoint::BluezCharacteristicConfirm(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInvocation)
{
    BluezConnection * conn = GetBluezConnectionViaDevice();
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

BluezGattCharacteristic1 * BluezEndpoint::CreateGattCharacteristic(BluezGattService1 * aService, const char * aCharName,
                                                                   const char * aUUID, const char * const * aFlags)
{
    const char * servicePath = g_dbus_object_get_object_path(g_dbus_interface_get_object(G_DBUS_INTERFACE(aService)));
    GAutoPtr<char> charPath(g_strdup_printf("%s/%s", servicePath, aCharName));
    BluezObjectSkeleton * object;
    BluezGattCharacteristic1 * characteristic;

    ChipLogDetail(DeviceLayer, "Create characteristic object at %s", charPath.get());
    object = bluez_object_skeleton_new(charPath.get());

    characteristic = bluez_gatt_characteristic1_skeleton_new();
    bluez_gatt_characteristic1_set_uuid(characteristic, aUUID);
    bluez_gatt_characteristic1_set_flags(characteristic, aFlags);
    bluez_gatt_characteristic1_set_service(characteristic, servicePath);

    // Initialize value to empty array, so it can be read without prior write from the client side.
    auto value = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, nullptr, 0, sizeof(uint8_t));
    bluez_gatt_characteristic1_set_value(characteristic, value);

    bluez_object_skeleton_set_gatt_characteristic1(object, characteristic);
    g_dbus_object_manager_server_export(mpRoot, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    return characteristic;
}

void BluezEndpoint::RegisterGattApplicationDone(GObject * aObject, GAsyncResult * aResult)
{
    GAutoPtr<GError> error;
    gboolean success = bluez_gatt_manager1_call_register_application_finish(reinterpret_cast<BluezGattManager1 *>(aObject), aResult,
                                                                            &error.GetReceiver());

    VerifyOrReturn(success == TRUE, {
        ChipLogError(DeviceLayer, "FAIL: RegisterApplication : %s", error->message);
        BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(false);
    });

    BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(true);
    ChipLogDetail(DeviceLayer, "BluezPeripheralRegisterAppDone done");
}

CHIP_ERROR BluezEndpoint::RegisterGattApplicationImpl()
{
    GDBusObject * adapterObject;
    GAutoPtr<BluezGattManager1> gattMgr;
    GVariantBuilder optionsBuilder;
    GVariant * options;

    VerifyOrExit(mAdapter.get() != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL mAdapter in %s", __func__));

    adapterObject = g_dbus_interface_get_object(G_DBUS_INTERFACE(mAdapter.get()));
    VerifyOrExit(adapterObject != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adapterObject in %s", __func__));

    gattMgr.reset(bluez_object_get_gatt_manager1(reinterpret_cast<BluezObject *>(adapterObject)));
    VerifyOrExit(gattMgr.get() != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL gattMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_gatt_manager1_call_register_application(
        gattMgr.get(), mpRootPath, options, nullptr,
        +[](GObject * aObj, GAsyncResult * aResult, void * self) {
            reinterpret_cast<BluezEndpoint *>(self)->RegisterGattApplicationDone(aObj, aResult);
        },
        this);

exit:
    return CHIP_NO_ERROR;
}

/// Update the table of open BLE connections whenever a new device is spotted or its attributes have changed.
void BluezEndpoint::UpdateConnectionTable(BluezDevice1 * apDevice)
{
    const char * objectPath      = g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(apDevice));
    BluezConnection * connection = GetBluezConnection(objectPath);

    if (connection != nullptr && !bluez_device1_get_connected(apDevice))
    {
        ChipLogDetail(DeviceLayer, "Bluez disconnected");
        BLEManagerImpl::CHIPoBluez_ConnectionClosed(connection);
        mConnMap.erase(objectPath);
        // TODO: the connection object should be released after BLEManagerImpl finishes cleaning up its resources
        // after the disconnection. Releasing it here doesn't cause any issues, but it's error-prone.
        chip::Platform::Delete(connection);
        return;
    }

    if (connection == nullptr)
    {
        HandleNewDevice(apDevice);
    }
}

void BluezEndpoint::BluezSignalInterfacePropertiesChanged(GDBusObjectManagerClient * aManager, GDBusObjectProxy * aObject,
                                                          GDBusProxy * aInterface, GVariant * aChangedProperties,
                                                          const char * const * aInvalidatedProps)
{
    VerifyOrReturn(mAdapter.get() != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL mAdapter in %s", __func__));
    VerifyOrReturn(strcmp(g_dbus_proxy_get_interface_name(aInterface), DEVICE_INTERFACE) == 0, );

    BluezDevice1 * device = BLUEZ_DEVICE1(aInterface);
    VerifyOrReturn(BluezIsDeviceOnAdapter(device, mAdapter.get()));

    UpdateConnectionTable(device);
}

void BluezEndpoint::HandleNewDevice(BluezDevice1 * device)
{
    VerifyOrReturn(bluez_device1_get_connected(device));
    VerifyOrReturn(!mIsCentral || bluez_device1_get_services_resolved(device));

    const char * objectPath = g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(device));
    BluezConnection * conn  = GetBluezConnection(objectPath);
    VerifyOrReturn(conn == nullptr,
                   ChipLogError(DeviceLayer, "FAIL: Connection already tracked: conn=%p device=%s path=%s", conn,
                                conn->GetPeerAddress(), objectPath));

    conn                       = chip::Platform::New<BluezConnection>(*this, device);
    mpPeerDevicePath           = g_strdup(objectPath);
    mConnMap[mpPeerDevicePath] = conn;

    ChipLogDetail(DeviceLayer, "New BLE connection: conn=%p device=%s path=%s", conn, conn->GetPeerAddress(), objectPath);

    BLEManagerImpl::HandleNewConnection(conn);
}

void BluezEndpoint::BluezSignalOnObjectAdded(GDBusObjectManager * aManager, GDBusObject * aObject)
{
    // TODO: right now we do not handle addition/removal of adapters
    // Primary focus here is to handle addition of a device
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(reinterpret_cast<BluezObject *>(aObject)));
    VerifyOrReturn(device.get() != nullptr);

    if (BluezIsDeviceOnAdapter(device.get(), mAdapter.get()) == TRUE)
    {
        HandleNewDevice(device.get());
    }
}

void BluezEndpoint::BluezSignalOnObjectRemoved(GDBusObjectManager * aManager, GDBusObject * aObject)
{
    // TODO: for Device1, lookup connection, and call otPlatTobleHandleDisconnected
    // for Adapter1: unclear, crash if this pertains to our adapter? at least null out the self->mAdapter.
    // for Characteristic1, or GattService -- handle here via calling otPlatTobleHandleDisconnected, or ignore.
}

BluezGattService1 * BluezEndpoint::CreateGattService(const char * aUUID)
{
    BluezObjectSkeleton * object;
    BluezGattService1 * service;

    mpServicePath = g_strdup_printf("%s/service", mpRootPath);
    ChipLogDetail(DeviceLayer, "CREATE service object at %s", mpServicePath);
    object = bluez_object_skeleton_new(mpServicePath);

    service = bluez_gatt_service1_skeleton_new();
    bluez_gatt_service1_set_uuid(service, aUUID);
    // device is only valid for remote services
    bluez_gatt_service1_set_primary(service, TRUE);

    // includes -- unclear whether required.  Might be filled in later
    bluez_object_skeleton_set_gatt_service1(object, service);
    g_dbus_object_manager_server_export(mpRoot, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    return service;
}

CHIP_ERROR BluezEndpoint::SetupAdapter()
{
    char expectedPath[32];
    snprintf(expectedPath, sizeof(expectedPath), BLUEZ_PATH "/hci%u", mAdapterId);

    for (BluezObject & object : BluezObjectList(mpObjMgr))
    {
        GAutoPtr<BluezAdapter1> adapter(bluez_object_get_adapter1(&object));
        if (adapter.get() != nullptr)
        {
            if (mpAdapterAddr == nullptr) // no adapter address provided, bind to the hci indicated by nodeid
            {
                if (strcmp(g_dbus_proxy_get_object_path(G_DBUS_PROXY(adapter.get())), expectedPath) == 0)
                {
                    mAdapter.reset(static_cast<BluezAdapter1 *>(g_object_ref(adapter.get())));
                    break;
                }
            }
            else
            {
                if (strcmp(bluez_adapter1_get_address(adapter.get()), mpAdapterAddr) == 0)
                {
                    mAdapter.reset(static_cast<BluezAdapter1 *>(g_object_ref(adapter.get())));
                    break;
                }
            }
        }
    }

    VerifyOrReturnError(mAdapter, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "FAIL: NULL mAdapter in %s", __func__));

    bluez_adapter1_set_powered(mAdapter.get(), TRUE);

    // Setting "Discoverable" to False on the adapter and to True on the advertisement convinces
    // Bluez to set "BR/EDR Not Supported" flag. Bluez doesn't provide API to do that explicitly
    // and the flag is necessary to force using LE transport.
    bluez_adapter1_set_discoverable(mAdapter.get(), FALSE);

    return CHIP_NO_ERROR;
}

BluezConnection * BluezEndpoint::GetBluezConnection(const char * aPath)
{
    auto it = mConnMap.find(aPath);
    return (it != mConnMap.end()) ? it->second : nullptr;
}

BluezConnection * BluezEndpoint::GetBluezConnectionViaDevice()
{
    return GetBluezConnection(mpPeerDevicePath);
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
static void UpdateAdditionalDataCharacteristic(BluezGattCharacteristic1 * characteristic)
{
    VerifyOrReturn(characteristic != nullptr);

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

void BluezEndpoint::SetupGattService()
{

    static const char * const c1_flags[] = { "write", nullptr };
    static const char * const c2_flags[] = { "read", "indicate", nullptr };
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    static const char * const c3_flags[] = { "read", nullptr };
#endif

    mpService = CreateGattService(CHIP_BLE_UUID_SERVICE_SHORT_STRING);

    // C1 characteristic
    mpC1 = CreateGattCharacteristic(mpService, "c1", CHIP_PLAT_BLE_UUID_C1_STRING, c1_flags);
    g_signal_connect(mpC1, "handle-read-value",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOpt,
                                    BluezEndpoint * self) { return self->BluezCharacteristicReadValue(aChar, aInv, aOpt); }),
                     this);
    g_signal_connect(mpC1, "handle-acquire-write",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOpt,
                                    BluezEndpoint * self) { return self->BluezCharacteristicAcquireWrite(aChar, aInv, aOpt); }),
                     this);
    g_signal_connect(mpC1, "handle-acquire-notify", G_CALLBACK(BluezCharacteristicAcquireNotifyError), nullptr);
    g_signal_connect(mpC1, "handle-confirm", G_CALLBACK(BluezCharacteristicConfirmError), nullptr);

    // C2 characteristic
    mpC2 = CreateGattCharacteristic(mpService, "c2", CHIP_PLAT_BLE_UUID_C2_STRING, c2_flags);
    g_signal_connect(mpC2, "handle-read-value",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOpt,
                                    BluezEndpoint * self) { return self->BluezCharacteristicReadValue(aChar, aInv, aOpt); }),
                     this);
    g_signal_connect(mpC2, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWriteError), nullptr);
    g_signal_connect(mpC2, "handle-acquire-notify",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOpt,
                                    BluezEndpoint * self) { return self->BluezCharacteristicAcquireNotify(aChar, aInv, aOpt); }),
                     this);
    g_signal_connect(mpC2, "handle-confirm",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, BluezEndpoint * self) {
                         return self->BluezCharacteristicConfirm(aChar, aInv);
                     }),
                     this);

    ChipLogDetail(DeviceLayer, "CHIP BTP C1 %s", bluez_gatt_characteristic1_get_service(mpC1));
    ChipLogDetail(DeviceLayer, "CHIP BTP C2 %s", bluez_gatt_characteristic1_get_service(mpC2));

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ChipLogDetail(DeviceLayer, "CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING is TRUE");
    // Additional data characteristics
    mpC3 = CreateGattCharacteristic(mpService, "c3", CHIP_PLAT_BLE_UUID_C3_STRING, c3_flags);
    g_signal_connect(mpC3, "handle-read-value",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOpt,
                                    BluezEndpoint * self) { return self->BluezCharacteristicReadValue(aChar, aInv, aOpt); }),
                     this);
    g_signal_connect(mpC3, "handle-acquire-write", G_CALLBACK(BluezCharacteristicAcquireWriteError), nullptr);
    g_signal_connect(mpC3, "handle-acquire-notify",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOpt,
                                    BluezEndpoint * self) { return self->BluezCharacteristicAcquireNotify(aChar, aInv, aOpt); }),
                     this);
    g_signal_connect(mpC3, "handle-confirm",
                     G_CALLBACK(+[](BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, BluezEndpoint * self) {
                         return self->BluezCharacteristicConfirm(aChar, aInv);
                     }),
                     this);

    // update the characteristic value
    UpdateAdditionalDataCharacteristic(mpC3);
    ChipLogDetail(DeviceLayer, "CHIP BTP C3 %s", bluez_gatt_characteristic1_get_service(mpC3));
#else
    ChipLogDetail(DeviceLayer, "CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING is FALSE");
#endif
}

void BluezEndpoint::SetupGattServer(GDBusConnection * aConn)
{
    VerifyOrReturn(!mIsCentral);

    mpRootPath = g_strdup_printf("/chipoble/%04x", getpid() & 0xffff);
    mpRoot     = g_dbus_object_manager_server_new(mpRootPath);

    SetupGattService();

    // Set connection after the service is set up in order to reduce the number
    // of signals sent on the bus.
    g_dbus_object_manager_server_set_connection(mpRoot, aConn);
}

CHIP_ERROR BluezEndpoint::StartupEndpointBindings()
{
    GAutoPtr<GError> err;
    GAutoPtr<GDBusConnection> conn(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &err.GetReceiver()));
    VerifyOrReturnError(conn != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: get bus sync in %s, error: %s", __func__, err->message));

    SetupGattServer(conn.get());

    mpObjMgr = g_dbus_object_manager_client_new_sync(conn.get(), G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/",
                                                     bluez_object_manager_client_get_proxy_type,
                                                     nullptr /* unused user data in the Proxy Type Func */,
                                                     nullptr /*destroy notify */, nullptr /* cancellable */, &err.GetReceiver());
    VerifyOrReturnError(mpObjMgr != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Error getting object manager client: %s", err->message));

    g_signal_connect(mpObjMgr, "object-added", G_CALLBACK(+[](GDBusObjectManager * aMgr, GDBusObject * aObj, BluezEndpoint * self) {
                         return self->BluezSignalOnObjectAdded(aMgr, aObj);
                     }),
                     this);
    g_signal_connect(mpObjMgr, "object-removed",
                     G_CALLBACK(+[](GDBusObjectManager * aMgr, GDBusObject * aObj, BluezEndpoint * self) {
                         return self->BluezSignalOnObjectRemoved(aMgr, aObj);
                     }),
                     this);
    g_signal_connect(mpObjMgr, "interface-proxy-properties-changed",
                     G_CALLBACK(+[](GDBusObjectManagerClient * aMgr, GDBusObjectProxy * aObj, GDBusProxy * aIface,
                                    GVariant * aChangedProps, const char * const * aInvalidatedProps, BluezEndpoint * self) {
                         return self->BluezSignalInterfacePropertiesChanged(aMgr, aObj, aIface, aChangedProps, aInvalidatedProps);
                     }),
                     this);

    SetupAdapter();

    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezEndpoint::RegisterGattApplication()
{
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezEndpoint * self) { return self->RegisterGattApplicationImpl(); }, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Ble, "Failed to schedule RegisterGattApplication() on CHIPoBluez thread"));
    return err;
}

CHIP_ERROR BluezEndpoint::Init(bool aIsCentral, uint32_t aAdapterId)
{
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    mAdapterId = aAdapterId;
    mIsCentral = aIsCentral;

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezEndpoint * self) { return self->StartupEndpointBindings(); }, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to schedule endpoint initialization"));

    ChipLogDetail(DeviceLayer, "BlueZ integration init success");
    mIsInitialized = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezEndpoint::Init(bool aIsCentral, const char * apBleAddr)
{
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    mpAdapterAddr = g_strdup(apBleAddr);
    return Init(aIsCentral, mAdapterId);
}

void BluezEndpoint::Shutdown()
{
    VerifyOrReturn(mIsInitialized);

    // Run endpoint cleanup on the CHIPoBluez thread. This is necessary because the
    // cleanup function releases the D-Bus manager client object, which handles D-Bus
    // signals. Otherwise, we will face race condition when the D-Bus signal is in
    // the middle of being processed when the cleanup function is called.
    PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezEndpoint * self) {
            if (self->mpObjMgr != nullptr)
                g_object_unref(self->mpObjMgr);
            self->mAdapter.reset();
            if (self->mpRoot != nullptr)
                g_object_unref(self->mpRoot);
            if (self->mpService != nullptr)
                g_object_unref(self->mpService);
            if (self->mpC1 != nullptr)
                g_object_unref(self->mpC1);
            if (self->mpC2 != nullptr)
                g_object_unref(self->mpC2);
            if (self->mpC3 != nullptr)
                g_object_unref(self->mpC3);
            return CHIP_NO_ERROR;
        },
        this);

    g_free(mpAdapterAddr);
    g_free(mpRootPath);
    g_free(mpServicePath);
    g_free(mpPeerDevicePath);

    mIsInitialized = false;
}

// ConnectDevice callbacks

CHIP_ERROR BluezEndpoint::ConnectDeviceImpl(BluezDevice1 & aDevice)
{
    // Due to radio interferences or Wi-Fi coexistence, sometimes the BLE connection may not be
    // established (e.g. Connection Indication Packet is missed by BLE peripheral). In such case,
    // BlueZ returns "Software caused connection abort error", and we should make a connection retry.
    // It's important to make sure that the connection is correctly ceased, by calling `Disconnect()`
    // D-Bus method, or else `Connect()` returns immediately without any effect.
    for (uint16_t i = 0; i < kMaxConnectRetries; i++)
    {
        GAutoPtr<GError> error;
        if (bluez_device1_call_connect_sync(&aDevice, mConnectCancellable.get(), &error.GetReceiver()))
        {
            ChipLogDetail(DeviceLayer, "ConnectDevice complete");
            return CHIP_NO_ERROR;
        }

        ChipLogError(DeviceLayer, "FAIL: ConnectDevice: %s (%d)", error->message, error->code);
        if (!g_error_matches(error.get(), G_IO_ERROR, G_IO_ERROR_DBUS_ERROR))
        {
            break;
        }

        ChipLogProgress(DeviceLayer, "ConnectDevice retry: %u out of %u", i + 1, kMaxConnectRetries);
        bluez_device1_call_disconnect_sync(&aDevice, nullptr, nullptr);
    }

    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL);
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR BluezEndpoint::ConnectDevice(BluezDevice1 & aDevice)
{
    auto params = std::make_pair(this, &aDevice);
    mConnectCancellable.reset(g_cancellable_new());
    return PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](typeof(params) * aParams) { return aParams->first->ConnectDeviceImpl(*aParams->second); }, &params);
}

void BluezEndpoint::CancelConnect()
{
    g_cancellable_cancel(mConnectCancellable.get());
    mConnectCancellable.reset();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
