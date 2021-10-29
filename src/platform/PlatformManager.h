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

#include <platform/CHIPDeviceBuildConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <system/SystemLayer.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <system/LwIPEventSupport.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {

namespace Dnssd {
class DiscoveryImplPlatform;
}

namespace DeviceLayer {

class ConnectivityManagerImpl;
class ConfigurationManagerImpl;
class TraitManager;
class ThreadStackManagerImpl;
class TimeSyncManager;

namespace Internal {
class DeviceControlServer;
class FabricProvisioningServer;
class ServiceProvisioningServer;
class BLEManagerImpl;
template <class>
class GenericConfigurationManagerImpl;
template <class>
class GenericConnectivityManagerImpl_Thread;
template <class>
class GenericThreadStackManagerImpl_OpenThread;
template <class>
class GenericThreadStackManagerImpl_OpenThread_LwIP;
} // namespace Internal

/**
 * Provides features for initializing and interacting with the chip network
 * stack on a chip-enabled device.
 */
class PlatformManager
{
public:
    // ===== Members that define the public interface of the PlatformManager

    typedef void (*EventHandlerFunct)(const ChipDeviceEvent * event, intptr_t arg);

    /**
     * InitChipStack() initializes the PlatformManager.  After calling that, a
     * consumer is allowed to call either StartEventLoopTask or RunEventLoop to
     * process pending work.  Calling both is not allowed: it must be one or the
     * other.
     */
    CHIP_ERROR InitChipStack()
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

        CHIP_ERROR err = InitChipStackInner();
        mInitialized   = (err == CHIP_NO_ERROR);
        return err;
    }

    CHIP_ERROR Shutdown()
    {
        mInitialized = false;
        return ShutdownInner();
    }

    virtual CHIP_ERROR InitChipStackInner() = 0;
    virtual CHIP_ERROR ShutdownInner() = 0;
    virtual CHIP_ERROR AddEventHandler(EventHandlerFunct handler, intptr_t arg = 0) = 0;
    virtual void RemoveEventHandler(EventHandlerFunct handler, intptr_t arg = 0) = 0;

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
    virtual void ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg = 0) = 0;

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
    virtual void RunEventLoop() = 0;

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
    virtual CHIP_ERROR StartEventLoopTask() = 0;

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
    virtual CHIP_ERROR StopEventLoopTask() = 0;
    virtual void LockChipStack() = 0;
    virtual bool TryLockChipStack() = 0;
    virtual void UnlockChipStack() = 0;

    /**
     * Software Diagnostics methods.
     */
    virtual CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) = 0;
    virtual CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) = 0;
    virtual CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) = 0;

    /**
     * General Diagnostics methods.
     */
    virtual CHIP_ERROR GetRebootCount(uint16_t & rebootCount) = 0;
    virtual CHIP_ERROR GetUpTime(uint64_t & upTime) = 0;
    virtual CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) = 0;
    virtual CHIP_ERROR GetBootReasons(uint8_t & bootReasons) = 0;

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    virtual bool IsChipStackLockedByCurrentThread() const = 0;
#endif

private:
    bool mInitialized = false;
    // ===== Members for internal use by the following friends.

    friend class ConnectivityManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class Dnssd::DiscoveryImplPlatform;
    friend class TraitManager;
    friend class ThreadStackManagerImpl;
    friend class TimeSyncManager;
    friend class Internal::DeviceControlServer;
    friend class Internal::FabricProvisioningServer;
    friend class Internal::ServiceProvisioningServer;
    friend class Internal::BLEManagerImpl;
    template <class>
    friend class Internal::GenericConnectivityManagerImpl_Thread;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_OpenThread;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_OpenThread_LwIP;
    template <class>
    friend class Internal::GenericConfigurationManagerImpl;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    friend class System::PlatformEventing;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

public:
    /*
     * PostEvent can be called safely on any thread without locking the stack.
     * When called from a thread that is not doing the stack work item
     * processing, the event might get dispatched (on the work item processing
     * thread) before PostEvent returns.
     */
    [[nodiscard]] virtual CHIP_ERROR PostEvent(const ChipDeviceEvent * event) = 0;

    void PostEventOrDie(const ChipDeviceEvent * event)
    {
        CHIP_ERROR status = PostEvent(event);
        VerifyOrDieWithMsg(status == CHIP_NO_ERROR, DeviceLayer, "Failed to post event %d: %" CHIP_ERROR_FORMAT,
            static_cast<int>(event->Type), status.Format());
    }

    virtual void DispatchEvent(const ChipDeviceEvent * event) = 0;
    virtual CHIP_ERROR StartChipTimer(System::Clock::Timeout duration) = 0;

protected:
    // Construction/destruction limited to subclasses.
    PlatformManager()  = default;
    virtual ~PlatformManager() = default;

private:
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
PlatformManager & PlatformMgr();

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
