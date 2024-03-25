/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "AdapterIterator.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR AdapterIterator::Initialize()
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    GAutoPtr<GError> error;
    mManager.reset(g_dbus_object_manager_client_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/",
        bluez_object_manager_client_get_proxy_type, nullptr /* unused user data in the Proxy Type Func */,
        nullptr /* destroy notify */, nullptr /* cancellable */, &error.GetReceiver()));

    VerifyOrReturnError(mManager, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Failed to get D-Bus object manager for listing adapters: %s", error->message));

    mObjectList.Init(mManager.get());
    mIterator = mObjectList.begin();

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdapterIterator::Shutdown()
{
    // Release resources on the glib thread to synchronize with potential signal handlers
    // attached to the manager client object that may run on the glib thread.
    return PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](AdapterIterator * self) {
            self->mManager.reset();
            return CHIP_NO_ERROR;
        },
        this);
}

bool AdapterIterator::Advance()
{
    for (; mIterator != BluezObjectList::end(); ++mIterator)
    {
        BluezAdapter1 * adapter = bluez_object_get_adapter1(&(*mIterator));
        if (adapter != nullptr)
        {
            mCurrentAdapter.reset(adapter);
            ++mIterator;
            return true;
        }
    }

    return false;
}

uint32_t AdapterIterator::GetIndex() const
{
    // PATH is of the for  BLUEZ_PATH / hci<nr>, i.e. like '/org/bluez/hci0'
    // Index represents the number after hci
    const char * path = g_dbus_proxy_get_object_path(G_DBUS_PROXY(mCurrentAdapter.get()));
    unsigned index    = 0;

    if (sscanf(path, BLUEZ_PATH "/hci%u", &index) != 1)
    {
        ChipLogError(DeviceLayer, "Failed to extract HCI index from '%s'", StringOrNullMarker(path));
        index = 0;
    }

    return index;
}

bool AdapterIterator::Next()
{
    if (!mManager)
    {
        CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
            +[](AdapterIterator * self) { return self->Initialize(); }, this);
        VerifyOrReturnError(err == CHIP_NO_ERROR, false, ChipLogError(DeviceLayer, "Failed to initialize adapter iterator"));
    }

    return Advance();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
