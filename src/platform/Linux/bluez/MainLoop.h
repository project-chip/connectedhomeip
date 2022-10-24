/*
 *    Copyright (c) 2021 Project CHIP Authors
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

    static MainLoop & Instance();

private:
    MainLoop() {}

    static void * Thread(void * self);

    GMainLoop * mBluezMainLoop = nullptr;
    pthread_t mThread          = 0;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
