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
#include "AppConfig.h"
#include "CHIPDeviceManager.h"
#include "init_OTARequestor.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <lib/dnssd/Advertiser.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/logging/Constants.h>
#if defined CONFIG_LWIP_HOOK_IP6_ROUTE_DEFAULT || defined CONFIG_LWIP_HOOK_ND6_GET_GW_DEFAULT
#include "route_hook/asr_route_hook.h"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::Logging;
using namespace ::chip::app::Clusters;

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

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static bool isOTAInitialized = false;
#endif
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ASR_LOG("IPv4 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ASR_LOG("Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ASR_LOG("IPv6 Server ready...");
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
        ASR_LOG("Lost IPv6 connectivity...");
    }
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                                  uint16_t size, uint8_t * value)
{
    ASR_LOG("PostAttributeChangeCallback - Cluster ID: '0x%" PRIx32 "', EndPoint ID: '0x%x', Attribute ID: '0x%" PRIx32 "'",
            clusterId, endpointId, attributeId);
}
