/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <glib.h>
#include <semaphore.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// The main loop provides a thread-based implementation that runs
/// the GLib main loop.
///
/// The main loop is used execute callbacks (e.g. dbus notifications).
class MainLoop
{
public:
    /// Ensure that a thread with g_main_loop_run is executing.
    CHIP_ERROR EnsureStarted();

    /// Executes a callback on the underlying main loop.
    ///
    /// The main loop MUST have been started already.
    bool RunOnBluezThread(GSourceFunc closure, void * arg);

    /// Executes a callback on the underlying main loop and waits for
    /// the method to complete.
    ///
    /// The main loop MUST have been started already.
    bool RunOnBluezThreadAndWait(GSourceFunc closure, void * arg);

    /// Convenience method to require less casts to void*
    template <class T>
    bool Schedule(int (*callback)(T *), T * value)
    {
        return RunOnBluezThread(G_SOURCE_FUNC(callback), value);
    }

    /// Convenience method to require less casts to void*
    template <class T>
    bool ScheduleAndWait(int (*callback)(T *), T * value)
    {
        return RunOnBluezThreadAndWait(G_SOURCE_FUNC(callback), value);
    }

    /// Schedules a method to be executed after the main loop has finished
    ///
    /// A single cleanup method can exist and the main loop has to be running
    /// to set a cleanup method.
    template <class T>
    bool SetCleanupFunction(int (*callback)(T *), T * value)
    {
        if (mCleanup != nullptr)
        {
            return false;
        }

        if ((mBluezMainLoop == nullptr) || !g_main_loop_is_running(mBluezMainLoop))
        {
            return false;
        }

        mCleanup         = G_SOURCE_FUNC(callback);
        mCleanupArgument = static_cast<void *>(value);

        return true;
    }

    static MainLoop & Instance();

private:
    MainLoop() {}

    static void * Thread(void * self);

    GMainLoop * mBluezMainLoop = nullptr;
    pthread_t mThread          = 0;

    // allow a single cleanup method
    GSourceFunc mCleanup    = nullptr;
    void * mCleanupArgument = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
