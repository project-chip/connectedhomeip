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

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <esp_wifi.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/internal/BLEManager.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI && CHIP_DEVICE_CONFIG_WIFI_NETWORK_DRIVER
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(CONFIG_WIFI_NETWORK_ENDPOINT_ID /* Endpoint Id */,
                                      &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET && CHIP_DEVICE_CONFIG_ETHERNET_NETWORK_DRIVER
app::Clusters::NetworkCommissioning::Instance
    sEthernetNetworkCommissioningInstance(CONFIG_ETHERNET_NETWORK_ENDPOINT_ID /* Endpoint Id */,
                                          &(NetworkCommissioning::ESPEthernetDriver::GetInstance()));
#endif
// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    InitWiFi();
#if CHIP_DEVICE_CONFIG_WIFI_NETWORK_DRIVER
    sWiFiNetworkCommissioningInstance.Init();
#endif // CHIP_DEVICE_CONFIG_WIFI_NETWORK_DRIVER
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    InitEthernet();
#if CHIP_DEVICE_CONFIG_ETHERNET_NETWORK_DRIVER
    sEthernetNetworkCommissioningInstance.Init();
#endif // CHIP_DEVICE_CONFIG_ETHERNET_NETWORK_DRIVER
#endif

#if defined(CONFIG_WIFI_NETWORK_ENDPOINT_ID) && defined(CONFIG_THREAD_NETWORK_ENDPOINT_ID)
    static_assert(CONFIG_WIFI_NETWORK_ENDPOINT_ID != CONFIG_THREAD_NETWORK_ENDPOINT_ID,
                  "Wi-Fi network endpoint id and Thread network endpoint id should not be the same.");
#endif
#if defined(CONFIG_WIFI_NETWORK_ENDPOINT_ID) && defined(CONFIG_ETHERNET_NETWORK_ENDPOINT_ID)
    static_assert(CONFIG_WIFI_NETWORK_ENDPOINT_ID != CONFIG_ETHERNET_NETWORK_ENDPOINT_ID,
                  "Wi-Fi network endpoint id and Ethernet network endpoint id should not be the same.");
#endif
#if defined(CONFIG_THREAD_NETWORK_ENDPOINT_ID) && defined(CONFIG_ETHERNET_NETWORK_ENDPOINT_ID)
    static_assert(CONFIG_THREAD_NETWORK_ENDPOINT_ID != CONFIG_ETHERNET_NETWORK_ENDPOINT_ID,
                  "Thread network endpoint id and Ethernet network endpoint id should not be the same.");
#endif

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    OnWiFiPlatformEvent(event);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    OnEthernetPlatformEvent(event);
#endif
}

} // namespace DeviceLayer
} // namespace chip
