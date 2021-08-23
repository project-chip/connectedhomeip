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

#include <platform/PlatformManager.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
#include <platform/internal/GenericPlatformManagerImpl.cpp>

#include <CoreFoundation/CoreFoundation.h>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::PosixConfig::Init();
    SuccessOrExit(err);

    mRunLoopSem = dispatch_semaphore_create(0);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    SystemLayer.SetDispatchQueue(GetWorkQueue());

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    if (mIsWorkQueueRunning == false)
    {
        mIsWorkQueueRunning = true;
        dispatch_resume(mWorkQueue);
    }

    return CHIP_NO_ERROR;
};

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    if (mIsWorkQueueRunning == true)
    {
        mIsWorkQueueRunning = false;
        if (dispatch_get_current_queue() != mWorkQueue)
        {
            // dispatch_sync is used in order to guarantee serialization of the caller with
            // respect to any tasks that might already be on the queue, or running.
            dispatch_sync(mWorkQueue, ^{
                dispatch_suspend(mWorkQueue);
            });
        }
        else
        {
            // We are called from a task running on our work queue.  Dispatch async,
            // so we don't deadlock ourselves.  Note that we do have to dispatch to
            // guarantee that we don't signal the semaphore until we have ensured
            // that no more tasks will run on the queue.
            dispatch_async(mWorkQueue, ^{
                dispatch_suspend(mWorkQueue);
                dispatch_semaphore_signal(mRunLoopSem);
            });
        }
    }

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_RunEventLoop()
{
    _StartEventLoopTask();

    //
    // Block on the semaphore till we're signalled to stop by
    // _StopEventLoopTask()
    //
    dispatch_semaphore_wait(mRunLoopSem, DISPATCH_TIME_FOREVER);
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    // Call up to the base class _Shutdown() to perform the bulk of the shutdown.
    return GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{
    const ChipDeviceEvent eventCopy = *event;
    dispatch_async(mWorkQueue, ^{
        Impl()->DispatchEvent(&eventCopy);
    });
}

} // namespace DeviceLayer
} // namespace chip
