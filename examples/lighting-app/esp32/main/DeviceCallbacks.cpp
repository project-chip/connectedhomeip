/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "LEDWidget.h"

#include "esp_bt.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "route_hook/esp_route_hook.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/Dnssd.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <ota/OTAHelper.h>

static const char * TAG                      = "light-app-callbacks";
constexpr uint32_t kInitOTARequestorDelaySec = 3;

extern LEDWidget AppLED;

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

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

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ESP_LOGI(TAG, "CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ESP_LOGI(TAG, "CHIPoBLE disconnected");
        break;

    case DeviceEventType::kCommissioningComplete: {
        ESP_LOGI(TAG, "Commissioning complete");
#if CONFIG_BT_NIMBLE_ENABLED && CONFIG_DEINIT_BLE_ON_COMMISSIONING_COMPLETE

        if (ble_hs_is_enabled())
        {
            int ret = nimble_port_stop();
            if (ret == 0)
            {
                nimble_port_deinit();
                esp_err_t err = esp_nimble_hci_and_controller_deinit();
                err += esp_bt_mem_release(ESP_BT_MODE_BLE);
                if (err == ESP_OK)
                {
                    ESP_LOGI(TAG, "BLE deinit successful and memory reclaimed");
                }
            }
            else
            {
                ESP_LOGW(TAG, "nimble_port_stop() failed");
            }
        }
        else
        {
            ESP_LOGI(TAG, "BLE already deinited");
        }
#endif
    }
    break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
        if (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned)
        {
            ESP_ERROR_CHECK(esp_route_hook_init(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF")));
        }
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                                  uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);

    switch (clusterId)
    {
    case OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case LevelControl::Id:
        OnLevelControlAttributeChangeCallback(endpointId, attributeId, value);
        break;

#if CONFIG_LED_TYPE_RMT
    case ColorControl::Id:
        OnColorControlAttributeChangeCallback(endpointId, attributeId, value);
        break;
#endif

    default:
        ESP_LOGI(TAG, "Unhandled cluster ID: %d", clusterId);
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    OTAHelpers::Instance().InitOTARequestor();
}

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    static bool isOTAInitialized = false;
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ESP_LOGI(TAG, "Server ready at: %s:%d", event->InternetConnectivityChange.address, CHIP_PORT);
        chip::app::DnssdServer::Instance().StartServer();

        if (!isOTAInitialized)
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
            isOTAInitialized = true;
        }
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ESP_LOGE(TAG, "Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ESP_LOGI(TAG, "IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();

        if (!isOTAInitialized)
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
            isOTAInitialized = true;
        }
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
    VerifyOrExit(attributeId == OnOff::Attributes::OnOff::Id, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    AppLED.Set(*value);

exit:
    return;
}

void DeviceCallbacks::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == LevelControl::Attributes::CurrentLevel::Id,
                 ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    AppLED.SetBrightness(*value);

exit:
    return;
}

// Currently ColorControl cluster is supported for ESP32C3_DEVKITM and ESP32S3_DEVKITM which have an on-board RGB-LED
#if CONFIG_LED_TYPE_RMT
void DeviceCallbacks::OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    uint8_t hue, saturation;

    VerifyOrExit(attributeId == ColorControl::Attributes::CurrentHue::Id ||
                     attributeId == ColorControl::Attributes::CurrentSaturation::Id,
                 ESP_LOGI(TAG, "Unhandled AttributeId ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    if (attributeId == ColorControl::Attributes::CurrentHue::Id)
    {
        hue = *value;
        emberAfReadServerAttribute(endpointId, ColorControl::Id, ColorControl::Attributes::CurrentSaturation::Id, &saturation,
                                   sizeof(uint8_t));
    }
    else
    {
        saturation = *value;
        emberAfReadServerAttribute(endpointId, ColorControl::Id, ColorControl::Attributes::CurrentHue::Id, &hue, sizeof(uint8_t));
    }
    AppLED.SetColor(hue, saturation);

exit:
    return;
}
#endif // CONFIG_LED_TYPE_RMT
