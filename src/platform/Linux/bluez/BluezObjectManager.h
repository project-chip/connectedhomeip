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

#include <mutex>
#include <string>
#include <vector>

#include <gio/gio.h>
#include <glib.h>

#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>
#include <system/SystemMutex.h>

#include "BluezObjectList.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Delegate for receiving notifications about various events on the Adapter1
/// interface managed by the BlueZ object manager.
class BluezObjectManagerAdapterNotificationsDelegate
{
public:
    virtual ~BluezObjectManagerAdapterNotificationsDelegate() {}
    virtual void OnDeviceAdded(BluezDevice1 & device)                                                                           = 0;
    virtual void OnDevicePropertyChanged(BluezDevice1 & device, GVariant * changedProps, const char * const * invalidatedProps) = 0;
    virtual void OnDeviceRemoved(BluezDevice1 & device)                                                                         = 0;
};

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

    // Subscribe to notifications associated with the given adapter.
    // In case when the adapter is removed, the subscription will be automatically canceled.
    CHIP_ERROR SubscribeDeviceNotifications(BluezAdapter1 * adapter, BluezObjectManagerAdapterNotificationsDelegate * delegate);

    // Unsubscribe from notifications associated with the given adapter.
    CHIP_ERROR UnsubscribeDeviceNotifications(BluezAdapter1 * adapter, BluezObjectManagerAdapterNotificationsDelegate * delegate);

private:
    CHIP_ERROR SetupDBusConnection();
    CHIP_ERROR SetupObjectManager();
    CHIP_ERROR SetupAdapter(BluezAdapter1 * aAdapter);

    void NotifyAdapterAdded(BluezAdapter1 * aAdapter);
    void NotifyAdapterRemoved(BluezAdapter1 * aAdapter);
    void RemoveAdapterSubscriptions(BluezAdapter1 * aAdapter);

    using NotificationsDelegates = std::vector<BluezObjectManagerAdapterNotificationsDelegate *>;
    NotificationsDelegates GetDeviceNotificationsDelegates(BluezDevice1 * device);

    void OnObjectAdded(GDBusObjectManager * aMgr, BluezObject * aObj);
    void OnObjectRemoved(GDBusObjectManager * aMgr, BluezObject * aObj);
    void OnInterfacePropertiesChanged(GDBusObjectManagerClient * aMgr, BluezObject * aObj, GDBusProxy * aIface,
                                      GVariant * aChangedProps, const char * const * aInvalidatedProps);

    GAutoPtr<GDBusConnection> mConnection;
    GAutoPtr<GDBusObjectManager> mObjectManager;

    std::mutex mSubscriptionsMutex;
    std::vector<std::pair<std::string, BluezObjectManagerAdapterNotificationsDelegate *>>
        mSubscriptions CHIP_GUARDED_BY(mSubscriptionsMutex);
};

// Helper function to convert glib error returned by bluez_*_call_*() functions to CHIP_ERROR.
CHIP_ERROR BluezCallToChipError(const GError * aError);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
