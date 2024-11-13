/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef RSI_CONFIG_H
#define RSI_CONFIG_H

#include "rsi_wlan_defines.h"

#if (SL_MATTER_GN_BUILD == 0)
#include "sl_matter_wifi_config.h"
#endif // SL_MATTER_GN_BUILD

//! Enable feature
#define RSI_ENABLE 1
//! Disable feature
#define RSI_DISABLE 0

//! To enable wlan opermode
#define RSI_OPERMODE_WLAN 0

//! To enable concurrent mode
#define CONCURRENT_MODE RSI_DISABLE

//! opermode command paramaters
/*=======================================================================*/
//! To set wlan feature select bit map
#define RSI_FEATURE_BIT_MAP (FEAT_SECURITY_OPEN)

//! TCP IP BYPASS feature check
#define RSI_TCP_IP_BYPASS RSI_ENABLE
#define RSI_TCP_IP_FEATURE_BIT_MAP (TCP_IP_FEAT_BYPASS /*| TCP_IP_FEAT_EXTENSION_VALID*/)

//! To set Extended custom feature select bit map
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
#ifdef CHIP_9117
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP                                                                                             \
    (EXT_FEAT_448K_M4SS_256K | EXT_FEAT_LOW_POWER_MODE | EXT_FEAT_XTAL_CLK_ENABLE | EXT_FEAT_IEEE_80211W)
#else /* !CHIP_9117 */
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP (EXT_FEAT_384K_MODE | EXT_FEAT_IEEE_80211W)
#endif /* CHIP_9117 */
#else  /* !WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
#ifdef CHIP_9117
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP (EXT_FEAT_448K_M4SS_256K | EXT_FEAT_LOW_POWER_MODE | EXT_FEAT_XTAL_CLK_ENABLE)
#else /* !CHIP_9117 */
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP EXT_FEAT_384K_MODE
#endif /* CHIP_9117 */
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */

//! To set Extended TCPIP feature select bit map
#define RSI_EXT_TCPIP_FEATURE_BITMAP (/*EXT_FEAT_HTTP_OTAF_SUPPORT |*/ EXT_TCP_IP_SSL_16K_RECORD)
//! Extended custom feature is selected internally
//! CCP         -- EXT_FEAT_256K_MODE
//! Wiseconnect -- EXT_FEAT_384K_MODE
/*=======================================================================*/
//! Feature frame parameters
/*=======================================================================*/
#define PLL_MODE 0
#define RF_TYPE 1 //! 0 - External RF 1- Internal RF
#define WIRELESS_MODE 0
#define ENABLE_PPP 0
#define AFE_TYPE 1
#define FEATURE_ENABLES 0
/*=======================================================================*/
//! Band command paramters
/*=======================================================================*/

//! RSI_BAND_2P4GHZ(2.4GHz) or RSI_BAND_5GHZ(5GHz) or RSI_DUAL_BAND
#define RSI_BAND RSI_BAND_2P4GHZ
/*=======================================================================*/
//! set region command paramters
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE Set region support
#define RSI_SET_REGION_SUPPORT RSI_DISABLE //@ RSI_ENABLE or RSI_DISABLE set region

//! If 1:region configurations taken from user ;0:region configurations taken from beacon
#define RSI_SET_REGION_FROM_USER_OR_BEACON 1

//! 0-Default Region domain ,1-US, 2-EUROPE, 3-JAPAN
#define RSI_REGION_CODE 1

//! 0- Without On Board Antenna , 1- With On Board Antenna
#define RSI_MODULE_TYPE 1

/*=======================================================================*/
//! set region AP command paramters
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE Set region AP support
#define RSI_SET_REGION_AP_SUPPORT RSI_DISABLE

//! If 1:region configurations taken from user ;0:region configurations taken from firmware
#define RSI_SET_REGION_AP_FROM_USER RSI_DISABLE

//! "US" or "EU" or "JP" or other region codes
#define RSI_COUNTRY_CODE "US"
/*=======================================================================*/

//! Rejoin parameters
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE rejoin params
#define RSI_REJOIN_PARAMS_SUPPORT RSI_ENABLE

//! Rejoin retry count. If 0 retries infinity times
#define RSI_REJOIN_MAX_RETRY 5

//! Periodicity of rejoin attempt
#define RSI_REJOIN_SCAN_INTERVAL 1

//! Beacon missed count
#define RSI_REJOIN_BEACON_MISSED_COUNT 40

//! RSI_ENABLE or RSI_DISABLE retry for first time join failure
#define RSI_REJOIN_FIRST_TIME_RETRY RSI_DISABLE

/*=======================================================================*/

//! BG scan command parameters
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE BG Scan support
#define RSI_BG_SCAN_SUPPORT RSI_ENABLE

//! RSI_ENABLE or RSI_DISABLE BG scan
#define RSI_BG_SCAN_ENABLE RSI_ENABLE

//! RSI_ENABLE or RSI_DISABLE instant BG scan
#define RSI_INSTANT_BG RSI_ENABLE

//! BG scan threshold value
#define RSI_BG_SCAN_THRESHOLD 63

//! RSSI tolerance Threshold
#define RSI_RSSI_TOLERANCE_THRESHOLD 4

//! BG scan periodicity
#define RSI_BG_SCAN_PERIODICITY 2

//! Active scan duration
#define RSI_ACTIVE_SCAN_DURATION 50

//! Passive scan duration
#define RSI_PASSIVE_SCAN_DURATION 50

//! Multi probe
#define RSI_MULTIPROBE RSI_ENABLE

/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE to set RTS threshold config
#define RSI_WLAN_CONFIG_ENABLE RSI_ENABLE

#define CONFIG_RTSTHRESHOLD 1

#define RSI_RTS_THRESHOLD 2346

/*=======================================================================*/

//! Roaming parameters
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE Roaming support
#define RSI_ROAMING_SUPPORT RSI_DISABLE

//! roaming threshold value
#define RSI_ROAMING_THRESHOLD 67

//! roaming hysterisis value
#define RSI_ROAMING_HYSTERISIS 4

/*=======================================================================*/
//! High Throughput Capabilies related information
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE 11n mode in AP mode
#define RSI_MODE_11N_ENABLE RSI_DISABLE

//! HT caps supported
#define RSI_HT_CAPS_NUM_RX_STBC (1 << 8)
#define RSI_HT_CAPS_SHORT_GI_20MHZ BIT(5)
#define RSI_HT_CAPS_GREENFIELD_EN BIT(4)
#define RSI_HT_CAPS_SUPPORT_CH_WIDTH BIT(1)

//! HT caps bit map.
#define RSI_HT_CAPS_BIT_MAP                                                                                                        \
    (RSI_HT_CAPS_NUM_RX_STBC | RSI_HT_CAPS_SHORT_GI_20MHZ | RSI_HT_CAPS_GREENFIELD_EN | RSI_HT_CAPS_SUPPORT_CH_WIDTH)

/*=======================================================================*/
//! Scan command parameters
/*=======================================================================*/

//! scan channel bit map in 2.4GHz band,valid if given channel to scan is 0
#define RSI_SCAN_CHANNEL_BIT_MAP_2_4 0

//! scan channle bit map in 5GHz band ,valid if given channel to scan is 0
#define RSI_SCAN_CHANNEL_BIT_MAP_5 0

//! scan_feature_bitmap ,valid only if specific channel to scan and ssid are given
#define RSI_SCAN_FEAT_BITMAP 0

/*=======================================================================*/
//! Enterprise configuration command parameters
/*=======================================================================*/

//! Enterprise method ,should be one of among TLS, TTLS, FAST or PEAP
#define RSI_EAP_METHOD "TTLS"
//! This parameter is used to configure the module in Enterprise security mode
#define RSI_EAP_INNER_METHOD "\"auth=MSCHAPV2\""
//! Private Key Password is required for encrypted private key, format is like "\"12345678\""
#define RSI_PRIVATE_KEY_PASSWORD ""
/*=======================================================================*/
//! AP configuration command parameters
/*=======================================================================*/

//! This Macro is used to enable AP keep alive functionality
#define RSI_AP_KEEP_ALIVE_ENABLE RSI_ENABLE

//! This parameter is used to configure keep alive type
#define RSI_AP_KEEP_ALIVE_TYPE RSI_NULL_BASED_KEEP_ALIVE

//! This parameter is used to configure keep alive period
#define RSI_AP_KEEP_ALIVE_PERIOD 100

//! This parameter is used to configure maximum stations supported
#define RSI_MAX_STATIONS_SUPPORT 4
/*=======================================================================*/
//! Join command parameters
/*=======================================================================*/

//! Tx power level
#define RSI_POWER_LEVEL RSI_POWER_LEVEL_HIGH

//! RSI_JOIN_FEAT_STA_BG_ONLY_MODE_ENABLE or RSI_JOIN_FEAT_LISTEN_INTERVAL_VALID
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
#define RSI_JOIN_FEAT_BIT_MAP RSI_JOIN_FEAT_MFP_CAPABLE_ONLY
#else
#define RSI_JOIN_FEAT_BIT_MAP 0
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANS */

#define RSI_LISTEN_INTERVAL 0

//! Transmission data rate. Physical rate at which data has to be transmitted.
#define RSI_DATA_RATE RSI_DATA_RATE_AUTO

/*=======================================================================*/
//! Ipconf command parameters
/*=======================================================================*/

//! DHCP client host name
#define RSI_DHCP_HOST_NAME "efr_9116"

//! Transmit test command parameters
/*=======================================================================*/
//! TX TEST rate flags
#define RSI_TX_TEST_RATE_FLAGS 0

//! TX TEST per channel bandwidth
#define RSI_TX_TEST_PER_CH_BW 0

//! TX TEST aggregation enable or disable
#define RSI_TX_TEST_AGGR_ENABLE RSI_DISABLE

//! TX TEST delay
#define RSI_TX_TEST_DELAY 0

/*======================================================================*/
//! ssl parameters
/*=======================================================================*/
//! ssl version
#define RSI_SSL_VERSION 0

//! ssl ciphers
#define RSI_SSL_CIPHERS SSL_ALL_CIPHERS

//! Enable TCP over SSL with TLS version depends on remote side
#define PROTOCOL_DFLT_VERSION BIT(0)

//! Enable TCP over SSL with TLS version 1.0
#define PROTOCOL_TLS_1_0 (BIT(0) | BIT(13))

//! Enable TCP over SSL with TLS version 1.1
#define PROTOCOL_TLS_1_1 (BIT(0) | BIT(14))

//! Enable TCP over SSL with TLS version 1.2
#define PROTOCOL_TLS_1_2 (BIT(0) | BIT(15))
/*=======================================================================*/
//! Power save command parameters
/*=======================================================================*/
//! set handshake type of power mode
#define RSI_HAND_SHAKE_TYPE GPIO_BASED

//! 0 - LP, 1- ULP mode with RAM retention and 2 - ULP with Non RAM retention
#define RSI_SELECT_LP_OR_ULP_MODE RSI_ULP_WITH_RAM_RET

//! set DTIM aligment required
//! 0 - module wakes up at beacon which is just before or equal to listen_interval
//! 1 - module wakes up at DTIM beacon which is just before or equal to listen_interval
#define RSI_DTIM_ALIGNED_TYPE 0

//! Monitor interval for the FAST PSP mode
//! default is 50 ms, and this parameter is valid for FAST PSP only
#define RSI_MONITOR_INTERVAL 50

//! Number of DTIMs to skip during powersave
#define RSI_NUM_OF_DTIM_SKIP 0

//! WMM PS parameters
//! set wmm enable or disable
#define RSI_WMM_PS_ENABLE RSI_DISABLE

//! set wmm enable or disable
//! 0- TX BASED 1 - PERIODIC
#define RSI_WMM_PS_TYPE 0

//! set wmm wake up interval
#define RSI_WMM_PS_WAKE_INTERVAL 20

//! set wmm UAPSD bitmap
#define RSI_WMM_PS_UAPSD_BITMAP 15

/*=======================================================================*/
//! Socket configuration
/*=======================================================================*/
//! RSI_ENABLE or RSI_DISABLE High performance socket
#define HIGH_PERFORMANCE_ENABLE RSI_ENABLE //@ RSI_ENABLE or RSI_DISABLE High performance socket
#define TOTAL_SOCKETS 10                   //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS 4                //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS 4                //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS 0              //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS 0              //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS 0              //@ Total UDP TX only sockets. UDP TX
#define UDP_RX_ONLY_SOCKETS 0              //@ Total UDP RX only sockets. UDP RX
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 1  //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP 10          //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR 10        //@ TCP RX Window division factor
/*=======================================================================*/

//! Socket Create parameters
/*=======================================================================*/

//! Initial timeout for Socket
#define RSI_SOCKET_KEEPALIVE_TIMEOUT 1200

//! VAP ID for Concurrent mode
#define RSI_VAP_ID 0

//! Timeout for join or scan
/*=======================================================================*/

//! RSI_ENABLE or RSI_DISABLE Timeout support
#define RSI_TIMEOUT_SUPPORT RSI_DISABLE

//! roaming threshold value
#define RSI_TIMEOUT_BIT_MAP 1

//! roaming hysterisis value
#define RSI_TIMEOUT_VALUE 1500

//! Timeout for ping request
/*=======================================================================*/

//! Timeout for PING_REQUEST
#define RSI_PING_REQ_TIMEOUT_MS 1000

//! Provide HTTP/HTTPS response status code indication to application e.g 200, 404 etc
/*=======================================================================*/
//! Enable or Diable feature
#define RSI_HTTP_STATUS_INDICATION_EN RSI_DISABLE
/*=======================================================================*/

//! Store Config Profile parameters
/*=======================================================================*/

//! Client profile
#define RSI_WLAN_PROFILE_CLIENT 0
//! P2P profile
#define RSI_WLAN_PROFILE_P2P 1
//! EAP profile
#define RSI_WLAN_PROFILE_EAP 2
//! AP profile
#define RSI_WLAN_PROFILE_AP 6
//! All profiles
#define RSI_WLAN_PROFILE_ALL 0xFF

//! AP Config Profile Parameters
/*==============================================================================*/

//! Transmission data rate. Physical rate at which data has to be transmitted.
#define RSI_CONFIG_AP_DATA_RATE RSI_DATA_RATE_AUTO
//! To set wlan feature select bit map
#define RSI_CONFIG_AP_WLAN_FEAT_BIT_MAP (FEAT_SECURITY_PSK)
//! TCP/IP feature select bitmap for selecting TCP/IP features
#define RSI_CONFIG_AP_TCP_IP_FEAT_BIT_MAP (TCP_IP_FEAT_DHCPV4_SERVER)
//! To set custom feature select bit map
#define RSI_CONFIG_AP_CUSTOM_FEAT_BIT_MAP 0
//! Tx power level
#define RSI_CONFIG_AP_TX_POWER RSI_POWER_LEVEL_HIGH
//! AP SSID
#define RSI_CONFIG_AP_SSID "SILABS_AP"
//! RSI_BAND_2P4GHZ(2.4GHz) or RSI_BAND_5GHZ(5GHz) or RSI_DUAL_BAND
#define RSI_CONFIG_AP_BAND RSI_BAND_2P4GHZ
//! To configure AP channle number
#define RSI_CONFIG_AP_CHANNEL 6
//! To configure security type
#define RSI_CONFIG_AP_SECURITY_TYPE RSI_WPA
//! To configure encryption type
#define RSI_CONFIG_AP_ENCRYPTION_TYPE 1
//! To configure PSK
#define RSI_CONFIG_AP_PSK "1234567890"
//! To configure beacon interval
#define RSI_CONFIG_AP_BEACON_INTERVAL 100
//! To configure DTIM period
#define RSI_CONFIG_AP_DTIM 2
//! This parameter is used to configure keep alive type
#define RSI_CONFIG_AP_KEEP_ALIVE_TYPE 0 //! RSI_NULL_BASED_KEEP_ALIVE

#define RSI_CONFIG_AP_KEEP_ALIVE_COUNTER 0 //! 100
//! This parameter is used to configure keep alive period
#define RSI_CONFIG_AP_KEEP_ALIVE_PERIOD 100
//! This parameter is used to configure maximum stations supported
#define RSI_CONFIG_AP_MAX_STATIONS_COUNT 4
//! P2P Network parameters
//! TCP_STACK_USED BIT(0) - IPv4, BIT(1) -IPv6, (BIT(0) | BIT(1)) - Both IPv4 and IPv6
#define RSI_CONFIG_AP_TCP_STACK_USED BIT(0)
//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define RSI_CONFIG_AP_IP_ADDRESS 0x0A0AA8C0
//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define RSI_CONFIG_AP_SN_MASK_ADDRESS 0x00FFFFFF
//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define RSI_CONFIG_AP_GATEWAY_ADDRESS 0x0A0AA8C0

//! Client Profile Parameters
/* ===================================================================================== */

//! To configure data rate
#define RSI_CONFIG_CLIENT_DATA_RATE RSI_DATA_RATE_AUTO
//! To configure wlan feature bitmap
#define RSI_CONFIG_CLIENT_WLAN_FEAT_BIT_MAP 0
//! To configure tcp/ip feature bitmap
#define RSI_CONFIG_CLIENT_TCP_IP_FEAT_BIT_MAP BIT(2)
//! To configure custom feature bit map
#define RSI_CONFIG_CLIENT_CUSTOM_FEAT_BIT_MAP 0
//! To configure TX power
#define RSI_CONFIG_CLIENT_TX_POWER RSI_POWER_LEVEL_HIGH
//! To configure listen interval
#define RSI_CONFIG_CLIENT_LISTEN_INTERVAL 0
//! To configure SSID
#define RSI_CONFIG_CLIENT_SSID "Matter_9116"
//! RSI_BAND_2P4GHZ(2.4GHz) or RSI_BAND_5GHZ(5GHz) or RSI_DUAL_BAND
#define RSI_CONFIG_CLIENT_BAND RSI_BAND_2P4GHZ
//! To configure channel number
#define RSI_CONFIG_CLIENT_CHANNEL 0
//! To configure security type
#define RSI_CONFIG_CLIENT_SECURITY_TYPE 0 // RSI_WPA
//! To configure encryption type
#define RSI_CONFIG_CLIENT_ENCRYPTION_TYPE 0
//! To configure PSK
#define RSI_CONFIG_CLIENT_PSK "1234567890"
//! To configure PMK
#define RSI_CONFIG_CLIENT_PMK ""
//! Client Network parameters
//! TCP_STACK_USED BIT(0) - IPv4, BIT(1) -IPv6, (BIT(0) | BIT(1)) - Both IPv4 and IPv6
#define RSI_CONFIG_CLIENT_TCP_STACK_USED BIT(0)
//! DHCP mode 1- Enable 0- Disable
//! If DHCP mode is disabled given IP statically
#define RSI_CONFIG_CLIENT_DHCP_MODE RSI_DHCP
//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define RSI_CONFIG_CLIENT_IP_ADDRESS 0x0A0AA8C0
//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define RSI_CONFIG_CLIENT_SN_MASK_ADDRESS 0x00FFFFFF
//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define RSI_CONFIG_CLIENT_GATEWAY_ADDRESS 0x010AA8C0
//! scan channel bit map in 2.4GHz band,valid if given channel to scan is 0
#define RSI_CONFIG_CLIENT_SCAN_FEAT_BITMAP 0
//! Scan channel magic code
#define RSI_CONFIG_CLIENT_MAGIC_CODE 0x4321
//! scan channel bit map in 2.4GHz band,valid if given channel to scan is 0
#define RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_2_4_GHZ 0
//! scan channle bit map in 5GHz band ,valid if given channel to scan is 0
#define RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_5_0_GHZ 0

//! EAP Client Profile Parameters
/* =========================================================================== */

//! To configure data rate
#define RSI_CONFIG_EAP_DATA_RATE RSI_DATA_RATE_AUTO
//! To configure wlan feature bitmap
#define RSI_CONFIG_EAP_WLAN_FEAT_BIT_MAP 0
//! To configure tcp/ip feature bitmap
#define RSI_CONFIG_EAP_TCP_IP_FEAT_BIT_MAP BIT(2)
//! To configure custom feature bit map
#define RSI_CONFIG_EAP_CUSTOM_FEAT_BIT_MAP 0
//! To configure EAP TX power
#define RSI_CONFIG_EAP_TX_POWER RSI_POWER_LEVEL_HIGH
//! To Configure scan channel feature bitmap
#define RSI_CONFIG_EAP_SCAN_FEAT_BITMAP 0
//! scan channel bit map in 2.4GHz band,valid if given channel to scan is 0
#define RSI_CONFIG_EAP_CHAN_MAGIC_CODE 0 // 0x4321
//! scan channel bit map in 2.4GHz band,valid if given channel to scan is 0
#define RSI_CONFIG_EAP_SCAN_CHAN_BITMAP_2_4_GHZ 0
//! scan channle bit map in 5GHz band ,valid if given channel to scan is 0
#define RSI_CONFIG_EAP_SCAN_CHAN_BITMAP_5_0_GHZ 0
//! To configure SSID
#define RSI_CONFIG_EAP_SSID "SILABS_AP"
//! RSI_BAND_2P4GHZ(2.4GHz) or RSI_BAND_5GHZ(5GHz) or RSI_DUAL_BAND
#define RSI_CONFIG_EAP_BAND RSI_BAND_2P4GHZ
//! To set security type
#define RSI_CONFIG_EAP_SECURITY_TYPE RSI_WPA2_EAP
//! To set encryption type
#define RSI_CONFIG_EAP_ENCRYPTION_TYPE 0
//! To set channel number
#define RSI_CONFIG_EAP_CHANNEL 0
//! Enterprise method ,should be one of among TLS, TTLS, FAST or PEAP
#define RSI_CONFIG_EAP_METHOD "TLS"
//! This parameter is used to configure the module in Enterprise security mode
#define RSI_CONFIG_EAP_INNER_METHOD "\"auth=MSCHAPV2\""
//! To configure EAP user identity
#define RSI_CONFIG_EAP_USER_IDENTITY "\"user1\""
//! TO configure EAP password
#define RSI_CONFIG_EAP_PASSWORD "\"test123\""
//! EAP Network parameters
//! TCP_STACK_USED BIT(0) - IPv4, BIT(1) -IPv6, (BIT(0) | BIT(1)) - Both IPv4 and IPv6
#define RSI_CONFIG_EAP_TCP_STACK_USED BIT(0)
//! DHCP mode 1- Enable 0- Disable
//! If DHCP mode is disabled given IP statically
#define RSI_CONFIG_EAP_DHCP_MODE RSI_DHCP
//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define RSI_CONFIG_EAP_IP_ADDRESS 0x0A0AA8C0
//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define RSI_CONFIG_EAP_SN_MASK_ADDRESS 0x00FFFFFF
//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define RSI_CONFIG_EAP_GATEWAY_ADDRESS 0x010AA8C0

//! P2P Profile parameters
/* ================================================================================= */

//! To configure data rate
#define RSI_CONFIG_P2P_DATA_RATE RSI_DATA_RATE_AUTO
//! To configure wlan feature bitmap
#define RSI_CONFIG_P2P_WLAN_FEAT_BIT_MAP 0
//! To configure P2P tcp/ip feature  bitmap
#define RSI_CONFIG_P2P_TCP_IP_FEAT_BIT_MAP BIT(2)
//! To configure P2P custom feature bitmap
#define RSI_CONFIG_P2P_CUSTOM_FEAT_BIT_MAP 0
//! TO configure P2P tx power level
#define RSI_CONFIG_P2P_TX_POWER RSI_POWER_LEVEL_HIGH
//! Set P2P go intent
#define RSI_CONFIG_P2P_GO_INTNET 16 //! Support only Autonomous GO mode
//! Set device name
#define RSI_CONFIG_P2P_DEVICE_NAME "WSC1.1"
//! Set device operating channel
#define RSI_CONFIG_P2P_OPERATING_CHANNEL 11
//! Set SSID postfix
#define RSI_CONFIG_P2P_SSID_POSTFIX "WSC_1_0_0"
//! Set P2P join SSID
#define RSI_CONFIG_P2P_JOIN_SSID "SILABS_AP"
//! Set psk key
#define RSI_CONFIG_P2P_PSK_KEY "12345678"
//! P2P Network parameters
//! TCP_STACK_USED BIT(0) - IPv4, BIT(1) -IPv6, (BIT(0) | BIT(1)) - Both IPv4 and IPv6
#define RSI_CONFIG_P2P_TCP_STACK_USED BIT(0)
//! DHCP mode 1- Enable 0- Disable
//! If DHCP mode is disabled given IP statically
#define RSI_CONFIG_P2P_DHCP_MODE 1
//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define RSI_CONFIG_P2P_IP_ADDRESS 0x0A0AA8C0
//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define RSI_CONFIG_P2P_SN_MASK_ADDRESS 0x00FFFFFF
//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define RSI_CONFIG_P2P_GATEWAY_ADDRESS 0x010AA8C0

#endif
