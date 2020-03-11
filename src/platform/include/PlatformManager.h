/*
 *
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
 *          Defines the public interface for the Device Layer PlatformManager object.
 */

#ifndef PLATFORM_MANAGER_H
#define PLATFORM_MANAGER_H

#include <Weave/DeviceLayer/WeaveDeviceEvent.h>

namespace nl {
namespace Weave {
namespace System {
namespace Platform {
namespace Layer {

System::Error PostEvent(System::Layer&, void*, System::Object&, System::EventType, uintptr_t);
System::Error DispatchEvents(System::Layer&, void*);
System::Error DispatchEvent(System::Layer&, void*, System::Event);
System::Error StartTimer(System::Layer&, void*, uint32_t);

} // namespace Layer
} // namespace Platform
} // namespace System

namespace DeviceLayer {

class PlatformManagerImpl;
class ConnectivityManagerImpl;
class ConfigurationManagerImpl;
class TraitManager;
class TimeSyncManager;
namespace Internal {
class FabricProvisioningServer;
class ServiceProvisioningServer;
class BLEManagerImpl;
template<class> class GenericConfigurationManagerImpl;
template<class> class GenericPlatformManagerImpl;
template<class> class GenericPlatformManagerImpl_FreeRTOS;
template<class> class GenericConnectivityManagerImpl_Thread;
template<class> class GenericThreadStackManagerImpl_OpenThread;
template<class> class GenericThreadStackManagerImpl_OpenThread_LwIP;
} // namespace Internal


/**
 * Provides features for initializing and interacting with the Weave network
 * stack on a Weave-enabled device.
 */
class PlatformManager
{
    using ImplClass = ::nl::Weave::DeviceLayer::PlatformManagerImpl;

public:

    // ===== Members that define the public interface of the PlatformManager

    typedef void (*EventHandlerFunct)(const WeaveDeviceEvent * event, intptr_t arg);

    WEAVE_ERROR InitWeaveStack();
    WEAVE_ERROR AddEventHandler(EventHandlerFunct handler, intptr_t arg = 0);
    void RemoveEventHandler(EventHandlerFunct handler, intptr_t arg = 0);
    void ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg = 0);
    void RunEventLoop(void);
    WEAVE_ERROR StartEventLoopTask(void);
    void LockWeaveStack(void);
    bool TryLockWeaveStack(void);
    void UnlockWeaveStack(void);

private:

    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConnectivityManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class TraitManager;
    friend class TimeSyncManager;
    friend class Internal::FabricProvisioningServer;
    friend class Internal::ServiceProvisioningServer;
    friend class Internal::BLEManagerImpl;
    template<class> friend class Internal::GenericPlatformManagerImpl;
    template<class> friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    template<class> friend class Internal::GenericConnectivityManagerImpl_Thread;
    template<class> friend class Internal::GenericThreadStackManagerImpl_OpenThread;
    template<class> friend class Internal::GenericThreadStackManagerImpl_OpenThread_LwIP;
    template<class> friend class Internal::GenericConfigurationManagerImpl;
    // Parentheses used to fix clang parsing issue with these declarations
    friend ::nl::Weave::System::Error (::nl::Weave::System::Platform::Layer::PostEvent(::nl::Weave::System::Layer & aLayer, void * aContext, ::nl::Weave::System::Object & aTarget, ::nl::Weave::System::EventType aType, uintptr_t aArgument));
    friend ::nl::Weave::System::Error (::nl::Weave::System::Platform::Layer::DispatchEvents(::nl::Weave::System::Layer & aLayer, void * aContext));
    friend ::nl::Weave::System::Error (::nl::Weave::System::Platform::Layer::DispatchEvent(::nl::Weave::System::Layer & aLayer, void * aContext, ::nl::Weave::System::Event aEvent));
    friend ::nl::Weave::System::Error (::nl::Weave::System::Platform::Layer::StartTimer(::nl::Weave::System::Layer & aLayer, void * aContext, uint32_t aMilliseconds));

    void PostEvent(const WeaveDeviceEvent * event);
    void DispatchEvent(const WeaveDeviceEvent * event);
    WEAVE_ERROR StartWeaveTimer(uint32_t durationMS);

protected:

    // Construction/destruction limited to subclasses.
    PlatformManager() = default;
    ~PlatformManager() = default;

    // No copy, move or assignment.
    PlatformManager(const PlatformManager &) = delete;
    PlatformManager(const PlatformManager &&) = delete;
    PlatformManager & operator=(const PlatformManager &) = delete;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * Weave applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
extern PlatformManager & PlatformMgr(void);

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * Weave applications can use this to gain access to features of the PlatformManager
 * that are specific to the selected platform.
 */
extern PlatformManagerImpl & PlatformMgrImpl(void);

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

/* Include a header file containing the implementation of the ConfigurationManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_PLATFORMMANAGERIMPL_HEADER
#include EXTERNAL_PLATFORMMANAGERIMPL_HEADER
#else
#define PLATFORMMANAGERIMPL_HEADER <Weave/DeviceLayer/WEAVE_DEVICE_LAYER_TARGET/PlatformManagerImpl.h>
#include PLATFORMMANAGERIMPL_HEADER
#endif

namespace nl {
namespace Weave {
namespace DeviceLayer {

inline WEAVE_ERROR PlatformManager::InitWeaveStack()
{
    return static_cast<ImplClass*>(this)->_InitWeaveStack();
}

inline WEAVE_ERROR PlatformManager::AddEventHandler(EventHandlerFunct handler, intptr_t arg)
{
    return static_cast<ImplClass*>(this)->_AddEventHandler(handler, arg);
}

inline void PlatformManager::RemoveEventHandler(EventHandlerFunct handler, intptr_t arg)
{
    static_cast<ImplClass*>(this)->_RemoveEventHandler(handler, arg);
}

inline void PlatformManager::ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    static_cast<ImplClass*>(this)->_ScheduleWork(workFunct, arg);
}

inline void PlatformManager::RunEventLoop(void)
{
    static_cast<ImplClass*>(this)->_RunEventLoop();
}

inline WEAVE_ERROR PlatformManager::StartEventLoopTask(void)
{
    return static_cast<ImplClass*>(this)->_StartEventLoopTask();
}

inline void PlatformManager::LockWeaveStack(void)
{
    static_cast<ImplClass*>(this)->_LockWeaveStack();
}

inline bool PlatformManager::TryLockWeaveStack(void)
{
    return static_cast<ImplClass*>(this)->_TryLockWeaveStack();
}

inline void PlatformManager::UnlockWeaveStack(void)
{
    static_cast<ImplClass*>(this)->_UnlockWeaveStack();
}

inline void PlatformManager::PostEvent(const WeaveDeviceEvent * event)
{
    static_cast<ImplClass*>(this)->_PostEvent(event);
}

inline void PlatformManager::DispatchEvent(const WeaveDeviceEvent * event)
{
    static_cast<ImplClass*>(this)->_DispatchEvent(event);
}

inline WEAVE_ERROR PlatformManager::StartWeaveTimer(uint32_t durationMS)
{
    return static_cast<ImplClass*>(this)->_StartWeaveTimer(durationMS);
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl



#endif // PLATFORM_MANAGER_H
