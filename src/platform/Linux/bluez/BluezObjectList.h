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
#include <platform/Linux/dbus/bluez/DBusBluez.h>

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
    BluezObjectList() = default;
    explicit BluezObjectList(GDBusObjectManager * manager) { Init(manager); }

    // This class is not trivially copyable
    BluezObjectList(const BluezObjectList &)             = delete;
    BluezObjectList & operator=(const BluezObjectList &) = delete;

    ~BluezObjectList()
    {
        if (mObjectList != nullptr)
            g_list_free_full(mObjectList, g_object_unref);
    }

    CHIP_ERROR Init(GDBusObjectManager * manager)
    {
        VerifyOrReturnError(manager != nullptr, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(DeviceLayer, "Manager is NULL in %s", __func__));
        mObjectList = g_dbus_object_manager_get_objects(manager);
        return CHIP_NO_ERROR;
    }

    BluezObjectIterator begin() const { return BluezObjectIterator(mObjectList); }
    static BluezObjectIterator end() { return BluezObjectIterator(); }

private:
    GList * mObjectList = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
