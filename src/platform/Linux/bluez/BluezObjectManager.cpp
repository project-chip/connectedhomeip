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

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

#include <ble/BleError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConfigurationManager.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <platform/PlatformManager.h>

#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

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

CHIP_ERROR BluezObjectManager::SetupDBusConnection()
{
    GAutoPtr<GError> err;
    mConnection.reset(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &err.GetReceiver()));
    VerifyOrReturnError(mConnection != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Get D-Bus system bus: %s", err->message));
    return CHIP_NO_ERROR;
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
