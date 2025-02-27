/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BluezObjectManager.h"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

#include <ble/Ble.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConfigurationManager.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>
#include <platform/PlatformManager.h>

#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

const char * GetAdapterObjectPath(BluezAdapter1 * aAdapter)
{
    return g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(aAdapter));
}

} // namespace

CHIP_ERROR BluezObjectManager::Init()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezObjectManager * self) {
            ReturnErrorOnFailure(self->SetupDBusConnection());
            ReturnErrorOnFailure(self->SetupObjectManager());
            return CHIP_NO_ERROR;
        },
        this);
}

void BluezObjectManager::Shutdown()
{
    // If the D-Bus connection or the object manager are not initialized,
    // there is nothing to shutdown. This check prevents unnecessary call
    // to the GLibMatterContextInvokeSync function.
    VerifyOrReturn(mConnection || mObjectManager);

    // Run endpoint cleanup on the CHIPoBluez thread. This is necessary because the
    // cleanup function releases the D-Bus manager client object, which handles D-Bus
    // signals. Otherwise, we will face race condition when the D-Bus signal is in
    // the middle of being processed when the cleanup function is called.
    PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezObjectManager * self) {
            self->mConnection.reset();
            self->mObjectManager.reset();
            return CHIP_NO_ERROR;
        },
        this);
}

BluezAdapter1 * BluezObjectManager::GetAdapter(unsigned int aAdapterId)
{
    char expectedPath[32];
    snprintf(expectedPath, sizeof(expectedPath), BLUEZ_PATH "/hci%u", aAdapterId);

    for (BluezObject & object : GetObjects())
    {
        GAutoPtr<BluezAdapter1> adapter(bluez_object_get_adapter1(&object));
        if (adapter && strcmp(g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(adapter.get())), expectedPath) == 0)
        {
            SetupAdapter(adapter.get());
            return adapter.release();
        }
    }

    return nullptr;
}

BluezAdapter1 * BluezObjectManager::GetAdapter(const char * aAdapterAddress)
{
    for (BluezObject & object : GetObjects())
    {
        GAutoPtr<BluezAdapter1> adapter(bluez_object_get_adapter1(&object));
        if (adapter && strcmp(bluez_adapter1_get_address(adapter.get()), aAdapterAddress) == 0)
        {
            SetupAdapter(adapter.get());
            return adapter.release();
        }
    }

    return nullptr;
}

CHIP_ERROR BluezObjectManager::SubscribeDeviceNotifications(BluezAdapter1 * aAdapter,
                                                            BluezObjectManagerAdapterNotificationsDelegate * aDelegate)
{
    std::lock_guard<std::mutex> lock(mSubscriptionsMutex);
    mSubscriptions.emplace_back(GetAdapterObjectPath(aAdapter), aDelegate);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezObjectManager::UnsubscribeDeviceNotifications(BluezAdapter1 * aAdapter,
                                                              BluezObjectManagerAdapterNotificationsDelegate * aDelegate)
{
    std::lock_guard<std::mutex> lock(mSubscriptionsMutex);
    const auto item = std::make_pair(std::string(GetAdapterObjectPath(aAdapter)), aDelegate);
    mSubscriptions.erase(std::remove(mSubscriptions.begin(), mSubscriptions.end(), item), mSubscriptions.end());
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezObjectManager::SetupAdapter(BluezAdapter1 * aAdapter)
{
    // Make sure the adapter is powered on.
    bluez_adapter1_set_powered(aAdapter, TRUE);
    // Setting "Discoverable" to False on the adapter and to True on the advertisement convinces
    // BlueZ to set "BR/EDR Not Supported" flag. BlueZ doesn't provide API to do that explicitly
    // and the flag is necessary to force using LE transport.
    bluez_adapter1_set_discoverable(aAdapter, FALSE);
    return CHIP_NO_ERROR;
}

void BluezObjectManager::NotifyAdapterAdded(BluezAdapter1 * aAdapter)
{
    unsigned int adapterId = 0;
    sscanf(GetAdapterObjectPath(aAdapter), BLUEZ_PATH "/hci%u", &adapterId);
    // Notify the application that new adapter has been just added
    BLEManagerImpl::NotifyBLEAdapterAdded(adapterId, bluez_adapter1_get_address(aAdapter));
}

void BluezObjectManager::NotifyAdapterRemoved(BluezAdapter1 * aAdapter)
{
    unsigned int adapterId = 0;
    sscanf(GetAdapterObjectPath(aAdapter), BLUEZ_PATH "/hci%u", &adapterId);
    // Notify the application that the adapter is no longer available
    BLEManagerImpl::NotifyBLEAdapterRemoved(adapterId, bluez_adapter1_get_address(aAdapter));
}

void BluezObjectManager::RemoveAdapterSubscriptions(BluezAdapter1 * aAdapter)
{
    std::lock_guard<std::mutex> lock(mSubscriptionsMutex);
    const auto adapterPath = GetAdapterObjectPath(aAdapter);
    // Remove all device notification subscriptions for the given adapter
    mSubscriptions.erase(std::remove_if(mSubscriptions.begin(), mSubscriptions.end(),
                                        [adapterPath](const auto & subscription) { return subscription.first == adapterPath; }),
                         mSubscriptions.end());
}

CHIP_ERROR BluezObjectManager::SetupDBusConnection()
{
    GAutoPtr<GError> err;
    mConnection.reset(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &err.GetReceiver()));
    VerifyOrReturnError(mConnection != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Get D-Bus system bus: %s", err->message));
    return CHIP_NO_ERROR;
}

BluezObjectManager::NotificationsDelegates BluezObjectManager::GetDeviceNotificationsDelegates(BluezDevice1 * device)
{
    const char * deviceAdapterPath = bluez_device1_get_adapter(device);
    NotificationsDelegates delegates;

    std::lock_guard<std::mutex> lock(mSubscriptionsMutex);
    for (auto & [adapterPath, delegate] : mSubscriptions)
    {
        if (adapterPath == deviceAdapterPath)
        {
            delegates.push_back(delegate);
        }
    }

    return delegates;
}

void BluezObjectManager::OnObjectAdded(GDBusObjectManager * aMgr, BluezObject * aObj)
{
    GAutoPtr<BluezAdapter1> adapter(bluez_object_get_adapter1(aObj));
    // Verify that the adapter is properly initialized - the class property must be set.
    // BlueZ can export adapter objects on the bus before it is fully initialized. Such
    // adapter objects are not usable and must be ignored.
    //
    // TODO: Find a better way to determine whether the adapter interface exposed by
    //       BlueZ D-Bus service is fully functional. The current approach is based on
    //       the assumption that the class property is non-zero, which is true only
    //       for BR/EDR + LE adapters. LE-only adapters do not have HCI command to read
    //       the class property and BlueZ sets it to 0 as a default value.
    if (adapter && bluez_adapter1_get_class(adapter.get()) != 0)
    {
        NotifyAdapterAdded(adapter.get());
        return;
    }

    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(aObj));
    if (device)
    {
        for (auto delegate : GetDeviceNotificationsDelegates(device.get()))
        {
            delegate->OnDeviceAdded(*device.get());
        }
    }
}

void BluezObjectManager::OnObjectRemoved(GDBusObjectManager * aMgr, BluezObject * aObj)
{
    GAutoPtr<BluezAdapter1> adapter(bluez_object_get_adapter1(aObj));
    if (adapter)
    {
        RemoveAdapterSubscriptions(adapter.get());
        NotifyAdapterRemoved(adapter.get());
        return;
    }

    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(aObj));
    if (device)
    {
        for (auto delegate : GetDeviceNotificationsDelegates(device.get()))
        {
            delegate->OnDeviceRemoved(*device.get());
        }
    }
}

void BluezObjectManager::OnInterfacePropertiesChanged(GDBusObjectManagerClient * aMgr, BluezObject * aObj, GDBusProxy * aIface,
                                                      GVariant * aChangedProps, const char * const * aInvalidatedProps)
{
    uint32_t classValue = 0;
    GAutoPtr<BluezAdapter1> adapter(bluez_object_get_adapter1(aObj));
    // When the adapter's readonly class property is set, it means that the adapter has been
    // fully initialized and is ready to be used. It's most likely that the adapter has been
    // previously ignored in the OnObjectAdded callback, so now we can notify the application
    // about the new adapter.
    if (adapter && g_variant_lookup(aChangedProps, "Class", "u", &classValue) && classValue != 0)
    {
        NotifyAdapterAdded(adapter.get());
        return;
    }

    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(aObj));
    if (device)
    {
        for (auto delegate : GetDeviceNotificationsDelegates(device.get()))
        {
            delegate->OnDevicePropertyChanged(*device.get(), aChangedProps, aInvalidatedProps);
        }
    }
}

CHIP_ERROR BluezObjectManager::SetupObjectManager()
{
    // When connecting to signals, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    GAutoPtr<GError> err;
    mObjectManager.reset(g_dbus_object_manager_client_new_sync(
        mConnection.get(), G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/",
        bluez_object_manager_client_get_proxy_type, nullptr /* unused user data in the proxy type func */,
        nullptr /* destroy notify */, nullptr /* cancellable */, &err.GetReceiver()));
    VerifyOrReturnError(mObjectManager, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Get D-Bus object manager client: %s", err->message));

    g_signal_connect(mObjectManager.get(), "object-added",
                     G_CALLBACK(+[](GDBusObjectManager * mgr, GDBusObject * obj, BluezObjectManager * self) {
                         return self->OnObjectAdded(mgr, reinterpret_cast<BluezObject *>(obj));
                     }),
                     this);
    g_signal_connect(mObjectManager.get(), "object-removed",
                     G_CALLBACK(+[](GDBusObjectManager * mgr, GDBusObject * obj, BluezObjectManager * self) {
                         return self->OnObjectRemoved(mgr, reinterpret_cast<BluezObject *>(obj));
                     }),
                     this);
    g_signal_connect(mObjectManager.get(), "interface-proxy-properties-changed",
                     G_CALLBACK(+[](GDBusObjectManagerClient * mgr, GDBusObjectProxy * obj, GDBusProxy * iface, GVariant * changed,
                                    const char * const * invalidated, BluezObjectManager * self) {
                         return self->OnInterfacePropertiesChanged(mgr, reinterpret_cast<BluezObject *>(obj), iface, changed,
                                                                   invalidated);
                     }),
                     this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezCallToChipError(const GError * aError)
{
    switch (aError->code)
    {
    // BlueZ crashed or the D-Bus connection is broken in both cases adapter is not available.
    case G_DBUS_ERROR_NO_REPLY:
    // BlueZ service is not available on the bus, hence the adapter is not available too.
    case G_DBUS_ERROR_SERVICE_UNKNOWN:
    // Requested D-Bus object is not available on the given path. This happens when the adapter
    // was unplugged and unregistered from the BlueZ object manager.
    case G_DBUS_ERROR_UNKNOWN_OBJECT:
        return BLE_ERROR_ADAPTER_UNAVAILABLE;
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
