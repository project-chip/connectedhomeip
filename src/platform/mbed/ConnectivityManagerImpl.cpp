/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
