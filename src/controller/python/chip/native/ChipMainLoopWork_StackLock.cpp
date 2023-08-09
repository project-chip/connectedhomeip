/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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
