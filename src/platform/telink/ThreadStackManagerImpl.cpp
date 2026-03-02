/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
 *          Provides an implementation of the ThreadStackManager object
 *          for Telink platform.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/telink/ThreadStackManagerImpl.h>

#include <lib/support/CodeUtils.h>
#include <platform/ThreadStackManager.h>

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

CHIP_ERROR ThreadStackManagerImpl::StartNonConcurrentThreadManagement()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "Switch to Thread");
    k_sleep(K_MSEC(50)); // Small delay to ensure BLE stack is fully disabled before Thread attach
    TEMPORARY_RETURN_IGNORED ThreadStackMgrImpl().SetThreadEnabled(true);

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    ChipDeviceEvent opEvent;
    opEvent.Type = DeviceEventType::kOperationalNetworkStarted;

    error = PlatformMgr().PostEvent(&opEvent);
    VerifyOrExit(error == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "PostEvent err: %" CHIP_ERROR_FORMAT, error.Format()));
#endif

exit:
    return error;
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

CHIP_ERROR ThreadStackManagerImpl::_StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback)
{
    /* In non-concurrent mode, BLE and Thread cannot run simultaneously.
     *
     * This request corresponds to a Thread prescan. If BLE is currently active,
     * a new scan cannot be started, so the cached prescan results are returned
     * instead. Once the device switches to Thread mode, scanning is available again.
     */
    if (bt_is_ready())
    {
        ChipLogProgress(DeviceLayer, "Thread prescan: BLE active, using cached results");

        if (callback != nullptr)
        {
            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, callback]() {
                callback->OnFinished(NetworkCommissioning::Status::kSuccess, CharSpan(), &mScanResponseIter);
            });
        }
        return CHIP_NO_ERROR;
    }

    return Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::_StartThreadScan(callback);
}

} // namespace DeviceLayer
} // namespace chip
