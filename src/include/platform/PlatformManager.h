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

#ifndef PLATFORM_MANAGER_H
#define PLATFORM_MANAGER_H

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

System::Error PostEvent(System::Layer &, void *, System::Object &, System::EventType, uintptr_t);
System::Error DispatchEvents(System::Layer &, void *);
System::Error DispatchEvent(System::Layer &, void *, System::Event);
System::Error StartTimer(System::Layer &, void *, uint32_t);

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
class FabricProvisioningServer;
class ServiceProvisioningServer;
class BLEManagerImpl;
template <class>
class GenericBLEManagerImpl_Zephyr;
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
    void RunEventLoop(void);
    CHIP_ERROR StartEventLoopTask(void);
    void LockChipStack(void);
    bool TryLockChipStack(void);
    void UnlockChipStack(void);
    CHIP_ERROR Shutdown(void);

private:
    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConnectivityManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class TraitManager;
    friend class ThreadStackManagerImpl;
    friend class TimeSyncManager;
    friend class Internal::FabricProvisioningServer;
    friend class Internal::ServiceProvisioningServer;
    friend class Internal::BLEManagerImpl;
    template <class>
    friend class Internal::GenericBLEManagerImpl_Zephyr;
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
extern PlatformManager & PlatformMgr(void);

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the selected platform.
 */
extern PlatformManagerImpl & PlatformMgrImpl(void);

} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the ConfigurationManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_PLATFORMMANAGERIMPL_HEADER
#include EXTERNAL_PLATFORMMANAGERIMPL_HEADER
#else
#define PLATFORMMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/PlatformManagerImpl.h>
#include PLATFORMMANAGERIMPL_HEADER
#endif

namespace chip {
namespace DeviceLayer {

inline CHIP_ERROR PlatformManager::InitChipStack()
{
    return static_cast<ImplClass *>(this)->_InitChipStack();
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

inline void PlatformManager::RunEventLoop(void)
{
    static_cast<ImplClass *>(this)->_RunEventLoop();
}

inline CHIP_ERROR PlatformManager::StartEventLoopTask(void)
{
    return static_cast<ImplClass *>(this)->_StartEventLoopTask();
}

inline void PlatformManager::LockChipStack(void)
{
    static_cast<ImplClass *>(this)->_LockChipStack();
}

inline bool PlatformManager::TryLockChipStack(void)
{
    return static_cast<ImplClass *>(this)->_TryLockChipStack();
}

inline void PlatformManager::UnlockChipStack(void)
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

inline CHIP_ERROR PlatformManager::Shutdown(void)
{
    return static_cast<ImplClass *>(this)->_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip

#endif // PLATFORM_MANAGER_H
