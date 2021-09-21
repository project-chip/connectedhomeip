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

#include <platform/ESP32/ESPThreadConfig.h>
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
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    esp_netif_t * openthread_netif          = NULL;
    esp_netif_config_t netif_cfg            = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config  = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };

    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 3,
    };
    VerifyOrExit(esp_netif_init() == ESP_OK, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(esp_vfs_eventfd_register(&eventfd_config) == ESP_OK, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(esp_openthread_init(&config) == ESP_OK, err = CHIP_ERROR_INTERNAL);
    openthread_netif = esp_netif_new(&netif_cfg);
    VerifyOrExit(openthread_netif != NULL, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(esp_netif_attach(openthread_netif, esp_openthread_netif_glue_init(&config)) == ESP_OK, err = CHIP_ERROR_INTERNAL);
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(NULL);
exit:
    return err;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    if (mThreadTask != NULL)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    xTaskCreate(ESPThreadTask, CHIP_DEVICE_CONFIG_THREAD_TASK_NAME, CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE / sizeof(StackType_t),
                this, CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY, &mThreadTask);
    if (mThreadTask == NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
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

void ThreadStackManagerImpl::ESPThreadTask(void * arg)
{
    esp_openthread_launch_mainloop();
    esp_openthread_netif_glue_deinit();
    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}

} // namespace DeviceLayer
} // namespace chip

extern "C" void otTaskletsSignalPending(otInstance * p_instance)
{
    // Intentionally empty
}
