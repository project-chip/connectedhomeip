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
#include <errno.h>
#include <pthread.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <support/CHIPMem.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

namespace {

pthread_t sPlatformMainThread;

void * PlatformMainLoop(void *)
{
    ChipLogProgress(DeviceLayer, "Platform main loop started.");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "Platform main loop completed.");
    return nullptr;
}

} // namespace

extern "C" void pychip_native_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: memory init failed: %s", chip::ErrorStr(err));
    }

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: platform init failed: %s", chip::ErrorStr(err));
    }
    int result   = pthread_create(&sPlatformMainThread, nullptr, PlatformMainLoop, nullptr);
    int tmpErrno = errno;

    if (result != 0)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: pthread_create failed: %s", strerror(tmpErrno));
    }
}
