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

#include <lwip_netconf.h>

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

            strncpy(thread->NameBuf, taskStatusArray[x].pcTaskName, kMaxThreadNameLength - 1);
            thread->NameBuf[kMaxThreadNameLength] = '\0';
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

    if (xnetif == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        for (struct netif * ifa = xnetif; ifa != NULL; ifa = ifa->next)
        {
            NetworkInterface * ifp = new NetworkInterface();

            strncpy(ifp->Name, ifa->name, Inet::InterfaceId::kMaxIfNameLength);
            ifp->Name[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';

            ifp->name          = CharSpan::fromCharString(ifp->Name);
            ifp->isOperational = true;
            if ((ifa->flags) & NETIF_FLAG_ETHERNET)
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_ETHERNET;
            else
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_WI_FI;
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

            if (ifa->ip6_addr->u_addr.ip6.addr != 0)
            {
                memcpy(ifp->Ipv6AddressesBuffer[0], &(ifa->ip6_addr->u_addr.ip6.addr), kMaxIPv6AddrSize);
                ifp->Ipv6AddressSpans[0] = ByteSpan(ifp->Ipv6AddressesBuffer[0], kMaxIPv6AddrSize);
                ifp->IPv6Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, 1);
            }

            ifp->Next = head;
            head      = ifp;
        }
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
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(ByteSpan & BssId)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;
    static uint8_t ameba_bssid[6];

    if (wifi_get_ap_bssid(ameba_bssid) == 0)
    {
        err = CHIP_NO_ERROR;
        ChipLogProgress(DeviceLayer, "%02x,%02x,%02x,%02x,%02x,%02x\n", ameba_bssid[0], ameba_bssid[1], ameba_bssid[2],
                        ameba_bssid[3], ameba_bssid[4], ameba_bssid[5]);
    }

    BssId = ameba_bssid;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(uint8_t & wifiVersion)
{
    // Support 802.11a/n Wi-Fi in AmebaD chipset
    wifiVersion = EMBER_ZCL_WI_FI_VERSION_TYPE_802__11N;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(uint8_t & securityType)
{
    unsigned int _auth_type;
    unsigned short _security = 0;
    rtw_wifi_setting_t setting;

#ifdef CONFIG_PLATFORM_8721D
    if (wext_get_enc_ext("wlan0", &_security, &setting.key_idx, setting.password) < 0)
    {
        securityType = 0;
    }
    else
    {
        switch (_security)
        {
        case IW_ENCODE_ALG_NONE:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_NONE;
            break;
        case IW_ENCODE_ALG_WEP:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_WEP;
            break;
        case IW_ENCODE_ALG_TKIP:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA;
            break;
        case IW_ENCODE_ALG_CCMP:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA2;
            break;
        default:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED;
            break;
        }
        securityType = setting.security_type;
    }
#else
    wext_get_enc_ext("wlan0", &_security, &setting.key_idx, setting.password);
    if (wext_get_auth_type("wlan0", &_auth_type) < 0)
    {
        securityType = 0;
    }
    else
    {
        switch (_security)
        {
        case IW_ENCODE_ALG_NONE:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_NONE;
            break;
        case IW_ENCODE_ALG_WEP:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_WEP;
            break;
        case IW_ENCODE_ALG_TKIP:
            if (_auth_type == WPA_SECURITY)
                setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA;
            else if (_auth_type == WPA2_SECURITY)
                setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA2;
            break;
        case IW_ENCODE_ALG_CCMP:
            if (_auth_type == WPA_SECURITY)
                setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA;
            else if (_auth_type == WPA2_SECURITY)
                setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA2;
            else if (_auth_type == WPA3_SECURITY)
                setting.security_type = EMBER_ZCL_SECURITY_TYPE_WPA3;
            break;
        default:
            setting.security_type = EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED;
            break;
        }
        securityType = setting.security_type;
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    unsigned char channel;

    if (wext_get_channel("wlan0", &channel) < 0)
        channelNumber = 0;
    else
        channelNumber = (uint16_t) channel;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    int _rssi = 0;
    if (wifi_get_rssi(&_rssi) < 0)
        rssi = 0;
    else
        rssi = _rssi;

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

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
