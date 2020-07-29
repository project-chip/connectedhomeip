/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "platform/internal/CHIPDeviceLayerInternal.h"
#include <assert.h>
#include <dbus/dbus.h>
#include <memory>
#include <support/CodeUtils.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include "platform/PlatformManager.h"
#include "platform/internal/BLEManager.h"

#if CHIP_DEVICE_LAYER_TARGET == LINUX
#include <thread>

struct DBusConnectionDeleter
{
    void operator()(DBusConnection * aConnection) { dbus_connection_unref(aConnection); }
};

using UniqueDBusConnection = std::unique_ptr<DBusConnection, DBusConnectionDeleter>;
#endif

void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
        // exit(0);
    }
}

int TestCHIPoBLEStackManager(void)
{
    chip::DeviceLayer::PlatformMgrImpl().InitChipStack();

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("CHIP0001");

    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(1, false);

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(
        chip::DeviceLayer::ConnectivityManager::kCHIPoBLEServiceMode_Enabled);

    ChipLogProgress(DeviceLayer, "Start Chip Over Ble task done");

    chip::DeviceLayer::PlatformMgrImpl().RunEventLoop();

    return -1;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
