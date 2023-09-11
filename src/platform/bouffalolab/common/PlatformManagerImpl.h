/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <aos/yloop.h>
#endif
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the Bouffalo Lab platform.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;
#endif

protected:
    void _RunEventLoop(void);

public:
    // ===== Platform-specific members that may be accessed directly by the application.
    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.
    CHIP_ERROR _InitChipStack(void);
    void _Shutdown();

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);
    friend class Internal::BLEManagerImpl;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    friend void OnWiFiPlatformEvent(input_event_t * event, void * private_data);
#endif

    System::Clock::Timestamp mStartTime = System::Clock::kZero;

    static PlatformManagerImpl sInstance;

    using Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::PostEventFromISR;

    CHIP_ERROR PlatformInit(void);
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * Chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * Chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the ESP32 platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance;
}

inline void PlatformManagerImpl::_RunEventLoop(void)
{
    PlatformInit();
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_RunEventLoop();
}

} // namespace DeviceLayer
} // namespace chip
