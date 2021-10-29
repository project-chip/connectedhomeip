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

#include <platform/POSIX/GenericPlatformManagerImpl_POSIX.h>

#if CHIP_WITH_GIO
#include <gio/gio.h>
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Linux platforms.
 */
class PlatformManagerImpl final : public Internal::GenericPlatformManagerImpl_POSIX
{
public:
    // ===== Platform-specific members that may be accessed directly by the application.
#if CHIP_WITH_GIO
    GDBusConnection * GetGDBusConnection();
#endif

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR InitChipStackInner() override;
    CHIP_ERROR ShutdownInner() override;
    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReasons(uint8_t & bootReasons) override;

    // ===== Members for internal use by the following friends.

    friend class Internal::BLEManagerImpl;

    uint64_t mStartTimeMilliseconds = 0;

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

} // namespace DeviceLayer
} // namespace chip
