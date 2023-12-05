/*
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

#include <lib/support/CHIPMemString.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>

extern "C" {
#include <bl_sys.h>

#include <bl60x_fw_api.h>
#include <bl60x_wifi_driver/bl_main.h>
#include <bl60x_wifi_driver/wifi_mgmr.h>
#include <wifi_mgmr_ext.h>
#include <wifi_mgmr_portable.h>
}

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    BL_RST_REASON_E bootCause = bl_sys_rstinfo_get();

    if (bootCause == BL_RST_POWER_OFF)
    {
        bootReason = BootReasonType::kPowerOnReboot;
    }
    else if (bootCause == BL_RST_HARDWARE_WATCHDOG)
    {
        bootReason = BootReasonType::kHardwareWatchdogReset;
    }
    else if (bootCause == BL_RST_SOFTWARE_WATCHDOG)
    {
        bootReason = BootReasonType::kSoftwareWatchdogReset;
    }
    else if (bootCause == BL_RST_SOFTWARE)
    {
        bootReason = BootReasonType::kSoftwareReset;
    }
    else
    {
        bootReason = BootReasonType::kUnspecified;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & BssId)
{
    if (ConnectivityMgrImpl()._IsWiFiStationConnected())
    {
        return CopySpanToMutableSpan(ByteSpan(wifiMgmr.wifi_mgmr_stat_info.bssid), BssId);
    }

    return CHIP_ERROR_READ_FAILED;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    if (ConnectivityMgrImpl()._IsWiFiStationConnected())
    {
        if (wifi_mgmr_security_type_is_open())
        {
            securityType = app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum::kNone;
        }
        else if (wifi_mgmr_security_type_is_wpa())
        {
            securityType = app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum::kWpa;
        }
        else if (wifi_mgmr_security_type_is_wpa2())
        {
            securityType = app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum::kWpa2;
        }
        else if (wifi_mgmr_security_type_is_wpa3())
        {
            securityType = app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum::kWpa3;
        }
        else
        {
            securityType = app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum::kWep;
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_READ_FAILED;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    if (ConnectivityMgrImpl()._IsWiFiStationConnected())
    {
        channelNumber = wifiMgmr.channel;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_READ_FAILED;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    if (ConnectivityMgrImpl()._IsWiFiStationConnected())
    {
        rssi = wifiMgmr.wlan_sta.sta.rssi;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_READ_FAILED;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    wifi_diagnosis_info_t * info;

    info = bl_diagnosis_get();
    if (info)
    {
        beaconLostCount = info->beacon_loss;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    wifi_diagnosis_info_t * info;

    info = bl_diagnosis_get();
    if (info)
    {
        packetMulticastRxCount = info->multicast_recv;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    wifi_diagnosis_info_t * info;

    info = bl_diagnosis_get();
    if (info)
    {
        packetMulticastTxCount = info->multicast_send;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    wifi_diagnosis_info_t * info;

    info = bl_diagnosis_get();
    if (info)
    {
        packetUnicastRxCount = info->unicast_recv;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    wifi_diagnosis_info_t * info;

    info = bl_diagnosis_get();
    if (info)
    {
        packetUnicastTxCount = info->multicast_send;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    wifi_diagnosis_info_t * info;

    info = bl_diagnosis_get();
    if (info)
    {
        beaconRxCount = info->beacon_recv;
    }

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
