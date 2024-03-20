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

#pragma once

#include <gio/gio.h>
#include <glib.h>

#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>

#include "BluezObjectList.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BluezObjectManager
{
public:
    BluezObjectManager()                                       = default;
    BluezObjectManager(const BluezObjectManager &)             = delete;
    BluezObjectManager & operator=(const BluezObjectManager &) = delete;

    ~BluezObjectManager() = default;

    CHIP_ERROR Init();
    void Shutdown();

    // Convenience method to get the D-Bus connection established by this object manager.
    GDBusConnection * GetConnection() const { return mConnection.get(); }
    // Convenience method to get the BlueZ object manager.
    GDBusObjectManager * GetObjectManager() const { return mObjectManager.get(); }

    // Get the list of objects in the BlueZ object manager.
    BluezObjectList GetObjects() const { return BluezObjectList(mObjectManager.get()); }

    // Get the adapter with the given HCI ID.
    BluezAdapter1 * GetAdapter(unsigned int aAdapterId);
    // Get the adapter with the given Bluetooth address.
    BluezAdapter1 * GetAdapter(const char * aAdapterAddress);

private:
    CHIP_ERROR SetupDBusConnection();
    CHIP_ERROR SetupObjectManager();
    CHIP_ERROR SetupAdapter(BluezAdapter1 * aAdapter);

    GAutoPtr<GDBusConnection> mConnection;
    GAutoPtr<GDBusObjectManager> mObjectManager;
};

// Helper function to convert glib error returned by bluez_*_call_*() functions to CHIP_ERROR.
CHIP_ERROR BluezCallToChipError(const GError * aError);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
