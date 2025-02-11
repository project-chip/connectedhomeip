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
#include "AppTask.h"
#include "Globals.h"

#include "BindingHandler.h"
#include "CHIPDeviceManager.h"
#include "app/clusters/bindings/BindingManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <app/server/Dnssd.h>
#include <assert.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <ota/OTAInitializer.h>
#endif

static const char * TAG = "app-devicecallbacks";

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DoorLock;
using ::chip::app::DataModel::Nullable;

using namespace ::chip::app;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS     = 250;
constexpr uint32_t kInitOTARequestorDelaySec = 3;

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;

namespace LedConsts {
constexpr uint32_t kBlinkRate_ms{ 500 };
constexpr uint32_t kIdentifyBlinkRate_ms{ 500 };

namespace StatusLed {
namespace Unprovisioned {
constexpr uint32_t kOn_ms{ 100 };
constexpr uint32_t kOff_ms{ kOn_ms };
} /* namespace Unprovisioned */
namespace Provisioned {
constexpr uint32_t kOn_ms{ 50 };
constexpr uint32_t kOff_ms{ 950 };
} /* namespace Provisioned */

} /* namespace StatusLed */
} /* namespace LedConsts */

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static bool isOTAInitialized = false;

void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(Zcl, "InitOTARequestorHandler");
    OTAInitializer::Instance().InitOTARequestor();
}
#endif

void DeviceCallbacks::UpdateStatusLED()
{
    // Update the status LED.
    //
    // If IPv6 network and service provisioned, keep the LED Off constantly.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
    // rate of 100ms.
    //
    // Otherwise, blink the LED for a very short time.
    if (sIsNetworkProvisioned && sIsNetworkEnabled)
    {
        systemStatusLED.Set(false);
    }
    else if (sHaveBLEConnections)
    {
        systemStatusLED.Blink(LedConsts::StatusLed::Unprovisioned::kOn_ms, LedConsts::StatusLed::Unprovisioned::kOff_ms);
    }
    else
    {
        // not commisioned yet
        systemStatusLED.Blink(LedConsts::StatusLed::Provisioned::kOn_ms, LedConsts::StatusLed::Provisioned::kOff_ms);
    }
}

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    // ChipLogProgress(Zcl, "DeviceEventCallback event_type 0x%x", event->Type);
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;

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
        }
        break;

    case DeviceEventType::kThreadConnectivityChange:
        break;

    case DeviceEventType::kThreadStateChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;

    case DeviceEventType::kCommissioningComplete:
        break;

    case DeviceEventType::kServerReady: {
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        if (!isOTAInitialized)
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
            isOTAInitialized = true;
        }
#endif
    }

    break;

#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
    case DeviceEventType::kBindingsChangedViaCluster:
        // TriggerSubscribe();
        break;
#endif
    }
}

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ChipLogProgress(Zcl, "IPv4 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
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

void IdentifyTimerHandler(Layer * systemLayer, void * appState)
{
    if (identifyTimerCount)
    {
        systemLayer->StartTimer(Clock::Milliseconds32(kIdentifyTimerDelayMS), IdentifyTimerHandler, appState);
        // Decrement the timer count.
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

    DeviceLayer::SystemLayer().CancelTimer(IdentifyTimerHandler, this);
    DeviceLayer::SystemLayer().StartTimer(Clock::Milliseconds32(kIdentifyTimerDelayMS), IdentifyTimerHandler, this);

exit:
    return;
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                                  uint16_t size, uint8_t * value)
{
    switch (clusterId)
    {
    case app::Clusters::Identify::Id:
        OnIdentifyPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    default:
        ChipLogProgress(DeviceLayer, "Unknown cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));
        break;
    }
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();

    if (cb != nullptr)
    {
        cb->PostAttributeChangeCallback(attributePath.mEndpointId, attributePath.mClusterId, attributePath.mAttributeId, type, size,
                                        value);
    }
}
