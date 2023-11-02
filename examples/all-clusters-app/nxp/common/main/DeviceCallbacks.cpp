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
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <laundry-washer-mode.h>

#include <lib/support/CodeUtils.h>
#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
#include "openthread-system.h"
#include "ot_platform_common.h"
#endif /* CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED */

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

using namespace chip::app;
void OnTriggerEffect(::Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

Identify gIdentify0 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    chip::app::Clusters::Identify::IdentifyTypeEnum::kNone,
    OnTriggerEffect,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    chip::app::Clusters::Identify::IdentifyTypeEnum::kNone,
    OnTriggerEffect,
};

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
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
        OnInterfaceIpAddressChanged(event);
        break;

#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    case DeviceEventType::kCommissioningComplete:
        DeviceCallbacks::OnComissioningComplete(event);
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

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                                  uint16_t size, uint8_t * value)
{
    ChipLogProgress(DeviceLayer,
                    "endpointId " ChipLogFormatMEI " clusterId " ChipLogFormatMEI " attribute ID: " ChipLogFormatMEI
                    " Type: %u Value: %u, length %u",
                    ChipLogValueMEI(endpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId), type, *value, size);
    switch (clusterId)
    {
    case Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;
    }
}

void DeviceCallbacks::OnWiFiConnectivityChange(const ChipDeviceEvent * event)
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

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        char ip_addr[Inet::IPAddress::kMaxStringLength];
        event->InternetConnectivityChange.ipAddress.ToString(ip_addr);
        ChipLogProgress(DeviceLayer, "Server ready at: %s:%d", ip_addr, CHIP_PORT);
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
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv6 connectivity...");
    }
}

void DeviceCallbacks::OnInterfaceIpAddressChanged(const ChipDeviceEvent * event)
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

using namespace chip::app::Clusters;
void DeviceCallbacks::OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId,
                                                         uint8_t * value)
{
#if 0
    /*
     * This implementation is try to implementation the handler for TC-LWM-3.1
     * Since this case is removed in SVE and there is a side-effect in TC-OO-2.4, remove it temporally
    */
    switch (attributeId) {
    case Clusters::OnOff::Attributes::OnOff::Id:
        if (*value == true) {
            // Update the current mode to OnMode after device is on
	    ModeBase::Instance * modeInstance = LaundryWasherMode::Instance();
	    DataModel::Nullable<uint8_t> mode = modeInstance->GetOnMode();
	    if (mode.IsNull() == false)
            {
	        modeInstance->UpdateCurrentMode(mode.Value());
	    }
        }
        break;
    }
#endif //
}

#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
void DeviceCallbacks::OnComissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event)
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
