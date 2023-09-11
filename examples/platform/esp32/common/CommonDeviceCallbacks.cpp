/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
        Esp32AppServer::DeInitBLEIfCommissioned();
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
