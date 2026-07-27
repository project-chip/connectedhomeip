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
#include "Esp32AppServer.h"

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

static const char TAG[] = "app-devicecallbacks";

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::System;

DeviceCallbacksDelegate * appDelegate = nullptr;

void CommonDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE is deinitialized");
        break;

    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ESP_LOGI(TAG, "CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ESP_LOGI(TAG, "CHIPoBLE disconnected");
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
        Esp32AppServer::DeInitBLEIfCommissioned();
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
            appDelegate = DeviceCallbacksDelegate::Instance().GetAppDelegate();
            if (appDelegate != nullptr)
            {
                appDelegate->OnIPv6ConnectivityEstablished();
            }
        }
        break;
    }

    ESP_LOGI(TAG, "Current free heap: Internal: %u/%u External: %u/%u",
             static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL)),
             static_cast<unsigned int>(heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL)),
             static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM)),
             static_cast<unsigned int>(heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM)));
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
