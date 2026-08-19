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

#include <platform/ti/cc35xx/CC35XXConfig.h>
#include <platform/ti/cc35xx/ConnectivityManagerImpl.h>
#include <platform/ti/cc35xx/ti_wifi_structs.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include <type_traits>

#include <network_lwip.h>
#include <ti/drivers/net/wifi/wifi_host_driver/inc_adapt/osi_kernel.h>
#include <ti/drivers/net/wifi/wifi_host_driver/inc_adapt/wlan_if.h>

#include <ti/drivers/Board.h>

/* Driver Header files */
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerWFF3.h>

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for CC35XX"
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
#error "WiFi AP support must be enabled when building for CC35XX"
#endif

#define SHOW_WARNING(ret, errortype) Report("\n\r[line:%d, error code:%d] %s\n\r", __LINE__, ret, errortype);

#define OS_ERROR ("OS error, please refer \"NETAPP ERRORS CODES\" section in errno.h")

#define HWREG(x) (*((volatile unsigned long *) (x))) // TODO temporary need to be removed
#define ICACHE_BASE 0x41902000                       // TODO temporary need to be removed, only for M3, M33 has different address

// Configure the AP SSID, Password and Security
// This release supports only hardcoded configs for Wi-Fi Access Points

#define AP_SSID "WLP_Test_2p4Ghz"
#define AP_PASSWORD "12345678"
#define WLAN_SEC_TYPE WLAN_SEC_TYPE_WPA_WPA2

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

extern "C" void cc35xxLog(const char * aFormat, ...);
extern uint32_t isIp;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    cc35xxLog("ConnectivityManagerImpl::_GetWiFiStationMode()\n\r");
    return kWiFiStationMode_Disabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    cc35xxLog("ConnectivityManagerImpl::_IsWiFiStationEnabled()\n\r");
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    cc35xxLog("ConnectivityManagerImpl::_SetWiFiStationMode(%d)\n\r", val);
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    cc35xxLog("ConnectivityManagerImpl::_IsWiFiStationProvisioned()\n\r");
    return true;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    cc35xxLog("ConnectivityManagerImpl::_ClearWiFiStationProvision()\n\r");
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    cc35xxLog("ConnectivityManagerImpl::_SetWiFiAPMode(%d)\n\r", val);
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    cc35xxLog("ConnectivityManagerImpl::_DemandStartWiFiAP()\n\r");
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    cc35xxLog("ConnectivityManagerImpl::_StopOnDemandWiFiAP()\n\r");
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    cc35xxLog("ConnectivityManagerImpl::_MaintainOnDemandWiFiAP()\n\r");
}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeout(System::Clock::Timeout val)
{
    cc35xxLog("ConnectivityManagerImpl::_SetWiFiAPIdleTimeout()\n\r");
}
CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    cc35xxLog("ConnectivityManagerImpl::_GetAndLogWifiStatsCounters()\n\r");
    return CHIP_NO_ERROR;
}

System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval(void)
{
    cc35xxLog("ConnectivityManagerImpl::_GetWiFiStationReconnectInterval()\n\r");
    return System::Clock::Seconds16(15);
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    cc35xxLog("ConnectivityManagerImpl::_SetWiFiStationReconnectInterval()\n\r");
    return CHIP_NO_ERROR;
}

System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiAPIdleTimeout(void)
{
    cc35xxLog("ConnectivityManagerImpl::_GetWiFiAPIdleTimeout()\n\r");
    return System::Clock::Seconds16(0);
}

bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    cc35xxLog("ConnectivityManagerImpl::_HaveIPv4InternetConnectivity()\n\r");
    return IS_STA_CONNECTED(app_CB.Status) && GET_STATUS_BIT(app_CB.Status, STATUS_BIT_IP_ACQUIRED);
}

bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    cc35xxLog("ConnectivityManagerImpl::_HaveIPv6InternetConnectivity()\n\r");
    return GET_STATUS_BIT(app_CB.Status, STATUS_BIT_IPV6_ACQUIRED);
}

bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    cc35xxLog("ConnectivityManagerImpl::_HaveServiceConnectivity()\n\r");
    return _HaveIPv4InternetConnectivity() || _HaveIPv6InternetConnectivity();
}

bool ConnectivityManagerImpl::_CanStartWiFiScan(void)
{
    cc35xxLog("ConnectivityManagerImpl::_CanStartWiFiScan()\n\r");
    return IS_BIT_SET(ActiveNetIfBitMap, NET_IF_STA_BIT) && !GET_STATUS_BIT(app_CB.Status, STATUS_BIT_SCAN_RUNNING);
}

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode(void)
{
    cc35xxLog("ConnectivityManagerImpl::_GetWiFiAPMode()\n\r");
    return kWiFiAPMode_NotSupported;
}

// ==================== ConnectivityManager Platform Internal Methods ====================

/*
 *  ======== WlanStackEventHandler ========
 *
 *  Callback from the Wi-Fi Stack to deliver events to the application.
 *  This is registered via Wlan_Start(WlanStackEventHandler)
 *
 */
void WlanStackEventHandler(WlanEvent_t * pWlanEvent)
{
    void * staif = NULL;
    if (!pWlanEvent)
    {
        return;
    }

    Report("\n\r--> WlanStackEventHandler Id = %d\n\r", pWlanEvent->Id);

    switch (pWlanEvent->Id)
    {
    case WLAN_EVENT_CONNECT: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_CONNECT\n\r");
        WlanEventConnect_t * pWlanEventConnect = &pWlanEvent->Data.Connect;

        // Check connection status
        if (pWlanEventConnect->Status < 0)
        {
            Report("\n\r[WLAN EVENT HANDLER] Connection failed with status: %d\n\r", pWlanEventConnect->Status);
            osi_SyncObjSignal(&app_CB.CON_CB.connectEventSyncObj);
            break;
        }

        char ssid[WLAN_SSID_MAX_LENGTH + 1];
        char bssid[WLAN_BSSID_LENGTH + 1];

        os_memset(ssid, 0, sizeof(ssid));
        os_memcpy(ssid, pWlanEventConnect->SsidName, pWlanEventConnect->SsidLen);
        os_memset(bssid, 0, sizeof(bssid));
        os_memcpy(bssid, pWlanEventConnect->Bssid, WLAN_BSSID_LENGTH);

        Report("[WLAN EVENT HANDLER] STA Connected to the AP: %s, "
               "BSSID: %x:%x:%x:%x:%x:%x, "
               "Channel : %d"
               "\n\r",
               ssid, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], pWlanEventConnect->Channel);

        // Set connection status bit
        SET_STATUS_BIT(app_CB.Status, STATUS_BIT_STA_CONNECTION);

        // Copy connection info
        os_memcpy(app_CB.CON_CB.ConnectionSSID, pWlanEventConnect->SsidName, pWlanEventConnect->SsidLen);
        os_memcpy(app_CB.CON_CB.ConnectionBSSID, pWlanEventConnect->Bssid, WLAN_BSSID_LENGTH);

        staif = network_get_sta_if();
        if (staif != NULL)
        {
            network_set_up(staif);
        }

        osi_SyncObjSignal(&app_CB.CON_CB.connectEventSyncObj);
    }
    break;

    case WLAN_EVENT_DISCONNECT: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_DISCONNECT\n\r");
        WlanEventDisconnect_t * pWlanEventDisconnect = &pWlanEvent->Data.Disconnect;

        Report("[WLAN EVENT HANDLER] STA Disconnected - Reason Code: %d\n\r", pWlanEventDisconnect->ReasonCode);

        CLR_STATUS_BIT(app_CB.Status, STATUS_BIT_STA_CONNECTION);
        osi_SyncObjSignal(&app_CB.CON_CB.disconnectEventSyncObj);
    }
    break;
    case WLAN_EVENT_SCAN_RESULT: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_SCAN_RESULT\n\r");
        WlanEventScanResult_t * pEventScanResult = &pWlanEvent->Data.ScanResult;

        uint32_t numResults = pEventScanResult->NetworkListResultLen;
        Report("[WLAN EVENT HANDLER] Number of scan results received: %d \n\r", numResults);
        Report("[WLAN SCAN] Results:\n\r");

        char ssid[WLAN_SSID_MAX_LENGTH + 1];
        char bssid[WLAN_BSSID_LENGTH + 1];
        bool targetApFound = false;

        for (int index = 0; index < numResults; index++)
        {
            os_memset(ssid, 0, sizeof(ssid));
            os_memcpy(ssid, pEventScanResult->NetworkListResult[index].Ssid, pEventScanResult->NetworkListResult[index].SsidLen);
            os_memset(bssid, 0, sizeof(bssid));
            os_memcpy(bssid, pEventScanResult->NetworkListResult[index].Bssid, WLAN_BSSID_LENGTH);

            // Check if this is our target AP
            bool isTargetAP = (strcmp(ssid, AP_SSID) == 0);
            if (isTargetAP)
            {
                targetApFound = true;
            }

            Report("SCAN : %02d : %32s : %02x:%02x:%02x:%02x:%02x:%02x : CH=%2d, SEC=%04x, RSSI=%3d %s\n\r", index, ssid, bssid[0],
                   bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], pEventScanResult->NetworkListResult[index].Channel,
                   pEventScanResult->NetworkListResult[index].SecurityInfo, pEventScanResult->NetworkListResult[index].Rssi,
                   isTargetAP ? "<<< TARGET AP" : "");
        }

        if (!targetApFound)
        {
            Report("\n\r[WARNING] Target AP '%s' NOT FOUND in scan results!\n\r", AP_SSID);
        }
        else
        {
            Report("\n\r[INFO] Target AP '%s' found in scan results\n\r", AP_SSID);
        }

        // Signal scan completion
        osi_SyncObjSignal(&app_CB.eventCompletedScanObj);
    }
    break;
    case WLAN_EVENT_ADD_PEER: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_ADD_PEER\n\r");
    }
    break;
    case WLAN_EVENT_REMOVE_PEER: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_REMOVE_PEER\n\r");
    }
    break;
    case WLAN_EVENT_CONNECTING: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_CONNECTING\n\r");
        WlanEventConnecting_t * pWlanEventConnecting = &pWlanEvent->Data.Connecting;

        char ssid[WLAN_SSID_MAX_LENGTH + 1];
        char bssid[WLAN_BSSID_LENGTH + 1];

        /* Copy new connection SSID and BSSID to global parameters */
        os_memset(ssid, 0, sizeof(ssid));
        os_memcpy(ssid, pWlanEventConnecting->SsidName, pWlanEventConnecting->SsidLen);
        os_memset(bssid, 0, sizeof(bssid));
        os_memcpy(bssid, pWlanEventConnecting->Bssid, WLAN_BSSID_LENGTH);

        Report("[WLAN EVENT HANDLER] STA Connecting to the AP: %s, "
               "BSSID: %x:%x:%x:%x:%x:%x, "
               "\n\r",
               ssid, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    }
    break;
    case WLAN_EVENT_ACTION_FRAME_RX: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_ACTION_FRAME_RX\n\r");
    }
    break;
    case WLAN_EVENT_ASSOCIATED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_ASSOCIATED\n\r");
    }
    break;
    case WLAN_EVENT_AP_EXT_WPS_SETTING_FAILED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_AP_EXT_WPS_SETTING_FAILED\n\r");
    }
    break;
    case WLAN_EVENT_BLE_ENABLED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_BLE_ENABLED\n\r");
    }
    break;
    case WLAN_EVENT_CS_FINISH: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_CS_FINISH\n\r");
    }
    break;
    case WLAN_EVENT_ROC_DONE: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_ROC_DONE\n\r");
    }
    break;
    case WLAN_EVENT_CROC_DONE: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_CROC_DONE\n\r");
    }
    break;
    case WLAN_EVENT_SEND_ACTION_DONE: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_SEND_ACTION_DONE\n\r");
    }
    break;
    case WLAN_EVENT_EXTENDED_SCAN_RESULT: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_EXTENDED_SCAN_RESULT\n\r");
    }
    break;
    case WLAN_EVENT_P2P_GROUP_STARTED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_P2P_GROUP_STARTED\n\r");
    }
    break;
    case WLAN_EVENT_P2P_GROUP_REMOVED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_P2P_GROUP_REMOVED\n\r");
    }
    break;
    case WLAN_EVENT_P2P_SCAN_COMPLETED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_P2P_SCAN_COMPLETED\n\r");
    }
    break;
    case WLAN_EVENT_P2P_GROUP_FORMATION_FAILED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_P2P_GROUP_FORMATION_FAILED\n\r");
    }
    break;
    case WLAN_EVENT_P2P_PEER_NOT_FOUND: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_P2P_PEER_NOT_FOUND\n\r");
    }
    break;
    case WLAN_EVENT_CONNECT_PERIODIC_SCAN_COMPLETE: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_CONNECT_PERIODIC_SCAN_COMPLETE\n\r");
    }
    break;
    case WLAN_EVENT_FW_CRASH: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_FW_CRASH\n\r");
    }
    break;
    case WLAN_EVENT_COMMAND_TIMEOUT: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_COMMAND_TIMEOUT\n\r");
    }
    break;
    case WLAN_EVENT_GENERAL_ERROR: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_GENERAL_ERROR\n\r");
    }
    break;
    case WLAN_EVENT_BSS_TRANSITION_INITIATED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_BSS_TRANSITION_INITIATED\n\r");
    }
    break;
    case WLAN_EVENT_PEER_AGING: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_PEER_AGING\n\r");
    }
    break;
    case WLAN_EVENT_ERROR: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_ERROR\n\r");
    }
    break;
    case WLAN_EVENT_AUTHENTICATION_REJECTED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_AUTHENTICATION_REJECTED\n\r");
        Report("\n\r[WLAN EVENT] Authentication rejected - check credentials\n\r");
        // Signal connection event to unblock waiting code
        osi_SyncObjSignal(&app_CB.CON_CB.connectEventSyncObj);
    }
    break;
    case WLAN_EVENT_ASSOCIATION_REJECTED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_ASSOCIATION_REJECTED\n\r");
        Report("\n\r[WLAN EVENT] Association rejected\n\r");
        // Signal connection event to unblock waiting code
        osi_SyncObjSignal(&app_CB.CON_CB.connectEventSyncObj);
    }
    break;
    case WLAN_EVENT_WPS_INVALID_PIN: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_WPS_INVALID_PIN\n\r");
    }
    break;
    case WLAN_EVENT_AP_WPS_START_FAILED: {
        Report("\n\r--> WlanStackEventHandler WLAN_EVENT_AP_WPS_START_FAILED\n\r");
    }
    break;
    default: {
        Report("\n\r--> WlanStackEventHandler !! UNEXPECTED !! Event ID: %d\n\r", pWlanEvent->Id);
    }
    break;
    }
}

int32_t initAppVariables(void)
{
    int32_t ret = 0;

    app_CB.Status = 0;
    app_CB.Role   = WLAN_ROLE_RESERVED;
    app_CB.Exit   = FALSE;

    os_memset(&app_CB.CmdBuffer, 0x0, CMD_BUFFER_LEN);
    os_memset(&app_CB.gDataBuffer, 0x0, sizeof(app_CB.gDataBuffer));
    os_memset(&app_CB.CON_CB, 0x0, sizeof(app_CB.CON_CB));

    ret = osi_SyncObjCreate(&app_CB.CON_CB.disconnectEventSyncObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }

    ret = osi_SyncObjCreate(&app_CB.CON_CB.connectEventSyncObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }

    ret = osi_SyncObjCreate(&app_CB.CON_CB.eventCompletedSyncObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }
    ret = osi_SyncObjCreate(&app_CB.eventCompletedScanObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }
    ret = osi_SyncObjCreate(&app_CB.CON_CB.dhcpIprecvSyncObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }

    ret = osi_SyncObjCreate(&app_CB.CON_CB.staRoleupSyncObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }

    ret = osi_SyncObjCreate(&app_CB.CON_CB.staRoledownSyncObj);
    if (ret != 0)
    {
        SHOW_WARNING(ret, OS_ERROR);
        return (-1);
    }

    return (ret);
}

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    int32_t ret = 0;

    HWREG(ICACHE_BASE + 0x84) |= 0x00000001;
    HWREG(ICACHE_BASE + 0x4) |= 0xc0000000;

    Board_init();

    // Initialize LWIP
    initAppVariables();
    network_stack_init();

    Report("\n\r\n\r");
    Report("**** CC35XX Wi-Fi Init ****\n\r");

    Report("Disabling Power Management (DISABLE_SLEEP)\n\r");
    Power_setConstraint(PowerWFF3_DISALLOW_SLEEP);

    Report("\n\r** Wlan_Start(<StackHandler>)\n\r");
    ret = Wlan_Start(WlanStackEventHandler);
    if (ret == 0)
    {
        SET_BIT_IN_BITMAP(ActiveNetIfBitMap, NET_IF_IS_UP);
        Report("Wlan_Start success!\n\r");

        // Configure power management (ELP mode for CC35XX)
        uint32_t powerManagement = (uint32_t) POWER_MANAGEMENT_ELP_MODE;
        ret                      = Wlan_Set(WLAN_SET_POWER_MANAGEMENT, &powerManagement);
        if (ret == 0)
        {
            Report("Power management (ELP mode) configured successfully\n\r");
        }
        else
        {
            Report("Power management configuration failed: %d\n\r", ret);
        }
    }
    else
    {
        Report("Wlan_Start failed: %d\n\r", ret);
        return CHIP_ERROR_INTERNAL;
    }

    Report("\n\r** Wlan_Set(WLAN_SET_TX_CTRL) **\n\r");
    WlanCtrlBlk_t CtrlBlkParam;
    CtrlBlkParam.TxSendPaceThresh      = 1;
    CtrlBlkParam.TransmitQOnTxComplete = 1; // Changed from 0 to 1 to match working example
    CtrlBlkParam.TxSendPaceTimeoutMsec = 1; // Changed from 16 to 1 to match working example
    ret                                = Wlan_Set(WLAN_SET_TX_CTRL, &CtrlBlkParam);
    if (ret == 0)
    {
        Report("Wlan_Set(WLAN_SET_TX_CTRL) success!\n\r");
    }
    else
    {
        Report("Wlan_Set(WLAN_SET_TX_CTRL) failed: %d\n\r", ret);
    }

    Report("\n\r** Wlan_RoleUp(WLAN_ROLE_STA) **\n\r");

    // Check if network station is already active
    if (IS_BIT_SET(ActiveNetIfBitMap, NET_IF_STA_BIT))
    {
        Report("\n\rNetwork Station Is Already Active.\n\r");
    }
    else
    {
        CLR_STATUS_BIT(app_CB.Status, STATUS_BIT_STA_CONNECTION);

        RoleUpStaCmd_t RoleUpStaParams;
        os_memset(&RoleUpStaParams, 0, sizeof(RoleUpStaCmd_t));

        // Set 2.4G and 5G bands for CC35XX
        uint8_t sta_wifi_band = (uint8_t) BAND_SEL_BOTH;
        Wlan_Set(WLAN_SET_STA_WIFI_BAND, &sta_wifi_band);

        // Configure WPS parameters (disabled by default for Matter)
        RoleUpStaParams.wpsDisabled      = TRUE;
        RoleUpStaParams.countryDomain[0] = '\0';
        RoleUpStaParams.countryDomain[1] = '\0';

        // Add network interface and role up
        network_stack_add_if_sta();

        // Wait for network stack interface to be ready
        ret = osi_SyncObjWait(&(app_CB.CON_CB.staRoleupSyncObj), OSI_WAIT_FOR_SECOND * 10);
        if (OSI_OK != ret)
        {
            Report("\n\r[ERROR]_Init: Failed waiting for staRoleup sync object (%d)\n\r", ret);
            network_stack_remove_if_sta();
            return CHIP_ERROR_TIMEOUT;
        }

        ret = Wlan_RoleUp(WLAN_ROLE_STA, &RoleUpStaParams, WLAN_WAIT_FOREVER);
        if (ret < 0)
        {
            network_stack_remove_if_sta();
            Report("\n\r[ERROR]_Init: Wlan_RoleUp Failed with error code: %d\n\r", ret);
            return CHIP_ERROR_INTERNAL;
        }

        SET_BIT_IN_BITMAP(ActiveNetIfBitMap, NET_IF_STA_BIT);
        app_CB.Role = WLAN_ROLE_STA;

        // Short delay after role up
        os_sleep(1, 0);

        Report("Wlan_RoleUp success!\n\r");
    }

    Report("\n\r** Wlan_Scan(BAND_SEL_BOTH, 30) **\n\r");
    scanCommon_t scanCommo;
    os_memset(&scanCommo, 0x0, sizeof(scanCommon_t));

    // options are BAND_SEL_ONLY_2_4GHZ , BAND_SEL_ONLY_5GHZ , BAND_SEL_BOTH
    scanCommo.Band = BAND_SEL_BOTH;

    // Clear scan sync object before starting scan
    osi_SyncObjClear(&app_CB.eventCompletedScanObj);

    ret = Wlan_Scan(WLAN_ROLE_STA, &scanCommo, 30);
    if (ret != 0)
    {
        Report("\n\r[ERROR]_Init: Wlan_Scan failed: %d\n\r", ret);
        return CHIP_ERROR_INTERNAL;
    }

    Report("Wlan_Scan initiated, waiting for scan results...\n\r");

    // Wait for scan completion with timeout (10 seconds)
    ret = osi_SyncObjWait(&app_CB.eventCompletedScanObj, OSI_WAIT_FOR_SECOND * 10);
    if (ret != OSI_OK)
    {
        Report("\n\r[ERROR]_Init: Scan timeout or failed (%d)\n\r", ret);
        return CHIP_ERROR_TIMEOUT;
    }

    Report("Scan completed successfully\n\r");

    Report("\n\r** Wlan_Connect(SSID/TYPE/PSWD) **\n\r");

    // Check if STA role is active before connecting
    if (!IS_BIT_SET(ActiveNetIfBitMap, NET_IF_STA_BIT))
    {
        Report("\n\rNo STA role up, cannot connect\n\r");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Clear the connect event sync object before attempting connection
    osi_SyncObjClear(&(app_CB.CON_CB.connectEventSyncObj));

    ret = Wlan_Connect((const signed char *) AP_SSID, strlen(AP_SSID), NULL, WLAN_SEC_TYPE, AP_PASSWORD, strlen(AP_PASSWORD), 0);

    if (ret != 0)
    {
        Report("\n\r[ERROR]_Init: Wlan_Connect failed: %d\n\r", ret);
        return CHIP_ERROR_INTERNAL;
    }

    Report("Wlan_Connect initiated successfully!\n\r");

    // Wait for connection event with timeout
    if (!IS_STA_CONNECTED(app_CB.Status))
    {
        Report("Waiting for connection event...\n\r");
        ret = osi_SyncObjWait(&(app_CB.CON_CB.connectEventSyncObj), OSI_WAIT_FOR_SECOND * 60);
        if (ret != OSI_OK)
        {
            Report("\n\r[ERROR]_Init: Timeout expired connecting to AP: %s (error: %d)\n\r", AP_SSID, ret);
            Wlan_Disconnect(WLAN_ROLE_STA, nullptr);
            return CHIP_ERROR_TIMEOUT;
        }
        Report("Connected to AP successfully!\n\r");
    }

    Report("Wait for IP address assignment...\n\r");
    const int kMaxIpWaitSeconds = 30;
    for (int i = 0; i < kMaxIpWaitSeconds && isIp == 0; i++)
    {
        Report(".");
        os_sleep(1, 0);
    }

    if (isIp == 0)
    {
        Report("\n\r[ERROR]_Init: Timeout waiting for IP address\n\r");
        return CHIP_ERROR_TIMEOUT;
    }

    Report("\n\rReceived IP address successfully!\n\r");
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    cc35xxLog("ConnectivityManagerImpl::_OnPlatformEvent()\n\r");

    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ChipLogProgress(AppServer, "Commissioning completed successfully");
    }
}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    cc35xxLog("ConnectivityManagerImpl::_OnWiFiScanDone()\n\r");
}
void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    cc35xxLog("ConnectivityManagerImpl::_OnWiFiStationProvisionChange()\n\r");
}

// ==================== ConnectivityManager Private Methods ====================

void ConnectivityManagerImpl::_OnIpAcquired()
{
    cc35xxLog("ConnectivityManagerImpl::OnIpAcquired() : Start DNS Server");
    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnStationConnected()
{
    cc35xxLog("ConnectivityManagerImpl::OnStationConnected()\n\r");
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    cc35xxLog("ConnectivityManagerImpl::ConfigureWiFiAP()\n\r");
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    cc35xxLog("ConnectivityManagerImpl::ChangeWiFiAPState()\n\r");
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    cc35xxLog("ConnectivityManagerImpl::UpdateInternetConnectivityState()\n\r");
}

void ConnectivityManagerImpl::OnStationIPv4AddressAvailable()
{
    cc35xxLog("ConnectivityManagerImpl::OnStationIPv4AddressAvailable()\n\r");
}

void ConnectivityManagerImpl::OnStationIPv4AddressLost(void)
{
    cc35xxLog("ConnectivityManagerImpl::OnStationIPv4AddressLost()\n\r");
}

void ConnectivityManagerImpl::OnIPv6AddressAvailable()
{
    cc35xxLog("ConnectivityManagerImpl::OnIPv6AddressAvailable()\n\r");
}

void ConnectivityManagerImpl::RefreshMessageLayer(void)
{
    cc35xxLog("ConnectivityManagerImpl::RefreshMessageLayer()\n\r");
}

} // namespace DeviceLayer
} // namespace chip
