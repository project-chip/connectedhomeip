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
    CHIP_ERROR _Shutdown();
    CHIP_ERROR _AddEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg);
    void _RemoveEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg);
    void _ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg);
    void _DispatchEvent(const ChipDeviceEvent * event);

    CHIP_ERROR _GetCurrentHeapFree(uint64_t & currentHeapFree);
    CHIP_ERROR _GetCurrentHeapUsed(uint64_t & currentHeapUsed);
    CHIP_ERROR _GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark);

    CHIP_ERROR _GetRebootCount(uint16_t & rebootCount);
    CHIP_ERROR _GetUpTime(uint64_t & upTime);
    CHIP_ERROR _GetTotalOperationalHours(uint32_t & totalOperationalHours);
    CHIP_ERROR _GetBootReasons(uint8_t & bootReasons);

    // ===== Support methods that can be overridden by the implementation subclass.

    void DispatchEventToSystemLayer(const ChipDeviceEvent * event);
    void DispatchEventToDeviceLayer(const ChipDeviceEvent * event);
    void DispatchEventToApplication(const ChipDeviceEvent * event);
    static void HandleMessageLayerActivityChanged(bool messageLayerIsActive);

private:
    bool mMsgLayerWasActive;

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl<PlatformManagerImpl>;

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetRebootCount(uint16_t & rebootCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetUpTime(uint64_t & upTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_GetBootReasons(uint8_t & bootReasons)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
