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
#include "esp_log.h"
#include <datamodel/ClusterServer.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>

#include "LEDWidget.h"
#include <inet/IPAddress.h>

static const char * TAG = "echo-devicecallbacks";
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DataModel;

extern LEDWidget statusLED; // In wifi-echo.cpp

void EchoDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceEventType::kInternetConnectivityChange)
    {
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
        {
            tcpip_adapter_ip_info_t ipInfo;
            if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo) == ESP_OK)
            {
                char ipAddrStr[INET_ADDRSTRLEN];
                IPAddress::FromIPv4(ipInfo.ip).ToString(ipAddrStr, sizeof(ipAddrStr));
                ESP_LOGI(TAG, "Server ready at: %s:%d", ipAddrStr, CHIP_PORT);
            }
        }
        else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
        {
            ESP_LOGE(TAG, "Lost IPv4 connectivity...");
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

extern ClusterServer gServer;
/* This function can be eliminated, and instead its contents will get executed */
void EchoDeviceCallbacks::PostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId,
                                                      uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                      uint8_t * value)
{
    // At this point we can assume that value points to a boolean value.
    Value cValue(kCHIPValueType_Bool);
    memcpy((void *) &cValue.Int64, (void *) value, size);

    gServer.SetValue(endpoint, clusterId, attributeId, cValue);
}
