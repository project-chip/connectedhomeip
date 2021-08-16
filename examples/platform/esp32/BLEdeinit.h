/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#ifndef BLE_DEINIT
#define BLE_DEINIT

#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"

static const char * BLE_TAG = "BLE Deinit";

void deinitBLE()
{
#if CONFIG_BT_NIMBLE_ENABLED
    int ret = nimble_port_stop();
    if (ret == 0)
    {
        nimble_port_deinit();
        ret = esp_nimble_hci_and_controller_deinit();
        if (ret == ESP_OK)
        {
            ESP_LOGI(BLE_TAG, "Bluetooth deinitialized");
        }
        else
        {
            ESP_LOGE(BLE_TAG, "esp_nimble_hci_and_controller_deinit() failed with error: %d", ret);
        }
    }
#endif
}
#endif
