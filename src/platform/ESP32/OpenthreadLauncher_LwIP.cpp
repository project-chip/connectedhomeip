/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"

static esp_netif_t * sOpenThreadNetif = nullptr;

esp_err_t openthread_init_netif_stack(void)
{
    return esp_netif_init();
}

esp_err_t openthread_init_netif_glue(const esp_openthread_platform_config_t * config)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    sOpenThreadNetif       = esp_netif_new(&cfg);
    if (sOpenThreadNetif == nullptr)
    {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_netif_attach(sOpenThreadNetif, esp_openthread_netif_glue_init(config));
    if (err != ESP_OK)
    {
        esp_openthread_netif_glue_deinit();
        esp_netif_destroy(sOpenThreadNetif);
        sOpenThreadNetif = nullptr;
    }
    return err;
}

void openthread_deinit_netif_glue(void)
{
    esp_openthread_netif_glue_deinit();
    if (sOpenThreadNetif != nullptr)
    {
        esp_netif_destroy(sOpenThreadNetif);
        sOpenThreadNetif = nullptr;
    }
}
