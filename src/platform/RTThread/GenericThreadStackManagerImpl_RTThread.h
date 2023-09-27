/*
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

/**
 *    @file
 *          Provides a generic implementation of ThreadStackManager features
 *          for use on RT-Thread platforms.
 */

#pragma once

#include "rtthread.h"
#include "rthw.h"

namespace chip {
namespace DeviceLayer {

class ThreadStackManagerImpl;

namespace Internal {

template <class ImplClass>
class GenericThreadStackManagerImpl_RTThread
{

protected:
    // Methods that implement the ThreadStackManager abstract interface.
    CHIP_ERROR _StartThreadTask(void);
    void _LockThreadStack(void);
    bool _TryLockThreadStack(void);
    void _UnlockThreadStack(void);

    // Members available to the implementation subclass.
    rt_sem_t mThreadStackLock;
    rt_event_t mThreadStackNotify;
    rt_thread_t mThreadTask;

    CHIP_ERROR DoInit();
    void SignalThreadActivityPending();

private:
    // Private members for use by this class only.
    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    static void ThreadTaskMain(void * arg);

#if defined(CHIP_CONFIG_RTTHREAD_USE_STATIC_TASK) && CHIP_CONFIG_RTTHREAD_USE_STATIC_TASK
    ALIGN(RT_ALIGN_SIZE)
    static rt_uint8_t mThreadStack[CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE];
    static struct rt_thread mThread;
#endif

#if defined(CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX) && CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX
    static struct rt_mutex mThreadStackLockMutex;
#endif

#if defined(CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT) && CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT
    static struct rt_event mThreadStackNotifyEvent;
#endif

};

extern template class GenericThreadStackManagerImpl_RTThread<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip