/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <platform/ESP32/ESP32Utils.h>

#include "esp_netif.h"
#include "esp_netif_net_stack.h"

using namespace ::chip::DeviceLayer::Internal;

struct netif * ESP32Utils::GetStationNetif(void)
{
    return GetNetif(kDefaultWiFiStationNetifKey);
}

struct netif * ESP32Utils::GetNetif(const char * ifKey)
{
    struct netif * netif       = NULL;
    esp_netif_t * netif_handle = NULL;
    netif_handle               = esp_netif_get_handle_from_ifkey(ifKey);
    netif                      = (struct netif *) esp_netif_get_netif_impl(netif_handle);
    return netif;
}

bool ESP32Utils::IsInterfaceUp(const char * ifKey)
{
    struct netif * netif = GetNetif(ifKey);
    return netif != NULL && netif_is_up(netif);
}

bool ESP32Utils::HasIPv6LinkLocalAddress(const char * ifKey)
{
    struct esp_ip6_addr if_ip6_unused;
    return esp_netif_get_ip6_linklocal(esp_netif_get_handle_from_ifkey(ifKey), &if_ip6_unused) == ESP_OK;
}
