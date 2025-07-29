/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_Zephyr.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the Zephyr platforms.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>;
#endif

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    System::Clock::Timestamp GetStartTime() { return mStartTime; }
    CHIP_ERROR UpdateOperationalHours(uint32_t * totalOperationalHours);

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);

    static void OperationalHoursSavingTimerEventHandler(k_timer * timer);

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);
    friend class Internal::BLEManagerImpl;

    System::Clock::Timestamp mStartTime      = System::Clock::kZero;
    uint32_t mSavedOperationalHoursSinceBoot = 0;

    explicit PlatformManagerImpl(ThreadStack stack) : Internal::GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>(stack) {}

    static PlatformManagerImpl sInstance;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the Zephyr platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl()
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
