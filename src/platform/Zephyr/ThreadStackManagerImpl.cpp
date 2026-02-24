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

#include <lib/support/CodeUtils.h>
#include <platform/ThreadStackManager.h>

#include <zephyr/version.h>

#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#include <ncs_version.h>
#endif

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    otInstance * const instance = openthread_get_default_instance();

    ReturnErrorOnFailure(GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(instance));

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    k_sem_init(&mSrpClearAllSemaphore, 0, 1);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_LockThreadStack()
{
#if KERNEL_VERSION_MAJOR >= 4 && KERNEL_VERSION_MINOR >= 2
    openthread_mutex_lock();
// nRF Connect SDK 3.1.0 supports Zephyr 4.1.99 version, so unfortunately it needs a separate check
#elif CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#if NCS_VERSION_MAJOR >= 3 && NCS_VERSION_MINOR >= 1
    openthread_mutex_lock();
#endif
#else
    openthread_api_mutex_lock(openthread_get_default_context());
#endif
}

bool ThreadStackManagerImpl::_TryLockThreadStack()
{
#if KERNEL_VERSION_MAJOR >= 4 && KERNEL_VERSION_MINOR >= 2
    return openthread_mutex_try_lock() == 0;
// nRF Connect SDK 3.1.0 supports Zephyr 4.1.99 version, so unfortunately it needs a separate check
#elif CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#if NCS_VERSION_MAJOR >= 3 && NCS_VERSION_MINOR >= 1
    return openthread_mutex_try_lock() == 0;
#endif
#else
    // There's no openthread_api_mutex_try_lock() in Zephyr, so until it's contributed we must use the low-level API
    return k_mutex_lock(&openthread_get_default_context()->api_lock, K_NO_WAIT) == 0;
#endif
}

void ThreadStackManagerImpl::_UnlockThreadStack()
{
#if KERNEL_VERSION_MAJOR >= 4 && KERNEL_VERSION_MINOR >= 2
    openthread_mutex_unlock();
// nRF Connect SDK 3.1.0 supports Zephyr 4.1.99 version, so unfortunately it needs a separate check
#elif CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#if NCS_VERSION_MAJOR >= 3 && NCS_VERSION_MINOR >= 1
    openthread_mutex_unlock();
#endif
#else
    openthread_api_mutex_unlock(openthread_get_default_context());
#endif
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
