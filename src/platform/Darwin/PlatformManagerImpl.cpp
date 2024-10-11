/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object
 *          for Darwin platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <tracing/metric_macros.h>

#if !CHIP_DISABLE_PLATFORM_KVS
#include <platform/Darwin/DeviceInstanceInfoProviderImpl.h>
#include <platform/DeviceInstanceInfoProvider.h>
#endif

#include <platform/Darwin/DiagnosticDataProviderImpl.h>
#include <platform/Darwin/PlatformMetricKeys.h>
#include <platform/PlatformManager.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <platform/internal/GenericPlatformManagerImpl.ipp>
#else
#include <platform/internal/GenericPlatformManagerImpl_POSIX.ipp>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#include <CoreFoundation/CoreFoundation.h>
#include <tracing/metric_event.h>

using namespace chip::Tracing::DarwinPlatform;

namespace chip {
namespace DeviceLayer {

AtomicGlobal<PlatformManagerImpl> PlatformManagerImpl::sInstance;

PlatformManagerImpl::PlatformManagerImpl() :
    mWorkQueue(dispatch_queue_create("org.csa-iot.matter.workqueue",
                                     dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL,
                                                                             QOS_CLASS_USER_INITIATED, QOS_MIN_RELATIVE_PRIORITY)))
{
    // Tag our queue for IsWorkQueueCurrentQueue()
    dispatch_queue_set_specific(mWorkQueue, this, this, nullptr);
    dispatch_suspend(mWorkQueue);
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    // Initialize the configuration system.
#if !CHIP_DISABLE_PLATFORM_KVS
    ReturnErrorOnFailure(Internal::PosixConfig::Init());
#endif // CHIP_DISABLE_PLATFORM_KVS

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    // Ensure there is a dispatch queue available
    static_cast<System::LayerSocketsLoop &>(DeviceLayer::SystemLayer()).SetDispatchQueue(GetWorkQueue());
#endif

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl<PlatformManagerImpl>::_InitChipStack());

#if !CHIP_DISABLE_PLATFORM_KVS
    // Now set up our device instance info provider.  We couldn't do that
    // earlier, because the generic implementation sets a generic one.
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
#endif // CHIP_DISABLE_PLATFORM_KVS

    mStartTime = System::SystemClock().GetMonotonicTimestamp();
    return CHIP_NO_ERROR;
}

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    auto expected = WorkQueueState::kSuspended;
    VerifyOrReturnError(mWorkQueueState.compare_exchange_strong(expected, WorkQueueState::kRunning), CHIP_ERROR_INCORRECT_STATE);
    dispatch_resume(mWorkQueue);
    return CHIP_NO_ERROR;
};

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    auto expected = WorkQueueState::kRunning;
    VerifyOrReturnError(mWorkQueueState.compare_exchange_strong(expected, WorkQueueState::kSuspensionPending),
                        CHIP_ERROR_INCORRECT_STATE);

    // We need to dispatch to the work queue to ensure any currently queued jobs
    // finish executing. When called from outside the work queue we also need to
    // wait for them to complete before returning to the caller, so we use
    // dispatch_sync in that case.
    (IsWorkQueueCurrentQueue() ? dispatch_async : dispatch_sync)(mWorkQueue, ^{
        dispatch_suspend(mWorkQueue);
        mWorkQueueState.store(WorkQueueState::kSuspended);
        auto * semaphore = mRunLoopSem;
        if (semaphore != nullptr)
        {
            dispatch_semaphore_signal(semaphore);
        }
    });
    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_RunEventLoop()
{
    mRunLoopSem = dispatch_semaphore_create(0);

    _StartEventLoopTask();

    //
    // Block on the semaphore till we're signalled to stop by
    // _StopEventLoopTask()
    //
    dispatch_semaphore_wait(mRunLoopSem, DISPATCH_TIME_FOREVER);
    dispatch_release(mRunLoopSem);
    mRunLoopSem = nullptr;
}

CHIP_ERROR PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{
    const ChipDeviceEvent eventCopy = *event;
    dispatch_async(mWorkQueue, ^{
        DispatchEvent(&eventCopy);
    });
    return CHIP_NO_ERROR;
}
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#if CHIP_STACK_LOCK_TRACKING_ENABLED
bool PlatformManagerImpl::_IsChipStackLockedByCurrentThread() const
{
    // Assume our caller knows what they are doing in terms of concurrency if the work queue is suspended.
    return IsWorkQueueCurrentQueue() || mWorkQueueState.load() == WorkQueueState::kSuspended;
};
#endif

bool PlatformManagerImpl::IsWorkQueueCurrentQueue() const
{
    return dispatch_get_specific(this) == this;
}

CHIP_ERROR PlatformManagerImpl::StartBleScan(BleScannerDelegate * delegate, BleScanMode mode)
{
#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailureWithMetric(kMetricBLEScan, Internal::BLEMgrImpl().StartScan(delegate, mode));
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR PlatformManagerImpl::StopBleScan()
{
#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailureWithMetric(kMetricBLEScan, Internal::BLEMgrImpl().StopScan());
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_NETWORK_LAYER_BLE
}

} // namespace DeviceLayer
} // namespace chip
