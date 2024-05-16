/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "driver/uart.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread.h"
#include "esp_openthread_cli.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/cli.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"
#include <lib/core/CHIPError.h>
#include <memory>

static esp_netif_t * openthread_netif                       = NULL;
static esp_openthread_platform_config_t * s_platform_config = NULL;
static TaskHandle_t cli_transmit_task                       = NULL;
static QueueHandle_t cli_transmit_task_queue                = NULL;
static constexpr uint16_t OTCLI_TRANSMIT_TASK_STACK_SIZE    = 1024;
static constexpr UBaseType_t OTCLI_TRANSMIT_TASK_PRIORITY   = 5;

CHIP_ERROR cli_transmit_task_post(std::unique_ptr<char[]> && cli_str)
{
    char * cmd = cli_str.get();
    if (!cli_transmit_task_queue || xQueueSend(cli_transmit_task_queue, &cmd, portMAX_DELAY) != pdTRUE)
    {
        return CHIP_ERROR_INTERNAL;
    }
    cli_str.release();
    return CHIP_NO_ERROR;
}

static int cli_output_callback(void * context, const char * format, va_list args)
{
    int ret = 0;
    char prompt_check[3];
    vsnprintf(prompt_check, sizeof(prompt_check), format, args);
    if (!strncmp(prompt_check, "> ", sizeof(prompt_check)) && cli_transmit_task)
    {
        xTaskNotifyGive(cli_transmit_task);
    }
    else
    {
        ret = vprintf(format, args);
    }
    return ret;
}

static void esp_openthread_matter_cli_init(void)
{
    otCliInit(esp_openthread_get_instance(), cli_output_callback, NULL);
}

static void cli_transmit_worker(void * context)
{
    cli_transmit_task_queue = xQueueCreate(8, sizeof(char *));
    if (!cli_transmit_task_queue)
    {
        vTaskDelete(NULL);
        return;
    }

    while (true)
    {
        char * cmd = NULL;
        if (xQueueReceive(cli_transmit_task_queue, &cmd, portMAX_DELAY) == pdTRUE)
        {
            if (cmd)
            {
                std::unique_ptr<char[]> cmd_ptr(cmd);
                esp_openthread_cli_input(cmd_ptr.get());
            }
            else
            {
                continue;
            }
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        }
    }
    vQueueDelete(cli_transmit_task_queue);
    vTaskDelete(NULL);
}

static esp_err_t cli_command_transmit_task(void)
{
    xTaskCreate(cli_transmit_worker, "otcli_trans", OTCLI_TRANSMIT_TASK_STACK_SIZE, xTaskGetCurrentTaskHandle(),
                OTCLI_TRANSMIT_TASK_PRIORITY, &cli_transmit_task);
    return ESP_OK;
}

static esp_netif_t * init_openthread_netif(const esp_openthread_platform_config_t * config)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_netif_t * netif    = esp_netif_new(&cfg);
    assert(netif != NULL);
    ESP_ERROR_CHECK(esp_netif_attach(netif, esp_openthread_netif_glue_init(config)));

    return netif;
}

static void ot_task_worker(void * context)
{
    // Run the main loop
    esp_openthread_launch_mainloop();

    esp_netif_destroy(openthread_netif);
    esp_openthread_netif_glue_deinit();

    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}

esp_err_t set_openthread_platform_config(esp_openthread_platform_config_t * config)
{
    if (!s_platform_config)
    {
        s_platform_config = (esp_openthread_platform_config_t *) malloc(sizeof(esp_openthread_platform_config_t));
        if (!s_platform_config)
        {
            return ESP_ERR_NO_MEM;
        }
    }
    memcpy(s_platform_config, config, sizeof(esp_openthread_platform_config_t));
    return ESP_OK;
}

esp_err_t openthread_init_stack(void)
{
    // Used eventfds:
    // * netif
    // * ot task queue
    // * radio driver
    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 3,
    };

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));
    assert(s_platform_config);
    // Initialize the OpenThread stack
    ESP_ERROR_CHECK(esp_openthread_init(s_platform_config));
#ifdef CONFIG_OPENTHREAD_CLI
    esp_openthread_matter_cli_init();
    cli_command_transmit_task();
#endif
    // Initialize the esp_netif bindings
    openthread_netif = init_openthread_netif(s_platform_config);
    free(s_platform_config);
    s_platform_config = NULL;
    return ESP_OK;
}

esp_err_t openthread_launch_task(void)
{
    xTaskCreate(ot_task_worker, "ot_task", CONFIG_THREAD_TASK_STACK_SIZE, xTaskGetCurrentTaskHandle(), 5, NULL);
    return ESP_OK;
}
