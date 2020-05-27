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
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
#include <lwip/netdb.h>

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <inet/UDPEndPoint.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>

#define PORT CONFIG_ECHO_PORT

static const char * TAG = "echo_server";

using namespace ::chip;
using namespace ::chip::Inet;

// UDP Endpoint Callbacks
static void echo(IPEndPointBasis * endpoint, System::PacketBuffer * buffer, const IPPacketInfo * packet_info)
{
    bool status = endpoint != NULL && buffer != NULL && packet_info != NULL;

    if (status)
    {
        char src_addr[INET_ADDRSTRLEN];
        char dest_addr[INET_ADDRSTRLEN];
        const size_t data_len = buffer->DataLength();

        packet_info->SrcAddress.ToString(src_addr, sizeof(src_addr));
        packet_info->DestAddress.ToString(dest_addr, sizeof(dest_addr));

        ESP_LOGI(TAG, "UDP packet received from %s:%u to %s:%u (%zu bytes)", src_addr, packet_info->SrcPort, dest_addr,
                 packet_info->DestPort, static_cast<size_t>(data_len));

        // attempt to print the incoming message
        char msg_buffer[data_len + 1];
        msg_buffer[data_len] = 0; // Null-terminate whatever we received and treat like a string...
        memcpy(msg_buffer, buffer->Start(), data_len);
        ESP_LOGI(TAG, "Client sent: \"%s\"", msg_buffer);

        // Attempt to echo back
        UDPEndPoint * udp_endpoint = static_cast<UDPEndPoint *>(endpoint);
        INET_ERROR err             = udp_endpoint->SendTo(packet_info->SrcAddress, packet_info->SrcPort, buffer);
        if (err != INET_NO_ERROR)
        {
            ESP_LOGE(TAG, "Unable to echo back to client: %s", ErrorStr(err));
            // Note the failure status
            status = !status;
        }
        else
        {
            ESP_LOGI(TAG, "Echo sent");
        }
    }

    if (!status)
    {
        ESP_LOGE(TAG, "Received data but couldn't process it...");

        // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
        if (buffer != NULL)
        {
            System::PacketBuffer::Free(buffer);
        }
    }
}

static void error(IPEndPointBasis * ep, INET_ERROR error, const IPPacketInfo * pi)
{
    ESP_LOGE(TAG, "ERROR: %s\n Got UDP error", ErrorStr(error));
}

// The echo server assumes the platform's networking has been setup already
void startServer(UDPEndPoint * endpoint)
{
    ESP_LOGI(TAG, "Trying to get Inet");
    INET_ERROR err = DeviceLayer::InetLayer.NewUDPEndPoint(&endpoint);
    if (err != INET_NO_ERROR)
    {
        ESP_LOGE(TAG, "ERROR: %s\n Couldn't create UDP Endpoint, server will not start.", ErrorStr(err));
        return;
    }

    endpoint->OnMessageReceived = echo;
    endpoint->OnReceiveError    = error;

    err = endpoint->Bind(kIPAddressType_IPv4, IPAddress::Any, PORT);
    if (err != INET_NO_ERROR)
    {
        ESP_LOGE(TAG, "Socket unable to bind: Error %s", ErrorStr(err));
        return;
    }

    err = endpoint->Listen();
    if (err != INET_NO_ERROR)
    {
        ESP_LOGE(TAG, "Socket unable to Listen: Error %s", ErrorStr(err));
        return;
    }
    ESP_LOGI(TAG, "Echo Server Listening on PORT:%d...", PORT);
}
