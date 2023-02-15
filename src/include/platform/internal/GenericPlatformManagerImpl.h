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
 *          Provides an generic implementation of PlatformManager features
 *          for use on various platforms.
 */

#pragma once

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on multiple platforms.
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on all platforms.  It is intended to be inherited (directly
 * or indirectly) by the PlatformManagerImpl class, which also appears as the template's ImplClass
 * parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl
{
protected:
    struct AppEventHandler
    {
        AppEventHandler * Next;
        PlatformManager::EventHandlerFunct Handler;
        intptr_t Arg;
    };

    AppEventHandler * mAppEventHandlerList;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();
    void _Shutdown();
    CHIP_ERROR _AddEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg);
    void _RemoveEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg);
    void _HandleServerStarted();
    void _HandleServerShuttingDown();
    CHIP_ERROR _ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg);
    CHIP_ERROR _ScheduleBackgroundWork(AsyncWorkFunct workFunct, intptr_t arg);
    CHIP_ERROR _PostBackgroundEvent(const ChipDeviceEvent * event);
    void _RunBackgroundEventLoop(void);
    CHIP_ERROR _StartBackgroundEventLoopTask(void);
    CHIP_ERROR _StopBackgroundEventLoopTask();
    void _DispatchEvent(const ChipDeviceEvent * event);

    // ===== Support methods that can be overridden by the implementation subclass.

    void DispatchEventToDeviceLayer(const ChipDeviceEvent * event);
    void DispatchEventToApplication(const ChipDeviceEvent * event);
    static void HandleMessageLayerActivityChanged(bool messageLayerIsActive);

private:
    bool mMsgLayerWasActive;

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
