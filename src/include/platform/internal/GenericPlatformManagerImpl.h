/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
