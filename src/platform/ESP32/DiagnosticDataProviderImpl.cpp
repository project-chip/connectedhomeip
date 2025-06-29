/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          for ESP32 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMemString.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ESP32/DiagnosticDataProviderImpl.h>
#include <platform/ESP32/ESP32Utils.h>

#include "esp_event.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "spi_flash_mmap.h"
#else
#include "esp_spi_flash.h"
#endif
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace {

InterfaceTypeEnum GetInterfaceType(const char * if_desc)
{
    if (strncmp(if_desc, "ap", strnlen(if_desc, 2)) == 0 || strncmp(if_desc, "sta", strnlen(if_desc, 3)) == 0)
        return InterfaceTypeEnum::kWiFi;
    if (strncmp(if_desc, "openthread", strnlen(if_desc, 10)) == 0)
        return InterfaceTypeEnum::kThread;
    if (strncmp(if_desc, "eth", strnlen(if_desc, 3)) == 0)
        return InterfaceTypeEnum::kEthernet;
    return InterfaceTypeEnum::kUnspecified;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum MapAuthModeToSecurityType(wifi_auth_mode_t authmode)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        return SecurityTypeEnum::kNone;
    case WIFI_AUTH_WEP:
        return SecurityTypeEnum::kWep;
    case WIFI_AUTH_WPA_PSK:
        return SecurityTypeEnum::kWpa;
    case WIFI_AUTH_WPA2_PSK:
        return SecurityTypeEnum::kWpa2;
    case WIFI_AUTH_WPA3_PSK:
        return SecurityTypeEnum::kWpa3;
    default:
        return SecurityTypeEnum::kUnspecified;
    }
}

app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum GetWiFiVersionFromAPRecord(wifi_ap_record_t ap_info)
{
    using app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum;
    if (ap_info.phy_11n)
        return WiFiVersionEnum::kN;
    else if (ap_info.phy_11g)
        return WiFiVersionEnum::kG;
    else if (ap_info.phy_11b)
        return WiFiVersionEnum::kB;
    else
        return WiFiVersionEnum::kUnknownEnumValue;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    currentHeapFree = esp_get_free_heap_size();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    currentHeapUsed = heap_caps_get_total_size(MALLOC_CAP_DEFAULT) - esp_get_free_heap_size();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    currentHeapHighWatermark = heap_caps_get_total_size(MALLOC_CAP_DEFAULT) - esp_get_minimum_free_heap_size();
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
    return ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    bootReason = BootReasonType::kUnspecified;
    uint8_t reason;
    reason = static_cast<uint8_t>(esp_reset_reason());
    if (reason == ESP_RST_UNKNOWN)
    {
        bootReason = BootReasonType::kUnspecified;
    }
    else if (reason == ESP_RST_POWERON)
    {
        bootReason = BootReasonType::kPowerOnReboot;
    }
    else if (reason == ESP_RST_BROWNOUT)
    {
        bootReason = BootReasonType::kBrownOutReset;
    }
    else if (reason == ESP_RST_SW)
    {
        bootReason = BootReasonType::kSoftwareReset;
    }
    else if (reason == ESP_RST_INT_WDT)
    {
        bootReason = BootReasonType::kSoftwareWatchdogReset;
        /* Reboot can be due to hardware or software watchdog*/
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    esp_netif_t * netif     = esp_netif_next(NULL);
    NetworkInterface * head = NULL;
    uint8_t ipv6_addr_count = 0;
    esp_ip6_addr_t ip6_addr[LWIP_IPV6_NUM_ADDRESSES];
    if (netif == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        for (esp_netif_t * ifa = netif; ifa != NULL; ifa = esp_netif_next(ifa))
        {
            NetworkInterface * ifp = new NetworkInterface();
            esp_netif_ip_info_t ipv4_info;
            uint8_t addressSize = 0;
            Platform::CopyString(ifp->Name, esp_netif_get_ifkey(ifa));
            ifp->name          = CharSpan::fromCharString(ifp->Name);
            ifp->isOperational = true;
            ifp->type          = GetInterfaceType(esp_netif_get_desc(ifa));
            ifp->offPremiseServicesReachableIPv4.SetNull();
            ifp->offPremiseServicesReachableIPv6.SetNull();
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
            if (ifp->type == InterfaceTypeEnum::kThread)
            {
                static_assert(OT_EXT_ADDRESS_SIZE <= sizeof(ifp->MacAddress), "Unexpected extended address size");
                if (ThreadStackMgr().GetPrimary802154MACAddress(ifp->MacAddress) == CHIP_NO_ERROR)
                {
                    addressSize = OT_EXT_ADDRESS_SIZE;
                }
            }
            else
#endif
                if (esp_netif_get_mac(ifa, ifp->MacAddress) == ESP_OK)
            {
                // For Wi-Fi or Ethernet interface, the MAC address size should be 6
                addressSize = 6;
            }
            if (addressSize != 0)
            {
                ifp->hardwareAddress = ByteSpan(ifp->MacAddress, addressSize);
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to get network hardware address");
            }
#ifndef CONFIG_DISABLE_IPV4
            if (esp_netif_get_ip_info(ifa, &ipv4_info) == ESP_OK)
            {
                memcpy(ifp->Ipv4AddressesBuffer[0], &(ipv4_info.ip.addr), kMaxIPv4AddrSize);
                ifp->Ipv4AddressSpans[0] = ByteSpan(ifp->Ipv4AddressesBuffer[0], kMaxIPv4AddrSize);
                ifp->IPv4Addresses       = app::DataModel::List<ByteSpan>(ifp->Ipv4AddressSpans, 1);
            }
#endif // !defined(CONFIG_DISABLE_IPV4)

            static_assert(kMaxIPv6AddrCount <= UINT8_MAX, "Count might not fit in ipv6_addr_count");
            auto addr_count = esp_netif_get_all_ip6(ifa, ip6_addr);
            if (addr_count < 0)
            {
                ipv6_addr_count = 0;
            }
            else
            {
                ipv6_addr_count = static_cast<uint8_t>(std::min(addr_count, static_cast<int>(kMaxIPv6AddrCount)));
            }
            for (uint8_t idx = 0; idx < ipv6_addr_count; ++idx)
            {
                memcpy(ifp->Ipv6AddressesBuffer[idx], ip6_addr[idx].addr, kMaxIPv6AddrSize);
                ifp->Ipv6AddressSpans[idx] = ByteSpan(ifp->Ipv6AddressesBuffer[idx], kMaxIPv6AddrSize);
            }
            ifp->IPv6Addresses = app::DataModel::List<ByteSpan>(ifp->Ipv6AddressSpans, ipv6_addr_count);

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

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
#ifdef CONFIG_FREERTOS_USE_TRACE_FACILITY
    ThreadMetrics * head = nullptr;
    uint32_t arraySize   = uxTaskGetNumberOfTasks();

    Platform::ScopedMemoryBuffer<TaskStatus_t> taskStatusArray;
    VerifyOrReturnError(taskStatusArray.Calloc(arraySize), CHIP_ERROR_NO_MEMORY);

    uint32_t dummyRunTimeCounter;
    arraySize = uxTaskGetSystemState(taskStatusArray.Get(), arraySize, &dummyRunTimeCounter);

    for (uint32_t i = 0; i < arraySize; i++)
    {
        auto thread = static_cast<ThreadMetrics *>(Platform::MemoryCalloc(1, sizeof(ThreadMetrics)));
        VerifyOrReturnError(thread, CHIP_ERROR_NO_MEMORY, ReleaseThreadMetrics(head));

        Platform::CopyString(thread->NameBuf, taskStatusArray[i].pcTaskName);
        thread->name.Emplace(CharSpan::fromCharString(thread->NameBuf));
        thread->id = taskStatusArray[i].xTaskNumber;
        thread->stackFreeMinimum.Emplace(taskStatusArray[i].usStackHighWaterMark);

        // Todo: Calculate stack size and current free stack value and assign.
        thread->stackFreeCurrent.ClearValue();
        thread->stackSize.ClearValue();

        thread->Next = head;
        head         = thread;
    }

    *threadMetricsOut = head;

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics)
{
#ifdef CONFIG_FREERTOS_USE_TRACE_FACILITY
    while (threadMetrics)
    {
        ThreadMetrics * del = threadMetrics;
        threadMetrics       = threadMetrics->Next;
        Platform::MemoryFree(del);
    }
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & BssId)
{
    constexpr size_t bssIdSize = 6;
    VerifyOrReturnError(BssId.size() >= bssIdSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err != ESP_OK)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    memcpy(BssId.data(), ap_info.bssid, bssIdSize);
    BssId.reduce_size(bssIdSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;

    securityType = SecurityTypeEnum::kUnspecified;
    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK)
    {
        securityType = MapAuthModeToSecurityType(ap_info.authmode);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion)
{
    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
    VerifyOrReturnError(err == ESP_OK, ESP32Utils::MapError(err));

    wifiVersion = GetWiFiVersionFromAPRecord(ap_info);
    VerifyOrReturnError(wifiVersion != app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum::kUnknownEnumValue,
                        CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    channelNumber = 0;
    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK)
    {
        channelNumber = ap_info.primary;
        return CHIP_NO_ERROR;
    }

    return ESP32Utils::MapError(err);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    rssi = 0;
    wifi_ap_record_t ap_info;
    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&ap_info);

    if (err == ESP_OK)
    {
        rssi = ap_info.rssi;
        return CHIP_NO_ERROR;
    }

    return ESP32Utils::MapError(err);
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
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
