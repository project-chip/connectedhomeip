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

#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/bouffalolab/BL702/wifi_mgmr_portable.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

extern "C" {
#include <bl_sys.h>
}

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    BL_RST_REASON_E bootCause = bl_sys_rstinfo_get();

    if (BL_RST_POR == bootCause)
    {
        bootReason = BootReasonType::kPowerOnReboot;
    }
    else if (BL_RST_BOR == bootCause)
    {
        bootReason = BootReasonType::kBrownOutReset;
    }
    else if (BL_RST_WDT == bootCause)
    {
        bootReason = BootReasonType::kHardwareWatchdogReset;
    }
    else if (BL_RST_SOFTWARE == bootCause)
    {
        bootReason = BootReasonType::kSoftwareReset;
    }
    else
    {
        bootReason = BootReasonType::kUnspecified;
    }
    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & BssId)
{
    struct bflbwifi_ap_record * pApInfo = wifiInterface_getApInfo();

    if (NULL == pApInfo)
    {
        ChipLogError(DeviceLayer, "Failed to get ap info.");
        return CHIP_ERROR_INTERNAL;
    }

    return CopySpanToMutableSpan(ByteSpan(pApInfo->bssid), BssId);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;
    struct bflbwifi_ap_record * pApInfo = wifiInterface_getApInfo();

    if (NULL == pApInfo)
    {
        ChipLogError(DeviceLayer, "Failed to get ap info.");
        return CHIP_ERROR_INTERNAL;
    }

    if (pApInfo->auth_mode < (uint8_t) (SecurityTypeEnum::kUnknownEnumValue))
    {
        securityType = (SecurityTypeEnum) (pApInfo->auth_mode);
    }
    else
    {
        securityType = SecurityTypeEnum::kUnknownEnumValue;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    struct bflbwifi_ap_record * pApInfo = wifiInterface_getApInfo();

    if (NULL == pApInfo)
    {
        ChipLogError(DeviceLayer, "Failed to get ap info.");
        return CHIP_ERROR_INTERNAL;
    }

    channelNumber = pApInfo->channel;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    struct bflbwifi_ap_record * pApInfo = wifiInterface_getApInfo();

    if (NULL == pApInfo)
    {
        ChipLogError(DeviceLayer, "Failed to get ap info.");
        return CHIP_ERROR_INTERNAL;
    }

    rssi = pApInfo->rssi;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
} // namespace DeviceLayer
} // namespace chip
