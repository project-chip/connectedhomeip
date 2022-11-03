/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/LockTracker.h>

#if CHIP_STACK_LOCK_TRACKING_ENABLED

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
namespace chip {
namespace Platform {
namespace Internal {

void AssertChipStackLockedByCurrentThread(const char * file, int line)
{
    if (!chip::DeviceLayer::PlatformMgr().IsChipStackLockedByCurrentThread())
    {
        ChipLogError(DeviceLayer, "Chip stack locking error at '%s:%d'. Code is unsafe/racy", file, line);
#if CHIP_STACK_LOCK_TRACKING_ERROR_FATAL
        chipDie();
#endif
    }
}

} // namespace Internal
} // namespace Platform
} // namespace chip

#endif
