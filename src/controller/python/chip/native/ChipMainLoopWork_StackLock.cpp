/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ChipMainLoopWork.h"

#include <platform/PlatformManager.h>

namespace chip {
namespace MainLoopWork {

void ExecuteInMainLoop(std::function<void()> f)
{
    // NOTE: requires CHIP_STACK_LOCK_TRACKING_ENABLED to be available (which python builds
    //       generally have) to ensure chip stack locks are not deadlocking, since these
    //       functions do not know the actual state of the chip main loop.
    //
    // TODO: it may be a good assumption that python code asking for this will NOT run in
    //       chip main loop, however we try to be generic
    if (chip::DeviceLayer::PlatformMgr().IsChipStackLockedByCurrentThread())
    {
        f();
        return;
    }

    chip::DeviceLayer::StackLock lock;
    f();
}

} // namespace MainLoopWork
} // namespace chip
