/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <condition_variable>
#include <mutex>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.h>

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP
#include <gio/gio.h>
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Linux platforms.
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

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

    /**
     * @brief Executes a callback in the GLib main loop thread.
     *
     * @param[in] callback The callback to execute.
     * @param[in] userData User data to pass to the callback.
     * @param[in] wait If true, the function will block until the callback has been executed.
     * @returns CHIP_NO_ERROR if the callback was successfully executed.
     */
    CHIP_ERROR RunOnGLibMainLoopThread(GSourceFunc callback, void * userData, bool wait = false);

    /**
     * @brief Convenience method to require less casts to void pointers.
     */
    template <class T>
    CHIP_ERROR ScheduleOnGLibMainLoopThread(gboolean (*callback)(T *), T * userData, bool wait = false)
    {
        return RunOnGLibMainLoopThread(G_SOURCE_FUNC(callback), userData, wait);
    }

#endif

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

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP

    class CallbackIndirection
    {
    public:
        CallbackIndirection(GSourceFunc callback, void * userData) : mCallback(callback), mUserData(userData) {}
        void Wait(std::unique_lock<std::mutex> & lock);
        static gboolean Callback(CallbackIndirection * self);

    private:
        GSourceFunc mCallback;
        void * mUserData;
        // Sync primitives to wait for the callback to be executed.
        std::condition_variable mDoneCond;
        bool mDone = false;
    };

    // XXX: Mutex for guarding access to glib main event loop callback indirection
    //      synchronization primitives. This is a workaround to suppress TSAN warnings.
    //      TSAN does not know that from the thread synchronization perspective the
    //      g_source_attach() function should be treated as pthread_create(). Memory
    //      access to shared data before the call to g_source_attach() without mutex
    //      is not a race condition - the callback will not be executed on glib main
    //      event loop thread before the call to g_source_attach().
    std::mutex mGLibMainLoopCallbackIndirectionMutex;

    GMainLoop * mGLibMainLoop;
    GThread * mGLibMainLoopThread;

#endif // CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr()
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl()
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
