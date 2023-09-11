/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides the implementation of the Device Layer Connectivity Manager class
 *          for Open IOT SDK platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

// ==================== ConnectivityManager Platform Internal Methods ====================
CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

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

} // namespace DeviceLayer
} // namespace chip
