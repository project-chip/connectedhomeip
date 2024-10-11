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
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/logging/Constants.h>

#include "LEDWidget.h"
#include "init_OTARequestor.h"
#if defined CONFIG_LWIP_HOOK_IP6_ROUTE_DEFAULT || defined CONFIG_LWIP_HOOK_ND6_GET_GW_DEFAULT
#include "route_hook/asr_route_hook.h"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::Logging;

extern LEDWidget sLightLED;

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS = 250;
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
constexpr uint32_t kInitOTARequestorDelaySec = 3;

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

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();

            if (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned)
            {
#if defined CONFIG_LWIP_HOOK_IP6_ROUTE_DEFAULT || defined CONFIG_LWIP_HOOK_ND6_GET_GW_DEFAULT
                ChipLogProgress(NotSpecified, "Initializing route hook...");
                asr_route_hook_init();
#endif
            }
        }
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
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static bool isOTAInitialized = false;
#endif
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
        if (!isOTAInitialized)
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
            isOTAInitialized = true;
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
    VerifyOrExit(attributeId == chip::app::Clusters::OnOff::Attributes::OnOff::Id,
                 ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04lx", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    switch (attributeId)
    {
    case chip::app::Clusters::OnOff::Attributes::OnOff::Id: {
        ChipLogProgress(Zcl, "ON/OFF level: %u ", *value);
        // At this point we can assume that value points to a bool value.
        sLightLED.Set(*value);
    }
    break;

    case chip::app::Clusters::OnOff::Attributes::OnTime::Id:
    case chip::app::Clusters::OnOff::Attributes::OffWaitTime::Id:
    case chip::app::Clusters::OnOff::Attributes::StartUpOnOff::Id:
    case chip::app::Clusters::OnOff::Attributes::GlobalSceneControl::Id: {
        ChipLogProgress(Zcl, "Unprocessed attribute ID: %" PRIx32, attributeId);
    }
    break;

    default:
        ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
        return;
    } // switch (attributeId)
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
    VerifyOrExit(attributeId == chip::app::Clusters::Identify::Attributes::IdentifyTime::Id,
                 ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04lx", attributeId));
    VerifyOrExit(endpointId == 1, ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // timerCount represents the number of callback executions before we stop the timer.
    // value is expressed in seconds and the timer is fired every 250ms, so just multiply value by 4.
    // Also, we want timerCount to be odd number, so the ligth state ends in the same state it starts.
    identifyTimerCount = (*value) * 4;
exit:
    return;
}
