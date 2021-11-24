/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for Ameba platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/Ameba/DiagnosticDataProviderImpl.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    currentHeapFree = xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    currentHeapUsed = xPortGetTotalHeapSize() - xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    currentHeapHighWatermark = xPortGetTotalHeapSize() - xPortGetMinimumEverFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetUpTime(uint64_t & upTime)
{
    System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp startTime   = PlatformMgrImpl().GetStartTime();

    if (currentTime >= startTime)
    {
        upTime = std::chrono::duration_cast<System::Clock::Seconds64>(currentTime - startTime).count();
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    uint64_t upTime = 0;

    if (GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalHours = 0;
        if (ConfigurationMgr().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(uint8_t & bootReason)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgr().GetBootReason(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<uint8_t>(reason);
    }

    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(uint8_t & securityType)
{
    securityType = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    channelNumber = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    rssi = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    beaconLostCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    currentMaxRate = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    packetMulticastRxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    packetMulticastTxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    packetUnicastRxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    packetUnicastTxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    overrunCount = 0;
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
