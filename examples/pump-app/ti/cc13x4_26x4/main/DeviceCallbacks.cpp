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
#include "AppConfig.h"
#include "PumpManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Dnssd.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kThreadConnectivityChange:
        OnThreadConnectivityChange(event);
        break;

    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kServiceProvisioningChange:
        PLAT_LOG("## Service provisioning state change (%d,%d)", event->ServiceProvisioningChange.IsServiceProvisioned,
                 event->ServiceProvisioningChange.ServiceConfigUpdated);
        break;

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        PLAT_LOG("CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        PLAT_LOG("CHIPoBLE disconnected");
        break;

    case DeviceEventType::kThreadStateChange:
        PLAT_LOG("## Thread stack state change (%x)", event->ThreadStateChange.OpenThread.Flags);
        break;

    case DeviceEventType::kCommissioningComplete:
        PLAT_LOG("Commissioning complete for fabric 0x%x", event->CommissioningComplete.fabricIndex);
        break;

    case DeviceEventType::kOperationalNetworkEnabled:
        PLAT_LOG("## Operational network enabled");
        break;

    case DeviceEventType::kDnssdInitialized:
        PLAT_LOG("## Dnssd platform initialized");
        break;

    case DeviceEventType::kFailSafeTimerExpired:
        PLAT_LOG("## Failsafe timer expired...");
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        PLAT_LOG("*** Interface IP address changed ***");
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
}

chip::Protocols::InteractionModel::Status DeviceCallbacks::PreAttributeChangeCallback(chip::EndpointId endpointId,
                                                                                      chip::ClusterId clusterId,
                                                                                      chip::AttributeId attributeId, uint8_t type,
                                                                                      uint16_t size, uint8_t * value)
{
    PLAT_LOG("PreAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);

    switch (clusterId)
    {
    case PumpConfigurationAndControl::Id:
        break;

    case OnOff::Id:
        break;

    case LevelControl::Id:
        break;

    default:
        PLAT_LOG("Unhandled cluster ID: %d", clusterId);
        break;
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                                  uint16_t size, uint8_t * value)
{
    PLAT_LOG("PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);

    switch (clusterId)
    {
    case PumpConfigurationAndControl::Id:
        break;

    case OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case LevelControl::Id:
        OnLevelControlAttributeChangeCallback(endpointId, attributeId, value);
        break;

    default:
        PLAT_LOG("Unhandled cluster ID: %d", clusterId);
        break;
    }
}

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        PLAT_LOG("Server ready at: %s:%d", event->InternetConnectivityChange.ipAddress, CHIP_PORT);
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        PLAT_LOG("Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        PLAT_LOG("IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        PLAT_LOG("Lost IPv6 connectivity...");
    }
}

void DeviceCallbacks::OnThreadConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->ThreadConnectivityChange.Result == kConnectivity_Established)
    {
        PLAT_LOG("## Thread connectivity established...");
    }
    else if (event->ThreadConnectivityChange.Result == kConnectivity_Lost)
    {
        PLAT_LOG("## Thread connectivity lost...");
    }
    else if (event->ThreadConnectivityChange.Result == kConnectivity_NoChange)
    {
        PLAT_LOG("## No change in Thread connectivity...");
    }
}

void DeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == OnOff::Attributes::OnOff::Id, PLAT_LOG("Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1, PLAT_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    PumpMgr().InitiateAction(0, *value ? PumpManager::START_ACTION : PumpManager::STOP_ACTION);

exit:
    return;
}

void DeviceCallbacks::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == LevelControl::Attributes::CurrentLevel::Id,
                 PLAT_LOG("Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1, PLAT_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    ChipLogProgress(Zcl, "[pump-app] Cluster LevelControl: attribute CurrentLevel set to %u", *value);

exit:
    return;
}
