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

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          ESP32 platform.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ThreadStackManager.h>

#include <platform/ESP32/OpenthreadLauncher.h>
#include <platform/ESP32/ThreadStackManagerImpl.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp>

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_vfs_eventfd.h"
#include "lib/core/CHIPError.h"
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    openthread_launch_task();
    return GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(esp_openthread_get_instance());
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    // Intentionally empty.
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_LockThreadStack()
{
    esp_openthread_lock_acquire(portMAX_DELAY);
}

bool ThreadStackManagerImpl::_TryLockThreadStack()
{
    return esp_openthread_lock_acquire(0);
}

void ThreadStackManagerImpl::_UnlockThreadStack()
{
    esp_openthread_lock_release();
}

void ThreadStackManagerImpl::_ProcessThreadActivity()
{
    // Intentionally empty.
}
void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStart()
{
    // Intentionally empty.
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStop()
{
    // Intentionally empty.
}

} // namespace DeviceLayer
} // namespace chip
