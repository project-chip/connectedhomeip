/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
