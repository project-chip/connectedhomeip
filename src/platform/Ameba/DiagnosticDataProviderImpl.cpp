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
#include <lib/support/CHIPMemString.h>
#include <platform/Ameba/AmebaUtils.h>
#include <platform/Ameba/DiagnosticDataProviderImpl.h>

#include <lwip_netconf.h>

using namespace chip::DeviceLayer::Internal;

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

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
    // If implemented, the server SHALL set the value of the CurrentHeapHighWatermark attribute to the
    // value of the CurrentHeapUsed.

    xPortResetHeapMinimumEverFreeHeapSize();

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    /* Obtain all available task information */
    TaskStatus_t * taskStatusArray;
    ThreadMetrics * head = nullptr;
    unsigned long arraySize, x, dummy;

    arraySize = uxTaskGetNumberOfTasks();

    taskStatusArray = (TaskStatus_t *) pvPortMalloc(arraySize * sizeof(TaskStatus_t));

    if (taskStatusArray != NULL)
    {
        /* Generate raw status information about each task. */
        arraySize = uxTaskGetSystemState(taskStatusArray, arraySize, &dummy);
        /* For each populated position in the taskStatusArray array,
           format the raw data as human readable ASCII data. */

        for (x = 0; x < arraySize; x++)
        {
            ThreadMetrics * thread = (ThreadMetrics *) pvPortMalloc(sizeof(ThreadMetrics));

            Platform::CopyString(thread->NameBuf, taskStatusArray[x].pcTaskName);
            thread->name.Emplace(CharSpan::fromCharString(thread->NameBuf));
            thread->id = taskStatusArray[x].xTaskNumber;

            thread->stackFreeMinimum.Emplace(taskStatusArray[x].usStackHighWaterMark);
            thread->stackSize.Emplace(uxTaskGetStackSize(taskStatusArray[x].xHandle));
            thread->stackFreeCurrent.Emplace(uxTaskGetFreeStackSize(taskStatusArray[x].xHandle));

            thread->Next = head;
            head         = thread;
        }

        *threadMetricsOut = head;
        /* The array is no longer needed, free the memory it consumes. */
        vPortFree(taskStatusArray);
    }

    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics)
{
    while (threadMetrics)
    {
        ThreadMetrics * del = threadMetrics;
        threadMetrics       = threadMetrics->Next;
        vPortFree(del);
    }
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

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgr().GetBootReason(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    NetworkInterface * head = NULL;
    struct ifaddrs * ifaddr = nullptr;

    // xnetif is never null, no need to check. If we do check with -Werror=address, we get compiler error.
    for (struct netif * ifa = xnetif; ifa != NULL; ifa = ifa->next)
    {
        NetworkInterface * ifp = new NetworkInterface();

        Platform::CopyString(ifp->Name, ifa->name);

        ifp->name          = CharSpan::fromCharString(ifp->Name);
        ifp->isOperational = true;
        if ((ifa->flags) & NETIF_FLAG_ETHERNET)
            ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kEthernet;
        else
            ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
        ifp->offPremiseServicesReachableIPv4.SetNull();
        ifp->offPremiseServicesReachableIPv6.SetNull();

        memcpy(ifp->MacAddress, ifa->hwaddr, sizeof(ifa->hwaddr));

        if (0)
        {
            ChipLogError(DeviceLayer, "Failed to get network hardware address");
        }
        else
        {
            // Set 48-bit IEEE MAC Address
            ifp->hardwareAddress = ByteSpan(ifp->MacAddress, 6);
        }

        if (ifa->ip_addr.u_addr.ip4.addr != 0)
        {
            memcpy(ifp->Ipv4AddressesBuffer[0], &(ifa->ip_addr.u_addr.ip4.addr), kMaxIPv4AddrSize);
            ifp->Ipv4AddressSpans[0] = ByteSpan(ifp->Ipv4AddressesBuffer[0], kMaxIPv4AddrSize);
            ifp->IPv4Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv4AddressSpans, 1);
        }

        // ifa->ip6_addr->u_addr.ip6.addr is never null, no need to check. If we do check with -Werror=address, we get compiler
        // error.
        memcpy(ifp->Ipv6AddressesBuffer[0], &(ifa->ip6_addr->u_addr.ip6.addr), kMaxIPv6AddrSize);
        ifp->Ipv6AddressSpans[0] = ByteSpan(ifp->Ipv6AddressesBuffer[0], kMaxIPv6AddrSize);
        ifp->IPv6Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, 1);

        ifp->Next = head;
        head      = ifp;
    }

    *netifpp = head;
    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface * netifp)
{
    while (netifp)
    {
        NetworkInterface * del = netifp;
        netifp                 = netifp->Next;
        delete del;
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & BssId)
{
    CHIP_ERROR err;
    int32_t error;

    constexpr size_t bssIdSize = 6;
    VerifyOrReturnError(BssId.size() >= bssIdSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    error = matter_wifi_get_ap_bssid(BssId.data());
    err   = AmebaUtils::MapError(error, AmebaErrorType::kWiFiError);

    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    BssId.reduce_size(bssIdSize);
    ChipLogProgress(DeviceLayer, "%02x,%02x,%02x,%02x,%02x,%02x\n", BssId.data()[0], BssId.data()[1], BssId.data()[2],
                    BssId.data()[3], BssId.data()[4], BssId.data()[5]);

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion)
{
    // Support 802.11a/n Wi-Fi in AmebaD chipset
    // TODO: https://github.com/project-chip/connectedhomeip/issues/25542
    wifiVersion = app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum::kN;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    CHIP_ERROR err;
    int32_t error;

    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;

    unsigned int _auth_type;
    unsigned short security = 0;
    rtw_wifi_setting_t setting;

    error = matter_wifi_get_security_type(WLAN0_IDX, &security, &setting.key_idx, setting.password);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kWiFiError);
    if (err != CHIP_NO_ERROR)
    {
        securityType = SecurityTypeEnum::kUnspecified;
    }
#ifdef CONFIG_PLATFORM_8721D
    else
    {
        switch (security)
        {
        case IW_ENCODE_ALG_NONE:
            securityType = SecurityTypeEnum::kNone;
            break;
        case IW_ENCODE_ALG_WEP:
            securityType = SecurityTypeEnum::kWep;
            break;
        case IW_ENCODE_ALG_TKIP:
            securityType = SecurityTypeEnum::kWpa;
            break;
        case IW_ENCODE_ALG_CCMP:
            securityType = SecurityTypeEnum::kWpa2;
            break;
        default:
            securityType = SecurityTypeEnum::kUnspecified;
            break;
        }
    }
#else
    else
    {
        switch (security)
        {
        case IW_ENCODE_ALG_NONE:
            securityType = SecurityTypeEnum::kNone;
            break;
        case IW_ENCODE_ALG_WEP:
            securityType = SecurityTypeEnum::kWep;
            break;
        case IW_ENCODE_ALG_TKIP:
            if (_auth_type == WPA_SECURITY)
                securityType = SecurityTypeEnum::kWpa;
            else if (_auth_type == WPA2_SECURITY)
                securityType = SecurityTypeEnum::kWpa2;
            break;
        case IW_ENCODE_ALG_CCMP:
            if (_auth_type == WPA_SECURITY)
                securityType = SecurityTypeEnum::kWpa;
            else if (_auth_type == WPA2_SECURITY)
                securityType = SecurityTypeEnum::kWpa2;
            else if (_auth_type == WPA3_SECURITY)
                securityType = SecurityTypeEnum::kWpa3;
            break;
        default:
            securityType = SecurityTypeEnum::kUnspecified;
            break;
        }
    }
#endif

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    CHIP_ERROR err;
    int32_t error;
    unsigned char channel;

    error = matter_wifi_get_wifi_channel_number(WLAN0_IDX, &channel);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kWiFiError);
    if (err != CHIP_NO_ERROR)
        channelNumber = 0;
    else
        channelNumber = (uint16_t) channel;

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    CHIP_ERROR err;
    int32_t error;

    error = matter_wifi_get_rssi((int *) &rssi);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kWiFiError);

    if (err != CHIP_NO_ERROR)
    {
        // set rssi to 0 upon error
        rssi = 0;
    }

    return err;
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

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
