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
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include "CHIPDeviceManager.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "WiFiWidget.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <app/Command.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/server/Mdns.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <lib/mdns/Advertiser.h>
#include <support/CodeUtils.h>

static const char * TAG = "app-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS = 250;

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kSessionEstablished:
        OnSessionEstablished(event);
        break;
    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::Mdns::StartServer();
        }
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %zu\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                                  uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);

    switch (clusterId)
    {
    case ZCL_ON_OFF_CLUSTER_ID:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case ZCL_IDENTIFY_CLUSTER_ID:
        OnIdentifyPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case ZCL_LEVEL_CONTROL_CLUSTER_ID:
        OnLevelControlAttributeChangeCallback(endpointId, attributeId, value);
        break;
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    case ZCL_COLOR_CONTROL_CLUSTER_ID:
        OnColorControlAttributeChangeCallback(endpointId, attributeId, value);
        break;
#endif
    default:
        ESP_LOGI(TAG, "Unhandled cluster ID: %d", clusterId);
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %zu\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ESP_LOGI(TAG, "Server ready at: %s:%d", event->InternetConnectivityChange.address, CHIP_PORT);
        wifiLED.Set(true);
        chip::app::Mdns::StartServer();
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ESP_LOGE(TAG, "Lost IPv4 connectivity...");
        wifiLED.Set(false);
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ESP_LOGI(TAG, "IPv6 Server ready...");
        chip::app::Mdns::StartServer();
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ESP_LOGE(TAG, "Lost IPv6 connectivity...");
    }
}

void DeviceCallbacks::OnSessionEstablished(const ChipDeviceEvent * event)
{
    if (event->SessionEstablished.IsCommissioner)
    {
        ESP_LOGI(TAG, "Commissioner detected!");
    }
}

void DeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;
    endpointId == 1 ? statusLED1.Set(*value) : statusLED2.Set(*value);

exit:
    return;
}

void DeviceCallbacks::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(attributeId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    endpointId == 1 ? statusLED1.SetBrightness(brightness) : statusLED2.SetBrightness(brightness);

exit:
    return;
}

// Currently we only support ColorControl cluster for ESP32C3_DEVKITM which has an on-board RGB-LED
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
void DeviceCallbacks::OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID ||
                     attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                 ESP_LOGI(TAG, "Unhandled AttributeId ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));
    if (endpointId == 1)
    {
        uint8_t hue, saturation;
        if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
        {
            hue = *value;
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                       &saturation, sizeof(uint8_t));
        }
        else
        {
            saturation = *value;
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue,
                                       sizeof(uint8_t));
        }
        statusLED1.SetColor(hue, saturation);
    }
exit:
    return;
}
#endif

void IdentifyTimerHandler(Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    statusLED1.Animate();

    if (identifyTimerCount)
    {
        SystemLayer.StartTimer(kIdentifyTimerDelayMS, IdentifyTimerHandler, appState);
        // Decrement the timer count.
        identifyTimerCount--;
    }
}

void DeviceCallbacks::OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    statusLED1.Blink(kIdentifyTimerDelayMS * 2);

    // timerCount represents the number of callback executions before we stop the timer.
    // value is expressed in seconds and the timer is fired every 250ms, so just multiply value by 4.
    // Also, we want timerCount to be odd number, so the ligth state ends in the same state it starts.
    identifyTimerCount = (*value) * 4;

    SystemLayer.CancelTimer(IdentifyTimerHandler, this);
    SystemLayer.StartTimer(kIdentifyTimerDelayMS, IdentifyTimerHandler, this);

exit:
    return;
}

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::Command * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
