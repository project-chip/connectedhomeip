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
#include <platform/CHIPDeviceLayer.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureTransport.h>

#include "DataModelHandler.h"

static const char * TAG = "echo_server";

using namespace ::chip;
using namespace ::chip::Inet;

// Transport Callbacks
static void echo(SecureTransport * transport, System::PacketBuffer * buffer, const IPPacketInfo * packet_info)
{
    bool status = transport != NULL && buffer != NULL && packet_info != NULL;

    if (status)
    {
        char src_addr[INET_ADDRSTRLEN];
        char dest_addr[INET_ADDRSTRLEN];
        const size_t data_len = buffer->DataLength();

        packet_info->SrcAddress.ToString(src_addr, sizeof(src_addr));
        packet_info->DestAddress.ToString(dest_addr, sizeof(dest_addr));

        ESP_LOGI(TAG, "UDP packet received from %s:%u to %s:%u (%zu bytes)", src_addr, packet_info->SrcPort, dest_addr,
                 packet_info->DestPort, static_cast<size_t>(data_len));

        if (data_len > 0 && buffer->Start()[0] < 0x20)
        {
            // Non-ACII; assume it's a data model message.
            HandleDataModelMessage(buffer);
            return;
        }

        ESP_LOGI(TAG, "Client sent: \"%.*s\"", data_len, buffer->Start());

        // Attempt to echo back
        CHIP_ERROR err = transport->SendMessage(buffer, packet_info->SrcAddress);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Unable to echo back to client: %s", ErrorStr(err));
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

static void error(SecureTransport * st, CHIP_ERROR error, const IPPacketInfo * pi)
{
    ESP_LOGE(TAG, "ERROR: %s\n Got UDP error", ErrorStr(error));
}

static const unsigned char local_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                                   0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                                   0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

static const unsigned char remote_public_key[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd, 0xfb, 0x1f,
                                                   0xcc, 0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3, 0xab, 0xe4, 0x33, 0xb6,
                                                   0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34, 0x92, 0x73, 0x14, 0x59, 0x0b, 0xbd,
                                                   0x44, 0x72, 0x1b, 0xcd, 0xb9, 0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae,
                                                   0xe8, 0x87, 0x2e, 0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

// The echo server assumes the platform's networking has been setup already
void setupTransport(IPAddressType type, SecureTransport * transport)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    transport->Init(&DeviceLayer::InetLayer);
    if (type == kIPAddressType_IPv6)
    {
        struct netif * netif;
        tcpip_adapter_get_netif(TCPIP_ADAPTER_IF_AP, (void **) &netif);
        err = transport->Connect(type, netif);
    }
    else
    {
        err = transport->Connect(type);
    }
 
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "ERROR: %s\n Couldn't create transport, server will not start.", ErrorStr(err));
        return;
    }

    transport->OnMessageReceived = echo;
    transport->OnReceiveError    = error;

    err = transport->ManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key, sizeof(local_private_key));
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "ERROR: %s\n Couldn't establish security keys.", ErrorStr(err));
        return;
    }

    ESP_LOGI(TAG, "Echo Server Listening...");
}

// The echo server assumes the platform's networking has been setup already
void startServer(SecureTransport * transport_ipv4, SecureTransport * transport_ipv6)
{
    setupTransport(kIPAddressType_IPv4, transport_ipv4);
    setupTransport(kIPAddressType_IPv6, transport_ipv6);
}
