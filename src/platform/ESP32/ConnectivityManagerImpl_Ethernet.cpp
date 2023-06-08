/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/internal/BLEManager.h>

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using chip::DeviceLayer::Internal::ESP32Utils;
namespace chip {
namespace DeviceLayer {

CHIP_ERROR ConnectivityManagerImpl::InitEthernet()
{
    // Initialize TCP/IP network interface (should be called for all Ethernet boards)
    ESP_ERROR_CHECK(esp_netif_init());
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::OnEthernetPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Platform.ESPSystemEvent.Id)
    {
    case IP_EVENT_ETH_GOT_IP:
        ChipLogProgress(DeviceLayer, "Ethernet Link Up");
        break;
    case IP_EVENT_ETH_LOST_IP:
        ChipLogProgress(DeviceLayer, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ChipLogProgress(DeviceLayer, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ChipLogProgress(DeviceLayer, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

} // namespace DeviceLayer
} // namespace chip
