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

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          Zephyr platforms.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/Zephyr/ThreadStackManagerImpl.h>

#include <inet/UDPEndPointImpl.h>
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::Inet;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    otInstance * const instance = openthread_get_default_instance();

    ReturnErrorOnFailure(GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(instance));

    UDPEndPointImplSockets::SetJoinMulticastGroupHandler([](InterfaceId, const IPAddress & address) {
        const otIp6Address otAddress = ToOpenThreadIP6Address(address);

        ThreadStackMgr().LockThreadStack();
        const auto otError = otIp6SubscribeMulticastAddress(openthread_get_default_instance(), &otAddress);
        ThreadStackMgr().UnlockThreadStack();

        return MapOpenThreadError(otError);
    });

    UDPEndPointImplSockets::SetLeaveMulticastGroupHandler([](InterfaceId, const IPAddress & address) {
        const otIp6Address otAddress = ToOpenThreadIP6Address(address);

        ThreadStackMgr().LockThreadStack();
        const auto otError = otIp6UnsubscribeMulticastAddress(openthread_get_default_instance(), &otAddress);
        ThreadStackMgr().UnlockThreadStack();

        return MapOpenThreadError(otError);
    });

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    k_sem_init(&mSrpClearAllSemaphore, 0, 1);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_LockThreadStack()
{
    openthread_api_mutex_lock(openthread_get_default_context());
}

bool ThreadStackManagerImpl::_TryLockThreadStack()
{
    // There's no openthread_api_mutex_try_lock() in Zephyr, so until it's contributed we must use the low-level API
    return k_mutex_lock(&openthread_get_default_context()->api_lock, K_NO_WAIT) == 0;
}

void ThreadStackManagerImpl::_UnlockThreadStack()
{
    openthread_api_mutex_unlock(openthread_get_default_context());
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
void ThreadStackManagerImpl::_WaitOnSrpClearAllComplete()
{
    k_sem_take(&mSrpClearAllSemaphore, K_SECONDS(2));
}

void ThreadStackManagerImpl::_NotifySrpClearAllComplete()
{
    k_sem_give(&mSrpClearAllSemaphore);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

} // namespace DeviceLayer
} // namespace chip
