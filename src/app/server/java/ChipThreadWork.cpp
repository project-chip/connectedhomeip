/*
 *
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

#include "ChipThreadWork.h"
#include <platform/CHIPDeviceLayer.h>
#include <semaphore.h>

namespace chip {
namespace ThreadWork {
namespace {

struct WorkData
{
    WorkCallback callback;
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

void ChipMainThreadScheduleAndWait(WorkCallback callback)
{
    WorkData workdata;
    workdata.callback = callback;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(PerformWork, reinterpret_cast<intptr_t>(&workdata));

    workdata.Wait();
}

} // namespace ThreadWork
} // namespace chip
