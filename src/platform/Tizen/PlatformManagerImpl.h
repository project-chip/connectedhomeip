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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include <glib.h>

#include <lib/core/CHIPError.h>
#include <platform/ConnectivityManager.h>

#include "platform/internal/GenericPlatformManagerImpl_POSIX.h"

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Tizen platforms.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;
#endif

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    /**
     * @brief Invoke a function on the Matter GLib context.
     *
     * If execution of the function will have to be scheduled on other thread,
     * this call will block the current thread until the function is executed.
     *
     * @param[in] function The function to call.
     * @param[in] userData User data to pass to the function.
     * @returns The result of the function.
     */
    template <typename T>
    CHIP_ERROR GLibMatterContextInvokeSync(CHIP_ERROR (*func)(T *), T * userData)
    {
        return _GLibMatterContextInvokeSync((CHIP_ERROR(*)(void *)) func, (void *) userData);
    }
    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();
    void _Shutdown();

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr();
    friend PlatformManagerImpl & PlatformMgrImpl();
    friend class Internal::BLEManagerImpl;

    System::Clock::Timestamp mStartTime = System::Clock::kZero;

    static PlatformManagerImpl sInstance;

    /**
     * @brief Invoke a function on the Matter GLib context.
     *
     * @note This function does not provide type safety for the user data. Please,
     *       use the GLibMatterContextInvokeSync() template function instead.
     */
    CHIP_ERROR _GLibMatterContextInvokeSync(CHIP_ERROR (*func)(void *), void * userData);

    GMainLoop * mGLibMainLoop;
    GThread * mGLibMainLoopThread;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * Chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr()
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * Chip applications can use this to gain access to features of the PlatformManager
 * that are specific to Tizen platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl()
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
