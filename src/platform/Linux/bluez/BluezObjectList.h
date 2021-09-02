/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <glib.h>

#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>

#include "BluezObjectIterator.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  C++ wrapper for a Bluez object list based on a object manager
 */
class BluezObjectList
{
public:
    explicit BluezObjectList(GDBusObjectManager * manager) { Initialize(manager); }

    ~BluezObjectList() { g_list_free_full(mObjectList, g_object_unref); }

    BluezObjectIterator begin() const { return BluezObjectIterator(mObjectList); }
    BluezObjectIterator end() const { return BluezObjectIterator(); }

protected:
    BluezObjectList() {}

    void Initialize(GDBusObjectManager * manager)
    {
        if (manager == nullptr)
        {
            ChipLogError(DeviceLayer, "Manager is NULL in %s", __func__);
            return;
        }

        mObjectList = g_dbus_object_manager_get_objects(manager);
    }

private:
    GList * mObjectList = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
