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

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using chip::DeviceLayer::Internal::ESP32Utils;
namespace chip {
namespace DeviceLayer {

CHIP_ERROR ConnectivityManagerImpl::InitEthernet()
{
    esp_netif_config_t cfg  = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t * eth_netif = esp_netif_new(&cfg);

    // Init MAC and PHY configs to default
    eth_mac_config_t mac_config  = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config  = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr          = CONFIG_ETH_PHY_ADDR;
    phy_config.reset_gpio_num    = CONFIG_ETH_PHY_RST_GPIO;
    mac_config.smi_mdc_gpio_num  = CONFIG_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_ETH_MDIO_GPIO;
    esp_eth_mac_t * mac          = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t * phy          = esp_eth_phy_new_ip101(&phy_config);

    esp_eth_config_t config     = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
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
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
