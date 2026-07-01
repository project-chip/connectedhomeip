/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/*
 * Structs and defines used across TI Wi-Fi and LWIP Drivers
 */

#define CMD_BUFFER_LEN (256)
#define WLAN_REASON_DEAUTH_LEAVING (3)
#define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY (4)

/* Application defines */
#define WLAN_EVENT_TOUT (40000)
#define MAX_SCAN_TRAILS (10)
#define P2P_CONNECT_PRIORITY (SPAWN_TASK_PRIORITY - 1)
#define P2P_STACK_SIZE (2048)
#define P2P_REMOTE_DEVICE ("StartScan")
#define P2P_DEVICE_TYPE ("1-0050F204-1")
#define P2P_DEVICE_NAME ("cc32xx_p2p_device")
#define LISTEN_CHANNEL (11)
#define LISTEN_REGULATORY_CLASS (81)
#define OPRA_CHANNEL (6)
#define OPRA_REGULATORY_CLASS (81)
#define TIMEOUT_SEM (1)
#define LPDS_WAKEUP_SW (1)
#define MGMT (0)
#define CTRL (1)
#define DATA (2)
#define CC3x35_BIT (0x100000)
#define NET_IF_STA_BIT (0)
#define NET_IF_AP_BIT (1)
#define NET_IF_IS_UP (2)

#define BIT_x(x) (1 << (x))
#define IS_BIT_SET(bit_field, bit_num) (((bit_field) & BIT_x(bit_num)) > 0)
#define CLEAR_BIT_IN_BITMAP(bit_field, bit_num)                                                                                    \
    {                                                                                                                              \
        (bit_field) &= ~BIT_x(bit_num);                                                                                            \
    }
#define SET_BIT_IN_BITMAP(bit_field, bit_num)                                                                                      \
    {                                                                                                                              \
        (bit_field) |= BIT_x(bit_num);                                                                                             \
    }

#define SET_STATUS_BIT(status_variable, bit) status_variable |= (1 << (bit))

#define CLR_STATUS_BIT(status_variable, bit) status_variable &= ~(1 << (bit))

#define GET_STATUS_BIT(status_variable, bit) (0 != (status_variable & (1 << (bit))))
#define IS_PING_RUNNING(status_variable) GET_STATUS_BIT(status_variable, STATUS_BIT_PING_STARTED)
#define IS_STA_CONNECTED(status_variable) GET_STATUS_BIT(status_variable, STATUS_BIT_STA_CONNECTION)

typedef union
{
    WlanNetworkEntry_t netEntries[WLAN_MAX_SCAN_COUNT];
} gDataBuffer_t;

typedef struct connectionControlBlock_t
{
    OsiSyncObj_t disconnectEventSyncObj;
    OsiSyncObj_t connectEventSyncObj;
    OsiSyncObj_t eventCompletedSyncObj;
    OsiSyncObj_t dhcpIprecvSyncObj;
    OsiSyncObj_t staRoleupSyncObj;
    OsiSyncObj_t staRoledownSyncObj;
    uint32_t GatewayIP;
    uint8_t ConnectionSSID[WLAN_SSID_MAX_LENGTH + 1];
    uint8_t ConnectionBSSID[WLAN_BSSID_LENGTH];
    uint32_t DestinationIp;
    uint32_t IpAddr;
    uint32_t StaIp;
    uint32_t Ipv6Addr[4];
} connection_CB;

typedef struct appControlBlock_t
{
    /* Status Variables */
    /* This bit-wise status variable shows the state of the NWP */
    uint32_t Status;
    /* This field keeps the device's role (STA, P2P or AP) */
    uint32_t Role;
    /* This field keeps the device's role (STA, P2P or AP) */
    uint32_t ConnectedStations;
    /* This flag lets the application to exit */
    uint32_t Exit;
    /* Sets the number of Ping attempts to send */
    uint32_t PingAttempts;
    /* Data & Network entry Union */
    gDataBuffer_t gDataBuffer;
    /* Cmd Prompt buffer */
    uint8_t CmdBuffer[CMD_BUFFER_LEN];
    /* STA/AP mode CB */
    connection_CB CON_CB;

    OsiSyncObj_t eventCompletedScanObj;
} appControlBlock;

extern appControlBlock app_CB;
extern uint32_t ActiveNetIfBitMap;

typedef struct ConnectCmd
{
    /* Ap's SSID */
    uint8_t * ssid;
    /* Security parameters - Security Type and Password */
    WlanSecParams_t secParams;
} ConnectCmd_t;

typedef enum
{
    /* This bit is set: Network Processor is powered up */
    STATUS_BIT_NWP_INIT = 0,
    /* This bit is set: the device is connected to the AP (STA) */
    STATUS_BIT_STA_CONNECTION,
    /* This bit is set: the device is configured as an AP
       and is up(AP) */
    STATUS_BIT_AP_CONNECTION,
    /* This bit is set: One or more clients are connected to device (AP) */
    STATUS_BIT_PEER_CONNECTED,
    /* This bit is set: the device has leased IP to any connected client */
    STATUS_BIT_IP_LEASED,
    /* This bit is set: the device has acquired an IP */
    STATUS_BIT_IP_ACQUIRED,
    /* If this bit is set: the device (P2P mode)
       found any p2p-device in scan */
    STATUS_BIT_P2P_DEV_FOUND,
    /* If this bit is set: the device (P2P mode)
       found any p2p-negotiation request */
    STATUS_BIT_P2P_REQ_RECEIVED,
    /* If this bit is set: the device(P2P mode)
       connection to client(or reverse way) is failed */
    STATUS_BIT_CONNECTION_FAILED,
    /* This bit is set: device is undergoing ping operation */
    STATUS_BIT_PING_STARTED,
    /* This bit is set: Scan is running is background */
    STATUS_BIT_SCAN_RUNNING,
    /* If this bit is set: the device
       has acquired an IPv6 address */
    STATUS_BIT_IPV6_ACQUIRED,
    /* If this bit is set: the device has acquired
       an IPv6 address */
    STATUS_BIT_IPV6_GLOBAL_ACQUIRED,
    /* If this bit is set: the device has acquired
       an IPv6 address */
    STATUS_BIT_IPV6_LOCAL_ACQUIRED,

    /* If this bit is set: Authentication with ENT AP failed. */
    STATUS_BIT_AUTHENTICATION_FAILED,

    STATUS_BIT_RESET_REQUIRED,

    STATUS_BIT_TX_STARED

} e_StatusBits;
