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
 *          Defines the public interface for the Device Layer PlatformManager object.
 */

#pragma once

#include <platform/AttributeList.h>
#include <platform/CHIPDeviceBuildConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemLayer.h>

namespace chip {

namespace Dnssd {
class DiscoveryImplPlatform;
}

namespace DeviceLayer {

static constexpr size_t kMaxCalendarTypes = 12;

class PlatformManagerImpl;
class ConnectivityManagerImpl;
class ConfigurationManagerImpl;
class DeviceControlServer;
class TraitManager;
class ThreadStackManagerImpl;
class TimeSyncManager;

namespace Internal {
class BLEManagerImpl;
template <class>
class GenericConfigurationManagerImpl;
template <class>
class GenericPlatformManagerImpl;
template <class>
class GenericPlatformManagerImpl_FreeRTOS;
template <class>
class GenericPlatformManagerImpl_POSIX;
template <class>
class GenericPlatformManagerImpl_Zephyr;
template <class>
class GenericConnectivityManagerImpl_Thread;
template <class>
class GenericThreadStackManagerImpl_OpenThread;
template <class>
class GenericThreadStackManagerImpl_OpenThread_LwIP;
} // namespace Internal

/**
 * Defines the delegate class of Platform Manager to notify platform updates.
 */
class PlatformManagerDelegate
{
public:
    virtual ~PlatformManagerDelegate() {}

    /**
     * @brief
     *   Called by the current Node after completing a boot or reboot process.
     */
    virtual void OnStartUp(uint32_t softwareVersion) {}

    /**
     * @brief
     *   Called by the current Node prior to any orderly shutdown sequence on a
     *   best-effort basis.
     */
    virtual void OnShutDown() {}
};

/**
 * Provides features for initializing and interacting with the chip network
 * stack on a chip-enabled device.
 */
class PlatformManager
{
    using ImplClass = ::chip::DeviceLayer::PlatformManagerImpl;

public:
    // ===== Members that define the public interface of the PlatformManager

    typedef void (*EventHandlerFunct)(const ChipDeviceEvent * event, intptr_t arg);

    /**
     * InitChipStack() initializes the PlatformManager.  After calling that, a
     * consumer is allowed to call either StartEventLoopTask or RunEventLoop to
     * process pending work.  Calling both is not allowed: it must be one or the
     * other.
     */
    CHIP_ERROR InitChipStack();

    CHIP_ERROR AddEventHandler(EventHandlerFunct handler, intptr_t arg = 0);
    void RemoveEventHandler(EventHandlerFunct handler, intptr_t arg = 0);
    void SetDelegate(PlatformManagerDelegate * delegate) { mDelegate = delegate; }
    PlatformManagerDelegate * GetDelegate() const { return mDelegate; }

    /**
     * Should be called after initializing all layers of the Matter stack to
     * run all needed post-startup actions.
     */
    void HandleServerStarted();

    /**
     * Should be called before shutting down the Matter stack or restarting the
     * application to run all needed pre-shutdown actions.
     */
    void HandleServerShuttingDown();

    /**
     * ScheduleWork can be called after InitChipStack has been called.  Calls
     * that happen before either StartEventLoopTask or RunEventLoop will queue
     * the work up but that work will NOT run until one of those functions is
     * called.
     *
     * ScheduleWork can be called safely on any thread without locking the
     * stack.  When called from a thread that is not doing the stack work item
     * processing, the callback function may be called (on the work item
     * processing thread) before ScheduleWork returns.
     */
    CHIP_ERROR ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg = 0);

    /**
     * Process work items until StopEventLoopTask is called.  RunEventLoop will
     * not return until work item processing is stopped.  Once it returns it
     * guarantees that no more work items will be processed unless there's
     * another call to RunEventLoop.
     *
     * Consumers that call RunEventLoop must not call StartEventLoopTask.
     *
     * Consumers that call RunEventLoop must ensure that RunEventLoop returns
     * before calling Shutdown.
     */
    void RunEventLoop();

    /**
     * Process work items until StopEventLoopTask is called.
     *
     * StartEventLoopTask processes items asynchronously.  It can return before
     * any items are processed, or after some items have been processed, or
     * while an item is being processed, or even after StopEventLoopTask() has
     * been called (e.g. if ScheduleWork() was called before StartEventLoopTask
     * was called, with a work item that calls StopEventLoopTask).
     *
     * Consumers that call StartEventLoopTask must not call RunEventLoop.
     *
     * Consumers that call StartEventLoopTask must ensure that they call
     * StopEventLoopTask before calling Shutdown.
     */
    CHIP_ERROR StartEventLoopTask();

    /**
     * Stop processing of work items by the event loop.
     *
     * If called from outside work item processing, StopEventLoopTask guarantees
     * that any currently-executing work item completes execution and no more
     * work items will run after StopEventLoopTask returns.  This is generally
     * how StopEventLoopTask is used in conjunction with StartEventLoopTask.
     *
     * If called from inside work item processing, StopEventLoopTask makes no
     * guarantees about exactly when work item processing will stop.  What it
     * does guarantee is that if it is used this way in conjunction with
     * RunEventLoop then all work item processing will stop before RunEventLoop
     * returns.
     */
    CHIP_ERROR StopEventLoopTask();

    void LockChipStack();
    bool TryLockChipStack();
    void UnlockChipStack();
    void Shutdown();

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool IsChipStackLockedByCurrentThread() const;
#endif

    /*
     * PostEvent can be called safely on any thread without locking the stack.
     * When called from a thread that is not doing the stack work item
     * processing, the event might get dispatched (on the work item processing
     * thread) before PostEvent returns.
     */
    [[nodiscard]] CHIP_ERROR PostEvent(const ChipDeviceEvent * event);
    void PostEventOrDie(const ChipDeviceEvent * event);

    /**
     * Generally this function has the same semantics as ScheduleWork
     * except it applies to background processing.
     *
     * Delegates to PostBackgroundEvent (which will delegate to PostEvent if
     * CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING is not true).
     */
    CHIP_ERROR ScheduleBackgroundWork(AsyncWorkFunct workFunct, intptr_t arg = 0);

    /**
     * Generally this function has the same semantics as PostEvent
     * except it applies to background processing.
     *
     * If CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING is not true, will delegate
     * to PostEvent.
     *
     * Only accepts events of type kCallWorkFunct or kNoOp.
     *
     * Returns CHIP_ERROR_INVALID_ARGUMENT if the event type is not acceptable.
     * Returns CHIP_ERROR_NO_MEMORY if resources are exhausted.
     */
    CHIP_ERROR PostBackgroundEvent(const ChipDeviceEvent * event);

    /**
     * Generally this function has the same semantics as RunEventLoop
     * except it applies to background processing.
     */
    void RunBackgroundEventLoop();

    /**
     * Generally this function has the same semantics as StartEventLoopTask
     * except it applies to background processing.
     */
    CHIP_ERROR StartBackgroundEventLoopTask();

    /**
     * Generally this function has the same semantics as StopEventLoopTask
     * except it applies to background processing.
     */
    CHIP_ERROR StopBackgroundEventLoopTask();

private:
    bool mInitialized                   = false;
    PlatformManagerDelegate * mDelegate = nullptr;

    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConnectivityManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class DeviceControlServer;
    friend class Dnssd::DiscoveryImplPlatform;
    friend class FailSafeContext;
    friend class TraitManager;
    friend class ThreadStackManagerImpl;
    friend class TimeSyncManager;
    friend class Internal::BLEManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_POSIX;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_Zephyr;
    template <class>
    friend class Internal::GenericConnectivityManagerImpl_Thread;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_OpenThread;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_OpenThread_LwIP;
    template <class>
    friend class Internal::GenericConfigurationManagerImpl;
    friend class System::PlatformEventing;

    void DispatchEvent(const ChipDeviceEvent * event);
    CHIP_ERROR StartChipTimer(System::Clock::Timeout duration);

protected:
    // Construction/destruction limited to subclasses.
    PlatformManager()  = default;
    ~PlatformManager() = default;

    // No copy, move or assignment.
    PlatformManager(const PlatformManager &)  = delete;
    PlatformManager(const PlatformManager &&) = delete;
    PlatformManager & operator=(const PlatformManager &) = delete;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
extern PlatformManager & PlatformMgr();

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the selected platform.
 */
extern PlatformManagerImpl & PlatformMgrImpl();

/**
 * @brief
 * RAII locking for PlatformManager to simplify management of
 * LockChipStack()/UnlockChipStack calls.
 */
class StackLock
{
public:
    StackLock() { PlatformMgr().LockChipStack(); }

    ~StackLock() { PlatformMgr().UnlockChipStack(); }
};

/**
 * @brief
 * RAII unlocking for PlatformManager to simplify management of
 * LockChipStack()/UnlockChipStack calls.
 */
class StackUnlock
{
public:
    StackUnlock() { PlatformMgr().UnlockChipStack(); }
    ~StackUnlock() { PlatformMgr().LockChipStack(); }
};

} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the ConfigurationManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_PLATFORMMANAGERIMPL_HEADER
#include EXTERNAL_PLATFORMMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define PLATFORMMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/PlatformManagerImpl.h>
#include PLATFORMMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {

#if CHIP_STACK_LOCK_TRACKING_ENABLED
inline bool PlatformManager::IsChipStackLockedByCurrentThread() const
{
    return static_cast<const ImplClass *>(this)->_IsChipStackLockedByCurrentThread();
}
#endif

inline CHIP_ERROR PlatformManager::InitChipStack()
{
    // NOTE: this is NOT thread safe and cannot be as the chip stack lock is prepared by
    // InitChipStack itself on many platforms.
    //
    // In the future, this could be moved into specific platform code (where it can
    // be made thread safe). In general however, init twice
    // is likely a logic error and we may want to avoid that path anyway. Likely to
    // be done once code stabilizes a bit more.
    if (mInitialized)
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = static_cast<ImplClass *>(this)->_InitChipStack();
    mInitialized   = (err == CHIP_NO_ERROR);
    return err;
}

inline CHIP_ERROR PlatformManager::AddEventHandler(EventHandlerFunct handler, intptr_t arg)
{
    return static_cast<ImplClass *>(this)->_AddEventHandler(handler, arg);
}

inline void PlatformManager::RemoveEventHandler(EventHandlerFunct handler, intptr_t arg)
{
    static_cast<ImplClass *>(this)->_RemoveEventHandler(handler, arg);
}

inline void PlatformManager::HandleServerStarted()
{
    static_cast<ImplClass *>(this)->_HandleServerStarted();
}

inline void PlatformManager::HandleServerShuttingDown()
{
    static_cast<ImplClass *>(this)->_HandleServerShuttingDown();
}

inline CHIP_ERROR PlatformManager::ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    return static_cast<ImplClass *>(this)->_ScheduleWork(workFunct, arg);
}

inline void PlatformManager::RunEventLoop()
{
    static_cast<ImplClass *>(this)->_RunEventLoop();
}

/**
 * @brief
 *  Starts the stack on its own task with an associated event queue
 *  to dispatch and handle events posted to that task.
 *
 *  This is thread-safe.
 *  This is *NOT SAFE* to call from within the CHIP event loop since it can grab the stack lock.
 */
inline CHIP_ERROR PlatformManager::StartEventLoopTask()
{
    return static_cast<ImplClass *>(this)->_StartEventLoopTask();
}

/**
 * @brief
 *  This will trigger the event loop to exit and block till it has exited the loop.
 *  This prevents the processing of any further events in the queue.
 *
 *  Additionally, this stops the CHIP task if the following criteria are met:
 *      1. One was created earlier through a call to StartEventLoopTask
 *      2. This call isn't being made from that task.
 *
 *  This is safe to call from any task.
 *  This is safe to call from within the CHIP event loop.
 *
 */
inline CHIP_ERROR PlatformManager::StopEventLoopTask()
{
    return static_cast<ImplClass *>(this)->_StopEventLoopTask();
}

/**
 * @brief
 *   Shuts down and cleans up the main objects in the CHIP stack.
 *   This DOES NOT stop the chip thread or event queue from running.
 *
 */
inline void PlatformManager::Shutdown()
{
    static_cast<ImplClass *>(this)->_Shutdown();
    mInitialized = false;
}

inline void PlatformManager::LockChipStack()
{
    static_cast<ImplClass *>(this)->_LockChipStack();
}

inline bool PlatformManager::TryLockChipStack()
{
    return static_cast<ImplClass *>(this)->_TryLockChipStack();
}

inline void PlatformManager::UnlockChipStack()
{
    static_cast<ImplClass *>(this)->_UnlockChipStack();
}

inline CHIP_ERROR PlatformManager::PostEvent(const ChipDeviceEvent * event)
{
    return static_cast<ImplClass *>(this)->_PostEvent(event);
}

inline void PlatformManager::PostEventOrDie(const ChipDeviceEvent * event)
{
    CHIP_ERROR status = static_cast<ImplClass *>(this)->_PostEvent(event);
    VerifyOrDieWithMsg(status == CHIP_NO_ERROR, DeviceLayer, "Failed to post event %d: %" CHIP_ERROR_FORMAT,
                       static_cast<int>(event->Type), status.Format());
}

inline CHIP_ERROR PlatformManager::ScheduleBackgroundWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    return static_cast<ImplClass *>(this)->_ScheduleBackgroundWork(workFunct, arg);
}

inline CHIP_ERROR PlatformManager::PostBackgroundEvent(const ChipDeviceEvent * event)
{
    return static_cast<ImplClass *>(this)->_PostBackgroundEvent(event);
}

inline void PlatformManager::RunBackgroundEventLoop()
{
    static_cast<ImplClass *>(this)->_RunBackgroundEventLoop();
}

inline CHIP_ERROR PlatformManager::StartBackgroundEventLoopTask()
{
    return static_cast<ImplClass *>(this)->_StartBackgroundEventLoopTask();
}

inline CHIP_ERROR PlatformManager::StopBackgroundEventLoopTask()
{
    return static_cast<ImplClass *>(this)->_StopBackgroundEventLoopTask();
}

inline void PlatformManager::DispatchEvent(const ChipDeviceEvent * event)
{
    static_cast<ImplClass *>(this)->_DispatchEvent(event);
}

inline CHIP_ERROR PlatformManager::StartChipTimer(System::Clock::Timeout duration)
{
    return static_cast<ImplClass *>(this)->_StartChipTimer(duration);
}

} // namespace DeviceLayer
} // namespace chip
