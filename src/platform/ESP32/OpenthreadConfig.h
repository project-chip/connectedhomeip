/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
