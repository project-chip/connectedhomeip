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

#include "platform/ESP32/OpenthreadLauncher.h"
#include "driver/uart.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_cli.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_types.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/cli.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <memory>

#ifdef CONFIG_OPENTHREAD_BORDER_ROUTER
#include <esp_openthread_border_router.h>
#endif

static esp_openthread_platform_config_t * s_platform_config = nullptr;
static TaskHandle_t openthread_task                         = nullptr;
static const char * TAG                                     = "OpenThread";

esp_err_t openthread_init_netif_stack(void);
esp_err_t openthread_init_netif_glue(const esp_openthread_platform_config_t * config);
void openthread_deinit_netif_glue(void);

#ifdef CONFIG_OPENTHREAD_CLI
static TaskHandle_t cli_transmit_task                     = nullptr;
static QueueHandle_t cli_transmit_task_queue              = nullptr;
static constexpr uint16_t OTCLI_TRANSMIT_TASK_STACK_SIZE  = 1024;
static constexpr UBaseType_t OTCLI_TRANSMIT_TASK_PRIORITY = 5;

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

static void cli_command_transmit_task_delete(void)
{
    if (cli_transmit_task)
    {
        vTaskDelete(cli_transmit_task);
        cli_transmit_task = nullptr;
    }
    if (cli_transmit_task_queue)
    {
        vQueueDelete(cli_transmit_task_queue);
        cli_transmit_task_queue = nullptr;
    }
}
#endif

static void ot_task_worker(void * context)
{
    // Run the main loop
    esp_openthread_launch_mainloop();

    openthread_deinit_netif_glue();

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

    esp_err_t err          = ESP_OK;
    bool eventfdRegistered = false;
    bool otInitialized     = false;
#ifdef CONFIG_OPENTHREAD_CLI
    bool cliTaskStarted = false;
#endif

    err = openthread_init_netif_stack();
    VerifyOrExit(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize OpenThread netif stack: %s", esp_err_to_name(err)));

    err = esp_vfs_eventfd_register(&eventfd_config);
    if (err == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW(TAG, "eventfd is already registered");
    }
    else if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register eventfd: %s", esp_err_to_name(err));
        goto exit;
    }
    else
    {
        eventfdRegistered = true;
    }
    assert(s_platform_config);
    // Initialize the OpenThread stack
    if ((err = esp_openthread_init(s_platform_config)) == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW(TAG, "OpenThread is already initialized");
        return ESP_OK;
    }
    VerifyOrExit(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize OpenThread stack: %s", esp_err_to_name(err)));
    otInitialized = true;
#ifdef CONFIG_OPENTHREAD_CLI
    esp_openthread_matter_cli_init();
    cli_command_transmit_task();
    cliTaskStarted = true;
#endif
    // Initialize the esp_netif bindings
    err = openthread_init_netif_glue(s_platform_config);
    VerifyOrExit(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize OpenThread netif glue: %s", esp_err_to_name(err)));

exit:
    if (err != ESP_OK)
    {
#ifdef CONFIG_OPENTHREAD_CLI
        if (cliTaskStarted)
        {
            cli_command_transmit_task_delete();
        }
#endif
        if (otInitialized)
        {
            esp_openthread_deinit();
        }
        if (eventfdRegistered)
        {
            esp_vfs_eventfd_unregister();
        }
    }
    return err;
}

esp_err_t openthread_launch_task(void)
{
    xTaskCreate(ot_task_worker, "ot_task", CONFIG_THREAD_TASK_STACK_SIZE, xTaskGetCurrentTaskHandle(), 5, &openthread_task);
    return ESP_OK;
}

esp_err_t openthread_deinit_stack(void)
{
    openthread_deinit_netif_glue();
#ifdef CONFIG_OPENTHREAD_CLI
    cli_command_transmit_task_delete();
#endif
    return esp_openthread_deinit();
}

void openthread_delete_task(void)
{
    if (openthread_task)
    {
        vTaskDelete(openthread_task);
    }
}
