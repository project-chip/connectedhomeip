/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file EchoDeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/

#include "EchoDeviceCallbacks.h"
#include "RendezvousSession.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>

#include "LEDWidget.h"
#include "WiFiWidget.h"
#include <inet/IPAddress.h>

extern "C" {
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
} // extern "C"

static const char * TAG = "echo-devicecallbacks";
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

// In wifi-echo.cpp
extern LEDWidget statusLED1;
extern LEDWidget statusLED2;
extern WiFiWidget wifiLED;
extern RendezvousSession * rendezvousSession;

void EchoDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceEventType::kInternetConnectivityChange)
    {
        ESP_LOGI(TAG, "Current free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
        {
            tcpip_adapter_ip_info_t ipInfo;
            if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo) == ESP_OK)
            {
                char ipAddrStr[INET_ADDRSTRLEN];
                IPAddress::FromIPv4(ipInfo.ip).ToString(ipAddrStr, sizeof(ipAddrStr));
                ESP_LOGI(TAG, "Server ready at: %s:%d", ipAddrStr, CHIP_PORT);

                // Since the commissioner device does not yet have a mechanism to discover the IP address
                // of the peripheral, the following code send it over the current Rendezvous session.
                if (rendezvousSession != NULL)
                {
                    rendezvousSession->Send(ipAddrStr);
                }
            }
            wifiLED.Set(true);
        }
        else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
        {
            ESP_LOGE(TAG, "Lost IPv4 connectivity...");
            wifiLED.Set(false);
        }
        if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
        {
            ESP_LOGI(TAG, "IPv6 Server ready...");
        }
        else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
        {
            ESP_LOGE(TAG, "Lost IPv6 connectivity...");
        }
    }
    if (event->Type == DeviceEventType::kSessionEstablished && event->SessionEstablished.IsCommissioner)
    {
        ESP_LOGI(TAG, "Commissioner detected!");
    }
}

void EchoDeviceCallbacks::PostAttributeChangeCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                                      uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                      uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        ESP_LOGI(TAG, "Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ESP_LOGI(TAG, "Unknown attribute ID: %d", attributeId);
        return;
    }
    ESP_LOGI(TAG, "Got the post attribute callback with value %d for endpoint %d", *value, endpoint);
    // At this point we can assume that value points to a bool value.
    if (endpoint == 1)
    {
        statusLED1.Set(*value);
    }
    else if (endpoint == 2)
    {
        statusLED2.Set(*value);
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected endpoint id: %d", endpoint);
    }

    ESP_LOGI(TAG, "Current free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}
