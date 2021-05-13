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

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

System::Error PostEvent(System::Layer & aLayer, void * aContext, System::Object & aTarget, System::EventType aType,
                        uintptr_t aArgument);
System::Error DispatchEvents(System::Layer & aLayer, void * aContext);
System::Error DispatchEvent(System::Layer & aLayer, void * aContext, System::Event aEvent);
System::Error StartTimer(System::Layer & aLayer, void * aContext, uint32_t aMilliseconds);

} // namespace Layer
} // namespace Platform
} // namespace System

namespace DeviceLayer {

class PlatformManagerImpl;
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
 * Provides features for initializing and interacting with the chip network
 * stack on a chip-enabled device.
 */
class PlatformManager
{
    using ImplClass = ::chip::DeviceLayer::PlatformManagerImpl;

public:
    // ===== Members that define the public interface of the PlatformManager

    typedef void (*EventHandlerFunct)(const ChipDeviceEvent * event, intptr_t arg);

    CHIP_ERROR InitChipStack();
    CHIP_ERROR AddEventHandler(EventHandlerFunct handler, intptr_t arg = 0);
    void RemoveEventHandler(EventHandlerFunct handler, intptr_t arg = 0);
    void ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg = 0);
    void RunEventLoop();
    CHIP_ERROR StartEventLoopTask();
    void LockChipStack();
    bool TryLockChipStack();
    void UnlockChipStack();
    CHIP_ERROR Shutdown();

#if defined(CHIP_STACK_LOCK_TRACKING_ENABLED)
    bool IsChipStackLockedByCurrentThread() const;
#endif

private:
    bool mInitialized = false;
    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConnectivityManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class TraitManager;
    friend class ThreadStackManagerImpl;
    friend class TimeSyncManager;
    friend class Internal::DeviceControlServer;
    friend class Internal::FabricProvisioningServer;
    friend class Internal::ServiceProvisioningServer;
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
    // Parentheses used to fix clang parsing issue with these declarations
    friend ::chip::System::Error(::chip::System::Platform::Layer::PostEvent)(::chip::System::Layer & aLayer, void * aContext,
                                                                             ::chip::System::Object & aTarget,
                                                                             ::chip::System::EventType aType, uintptr_t aArgument);
    friend ::chip::System::Error(::chip::System::Platform::Layer::DispatchEvents)(::chip::System::Layer & aLayer, void * aContext);
    friend ::chip::System::Error(::chip::System::Platform::Layer::DispatchEvent)(::chip::System::Layer & aLayer, void * aContext,
                                                                                 ::chip::System::Event aEvent);
    friend ::chip::System::Error(::chip::System::Platform::Layer::StartTimer)(::chip::System::Layer & aLayer, void * aContext,
                                                                              uint32_t aMilliseconds);

    void PostEvent(const ChipDeviceEvent * event);
    void DispatchEvent(const ChipDeviceEvent * event);
    CHIP_ERROR StartChipTimer(uint32_t durationMS);

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

#if defined(CHIP_STACK_LOCK_TRACKING_ENABLED)
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

inline void PlatformManager::ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    static_cast<ImplClass *>(this)->_ScheduleWork(workFunct, arg);
}

inline void PlatformManager::RunEventLoop()
{
    static_cast<ImplClass *>(this)->_RunEventLoop();
}

inline CHIP_ERROR PlatformManager::StartEventLoopTask()
{
    return static_cast<ImplClass *>(this)->_StartEventLoopTask();
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

inline void PlatformManager::PostEvent(const ChipDeviceEvent * event)
{
    static_cast<ImplClass *>(this)->_PostEvent(event);
}

inline void PlatformManager::DispatchEvent(const ChipDeviceEvent * event)
{
    static_cast<ImplClass *>(this)->_DispatchEvent(event);
}

inline CHIP_ERROR PlatformManager::StartChipTimer(uint32_t durationMS)
{
    return static_cast<ImplClass *>(this)->_StartChipTimer(durationMS);
}

inline CHIP_ERROR PlatformManager::Shutdown()
{
    return static_cast<ImplClass *>(this)->_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
