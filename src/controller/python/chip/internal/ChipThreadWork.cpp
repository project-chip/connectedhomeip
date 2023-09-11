/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ChipThreadWork.h"

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace python {
namespace {

struct WorkData
{
    WorkCallback callback;
#ifdef __APPLE__
    dispatch_semaphore_t done;

    WorkData() { done = dispatch_semaphore_create(0); }
    ~WorkData() { dispatch_release(done); }
    void Post() { dispatch_semaphore_signal(done); }
    void Wait() { dispatch_semaphore_wait(done, DISPATCH_TIME_FOREVER); }
#else
    sem_t done;

    WorkData() { sem_init(&done, 0 /* shared */, 0); }
    ~WorkData() { sem_destroy(&done); }
    void Post() { sem_post(&done); }
    void Wait() { sem_wait(&done); }
#endif
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

} // namespace python
} // namespace chip
