/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
 * @file CommonDeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "CommonDeviceCallbacks.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Dnssd.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>

#include <lib/support/CodeUtils.h>
#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
#include "openthread-system.h"
#include "ot_platform_common.h"
#endif /* CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED */

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

using namespace chip::app;
using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;

void chip::NXP::App::CommonDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    ChipLogDetail(DeviceLayer, "DeviceEventCallback: 0x%04x", event->Type);
    switch (event->Type)
    {
    case DeviceEventType::kWiFiConnectivityChange:
        OnWiFiConnectivityChange(event);
        break;

    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
#if !CHIP_ENABLE_OPENTHREAD // No need to do this for OT mDNS server
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
#endif
        OnInterfaceIpAddressChanged(event);
        break;

#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    case DeviceEventType::kCommissioningComplete:
        CommonDeviceCallbacks::OnComissioningComplete(event);
        break;
#endif
    case DeviceLayer::DeviceEventType::kDnssdInitialized:
        ChipLogProgress(DeviceLayer, "kDnssdInitialized");
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        /* Initialize OTA Requestor */
        OTARequestorInitiator::Instance().InitOTA(reinterpret_cast<intptr_t>(&OTARequestorInitiator::Instance()));
#endif
        break;
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnWiFiConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->WiFiConnectivityChange.Result == kConnectivity_Established)
    {
        ChipLogProgress(DeviceLayer, "WiFi connection established");
    }
    else if (event->WiFiConnectivityChange.Result == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "WiFi connection lost");
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        char ip_addr[Inet::IPAddress::kMaxStringLength];
        event->InternetConnectivityChange.ipAddress.ToString(ip_addr);
        ChipLogProgress(DeviceLayer, "Server ready at: %s:%d", ip_addr, CHIP_PORT);
#if !CHIP_ENABLE_OPENTHREAD // No need to do this for OT mDNS server
        chip::app::DnssdServer::Instance().StartServer();
#endif
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        char ip_addr[Inet::IPAddress::kMaxStringLength];
        event->InternetConnectivityChange.ipAddress.ToString(ip_addr);
        ChipLogProgress(DeviceLayer, "IPv6 Server ready at: [%s]:%d", ip_addr, CHIP_PORT);
#if !CHIP_ENABLE_OPENTHREAD // No need to do this for OT mDNS server
        chip::app::DnssdServer::Instance().StartServer();
#endif
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv6 connectivity...");
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnInterfaceIpAddressChanged(const ChipDeviceEvent * event)
{
    switch (event->InterfaceIpAddressChanged.Type)
    {
    case InterfaceIpChangeType::kIpV4_Assigned:
        ChipLogProgress(DeviceLayer, "Interface IPv4 address assigned");
        break;
    case InterfaceIpChangeType::kIpV4_Lost:
        ChipLogProgress(DeviceLayer, "Interface IPv4 address lost");
        break;
    case InterfaceIpChangeType::kIpV6_Assigned:
        ChipLogProgress(DeviceLayer, "Interface IPv6 address assigned");
        break;
    case InterfaceIpChangeType::kIpV6_Lost:
        ChipLogProgress(DeviceLayer, "Interface IPv6 address lost");
        break;
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnSessionEstablished(chip::DeviceLayer::ChipDeviceEvent const *)
{
    /* Empty */
}

#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
void chip::NXP::App::CommonDeviceCallbacks::OnComissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event)
{
    /*
     * If a transceiver supporting a multiprotocol scenario is used, a check of the provisioning state is required,
     * so that we can inform the transceiver to stop BLE to give the priority to another protocol.
     * For example it is the case when a K32W0 transceiver supporting OT+BLE+Zigbee is used. When the device is already provisioned,
     * BLE is no more required and the transceiver needs to be informed so that Zigbee can be switched on and BLE switched off.
     *
     * If a transceiver does not support such vendor property the cmd would be ignored.
     */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogDetail(DeviceLayer, "Provisioning complete, stopping BLE\n");
        ThreadStackMgrImpl().LockThreadStack();
        PlatformMgrImpl().StopBLEConnectivity();
        ThreadStackMgrImpl().UnlockThreadStack();
    }
}
#endif
