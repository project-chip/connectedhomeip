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

#include <platform/CHIPDeviceLayer.h>
#include <semaphore.h>

namespace chip {
namespace MainLoopWork {
namespace {

struct WorkData
{
    std::function<void()> callback;
    sem_t done;

    WorkData() { sem_init(&done, 0 /* shared */, 0); }
    ~WorkData() { sem_destroy(&done); }
    void Post() { sem_post(&done); }
    void Wait() { sem_wait(&done); }
};

void PerformWork(intptr_t arg)
{
    WorkData * work = reinterpret_cast<WorkData *>(arg);

    work->callback();
    work->Post();
}

} // namespace

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

    // NOTE: the code below assumes that chip main loop is running.
    //       if it does not, this will deadlock.
    //
    //       IsChipStackLockedByCurrentThread is expected to be aware of main loop
    //       not running.
    WorkData workdata;
    workdata.callback = f;
    chip::DeviceLayer::PlatformMgr().ScheduleWork(PerformWork, reinterpret_cast<intptr_t>(&workdata));
    workdata.Wait();
}

} // namespace MainLoopWork
} // namespace chip
