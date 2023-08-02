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

#pragma once

#include "esp_openthread_types.h"

#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                                                                      \
    {                                                                                                                              \
        .radio_mode = RADIO_MODE_NATIVE,                                                                                           \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                                                                       \
    {                                                                                                                              \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                                                                         \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                                                                       \
    {                                                                                                                              \
        .storage_partition_name = "ot_storage", .netif_queue_size = 10, .task_queue_size = 10,                                     \
    }
