/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <sdkconfig.h>

#if CONFIG_OPENTHREAD_ENABLED
#include "esp_openthread_types.h"

#if CONFIG_OPENTHREAD_RADIO_NATIVE
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                                                                      \
    {                                                                                                                              \
        .radio_mode = RADIO_MODE_NATIVE,                                                                                           \
    }
#elif CONFIG_OPENTHREAD_RADIO_SPINEL_UART
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                                                                      \
    {                                                                                                                              \
        .radio_mode = RADIO_MODE_UART_RCP,                 \
        .radio_uart_config = {                             \
            .port = UART_NUM_1,                            \
            .uart_config =                                 \
                {                                          \
                    .baud_rate = 460800,                   \
                    .data_bits = UART_DATA_8_BITS,         \
                    .parity = UART_PARITY_DISABLE,         \
                    .stop_bits = UART_STOP_BITS_1,         \
                    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, \
                    .rx_flow_ctrl_thresh = 0,              \
                    .source_clk = UART_SCLK_DEFAULT,       \
                },                                         \
            .rx_pin = GPIO_NUM_17,                         \
            .tx_pin = GPIO_NUM_18,                         \
        },                                                                       \
    }
#else
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                                                                      \
    {                                                                                                                              \
        .radio_mode = RADIO_MODE_SPI_RCP,          \
        .radio_spi_config = {                      \
            .host_device = SPI2_HOST,              \
            .dma_channel = 2,                      \
            .spi_interface =                       \
                {                                  \
                    .mosi_io_num = 11,             \
                    .sclk_io_num = 12,             \
                    .miso_io_num = 13,             \
                },                                 \
            .spi_device =                          \
                {                                  \
                    .cs_ena_pretrans = 2,          \
                    .input_delay_ns = 100,         \
                    .mode = 0,                     \
                    .clock_speed_hz = 2500 * 1000, \
                    .spics_io_num = 10,            \
                    .queue_size = 5,               \
                },                                 \
            .intr_pin = 8,                         \
        },                                                                               \
    }
#endif // CONFIG_OPENTHREAD_RADIO_SPINEL_UART OR  CONFIG_OPENTHREAD_RADIO_SPINEL_SPI

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                                                                       \
    {                                                                                                                              \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                                                                         \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                                                                       \
    {                                                                                                                              \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10,                                            \
    }

#if defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && defined(CONFIG_AUTO_UPDATE_RCP)
#include <esp_rcp_update.h>
#define RCP_FIRMWARE_DIR "/spiffs/ot_rcp"

#define ESP_OPENTHREAD_RCP_UPDATE_CONFIG()                                                                                         \
    {                                                                                                                              \
        .rcp_type = RCP_TYPE_ESP32H2_UART, .uart_rx_pin = 17, .uart_tx_pin = 18, .uart_port = 1, .uart_baudrate = 115200,          \
        .reset_pin = 7, .boot_pin = 8, .update_baudrate = 460800, .firmware_dir = "/rcp_fw/ot_rcp", .target_chip = ESP32H2_CHIP,   \
    }
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER && CONFIG_AUTO_UPDATE_RCP

#endif // CONFIG_OPENTHREAD_ENABLED

void ESPOpenThreadInit();
