/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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
/* this file behaves like a config.h, comes first */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

// ==================== ConnectivityManager Platform Internal Methods ====================
CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    err = InitWiFi();
#endif
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    OnWiFiPlatformEvent(event);
#endif
}

CHIP_ERROR ConnectivityManagerImpl::PostEvent(const ChipDeviceEvent * event, bool die)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (die)
    {
        PlatformMgr().PostEventOrDie(event);
    }
    else
    {
        err = PlatformMgr().PostEvent(event);
    }

    return err;
}
void ConnectivityManagerImpl::AddTask(AsyncWorkFunct workFunct, intptr_t arg)
{
    PlatformMgr().ScheduleWork(workFunct, arg);
}

void ConnectivityManagerImpl::AddQueueEvent(void (*func)(nsapi_event_t, intptr_t), nsapi_event_t event, intptr_t data)
{
    PlatformMgrImpl().mQueue.call([func, event, data] {
        PlatformMgr().LockChipStack();
        (*func)(event, data);
        PlatformMgr().UnlockChipStack();
    });
}

} // namespace DeviceLayer
} // namespace chip
