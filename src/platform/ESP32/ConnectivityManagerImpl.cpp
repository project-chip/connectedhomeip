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

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.cpp>
#endif

#include <esp_wifi.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/BLEManager.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;
// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    InitWiFi();
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
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR ConnectivityManagerImpl::_GetWiFiSecurityType(uint8_t & securityType)
{
    securityType = 0;
    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK)
    {
        securityType = ap_info.authmode;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiChannelNumber(uint16_t & channelNumber)
{
    channelNumber = 0;
    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK)
    {
        channelNumber = ap_info.primary;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiRssi(int8_t & rssi)
{
    rssi = 0;
    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);

    if (err == ESP_OK)
    {
        rssi = ap_info.rssi;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    beaconLostCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    currentMaxRate = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    packetMulticastRxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    packetMulticastTxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    packetUnicastRxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    packetUnicastTxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiOverrunCount(uint64_t & overrunCount)
{
    overrunCount = 0;
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
