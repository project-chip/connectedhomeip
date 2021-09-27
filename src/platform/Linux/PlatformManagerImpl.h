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

#include <memory>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.h>

#if CHIP_WITH_GIO
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
#if CHIP_WITH_GIO
    GDBusConnection * GetGDBusConnection();
#endif

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();
    CHIP_ERROR _Shutdown();
    CHIP_ERROR _GetCurrentHeapFree(uint64_t & currentHeapFree);
    CHIP_ERROR _GetCurrentHeapUsed(uint64_t & currentHeapUsed);
    CHIP_ERROR _GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark);

    CHIP_ERROR _GetRebootCount(uint16_t & rebootCount);
    CHIP_ERROR _GetUpTime(uint64_t & upTime);
    CHIP_ERROR _GetTotalOperationalHours(uint32_t & totalOperationalHours);
    CHIP_ERROR _GetBootReasons(uint8_t & bootReasons);

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr();
    friend PlatformManagerImpl & PlatformMgrImpl();
    friend class Internal::BLEManagerImpl;

    uint64_t mStartTimeMilliseconds = 0;

    static PlatformManagerImpl sInstance;

    // The temporary hack for getting IP address change on linux for network provisioning in the rendezvous session.
    // This should be removed or find a better place once we depercate the rendezvous session.
    static void WiFIIPChangeListener();

#if CHIP_WITH_GIO
    struct GDBusConnectionDeleter
    {
        void operator()(GDBusConnection * conn) { g_object_unref(conn); }
    };
    using UniqueGDBusConnection = std::unique_ptr<GDBusConnection, GDBusConnectionDeleter>;
    UniqueGDBusConnection mpGDBusConnection;
#endif
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
 * that are specific to the ESP32 platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl()
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
