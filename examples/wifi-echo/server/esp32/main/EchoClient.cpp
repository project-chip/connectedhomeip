/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, softwarEchoe
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifdef CONFIG_ECHO_USE_CLIENT

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <core/CHIPConfig.h>
#include <lwip/netdb.h>

#define PORT CHIP_PORT
#define RX_LEN 128
#define ADDR_LEN 128

#define HOST_IP_ADDR CONFIG_ECHO_HOST_IP

static const char * TAG     = "echo_client";
static const char * PAYLOAD = "Message from echo client!";

static void udp_client_task(void * pvParameters)
{
    char rx_buffer[RX_LEN];
    char host_ip[]  = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1)
    {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family      = AF_INET;
        dest_addr.sin_port        = htons(PORT);
        addr_family               = AF_INET;
        ip_protocol               = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        while (1)
        {
            int err = sendto(sock, PAYLOAD, strlen(PAYLOAD), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
            if (err < 0)
            {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len           = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *) &source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                continue;
            }
            // Data received
            else
            {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                if (strncmp(rx_buffer, PAYLOAD, strlen(PAYLOAD)) == 0)
                {
                    ESP_LOGI(TAG, "Received expected message...");
                }
            }

            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// The echo client assumes the platform's networking has been setup already
void startClient(void)
{
    xTaskCreate(udp_client_task, "udp_client", 4096, (void *) AF_INET, 5, NULL);
}

#endif // CONFIG_USE_ECHO_CLIENT
