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

#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "pw_rpc/server.h"
#include "wifi_service.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

namespace {

const char * TAG = "ipv6only";

constexpr size_t kUdpBufferSize = 512;

constexpr size_t kRpcStackSizeBytes  = (6 * 1024);
constexpr uint8_t kRpcTaskPriority   = 5;
constexpr size_t kTestStackSizeBytes = (8 * 1024);
constexpr uint8_t kTestTaskPriority  = 5;

TaskHandle_t rpcTaskHandle;
TaskHandle_t testTaskHandle;

void UdpReceiver(void * pvParameters)
{
    int portno;                     // port to listen on
    struct sockaddr_in6 serveraddr; // server's addr
    char buf[kUdpBufferSize];       // rx message buf
    char * hostaddrp;               // dotted decimal host addr string
    int optval;                     // flag value for setsockopt
    int n;                          // message byte size
    int sockfd = 0;
    unsigned int clientlen;         // byte size of client's address
    struct sockaddr_in6 clientaddr; // client addr

    while (1)
    {
        // Start the udp server after the wifi is connectd.
        chip::rpc::Wifi::Instance().BlockUntilWifiConnected();
        ESP_LOGI(TAG, "UDP server starting");

        portno = 8765;
        // socket: create the parent socket
        sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            ESP_LOGE(TAG, "ERROR opening socket");
            assert(0);
            return;
        }

        // setsockopt: Handy debugging trick that lets
        // us rerun the server immediately after we kill it;
        // otherwise we have to wait about 20 secs.
        // Eliminates "ERROR on binding: Address already in use" error.
        optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, static_cast<const void *>(&optval), sizeof(int));

        // build the server's Internet address
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin6_len    = sizeof(serveraddr);
        serveraddr.sin6_family = AF_INET6;
        serveraddr.sin6_addr   = in6addr_any;
        serveraddr.sin6_port   = htons((unsigned short) portno);

        //  bind: associate the parent socket with a port
        if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&serveraddr), sizeof(serveraddr)) < 0)
        {
            ESP_LOGE(TAG, "ERROR on binding");
            assert(0);
            vTaskDelete(NULL);
        }

        ESP_LOGI(TAG, "UDP server bound to port %d", portno);

        // main loop: wait for a datagram, then respond
        clientlen = sizeof(clientaddr);
        wifi_ap_record_t ap_info;
        fd_set readset;
        while (ESP_OK == esp_wifi_sta_get_ap_info(&ap_info))
        {
            // recvfrom: receive a UDP datagram from a client
            memset(buf, 0, sizeof(buf));

            FD_ZERO(&readset);
            FD_SET(sockfd, &readset);

            int select_err = select(sockfd + 1, &readset, nullptr, nullptr, nullptr);
            if (select_err < 0)
                continue;

            n = recvfrom(sockfd, buf, kUdpBufferSize, 0, reinterpret_cast<struct sockaddr *>(&clientaddr), &clientlen);
            if (n < 0)
                continue;
            // Echo back
            n = sendto(sockfd, buf, n, 0, reinterpret_cast<struct sockaddr *>(&clientaddr), clientlen);
        }
    }
    // Never returns
}

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(chip::rpc::Wifi::Instance());
}

void RunRpcService(void *)
{
    ::chip::rpc::Start(RegisterServices, &::chip::rpc::logger_mutex);
}

} // namespace

extern "C" void app_main()
{
    PigweedLogger::init();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Wifi Init: %s", pw_StatusString(chip::rpc::Wifi::Instance().Init()));
    ESP_LOGI(TAG, "----------- chip-esp32-ipv6-example starting -----------");

    xTaskCreate(RunRpcService, "RPC", kRpcStackSizeBytes / sizeof(StackType_t), nullptr, kRpcTaskPriority, &rpcTaskHandle);
    xTaskCreate(UdpReceiver, "TestTask", kTestStackSizeBytes / sizeof(StackType_t), nullptr, kTestTaskPriority, &testTaskHandle);
}
