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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/server/Dnssd.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <lib/dnssd/Advertiser.h>
#include <platform/Ameba/AmebaUtils.h>
#include <route_hook/ameba_route_hook.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/logging/Constants.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <ota/OTAInitializer.h>
#endif

#include "Globals.h"
#include "LEDWidget.h"

static const char TAG[] = "app-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::Logging;

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS     = 250;
constexpr uint32_t kInitOTARequestorDelaySec = 3;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    OTAInitializer::Instance().InitOTARequestor();
}
#endif

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ChipLogProgress(DeviceLayer, "CHIPoBLE Connection Established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ChipLogProgress(DeviceLayer, "CHIPoBLE Connection Closed");
        break;

    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising has changed");
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
            ChipLogProgress(DeviceLayer, "Initializing route hook...");
            ameba_route_hook_init();
        }
        break;

    case DeviceEventType::kCommissioningComplete:
        ChipLogProgress(DeviceLayer, "Commissioning Complete");
        chip::DeviceLayer::Internal::AmebaUtils::SetCurrentProvisionedNetwork();
        break;
    }
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                                  uint16_t size, uint8_t * value)
{
    switch (clusterId)
    {
    case app::Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case app::Clusters::Identify::Id:
        OnIdentifyPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    default:
        break;
    }
}

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ChipLogProgress(DeviceLayer, "IPv4 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ChipLogProgress(DeviceLayer, "IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        // Init OTA requestor only when we have gotten IPv6 address
        if (!OTAInitializer::Instance().CheckInit())
        {
            ChipLogProgress(DeviceLayer, "Initializing OTA");
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
        }
#endif
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv6 connectivity...");
    }
}

void DeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == app::Clusters::OnOff::Attributes::OnOff::Id,
                 ChipLogError(DeviceLayer, TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2,
                 ChipLogError(DeviceLayer, TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    statusLED1.Set(*value);

exit:
    return;
}

void IdentifyTimerHandler(Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    if (identifyTimerCount)
    {
        identifyTimerCount--;
    }
}

void DeviceCallbacks::OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == app::Clusters::Identify::Attributes::IdentifyTime::Id,
                 ChipLogError(DeviceLayer, "[%s] Unhandled Attribute ID: '0x%04x", TAG, attributeId));
    VerifyOrExit(endpointId == 1, ChipLogError(DeviceLayer, "[%s] Unexpected EndPoint ID: `0x%02x'", TAG, endpointId));

    // timerCount represents the number of callback executions before we stop the timer.
    // value is expressed in seconds and the timer is fired every 250ms, so just multiply value by 4.
    // Also, we want timerCount to be odd number, so the ligth state ends in the same state it starts.
    identifyTimerCount = (*value) * 4;
exit:
    return;
}
