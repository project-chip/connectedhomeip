/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for SILABS platforms using the Silicon Labs SDK.
 */

#pragma once

#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#include "wfx_host_events.h"
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the SILABS platform.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

#if defined(TINYCRYPT_PRIMITIVES)
    // Since the RNG callback will be called from multiple threads,
    // use this mutex to lock/unlock the call to Matter RNG API, which
    // uses some global variables.
    static sys_mutex_t rngMutexHandle;

    // Callback used by tinycrypt to generate random numbers.
    // It must be set before calling any sign operations,
    // which are used in both Matter and OT threads.
    static int uECC_RNG_Function(uint8_t * dest, unsigned int size);
#endif

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;
#endif

public:
    // ===== Platform-specific members that may be accessed directly by the application.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    void HandleWFXSystemEvent(wfx_event_base_t eventBase, sl_wfx_generic_message_t * eventData);
#endif

    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);
    void _Shutdown(void);

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);
    friend class Internal::BLEManagerImpl;

    System::Clock::Timestamp mStartTime = System::Clock::kZero;

    static PlatformManagerImpl sInstance;

    using Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::PostEventFromISR;
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

} // namespace DeviceLayer
} // namespace chip
