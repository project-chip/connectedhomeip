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
 *          Provides an generic implementation of PlatformManager features
 *          for use on Zephyr RTOS platforms.
 */

#pragma once

#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <sys/select.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include <zephyr/net/socket.h>
#endif

#include <zephyr/kernel.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on Zephyr RTOS platforms.
 *
 * This template contains implementations of selected features from the PlatformManager abstract
 * interface that are suitable for use on Zephyr-based platforms.  It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl_Zephyr : public GenericPlatformManagerImpl<ImplClass>
{
protected:
    using ThreadStack = k_thread_stack_t *;

    // Members for select() loop
    int mMaxFd;
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;
    timeval mNextTimeout;

    // Lock for the whole CHIP stack
    k_mutex mChipStackLock;

    // Members for CHIP event processing
    ChipDeviceEvent mChipEventRingBuffer[CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE];
    k_msgq mChipEventQueue;

    // Main CHIP thread
    // Although defining thread stack as a class member is feasible it's discouraged according to
    // the Zephyr documentation (see remarks on K_THREAD_STACK_MEMBER macro). Therefore, this class
    // requires the stack reference to be passed in the constructor.
    ThreadStack mChipThreadStack;
    k_thread mChipThread;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();
    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StopEventLoopTask();
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout duration);
    void _Shutdown(void);

    // ===== Methods available to the implementation subclass.
    explicit GenericPlatformManagerImpl_Zephyr(ThreadStack stack) : mChipThreadStack(stack) {}

private:
    // ===== Private members for use by this class only.
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
    void SysUpdate();
    void SysProcess();
    void ProcessDeviceEvents();

    volatile bool mShouldRunEventLoop;

    bool mInitialized = false;

    static void EventLoopTaskMain(void * thisPtr, void *, void *);
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
