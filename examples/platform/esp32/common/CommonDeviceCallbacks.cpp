/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "CommonDeviceCallbacks.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CONFIG_BT_ENABLED
#include "esp_bt.h"
#if CONFIG_BT_NIMBLE_ENABLED
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_nimble_hci.h"
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include "nimble/nimble_port.h"
#endif // CONFIG_BT_NIMBLE_ENABLED
#endif // CONFIG_BT_ENABLED
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#if CONFIG_ENABLE_OTA_REQUESTOR
#include <ota/OTAHelper.h>
#endif

static const char * TAG = "app-devicecallbacks";

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::System;

DeviceCallbacksDelegate * appDelegate = nullptr;

void CommonDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ESP_LOGI(TAG, "CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ESP_LOGI(TAG, "CHIPoBLE disconnected");
#if CONFIG_BT_ENABLED
#if CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING

        if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0)
        {
            esp_err_t err = ESP_OK;
#if CONFIG_BT_NIMBLE_ENABLED
            if (ble_hs_is_enabled())
            {
                int ret = nimble_port_stop();
                if (ret == 0)
                {
                    nimble_port_deinit();
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
                    err = esp_nimble_hci_and_controller_deinit();
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#endif // CONFIG_BT_NIMBLE_ENABLED

#if CONFIG_IDF_TARGET_ESP32
                    err += esp_bt_mem_release(ESP_BT_MODE_BTDM);
#elif CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32H2
            err += esp_bt_mem_release(ESP_BT_MODE_BLE);
#endif
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
            else { ESP_LOGI(TAG, "BLE already deinited"); }
        }
#endif // CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING
#endif // CONFIG_BT_ENABLED
        break;

    case DeviceEventType::kDnssdInitialized:
#if CONFIG_ENABLE_OTA_REQUESTOR
        OTAHelpers::Instance().InitOTARequestor();
#endif
        appDelegate = DeviceCallbacksDelegate::Instance().GetAppDelegate();
        if (appDelegate != nullptr)
        {
            appDelegate->OnDnssdInitialized();
        }
        break;

    case DeviceEventType::kCommissioningComplete: {
        ESP_LOGI(TAG, "Commissioning complete");
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
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void CommonDeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    appDelegate = DeviceCallbacksDelegate::Instance().GetAppDelegate();
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ESP_LOGI(TAG, "IPv4 Server ready...");
        if (appDelegate != nullptr)
        {
            appDelegate->OnIPv4ConnectivityEstablished();
        }
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ESP_LOGE(TAG, "Lost IPv4 connectivity...");
        if (appDelegate != nullptr)
        {
            appDelegate->OnIPv4ConnectivityLost();
        }
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ESP_LOGI(TAG, "IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ESP_LOGE(TAG, "Lost IPv6 connectivity...");
    }
}
