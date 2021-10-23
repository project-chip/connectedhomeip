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

#include "OpenThreadLaunch.h"

#include "sdkconfig.h"

#if CONFIG_OPENTHREAD_ENABLED

#include <stdio.h>
#include <unistd.h>

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/platform/logging.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32H2
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                                                                      \
    {                                                                                                                              \
        .radio_mode = RADIO_MODE_NATIVE,                                                                                           \
    }
#else
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                                                                      \
    {                                                                                                                              \
        .radio_mode = RADIO_MODE_UART_RCP,                    \
        .radio_uart_config = {                                \
            .port = 1,                                        \
            .uart_config =                                    \
                {                                             \
                    .baud_rate = 115200,                      \
                    .data_bits = UART_DATA_8_BITS,            \
                    .parity = UART_PARITY_DISABLE,            \
                    .stop_bits = UART_STOP_BITS_1,            \
                    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,    \
                    .rx_flow_ctrl_thresh = 0,                 \
                    .source_clk = UART_SCLK_APB,              \
                },                                            \
            .rx_pin = 4,                                      \
            .tx_pin = 5,                                      \
        },                                                                    \
    }
#endif

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                                                                       \
    {                                                                                                                              \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                                                                         \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                                                                       \
    {                                                                                                                              \
        .storage_partition_name = "ot_storage", .netif_queue_size = 10, .task_queue_size = 10,                                     \
    }

static void OpenThreadTask(void * aContext)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config  = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };

    // Initialize the OpenThread stack
    ESP_ERROR_CHECK(esp_openthread_init(&config));
    (void) otLoggingSetLevel(OT_LOG_LEVEL_INFO);

    // Run the main loop
    esp_openthread_launch_mainloop();

    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}

void LaunchOpenThread()
{
    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 3,
    };
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));
    xTaskCreate(OpenThreadTask, "ot", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}

#endif // CONFIG_OPENTHREAD_ENABLED
