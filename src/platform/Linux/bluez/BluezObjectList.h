/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
