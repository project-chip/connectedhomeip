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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/Dnssd.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::System;

void CommonDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    ChipLogProgress(Zcl, "DeviceEventCallback, event->Type:%d \r\n", event->Type);
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        ChipLogProgress(Zcl, "IP(%s) changed event",
                        (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ? "IPv4" : "IPv6");
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
    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ChipLogProgress(Zcl, "CHIPoBLE connection established");
        break;
    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ChipLogProgress(Zcl, "CHIPoBLE disconnected");
        break;
    case DeviceEventType::kCommissioningComplete:
        ChipLogProgress(Zcl, "Commissioning complete");
        break;
    }
}

void CommonDeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ChipLogProgress(Zcl, "Server ready at:%d", CHIP_PORT);
        // we start dnssd when ipv6 is ready.
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ChipLogProgress(Zcl, "Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ChipLogProgress(Zcl, "IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ChipLogProgress(Zcl, "Lost IPv6 connectivity...");
    }
}
