/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Contains non-inline method definitions for the
 *          GenericThreadStackManagerImpl_RTThread<> template.
 */

#ifndef GENERIC_THREAD_STACK_MANAGER_IMPL_RTTHREAD_IPP
#define GENERIC_THREAD_STACK_MANAGER_IMPL_RTTHREAD_IPP

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/RTThread/GenericThreadStackManagerImpl_RTThread.h>
#include <platform/RTThread/CHIPPlatformConfig.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_RTThread<ImplClass>::DoInit(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if defined(CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX) && CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX
    rt_mutex_init(&mThreadStackLockMutex,"ThreadStackLockMutex", RT_IPC_FLAG_FIFO);
#else
    mThreadStackLock = rt_mutex_create("ThreadStackLock", RT_IPC_FLAG_FIFO);
#endif // CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX

#if defined(CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT) && CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT
    rt_event_init(&mThreadStackNotifyEvent, "ThreadStackNotifyEvent", RT_IPC_FLAG_PRIO);
#else
    mThreadStackNotify = rt_event_create("ThreadStackEvent", RT_IPC_FLAG_PRIO);
#endif // CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT
    
    if (mThreadStackLock == RT_NULL)
    {
        ChipLogError(DeviceLayer, "Failed to create Thread stack lock");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
    }
    else if (mThreadStackNotify == RT_NULL)
    {
        ChipLogError(DeviceLayer, "Failed to create Thread stack notify");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
	}

    mThreadTask = RT_NULL;

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_RTThread<ImplClass>::_StartThreadTask(void)
{
    if (mThreadTask != RT_NULL)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
#if defined(CHIP_CONFIG_RTTHREAD_USE_STATIC_THREAD) && CHIP_CONFIG_RTTHREAD_USE_STATIC_THREAD
    rt_thread_init(&mThreadTask, 
                   CHIP_DEVICE_CONFIG_THREAD_TASK_NAME, 
                   ThreadTaskMain, 
                   RT_NULL, 
                   &mThreadStack[0], 
                   ArraySize(mThreadStack), 
                   CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY, 
                   RT_TICK_PER_SECOND);
#else
    mThreadTask = rt_thread_create(CHIP_DEVICE_CONFIG_THREAD_TASK_NAME, 
                                   ThreadTaskMain, 
                                   RT_NULL,
                                   ArraySize(mThreadStack), 
                                   CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY, 
                                   RT_TICK_PER_SECOND);
#endif
    
    if (mThreadTask == RT_NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    rt_thread_startup(mThreadTask);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_RTThread<ImplClass>::_LockThreadStack(void)
{
    rt_mutex_take(mThreadStackLock, RT_WAITING_FOREVER);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_RTThread<ImplClass>::_TryLockThreadStack(void)
{
    return rt_mutex_trytake(mThreadStackLock) == RT_EOK;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_RTThread<ImplClass>::_UnlockThreadStack(void)
{
    rt_mutex_release(mThreadStackLock);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_RTThread<ImplClass>::SignalThreadActivityPending()
{
    // Implement the logic to signal thread activity pending using RT-Thread's mechanism
    if(mThreadTask != NULL)
    {
		rt_event_send(&mThreadStackNotify, PENDING_EVENT_FLAG);
	}
}

template <class ImplClass>
rt_bool_t GenericThreadStackManagerImpl_RTThread<ImplClass>::SignalThreadActivityPendingFromISR()
{
    // Implement the logic to signal thread activity pending from an ISR using RT-Thread's mechanism
	rt_interrupt_enter();
    if (mThreadTask != NULL)
    {
		rt_event_send(&mThreadStackNotify, PENDING_EVENT_ISR_FLAG);
	}
	rt_interrupt_leave();
}

template <class ImplClass>
void GenericThreadStackManagerImpl_RTThread<ImplClass>::ThreadTaskMain(void * arg)
{
    GenericThreadStackManagerImpl_RTThread<ImplClass> * self =
        static_cast<GenericThreadStackManagerImpl_RTThread<ImplClass> *>(arg);

    ChipLogDetail(DeviceLayer, "Thread task running");

    while (1)
    {
        self->Impl()->LockThreadStack();
        self->Impl()->ProcessThreadActivity();
        self->Impl()->UnlockThreadStack();

        // Implement the logic to wait for a notification using RT-Thread's mechanism
        if(rt_event_recv(&mThreadStackNotify, 
                          PENDING_EVENT_FLAG,
                          RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER,
                          RT_NULL) == RT_EOK)
        {
            rt_schedule();
		}
        else if (rt_event_recv(&mThreadStackNotify, 
                          PENDING_EVENT_ISR_FLAG,
                          RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER,
                          RT_NULL) == RT_EOK)
        {
            rt_thread_yield();
		}
            
    }
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericThreadStackManagerImpl_RTThread<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_THREAD_STACK_MANAGER_IMPL_RTTHREAD_IPP