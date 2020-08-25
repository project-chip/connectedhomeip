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

#include <assert.h>
#include <dbus/dbus.h>
#include <memory>

#include "platform/internal/CHIPDeviceLayerInternal.h"

#include "platform/PlatformManager.h"
#include "platform/ThreadStackManager.h"

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
    if (event->Type == chip::DeviceLayer::DeviceEventType::kThreadConnectivityChange)
    {
        if (event->ThreadConnectivityChange.Result == chip::DeviceLayer::ConnectivityChange::kConnectivity_Established)
        {
            exit(0);
        }
    }
}

int TestThreadStackManager(void)
{
    chip::DeviceLayer::ThreadStackManagerImpl impl;
    chip::DeviceLayer::Internal::DeviceNetworkInfo info;
    uint16_t masterKey[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    strncpy(info.ThreadNetworkName, "CHIP-TEST", sizeof(info.ThreadNetworkName));
    info.ThreadChannel                    = UINT8_MAX;
    info.ThreadPANId                      = 0x3455;
    info.FieldPresent.ThreadExtendedPANId = false;
    info.FieldPresent.ThreadMeshPrefix    = false;
    info.FieldPresent.ThreadPSKc          = false;
    memcpy(&info.ThreadNetworkKey, &masterKey, sizeof(masterKey));

    chip::DeviceLayer::PlatformMgrImpl().InitChipStack();
    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    impl.InitThreadStack();
    impl.StartThreadTask();
    impl._SetThreadProvision(info);
    impl._SetThreadEnabled(true);

    printf("Start Thread task done\n");

    chip::DeviceLayer::PlatformMgrImpl().RunEventLoop();

    return -1;
}
