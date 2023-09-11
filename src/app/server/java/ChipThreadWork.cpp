/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
