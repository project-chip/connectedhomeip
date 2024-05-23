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

#include <condition_variable>
#include <mutex>

namespace chip {
namespace ThreadWork {
namespace {

struct WorkData
{
    WorkCallback callback;
    std::mutex mux;
    std::condition_variable cond;
    bool done = false;

    void Post()
    {
        std::unique_lock lock(mux);
        done = true;
        cond.notify_all();
    }
    void Wait()
    {
        std::unique_lock lock(mux);
        cond.wait(lock, [&] { return done; });
    }
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
