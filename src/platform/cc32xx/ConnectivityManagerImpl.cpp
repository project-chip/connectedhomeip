/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>

#include <platform/cc32xx/CC32XXConfig.h>

#include <app/server/Dnssd.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include <type_traits>

extern "C" {
#include <ti/net/slnetconn.h>
#define SLNETCONN_TIMEOUT 0xffff // "infinite" Timeout

extern void SlNetConnEventHandler(uint32_t ifID, SlNetConnStatus_e netStatus, void * data);
}

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for CC32XX"
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
#error "WiFi AP support must be enabled when building for CC32XX"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;

extern "C" void cc32xxLog(const char * aFormat, ...);
static struct netif * m_pNetIf = NULL;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    cc32xxLog("ConnectivityManagerImpl::_GetWiFiStationMode()\n\r");
    return mWiFiStationMode;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    cc32xxLog("ConnectivityManagerImpl::_IsWiFiStationEnabled()\n\r");
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    cc32xxLog("ConnectivityManagerImpl::_SetWiFiStationMode(%d)\n\r", val);
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    cc32xxLog("ConnectivityManagerImpl::_IsWiFiStationProvisioned()\n\r");
    return true;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    cc32xxLog("ConnectivityManagerImpl::_ClearWiFiStationProvision()\n\r");
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    cc32xxLog("ConnectivityManagerImpl::_SetWiFiAPMode(%d)\n\r", val);
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    cc32xxLog("ConnectivityManagerImpl::_DemandStartWiFiAP()\n\r");
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    cc32xxLog("ConnectivityManagerImpl::_StopOnDemandWiFiAP()\n\r");
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    cc32xxLog("ConnectivityManagerImpl::_MaintainOnDemandWiFiAP()\n\r");
}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeoutMS(uint32_t val)
{
    cc32xxLog("ConnectivityManagerImpl::_SetWiFiAPIdleTimeoutMS()\n\r");
    mWiFiAPIdleTimeoutMS = val;
    // SystemLayer.ScheduleWork(DriveAPState, NULL);
}

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    cc32xxLog("ConnectivityManagerImpl::_GetAndLogWifiStatsCounters()\n\r");
    return CHIP_NO_ERROR;
}

// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    int rc;
    cc32xxLog("Start LWIP");
    rc = LWIP_IF_init(_OnLwipEvent, false);
    if (rc == 0)
    {
        m_pNetIf = LWIP_IF_addInterface();
    }
    if (m_pNetIf == NULL)
    {
        cc32xxLog("LWIP IF not started, error = %d", rc);
    }
    else
    {
        cc32xxLog("Start Wi-Fi");
        /* Try to connect to AP and go through provisioning (if needed) */
        rc = SlNetConn_start(SLNETCONN_SERVICE_LVL_MAC, SlNetConnEventHandler, SLNETCONN_TIMEOUT, 0);
        assert(rc == 0);

        LWIP_IF_setLinkUp(m_pNetIf);
    }
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    cc32xxLog("ConnectivityManagerImpl::_OnPlatformEvent()\n\r");

    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        // if (event->CommissioningComplete.Status == CHIP_NO_ERROR)
        // {
        ChipLogProgress(AppServer, "Commissioning completed successfully");
        DeviceLayer::Internal::CC32XXConfig::WriteKVSToNV();
        // }
        // else
        // {
        //     ChipLogError(AppServer, "Commissioning failed with error %" CHIP_ERROR_FORMAT,
        //                  event->CommissioningComplete.Status.Format());
        // }
    }
}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    cc32xxLog("ConnectivityManagerImpl::_OnWiFiScanDone()\n\r");
}
void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    cc32xxLog("ConnectivityManagerImpl::_OnWiFiStationProvisionChange()\n\r");
}

// ==================== ConnectivityManager Private Methods ====================
void ConnectivityManagerImpl::_OnLwipEvent(struct netif * pNetIf, NetIfStatus_e status, void * pParams)
{
    switch (status)
    {
    case E_NETIF_STATUS_IP_ACQUIRED:
        PlatformMgr().ScheduleWork(_OnIpAcquired);
        break;
    default:
        break;
    }
}

void ConnectivityManagerImpl::_OnIpAcquired(intptr_t arg)
{
    cc32xxLog("ConnectivityManagerImpl::OnIpAcquired() : Start DNS Server");
    chip::app::DnssdServer::Instance().StartServer();
}

void ConnectivityManagerImpl::OnStationConnected()
{
    cc32xxLog("ConnectivityManagerImpl::OnStationConnected()\n\r");
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    cc32xxLog("ConnectivityManagerImpl::ChangeWiFiStationState()\n\r");
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    cc32xxLog("ConnectivityManagerImpl::ConfigureWiFiAP()\n\r");
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    cc32xxLog("ConnectivityManagerImpl::ChangeWiFiAPState()\n\r");
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    cc32xxLog("ConnectivityManagerImpl::UpdateInternetConnectivityState()\n\r");
}

void ConnectivityManagerImpl::OnStationIPv4AddressAvailable()
{
    cc32xxLog("ConnectivityManagerImpl::OnStationIPv4AddressAvailable()\n\r");
}

void ConnectivityManagerImpl::OnStationIPv4AddressLost(void)
{
    cc32xxLog("ConnectivityManagerImpl::OnStationIPv4AddressLost()\n\r");
}

void ConnectivityManagerImpl::OnIPv6AddressAvailable()
{
    cc32xxLog("ConnectivityManagerImpl::OnIPv6AddressAvailable()\n\r");
}

void ConnectivityManagerImpl::RefreshMessageLayer(void)
{
    cc32xxLog("ConnectivityManagerImpl::RefreshMessageLayer()\n\r");
}

} // namespace DeviceLayer
} // namespace chip
